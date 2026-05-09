#include <math.h>
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define SERVER_URL "http://willwill.immenseaccumulationonline.online:8080/update"
#define MAX_OTHER_PLAYERS 16

typedef struct {
    int id;
    float x, y, z;
} Player;

int main(void) {
    InitWindow(1280, 720, "Multiplayer 3D Environment - raylib (HTTP)");
    SetTargetFPS(60);

    Camera3D camera = {0};
    camera.position = (Vector3){0.0f, 15.0f, 15.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float yaw = 0.0f;
    float pitch = 0.0f;

    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to initialize curl\n");
        return 1;
    }

    int my_id = 1;
    Player local = {my_id, 0.0f, 0.0f, 0.0f};
    Player others[MAX_OTHER_PLAYERS];
    int num_others = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float rotSpeed = 120.0f * dt;
        float moveSpeed = 12.0f * dt;

        // Arrow keys rotate camera
        if (IsKeyDown(KEY_LEFT))  yaw -= rotSpeed;
        if (IsKeyDown(KEY_RIGHT)) yaw += rotSpeed;
        if (IsKeyDown(KEY_UP))    pitch -= rotSpeed * 0.7f;
        if (IsKeyDown(KEY_DOWN))  pitch += rotSpeed * 0.7f;

        if (pitch > 85.0f) pitch = 85.0f;
        if (pitch < -85.0f) pitch = -85.0f;

        // WASD movement
        float moveX = 0.0f, moveZ = 0.0f;
        if (IsKeyDown(KEY_W)) { moveX += sinf(yaw * DEG2RAD); moveZ += cosf(yaw * DEG2RAD); }
        if (IsKeyDown(KEY_S)) { moveX -= sinf(yaw * DEG2RAD); moveZ -= cosf(yaw * DEG2RAD); }
        if (IsKeyDown(KEY_A)) { moveX += sinf((yaw - 90.0f) * DEG2RAD); moveZ += cosf((yaw - 90.0f) * DEG2RAD); }
        if (IsKeyDown(KEY_D)) { moveX += sinf((yaw + 90.0f) * DEG2RAD); moveZ += cosf((yaw + 90.0f) * DEG2RAD); }

        local.x += moveX * moveSpeed;
        local.z += moveZ * moveSpeed;

        // === Send position via HTTP ===
        char post_data[256];
        sprintf(post_data, "{\"id\":%d,\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}", 
                my_id, local.x, local.y, local.z);

        curl_easy_setopt(curl, CURLOPT_URL, SERVER_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data));
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("HTTP Error: %s\n", curl_easy_strerror(res));
        }

        // Camera follows player
        float distance = 10.0f;
        float camX = local.x - sinf(yaw * DEG2RAD) * distance;
        float camZ = local.z - cosf(yaw * DEG2RAD) * distance;
        float camY = local.y + 6.0f + sinf(pitch * DEG2RAD) * 4.0f;

        camera.position = (Vector3){camX, camY, camZ};
        camera.target = (Vector3){local.x, local.y + 2.5f, local.z};

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

            // Black & White Checkerboard
            for (int x = -20; x <= 20; x++) {
                for (int z = -20; z <= 20; z++) {
                    Color tileColor = ((x + z) % 2 == 0) ? BLACK : WHITE;
                    DrawPlane((Vector3){x * 2.0f, 0.0f, z * 2.0f}, (Vector2){2.0f, 2.0f}, tileColor);
                }
            }

            DrawCube((Vector3){local.x, 1.5f, local.z}, 1.5f, 3.0f, 1.5f, BLUE);
            DrawCubeWires((Vector3){local.x, 1.5f, local.z}, 1.5f, 3.0f, 1.5f, DARKBLUE);

        EndMode3D();

        DrawText(TextFormat("Player %d (HTTP Mode)", my_id), 10, 10, 20, WHITE);
        DrawText("WASD = Move | Arrow Keys = Look Around", 10, 40, 16, WHITE);
        DrawText(TextFormat("Connected to: %s", SERVER_URL), 10, 70, 16, WHITE);

        EndDrawing();
    }

    curl_easy_cleanup(curl);
    CloseWindow();
    return 0;
}