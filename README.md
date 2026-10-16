# Astral Barrage

A C++17 desktop project using [raylib](https://github.com/raysan5/raylib) 6.0. It loads a GLSL 330 fragment shader from a file and displays an animated full-screen background.

## Requirements

- CMake 3.24 or newer
- Visual Studio 2022 with the Desktop development with C++ workload
- Network access for the first CMake configure, which downloads raylib 6.0 into `build/_deps/`

## Build and run on Windows

From the project directory in PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
.\build\Debug\AstralBarrage.exe
```

The program opens a 1280 × 720 window. Close the window or press Escape to exit.

The build copies `assets/shaders/background.fs` beside the executable. Edit that file in the source tree and rebuild to update the effect. Shader errors are printed to the terminal and cause the program to exit.

Build output and downloaded dependencies stay under `build/` and are ignored by Git.
