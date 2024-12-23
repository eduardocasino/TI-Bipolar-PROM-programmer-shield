/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
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
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <stdint.h>
#include <stdbool.h>

#include "globals.h"
#include "options.h"
#include "serial.h"
#include "files.h"
#include "ihex.h"
#include "binfile.h"
#include "command.h"

#define RETRIES 1

static status_t cleanup( int fd, status_t status )
{
    if ( fd != -1 )
    {
        serial_close( fd );
    }

    return status;
}

int main( int argc, char **argv )
{
    options_t options;
    int fd = -1;
    status_t ret;

    ret = get_options( &options, argc, argv );

    if ( ret == SUCCESS ) ret = serial_init( &fd, options.device );

    if ( ret == SUCCESS ) ret = command_init( fd, options.device );

    if ( ret == SUCCESS ) ret = options.command->function(
                                        fd,
                                        options.device,
                                        options.chip,
                                        options.flags.address ? options.address : 0xFFFF,
                                        options.flags.count ? options.count : 0xFFFF,
                                        options.data,
                                        options.ifile,
                                        options.ofile,
                                        options.format );

    return cleanup( fd, ret );

}