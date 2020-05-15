//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
//
// Example of socket server.
//
// This program is example of socket client.
// The mandatory arguments of program is IP adress or name of server and
// a port number.
//
//***************************************************************************

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/wait.h>

#include "library.h"

// debug flag
int debug = LOG_INFO;

int sock_server;

void log_msg( int log_level, const char *form, ... )
{
    const char *out_fmt[] = {
            "ERR: (%d-%s) %s\n",
            "INF: %s\n",
            "DEB: %s\n" };

    if ( log_level && log_level > debug ) return;

    char buf[ 1024 ];
    va_list arg;
    va_start( arg, form );
    vsprintf( buf, form, arg );
    va_end( arg );

    switch ( log_level )
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf( stdout, out_fmt[ log_level ], buf );
        break;

    case LOG_ERROR:
        fprintf( stderr, out_fmt[ log_level ], errno, strerror( errno ), buf );
        break;
    }
}

//***************************************************************************
// help

void help( int argn, char **arg )
{
    if ( argn <= 1 ) return;

    if ( !strcmp( arg[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Socket client example.\n"
            "\n"
            "  Use: %s [-h -d] ip_or_name port_number number_of_clients( optional )\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n", arg[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( arg[ 1 ], "-d" ) )
        debug = LOG_DEBUG;
}

//***************************************************************************

#define NUM_FD 2048 

ssize_t readLine( int fd, void *buf, size_t count )
{//-- Cteni radku do buffer

    //!!! -- debug 
    // -- log_msg( LOG_INFO, "params: %d %d", fd, count );
    //!!! 
    //!!! 

    static char* localStorage[ NUM_FD ];    //-- Momentalne pouze pro 10 file deskriptoru
    static char overLoadStorage[ 2048 ];    //-- Pomocny buffer pouzit v momente 
                                            //-- prelozeni dat ze stareho buffer do noveho
    static int alreadyVisited[ NUM_FD ];    //-- Pole vsech file deskriptoru, ktere jiz maji svuj buffer
    static int lastVisitedPlace = 0;        //-- Index posledniho file dekriptoru v poli alreadyVisited
    static int offset = 0;                  //-- Odsazeni na posledni znak v bufferu ktery se precetl 
    static int lenght = 0;                  //-- Velikost retezce(Vety), ktera bude navracena
    static int overLoad = 0;                //-- Velikost retezce(Casti vety)
                                            //-- ktera se bude premistovat ze stareho buffer do noveho
    int counterNewLine = 0;                 //-- Pocitac znaku(Delky) retezce(Vety), ktera se vrati
    int isIn = 0;                           //-- Pomocna promenna, ktera uchovava informaci 
                                            //-- zdali dany file deskripor jiz je v poli alreadyVisited


    for( int i = 0; i < lastVisitedPlace; i ++ )
    {// -- Zjisteni zdali dany file deskripor jiz je v poli alreadyVisited 

        if( fd == alreadyVisited[ i ] )
        {
        
            isIn = 1;
            break;

        }
        
    }

    if( !isIn )
    {// -- Pokud dany file deskripor neni v poli alreadyVisited
    // -- Bude vlozen do pole alreadyVisited a bude mu vytvoren buffer 

        alreadyVisited[ lastVisitedPlace ] = fd;
        localStorage[ fd ] = new char[ 2048 ] ;

    }

    if( lenght - offset == 0 || overLoad )
    {// -- Pokud v bufferu uz neni co cist, nebo overLoad obsahuje delku stareho buffer(>0)
    // -- Pak se resetuje offset a prectou se dalsi data

        offset = 0;
        lenght = read( fd, localStorage[ fd ], 2048 );

        //!!! -- debug 
        // -- log_msg( LOG_INFO, "lenght: %d ", lenght );
        // -- log_msg( LOG_INFO, "entry data: :%s:?? ", localStorage[ fd ] );
        //!!! 
        //!!! 

    }
    
    for( int i = offset; i < lenght; i ++ )
    {// -- Cteni zaznamu v bufferu od offsetu do lenght

        //!!! -- debug 
        // -- log_msg( LOG_INFO, "char: %c ", localStorage[ fd ][ i ] );
        //!!! 
        //!!! 

        if( localStorage[ fd ][ i ] == '\n' )
        {// -- Pokud se narazi na \n pak se nastavi delka vety a ukonci se cyklus

            counterNewLine = i - offset + 1;
            break;

        }

        if( i + 1 == lenght )
        {// -- Pokud se nenarazilo na \n (konec vety), pak se data museji ulozit
        // -- do pomocneho bufferu a nastavi se delka daneho retezce do promenne
        // -- overLoad. Pote se opet zavola funkce readLine aby se dokoncilo cteni vety

            memcpy( overLoadStorage, localStorage[ fd ] + offset, lenght - offset );
            overLoad = lenght - offset;
            return readLine( fd, buf, count );

        }

    }

    if( overLoad )
    {// -- Pokud overLoad obsahuje delku retezce (>0)
    // -- pak se musi data ze stareho bufferu spojit s novym buffer
    // -- a vytvorit tim vetu, nasledne prenastavit offset, counterNewLine
    // -- a overload a vratit celkovou delku nove vety

        strncat( overLoadStorage, localStorage[ fd ], overLoad );
        memcpy( buf, overLoadStorage, counterNewLine + overLoad );

        offset += counterNewLine;  
        counterNewLine += overLoad;
        overLoad = 0;
        return counterNewLine;

    }else
    {// -- Vlozeni vety do buf delky od offsetu do counterNewLine

        memcpy( buf, localStorage[ fd ] + offset, counterNewLine );
        //!!! -- debug 
        // -- log_msg( LOG_INFO, "[%d]", fd );
        // -- log_msg( LOG_INFO, "%d mess: %s\n", fd, buf );
        // -- log_msg( LOG_INFO, "offset: %d ", offset );
        // -- log_msg( LOG_INFO, "lenLine: %d ", counterNewLine );
        //!!!
        //!!!

    }

    if( counterNewLine > count )
    {// -- Pokud dana veta je vetsi nez je velikost bufferu
    // -- vrati se chybova hodnota

        //!!! -- debug 
        // -- log_msg( LOG_INFO, "lenLine: %d ", counterNewLine );
        // -- log_msg( LOG_INFO, "count: %d ", count );
        //!!!
        //!!!
        return -1;

    }

    offset += counterNewLine;  

    //!!! -- debug 
    // -- log_msg( LOG_INFO, "lenLine: %d ", counterNewLine );
    //!!!
    //!!!

    return counterNewLine;

}


// catch signal
void catch_sig( int sig )
{
    dprintf( sock_server, "W%d:%s\n", CI_VYNUCENY_KONEC_BOTH, CS_VYNUCENY_KONEC_BOTH );
    close( sock_server );
    printf( "process[%d] forced to exit\n", getpid() );
    exit( 1 );
}


//***************************************************************************

int main( int argn, char **arg )
{

    if ( argn <= 2 ) help( argn, arg );

    int port = 0,
        num_client = 2,
        first_Socket = -1;

    char *host = NULL;
    struct sigaction sa;

    srand( time( NULL ) );

    // parsing arguments
    for ( int i = 1; i < argn; i++ )
    {
        if ( !strcmp( arg[ i ], "-d" ) )
            debug = LOG_DEBUG;

        if ( !strcmp( arg[ i ], "-h" ) )
            help( argn, arg );

        if ( *arg[ i ] != '-' )
        {
            if ( !host )
                host = arg[ i ];
            else if ( !port )
                port = atoi( arg[ i ] );
            else
            {
                num_client = atoi( arg[ i ] );
            }
            
        }
        

    }

    if ( !host || !port )
    {
        log_msg( LOG_INFO, "Host or port is missing!" );
        help( argn, arg );
        exit( 1 );
    }

    log_msg( LOG_INFO, "Connection to '%s':%d.", host, port );

        // -- Some amazing stuffs going here to catch signals from our Keyboard
    bzero( &sa, sizeof( sa ) );
    sa.sa_handler = catch_sig;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;

    // -- catch sig <CTRL-C>
    sigaction( SIGINT, &sa, NULL );

    // -- catch SIG_PIPE
    sigaction( SIGPIPE, &sa, NULL );


    addrinfo ai_req, *ai_ans;
    bzero( &ai_req, sizeof( ai_req ) );
    ai_req.ai_family = AF_INET;
    ai_req.ai_socktype = SOCK_STREAM;

    int get_ai = getaddrinfo( host, NULL, &ai_req, &ai_ans );
    if ( get_ai )
    {
    log_msg( LOG_ERROR, "Unknown host name!" );
    exit( 1 );
    }

    sockaddr_in cl_addr =  *( sockaddr_in * ) ai_ans->ai_addr;
    cl_addr.sin_port = htons( port );
    freeaddrinfo( ai_ans );  

    // go!

       for( int i = 0; i < num_client; i ++ )
       {

            sock_server = socket( AF_INET, SOCK_STREAM, 0 );
            if ( sock_server == -1 )
            {
                log_msg( LOG_ERROR, "Unable to create socket.");
                exit( 1 );
            }

            // connect to server
            if ( connect( sock_server, ( sockaddr * ) &cl_addr, sizeof( cl_addr ) ) < 0 )
            {
                log_msg( LOG_ERROR, "Unable to connect server." );
                exit( 1 );
            }

            uint lsa = sizeof( cl_addr );
            // my IP
            getsockname( sock_server, ( sockaddr * ) &cl_addr, &lsa );
            log_msg( LOG_DEBUG, "My IP: '%s'  port: %d",
                    inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
            // server IP
            getpeername( sock_server, ( sockaddr * ) &cl_addr, &lsa );
            log_msg( LOG_DEBUG, "Server IP: '%s'  port: %d",
            inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );

            if( fork() == 0 )
            {

                int phase = 0;

                bool is_reader = i % 2;               

                while( true )
                {

                    char buf[ 256 ] = { 0 };


                    std::string request;

                    switch ( phase )
                    {
                    case 0:
                    {

                        request = !is_reader? "C20:Chci cist\n" : "C21:Chci psat\n" ;
                        break;

                    }
                    case 1 ... 3:
                    {

                        request = !is_reader? "C22:Chci cist od 2 do 3\n" : "C23:Zapis DATA na 2 \n" ;
                        break;
                        
                    }
                    default:
                    {
                        
                        request = "C24:Konec\n";
                        break;
                    
                    }
                        
                    }

                    dprintf( sock_server, "%s", request.c_str() );
                    log_msg( LOG_INFO, "Pozadavek: [%d]: %s", sock_server, request.c_str() );

                    
                    // set of handles
                    fd_set read_wait_set;
                    // clean set
                    FD_ZERO( &read_wait_set );
                    // add stdin
                    FD_SET( STDIN_FILENO, &read_wait_set );
                    // add socket
                    FD_SET( sock_server, &read_wait_set );

                    // select from handles
                    if ( select( sock_server + 1, &read_wait_set, 0, 0, 0 ) < 0 ) break;

                    // data on stdin?
                    if ( FD_ISSET( STDIN_FILENO, &read_wait_set ) )
                    {

                        //  read from stdin
                        int l = readLine( STDIN_FILENO, buf, sizeof( buf ) );
                        if ( l < 0 )
                            log_msg( LOG_ERROR, "Unable to read from stdin." );
                        else
                            log_msg( LOG_DEBUG, "Read %d bytes from stdin.", l );


                        // send data to server
                        l = write( sock_server, buf, l );
                        if ( l < 0 )
                            log_msg( LOG_ERROR, "Unable to send data to server." );
                        else
                            log_msg( LOG_DEBUG, "Sent %d bytes to server.", l );

                    }

                    // data from server?
                    if ( FD_ISSET( sock_server, &read_wait_set ) )
                    {
                        // read data from server
                        int l = readLine( sock_server, buf, sizeof( buf ) );
                        if ( !l )
                        {
                            log_msg( LOG_DEBUG, "Server closed socket." );
                            break;
                        }
                        else if ( l < 0 )
                            log_msg( LOG_DEBUG, "Unable to read data from server." );
                        else{
                            log_msg( LOG_DEBUG, "Read %d bytes from server.", l );
                        }

                        // display on stdout
                        log_msg( LOG_INFO, "Odpoved: [%d]: %s", sock_server, buf );
                        
                        if( std::string( buf ).find( "Naschledanou" ) != std::string::npos )
                        {

                            close( sock_server );
                            exit( 1 );

                        }

                        phase ++;

                    }

                }// -- Fork while    

                exit( 1 );

            } // -- Fork

        }// -- For which creates Forks

        while( true )
            sleep( 2 );

    return 0;
}