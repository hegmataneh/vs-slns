#include <cstdio>
#include <pthread.h> // Required for Pthreads functions
#include <stdio.h>   // Required for printf
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // For close()
#include <errno.h> // For errno
#include <stdarg.h>

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"

#define BUFFER_SIZE 9000

#define TUNNEL_COUNT 2

#define UDP_PORT_1 1234
#define UDP_PORT_2 1235

#define TCP_PORT_1 4321
#define TCP_SERVER_IP_1 "192.168.1.60"

#define TCP_PORT_2 4322
#define TCP_SERVER_IP_2 "192.168.1.60"

const char * __msg( char * msg_holder , ssize_t size_of_msg_holder , const char * msg , int line_number)
{
    snprintf(msg_holder, size_of_msg_holder, "%s: ln(%d)\n", msg , line_number);
    return msg_holder;
}

const char * __snprintf( char * msg_holder , ssize_t size_of_msg_holder , const char * format , ... )
{
    va_list args;

    va_start(args, format);
    vsnprintf(msg_holder, size_of_msg_holder, format, args);
    va_end(args);

    return msg_holder;
}

#define _MSG(s) __msg(custom_message,sizeof(custom_message),s,__LINE__)

#define _DETAIL_ERROR( user_friendly_msg ) do { perror(_MSG(user_friendly_msg)); perror( __snprintf( custom_message , sizeof(custom_message) , "more details: %s(#%d)@ln(%d)\n" , strerror(errno), errno , __LINE__ ) ); } while(0);

#define BAD_RETURN "bad"

struct udp_connection_data
{
    int udp_port_number;
    int udp_sockfd;
    int udp_connection_established; // udp socket established
};

struct tcp_connection_data
{
    int tcp_port_number;
    const char * tcp_ip;
    int tcp_sockfd;
    int tcp_connection_established; // tcp connection established
};

struct tunnel_data
{
    udp_connection_data * p_udp_data;
    tcp_connection_data * p_tcp_data;
    int counterpart_connection_establishment_count; // 2 means both side socket connected
};

struct tunnels_data
{
    tunnel_data tunnels_data[ TUNNEL_COUNT ];
    int udp_connection_count;
    int tcp_connection_count;
    int tunnel_connection_establishment_count; // each tunnel established
};

void _close_socket( int & socket_id )
{
    close(socket_id);
    socket_id = -1;
}

void* thread_udp_connection_proc(void* arg)
{
    char custom_message[256];

    tunnels_data * pTunnels_data = ( tunnels_data * )arg;

    printf(_MSG("try to connect inbound udp connection"));

    while( pTunnels_data->udp_connection_count < TUNNEL_COUNT )
    {
        if ( ( pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_sockfd = socket(AF_INET , SOCK_DGRAM , 0) ) < 0 )
        {
            _DETAIL_ERROR( "socket creation failed" );
            pthread_exit( ( void * )BAD_RETURN );
            return NULL;
        }

        struct sockaddr_in server_addr;
        memset( &server_addr , 0 , sizeof( server_addr ) ); // Clear the structure

        server_addr.sin_family = AF_INET; // IPv4
        server_addr.sin_port = htons( pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_port_number ); // Convert port to network byte order
        //server_addr.sin_addr.s_addr = inet_addr("YOUR_IP_ADDRESS"); // Specify the IP address to bind to
        // Or use INADDR_ANY to bind to all available interfaces:
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if ( bind( pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_sockfd , ( const struct sockaddr * )&server_addr , sizeof( server_addr ) ) < 0 )
        {
            _DETAIL_ERROR( "bind failed" );
            _close_socket(pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_sockfd);

            //close( pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_sockfd );
            //pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_sockfd = -1;
            pthread_exit( ( void * )BAD_RETURN );
            return NULL;
        }

        pTunnels_data->tunnels_data[pTunnels_data->udp_connection_count].p_udp_data->udp_connection_established++;
        // one side connection established
        pTunnels_data->tunnels_data[ pTunnels_data->udp_connection_count ].counterpart_connection_establishment_count++;
        printf( _MSG( "inbound udp connected" ) );
        pTunnels_data->udp_connection_count++;
    }

    return NULL; // Threads can return a value, but this example returns NULL
}

int _connect_tcp( tcp_connection_data * pTCP )
{
    char custom_message[256];
    while ( 1 )
    {
        // try to create TCP socket
        if ( ( pTCP->tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1 )
        {
            _DETAIL_ERROR("Error creating TCP socket");
            //pthread_exit((void *)BAD_RETURN);
            //return NULL;
            return -1;
        }

        struct sockaddr_in tcp_addr;

        // Connect to TCP server
        tcp_addr.sin_family = AF_INET;
        tcp_addr.sin_port = htons(pTCP->tcp_port_number);
        if (inet_pton(AF_INET, pTCP->tcp_ip, &tcp_addr.sin_addr) <= 0) {
            _DETAIL_ERROR("Invalid TCP address");
            _close_socket(pTCP->tcp_sockfd);
            //pthread_exit((void *)BAD_RETURN);
            //return NULL;
            return -1;
        }

        if ( connect( pTCP->tcp_sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
        {
            if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
            {
                _close_socket(pTCP->tcp_sockfd);
                sleep( 2 ); // sec
                continue;
            }

            _DETAIL_ERROR( "Error connecting to TCP server" );
            _close_socket(pTCP->tcp_sockfd);
            //pthread_exit( ( void * )BAD_RETURN );
            //return NULL;
            return -1;
        }
        else
        {
            pTCP->tcp_connection_established = 1;
            printf(_MSG("outbound tcp connected"));
            return 0;
        }
    }
    return -1;
}

void * thread_tcp_connection_proc( void * arg )
{
    char custom_message[ 256 ];

    tunnels_data * pTunnels_data = ( tunnels_data * )arg;

    printf(_MSG("try to connect outbound tcp connection"));

    while ( pTunnels_data->tcp_connection_count < TUNNEL_COUNT )
    {
        if ( _connect_tcp( pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].p_tcp_data ) == 0 )
        {
            if
            (
                pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].p_udp_data->udp_connection_established ||
                pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].p_tcp_data->tcp_connection_established
            )
            {
                // both side connection established
                pTunnels_data->tunnels_data[ pTunnels_data->tcp_connection_count ].counterpart_connection_establishment_count++;
                pTunnels_data->tunnel_connection_establishment_count++;
            }
            pTunnels_data->tcp_connection_count++;
            
            continue;
        }
        //pthread_exit((void *)BAD_RETURN);
        //return NULL;
    }

    //while ( 1 )
    //{

    //    // try to create TCP socket
    //    ptcp_connection->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //    if (ptcp_connection->sockfd == -1) {
    //        _DETAIL_ERROR("Error creating TCP socket");
    //        pthread_exit((void *)BAD_RETURN);
    //        return NULL;
    //    }

    //    struct sockaddr_in tcp_addr;

    //    // Connect to TCP server
    //    tcp_addr.sin_family = AF_INET;
    //    tcp_addr.sin_port = htons(TCP1_PORT);
    //    if (inet_pton(AF_INET, TCP1_SERVER_IP, &tcp_addr.sin_addr) <= 0) {
    //        _DETAIL_ERROR("Invalid TCP address");
    //        close(ptcp_connection->sockfd);
    //        ptcp_connection->sockfd = -1;
    //        pthread_exit((void *)BAD_RETURN);
    //        return NULL;
    //    }

    //    if ( connect( ptcp_connection->sockfd , ( struct sockaddr * )&tcp_addr , sizeof( tcp_addr ) ) == -1 )
    //    {
    //        if ( errno == ECONNREFUSED || errno == ETIMEDOUT )
    //        {
    //            close( ptcp_connection->sockfd );
    //            ptcp_connection->sockfd = -1;
    //            sleep( 2 ); // sec
    //            continue;
    //        }

    //        _DETAIL_ERROR( "Error connecting to TCP server" );
    //        close( ptcp_connection->sockfd );
    //        ptcp_connection->sockfd = -1;
    //        pthread_exit( ( void * )BAD_RETURN );
    //        return NULL;
    //    }
    //    else
    //    {
    //        ptcp_connection->connection_stablished = 1;
    //        printf(_MSG("outbound tcp connected"));
    //        break;
    //    }
    //}

    return NULL; // Threads can return a value, but this example returns NULL
}

void * thread_tunnel_proc( void * arg )
{
    char custom_message[ 256 ];

    tunnels_data * pTunnels_data = ( tunnels_data * )arg;

    while ( !pTunnels_data->tunnel_connection_establishment_count )
    {
        sleep(1);
    }

    //printf( "udp to tcp tunnel stablished\n" );

    char buffer[BUFFER_SIZE]; // Define a buffer to store received data
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    ssize_t bytes_received;

    fd_set readfds; // Set of socket descriptors

    while ( 1 )
    {
        // Clear the socket set
        FD_ZERO(&readfds);

        int sockfd_max = -1; // for select compulsion
        for ( int i = 0 ; i < pTunnels_data->tunnel_connection_establishment_count ; i++ )
        {
            if ( pTunnels_data->tunnels_data[ i ].counterpart_connection_establishment_count )
            {
                // Add socket to set
                FD_SET( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd , &readfds );
                if ( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd > sockfd_max )
                {
                    sockfd_max = pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd;
                }
            }
        }

        if ( sockfd_max <= 1 )
        {
            sleep( 1 );
            continue;
        }

        struct timeval timeout;
        // Set timeout (e.g., 5 seconds)
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
        int activity = select(sockfd_max+1, &readfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            _DETAIL_ERROR("select error");
            continue;
        }
        if ( activity == 0 )
        {
            //_DETAIL_ERROR("timed out");
            continue;
        }

        for ( int i = 0 ; i < pTunnels_data->tunnel_connection_establishment_count ; i++ )
        {
            if ( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_connection_established )
            {
                if ( FD_ISSET( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd , &readfds ) )
                {
                    // good for udp data recieve
                    bytes_received = recvfrom( pTunnels_data->tunnels_data[ i ].p_udp_data->udp_sockfd , buffer , BUFFER_SIZE , MSG_WAITALL ,
                                  ( struct sockaddr * )&client_addr , &client_len );
                    if ( bytes_received < 0 )
                    {
                        _DETAIL_ERROR("Error receiving UDP packet");
                        continue;
                    }
                    buffer[ bytes_received ] = '\0'; // Null-terminate the received data

                    // Send data over TCP
                    if (send(pTunnels_data->tunnels_data[ i ].p_tcp_data->tcp_sockfd, buffer, bytes_received, 0) == -1) {
                        _DETAIL_ERROR("Error sending data over TCP");
                        continue;
                    }

                    printf( "Received from client: %s\n" , buffer );
                    printf( "Forwarded %zd bytes from UDP to TCP.\n", bytes_received);
                }
            }
            else
            {
                printf( "\n" );
            }
        }
    }
}

int main()
{
    char custom_message[256];
    pthread_t thread_udp_connection , thread_tcp_connection;
    pthread_t thread_tunnel;

    udp_connection_data udp_connection_datas[TUNNEL_COUNT];
    memset( udp_connection_datas , 0 , sizeof(udp_connection_datas) );
    udp_connection_datas[ 0 ].udp_port_number = UDP_PORT_1;
    udp_connection_datas[ 1 ].udp_port_number = UDP_PORT_2;

    tcp_connection_data tcp_connection_datas[TUNNEL_COUNT];
    memset( tcp_connection_datas , 0 , sizeof(tcp_connection_datas) );
    tcp_connection_datas[ 0 ].tcp_ip = TCP_SERVER_IP_1;
    tcp_connection_datas[ 0 ].tcp_port_number = TCP_PORT_1;
    tcp_connection_datas[ 1 ].tcp_ip = TCP_SERVER_IP_2;
    tcp_connection_datas[ 1 ].tcp_port_number = TCP_PORT_2;

    tunnels_data tunnels;
    memset( &tunnels , 0 , sizeof(tunnels) );
    tunnels.tunnels_data[0].p_udp_data = &udp_connection_datas[0];
    tunnels.tunnels_data[0].p_tcp_data = &tcp_connection_datas[0];

    tunnels.tunnels_data[1].p_udp_data = &udp_connection_datas[1];
    tunnels.tunnels_data[1].p_tcp_data = &tcp_connection_datas[1];

    if (pthread_create(&thread_udp_connection, NULL, thread_udp_connection_proc, (void *)&tunnels) != 0) {
        _DETAIL_ERROR("Failed to create thread");
        return 1; // Indicate an error
    }

    if (pthread_create(&thread_tcp_connection, NULL, thread_tcp_connection_proc, (void *)&tunnels) != 0) {
        _DETAIL_ERROR("Failed to create thread");
        return 1; // Indicate an error
    }

    if (pthread_create(&thread_tunnel, NULL, thread_tunnel_proc, (void *)&tunnels) != 0) {
        _DETAIL_ERROR("Failed to create thread");
        return 1; // Indicate an error
    }

    if (pthread_join(thread_tunnel, NULL) != 0) {
        _DETAIL_ERROR("Failed to join thread");
        return 1; // Indicate an error
    }

    //close(udp1_connection_data.sockfd);
    //close(tcp1_connection_data.sockfd);

    printf(_MSG("Main thread finished waiting for the new thread."));

    return 0;
}

