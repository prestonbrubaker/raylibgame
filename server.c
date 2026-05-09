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
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        printf("New player connected!\n");

        // Send welcome
        char welcome[] = "WELCOME 1\n";
        send(client_fd, welcome, strlen(welcome), 0);

        // Keep connection alive and relay (very simple)
        while (1) {
            char buffer[256] = {0};
            int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) {
                printf("Player disconnected\n");
                close(client_fd);
                break;
            }
            // Echo back to same client for now (for testing)
            send(client_fd, buffer, bytes, 0);
        }
    }
    return 0;
}