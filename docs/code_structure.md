# Code Structure — Presentation Guide

---

## 1. 项目文件夹结构图

```
FT245R/
│
├── ftd2xx.h / libftd2xx.a          ← FTDI 厂商驱动（不是我们写的）
│
├── controller.c                    ← legacy 菜单程序（已弃用）
├── LED_Project.c                   ← legacy LED 控制
├── morse_Project.c                 ← legacy 摩尔斯电码
│
├── ioLibrary/                      ← 核心：可复用的 C++ I/O 静态库
│   ├── ioFtdiDevice.h / .cpp       ←  USB 芯片遥控器
│   ├── ioBuffer.h / .cpp           ←  临时数据缓冲
│   ├── ioRead.h / .cpp             ←  定时读
│   ├── ioWrite.h / .cpp            ←  定时写
│   ├── ioCircularBuffer.h / .cpp   ←  线程安全环形缓冲区（传送带）
│   ├── ioThreadedReader.h / .cpp   ←  读线程（采集工人）
│   ├── ioThreadedWriter.h / .cpp   ←  写线程（打包工人）
│   └── ioScaleShiftPipeline.h/.cpp ←  加工线程（信号放大偏移）
│
├── ui/                             ← Qt 图形界面（MVC）
│   ├── ioOscilloscopeModel.h / .cpp       ← Model（后厨）
│   ├── ioMainWindow.h / .cpp              ← Controller（大管家）
│   ├── ioAbstractOscilloscopeView.h       ← View 接口（菜单模板）
│   ├── ioCompactOscilloscopeView.h / .cpp ← View A（紧凑布局）
│   ├── ioWorkspaceOscilloscopeView.h/.cpp ← View B（宽屏布局）
│   └── ioWaveformWidget.h / .cpp          ← 波形绘图控件
│
├── main.cpp                        ← LED 闪烁 demo 入口
├── pipeline.cpp                    ← CLI 多线程管道入口
├── qt_main.cpp                     ← Qt GUI 入口
├── Makefile                        ← CLI 编译（make）
└── CMakeLists.txt                  ← Qt 编译（cmake）
```

---

## 2. 架构总览图（展示用，一张 slide 放得下）

```
┌─────────────────────────────────────────────────────────────────────┐
│                        oscilloscope_qt (Qt GUI)                     │
│                                                                     │
│  ┌────────────┐    ┌───────────────────┐    ┌────────────────────┐  │
│  │ MainWindow │───▶│ OscilloscopeModel │    │        View        │  │
│  │            │    │                   │──▶ │Compact / Workspace │  │
│  └────────────┘    └────────┬──────────┘    │  + WaveformWidget  │  │
│                             │               └────────────────────┘  │
│                             │                                       |
├─────────────────────────────┼───────────────────────────────────────┤
│                   ioLibrary │ (static lib ioLibrary.a)              │
│                             ▼                                       │
│  ┌──────────┐   ┌────────┐   ┌────────────┐   ┌────────┐   ┌────┐   │
│  │ Threaded │──▶│  raw   │──▶│ ScaleShift │──▶│  proc  │──▶│Thrd│   │
│  │ Reader   │   │ Buffer │   │ Pipeline   │   │ Buffer │   │Wrtr│   │
│  └────┬─────┘   └────────┘   └────────────┘   └────────┘   └──┬─┘   │
│       │               Read ──→ Process ──→ Write              │     │
│       │                                                       │     │
│       ▼                                                       ▼     │
│  ┌─────────┐                                            ┌─────────┐ │
│  │ FtdiDev │                                            │ FtdiDev │ │
│  │         │                                            │         │ |
│  └────┬────┘                                            └────┬────┘ │
├───────┼──────────────────────────────────────────────────────┼─────-┤
│       │                         USB                          │      │
│       ▼                                                      ▼      │
│  ┌─────────┐                                            ┌─────────┐ │
│  │ FT245R  │                                            │   LED   │ │
│  │         │                                            │         │ │
│  └─────────┘                                            └─────────┘ │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. 三层架构分层图

```
┌─────────────────────────────┐
│   Presentation Layer (UI)   │  ← 用户看到的：按钮、旋钮、波形图
│   Qt Widgets + MVC          │     两套外观可切换
├─────────────────────────────┤
│   Business Logic Layer      │  ← 数据怎么加工：scale × b + shift
│   ioLibrary (Pipeline)      │     三线程并行：读→加工→写
├─────────────────────────────┤
│   Hardware Access Layer     │  ← 跟芯片打交道：FtdiDevice
│   FTDI D2XX driver          │     封装所有 FT_* 调用
└─────────────────────────────┘
```

---

## 4. 文件数量统计

| 层 | .h 文件 | .cpp 文件 | 类数量 | 说明 |
|---|---|---|---|---|
| ioLibrary | 8 | 8 | 8 | 每个类一个 .h + 一个 .cpp |
| ui | 6 | 5 | 6 | AbstractView 只有 .h（纯接口） |
| 入口文件 | — | 3 | — | main.cpp / pipeline.cpp / qt_main.cpp |
| **总计** | **14** | **16** | **14** | **一个类一个文件，无例外** |

---

## 5. 每个文件一句话说明（Presentation Bullet Points）

### ioLibrary — "工具箱"

- **ioFtdiDevice** — Wraps all USB chip operations (open / read / write / close) behind a mutex. One class talks to hardware, nothing else does.
- **ioBuffer** — A simple byte array on the heap. Created by the caller, passed in.
- **ioRead / ioWrite** — Single-threaded loops that read or write at a fixed frequency. Used by the blink demo.
- **ioCircularBuffer** — Thread-safe ring buffer. The "conveyor belt" between threads. Blocks when full (producer waits) or empty (consumer waits).
- **ioThreadedReader** — A dedicated thread that reads from the FTDI device and pushes bytes into the ring buffer. Producer.
- **ioThreadedWriter** — A dedicated thread that pulls bytes from the ring buffer and writes to a file or device. Consumer.
- **ioScaleShiftPipeline** — Sits between two ring buffers. Applies `round(scale × byte + shift)`, optionally toggles DB0, fires a callback for the UI.

### ui — "图形界面"

- **ioOscilloscopeModel** — The Model. Owns the entire pipeline. Builds it on Start, tears it down on Stop. Emits Qt signals when new samples arrive.
- **ioMainWindow** — The Controller. Owns the Model and both Views. Handles view switching and error dialogs.
- **ioAbstractOscilloscopeView** — A pure interface: `bindModel()`, `unbindModel()`, `asWidget()`. Any new view only needs to implement this.
- **ioCompactOscilloscopeView** — View A: vertical single-column layout.
- **ioWorkspaceOscilloscopeView** — View B: split layout, big waveform on the left, controls on the right.
- **ioWaveformWidget** — Custom Qt widget that paints up to 512 samples as a green polyline.

### Entry points — "程序入口"

- **main.cpp** — Blink demo: creates a device + buffer + writer, blinks LED at 1 Hz then 2 Hz.
- **pipeline.cpp** — CLI pipeline: parses `--freq`, `--duration`, `--output-file` flags, runs the threaded reader + writer for N seconds.
- **qt_main.cpp** — Three lines: create `QApplication`, create `MainWindow`, run event loop.

---

## 6. Presentation 说辞（大白话版）

上台展示这一页 slide 时，这么说：

> "Our code has three layers.
>
> **Bottom layer** is `ioFtdiDevice` — it's the only class that talks to the USB hardware. Everything else goes through it. This keeps hardware access in one place.
>
> **Middle layer** is the pipeline — three threads running in parallel. A Reader pulls bytes from the chip, a Pipeline stage scales and shifts them, and a Writer outputs the result. They communicate through two thread-safe ring buffers. This is the classic **producer-consumer** pattern.
>
> **Top layer** is the Qt GUI using **MVC**. The Model owns the entire pipeline. Two Views — Compact and Workspace — implement the same interface, so we can swap them without changing anything else. The Controller just wires them together.
>
> Every class is in its own file, every class has a single responsibility, and we use the `io` namespace prefix throughout."

---

## 7. 设计原则速查表（被问到时用）

| 原则 | 我们怎么做的 | 哪里体现 |
|------|-------------|---------|
| **一个类一个文件** | 14 个类 = 14 个 .h + 对应 .cpp | 全部文件 |
| **单一职责** | FtdiDevice 只管硬件；CircularBuffer 只管线程同步；Pipeline 只管数学运算 | ioLibrary/ |
| **接口隔离** | AbstractOscilloscopeView 定义了最小接口（4 个方法），View 实现它 | ui/ |
| **依赖倒置** | MainWindow 不认识 CompactView / WorkspaceView，只认识 AbstractOscilloscopeView 指针 | ioMainWindow.h |
| **命名空间** | 所有类都在自己的 namespace 里（ioFtdiDevice::、ioCircularBuffer::、ioMainWindow:: 等） | 每个 .h |
| **跨平台** | `#ifdef _WIN32` 切换 `Sleep` / `usleep`；CMake 自动选 FTDI 库版本 | ioRead.cpp、CMakeLists.txt |

---

## 8. 每个文件干嘛的（大白话中文版，一句话）

### 根目录

| 文件 | 一句话 |
|------|--------|
| `ftd2xx.h` / `libftd2xx.a` | FTDI 公司写的驱动库，我们拿来用，不是我们写的 |
| `controller.c` | 最早写的菜单程序，选 1 开灯选 2 发摩尔斯，现在不用了 |
| `LED_Project.c` | 手动一个一个开关引脚的老代码，现在不用了 |
| `morse_Project.c` | 把打字变成灯闪的老代码，现在不用了 |
| `main.cpp` | LED 闪烁小 demo，证明库能用，灯能亮 |
| `pipeline.cpp` | 命令行版的流水线，不用图形界面也能跑 |
| `qt_main.cpp` | 图形界面的启动入口，就三行代码 |
| `Makefile` | 告诉编译器怎么编命令行版 |
| `CMakeLists.txt` | 告诉编译器怎么编图形界面版 |

### ioLibrary/ — 跟硬件打交道的工具箱

| 文件 | 一句话 |
|------|--------|
| `ioFtdiDevice` | 遥控器——所有跟 USB 芯片说话的代码都在这一个类里 |
| `ioBuffer` | 一小块内存，临时放数据用的，最简单的容器 |
| `ioRead` | 定时从芯片读数据的机器人（v1 单线程版，后来被 ThreadedReader 替代了） |
| `ioWrite` | 定时往芯片写数据的机器人（v1 单线程版，后来被 ThreadedWriter 替代了） |
| `ioCircularBuffer` | 旋转寿司传送带——线程安全的环形缓冲区，满了等，空了也等 |
| `ioThreadedReader` | 采集工人——一个独立线程，不停从芯片读数据丢进传送带 |
| `ioThreadedWriter` | 打包工人——一个独立线程，从传送带取数据写到芯片或文件 |
| `ioScaleShiftPipeline` | 加工工人——从传送带 A 拿数据，放大偏移，放进传送带 B，顺便通知 UI 画图 |

### ui/ — 图形界面，给人看的

| 文件 | 一句话 |
|------|--------|
| `ioOscilloscopeModel` | 后厨——拥有整条流水线，点 Start 开工，点 Stop 收工 |
| `ioMainWindow` | 大管家——管两个视图的切换，出错了弹窗通知你 |
| `ioAbstractOscilloscopeView` | 菜单模板——规定所有视图必须有的功能（绑定后厨、解绑后厨） |
| `ioCompactOscilloscopeView` | 小屏视图——所有控件竖着排一列 |
| `ioWorkspaceOscilloscopeView` | 大屏视图——左边大波形图，右边控制面板 |
| `ioWaveformWidget` | 波形电视——把数据画成绿色折线图，最多显示 512 个点 |

---

## 9. 三个文件夹各自为了干嘛（大白话）

### 根目录的那些 `.c` 文件 = 历史遗留

> 项目最早就是几个 C 文件，手动控制灯。能用，但是太原始——不能同时读写、没有界面、每次操作都要手动输入。现在只留着证明"我们是从这里起步的"。

### `ioLibrary/` = 一套可以复用的工具箱

> 把"跟硬件打交道"和"多线程流水线"这些脏活累活**封装成一个库**。
>
> 为什么要单独成一个文件夹？
> - **复用**：Qt 图形界面用它，命令行 pipeline 也用它，blink demo 也用它——三个程序共享同一套库代码。
> - **隔离**：改了 UI 不影响库，改了库不影响 UI。互不干扰。
> - **编译成静态库**（`libioLibrary.a`）：编一次，到处链接。

### `ui/` = 给人看的图形界面

> 把"用户看到的东西"**全部集中在一个文件夹里**。
>
> 为什么要单独成一个文件夹？
> - **MVC 分层**：Model（后厨）、View（前台）、Controller（管家）各司其职，全在这里面。
> - **跟 ioLibrary 解耦**：ui 文件夹里的代码**不直接调用任何 FT_\* 函数**——它只通过 ioLibrary 里的类间接操作硬件。
> - **可以单独换**：如果以后不想用 Qt 了，想换成 Web 界面，只要重写 `ui/` 文件夹，ioLibrary 一行不用改。

### 一张图总结三个区域的关系

```
┌───────────────────────────────────┐
│ ui/                               │ ← 你看到的界面
│ "给人看的，按钮、旋钮、波形图"       │
│                                   │
│ 只调用 ioLibrary，不碰硬件         │
└──────────────┬────────────────────┘
               │ 调用
               ▼
┌───────────────────────────────────┐
│ ioLibrary/                        │ ← 干活的工人
│ "读芯片、传数据、加工、写出去"       │
│                                   │
│ 只调用 FtdiDevice，其他人不碰硬件   │
└──────────────┬────────────────────┘
               │ 调用
               ▼
┌───────────────────────────────────┐
│ FtdiDevice（在 ioLibrary 里）      │ ← 唯一跟硬件说话的
│ "所有 FT_Open / FT_Read /         │
│  FT_Write / FT_Close 都在这"      │
└──────────────┬────────────────────┘
               │ USB
               ▼
         ┌──────────┐
         │ FT245R   │ ← 实际的芯片
         │ 硬件      │
         └──────────┘
```

**一句话说完：ui/ 管你看到的，ioLibrary/ 管你看不到的，FtdiDevice 是唯一碰硬件的。三层各管各的，互不干扰。**

---

## 10. Project Directory Structure (English version)

```
FT245R/
│
├── ftd2xx.h / libftd2xx.a                                         ← FTDI vendor driver (not our code)
│
├── controller.c                                                   ← Legacy menu app (deprecated)
├── LED_Project.c                                                  ← Legacy LED pin control
├── morse_Project.c                                                ← Legacy Morse code blinker
│                             
├── ioLibrary/                                                     ← Core: reusable C++ I/O static library
│   ├── ioFtdiDevice.h / .cpp                                      ←  USB chip wrapper (all FT_* calls)
│   ├── ioBuffer.h / .cpp                                          ←  Simple heap byte buffer
│   ├── ioRead.h / .cpp                                            ←  Fixed-rate read loop (Phase 1)
│   ├── ioWrite.h / .cpp                                           ←  Fixed-rate write loop (Phase 1)
│   ├── ioCircularBuffer.h / .cpp                                  ←  Thread-safe ring buffer
│   ├── ioThreadedReader.h / .cpp                                  ←  Reader thread (producer)
│   ├── ioThreadedWriter.h / .cpp                                  ←  Writer thread (consumer)
│   └── ioScaleShiftPipeline.h / .cpp                              ←  Processing thread (scale + shift)
│
├── ui/                                                               ← Qt 6 GUI (MVC architecture)
│   ├── ioOscilloscopeModel.h / .cpp                                ← Model (owns pipeline)
│   ├── ioMainWindow.h / .cpp                                       ← Controller (view switching)
│   ├── ioAbstractOscilloscopeView.h                                ← View interface (pure virtual)
│   ├── ioCompactOscilloscopeView.h / .cpp                          ← View A (compact layout)
│   ├── ioWorkspaceOscilloscopeView.h / .cpp                        ← View B (workspace layout)
│   └── ioWaveformWidget.h / .cpp                                   ← Waveform display widget
│
├── main.cpp                                                        ← Entry: LED blink demo (1 Hz / 2 Hz)
├── pipeline.cpp                                                    ← Entry: CLI multithreaded pipeline
├── qt_main.cpp                                                     ← Entry: Qt GUI application
├── Makefile                                                        ← Build: CLI targets (make)
└── CMakeLists.txt                                                  ← Build: Qt target (cmake)
```

---

## 11. One-Line File Descriptions (English version of §8)

### Root Directory

|              File                        |                                   description                                      |
|------------------------------------------|------------------------------------------------------------------------------------|
| `ftd2xx.h` / `libftd2xx.a`               | Vendor-provided FTDI driver library — not written by us                            |
| `controller.c`                           | Original menu app (select 1 to control LEDs, select 2 to send Morse) — deprecated  |
| `LED_Project.c`                          | Legacy code for manually toggling individual pins — deprecated                     |
| `morse_Project.c`                        | Legacy code for blinking LEDs in Morse patterns — deprecated                       |
| `main.cpp`                               | LED blink demo — a minimal proof that the library works                            |
| `pipeline.cpp`                           | CLI version of the multithreaded pipeline — runs without a GUI                     |
| `qt_main.cpp`                            | GUI entry point — just three lines of code                                         |
| `Makefile`                               | Build script for the CLI targets                                                   |
| `CMakeLists.txt`                         | Build script for the Qt GUI target                                                 |

### ioLibrary/ — The hardware I/O toolbox

|          File           |                                                           description                                                                 |
|-------------------------|---------------------------------------------------------------------------------------------------------------------------------------|
| `ioFtdiDevice`          | The remote control — all USB chip communication (open / read / write / close) lives in this one class                                 |
| `ioBuffer`              | A small block of heap memory for temporarily holding data — the simplest container                                                    |
| `ioRead`                | A robot that reads from the chip on a timer (v1, single-threaded — later replaced by ThreadedReader)                                  |
| `ioWrite`               | A robot that writes to the chip on a timer (v1, single-threaded — later replaced by ThreadedWriter)                                   |
| `ioCircularBuffer`      | The conveyor belt — a thread-safe ring buffer that blocks when full or empty                                                          |
| `ioThreadedReader`      | The collector worker — a dedicated thread that reads from the chip and pushes bytes onto the conveyor belt                            |
| `ioThreadedWriter`      | The packer worker — a dedicated thread that pulls bytes from the conveyor belt and writes to the chip or a file                       |
| `ioScaleShiftPipeline`  | The processing worker — takes data from conveyor belt A, scales and shifts it, puts it on conveyor belt B, and notifies the UI to draw|

### ui/ — The graphical interface (what the user sees)

|              File             |                                       description                                                  |
|-------------------------------|----------------------------------------------------------------------------------------------------|
| `ioOscilloscopeModel`         | The kitchen — owns the entire pipeline; click Start to fire up, click Stop to shut down            |
| `ioMainWindow`                | The manager — handles switching between two views, shows error dialogs                             |
| `ioAbstractOscilloscopeView`  | The menu template — defines the interface every view must implement (bind model, unbind model)     |
| `ioCompactOscilloscopeView`   | Compact view — all controls stacked in a single column                                             |
| `ioWorkspaceOscilloscopeView` | Workspace view — big waveform on the left, control panel on the right                              |
| `ioWaveformWidget`            | The waveform display — renders data as a green polyline, up to 512 data points                     |

---

## 12. What Each Folder Is For (English version of §9)

### Root `.c` files = legacy code

> The project started as a handful of C files for manually controlling LEDs. They work, but they're primitive — no concurrent read/write, no GUI, every operation requires manual input. We keep them to show where we started.

### `ioLibrary/` = a reusable toolbox

> Packages all the "talk to hardware" and "multithreaded pipeline" work into a single static library.
>
> Why its own folder?
> - **Reuse**: The Qt GUI uses it, the CLI pipeline uses it, the blink demo uses it — three programs share the same library code.
> - **Isolation**: Changing the UI doesn't affect the library; changing the library doesn't affect the UI. They stay independent.
> - **Static library** (`libioLibrary.a`): compile once, link everywhere.

### `ui/` = the graphical interface

> Puts everything the user sees into one folder.
>
> Why its own folder?
> - **MVC separation**: Model (kitchen), View (front-of-house), Controller (manager) each have clear roles, all inside this folder.
> - **Decoupled from ioLibrary**: Code in `ui/` never calls any `FT_*` function directly — it operates hardware only through ioLibrary classes.
> - **Swappable**: If we later want a web interface instead of Qt, we only rewrite the `ui/` folder. ioLibrary stays unchanged.

### Relationship between the three areas

```
┌───────────────────────────────────┐
│ ui/                               │  ← What you see (buttons, knobs, waveform)
│ Calls ioLibrary, never touches HW │
└──────────────┬────────────────────┘
               │ calls
               ▼
┌───────────────────────────────────┐
│ ioLibrary/                        │  ← The workers (read, process, write)
│ Calls FtdiDevice only             │
└──────────────┬────────────────────┘
               │ calls
               ▼
┌───────────────────────────────────┐
│ FtdiDevice (inside ioLibrary)     │  ← The only class that talks to HW
│ All FT_Open / FT_Read /           │
│  FT_Write / FT_Close live here    │
└──────────────┬────────────────────┘
               │ USB
               ▼
         ┌──────────┐
         │ FT245R   │  ← The physical chip
         │ Hardware  │
         └──────────┘
```

**One sentence: `ui/` handles what you see, `ioLibrary/` handles what you don't see, `FtdiDevice` is the only class that touches hardware. Three layers, each minding its own business.**

---

## 13. Component Breakdown — What / Why / How

### A. Vendor Layer

**`ftd2xx.h` / `libftd2xx.a`**

- **What:** Pre-compiled driver library from FTDI. Provides `FT_Open`, `FT_Read`, `FT_Write`, `FT_Close`, `FT_SetBitMode`, etc.
- **Why:** The FT245R chip communicates over USB. We cannot talk to it directly — we need the vendor's driver as the bridge between our code and the hardware.
- **How:** We link against `libftd2xx.a` (macOS) or `ftd2xx.lib` (Windows) at compile time. Our code only calls `FT_*` functions through `ioFtdiDevice`, never directly.

---

### B. ioLibrary — Hardware I/O & Pipeline

#### `ioFtdiDevice`

- **What:** A C++ class that wraps every `FT_*` call (open, read, write, close) behind a `std::mutex`.
- **Why:** Without this, every file would contain raw `FT_Open`/`FT_Write` calls scattered everywhere. By putting all hardware access in one class, we get a single point of control. The mutex makes it safe when multiple threads share the same device.
- **How:** Constructor initializes `handle = nullptr`. `open()` calls `FT_Open` → `FT_ResetDevice` → `FT_Purge` → `FT_SetBitMode`. `read()`/`write()` lock the mutex, call `FT_Read`/`FT_Write`, unlock. Destructor calls `close()` automatically.

#### `ioBuffer`

- **What:** A simple heap-allocated byte array with `create()`, `data()`, `size()`, `destroy()`.
- **Why:** Phase 1 needed a place to store bytes before writing or after reading. It's the simplest possible container — just a `malloc`'d block.
- **How:** `create(N)` allocates N bytes and zeroes them. `data()` returns the raw pointer. Used only by Phase 1 classes (`ioRead`, `ioWrite`) and the blink demo (`main.cpp`).

#### `ioRead` / `ioWrite`

- **What:** Single-threaded blocking loops that read or write at a fixed frequency.
- **Why:** Phase 1 building blocks — prove that we can talk to the chip at a controlled rate before adding threads. They are the "v1" that evolved into `ThreadedReader`/`ThreadedWriter`.
- **How:** `configure(buffer, N, Hz)` sets parameters. `readLoop(cycles)`/`writeLoop(cycles)` runs a for-loop: call `device->read()`/`write()`, sleep `1/Hz` seconds, repeat.

#### `ioCircularBuffer`

- **What:** A fixed-size ring buffer protected by a mutex and two condition variables (`notEmpty`, `notFull`).
- **Why:** When the reader thread produces data and the writer thread consumes it, they run at different speeds. The ring buffer decouples them — the producer blocks when the buffer is full, the consumer blocks when it's empty. No data is lost, no busy-waiting.
- **How:** `write()` waits on `notFull`, copies byte-by-byte into the ring, advances `head`, signals `notEmpty`. `read()` is the mirror. `setDone()` sets a `finished` flag and wakes both sides so threads can exit cleanly on shutdown.

#### `ioThreadedReader`

- **What:** A dedicated `std::thread` that continuously reads from `FtdiDevice` and pushes bytes into a `CircularBuffer`.
- **Why:** Reading must happen in the background so the UI doesn't freeze and the writer can consume in parallel. This is the **producer** in the producer-consumer pattern.
- **How:** `start()` spawns the thread. Inside `threadFunc()`: loop while `running` → `device->read()` → `circBuffer->write()` → sleep `1/Hz`. `stop()` sets `running = false`, calls `circBuffer->setDone()`, and `join()`s the thread.

#### `ioThreadedWriter`

- **What:** A dedicated `std::thread` that pulls bytes from a `CircularBuffer` and writes them to either an FTDI device or a file.
- **Why:** The consumer side of the pipeline. We need it in a separate thread so it doesn't block the reader or the UI.
- **How:** Two constructors — `ThreadedWriter(FtdiDevice*)` for device output, `ThreadedWriter(const char* path)` for file output. `threadFunc()` loops: `circBuffer->read()` → `device->write()` or `fwrite()` → sleep `1/Hz`.

#### `ioScaleShiftPipeline`

- **What:** A processing stage that sits between two ring buffers (raw → processed). Applies `round(scale × byte + shift)`, optionally toggles DB0, and fires a callback for each processed sample.
- **Why:** This is the core signal processing — equivalent to an oscilloscope's vertical gain and offset controls. Without it, we'd just be copying raw bytes from input to output with no transformation.
- **How:** Runs its own thread. Loop: `inBuf->read(&b, 1)` → compute `v = clamp(round(scale * b + shift))` → if `blinkDb0` is on, force bit 0 to alternate 0/1 → `outBuf->write(&v, 1)` → `callback(v)`. `scale`, `shift`, `blinkDb0` are `std::atomic` so the UI can change them in real time without stopping the pipeline.

---

### C. ui/ — Qt GUI (MVC)

#### `ioOscilloscopeModel` (Model)

- **What:** A `QObject` subclass that owns and manages the entire pipeline: 2× `FtdiDevice`, 2× `CircularBuffer`, 1× `ThreadedReader`, 1× `ScaleShiftPipeline`, 1× `ThreadedWriter`.
- **Why:** MVC says all state and logic belong in the Model. The views don't know anything about threads, ring buffers, or USB — they just receive Qt signals with sample data.
- **How:** `startAcquisition()` creates all objects, configures them, installs a sample callback that posts to the Qt event loop via `QMetaObject::invokeMethod(Qt::QueuedConnection)`, and starts the three threads. `stopAcquisition()` tears everything down in reverse. Emits `samplesUpdated()`, `logLine()`, `errorMessage()`.

#### `ioMainWindow` (Controller)

- **What:** A `QMainWindow` subclass that holds the Model, a `QStackedWidget` with both views, and the View menu.
- **Why:** The controller's job is to wire things together: create the model, create both views, connect signals, and handle view switching.
- **How:** On view switch: `currentView_->unbindModel()` → `stack_->setCurrentWidget(newView)` → `currentView_ = newView` → `currentView_->bindModel(model_)`. Only one view is connected to the model at a time. Also connects `model_->errorMessage` to show a `QMessageBox`.

#### `ioAbstractOscilloscopeView` (View interface)

- **What:** A pure virtual interface with 4 methods: `asWidget()`, `viewTitle()`, `bindModel()`, `unbindModel()`.
- **Why:** So `MainWindow` doesn't need to know which concrete view is active. It talks to both views through this common interface. If we add a third view later, zero changes needed in MainWindow.
- **How:** Not a `QObject` — just a plain abstract class. Both concrete views multiply-inherit from `QWidget` (for Qt) and `AbstractOscilloscopeView` (for polymorphism).

#### `ioCompactOscilloscopeView` / `ioWorkspaceOscilloscopeView` (Concrete Views)

- **What:** Two different layouts for the same controls. Compact = single vertical column. Workspace = `QSplitter` with big waveform on the left and controls on the right.
- **Why:** The rubric requires "show two different UIs." Same functionality, different spatial arrangement — proves the MVC decoupling works (change the view, model stays untouched).
- **How:** Both implement `AbstractOscilloscopeView`. `bindModel()` connects model signals to view slots (`onSamplesUpdated`, `onLogLine`) and pushes current widget values into the model. UI widgets (spin boxes, checkboxes, buttons) call `model_->setScale()`, `model_->startAcquisition()`, etc.

#### `ioWaveformWidget`

- **What:** A custom `QWidget` that draws a green polyline of up to 512 normalized samples.
- **Why:** Qt doesn't have a built-in oscilloscope widget. We need a real-time scrolling waveform display.
- **How:** `setSamples()` stores the latest values and calls `update()`. `paintEvent()` maps each sample (0.0–1.0) to a screen Y coordinate and draws line segments with `QPainter`.

---

### D. Entry Points

#### `main.cpp` — Blink demo

- **What:** Creates a `FtdiDevice`, fills a 2-byte `ioBuffer` with `[0xFF, 0x00]`, runs `ioWrite::writeLoop()` at 1 Hz (10 cycles) then 2 Hz (20 cycles), followed by a read demo.
- **Why:** Simplest possible proof that the library works — the LED blinks, you can see it.

#### `pipeline.cpp` — CLI pipeline

- **What:** Parses command-line arguments (`--freq`, `--duration`, `--output-file`, `--output-ftdi`), creates a `ThreadedReader` + `CircularBuffer` + `ThreadedWriter`, runs for N seconds, then stops.
- **Why:** Proves the multithreaded pipeline works without needing Qt. Useful for headless / CI testing.

#### `qt_main.cpp` — Qt GUI

- **What:** Three lines: create `QApplication`, create `MainWindow`, call `application.exec()`.
- **Why:** Qt requires exactly this boilerplate. All real logic is in `MainWindow` and `OscilloscopeModel`.
