#include <stdio.h>
#include <stdlib.h> // also Required for rand() and srand()
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // For close()
#include <errno.h> // For errno
#include <stdarg.h>
#include <time.h>   // Required for time() to seed srand()

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

#define BUFFER_SIZE 1024

#define DEST_PORT_1 1234
#define DEST_IP_1 "192.168.1.60"

#define DEST_PORT_2 1235
#define DEST_IP_2 "192.168.1.60"

#define SOCKS_COUNT 2

// Structure to pass arguments to the thread
struct ThreadArgs {
    int sockfds[SOCKS_COUNT];
    struct sockaddr_in dest_addrs[SOCKS_COUNT];
};

// Thread function to send UDP packets
void *send_udp_thread(void *arg) {

    char custom_message[256];

    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("Enter text to send (or 'exit'): ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            _DETAIL_ERROR("fgets failed");
            break;
        }

        // Remove trailing newline character if present
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting sender thread.\n");
            break;
        }
        ssize_t bytes_sent = 0;
        int dest = rand() % 3;
        switch ( dest )
        {
            case 0:
            {
                bytes_sent += sendto(args->sockfds[0] , buffer , strlen(buffer) , 0 ,
                                     (struct sockaddr *)&args->dest_addrs[0] ,
                                     sizeof(args->dest_addrs[0]));
                break;
            }
            case 1:
            {
                bytes_sent = sendto(args->sockfds[1] , buffer , strlen(buffer) , 0 ,
                                     (struct sockaddr *)&args->dest_addrs[1] ,
                                     sizeof(args->dest_addrs[1]));
                break;
            }
            case 2:
            {
                bytes_sent = sendto(args->sockfds[0] , buffer , strlen(buffer) , 0 ,
                                     (struct sockaddr *)&args->dest_addrs[0] ,
                                     sizeof(args->dest_addrs[0]));
                bytes_sent += sendto(args->sockfds[1] , buffer , strlen(buffer) , 0 ,
                                     (struct sockaddr *)&args->dest_addrs[1] ,
                                     sizeof(args->dest_addrs[1]));
                break;
            }
        }
        if ( bytes_sent < 0 )
        {
            _DETAIL_ERROR("sendto failed");
            break;
        }
        printf("Sent %zd bytes: '%s' to terminal %d\n", bytes_sent, buffer, dest);
    }
    pthread_exit(NULL);
}

int main() {

    char custom_message[256];

    srand(time(NULL));

    //int sockfd_1;
    //struct sockaddr_in dest_addr_1;
    pthread_t send_thread_id;
    struct ThreadArgs thread_args = {0};

    // Create UDP socket
    if ( ( thread_args.sockfds[0] = socket(AF_INET, SOCK_DGRAM, 0) ) == -1 )
    {
        _DETAIL_ERROR("socket creation failed");
        return 1;
    }
    if ( ( thread_args.sockfds[1] = socket(AF_INET, SOCK_DGRAM, 0) ) == -1 )
    {
        _DETAIL_ERROR("socket creation failed");
        return 1;
    }

    // Setup destination address
    memset(&thread_args.dest_addrs[0] , 0 , sizeof(thread_args.dest_addrs[0]));
    thread_args.dest_addrs[0].sin_family = AF_INET;
    thread_args.dest_addrs[0].sin_port = htons(DEST_PORT_1);
    if (inet_pton(AF_INET, DEST_IP_1, &thread_args.dest_addrs[0].sin_addr) <= 0) { // Example: localhost
        _DETAIL_ERROR("invalid address/address not supported");
        close(thread_args.sockfds[0]);
        return 1;
    }

    memset(&thread_args.dest_addrs[1] , 0 , sizeof(thread_args.dest_addrs[1]));
    thread_args.dest_addrs[1].sin_family = AF_INET;
    thread_args.dest_addrs[1].sin_port = htons(DEST_PORT_2);
    if (inet_pton(AF_INET, DEST_IP_2, &thread_args.dest_addrs[1].sin_addr) <= 0) { // Example: localhost
        _DETAIL_ERROR("invalid address/address not supported");
        close(thread_args.sockfds[1]);
        return 1;
    }

    // Prepare thread arguments
    //thread_args_1.sockfd = sockfd_1;
    //thread_args_1.dest_addr = dest_addr_1;

    // Create the sending thread
    if (pthread_create(&send_thread_id, NULL, send_udp_thread, (void *)&thread_args) != 0) {
        _DETAIL_ERROR("pthread_create failed");
        close(thread_args.sockfds[0]);
        close(thread_args.sockfds[1]);
        return 1;
    }

    // Main thread can do other work or simply wait for the sender thread to finish
    pthread_join(send_thread_id, NULL);

    close(thread_args.sockfds[0]);
    close(thread_args.sockfds[1]);
    return 0;
}

