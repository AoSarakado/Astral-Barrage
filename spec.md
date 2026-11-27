# Astral Barrage 规格

本文件记录已经确认并实现的行为。新玩法应由用户明确提出。

## 当前行为

- 打开标题为 `Astral Barrage` 的 1280 × 720 桌面窗口，目标帧率设为 60 FPS。
- 将 [ShaderToy wXdfzj](https://www.shadertoy.com/view/wXdfzj) 的 2026-02-10
  源码快照离线渲染为 1280 × 720、24 FPS、4 秒的 Kerr–Newman 黑洞动画；
  程序循环播放该动画，标题绘制在背景上方，背景相机固定。
- 游戏运行时在独立线程解码视频，在绘制线程上传新帧；鼠标和键盘不再控制背景。
- 背景视频缺失、无效或首帧解码失败时报告错误并退出。
- 随项目提供离线渲染工具，按 Buffer A、B、C、D、Image 顺序渲染，
  使用 A/B 浮点反馈、固定相机及固定时间步，导出供 FFmpeg 编码的 PNG 帧。
- 关闭窗口或按 Escape 后正常退出。

## 技术约束与已知限制

- 使用 C++17、CMake 3.24+、raylib 6.0、pl_mpeg 和离线 GLSL 330。Windows 使用
  Visual Studio 2022/MSVC，依赖与构建产物留在 `build/`。
- 原 Shader 的运算量很高。AMD Radeon 610M 上 1280 × 720 Debug
  实测约 2 FPS，因此仅用于离线生成；预渲染循环可能在片段边界出现跳变。
- 当前源码来自可追溯的历史快照，可能与 ShaderToy 页面后续修订有差异；
  来源与再分发权利见 `assets/shaders/kerr_newman/SOURCE.md`。
- 尚未定义玩家、弹幕、关卡、场景切换或存档行为。
