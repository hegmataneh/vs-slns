/*
  ID: C_SAMPLE_UDP_TO_TCP_01
  Description:
    Listen on UDP port 10200.
    For each received UDP datagram:
      → forward data to a persistent TCP connection at 192.168.4.2:10100.

  Notes:
    - Uses basic socket options (reuseaddr).
    - Keeps TCP socket connected.
    - Handles UDP packets of any size up to buffer limit.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define UDP_PORT 10200
#define TCP_PORT 10100
#define TCP_IP   "192.168.4.2"

#define BUF_SIZE 65535

int main() {
    int udp_sock, tcp_sock;
    struct sockaddr_in udp_addr, tcp_addr;
    char buffer[BUF_SIZE];

    /* ---------------------- UDP SOCKET SETUP ---------------------- */
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("UDP socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);

    if (bind(udp_sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("bind");
        close(udp_sock);
        exit(1);
    }

    printf("UDP listener started on port %d …\n", UDP_PORT);

    /* ---------------------- TCP SOCKET SETUP ---------------------- */
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) {
        perror("TCP socket");
        exit(1);
    }

    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(TCP_PORT);

    if (inet_pton(AF_INET, TCP_IP, &tcp_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    printf("Connecting TCP to %s:%d …\n", TCP_IP, TCP_PORT);

    /* Persistent TCP connection attempt */
    if (connect(tcp_sock, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0) {
        perror("connect");
        close(tcp_sock);
        exit(1);
    }

    printf("TCP connected.\n");

    long _count = 0;

    /* ---------------------- MAIN LOOP ---------------------- */
    while (1) {
        ssize_t received = recvfrom(udp_sock, buffer, BUF_SIZE, 0, NULL, NULL);
        if (received < 0) {
            perror("recvfrom");
            continue;
        }

        //buffer[ received ] = '\n';
        //buffer[ received + 1 ] = 0;


        /* Forward UDP data to TCP */
        ssize_t sent = send(tcp_sock, buffer, received, 0);
        if (sent < 0) {
            perror("send to TCP");
            // You may reconnect TCP again here if you want auto-recovery.
        }

        send(tcp_sock, "\n", 1, 0);


        if ( !( ( _count++ ) % 1000 ) )
        {
            printf("pkts %ld\n", _count );
        }

        // Optionally print debug information
        //printf("Forwarded %ld bytes UDP → TCP\n", (long)received);

        //break;

        if ( _count >= 10000 ) break;
    }

    printf("pkts %ld\n", _count );

    close(udp_sock);
    close(tcp_sock);
    return 0;
}
