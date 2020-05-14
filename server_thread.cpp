#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sstream>
#include <vector>

#include "library.h"


// debug flag
int debug = LOG_INFO;
int counter_writers = 0;

// -- Semaphores
sem_t *sem_accessLibrary;
sem_t *sem_writers;
sem_t *sem_readers;

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

void help( int num, char **arg )
{
    if ( num <= 1 ) return;

    if ( !strcmp( arg[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Socket server example.\n"
            "\n"
            "  Use: %s [-h -d] port_number\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n", arg[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( arg[ 1 ], "-d" ) )
        debug = LOG_DEBUG;
}

// catch signal
void catch_sig( int sig )
{

    printf( "process[%d] forced to exit\n", getpid() );
    exit( 1 );

}


//***************************************************************************


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
        // -- log_msg( LOG_INFO, "entry data: %s ", localStorage[ fd ] );
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
        // -- log_msg( LOG_INFO, "fd: [%d]", fd );
        // -- log_msg( LOG_INFO, "offset: [%d]", offset );
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

// **** Thread struct parameters

struct threadParams
{

    int socket_client;

};



// -- Thread work
void *comunicationThread( void *vargp )
{

    threadParams* paramsFromMain = ( threadParams* )vargp;
    int socket_client = paramsFromMain->socket_client;

    bool    is_Reader = false,
            is_Writer = false;

    log_msg( LOG_INFO, " inside socket_client [%d]", socket_client );

    while ( 1 )
    { // communication between client and server
        
        char buf[ 256 ] = { 0 };
        // set for handles
        fd_set read_wait_set;
        // empty set
        FD_ZERO( &read_wait_set );
        // add stdin
        FD_SET( STDIN_FILENO, &read_wait_set );
        // add client
        FD_SET( socket_client, &read_wait_set );

        int sel = select( socket_client + 1, &read_wait_set, NULL, NULL, NULL );

        if ( sel < 0 )
        {
            log_msg( LOG_ERROR, "Select failed!" );
            return NULL;
        }

        // -- data from client?
        if ( FD_ISSET( socket_client, &read_wait_set ) )
        {

            int x,
                z,
                lenght,
                randNum = 10,
                returnLenght = 0;

            char y,
                 controlChar = 'D',
                 expectedChar = 'D';

            bool correct_Mess;


            // --  read data from socket
            sem_wait( sem_accessLibrary );

            lenght = readLine( socket_client, buf, 256 );
            
            sem_post( sem_accessLibrary );
            
            if ( !lenght )
            {
                    log_msg( LOG_DEBUG, "Client closed socket!" );
                    close( socket_client );
                    break;
            }
            else if ( lenght < 0 )
                    log_msg( LOG_DEBUG, "Unable to read data from client." );
            else
                    log_msg( LOG_DEBUG, "Read %d bytes from client.", lenght );

  
            if( buf[1] == ':')
            {
                returnLenght = sscanf( ( const char* )buf, "%c%c", &controlChar, &expectedChar) > 0;
            }
            else
            {
                returnLenght = sscanf( ( const char* )buf, "%c%d%c", &controlChar, &randNum, &expectedChar) > 0;
            }

            // -- log_msg( LOG_INFO, "%s\n", buf );

            if( returnLenght > 0 ) 
            {

                if( ( controlChar == 'W' || controlChar == 'C' || controlChar == 'A' || controlChar == 'I' || controlChar == 'E')
                && randNum > 9 && randNum < 100 && expectedChar == ':' && lenght < 256 )
                {

                    correct_Mess = true;
                    // dprintf( socket_client, "Correct message:\t" );    
                    // log_msg( LOG_INFO, "Correct message:\t\n" );
                    
                }
                else
                {
                
                    correct_Mess = false;
                    dprintf( socket_client, "Bad message majster:\t\n" );
                
                }

            }
            else
            {

                correct_Mess = false;
                dprintf( socket_client, "You need to send something:\t\n" );
            
            }

            if( correct_Mess )
            {

                std::string input = buf;

                if( input.find( CS_VYNUCENY_KONEC_BOTH ) != std::string::npos )
                {

                    log_msg( LOG_INFO, "Client vynucene ukoncil spojeni" );
                    close( socket_client );
                    break;

                }

                if( !is_Reader && !is_Writer )
                {

                    if( input.find( CS_VSTUP_CT ) != std::string::npos )
                    {

                        is_Reader = true;

                        log_msg( LOG_INFO, "Citatel chce vstoupit: %s", buf );


                        sem_wait( sem_writers );
                            counter_writers ++;
                            if( counter_writers == 1 )
                                sem_wait( sem_readers );
                        sem_post( sem_writers );

                        //log_msg( LOG_INFO, "A%d:%s\n", CI_VYSTUP_CT, CS_VYSTUP_CT );
                        dprintf( socket_client, "A%d:%s\n", CI_VYSTUP_CT, CS_VYSTUP_CT );

                    }else
                    {
                        is_Writer = true;

                        log_msg( LOG_INFO, "Spisovatel chce vstoupit: %s", buf );
                        sem_wait( sem_readers );
                        //sem_wait( sem_writers );
                        //log_msg( LOG_INFO, "A%d:%s\n", CI_VYSTUP_CT, CS_VYSTUP_CT );
                        dprintf( socket_client, "A%d:%s\n", CI_VYSTUP_SP, CS_VYSTUP_SP );
                        
                    }

                }else
                {

                    if( input.find( CS_DOBA_CT ) != std::string::npos )
                    {

                        std::stringstream ss;
                        
                        std::string word;

                        int from,
                            to,
                            temp;

                        bool first_num = true;

                        ss << std::string( buf );

                        log_msg( LOG_DEBUG, "%s", buf );

                        while( !ss.eof() )
                        {
                            
                            ss >> word;

                            if( first_num ){
                            
                                if ( std::stringstream( word ) >> from )
                                {

                                    log_msg( LOG_DEBUG, "First number is %d", from );
                                    first_num = !first_num;

                                }
                            }
                            else
                                if ( std::stringstream( word ) >> to )
                                    log_msg( LOG_DEBUG, "Second number is %d", to );


                            word = "";

                        }

                        sleep( rand()% ( to - from ) + from );
                        dprintf( socket_client, "A%d:%s\n", CI_DATA_CT, CS_DATA_CT );
                        //sem_post( sem_writers );


                    }

                    else if( input.find( CS_DOBA_SP ) != std::string::npos )
                    {

                        std::stringstream ss;
                        
                        std::string word;

                        int how_Long;


                        ss << std::string( buf );

                        log_msg( LOG_DEBUG, "%s", buf );

                        while( !ss.eof() )
                        {
                            
                            ss >> word;


                            if ( std::stringstream( word ) >> how_Long )
                            
                                log_msg( LOG_DEBUG, "How long number is %d", how_Long );
                        
                            word = "";

                        }

                        sleep( how_Long );
                        dprintf( socket_client, "A%d:%s\n", CI_HOTOVO_CT, CS_HOTOVO_CT );
                        //sem_post( sem_writers );

                    }

                    else if( input.find( CS_KONEC_BOTH ) != std::string::npos )
                    {
                        
                        if( is_Reader )
                        {

                            log_msg( LOG_INFO, "ctenar nas opustil");
                            sem_wait( sem_writers );
                                counter_writers --;
                                if( counter_writers == 0 )
                                    sem_post( sem_readers );
                            sem_post( sem_writers );

                        }else
                        {

                            log_msg( LOG_INFO, "spisovatel nas opustil");
                            sem_post( sem_readers );
                        }

                        //log_msg( LOG_INFO, "A%d:%s\n", CI_NASCHLEDANOU_BOTH, CS_NASCHLEDANOU_BOTH );
                        log_msg( LOG_INFO, "A%d:%s\n", CI_KONEC_BOTH, CS_KONEC_BOTH );
                        dprintf( socket_client, "A%d:%s\n", CI_NASCHLEDANOU_BOTH, CS_NASCHLEDANOU_BOTH );
                        close( socket_client );
                        break;

                    }

                }

            }



            // write data to client
            
            // close request?
            /*if ( !strncasecmp( buf, "close", strlen( STR_CLOSE ) ) )
            {

                dprintf( temp, "close");
                log_msg( LOG_INFO, "Client sent 'close' request to close connection." );
                close( temp );
                log_msg( LOG_INFO, "Connection closed. Waiting for new client." );
                return NULL;

            }*/

            // dprintf( socket_client , "%s", buf);                
        }

    } // while communication

}


int main( int argn, char **arg )
{

    // -- Variables

    sem_accessLibrary = sem_open( "/accessLibrary", O_RDWR | O_CREAT, 0600, 1 ); // -- Semaphore for accessing library
    sem_writers = sem_open( "/writers", O_RDWR | O_CREAT, 0600, 1 ); // -- Semaphore for writer access
    sem_readers = sem_open( "/readers", O_RDWR | O_CREAT, 0600, 1 ); // -- Semaphore for reader access

    int sock_listen = socket( AF_INET, SOCK_STREAM, 0 ); // -- Socket creation
    int sock_client; // -- Socket listener for client
    int port = 0; // -- Port on which comunication will be working
    int opt = 1; // -- For enable the port number reusing
    int counter = -1; // -- Counter clients

    std::vector< threadParams* > storage_file_descriptors;

    std::vector< pthread_t* > storage_threads;

    in_addr addr_any = { INADDR_ANY }; // -- Server address
    sockaddr_in srv_addr; // -- Server socket address

    struct sigaction sa;

    srand( time( NULL ) );


    for ( int i = 1; i < argn; i++ )
    {// -- Parsing argument
    
        if ( !strcmp( arg[ i ], "-d" ) )
            debug = LOG_DEBUG; // -- Debug mode (more information what is going on)

        if ( !strcmp( arg[ i ], "-h" ) )
            help( argn, arg ); // -- Info about parameteres

        if ( *arg[ i ] != '-' && !port )
        {// -- Setting port on our server
    
            port = atoi( arg[ i ] ); 
            break;
    
        }
    
    }


    if ( port <= 0 )
    {// -- If bad port has been given

        log_msg( LOG_INFO, "Bad or missing port number %d!", port );
        help( argn, arg );

    }else
        log_msg( LOG_INFO, "Server will listen on port: %d.", port );

    // -- Some amazing stuffs going here to catch signals from our Keyboard
    bzero( &sa, sizeof( sa ) );
    sa.sa_handler = catch_sig;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;

    // -- catch sig <CTRL-C>
    sigaction( SIGINT, &sa, NULL );

    // -- catch SIG_PIPE
    sigaction( SIGPIPE, &sa, NULL );

    // -- Initializing semaphores, if some of these were already in function
    // -- They need to reinitialize to work in our program
    sem_init( sem_accessLibrary, 1, 1 );
    sem_init( sem_writers, 1, 1 );
    sem_init( sem_readers, 1, 1 );

    if( !sem_accessLibrary || !sem_writers )
    {// -- If semaphores are unable for some reasons ..

        printf( "Nejsou semafory.\n" );
        exit( -1 );

    }

    if ( sock_listen == -1 )
    {// -- If socket was unable to created

        log_msg( LOG_ERROR, "Unable to create socket.");
        exit( -1 );
    
    }

    // -- Initializing Server address
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons( port );
    srv_addr.sin_addr = addr_any;

    // -- Enable the port number reusing
    if ( setsockopt( sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) ) < 0 )
      log_msg( LOG_ERROR, "Unable to set socket option!" );

    if ( bind( sock_listen, (const sockaddr * ) &srv_addr, sizeof( srv_addr ) ) < 0 )
    {// -- assign port number to socket
        
        log_msg( LOG_ERROR, "Bind failed!" );
        close( sock_listen );
        exit( -1 );

    }

    if ( listen( sock_listen, 1 ) < 0 )
    {// -- listenig on set port

        log_msg( LOG_ERROR, "Unable to listen on given port!" );
        close( sock_listen );
        exit( -1 );

    }

    while ( true )
    {// -- And there.. There is where our server is working

        sock_client = counter;

        while ( true ) 
        {// -- In this section, it´s waiting for new client

            // -- set for handles
            fd_set read_wait_set;
            // -- empty set
            FD_ZERO( &read_wait_set );
            // -- add stdin
            FD_SET( STDIN_FILENO, &read_wait_set );
            // -- add listen socket
            FD_SET( sock_listen, &read_wait_set );

            int sel = select( sock_listen + 1, &read_wait_set, NULL, NULL, NULL );

            if ( sel < 0 )
            {

                log_msg( LOG_ERROR, "Select failed!" );
                exit( -1 );

            }

            if ( FD_ISSET( sock_listen, &read_wait_set ) )
            { // -- new client

                sockaddr_in rsa;
                int rsa_size = sizeof( rsa );

                // -- new connection
                sock_client = accept( sock_listen, ( sockaddr * ) &rsa, ( socklen_t * ) &rsa_size );
                if ( sock_client == -1 )
                {
                        
                        log_msg( LOG_ERROR, "Unable to accept new client." );
                        close( sock_listen );
                        exit( -1 );

                }

                storage_file_descriptors.push_back( new threadParams() );
                int last_place = storage_file_descriptors.size() - 1;
                storage_file_descriptors[ last_place ]->socket_client = sock_client;

                // -- log_msg( LOG_INFO, "[%d]", sock_client );
                // --log_msg( LOG_INFO, "sockted: [%d]", storage_file_descriptors[ last_place ].socket_client );

                uint lsa = sizeof( srv_addr );

                // -- server IP
                getsockname( sock_client, ( sockaddr * ) &srv_addr, &lsa );
                log_msg( LOG_DEBUG, "Server IP: '%s'  port: %d",
                                 inet_ntoa( srv_addr.sin_addr ), ntohs( srv_addr.sin_port ) );
                // -- client IP
                getpeername( sock_client, ( sockaddr * ) &srv_addr, &lsa );
                log_msg( LOG_DEBUG, "Client IP: '%s'  port: %d",
                                 inet_ntoa( srv_addr.sin_addr ), ntohs( srv_addr.sin_port ) );

                storage_threads.push_back( new pthread_t() );
                pthread_create( storage_threads.back(), NULL, comunicationThread, storage_file_descriptors[ last_place ] );

                usleep( 10 );// -- Bug fix .. yea I know but sometimes that little time can help prevent unwanted stuffs :)
                      
                break;
            }

            if ( FD_ISSET( STDIN_FILENO, &read_wait_set ) )
            { // -- data on stdin

                char buf[ 128 ];
                int len = readLine( STDIN_FILENO, buf, sizeof( buf) );
                if ( len < 0 )
                {
                    log_msg( LOG_DEBUG, "Unable to read from stdin!" );
                    exit( -1 );
                }

                log_msg( LOG_DEBUG, "Read %d bytes from stdin" );
                
            }

        } // while wait for client

    } // while ( true )

    return 0;
}