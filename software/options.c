/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Options and argument parsing 
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
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

#include "globals.h"
#include "options.h"
#include "binfile.h"
#include "ihex.h"
#include "command.h"
#include "files.h"

static const format_st_t formats[] = {
    { "bin",  BIN,  bin_read,  bin_write },
    { "ihex", IHEX, ihex_read, ihex_write },
    { NULL }
};

static const command_t commands[] = {
    { 'k', "blank test", command_blank }, 
    { 'r', "read",       command_read }, 
    { 'w', "write",      command_write }, 
    { 's', "simulate",   command_simul }, 
    { 'v', "verify",     command_verify },
    { 0 }
};

static int get_uint16( char *optarg, uint16_t *value )
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

static int get_uint8( char *optarg, uint8_t *value )
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

static const command_t *get_command( char command )
{
    for ( int i=0; commands[i].command; ++i )
    {
        if ( commands[i].command == command )
        {
            return &commands[i];
        }
    }

    return NULL;
}

static status_t usage( char *myname, status_t status )
{
    fprintf( stderr, "\nUsage: %s [-h]", myname );
    fprintf( stderr, "\n       %s DEVICE [-c NUM] -b", myname );
    fprintf( stderr, "\n       %s DEVICE [-c NUM] -r [ADDRESS | -o FILE [-f FORMAT]]", myname );
    fprintf( stderr, "\n       %s DEVICE [-c NUM] {-w|-s|-v} {ADDRESS -d BYTE | -i FILE [-f FORMAT]}\n\n", myname );

    fputs( "Arguments:\n", stderr );
    fputs( "    DEVICE                  Serial device.\n\n", stderr );

    fputs( "Options:\n", stderr );
    fputs( "    -h[elp]                 Show this help message and exit.\n", stderr );
    fputs( "    -c[hip]     NUM         Chip to program: 0 == 74s471 (default), 1 == 74s472.\n", stderr );
    fputs( "    -b[lank]                Do a whole chip blank test.\n", stderr );
    fputs( "    -r[ead]     [ADDRESS]   Read chip. If ADDRESS is specified, read just that\n", stderr );
    fputs( "                            byte. If not, do a hexdump of the chip contents to\n", stderr );
    fputs( "                            the screen or, if an output file name is provided,\n", stderr );
    fputs( "                            dump the contents with the format specified by the\n", stderr );
    fputs( "                            -format option.\n", stderr );
    fputs( "    -w[rite]    [ADDRESS]   Program chip. If ADDRESS is provided, program just\n", stderr );
    fputs( "                            that location with data specified with the -data\n", stderr );
    fputs( "                            option. If not, an input filename must be specified.\n", stderr );
    fputs( "    -s[imulate] [ADDRESS]   Program simulation. Will success of fail as the write\n", stderr );
    fputs( "                            command, but without actually burning the chip.\n", stderr );
    fputs( "    -v[erify]   [ADDRESS]   Verify data. Same options as for -write|-simulate.\n", stderr );
    fputs( "    -d[ata]     BYTE        Byte to program, simulate or verify.\n", stderr );
    fputs( "    -i[nput]    FILE        File to read the data from.\n", stderr );
    fputs( "    -o[utput]   FILE        File to save the data to.\n", stderr );
    fputs( "    -f[ormat]   {bin,ihex}  File format. Defaults to bin.\n\n", stderr );

    fputs( "Note: Long and short options, with single or dual '-' are supported\n\n", stderr );

    return status;
}

static status_t duplicate( char *myname, char opt )
{
    fprintf( stderr, "Duplicate or incompatible option: -%c\n", opt );
    return usage( myname, FAILURE );
}

status_t get_options( options_t *options, int argc, char **argv )
{
    int opt, opt_index = 0;
    char *myname = basename( argv[0] );

    static const struct option long_opts[] = {
        {"help",     no_argument,       0, 'h' },
        {"chip",     required_argument, 0, 'c' },
        {"blank",    no_argument,       0, 'k' },
        {"read",     optional_argument, 0, 'r' },
        {"write",    optional_argument, 0, 'w' },
        {"simulate", optional_argument, 0, 's' },
        {"verify",   optional_argument, 0, 'v' },
        {"data",     required_argument, 0, 'd' },
        {"input",    required_argument, 0, 'i' },
        {"output",   required_argument, 0, 'o' },
        {"format",   required_argument, 0, 'f' },
        {0,          0,                 0,  0  }
    };

    memset( options, 0, sizeof( options_t ) );

    if ( argc < 2 )
    {
        fputs( "Invalid number of arguments\n", stderr );
        return usage( myname, FAILURE );
    }

    if ( *argv[1] != '-' )
    {
        options->device = argv[1];
        --argc, ++argv;
    }

    while (( opt = getopt_long_only( argc, argv, ":", long_opts, &opt_index)) != -1 )
    {
        int f_index = 0;

        switch( opt )
        {
            case 'c':
                if ( options->flags.chip++ )
                {
                    return duplicate( myname, opt );
                }

                if ( EINVAL == get_uint8( optarg, &options->chip ) || options->chip > MAX_CHIP )
                {
                    fprintf( stderr, "Error: Invalid chip number: %s\n", optarg );
                    return usage( myname, FAILURE );
                } 

                break;
            
            case 'k':
                if ( options->command )
                {
                    return duplicate( myname, opt );
                }
                options->command = get_command( opt );
                assert( options->command );

                break;

            case 'r':
            case 'w':
            case 's':
            case 'v':
                if ( options->command )
                {
                    return duplicate( myname, opt );
                }
                options->command = get_command( opt );
                assert( options->command );

                // Trick to allow optional arguments without '=' by
                // https://stackoverflow.com/users/549246/brian-vandenberg
                //
                if ( !optarg && NULL != argv[optind] && '-' != argv[optind][0] )
                {
                    char *alt_optarg = argv[optind++];

                    if ( EINVAL == get_uint16( alt_optarg, &options->address ) || options->address > 0x1ff )
                    {
                        fprintf( stderr, "Error: Invalid memory address: %s\n", alt_optarg );
                        return usage( myname, FAILURE );
                    }
                    options->flags.address++;
                }
                
                break;

            case 'd':
                if ( options->flags.value++ )
                {
                    return duplicate( myname, opt );
                }

                if ( EINVAL == get_uint8( optarg, &options->value ) )
                {
                    fprintf( stderr, "Error: Invalid data value: %s\n", optarg );
                    return usage( myname, FAILURE );
                } 

                break;
            
            case 'i':
                if ( options->ifile )
                {
                    return duplicate( myname, opt );

                }
                options->ifile = optarg;
                break;
                        
            case 'o':
                if ( options->ofile )
                {
                    return duplicate( myname, opt );
                }
                options->ofile = optarg;
                break;

            case 'f':
                if ( options->format )
                {
                    return duplicate( myname, opt );
                }
                
                while ( NULL != formats[f_index].format_string )
                {
                    if ( ! strcmp( formats[f_index].format_string, optarg ) )
                    {
                        options->format = &formats[f_index];
                        break;
                    }
                    ++f_index;
                }
                if ( options->format == NULL )
                {
                    fprintf( stderr, "Invalid format: %s\n", optarg );
                    return usage( myname, FAILURE );
                }
                break;

            case ':':
                fprintf( stderr, "%s: Option requires an argument: -- '%s'\n", myname, argv[optind-1] );
                // FALLTHROUGH

            case 'h':
                return usage( myname, FAILURE );

            default:
                fprintf( stderr, "%s: Invalid option: -- '%s'\n", myname, argv[optind-1] );
                return usage( myname, FAILURE );
        }
    }

    if ( argv[optind] )
    {
        fprintf( stderr, "%s: Unexpected argument: '%s'\n", myname, argv[optind] );
        return usage( myname, FAILURE );
    }

    if ( options->device == NULL )
    {
        fprintf( stderr, "%s: Missing device name\n", myname );
        return usage( myname, FAILURE );
    }

    if ( ! options->command )
    {
        fprintf( stderr, "%s: At least one command ('-k', '-r', '-w', '-s' or '-v') must be specified.\n", myname );
        return usage( myname, FAILURE );
    }

    if ( options->flags.address && ! options->flags.value && ! (options->command->command == 'r') )
    {
        fprintf( stderr, "%s: Missing mandatory option '-d'\n", myname );
        return usage( myname, FAILURE );
    }

    if ( (options->command->command == 'w' || options->command->command == 's' || options->command->command == 'v')
            && ! options->flags.address && ! options->ifile )
    {
        fprintf( stderr, "%s: Either '-d' or '-i' is mandatory for the '%s' command.\n", myname, options->command->name );
        return usage( myname, FAILURE );
    } 

    if ( (options->command->command == 'r' && options->ifile )
        || (options->command->command != 'r' && options->ofile ) )
    {
        fprintf( stderr, "%s: Incompatible options: '-%c' and '-%c'.\n", myname,
                options->command->command,
                (options->command->command == 'r') ? 'i' : 'o' );
        return usage( myname, FAILURE );
    }

    if ( options->command->command == 'k'
        && (options->ifile || options->ofile || options->flags.value || options->format ) )
    {
        fprintf( stderr, "%s: Command '-b' does not accept any other option.\n", myname );
        return usage( myname, FAILURE );
    }

    if ( options->flags.address && ( options->ifile || options->ofile ) )
    {
        fprintf( stderr, "%s: Mutually exclusive options: '-d' and '-%c'.\n", myname, options->ifile ? 'i' : 'o' );
        return usage( myname, FAILURE );
    }

    if ( options->format && !( options->ifile || options->ofile) )
    {
        fprintf( stderr, "%s: Option '-f' only valid with '-i' or '-o'.\n", myname );
        return usage( myname, FAILURE );
    }

    if ( ! options->format )
    {
        options->format = &formats[0];
    }

    return SUCCESS;
}

