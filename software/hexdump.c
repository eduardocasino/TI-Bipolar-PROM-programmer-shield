/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Human readable hexadecimal dump
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
#include <ctype.h>

#define COLUMNS 16
#define HALF_COLUMNS ( COLUMNS / 2 )

void hexdump( uint8_t* data, size_t size, uint16_t base_addr )
{
	static char ascii[COLUMNS + 1];
	
	for ( size_t i = 0; i < size+1; ++i )
    {
        if ( ! ( i % COLUMNS ) )
        {
            printf( "%03X  ", base_addr + (uint16_t) i );
            memset( ascii, 0, sizeof( ascii) );
        }
        else if ( ! ( i % HALF_COLUMNS ) )
        {
            fputs( " ", stdout );
        }

        if ( i == size )
        {
            break;
        }

		printf( "%02x ", ((uint8_t *)data)[i] );

		ascii[i % COLUMNS] = isprint( ((uint8_t *)data)[i] ) ? ((uint8_t *)data)[i] : '.';

		if ( (i+1) % COLUMNS == 0 || i+1 == size )
        {
            size_t j;
			for ( j = (i+1) % COLUMNS; j && j < COLUMNS; ++j )
            {
				fputs( "   ", stdout );
			}
            if ( j && (i+1) % COLUMNS <= HALF_COLUMNS )
            {
				fputs( " ", stdout );
		    }
		    printf( " |%-16s|\n", ascii);
		}
	}

    return;
}