# Pet_catBug

一个用 Qt/C++ 实现的宠物动画程序，按帧播放预先准备的图片序列来展示动作，并支持两种运行模式：待机交替（StayingA ⇄ StayingB）和“大循环”按序列循环动作。

## 特性

- 多动作逐帧动画（图片资源位于资源文件 `:/image/`）。
- 启动后默认在 `StayingA` 和 `StayingB` 之间交替显示。
- 右键菜单提供手动切换动作和可勾选的 `BigLoop`：勾选后按下面指定的顺序循环动作并与当前状态衔接。
- 所有动作的帧序列路径配置在 `widget.cpp` 的 `loadRoleActRes()` 中。

## 目录结构（简要）

- `widget.h` / `widget.cpp`：核心窗口与逻辑实现。
- `image/`：项目资源帧（已打包到 qrc）。
- `CMakeLists.txt`：构建脚本。

## 构建与运行

要求：Qt6 + CMake（Windows 下使用 MinGW 或 MSVC 均可）。

在 VS Code 中可以直接使用已有的任务（`C/C++: g++.exe 生成活动文件`），或使用 CMake 进行构建：

```powershell
mkdir build
cd build
cmake ..
cmake --build .
# 运行可执行文件
.\Pet_catBug.exe
```

## 使用说明

- 右键点击窗口打开菜单：可以选择切换到任意动作（会停止大循环）或勾选 `BigLoop` 开启按序列循环。
- 默认大循环顺序在 `widget.cpp` 构造函数中定义为：
  StayingA → Stay_WriteA → WriteA → Write_SleepA → SleepA → SleepB → Write_SleepB → WriteB → Stay_WriteB → StayingB
- 启用 `BigLoop` 后程序会尽量从当前显示的动作继续向序列后的动作衔接，避免突兀跳转。

## 可配置项与二次开发提示

- 每帧间隔：修改 `widget.h` 中 `frameIntervalMs` 的默认值（当前 120 毫秒），或在运行时修改以改变动画速度。
- 资源路径与帧数：`widget.cpp` 中 `loadRoleActRes()` 使用 `addResA/addResB` 函数按编号载入图片，默认路径模板为 `:/image/%d.png`。
- 修改大循环顺序：编辑 `widget.cpp` 构造函数中的 `bigLoopSequence` 列表。
- 性能优化：当前实现按需用 `QPixmap::load` 在绘制时加载图片；如果机器性能允许，可改为预加载并缓存常用动作的 `QPixmap`，减少 I/O 开销。

## 调试与常见问题

- 如果窗口未显示图片，检查资源是否正确打包到 qrc（`image.qrc`）并且路径正确。
- 若发现动画卡顿，可尝试减小 `frameIntervalMs` 或减少图片分辨率/数量。

---
##主要参考文献
https://doc.qt.io/qt-6/resources.html 据此解决了Qt+CMake加载不了qrc的问题
https://www.bilibili.com/video/BV1paeneGEd9/?spm_id_from=333.337.search-card.all.click&vd_source=982f5b0d4fe15aac09b82e6ffcedbe98  大力参考此视频制作了代码主干
