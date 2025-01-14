//Emanuel Contreras lct377

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    // 1.) socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2.) connect
    struct sockaddr_in server_addr; //Will hold server address information
    memset(&server_addr, 0, sizeof(server_addr)); //initialize the server address structure with the zeros
    server_addr.sin_family = AF_INET; //Set the address family to IPv4
    server_addr.sin_port = htons(8080); //Set the port number to 8080
    char *ip_address = "127.0.0.1"; //Server IP address
    inet_pton(AF_INET, ip_address, &server_addr.sin_addr); //converting IP address to a binary format

    printf("Connecting to server\n");
    int connect_result = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_result == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\n");

    // 3.) communicate
    printf("Sending hello to server\n");
    send(client_fd, "hello", strlen("hello"), 0);

    char buf[1024] = {0}; //buffer to store received data
    ssize_t bytes_received = recv(client_fd, buf, sizeof(buf) - 1, 0); //will receive data from the server
    if (bytes_received == -1) {
        perror("recv");
    } else if (bytes_received == 0) {
        printf("Connection closed by server\n");
    } else {
        buf[bytes_received] = '\0'; //null-terminate teh received data
        printf("Received: %s\n", buf); //Prints the received data
    }

    // 4.) close connection
    close(client_fd);

    return 0;
}
