#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int server_sockfd, client_sockfd, maxfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // create a TCP socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind socket to server address
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // listen for connections
    if (listen(server_sockfd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }

    // initialize pollfd array for server and clients
    struct pollfd pollfds[MAX_CLIENTS + 1];
    memset(pollfds, 0, sizeof(pollfds));
    pollfds[0].fd = server_sockfd;
    pollfds[0].events = POLLIN;
    maxfd = 0;

    printf("Server started on port %d\n", atoi(argv[1]));

    while (1) {
        int ready = poll(pollfds, maxfd + 1, -1);
        if (ready < 0) {
            perror("poll");
            exit(1);
        }

        // check for incoming connection requests
        if (pollfds[0].revents & POLLIN) {
            client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
            if (client_sockfd < 0) {
                perror("accept");
                continue;
            }
            printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // add client socket to pollfd array
            int i;
            for (i = 1; i < MAX_CLIENTS + 1; i++) {
                if (pollfds[i].fd == 0) {
                    pollfds[i].fd = client_sockfd;
                    pollfds[i].events = POLLIN;
                    break;
                }
            }
            if (i == MAX_CLIENTS + 1) {
                fprintf(stderr, "Too many clients\n");
                close(client_sockfd);
            }

            if (i > maxfd) {
                maxfd = i;
            }
        }

        // check for data from clients
        int i;
        for (i = 1; i < maxfd + 1; i++) {
            if (pollfds[i].revents & POLLIN) {
                int n = read(pollfds[i].fd, buffer, BUFFER_SIZE);
                if (n <= 0) {
                    printf("Client disconnected\n");
                    // remove client socket from pollfd array
                    close(pollfds[i].fd);
                    pollfds[i].fd = 0;
                } else {
                    buffer[n] = '\0';
                    printf("Received from client: %s\n", buffer);
                }
            }
        }
    }

    return 0;
}
