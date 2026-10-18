#include "raylib.h"
#include "rlgl.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;
constexpr int keyboardWidth = 256;
constexpr int keyboardHeight = 3;

struct Pass {
    Shader shader{};
    int resolution = -1;
    int time = -1;
    int delta = -1;
    int frame = -1;
    int mouse = -1;
    int channelResolution = -1;
    std::array<int, 4> channels{ -1, -1, -1, -1 };
};

Pass loadPass(const std::filesystem::path& directory, const char* name)
{
    const std::string file = (directory / name).string();
    if (!FileExists(file.c_str())) {
        std::cerr << "Shadertoy pass not found: " << file << '\n';
        return {};
    }

    Pass pass;
    pass.shader = LoadShader(nullptr, file.c_str());
    if (!IsShaderValid(pass.shader) || pass.shader.id == rlGetShaderIdDefault()) {
        std::cerr << "Failed to compile or link Shadertoy pass: " << file << '\n';
        return {};
    }

    pass.resolution = GetShaderLocation(pass.shader, "iResolution");
    pass.time = GetShaderLocation(pass.shader, "iTime");
    pass.delta = GetShaderLocation(pass.shader, "iTimeDelta");
    pass.frame = GetShaderLocation(pass.shader, "iFrame");
    pass.mouse = GetShaderLocation(pass.shader, "iMouse");
    pass.channelResolution = GetShaderLocation(pass.shader, "iChannelResolution[2]");
    for (int i = 0; i < 4; ++i) {
        pass.channels[i] = GetShaderLocation(pass.shader, ("iChannel" + std::to_string(i)).c_str());
    }
    if (pass.resolution < 0) {
        std::cerr << "Shadertoy pass requires iResolution: " << file << '\n';
        UnloadShader(pass.shader);
        return {};
    }
    return pass;
}

RenderTexture2D loadFloatTarget()
{
    RenderTexture2D target{};
    target.texture.width = screenWidth;
    target.texture.height = screenHeight;
    target.texture.mipmaps = 1;
    target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
    target.texture.id = rlLoadTexture(nullptr, screenWidth, screenHeight,
                                      RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
    if (target.texture.id == 0) {
        std::cerr << "Failed to create floating-point Shadertoy texture\n";
        return {};
    }

    rlTextureParameters(target.texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
    rlTextureParameters(target.texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
    rlTextureParameters(target.texture.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
    rlTextureParameters(target.texture.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);

    target.id = rlLoadFramebuffer();
    if (target.id == 0) {
        std::cerr << "Failed to create Shadertoy framebuffer\n";
        rlUnloadTexture(target.texture.id);
        return {};
    }
    rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                        RL_ATTACHMENT_TEXTURE2D, 0);
    if (!rlFramebufferComplete(target.id)) {
        std::cerr << "Incomplete floating-point Shadertoy framebuffer\n";
        rlUnloadFramebuffer(target.id);
        rlUnloadTexture(target.texture.id);
        return {};
    }
    return target;
}

void unloadTarget(RenderTexture2D& target)
{
    if (target.id != 0) rlUnloadFramebuffer(target.id);
    if (target.texture.id != 0) rlUnloadTexture(target.texture.id);
    target = {};
}

void setKeyboardPixel(std::array<unsigned char, keyboardWidth*keyboardHeight*4>& pixels,
                      int key, int row, bool active)
{
    const int index = 4*(row*keyboardWidth + key);
    pixels[index] = active ? 255 : 0;
    pixels[index + 1] = pixels[index];
    pixels[index + 2] = pixels[index];
    pixels[index + 3] = 255;
}

void updateKeyboard(Texture2D texture)
{
    static const std::array<int, 8> keys{
        KEY_W, KEY_A, KEY_S, KEY_D, KEY_Q, KEY_E, KEY_R, KEY_F
    };
    static std::array<bool, keyboardWidth> toggled{};
    std::array<unsigned char, keyboardWidth*keyboardHeight*4> pixels{};
    for (int key : keys) {
        const bool pressed = IsKeyPressed(key);
        if (pressed) toggled[key] = !toggled[key];
        setKeyboardPixel(pixels, key, 0, IsKeyDown(key));
        setKeyboardPixel(pixels, key, 1, pressed);
        setKeyboardPixel(pixels, key, 2, toggled[key]);
    }
    UpdateTexture(texture, pixels.data());
}

void drawPass(const Pass& pass, const std::array<Texture2D, 4>& inputs,
              int frame, float elapsed, float delta, const float mouse[4],
              RenderTexture2D* target)
{
    if (target != nullptr) BeginTextureMode(*target);
    BeginShaderMode(pass.shader);

    const float resolution[3] = {
        static_cast<float>(screenWidth), static_cast<float>(screenHeight), 1.0f
    };
    SetShaderValue(pass.shader, pass.resolution, resolution, SHADER_UNIFORM_VEC3);
    if (pass.time >= 0) SetShaderValue(pass.shader, pass.time, &elapsed, SHADER_UNIFORM_FLOAT);
    if (pass.delta >= 0) SetShaderValue(pass.shader, pass.delta, &delta, SHADER_UNIFORM_FLOAT);
    if (pass.frame >= 0) SetShaderValue(pass.shader, pass.frame, &frame, SHADER_UNIFORM_INT);
    if (pass.mouse >= 0) SetShaderValue(pass.shader, pass.mouse, mouse, SHADER_UNIFORM_VEC4);
    if (pass.channelResolution >= 0) {
        const float size[3] = {
            static_cast<float>(inputs[2].width), static_cast<float>(inputs[2].height), 1.0f
        };
        SetShaderValue(pass.shader, pass.channelResolution, size, SHADER_UNIFORM_VEC3);
    }
    for (int i = 0; i < 4; ++i) {
        if (pass.channels[i] >= 0) {
            if (inputs[i].id == 0) {
                std::cerr << "Missing Shadertoy input channel " << i << '\n';
                std::exit(EXIT_FAILURE);
            }
            SetShaderValueTexture(pass.shader, pass.channels[i], inputs[i]);
        }
    }
    rlDisableColorBlend();
    DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
    EndShaderMode();
    rlEnableColorBlend();
    if (target != nullptr) EndTextureMode();
}

} // namespace

int main()
{
    InitWindow(screenWidth, screenHeight, "Astral Barrage");
    const std::filesystem::path directory =
        std::filesystem::path(GetApplicationDirectory()) / "assets" / "shaders" / "kerr_newman";

    std::array<Pass, 5> passes{
        loadPass(directory, "buffer_a.fs"),
        loadPass(directory, "buffer_b.fs"),
        loadPass(directory, "buffer_c.fs"),
        loadPass(directory, "buffer_d.fs"),
        loadPass(directory, "image.fs")
    };
    bool ready = true;
    for (const Pass& pass : passes) ready &= IsShaderValid(pass.shader) &&
                                           pass.shader.id != rlGetShaderIdDefault();

    std::array<RenderTexture2D, 6> targets{};
    if (ready) {
        for (RenderTexture2D& target : targets) {
            target = loadFloatTarget();
            ready &= target.id != 0;
            if (!ready) break;
        }
    }

    Texture2D keyboard{};
    if (ready) {
        Image image = GenImageColor(keyboardWidth, keyboardHeight, BLACK);
        keyboard = LoadTextureFromImage(image);
        UnloadImage(image);
        ready = keyboard.id != 0;
        if (!ready) std::cerr << "Failed to create Shadertoy keyboard texture\n";
    }

    if (ready) {
        BeginDrawing();
        for (RenderTexture2D& target : targets) {
            BeginTextureMode(target);
            ClearBackground(BLANK);
            EndTextureMode();
        }
        EndDrawing();

        SetTargetFPS(60);
        const float startTime = static_cast<float>(GetTime());
        float lastTime = startTime;
        float clickX = 0.0f;
        float clickY = 0.0f;
        int frame = 0;

        while (!WindowShouldClose()) {
            const float now = static_cast<float>(GetTime());
            const float elapsed = now - startTime;
            const float delta = now - lastTime;
            lastTime = now;
            updateKeyboard(keyboard);

            const Vector2 cursor = GetMousePosition();
            const float x = cursor.x;
            const float y = static_cast<float>(screenHeight) - cursor.y;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                clickX = x;
                clickY = y;
            }
            const bool mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            const float mouse[4] = {
                mouseDown ? x : 0.0f, mouseDown ? y : 0.0f,
                mouseDown ? clickX : -clickX, mouseDown ? clickY : -clickY
            };

            const int read = frame % 2;
            const int write = 1 - read;
            const Texture2D aPrevious = targets[read].texture;
            const Texture2D bPrevious = targets[2 + read].texture;
            const Texture2D aCurrent = targets[write].texture;
            const Texture2D bCurrent = targets[2 + write].texture;
            const Texture2D cCurrent = targets[4].texture;
            const Texture2D dCurrent = targets[5].texture;

            BeginDrawing();
            ClearBackground(BLACK);
            drawPass(passes[0], { keyboard, {}, bPrevious, aPrevious },
                     frame, elapsed, delta, mouse, &targets[write]);
            drawPass(passes[1], { aCurrent, bPrevious, {}, keyboard },
                     frame, elapsed, delta, mouse, &targets[2 + write]);
            drawPass(passes[2], { bCurrent, {}, {}, {} },
                     frame, elapsed, delta, mouse, &targets[4]);
            drawPass(passes[3], { cCurrent, {}, {}, {} },
                     frame, elapsed, delta, mouse, &targets[5]);
            drawPass(passes[4], { aCurrent, bCurrent, cCurrent, dCurrent },
                     frame, elapsed, delta, mouse, nullptr);
            DrawText("Astral Barrage", 40, 40, 32, RAYWHITE);
            EndDrawing();
            ++frame;
        }
    }

    if (keyboard.id != 0) UnloadTexture(keyboard);
    for (RenderTexture2D& target : targets) unloadTarget(target);
    for (Pass& pass : passes) {
        if (IsShaderValid(pass.shader) && pass.shader.id != rlGetShaderIdDefault()) {
            UnloadShader(pass.shader);
        }
    }
    CloseWindow();
    return ready ? EXIT_SUCCESS : EXIT_FAILURE;
}
