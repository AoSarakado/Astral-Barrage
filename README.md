# Astral Barrage

A single-screen desktop bullet-hell game built with C++17 and
[raylib](https://github.com/raysan5/raylib) 6.0. The fight plays over a looping
Kerr–Newman black hole animation from the five-pass
[ShaderToy work](https://www.shadertoy.com/view/wXdfzj). Runtime playback uses
[pl_mpeg](https://github.com/phoboslab/pl_mpeg) at commit
`c871f2be022ece7ef4f64230b4fb8e1fb9eb6023` (MIT license).

The overall product definition is pending. See the [project brief template](docs/PROJECT_BRIEF.md) for the decisions that still need confirmation.

## Requirements

- CMake 3.24 or newer
- Visual Studio 2022 with the Desktop development with C++ workload
- Network access for the first CMake configure, which downloads raylib 6.0

## Build and run on Windows

From the project directory in PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
.\build\Debug\AstralBarrage.exe
```

The build copies `assets/background/kerr_newman.mpg` beside the executable.
Press `A` / `D` or the left and right arrow keys to move along the lane. Hold
`Space` to fire cyan bolts to the right while dodging orange enemy rounds.
Destroy the enemy core to win. Press `R` after a win or loss to restart, or
press `Escape` to exit.

The video is 1280 × 720, 24 FPS, four seconds long, and loops without audio.
The runtime decodes it on a worker thread, uploads each new frame to a raylib
texture, and draws the title over it. Missing or invalid video files cause an
explicit error and nonzero exit code.

## Regenerate the background

The `AstralBarrageRenderBackground` executable keeps the original five-pass
shader pipeline as an offline renderer. It exports 96 PNG frames after six
warm-up frames with a fixed camera and 1/24-second steps. FFmpeg is needed only
to encode a replacement video, not to build or play the game:

```powershell
cmake --build build --config Debug --target AstralBarrageRenderBackground copy_shaders
.\build\Debug\AstralBarrageRenderBackground.exe .\build\background_frames
ffmpeg -framerate 24 -i build/background_frames/frame_%04d.png -an -c:v mpeg1video -q:v 2 -pix_fmt yuv420p -y assets/background/kerr_newman.mpg
cmake --build build --config Debug
```

The original shader rendered at roughly 2 FPS on an AMD Radeon 610M in Debug.
The pre-rendered animation removes that shader workload from game runtime.
See [source and rights notes](assets/shaders/kerr_newman/SOURCE.md) before
redistributing either the shader or the derived video. Build output and
downloaded dependencies stay under `build/` and are ignored by Git.
