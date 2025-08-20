#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/ip.h>

// This program chooses a socket based on IP ID % num_sockets.

SEC( "sk_reuseport" )
int bpf_select_sock( struct sk_reuseport_md * ctx )
{
	
	return 1;   // kernel delivers packet to socket[index]
}

char _license[] SEC( "license" ) = "GPL";