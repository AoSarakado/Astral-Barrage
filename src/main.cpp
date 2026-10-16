#include "raylib.h"

int main() {
    constexpr int screenWidth = 1280;
    constexpr int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Astral Barrage");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Astral Barrage", 40, 40, 32, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
