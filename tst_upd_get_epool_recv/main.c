// udp_epoll_simple.c
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

static volatile sig_atomic_t stop = 0;
//static unsigned long long count = 0;

void handle_int( int _ )
{
	( void )_; stop = 1;
}

int _byte_counter = 0;
int _continue_counter = 0;

void * config_loader( void * )
{
	while ( 1 )
	{
		printf( "%d - %d\n" , _byte_counter , _continue_counter );
		sleep( 1 );
	}

	return NULL;
}

#define BUFF_SIZE 1

int main( void )
{
	signal( SIGINT , handle_int );

	int sock = socket( AF_INET , SOCK_DGRAM , 0 );
	if ( sock < 0 )
	{
		perror( "socket" );
		return 1;
	}

	pthread_t trd_config_loader;
	pthread_create( &trd_config_loader , NULL , config_loader , NULL );

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons( 1234 );
	addr.sin_addr.s_addr = INADDR_ANY;

	if ( bind( sock , ( struct sockaddr * )&addr , sizeof( addr ) ) < 0 )
	{
		perror( "bind" );
		close( sock );
		return 1;
	}

	int ep = epoll_create1( 0 );
	if ( ep < 0 )
	{
		perror( "epoll_create1" );
		close( sock );
		return 1;
	}

	struct epoll_event ev = { 0 };
	ev.events = EPOLLIN;
	ev.data.fd = sock;
	if ( epoll_ctl( ep , EPOLL_CTL_ADD , sock , &ev ) < 0 )
	{
		perror( "epoll_ctl" );
		close( sock );
		close( ep );
		return 1;
	}

	char buf[ BUFF_SIZE ];
	struct epoll_event events[ 1 ];

	while ( !stop )
	{
		int n = epoll_wait( ep , events , 1 , -1 );
		if ( n < 0 )
		{
			if ( errno == EINTR )
			{
				_continue_counter++;
				continue;
			}
			perror( "epoll_wait" );
			break;
		}
		if ( events[ 0 ].events & EPOLLIN )
		{
			while( 1 )
			{
				//ssize_t r = recvfrom( sock , buf , BUFF_SIZE , 0 , NULL , NULL );
				ssize_t r = recv( sock , buf , BUFF_SIZE , 0 );
				if ( r > 0 )
				{
					_byte_counter++;
					continue;
				}
				break;
			}
		}
	}

	//printf( "\nTotal UDP packets received on port 1234: %llu\n" , count );
	close( sock );
	close( ep );
	return 0;
}
