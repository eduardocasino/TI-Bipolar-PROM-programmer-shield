/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Support functions for binary file formats
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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "globals.h"
#include "files.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

status_t bin_read( char *filename, uint8_t* data, size_t size, mem_block_t **blocks )
{
    FILE *file = NULL;
    status_t status = SUCCESS;
    mem_block_t *b = malloc( sizeof( mem_block_t) );

    if ( NULL == b )
    {
        perror( "Can't alloc memory for new bin block\n" );
        return files_cleanup( file, b, FAILURE );
    }

    if ( NULL == ( file = fopen( filename, "rb" ) ) )
    {
        fprintf( stderr, "Error %d opening file '%s': %s\n", errno, filename, strerror( errno ) );
        return files_cleanup( file, b, FAILURE );
    }
    
    if ( fseek( file, 0L, SEEK_END ) )
    {
        perror( "Can't determine file size" );
        return files_cleanup( file, b, FAILURE );
    }

    b->start = 0;
    b->count = ftell( file );
    b->next = NULL;

    rewind( file );

    if ( b->count > size || b->count == 0 )
    {
        fputs( "Invalid file size\n", stderr );
        return files_cleanup( file, b, FAILURE );
    }

    *blocks = b;

    if ( fread( data, 1, b->count, file ) != b->count )
    {
        perror( "Error reading from binary file" );
        return files_cleanup( file, b, FAILURE );
    }

    fclose( file );

    return SUCCESS;
}

status_t bin_write( char *filename, uint8_t* data, size_t size, uint64_t base_addr )
{
    FILE *file;

    UNUSED( base_addr );

    status_t status = SUCCESS;
    int rc;

    if ( NULL == ( file = fopen( filename, "wb" ) ) )
    {
        fprintf( stderr, "Error %d opening file '%s': %s\n", errno, filename, strerror( errno ) );
        return FAILURE;
    }
    
	for ( size_t i = 0; i < size; ++i )
    {
        rc = fwrite( &data[i], 1, 1, file );
        if ( rc == 0 )
        {
            fprintf( stderr, "Error writing to file '%s'\n", filename );
            status = FAILURE;
        }
    }

    if ( EOF == fclose( file ) )
    {
        fprintf( stderr, "Error %d closing file '%s': %s\n", errno, filename, strerror( errno ) );
        status = FAILURE;
    }

    return status;
}
