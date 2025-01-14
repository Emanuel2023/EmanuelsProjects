//Emanuel Contreras lct377

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

int main() {
    // 1.) Socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket"); //Will print the error message
        exit(EXIT_FAILURE); //Exists the program with a known failure.
    }

    // 2.) bind the socket to the address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //sets the IP address
    addr.sin_port = htons(8080); //set the port number

    int bind_result = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (bind_result == -1) {
        perror("bind"); //print the error message
        close(server_fd); //close the socket
        exit(EXIT_FAILURE); //exits the program with a known failure
    }

    // 3.) listen for the incoming connections
    int listen_result = listen(server_fd, 1);
    if (listen_result == -1) {
        perror("listen"); //print error message if listening fails
        close(server_fd); //close the socket
        exit(EXIT_FAILURE); //exits the porgram with a known failure
    }

    // 4.) accept an incoming connection
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    printf("Waiting for client to connect\n");
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd == -1) {
        perror("accept"); //print error message if accept fails
        close(server_fd); //will close the server socket
        exit(EXIT_FAILURE);
    }
    printf("Connected to client\n");

    // 5.) communicate with the client
    char buf[1024] = {0};
    ssize_t bytes_received = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (bytes_received == -1) {
        perror("recv"); //print the error message if receiving fails
    } else if (bytes_received == 0) {
        printf("Connection closed by client\n");
    } else {
        buf[bytes_received] = '\0';
        printf("Server received: %s\n", buf);
        send(client_fd, "world", strlen("world"), 0); //Send "world" back to the client
    }

    // 6.) close connections
    close(client_fd); //close the client socket
    close(server_fd); //close the server socket

    return 0; //return with a success
}
