/*
 * prom - A command-line utility to interface with the poor's man
 *        National/TI Bipolar PROM Programmer
 *  
 * Serial port support
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
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include "globals.h"

static void serial_config( int fd )
{
    struct termios cfg;

    cfmakeraw( &cfg );
    cfg.c_cflag |= (CLOCAL | CREAD);
    cfg.c_iflag &= ~(IXOFF | IXANY);

    cfg.c_cc[VMIN] = 0;
    cfg.c_cc[VTIME] = 0;

    cfsetispeed( &cfg, B57600 );
    cfsetospeed( &cfg, B57600 );

    tcsetattr( fd, TCSANOW, &cfg );
}

status_t serial_init( int *fd, char *device )
{
    *fd = open( device, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK );
    
    if ( *fd < 0 )
    {
        fprintf( stderr, "Error %d opening %s: %s", errno, device, strerror( errno ) ) ;
        return FAILURE;
    }

    serial_config( *fd );

    return SUCCESS;
}


status_t serial_read( int fd, char *device, uint8_t *buffer, size_t bufsiz, ssize_t *returned )
{
    struct pollfd pfds;
    int ret;

    pfds.fd     = fd;
    pfds.events = POLLIN;

    if ( -1 == ( ret = poll( &pfds, 1, 2000 ) ) )
    {
        fprintf( stderr, "Error %d while polling port %s: %s\n", errno, device, strerror( errno ) );
        return FAILURE;
    }

    if ( ret > 0 )
    {
        //printf( "revents: %d\n", pfds.revents );
        if ( pfds.revents & POLLIN )
        {
            *returned = read( pfds.fd, buffer, bufsiz );

            if ( *returned < 0 )
            {
                fprintf( stderr, "Error %d reading from port %s: %s\n", errno, device, strerror( errno ) );
                return FAILURE;
            }

            buffer[*returned] = 0;
        }
        else
        {
            fprintf( stderr, "Error: unexpected event(s): %s%sreceived when polling the %s port\n",
                    ( pfds.revents & POLLHUP ) ? "POLLHUP " : "",
                    ( pfds.revents & POLLERR ) ? "POLLERR " : "",
                    device );
            return FAILURE;
        }
    }
    else
    {
        *returned = 0;
        *buffer = 0;
    }

    return SUCCESS;    
}

status_t serial_write( int fd, char *device, uint8_t *buffer, size_t len )
{
    if ( -1 == write( fd, buffer, len ) )
    {
        fprintf( stderr, "Error %d writing port %s: %s\n", errno, device, strerror( errno ) );
        return FAILURE;       
    }

    return SUCCESS;
}

void serial_close( int fd )
{
    close( fd );
}
