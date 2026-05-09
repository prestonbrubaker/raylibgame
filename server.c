#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9099

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);
    printf("HTTP Server listening on port %d...\n", PORT);

    while (1) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) continue;

        char buffer[4096] = {0};
        read(client, buffer, sizeof(buffer) - 1);

        printf("\n=== Received HTTP Request ===\n%s\n", buffer);

        // Very basic response
        char response[] = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n"
            "Position received\n";

        write(client, response, strlen(response));
        close(client);
    }
    return 0;
}