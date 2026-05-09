#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 9099
#define MAX_CLIENTS 16

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

    int clients[MAX_CLIENTS];
    int num_clients = 0;

    while (1) {
        // Accept new clients
        int new_client = accept(server_fd, NULL, NULL);
        if (new_client >= 0 && num_clients < MAX_CLIENTS) {
            clients[num_clients] = new_client;
            char welcome[32];
            sprintf(welcome, "WELCOME %d\n", num_clients + 1);
            send(new_client, welcome, strlen(welcome), 0);
            printf("Player %d connected\n", num_clients + 1);
            num_clients++;
        }

        // Relay messages between clients
        for (int i = 0; i < num_clients; i++) {
            char buffer[256] = {0};
            int bytes = recv(clients[i], buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
            if (bytes > 0) {
                for (int j = 0; j < num_clients; j++) {
                    if (i != j) {
                        send(clients[j], buffer, bytes, 0);
                    }
                }
            }
        }
    }
    return 0;
}