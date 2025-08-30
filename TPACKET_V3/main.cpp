// tpackv3_rx.c
//#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
//#include <linux/if.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

/* ===== User defaults (matches your setup) ===== */
static const char *IFACE_NAME = "enp0s3";
static const char *DST_IP_STR = "192.168.100.60";
static const uint16_t DST_PORT = 1234;

/* ===== Ring sizing (tune for your NIC/traffic) =====
   block_size must be page-multiple; bigger blocks => fewer wakeups.
   frame_size typically 2K (>= max_hdr + snaplen); must be power-of-two.
   block_size should be multiple of frame_size.
*/
#define BLOCK_SIZE   (1u << 20)   /* 1 MiB per block */
#define BLOCK_NR     64           /* total 64 MiB ring */
#define FRAME_SIZE   2048         /* 2 KiB frames (hint for alignment) */
#define RETIRE_TOV   60           /* ms to flush partially filled blocks */

#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

static volatile sig_atomic_t g_stop = 0;
static void on_sigint(int signo) { (void)signo; g_stop = 1; }

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

#ifndef ETH_P_8021Q
#define ETH_P_8021Q 0x8100
#endif
#ifndef ETH_P_8021AD
#define ETH_P_8021AD 0x88A8
#endif

struct vlan_hdr
{
	uint16_t h_vlan_TCI;
	uint16_t h_vlan_encapsulated_proto;
} __attribute__( ( packed ) );

/* Simple helpers for memory barriers with TPACKET_V3 producer/consumer */
static inline void mb(void)   { __sync_synchronize(); }

/* Parse Ethernet (+ optional VLAN) -> IPv4 -> UDP.
   Returns pointer to UDP header and sets *payload,*payload_len if matches our filter.
   Returns NULL if not UDP/doesn't match, or malformed. */
static const struct udphdr* parse_udp_filter(const uint8_t *base, size_t cap_len,
                                             uint32_t dst_ip_be, uint16_t dst_port_be,
                                             const uint8_t **payload, uint32_t *payload_len)
{
    if (cap_len < sizeof(struct ethhdr)) return NULL;
    const struct ethhdr *eth = (const struct ethhdr*)base;
    uint16_t eth_type = ntohs(eth->h_proto);
    size_t off = sizeof(struct ethhdr);

    /* Handle single VLAN tag (common). Extend if you need double-tag (QinQ). */
    if (eth_type == ETH_P_8021Q || eth_type == ETH_P_8021AD) {
        if (cap_len < off + sizeof(struct vlan_hdr)) return NULL;
        const struct vlan_hdr *vh = (const struct vlan_hdr*)(base + off);
        eth_type = ntohs(vh->h_vlan_encapsulated_proto);
        off += sizeof(struct vlan_hdr);
    }

    if (eth_type != ETH_P_IP) return NULL;
    if (cap_len < off + sizeof(struct iphdr)) return NULL;

    const struct iphdr *ip = (const struct iphdr*)(base + off);
    if (ip->version != 4) return NULL;
    size_t ihl = ip->ihl * 4u;
    if (ihl < sizeof(struct iphdr)) return NULL;
    if (cap_len < off + ihl) return NULL;

    if (ip->protocol != IPPROTO_UDP) return NULL;

    off += ihl;
    if (cap_len < off + sizeof(struct udphdr)) return NULL;
    const struct udphdr *udp = (const struct udphdr*)(base + off);

    if (ip->daddr != dst_ip_be) return NULL;
    if (udp->dest != dst_port_be) return NULL;

    uint16_t udp_len = ntohs(udp->len);
    if (udp_len < sizeof(struct udphdr)) return NULL;

    size_t udp_payload_off = off + sizeof(struct udphdr);
    size_t udp_payload_len = udp_len - sizeof(struct udphdr);
    if (cap_len < udp_payload_off + udp_payload_len) {
        /* Captured less than full UDP payload (unlikely with PACKET_MMAP unless snaplen small). */
        if (cap_len > udp_payload_off) {
            udp_payload_len = cap_len - udp_payload_off;
        } else {
            return NULL;
        }
    }

    *payload = base + udp_payload_off;
    *payload_len = (uint32_t)udp_payload_len;
    return udp;
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

int main(void)
{
    /* Precompute destination in network order */
    struct in_addr dst_ip;
    if (inet_pton(AF_INET, DST_IP_STR, &dst_ip) != 1) {
        fprintf(stderr, "Bad DST_IP_STR: %s\n", DST_IP_STR);
        return EXIT_FAILURE;
    }
    uint32_t dst_ip_be = dst_ip.s_addr;
    uint16_t dst_port_be = htons(DST_PORT);

	pthread_t trd_config_loader;
	pthread_create( &trd_config_loader , NULL , config_loader , NULL );

    /* 1) Open AF_PACKET socket */
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0) die("socket(AF_PACKET)");

	int val = 50; // microseconds to spin per syscall
	if ( setsockopt( fd , SOL_SOCKET , SO_BUSY_POLL , &val , sizeof( val ) ) < 0 )
	{
		die("setsockopt(SO_BUSY_POLL)");
	}

    /* 2) Version = TPACKET_V3 */
    int ver = TPACKET_V3;
    if (setsockopt(fd, SOL_PACKET, PACKET_VERSION, &ver, sizeof(ver)) < 0)
        die("setsockopt(PACKET_VERSION)");

    /* 3) Optional: report drops instead of blocking if ring is full */
    int loss = 1;
    if (setsockopt(fd, SOL_PACKET, PACKET_LOSS, &loss, sizeof(loss)) < 0)
        die("setsockopt(PACKET_LOSS)");

    /* 4) Build RX ring */
    struct tpacket_req3 req;
    memset(&req, 0, sizeof(req));
    req.tp_block_size = BLOCK_SIZE;
    req.tp_block_nr   = BLOCK_NR;
    req.tp_frame_size = FRAME_SIZE;
    req.tp_frame_nr   = (req.tp_block_size * req.tp_block_nr) / req.tp_frame_size;
    req.tp_retire_blk_tov = RETIRE_TOV; /* flush partially filled blocks after timeout (ms) */
    req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH; /* ask kernel to compute rxhash (optional) */

    if (setsockopt(fd, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req)) < 0)
        die("setsockopt(PACKET_RX_RING)");

    /* 5) mmap the ring (shared between kernel/user) */
    size_t mmap_len = (size_t)req.tp_block_size * req.tp_block_nr;
    void *ring = mmap(NULL, mmap_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ring == MAP_FAILED) die("mmap(PACKET_RX_RING)");

    /* 6) Bind to interface */
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family   = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex  = if_nametoindex(IFACE_NAME);
    if (sll.sll_ifindex == 0) die("if_nametoindex");
    if (bind(fd, (struct sockaddr*)&sll, sizeof(sll)) < 0)
        die("bind(AF_PACKET)");

    /* 7) Setup poll to sleep until blocks are ready */
    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    /* 8) Signal handler for clean exit */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* 9) Iterate ring blocks */
    unsigned int block_num = req.tp_block_nr;
    unsigned int block_idx = 0;

    //uint64_t matched_pkts = 0, matched_bytes = 0;
    uint64_t seen_pkts = 0;

    fprintf(stderr, "TPACKET_V3 RX on %s. Filtering dst %s:%u …\n",
            IFACE_NAME, DST_IP_STR, (unsigned)DST_PORT);

    while (!g_stop) {
        /* Examine current block */
        uint8_t *blk_ptr = (uint8_t*)ring + (size_t)block_idx * req.tp_block_size;
        struct tpacket_block_desc *bd = (struct tpacket_block_desc*)blk_ptr;

        /* Check if kernel has handed us the block */
        if ((bd->hdr.bh1.block_status & TP_STATUS_USER) == 0) {
            /* Sleep until data or timeout (to re-check stop flag) */
            int rc = poll(&pfd, 1, 200 /*ms*/);
            if (rc < 0) {
                if (errno == EINTR) continue;
                die("poll");
            }
            continue;
        }

        /* Memory barrier before reading block contents */
        mb();

        /* Walk all packets in this block */
        uint32_t num_pkts = bd->hdr.bh1.num_pkts;
        uint32_t offset = bd->hdr.bh1.offset_to_first_pkt;

        for (uint32_t i = 0; i < num_pkts; i++) {
            struct tpacket3_hdr *tp3 = (struct tpacket3_hdr*)(blk_ptr + offset);
            const uint8_t *pkt_data = (const uint8_t*)tp3 + tp3->tp_mac;
            uint32_t cap_len = tp3->tp_snaplen;   /* captured length */
            /* Process (filter & count) */
            const uint8_t *payload; uint32_t payload_len;
            const struct udphdr *udph = parse_udp_filter(pkt_data, cap_len,
                                                         dst_ip_be, dst_port_be,
                                                         &payload, &payload_len);
            seen_pkts++;
            if (likely(udph != NULL)) {
                _continue_counter++;
                _byte_counter += payload_len;
                /* Do your fast-path work here; e.g., parse payload */
                /* Example: print small messages (guarded) */
                /*
                if (payload_len <= 64) {
                    fwrite(payload, 1, payload_len, stdout);
                    fputc('\n', stdout);
                }
                */
            }

            /* Advance to next packet in the block */
            if (tp3->tp_next_offset == 0) break;
            offset += tp3->tp_next_offset;
        }

        /* Release block back to kernel */
        bd->hdr.bh1.block_status = TP_STATUS_KERNEL;
        mb();

        /* Next block (ring) */
        block_idx++;
        if (block_idx >= block_num) block_idx = 0;
    }

    /* Graceful shutdown: fetch stats */
    struct tpacket_stats_v3 stats;
    socklen_t slen = sizeof(stats);
    memset(&stats, 0, sizeof(stats));
    if (getsockopt(fd, SOL_PACKET, PACKET_STATISTICS, &stats, &slen) == 0) {
        fprintf(stderr, "\n=== PACKET_MMAP stats ===\n");
        fprintf(stderr, "tp_packets:      %u\n", stats.tp_packets);
        fprintf(stderr, "tp_drops:        %u\n", stats.tp_drops);
        fprintf(stderr, "freeze_q_cnt:    %u\n", stats.tp_freeze_q_cnt);
    }

    fprintf(stderr, "Seen packets:    %llu\n", (unsigned long long)seen_pkts);
    //fprintf(stderr, "Matched packets: %llu\n", (unsigned long long)matched_pkts);
    //fprintf(stderr, "Matched bytes:   %llu\n", (unsigned long long)matched_bytes);

    /* Cleanup */
    munmap(ring, mmap_len);
    close(fd);
    return 0;
}

