#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT1 12345
#define PORT2 12346
#define BUFFER_SIZE 1024

int client1_sock = -1, client2_sock = -1;

void* relay_messages(void* arg) {
    int from_sock = *((int*)arg);
    int to_sock = (from_sock == client1_sock) ? client2_sock : client1_sock;

    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = read(from_sock, buffer, BUFFER_SIZE);
        if (bytes <= 0) {
            printf("One client disconnected.\n");
            break;
        }
        printf("Forwarding message: %s", buffer);
        send(to_sock, buffer, bytes, 0);
    }

    close(from_sock);
    return NULL;
}

int setup_listener(int port) {
    int server_fd;
    struct sockaddr_in address;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 1);

    return server_fd;
}

int main() {
    int server1_fd = setup_listener(PORT1);
    int server2_fd = setup_listener(PORT2);

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    printf("Waiting for Client 1 on port %d...\n", PORT1);
    client1_sock = accept(server1_fd, (struct sockaddr*)&addr, &addrlen);
    printf("Client 1 connected.\n");

    printf("Waiting for Client 2 on port %d...\n", PORT2);
    client2_sock = accept(server2_fd, (struct sockaddr*)&addr, &addrlen);
    printf("Client 2 connected.\n");

    pthread_t t1, t2;
    pthread_create(&t1, NULL, relay_messages, &client1_sock);
    pthread_create(&t2, NULL, relay_messages, &client2_sock);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    close(server1_fd);
    close(server2_fd);
    return 0;
}

