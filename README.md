# Astral Barrage

A C++17 desktop project using [raylib](https://github.com/raysan5/raylib) 6.0.
The current window renders a Kerr–Newman black hole background from the
five-pass [ShaderToy work](https://www.shadertoy.com/view/wXdfzj), with the
`Astral Barrage` title drawn above it.

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

The build copies `assets/shaders/kerr_newman/` beside the executable. Close
the window or press Escape to exit. Mouse dragging and W/A/S/D, Q/E, and R/F
are passed to the ShaderToy camera controls.

The shader runtime executes Buffer A, B, C, D, and Image in order. A and B
have floating-point feedback buffers. It supplies ShaderToy resolution, time,
frame, mouse, keyboard-texture, and channel inputs. Shader compilation and
render-target failures are reported rather than replaced with another effect.

This is a computationally expensive shader. On the tested AMD Radeon 610M,
1280 × 720 Debug rendering took about 3.9 seconds for eight frames, roughly
2 FPS. The 60 FPS target is a cap, not a measured frame rate for this effect.
See [source and rights notes](assets/shaders/kerr_newman/SOURCE.md) before
redistributing the shader. Build output and downloaded dependencies stay under
`build/` and are ignored by Git.
