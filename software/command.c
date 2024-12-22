/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Command execution
 * 
 * (C) 2024 Eduardo Casino
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the “Software”), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "serial.h"
#include "hexdump.h"
#include "files.h"

#define REC_BUF_SIZE    100
#define RW_BUF_SIZE     4096

static const uint16_t chip_sizes[] =
{
    256, 512
};

static uint8_t rec_buf[REC_BUF_SIZE];
static uint8_t rw_buf[RW_BUF_SIZE];

status_t command_blank(
    int fd,
    char *device,
    uint8_t chip,
    uint16_t address,
    uint8_t data,
    char *ifile,
    char *ofile,
    const format_st_t *format )
{
    int n, tries;
    ssize_t returned;
    uint16_t end;
    uint8_t ret_stat;

    sprintf( rec_buf, "K %x\n", chip );

    if ( FAILURE == serial_write( fd, device, rec_buf, strlen( rec_buf ) ) )
    {
        return FAILURE;            
    }
        
    for ( tries = 0; tries < 5; ++tries )
    {
        if ( FAILURE == serial_read( fd, device, rec_buf, sizeof( rec_buf ), &returned ) )
        {
            return FAILURE;
        }

        if ( returned != 0 )
        {
            break;
        }
    }

    if ( returned == 0 )
    {
        fprintf( stderr, "Error: No response from programmer at port %s.\n", device );
        return FAILURE;
    }

    if ( 2 != sscanf( rec_buf, "%hx\r\n%c\r\n", &end, &ret_stat ) || ret_stat != 'R' )
    {
        fputs( "Error executing blank test. Bad programmer response.\n", stderr );
        return FAILURE;
    }

    fputs( "Chip is ", stdout );

    if ( end == chip_sizes[chip] )
    {
        puts( "blank." );
    }
    else
    {
        printf( "not blank. Found non-zero data at address 0x%x.\n", end );
    }

    return SUCCESS;
}

status_t command_read(
    int fd,
    char *device,
    uint8_t chip,
    uint16_t address,
    uint8_t data,
    char *ifile,
    char *ofile,
    const format_st_t *format )
{
    int n, tries;
    ssize_t returned;
    uint8_t ret_stat;
    size_t start, end;

    fputs( "Reading\n", stderr );
    if ( address != 0xFFFF )
    {
        start = address;
        end = address+1;
    }
    else
    {
        start = 0;
        end = chip_sizes[chip];
    }

    for ( size_t i = start; i < end; ++i )
    {
        putc( '.', stderr );
        sprintf( rec_buf, "r %x %x\n", chip, (uint16_t) i );

        if ( FAILURE == serial_write( fd, device, rec_buf, strlen( rec_buf ) ) )
        {
            return FAILURE;            
        }
        
        for ( tries = 0; tries < 5; ++tries )
        {
            if ( FAILURE == serial_read( fd, device, rec_buf, sizeof( rec_buf ), &returned ) )
            {
                return FAILURE;
            }

            if ( returned != 0 )
            {
                break;
            }
        }

        if ( returned == 0 )
        {
            fprintf( stderr, "\nError: No response from programmer at port %s.\n", device );
            return FAILURE;
        }

        if ( 2 != sscanf( rec_buf, "%hhx\r\n%c\r\n", &rw_buf[i], &ret_stat ) || ret_stat != 'R' )
        {
            fputs( "\nError reading from prom. Bad programmer response.\n", stderr );
            return FAILURE;
        }

        if ( ! ((i+1) % 73) )
        {
            fputs( "\n", stderr );
        } 
    }

    fputs( "\n", stderr );
    fputs( "Success.\n", stderr );

    if ( ofile )
    {
        status_t status;

        status = format->write_fn( ofile, rw_buf, chip_sizes[chip], start );

        return status;
    }
    else
    {
        hexdump( &rw_buf[start], end-start, start );
        return SUCCESS;
    }
}

static status_t command_execute(
    char command,
    const char *message,
    int fd,
    char *device,
    uint8_t chip,
    uint16_t address,
    uint8_t data,
    char *ifile,
    char *ofile,
    const format_st_t *format )
{
    status_t status = SUCCESS;
    mem_block_t *b, *blocks = NULL;
    int tries;
    ssize_t returned;
    uint8_t ret_stat;
    uint8_t ret_val;


    if ( ifile )
    {
        if ( FAILURE == format->read_fn( ifile, rw_buf, sizeof( rw_buf ), &blocks ) )
        {
            return FAILURE;
        }
    }
    else
    {
        blocks = malloc( sizeof( sizeof( mem_block_t ) ) );

        if ( NULL == blocks )
        {
            perror( "Can't alloc memory for new hex block\n" );
            return FAILURE;
        }
        rw_buf[address] = data;
        blocks->start = address;
        blocks->count = 1;
        blocks->next = NULL;
    }

    b = blocks;
    while ( NULL != b )
    {
        uint16_t loc, end = b->start + b->count; 

        for ( loc = b->start; loc < end; ++loc )
        {
            if ( loc >= chip_sizes[chip] )
            {
                fprintf( stderr, "Address 0x%X is larger than last chip cell ( 0x%X )\n", loc, chip_sizes[chip]-1 );
                status = FAILURE;
                break;
            }

            putc( '.', stderr );
            sprintf( rec_buf, "%c %x %x %x\n", command, chip, loc, rw_buf[loc] );

            if ( FAILURE == serial_write( fd, device, rec_buf, strlen( rec_buf ) ) )
            {
                status = FAILURE;
                break;            
            }
        
            for ( tries = 0; tries < 5; ++tries )
            {
                if ( FAILURE == (status = serial_read( fd, device, rec_buf, sizeof( rec_buf ), &returned ) ) )
                {
                    break;
                }

                if ( returned != 0 )
                {
                    break;
                }
            }

            if ( status == FAILURE )
            {
                break;
            }

            if ( returned == 0 )
            {
                fprintf( stderr, "\nError: No response from programmer at port %s.\n", device );
                status = FAILURE;
                break;
            }

            if ( 2 != sscanf( rec_buf, "%hhx\r\n%c\r\n", &ret_val, &ret_stat ) || ret_stat != 'R' )
            {
                fputs( "\nError: Bad programmer response.\n", stderr );
                status = FAILURE;
                break;
            }

            if ( ! ((loc+1) % 73) )
            {
                fputs( "\n", stderr );
            }            
            
            if ( ret_val != rw_buf[loc] )
            {
                fprintf( stderr, "\nError %s prom address 0x%03X: Read == 0x%02x, expected == 0x%02x\n",
                            message,
                            loc,
                            ret_val,
                            rw_buf[loc]);
                status = FAILURE;
                break;
            } 
        }

        fputs( "\n", stderr );

        if ( status == FAILURE )
        {
            break;
        }
        b = b->next;
    }

    files_free_blocks( blocks );

    if ( status == SUCCESS )
    {
        fputs( "Success.\n", stderr );
    }

    return status;
}

status_t command_write(
    int fd,
    char *device,
    uint8_t chip,
    uint16_t address,
    uint8_t data,
    char *ifile,
    char *ofile,
    const format_st_t *format )
{
    fputs( "Writing\n", stderr );
    return command_execute( 'w', "writing to", fd, device, chip, address, data, ifile, ofile, format );
}

status_t command_simul(
    int fd,
    char *device,
    uint8_t chip,
    uint16_t address,
    uint8_t data,
    char *ifile,
    char *ofile,
    const format_st_t *format )
{
    fputs( "Performing a write simulation\n", stderr );
    return command_execute( 's', "writing (simulated) to", fd, device, chip, address, data, ifile, ofile, format );
}

status_t command_verify(
    int fd,
    char *device,
    uint8_t chip,
    uint16_t address,
    uint8_t data,
    char *ifile,
    char *ofile,
    const format_st_t *format )
{
    fputs( "Verifying\n", stderr );
    return command_execute( 'r', "verifying", fd, device, chip, address, data, ifile, ofile, format );
}

status_t command_init( int fd, char *device )
{
    int n, tries;
    ssize_t returned;

    // Flush does not work for USB adapters, so we just discard any data that
    // does not fit with what we expect
    //
    for ( tries = 0; tries < 5; ++tries )
    {
        if ( FAILURE == serial_write( fd, device, "V", 1 ) )                   // Get version
        {
            return FAILURE;
        }

        if ( FAILURE == serial_read( fd, device, rec_buf, sizeof( rec_buf ), &returned ) )
        {
            return FAILURE;
        }

        if ( returned == 12 )
        {
            break;
        }
    }
    
    uint8_t v1, v2, v3, stat;

    if ( returned != 12 || 4 != sscanf( rec_buf, "%*[V]%2hhd%2hhd%2hhd\r\n%c\r\n", &v1, &v2, &v3, &stat ) || stat != 'R' )
    {
        fprintf( stderr, "Error: Programmer not detected at port %s\n", device );
        return FAILURE;
    }
    else
    {
        fprintf( stderr, "Connected to programmer, firmware V%2.2d.%2.2d.%2.2d.\n", v1, v2, v3 );
        return SUCCESS;
    }

}