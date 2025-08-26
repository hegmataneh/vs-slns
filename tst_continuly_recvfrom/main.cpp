#define _POSIX_C_SOURCE 200809L
//#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>

#include <sys/time.h>
#include <sched.h>
#include <sys/mman.h>
#include <time.h>


#define MYPORT "1234"
#define MAXBUFLEN 100


void * get_in_addr( struct sockaddr * sa )
{
	if ( sa->sa_family == AF_INET )
	{
		return &( ( ( struct sockaddr_in * )sa )->sin_addr );
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int _byte_counter = 0;
int _continue_counter = 0;

void * config_loader( void * )
{
	while ( 1 )
	{
		printf( "%d - %d\n" , _byte_counter , _continue_counter );
		sleep(1);
	}
	
	return NULL;
}

volatile sig_atomic_t stop = 0;

void handle_sigint( int sig )
{
	( void )sig;
	stop = 1;
}

int set_cpu_affinity( int cpu )
{
	cpu_set_t cpus;
	CPU_ZERO( &cpus );
	CPU_SET( cpu , &cpus );
	return sched_setaffinity( 0 , sizeof( cpus ) , &cpus );
}

#define RX_BUF_BYTES (4 * 1024 * 1024)

#define PACKET_SIZE 1

// 4
#define ALLOCATION_ALIGN_SIZE 64 /*64*/

int main( void )
{
	int sockfd;
	struct addrinfo hints , * servinfo , * p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	
	// 1
	//char buf[ PACKET_SIZE ];
	char * buf = ( char * )malloc( PACKET_SIZE );

	// 2. aligned allocation for potential cache benefits
	if ( posix_memalign( ( void ** )&buf , ALLOCATION_ALIGN_SIZE , PACKET_SIZE ) != 0 )
	{
		fprintf( stderr , "posix_memalign failed\n" );
		return 1;
	}

	// 3
	if ( set_cpu_affinity( 0 ) != 0 )
	{
		perror( "sched_setaffinity" );
	}


	socklen_t addr_len;
	//char s[ INET6_ADDRSTRLEN ];

	/*signal( SIGINT , handle_sigint );
	signal( SIGTERM , handle_sigint );*/

	memset( &hints , 0 , sizeof( hints ) );
	hints.ai_family = AF_UNSPEC; //settoAF_INETtouseIPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; //usemy IP

	pthread_t trd_config_loader;
	pthread_create( &trd_config_loader , NULL , config_loader , NULL );

	if ( ( rv = getaddrinfo( NULL , MYPORT , &hints , &servinfo ) ) != 0 )
	{
		fprintf( stderr , "getaddrinfo: %s\n" , gai_strerror( rv ) );
		return 1;
	}

	//loopthroughalltheresultsandbindtothe firstwecan
	for ( p = servinfo; p != NULL; p = p->ai_next )
	{
		if ( ( sockfd = socket( p->ai_family , p->ai_socktype ,
			p->ai_protocol ) ) == -1 )
		{
			perror( "listener:socket" );
			continue;
		}

		int val = 100; // microseconds to spin per syscall
		if ( setsockopt( sockfd , SOL_SOCKET , SO_BUSY_POLL , &val , sizeof( val ) ) < 0 )
		{
			perror( "setsockopt SO_BUSY_POLL" );
			close( sockfd );
			pthread_exit( NULL );
		}

		if ( bind( sockfd , p->ai_addr , p->ai_addrlen ) == -1 )
		{
			close( sockfd );
			perror( "listener:bind" );
			continue;
		}
		break;
	}

	if ( p == NULL )
	{
		fprintf( stderr , "listener:failedtobindsocket\n" );
		return 2;
	}

	freeaddrinfo( servinfo );

	// 5. Enlarge the receive buffer; may require permissions to set very large.
	int iii = RX_BUF_BYTES;
	if ( setsockopt( sockfd , SOL_SOCKET , SO_RCVBUF , &iii , sizeof( iii ) ) != 0 )
		perror( "SO_RCVBUF" );

	//socklen_t optlen = sizeof(iii);

	//if ( getsockopt( sockfd , SOL_SOCKET , SO_RCVBUF , &iii , &optlen ) < 0 )
	//{
	//}

	printf( "listener:waitingtorecvfrom...\n" );

	_byte_counter = 0;
	_continue_counter = 0;

	// 6
	//int yes=1;
	//setsockopt( sockfd , SOL_SOCKET , SO_REUSEADDR , &yes ,  sizeof( int ) );

	// 7
	//fcntl(sockfd, F_SETFL, O_NONBLOCK);

	// 8
	addr_len = sizeof( their_addr );
	socklen_t * paddr_len = &addr_len;
	struct sockaddr * ptheir_addr = ( struct sockaddr * )&their_addr;

	while(!stop)
	{
		// 9
		numbytes = recvfrom( sockfd , buf , PACKET_SIZE , MSG_DONTWAIT , ptheir_addr , paddr_len);

		if ( numbytes <= 0 )
		{
			if ( numbytes == 0 )
			{
				break;
			}
			if ( errno == EAGAIN || errno == EWOULDBLOCK )
			{
				continue;
			}
			perror( "recvfrom" );
			break;
		}
		
		_byte_counter++;
		
	}


	close( sockfd );
	return 0;
}
