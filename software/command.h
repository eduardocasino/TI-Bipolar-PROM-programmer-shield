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

#ifndef COMMAND_H
#define COMMAND_H

#include "globals.h"
#include "options.h"
#include "files.h"

typedef status_t (*cmd_fn_t)( int fd, char *device, uint8_t chip, uint16_t address, uint8_t data, char *ifile, char *ofile, const format_st_t *format );

status_t command_init( int fd, char *device );
status_t command_blank( int fd, char *device, uint8_t chip, uint16_t address, uint8_t data, char *ifile, char *ofile, const format_st_t *format );
status_t command_read( int fd, char *device, uint8_t chip, uint16_t address, uint8_t data, char *ifile, char *ofile, const format_st_t *format );
status_t command_write( int fd, char *device, uint8_t chip, uint16_t address, uint8_t data, char *ifile, char *ofile, const format_st_t *format );
status_t command_simul( int fd, char *device, uint8_t chip, uint16_t address, uint8_t data, char *ifile, char *ofile, const format_st_t *format );
status_t command_verify( int fd, char *device, uint8_t chip, uint16_t address, uint8_t data, char *ifile, char *ofile, const format_st_t *format );

#endif /* COMMAND_H */
