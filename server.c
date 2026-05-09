#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>     // ← ADDED
#include <sys/time.h>       // ← ADDED
#endif

#define PORT 9099
#define MAX_CLIENTS 16

typedef struct {
    uint8_t type;     // 1 = welcome, 2 = player update
    int32_t player_id;
    float x, y, z;
} NetMsg;

int main(void) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    listen(server_fd, 5);
    printf("Server listening on port %d...\n", PORT);

    int client_sockets[MAX_CLIENTS];
    int player_ids[MAX_CLIENTS];
    float player_pos[MAX_CLIENTS][3];
    int num_clients = 0;
    int next_player_id = 1;

    for (int i = 0; i < MAX_CLIENTS; i++) client_sockets[i] = -1;

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_fd = server_fd;

        for (int i = 0; i < num_clients; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &readfds);
                if (client_sockets[i] > max_fd) max_fd = client_sockets[i];
            }
        }

        struct timeval tv = {0, 50000}; // 50ms timeout
        int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        if (activity < 0) continue;

        // New connection
        if (FD_ISSET(server_fd, &readfds)) {
            int new_sock = accept(server_fd, NULL, NULL);
            if (num_clients < MAX_CLIENTS) {
                client_sockets[num_clients] = new_sock;
                player_ids[num_clients] = next_player_id++;
                player_pos[num_clients][0] = player_pos[num_clients][1] = player_pos[num_clients][2] = 0.0f;

                NetMsg welcome = {1, player_ids[num_clients], 0, 0, 0};
                send(new_sock, &welcome, sizeof(welcome), 0);

                printf("Player %d connected (total: %d)\n", player_ids[num_clients], num_clients + 1);
                num_clients++;
            } else {
#ifdef _WIN32
                closesocket(new_sock);
#else
                close(new_sock);
#endif
            }
        }

        // Handle client messages
        for (int i = 0; i < num_clients; i++) {
            int sock = client_sockets[i];
            if (sock == -1 || !FD_ISSET(sock, &readfds)) continue;

            NetMsg msg;
            int bytes = recv(sock, &msg, sizeof(msg), 0);
            if (bytes <= 0) {
                printf("Player %d disconnected\n", player_ids[i]);
#ifdef _WIN32
                closesocket(sock);
#else
                close(sock);
#endif
                // Swap-remove
                client_sockets[i] = client_sockets[num_clients - 1];
                player_ids[i] = player_ids[num_clients - 1];
                player_pos[i][0] = player_pos[num_clients - 1][0];
                player_pos[i][1] = player_pos[num_clients - 1][1];
                player_pos[i][2] = player_pos[num_clients - 1][2];
                num_clients--;
                i--;
                continue;
            }

            if (msg.type == 2) {
                player_pos[i][0] = msg.x;
                player_pos[i][1] = msg.y;
                player_pos[i][2] = msg.z;

                // Broadcast to everyone
                for (int j = 0; j < num_clients; j++) {
                    if (client_sockets[j] != -1) {
                        send(client_sockets[j], &msg, sizeof(msg), 0);
                    }
                }
            }
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
