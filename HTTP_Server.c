#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

void *connection_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024] = {0};
    char url[1024], method[16], protocol[16];

    printf("About to read.\n");

    int req_read = read(sock, buffer, 1024);
    if (req_read < 0) {
        perror("Error! Read failure!");
        close(sock);
        free(socket_desc);
        return NULL;
    }

    sscanf(buffer, "%15s %1023s %15s", method, url, protocol);

    printf("Parsed: Method: %s | URL: %s | Protocol: %s\n", method, url, protocol);

    char body[2048];
    snprintf(body, sizeof(body),
        "Method: %s\nURL: %s\nProtocol: %s\n\nHello, world!",
        method, url, protocol
    );

    int body_length = strlen(body);

    char response[4096];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r"
        "\r"
        "%s",
        body_length, body
    );

    write(sock, response, strlen(response));
    printf("Response sent\n");

    close(sock);
    free(socket_desc);
    return NULL;
}


int main() {

    printf("Main");

    int server_fd, new_socket, *new_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_t thread;
        new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

	printf("Calling the connection handler");

        if (pthread_create(&thread, NULL, connection_handler, (void*)new_sock) < 0) {
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }

        // Detach the thread so it doesn't need to be joined
        pthread_detach(thread);
    }

    return 0;
}

