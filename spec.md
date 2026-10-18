# Astral Barrage 规格

本文件记录已经确认并实现的行为。新玩法应由用户明确提出。

## 当前行为

- 打开标题为 `Astral Barrage` 的 1280 × 720 桌面窗口，目标帧率设为 60 FPS。
- 以 [ShaderToy wXdfzj](https://www.shadertoy.com/view/wXdfzj) 的 2026-02-10
  源码快照为背景，按 Buffer A、B、C、D、Image 顺序渲染 Kerr–Newman
  黑洞、吸积盘、星空及 Bloom；标题绘制在背景上方。
- 为原 Shader 提供时间、帧号、鼠标、键盘纹理和通道输入。Buffer A、B
  使用浮点双缓冲保留跨帧状态；鼠标拖动及 W/A/S/D、Q/E、R/F 传入原相机控制。
- Shader 文件缺失、编译或链接失败，以及浮点渲染目标创建失败时报告错误并退出。
- 关闭窗口或按 Escape 后正常退出。

## 技术约束与已知限制

- 使用 C++17、CMake 3.24+、raylib 6.0 和 GLSL 330。Windows 使用
  Visual Studio 2022/MSVC，依赖与构建产物留在 `build/`。
- 原 Shader 的运算量很高。AMD Radeon 610M 上 1280 × 720 Debug
  实测约 2 FPS，不能把 60 FPS 的目标帧率当成实际性能。
- 当前源码来自可追溯的历史快照，可能与 ShaderToy 页面后续修订有差异；
  来源与再分发权利见 `assets/shaders/kerr_newman/SOURCE.md`。
- 尚未定义玩家、弹幕、关卡、场景切换或存档行为。
