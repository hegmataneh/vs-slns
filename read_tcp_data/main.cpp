#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h> // For errno
#include <stdarg.h>

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"

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

#define _DETAIL_ERROR( user_friendly_msg ) do { perror(_MSG(user_friendly_msg)); perror( __snprintf( custom_message , sizeof(custom_message) , "more details: %s(#%d)@%d\n" , strerror(errno), errno , __LINE__ ) ); } while(0);

#define INCOME_CONNECTION_COUNT 2

#define TCP_PORT_1 4321
#define TCP_SERVER_IP_1 "192.168.1.60"

#define TCP_PORT_2 4322
#define TCP_SERVER_IP_2 "192.168.1.60"

#define BUFFER_SIZE 9000

struct tcp_connection_data
{
    int tcp_port_number;
    const char * tcp_ip;
    int tcp_server_sock;
    int tcp_client_sock;
    int tcp_connection_established; // tcp connection established
};

struct income_data
{
    tcp_connection_data tunnels_data[ INCOME_CONNECTION_COUNT ];
    int tcp_connection_count;
};

void _close_socket( int & socket_id )
{
    close(socket_id);
    socket_id = -1;
}

void * thread_tcp_connection_proc( void * arg )
{
    char custom_message[256];

    income_data * pIncomeData = ( income_data * )arg;

    printf(_MSG("try to connect inbound tcp connection"));

    while( pIncomeData->tcp_connection_count < INCOME_CONNECTION_COUNT )
    {
        for ( int iIncome = 0 ; iIncome < INCOME_CONNECTION_COUNT ; iIncome++ )
        {
            if ( !pIncomeData->tunnels_data[ iIncome ].tcp_connection_established )
            {
                struct sockaddr_in address;
                int addrlen = sizeof(address);

                // Create socket file descriptor
                if ((pIncomeData->tunnels_data[ iIncome ].tcp_server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
                    _DETAIL_ERROR("socket failed");
                    continue;
                }

                // Prepare the sockaddr_in structure
                address.sin_family = AF_INET;
                address.sin_addr.s_addr = INADDR_ANY;
                //address.sin_addr.s_addr = inet_addr(pIncomeData->tunnels_data[ iIncome ].tcp_ip); // Specify the IP address to bind to
                address.sin_port = htons(pIncomeData->tunnels_data[ iIncome ].tcp_port_number);

                // Bind the socket to the specified port and IP address
                if (bind(pIncomeData->tunnels_data[ iIncome ].tcp_server_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
                    _DETAIL_ERROR("bind failed");
                    continue;
                }

                // Listen for incoming connections
                if (listen(pIncomeData->tunnels_data[ iIncome ].tcp_server_sock, 1) < 0)
                {
                    // 3 is the backlog queue size
                    _DETAIL_ERROR("listen failed");
                    continue;
                }

                // Accept a new connection
                int newfd = -1;
                if ((newfd = accept(pIncomeData->tunnels_data[ iIncome ].tcp_server_sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                    _close_socket( pIncomeData->tunnels_data[ iIncome ].tcp_server_sock );
                    _DETAIL_ERROR("accept failed");
                    continue; // Continue listening for other connections
                }

                pIncomeData->tunnels_data[ iIncome ].tcp_client_sock = newfd;
                pIncomeData->tunnels_data[ iIncome ].tcp_connection_established++;
                pIncomeData->tcp_connection_count++;

                printf("Server listening on port %d\n", pIncomeData->tunnels_data[ iIncome ].tcp_port_number );
            }
        }
    }

    return NULL; // Threads can return a value, but this example returns NULL
}

// Function to handle client communication in a separate thread
void *income_handler(void *p_income_data)
{
    
    char custom_message[256];
    income_data * pIncomeData = ( income_data * )p_income_data;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ( !pIncomeData->tcp_connection_count )
    {
        sleep(1);
    }

    fd_set readfds; // Set of socket descriptors

    while ( 1 )
    {
        // Clear the socket set
        FD_ZERO( &readfds );

        int sockfd_max = -1; // for select compulsion
        for ( int i = 0 ; i < pIncomeData->tcp_connection_count ; i++ )
        {
            if ( pIncomeData->tunnels_data[ i ].tcp_connection_established )
            {
                // Add socket to set
                FD_SET( pIncomeData->tunnels_data[ i ].tcp_client_sock , &readfds );
                if ( pIncomeData->tunnels_data[ i ].tcp_client_sock > sockfd_max )
                {
                    sockfd_max = pIncomeData->tunnels_data[ i ].tcp_client_sock;
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
            //_DETAIL_ERROR("select error");
            continue;
        }
        if ( activity == 0 )
        {
            //_DETAIL_ERROR("timed out");
            continue;
        }

        for ( int i = 0 ; i < pIncomeData->tcp_connection_count ; i++ )
        {
            if ( pIncomeData->tunnels_data[i].tcp_connection_established )
            {
                if ( FD_ISSET( pIncomeData->tunnels_data[i].tcp_client_sock , &readfds ) )
                {
                    bytes_read = recv( pIncomeData->tunnels_data[i].tcp_client_sock , buffer , BUFFER_SIZE - 1 , 0 );
                    if ( bytes_read >= 0 )
                    {
                        buffer[ bytes_read ] = '\0'; // Null-terminate the received data
                        printf( "Received from client %s %d: %s\n" , pIncomeData->tunnels_data[i].tcp_ip , pIncomeData->tunnels_data[i].tcp_port_number , buffer );
                    }
                }
            }
        }
    }

    //printf("Client connected on socket %d\n", client_socket);

    //while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
    //    buffer[bytes_read] = '\0'; // Null-terminate the received data
    //    printf("Received from client %d: %s\n", client_socket, buffer);

    //    // Echo back the received data
    //    //send(client_socket, buffer, bytes_read, 0);
    //}

    //if (bytes_read == 0) {
    //    printf("Client %d disconnected\n", client_socket);
    //} else {
    //    _DETAIL_ERROR("recv failed");
    //}

    //close(client_socket);
    //free(client_socket_ptr); // Free the dynamically allocated memory for the socket
    //pthread_exit(NULL);
}

int main()
{
    char custom_message[256];
    //int server_fd, new_socket;
    //struct sockaddr_in address;
    //int addrlen = sizeof(address);

    pthread_t thread_tcp_connection;

    income_data tcps;
    memset( &tcps , 0 , sizeof(tcps) );
    tcps.tunnels_data[ 0 ].tcp_ip = TCP_SERVER_IP_1;
    tcps.tunnels_data[ 0 ].tcp_port_number = TCP_PORT_1;

    tcps.tunnels_data[ 1 ].tcp_ip = TCP_SERVER_IP_2;
    tcps.tunnels_data[ 1 ].tcp_port_number = TCP_PORT_2;

    if (pthread_create(&thread_tcp_connection, NULL, thread_tcp_connection_proc, (void *)&tcps) != 0) {
        _DETAIL_ERROR("Failed to create thread");
        return 1; // Indicate an error
    }

    pthread_t thread_income_handler;

    if (pthread_create(&thread_income_handler, NULL, income_handler, (void *)&tcps) != 0) {
        _DETAIL_ERROR("Failed to create thread");
        return 1; // Indicate an error
    }

    //// Create socket file descriptor
    //if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    //    _DETAIL_ERROR("socket failed");
    //    exit(EXIT_FAILURE);
    //}

    //// Prepare the sockaddr_in structure
    //address.sin_family = AF_INET;
    ////address.sin_addr.s_addr = INADDR_ANY;
    //address.sin_addr.s_addr = inet_addr(TCP_IP); // Specify the IP address to bind to
    //address.sin_port = htons(TCP_PORT);

    //// Bind the socket to the specified port and IP address
    //if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    //    _DETAIL_ERROR("bind failed");
    //    exit(EXIT_FAILURE);
    //}

    //// Listen for incoming connections
    //if (listen(server_fd, 3) < 0) { // 3 is the backlog queue size
    //    _DETAIL_ERROR("listen failed");
    //    exit(EXIT_FAILURE);
    //}

    //printf("Server listening on port %d\n", TCP_PORT);

    //while (1) {
    //    // Accept a new connection
    //    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
    //        _DETAIL_ERROR("accept failed");
    //        continue; // Continue listening for other connections
    //    }

    //    // Dynamically allocate memory for the client socket descriptor
    //    int *client_socket_ptr = (int *)malloc(sizeof(int));
    //    if (client_socket_ptr == NULL) {
    //        _DETAIL_ERROR("malloc failed");
    //        close(new_socket);
    //        continue;
    //    }
    //    *client_socket_ptr = new_socket;

    //    pthread_t thread_id;
    //    if (pthread_create(&thread_id, NULL, handle_client, (void *)client_socket_ptr) != 0) {
    //        _DETAIL_ERROR("pthread_create failed");
    //        close(new_socket);
    //        free(client_socket_ptr);
    //        continue;
    //    }
    //    pthread_detach(thread_id); // Detach the thread so its resources are automatically reclaimed on exit
    //}

    if (pthread_join(thread_income_handler, NULL) != 0) {
        _DETAIL_ERROR("Failed to join thread");
        return 1; // Indicate an error
    }

    //close(server_fd); // This part is typically unreachable in a continuous server
    return 0;
}

