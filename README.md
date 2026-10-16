# Astral Barrage

A C++17 desktop project using [raylib](https://github.com/raysan5/raylib) 6.0. A GLSL 330 fragment shader traces rays against a small 3D scene and renders stylized lighting and soft-edged shadows from a moving area light.

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

The build copies `assets/shaders/background.fs` beside the executable. Edit that file in the source tree and rebuild to update the effect. Shader errors are printed to the terminal and cause the program to exit. The current demonstration intersects two spheres and a ground plane. Its soft shadow uses a continuous sphere and light silhouette overlap approximation; it does not trace imported meshes or multiple light bounces.

The application passes `nullptr` for the vertex shader when loading `background.fs`, so raylib uses its default vertex shader to draw the full-screen rectangle. This fragment shader generates the demonstration scene; it is not a material shader for imported game models.

Build output and downloaded dependencies stay under `build/` and are ignored by Git.
