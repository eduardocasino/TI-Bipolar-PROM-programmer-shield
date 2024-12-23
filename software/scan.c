/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Scanning support functions
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
/*
 * memcfg - A command line utility for managing the Pico KIM-1 Memory Emulator board
 *   https://github.com/eduardocasino/kim-1-programmable-memory-card
 *
 * Parsing support functions
 * 
 *  Copyright (C) 2024 Eduardo Casino
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

enum uint_len_t { UINT8 = 2, UINT16 = 4, UINT32 = 8 };

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

int get_uint16( char *optarg, uint16_t *value )
{
    char *endc;

    uint64_t number = strtoul( optarg, &endc, 0 );

    if ( !( isblank( *++endc ) || *endc  ) || number > 0xffff )
    {
        return EINVAL;
    }

    *value = (uint16_t) number;

    return 0;
}

int get_uint8( char *optarg, uint8_t *value )
{
    uint16_t number;

    int ret = get_uint16( optarg, &number );

    if ( 0 == ret )
    {
        if ( number > 0xff )
        {
            ret = EINVAL;
        }
        *value = (uint8_t) number;
    }

    return ret;
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

int get_octbyte( const char *s, uint8_t *byte )
{
    static const int base8[] = { 6, 3, 0 };

    if ( strlen( s ) < 3 )
    {
        return EINVAL;
    }

    *byte = 0;

    for ( int pos = 0; pos < 3; ++pos )
    {
        int val = s[pos] - '0';

        if ( !isdigit( s[pos] ) || val > 7 )
        {
            return EINVAL;
        }
        *byte += val << base8[pos];
    }

    return 0;
}
