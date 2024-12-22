/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * File support definitions
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

#ifndef FILES_H
#define FILES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "globals.h"

typedef struct mem_block_s {
    uint16_t start;
    uint16_t count;
    struct mem_block_s *next;
} mem_block_t;

typedef status_t (*write_fn_t)( char *filename, const uint8_t *buffer, size_t buffer_size, uint64_t base_addr );
typedef status_t (*read_fn_t)( char *filename, const uint8_t *buffer, size_t buffer_size, mem_block_t **blocks );

typedef enum { BIN = 0, IHEX = 1 } format_t;

typedef struct {
    const char *format_string;
    format_t format;
    read_fn_t read_fn;
    write_fn_t write_fn;
} format_st_t;

void files_free_blocks( mem_block_t *blocks );
status_t files_cleanup( FILE *file, mem_block_t *blocks, status_t status );

#endif /* FILES_H */