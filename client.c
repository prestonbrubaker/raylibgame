#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

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
#include <netdb.h>
#endif

#define SERVER_HOST "willwill.immenseaccumulationonline.online"
#define PORT 8080
#define MAX_OTHER_PLAYERS 16

typedef struct {
    int id;
    float x, y, z;
} Player;

int main(void) {
    InitWindow(1280, 720, "Multiplayer 3D Environment - raylib");
    SetTargetFPS(60);

    Camera3D camera = {0};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float yaw = 0.0f;
    float pitch = 0.0f;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    int sock = -1;
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    struct hostent *host = gethostbyname(SERVER_HOST);
    if (host != NULL) {
        memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
            printf("Connected to %s!\n", SERVER_HOST);
        } else {
            printf("Failed to connect to server\n");
            sock = -1;
        }
    } else {
        printf("Could not resolve %s\n", SERVER_HOST);
    }

    int my_id = 0;
    if (sock != -1) {
        char buffer[128] = {0};
        recv(sock, buffer, sizeof(buffer), 0);
        sscanf(buffer, "WELCOME %d", &my_id);
        printf("You are Player %d\n", my_id);

#ifdef _WIN32
        u_long mode = 1; ioctlsocket(sock, FIONBIO, &mode);
#else
        fcntl(sock, F_SETFL, O_NONBLOCK);
#endif
    }

    Player local = {my_id, 0.0f, 0.0f, 0.0f};
    Player others[MAX_OTHER_PLAYERS];
    int num_others = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float rotSpeed = 120.0f * dt;
        float moveSpeed = 12.0f * dt;

        if (IsKeyDown(KEY_LEFT))  yaw -= rotSpeed;
        if (IsKeyDown(KEY_RIGHT)) yaw += rotSpeed;
        if (IsKeyDown(KEY_UP))    pitch -= rotSpeed * 0.7f;
        if (IsKeyDown(KEY_DOWN))  pitch += rotSpeed * 0.7f;

        if (pitch > 85.0f) pitch = 85.0f;
        if (pitch < -85.0f) pitch = -85.0f;

        float moveX = 0.0f, moveZ = 0.0f;
        if (IsKeyDown(KEY_W)) { moveX += sinf(yaw * DEG2RAD); moveZ += cosf(yaw * DEG2RAD); }
        if (IsKeyDown(KEY_S)) { moveX -= sinf(yaw * DEG2RAD); moveZ -= cosf(yaw * DEG2RAD); }
        if (IsKeyDown(KEY_A)) { moveX += sinf((yaw - 90.0f) * DEG2RAD); moveZ += cosf((yaw - 90.0f) * DEG2RAD); }
        if (IsKeyDown(KEY_D)) { moveX += sinf((yaw + 90.0f) * DEG2RAD); moveZ += cosf((yaw + 90.0f) * DEG2RAD); }

        local.x += moveX * moveSpeed;
        local.z += moveZ * moveSpeed;

        if (sock != -1) {
            char msg[128];
            sprintf(msg, "POS %d %.2f %.2f %.2f\n", my_id, local.x, local.y, local.z);
            send(sock, msg, strlen(msg), 0);
        }

        if (sock != -1) {
            char buffer[256] = {0};
            int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                int id; float x, y, z;
                if (sscanf(buffer, "POS %d %f %f %f", &id, &x, &y, &z) == 4 && id != my_id) {
                    bool found = false;
                    for (int i = 0; i < num_others; i++) {
                        if (others[i].id == id) {
                            others[i].x = x; others[i].y = y; others[i].z = z;
                            found = true; break;
                        }
                    }
                    if (!found && num_others < MAX_OTHER_PLAYERS) {
                        others[num_others].id = id;
                        others[num_others].x = x; others[num_others].y = y; others[num_others].z = z;
                        num_others++;
                    }
                }
            }
        }

        float distance = 10.0f;
        float camX = local.x - sinf(yaw * DEG2RAD) * distance;
        float camZ = local.z - cosf(yaw * DEG2RAD) * distance;
        float camY = local.y + 6.0f + sinf(pitch * DEG2RAD) * 4.0f;

        camera.position = (Vector3){camX, camY, camZ};
        camera.target = (Vector3){local.x, local.y + 2.5f, local.z};

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

        for (int x = -20; x <= 20; x++) {
            for (int z = -20; z <= 20; z++) {
                Color tileColor = ((x + z) % 2 == 0) ? BLACK : WHITE;
                DrawPlane((Vector3){x * 2.0f, 0.0f, z * 2.0f}, (Vector2){2.0f, 2.0f}, tileColor);
            }
        }

        DrawCube((Vector3){local.x, 1.5f, local.z}, 1.5f, 3.0f, 1.5f, BLUE);
        DrawCubeWires((Vector3){local.x, 1.5f, local.z}, 1.5f, 3.0f, 1.5f, DARKBLUE);

        for (int i = 0; i < num_others; i++) {
            Color c = (others[i].id % 3 == 0) ? RED : (others[i].id % 3 == 1) ? GREEN : ORANGE;
            DrawCube((Vector3){others[i].x, 1.5f, others[i].z}, 1.5f, 3.0f, 1.5f, c);
            DrawCubeWires((Vector3){others[i].x, 1.5f, others[i].z}, 1.5f, 3.0f, 1.5f, DARKGRAY);
        }

        DrawCube((Vector3){12, 4, 15}, 6, 8, 6, BROWN);
        DrawCube((Vector3){-14, 3, -12}, 5, 6, 5, DARKGREEN);

        EndMode3D();

        DrawText(TextFormat("Player %d | Others: %d", my_id, num_others), 10, 10, 20, WHITE);
        DrawText("WASD = Move | Arrow Keys = Look Around", 10, 40, 16, WHITE);
        DrawText(TextFormat("Connected to: %s:%d", SERVER_HOST, PORT), 10, 70, 16, WHITE);
        if (sock == -1) DrawText("OFFLINE", 10, 100, 20, RED);

        EndDrawing();
    }

    CloseWindow();
    if (sock != -1) {
#ifdef _WIN32
        closesocket(sock); WSACleanup();
#else
        close(sock);
#endif
    }
    return 0;
}