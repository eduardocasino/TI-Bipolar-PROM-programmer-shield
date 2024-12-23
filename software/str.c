/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Binary string support
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
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "scan.h"

#define STR_BUFFER_LENGTH 1024

status_t str_process( const char *str, uint8_t *buffer, size_t size, uint16_t *len )
{
    size_t i = 0;
    int skip;
    status_t status = SUCCESS;

    while ( *str != '\0' && i < size )
    {
        // Check for escape
        if ( *str == '\\' )
        {
            switch ( str[1] )
            {
                case '\\':
                    buffer[i++] = '\\';
                    skip = 2;
                    break;

                case '"':
                    buffer[i++] = '"';
                    skip = 2;
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                    ++str;
                    skip = get_octbyte( str, &buffer[i++] ) ? -1 : 3;
                    break;

                case 'x':
                    str += 2;
                    skip = get_hexbyte( str, &buffer[i++] ) ? -1 : 2;
                    break;

                default:
                    skip = -1;
            }

            if ( skip < 0 )
            {
                fputs( "Error: Invalid escape sequence\n", stderr );
                status = FAILURE;
                break;
            }

            str += skip;
        }
        else
        {
            buffer[i++] = *(str++);
        }

    }

    if ( i == size )
    {
        fputs( "Error: Data string too long.\n", stderr);
        *len = 0;
        status = FAILURE;
    }
    else
    {
        *len = (uint16_t) i;
    }
    
    return status;
} 