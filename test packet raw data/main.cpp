#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>

#define BUFFER_SIZE 2048
#define TARGET_PORT 1234   // Change this to your desired UDP port

void dump_buffer(const void *buffer, size_t size) {
    const unsigned char *buf = (const unsigned char *)buffer;
    size_t i;

    for (i = 0; i < size; i += 16) {
        printf("%08zx  ", i);

        // Print hex part
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02x ", buf[i + j]);
            else
                printf("   ");
        }

        printf(" ");

        // Print ASCII part
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            unsigned char c = buf[i + j];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("\n");
    }
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];

    // Open raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Bind to a specific interface
    struct sockaddr_ll sll;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "enp0s3", IFNAMSIZ);  // <-- Change this to your interface

    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        perror("SIOCGIFINDEX");
        close(sockfd);
        return 1;
    }

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(sockfd, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("Listening for UDP packets to port %d on interface %s...\n", TARGET_PORT, ifr.ifr_name);



    while (1) {
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom");
            break;
        }

        

        // Parse Ethernet header
        if (n < sizeof(struct ethhdr)) continue;
        struct ethhdr *eth = (struct ethhdr *)buffer;
        if (ntohs(eth->h_proto) != ETH_P_IP) continue;

        // Parse IP header
        if (n < sizeof(struct ethhdr) + sizeof(struct iphdr)) continue;
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol != IPPROTO_UDP) continue;

        int ip_header_len = ip->ihl * 4;
        if (n < sizeof(struct ethhdr) + ip_header_len + sizeof(struct udphdr)) continue;

        // Parse UDP header
        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + ip_header_len);
        int dest_port = ntohs(udp->dest);
        if (dest_port != TARGET_PORT) continue;

        int udp_payload_len = ntohs(udp->len) - sizeof(struct udphdr);

        if ( n == 60 )
        {
        dump_buffer( buffer , n );
        break;
        }

        printf("-------------------------------------------\n");
        printf("Captured full Ethernet frame: %ld bytes\n", n);
        printf("  - UDP Dest Port: %d\n", dest_port);
        printf("  - UDP Payload Length: %d bytes\n", udp_payload_len);
        printf("  - Total Overhead (Ethernet+IP+UDP): ~%ld bytes\n", n - udp_payload_len);

        // Optional: print first few payload bytes
        char *payload = (char *)udp + sizeof(struct udphdr);
        int printable = udp_payload_len < 16 ? udp_payload_len : 16;
        printf("  - First %d payload bytes: ", printable);
        for (int i = 0; i < printable; i++) {
            printf("%02x ", (unsigned char)payload[i]);
        }
        printf("\n");
    }

    close(sockfd);
    return 0;
}


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <netinet/ip.h>
//#include <netinet/udp.h>
//#include <netinet/if_ether.h>
//#include <sys/socket.h>
//#include <netpacket/packet.h>
//#include <net/ethernet.h>
//#include <net/if.h>
//#include <sys/ioctl.h>
//
//#define BUFFER_SIZE 2048
//
//int main() {
//    int sockfd;
//    char buffer[BUFFER_SIZE];
//
//    // Open raw socket for Ethernet packets
//    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
//    if (sockfd < 0) {
//        perror("socket");
//        return 1;
//    }
//
//    // Bind to a specific interface (change to your interface name)
//    struct sockaddr_ll sll;
//    struct ifreq ifr;
//    memset(&ifr, 0, sizeof(ifr));
//    strncpy(ifr.ifr_name, "enp0s3", IFNAMSIZ);  // <-- change this if needed
//
//    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
//        perror("SIOCGIFINDEX");
//        return 1;
//    }
//
//    memset(&sll, 0, sizeof(sll));
//    sll.sll_family = AF_PACKET;
//    sll.sll_ifindex = ifr.ifr_ifindex;
//    sll.sll_protocol = htons(ETH_P_ALL);
//
//    if (bind(sockfd, (struct sockaddr*)&sll, sizeof(sll)) == -1) {
//        perror("bind");
//        return 1;
//    }
//
//    printf("Listening on raw socket...\n");
//
//    while (1) {
//        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
//        if (n < 0) {
//            perror("recvfrom");
//            break;
//        }
//
//        // Optional filter: only UDP packets
//        struct ethhdr *eth = (struct ethhdr *)buffer;
//        if (ntohs(eth->h_proto) != ETH_P_IP) continue;
//
//        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
//        if (ip->protocol != IPPROTO_UDP) continue;
//
//        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + ip->ihl * 4);
//        int udp_payload_len = ntohs(udp->len) - sizeof(struct udphdr);
//
//        printf("Got full packet: %ld bytes (UDP payload: %d bytes)\n", n, udp_payload_len);
//    }
//
//    close(sockfd);
//    return 0;
//}
