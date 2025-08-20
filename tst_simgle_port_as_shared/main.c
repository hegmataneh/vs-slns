#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PORT 1234
#define NUM_PROCESSES 8
#define BUF_SIZE 2048

int main()
{
	int i;
	pid_t pid;

	for ( i = 0; i < NUM_PROCESSES; i++ )
	{
		pid = fork();
		if ( pid < 0 )
		{
			perror( "fork" );
			exit( 1 );
		}
		else if ( pid == 0 )
		{
			// Child process: create UDP socket
			int sock = socket( AF_INET , SOCK_DGRAM , 0 );
			if ( sock < 0 )
			{
				perror( "socket" );
				exit( 1 );
			}

			int opt = 1;
			if ( setsockopt( sock , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof( opt ) ) < 0 )
			{
				perror( "setsockopt SO_REUSEPORT" );
				exit( 1 );
			}

			opt = 1;
			if ( setsockopt( sock , SOL_SOCKET , 15 , &opt , sizeof( opt ) ) < 0 )
			{
				perror( "setsockopt SO_REUSEPORT" );
				exit( 1 );
			}

			struct sockaddr_in addr;
			memset( &addr , 0 , sizeof( addr ) );
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = inet_addr("192.168.100.60");
			addr.sin_port = htons( PORT );

			if ( bind( sock , ( struct sockaddr * )&addr , sizeof( addr ) ) < 0 )
			{
				perror( "bind" );
				exit( 1 );
			}

			printf( "Process %d listening on UDP port %d\n" , getpid() , PORT );

			unsigned char buf[ BUF_SIZE ];
			static int count = 0;

			while ( 1 )
			{
				ssize_t n = recvfrom( sock , buf , BUF_SIZE , 0 , NULL , NULL );
				if ( n > 0 )
				{
					count++;
					// Optional: print packet info
					// printf("Process %d received packet of %zd bytes\n", getpid(), n);
				}
				if ( count % 1000 == 0 )
				{
					printf( "Process %d received %d packets\n" , getpid() , count );
				}
			}
			close( sock );
			exit( 0 );
		}
		// Parent continues to fork next child
	}

	// Parent waits for children
	for ( i = 0; i < NUM_PROCESSES; i++ )
	{
		wait( NULL );
	}

	return 0;
}
