#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

static bool running = true;

// signal handler
void stop(const int s)
{
    running = false;
}

// very poor routine to find eor in buf
bool find( const char *buf, const int len, const char *eor, const int eorlen )
{
    for( unsigned int i = 0; i <= len-eorlen; i++ )
        if( buf[i] == eor[0] && memcmp( buf+i+1, eor+1, eorlen-1 ) == 0 ) return true;
    return false;
}

// call as ttycat -d <device> -s <speed> -e <end-of-record-string> -c <command> -f
int main( int argc, char** argv )
{
    const char *device = "/dev/cu.wchusbserial1420", *cmd = NULL, *eor = NULL;
    unsigned int speed = B38400;
    bool flush = false;
    int ret;

    // set up signal handler
    signal( SIGINT, stop );
    siginterrupt( SIGINT, 1 );

    // read command line options
    while( (ret = getopt( argc, argv, "s:fd:c:e:" )) != -1 )
        switch( ret )
        {
            case 'd':
                device = optarg;
                break;
            case 's':
                speed = strtol( optarg, NULL, 10 );
                break;
            case 'f':
                flush = true;
                break;
            case 'c':
                cmd = optarg;
                break;
            case 'e':
                eor = optarg;
                break;
            default:
                printf( "Unknown option: %c\n", ret );
                return 255;
        }

    const unsigned int eorlen = eor ? strlen( eor ) : 0;
    char buf[eorlen+2048], *p = buf;

    // open tty
    int fd = open( device, O_RDWR | O_NONBLOCK );
    if( fd == -1 )
    {
        perror( "Unable to open device" );
        return 255;
    }

    // set up serial port
    struct termios tio = {0};
    //ret = tcgetattr( fd, &tio );
    cfmakeraw( &tio );
    tio.c_cflag &= ~HUPCL;   // disable hang-up-on-close to avoid reset on disconnect
    cfsetspeed( &tio, speed );
    ret = tcsetattr( fd, TCSANOW, &tio );
    if( ret == -1 )
    {
        perror( "Unable to set line speed" );
        close( fd );
        return 255;
    }

    // prepare fd and flush receive buffer
    fcntl( fd, F_NOCACHE, 1 );
    fcntl( fd, F_FULLFSYNC, 1 );
    while( flush && read( fd, buf, sizeof(buf) ) > 0 );
    fcntl( fd, F_SETFL, 0 );    // disable non-blocking I/O

    // read for all eternity
    while( running )
    {
        const ssize_t len = read( fd, p, sizeof(buf)-(p-buf) );
        if( len > 0 )
        {
            // send command and ignore on first read (which should be a prompt)
            if( cmd )
            {
                write( fd, cmd, strlen( cmd ) );
                cmd = NULL;
            }
            else
            {
                write( STDOUT_FILENO, p, len );
                if( eorlen > 0 )
                {
                    p += len;
                    if( p-buf >= eorlen )
                    {
                        // have we read enough characters: check for EOR
                        running = !find( buf, p-buf, eor, eorlen );
                        memmove( buf, p-eorlen+1, eorlen-1 );
                        p = buf+eorlen-1;
                    }
                }
            }
        }
    }

    // clean up
    close( fd );
    return 0;
}
