#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>           // for close()
#include <arpa/inet.h>        // for sockaddr_in, inet_ntoa()
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int udp_sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to local port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // listen on all interfaces
    server_addr.sin_port = htons(PORT);

    if (bind(udp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(udp_sock);
        exit(EXIT_FAILURE);
    }

    printf("UDP server is listening on port %d\n", PORT);

    // Set up for select()
    fd_set read_fds;
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(udp_sock, &read_fds);

        int max_fd = udp_sock + 1;

        int activity = select(max_fd, &read_fds, NULL, NULL, NULL); // blocking wait
        if (activity < 0) {
            perror("select error");
            break;
        }

        if (FD_ISSET(udp_sock, &read_fds)) {
            int len = recvfrom(udp_sock, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr*)&client_addr, &addr_len);
            if (len > 0) {
                buffer[len] = '\0';  // Null-terminate the string
                printf("Received from %s:%d -> %s\n",
                        inet_ntoa(client_addr.sin_addr),
                        ntohs(client_addr.sin_port),
                        buffer);
            }
        }
    }

    close(udp_sock);
    return 0;
}
