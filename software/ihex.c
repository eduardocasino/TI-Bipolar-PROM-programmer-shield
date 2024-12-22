/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Intel HEX format support
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#include "globals.h"
#include "files.h"

#define LINE_BUF_LEN 1024
#define INTEL_WRITE_BYTES_PER_LINE 32

typedef struct {
    char start_char;
    uint8_t *buffer;
    size_t buffer_size;
    int32_t current_addr;
    uint16_t lines;
    uint16_t checksum;
    char *line_buffer;
    int line_index;
    mem_block_t *blocks;
    bool complete;
} hexfile_t;

enum uint_len_t { UINT8 = 2, UINT16 = 4 };

static uint8_t hexvalue( char digit )
{
    uint8_t value;

    char c = toupper( digit );

    if ( isdigit( c ))
    {
        value = digit - '0';
    }
    else
    {
        value = digit - 'A' + 10;
    }

    return value;
}

static int get_hexnum( int len, const char *s )
{
    static const int base16[] = { 12, 8, 4, 0 };

    int result = 0;

    if ( strlen( s ) < len )
    {   
        return -1;
    }

    for ( int pos = 0; pos < len; ++pos )
    {
        int val;
        char c = s[pos];

        if ( !isxdigit( c ) )
        {
            return -1;
        }
        result |= (hexvalue( c ) << base16[4 - len + pos] ) & ( 0xF << base16[4 - len + pos] );
    }

    return result;
}

int get_hexbyte( const char *s, uint8_t *byte )
{
    int result = get_hexnum( UINT8, s );

    if ( result < 0 )
    {
        return EINVAL;
    }

    *byte = (uint8_t) result;

    return 0;
}

int get_hexword( const char *s, uint16_t *word )
{
    int result = get_hexnum( UINT16, s );

    if ( result < 0 )
    {
        return EINVAL;
    }

    *word = (uint16_t) result;

    return 0;
}

static status_t get_byte( char *buffer, int *index, uint8_t *value )
{    
    if ( get_hexbyte( &buffer[*index], value ) )
    {
        return FAILURE;
    }

    *index += 2;

    return SUCCESS;
}

static status_t get_word( char *buffer, int *index, uint16_t *value )
{
    if ( get_hexword( &buffer[*index], value ) )
    {
        return FAILURE;
    }

    *index += 4;

    return SUCCESS;
}

static status_t ihex_read_record( hexfile_t *hex )
{
    status_t status = SUCCESS;
    uint8_t line_bytes;
    uint16_t load_address;
    uint8_t record_type;
    uint8_t nbytes;
    uint16_t checksum;

    ++hex->lines;
    hex->line_index = 0;

    if ( hex->start_char != hex->line_buffer[hex->line_index++] )
    {
        fprintf( stderr, "Malformed hex file at line %d\n", hex->lines );
        return FAILURE;
    }

    if ( FAILURE == get_byte( hex->line_buffer, &hex->line_index, &line_bytes ) )
    {
        fprintf( stderr, "Invalid register length in hex file at line %d\n", hex->lines );
        return FAILURE;
    }

    // line length should be:
    //  1 for ':'
    //  2 for byte count
    //  4 for address
    //  2 for record type ( 0 for PAP )
    //  2 * byte count for data
    //  2 for checksum ( 4 for PAP)
    //  == 11 + ( 2 * byte count ) in any case
    // 
    if ( strlen( hex->line_buffer ) < 11 + 2 * line_bytes )
    {
        fprintf( stderr, "Malformed hex file: line %d is too short\n", hex->lines );
        return FAILURE;
    }

    if ( 0 == line_bytes )
    {
        if ( ! strncmp( ":00000001FF", hex->line_buffer, 11 ) )
        {
            hex->complete = true;
            return SUCCESS;
        }
        return FAILURE;
    }

    if ( FAILURE == get_word( hex->line_buffer, &hex->line_index, &load_address ) || load_address >= hex->buffer_size )
    {
        fprintf( stderr, "Invalid load address in hex file at line %d\n", hex->lines );
        return FAILURE;
    }

    if ( FAILURE == get_byte( hex->line_buffer, &hex->line_index, &record_type ) || record_type != 0 )
    {
        // Only data records are supported
        fprintf( stderr, "Invalid or unsupported record type in hex file at line %d\n", hex->lines );
        return FAILURE;
    }

    if ( load_address != hex->current_addr )
    {
        // New block
        mem_block_t *newblock = malloc( sizeof( sizeof( mem_block_t ) ) );

        if ( NULL == newblock )
        {
            perror( "Can't alloc memory for new hex block\n" );
            return FAILURE;
        }

        newblock->start = load_address;
        newblock->count = 0;
        newblock->next = hex->blocks;
        hex->blocks = newblock;
        hex->current_addr = load_address;
    }

    hex->checksum = line_bytes + ( ( load_address >> 8 ) & 0xFF ) + ( load_address & 0xFF );

    for ( nbytes = 0; nbytes < line_bytes; ++nbytes )
    {
        uint8_t byte;

        if ( FAILURE == get_byte( hex->line_buffer, &hex->line_index, &byte ) )
        {
            fprintf( stderr, "Malformed hex file: Invalid byte at line %d\n", hex->lines );
            return FAILURE;
        }
        hex->buffer[hex->current_addr++] = byte;
        hex->blocks->count++;
        hex->checksum += byte;

    }

    // Get checksum
    status = get_byte( hex->line_buffer, &hex->line_index, (uint8_t *)&checksum );

    if ( FAILURE == status )
    {
        fprintf( stderr, "Malformed hex file: can't get checksum at line %d\n", hex->lines );
        return FAILURE;
    }

    if ( checksum != (uint8_t)(~hex->checksum + 1 ) )
    {
        fprintf( stderr, "Malformed hex file: bad checksum at line %d\n", hex->lines );
        return FAILURE;
    }

    return status;
}

status_t ihex_read( char *filename, uint8_t* data, size_t size, mem_block_t **blocks )
{
    status_t status = SUCCESS;
    static char line_buf[LINE_BUF_LEN];
    hexfile_t hex = { 0 };
    FILE *file;

    if ( NULL == ( file = fopen( filename, "r" ) ) )
    {
        fprintf( stderr, "Error %d opening file '%s': %s\n", errno, filename, strerror( errno ) );
        return FAILURE;
    }
    
    hex.start_char = ':';
    hex.buffer = data;
    hex.buffer_size = size;
    hex.line_buffer = line_buf;
    hex.current_addr = -1;

    while ( fgets( line_buf, sizeof( line_buf ), file ) > 0 )
    {
        if ( FAILURE == ( status = ihex_read_record( &hex ) ) )
        {
            files_free_blocks( hex.blocks );
            hex.blocks = NULL;
            break;
        }
    }

    if ( SUCCESS == status && ( !feof( file ) || ! hex.complete ) )
    {
        fputs( "Unexpected end of hex file\n", stderr );
        status = FAILURE;
    }

    *blocks = hex.blocks;

    fclose( file );

    return status;
}

status_t ihex_write( char *filename, uint8_t* data, size_t size, uint64_t base_addr )
{
    status_t status = SUCCESS;
    int rc;
    int byte_num = 0;
    uint16_t checksum = 0;
    uint16_t lines = 0;
    char start_char = ':';
    uint8_t max_bytes_per_line = INTEL_WRITE_BYTES_PER_LINE;
    FILE *file;

    if ( NULL == ( file = fopen( filename, "w" ) ) )
    {
        fprintf( stderr, "Error %d opening file '%s': %s\n", errno, filename, strerror( errno ) );
        return FAILURE;
    }

    while ( byte_num < size )
    {
        if ( ! (byte_num % max_bytes_per_line) )
        {
            ++lines;

            // New line
            if ( byte_num )
            {
                // Print checksum
                rc = fprintf( file, "%2.2X\n", (uint8_t)(~checksum + 1 ) );
                
                if ( rc < 0 ) status = FAILURE;
            }
            uint8_t bytes_in_line = size - byte_num > max_bytes_per_line ? max_bytes_per_line : size - byte_num;
            checksum = bytes_in_line + ( ( base_addr >> 8 ) & 0xFF ) + ( base_addr & 0xFF );
            rc = fprintf( file, "%c%2.2X%4.4X", start_char, bytes_in_line, (uint16_t) base_addr );
            rc = fputs( "00", file );
            if ( rc < 0 ) status = FAILURE;
        }
        rc = fprintf( file, "%2.2X", data[byte_num] );
        if ( rc < 0 ) status = FAILURE;

        checksum += data[byte_num];
        ++byte_num; ++base_addr;
    }

    if ( fprintf( file, "%2.2X\n", (uint8_t)(~checksum + 1 ) ) < 0
            || fputs( ":00000001FF\n", file ) < 0 )
    {
        status = FAILURE;
    }    

    if ( EOF == fclose( file ) )
    {
        fprintf( stderr, "Error %d closing file '%s': %s\n", errno, filename, strerror( errno ) );
        status = FAILURE;
    }

    return status;
}

