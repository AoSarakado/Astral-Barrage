#include "game.h"
#include "raylib.h"

#include <cstdio>
#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace {

constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;

struct VideoFrames {
    std::mutex mutex;
    std::vector<unsigned char> decoding;
    std::vector<unsigned char> pending;
    std::vector<unsigned char> uploading;
    bool pendingReady = false;
};

void receiveFrame(plm_t*, plm_frame_t* frame, void* user)
{
    auto& frames = *static_cast<VideoFrames*>(user);
    plm_frame_to_rgb(frame, frames.decoding.data(), frame->width * 3);
    {
        std::lock_guard<std::mutex> lock(frames.mutex);
        frames.decoding.swap(frames.pending);
        frames.pendingReady = true;
    }
}

bool uploadPendingFrame(VideoFrames& frames, Texture2D texture)
{
    {
        std::lock_guard<std::mutex> lock(frames.mutex);
        if (!frames.pendingReady) return false;
        frames.uploading.swap(frames.pending);
        frames.pendingReady = false;
    }
    UpdateTexture(texture, frames.uploading.data());
    return true;
}

} // namespace

int main()
{
    InitWindow(screenWidth, screenHeight, "Astral Barrage");

    const std::filesystem::path videoPath =
        std::filesystem::path(GetApplicationDirectory()) / "assets" / "background" /
        "kerr_newman.mpg";
    const std::string videoFile = videoPath.string();
    if (!FileExists(videoFile.c_str())) {
        std::cerr << "Background video not found: " << videoFile << '\n';
        CloseWindow();
        return EXIT_FAILURE;
    }

    plm_t* player = plm_create_with_filename(videoFile.c_str());
    if (player == nullptr || plm_get_num_video_streams(player) != 1 ||
        plm_get_width(player) != screenWidth || plm_get_height(player) != screenHeight ||
        plm_get_framerate(player) <= 0.0) {
        std::cerr << "Invalid background video: " << videoFile << '\n';
        if (player != nullptr) plm_destroy(player);
        CloseWindow();
        return EXIT_FAILURE;
    }

    plm_set_audio_enabled(player, 0);
    plm_set_loop(player, 1);

    VideoFrames frames;
    const size_t bytesPerFrame = static_cast<size_t>(screenWidth) * screenHeight * 3;
    frames.decoding.resize(bytesPerFrame);
    frames.pending.resize(bytesPerFrame);
    frames.uploading.resize(bytesPerFrame);

    Image blank = GenImageColor(screenWidth, screenHeight, BLACK);
    ImageFormat(&blank, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    Texture2D background = LoadTextureFromImage(blank);
    UnloadImage(blank);
    if (background.id == 0) {
        std::cerr << "Failed to create background video texture\n";
        plm_destroy(player);
        CloseWindow();
        return EXIT_FAILURE;
    }

    const double frameSeconds = 1.0 / plm_get_framerate(player);
    plm_set_video_decode_callback(player, receiveFrame, &frames);
    plm_decode(player, frameSeconds);
    if (!uploadPendingFrame(frames, background)) {
        std::cerr << "Failed to decode background video: " << videoFile << '\n';
        UnloadTexture(background);
        plm_destroy(player);
        CloseWindow();
        return EXIT_FAILURE;
    }

    std::atomic<bool> running{ true };
    std::thread decoder([&] {
        const auto interval = std::chrono::duration<double>(frameSeconds);
        auto nextFrame = std::chrono::steady_clock::now() + interval;
        while (running.load()) {
            std::this_thread::sleep_until(nextFrame);
            if (!running.load()) break;
            plm_decode(player, frameSeconds);
            nextFrame += std::chrono::duration_cast<std::chrono::steady_clock::duration>(interval);
        }
    });

    Game game(screenWidth, screenHeight);
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        game.update();
        uploadPendingFrame(frames, background);
        BeginDrawing();
        DrawTexture(background, 0, 0, WHITE);
        game.draw();
        EndDrawing();
    }

    running = false;
    decoder.join();
    UnloadTexture(background);
    plm_destroy(player);
    CloseWindow();
    return EXIT_SUCCESS;
}
