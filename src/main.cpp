#include "raylib.h"
#include "rlgl.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

int main() {
    constexpr int screenWidth = 1280;
    constexpr int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Astral Barrage");

    const std::filesystem::path shaderPath =
        std::filesystem::path(GetApplicationDirectory()) / "assets" / "shaders" / "background.fs";
    const std::string shaderFile = shaderPath.string();
    if (!FileExists(shaderFile.c_str())) {
        std::cerr << "GLSL fragment shader not found: " << shaderFile << '\n';
        CloseWindow();
        return EXIT_FAILURE;
    }

    Shader shader = LoadShader(nullptr, shaderFile.c_str());
    if (!IsShaderValid(shader) || shader.id == rlGetShaderIdDefault()) {
        std::cerr << "Failed to compile or link GLSL fragment shader: " << shaderFile << '\n';
        UnloadShader(shader);
        CloseWindow();
        return EXIT_FAILURE;
    }

    const int resolutionLocation = GetShaderLocation(shader, "resolution");
    const int timeLocation = GetShaderLocation(shader, "time");
    if (resolutionLocation < 0 || timeLocation < 0) {
        std::cerr << "GLSL fragment shader requires resolution and time uniforms: " << shaderFile << '\n';
        UnloadShader(shader);
        CloseWindow();
        return EXIT_FAILURE;
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        const float resolution[2] = {
            static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight())
        };
        const float time = static_cast<float>(GetTime());
        SetShaderValue(shader, resolutionLocation, resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, timeLocation, &time, SHADER_UNIFORM_FLOAT);

        BeginDrawing();
        ClearBackground(BLACK);
        BeginShaderMode(shader);
        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
        EndShaderMode();
        DrawText("Astral Barrage", 40, 40, 32, RAYWHITE);
        EndDrawing();
    }

    UnloadShader(shader);
    CloseWindow();
    return 0;
}
