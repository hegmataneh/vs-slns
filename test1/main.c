#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1" // Replace with the server's IP address
#define SERVER_PORT 8080
#define MESSAGE "Hello, Server!"

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[1024] = {0};

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // Send data
    send(client_socket, MESSAGE, strlen(MESSAGE), 0);
    printf("Message sent: %s\n", MESSAGE);

    // Receive data (optional)
    ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer));
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
    }

    // Close the socket
    close(client_socket);

    return 0;
}