/* High-performance UDP packet counter using AF_PACKET and mmap with TPACKET_V3.
 * Counts UDP packets destined to port 1234 on interface enp0s3 without loss at high speeds.
 * Based on Linux kernel documentation example, adapted for specific counting.
 * Run as root. Use Ctrl+C to stop and see the count.
 */

 #include <linux/filter.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <netinet/udp.h>

#ifndef likely
# define likely(x)          __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)                __builtin_expect(!!(x), 0)
#endif

struct block_desc {
        uint32_t version;
        uint32_t offset_to_priv;
        struct tpacket_hdr_v1 h1;
};

struct ring {
        struct iovec *rd;
        uint8_t *map;
        struct tpacket_req3 req;
};

static uint64_t udp_count = 0;
static sig_atomic_t sigint = 0;

static void sighandler(int num) {
        sigint = 1;
}

static int setup_socket(struct ring *ring, char *netdev) {
        int err, i, fd, v = TPACKET_V3;
        struct sockaddr_ll ll;
        unsigned int blocksiz = 1 << 22;  // 4MB block size for high performance
        unsigned int framesiz = 1 << 11;  // 2KB frame size
        unsigned int blocknum = 64;       // 64 blocks

        fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (fd < 0) {
                perror("socket");
                exit(1);
        }

        err = setsockopt(fd, SOL_PACKET, PACKET_VERSION, &v, sizeof(v));
        if (err < 0) {
                perror("setsockopt PACKET_VERSION");
                exit(1);
        }

        memset(&ring->req, 0, sizeof(ring->req));
        ring->req.tp_block_size = blocksiz;
        ring->req.tp_frame_size = framesiz;
        ring->req.tp_block_nr = blocknum;
        ring->req.tp_frame_nr = (blocksiz * blocknum) / framesiz;
        ring->req.tp_retire_blk_tov = 60;  // Timeout in ms to retire blocks
        ring->req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;

        err = setsockopt(fd, SOL_PACKET, PACKET_RX_RING, &ring->req, sizeof(ring->req));
        if (err < 0) {
                perror("setsockopt PACKET_RX_RING");
                exit(1);
        }

        ring->map = (uint8_t*)mmap(NULL, ring->req.tp_block_size * ring->req.tp_block_nr,
                         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
        if (ring->map == MAP_FAILED) {
                perror("mmap");
                exit(1);
        }

        ring->rd = (iovec*)malloc(ring->req.tp_block_nr * sizeof(*ring->rd));
        assert(ring->rd);
        for (i = 0; i < ring->req.tp_block_nr; ++i) {
                ring->rd[i].iov_base = ring->map + (i * ring->req.tp_block_size);
                ring->rd[i].iov_len = ring->req.tp_block_size;
        }

        memset(&ll, 0, sizeof(ll));
        ll.sll_family = PF_PACKET;
        ll.sll_protocol = htons(ETH_P_ALL);
        ll.sll_ifindex = if_nametoindex(netdev);
        ll.sll_hatype = 0;
        ll.sll_pkttype = 0;
        ll.sll_halen = 0;


		struct sock_filter filter[] = {
			{ 0x28, 0, 0, 0x0000000c },  // ldh [12]   (ether proto)
			{ 0x15, 0, 7, 0x00000800 },  // jeq #2048 jt 2 jf 9  (IP)
			{ 0x30, 0, 0, 0x00000017 },  // ldb [23]   (IP proto)
			{ 0x15, 0, 5, 0x00000011 },  // jeq #17 jt 4 jf 9   (UDP)
			{ 0x28, 0, 0, 0x00000014 },  // ldh [20]   (IP len)
			{ 0x45, 3, 0, 0x00001fff },  // jset #8191 jt 9 jf 7 (no frag)
			{ 0xb1, 0, 0, 0x0000000e },  // ldx 4*([14]&0xf) (IP hdr len)
			{ 0x48, 0, 0, 0x00000012 },  // ldh [x + 18] (UDP dest port, adjust offset if var hdr)
			{ 0x15, 0, 0, 0x000004d2 },  // jeq #1234 jt 9 jf 9
			{ 0x06, 0, 0, 0x00040000 },  // ret #262144 (accept)
			{ 0x06, 0, 0, 0x00000000 },  // ret #0 (drop)
		};
		struct sock_fprog prog = { .len = sizeof( filter ) / sizeof( filter[ 0 ] ), .filter = filter };
		setsockopt( fd , SOL_SOCKET , SO_ATTACH_FILTER , &prog , sizeof( prog ) );


        err = bind(fd, (struct sockaddr *) &ll, sizeof(ll));
        if (err < 0) {
                perror("bind");
                exit(1);
        }

        return fd;
}

static void flush_block(struct block_desc *pbd) {
        pbd->h1.block_status = TP_STATUS_KERNEL;
}

static void walk_block(struct block_desc *pbd) {
        int num_pkts = pbd->h1.num_pkts, i;
        struct tpacket3_hdr *ppd;

        ppd = (struct tpacket3_hdr *) ((uint8_t *) pbd + pbd->h1.offset_to_first_pkt);
        for (i = 0; i < num_pkts; ++i) {
                struct ethhdr *eth = (struct ethhdr *) ((uint8_t *) ppd + ppd->tp_mac);
                if (ntohs(eth->h_proto) == ETH_P_IP) {
                        struct iphdr *iph = (struct iphdr *) ((uint8_t *)eth + ETH_HLEN);
                        if (iph->protocol == IPPROTO_UDP && (iph->ihl * 4 + ETH_HLEN) <= ppd->tp_snaplen) {
                                struct udphdr *udph = (struct udphdr *) ((uint8_t *)iph + (iph->ihl * 4));
                                if (ntohs(udph->dest) == 1234) {
                                        udp_count++;
                                }
                        }
                }
                ppd = (struct tpacket3_hdr *) ((uint8_t *) ppd + ppd->tp_next_offset);
        }
}

int main(int argc, char **argv) {
        int fd;
        struct ring ring;
        struct pollfd pfd;
        unsigned int block_num = 0;
        char *interface = (char *)"enp0s3";  // Hardcoded interface

        signal(SIGINT, sighandler);

        memset(&ring, 0, sizeof(ring));
        fd = setup_socket(&ring, interface);
        assert(fd > 0);

        memset(&pfd, 0, sizeof(pfd));
        pfd.fd = fd;
        pfd.events = POLLIN | POLLERR;
        pfd.revents = 0;

        while (likely(!sigint)) {
                struct block_desc *pbd = (struct block_desc *) ring.rd[block_num].iov_base;

                if ((pbd->h1.block_status & TP_STATUS_USER) == 0) {
                        poll(&pfd, 1, -1);
                        continue;
                }

                walk_block(pbd);
                flush_block(pbd);
                block_num = (block_num + 1) % ring.req.tp_block_nr;
        }

        printf("\nCounted %" PRIu64 " UDP packets to port 1234\n", udp_count);

        close(fd);
        munmap(ring.map, ring.req.tp_block_size * ring.req.tp_block_nr);
        free(ring.rd);
        return 0;
}
