# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个 C 语言项目，使用 FTDI D2XX 驱动库控制 FT245R USB 转并行 FIFO 芯片。芯片运行在同步 bit-bang 模式下，通过 USB 控制 8 个 GPIO 引脚（DB0-DB7）。属于 Stevens CS565（软件架构与组件化设计）课程项目。

## 编译命令

**macOS:**
```bash
gcc *.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller
```
加 `-g` 可生成调试符号。

**Windows (MSVC，在 Developer PowerShell 中执行):**
```bash
cl *.c /Zi /EHsc /nologo /link i386/ftd2xx.lib /out:controller.exe    # x86 架构
cl *.c /Zi /EHsc /nologo /link amd64/ftd2xx.lib /out:controller.exe   # x64 架构
```

**Windows (GCC/MinGW):**
```bash
gcc *.c -lftd2xx -o controller.exe -g
```

VSCode 构建任务 `build-controller` 已在 `.vscode/tasks.json` 中预配置。

## 启动与测试

### 前置条件

1. **安装编译器**
   - macOS: `brew install gcc`（或 `xcode-select --install` 安装 XCode 命令行工具）
   - Windows: 安装 [MSVC Build Tools](https://visualstudio.microsoft.com/downloads/#remote-tools-for-visual-studio-2022)（页面底部选 "Build Tools for Visual Studio 2022"，勾选桌面开发组件）

2. **连接硬件** — 将 FT245R USB 设备插入电脑。程序启动时会调用 `FT_Open`，如果没有设备会直接报错退出。

3. **macOS 驱动注意** — 如果 `FT_Open` 失败，可能需要卸载 macOS 自带的 FTDI 内核驱动：
   ```bash
   sudo kextunload -b com.apple.driver.AppleUSBFTDI
   ```

### 编译运行

```bash
# macOS 编译
gcc *.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller

# 运行
./controller
```

### 程序操作

启动后会出现交互式菜单：

```
Control Menu
1. Control LEDs        — 控制单个引脚的开关（0-7）
2. Send Morse Code     — 输入文本，通过 LED 闪烁发送摩尔斯电码
3. Write byte to port  — 向端口写入一个字节（直接控制 8 个引脚）
4. Read byte from port — 从端口读取一个字节（读取引脚状态）
5. Exit                — 退出程序
```

### 功能测试方法

| 功能 | 测试步骤 | 预期结果 |
|------|---------|---------|
| LED 控制 | 选择 1 → 输入引脚号（如 `0`）→ 输入状态（`1` 开 / `0` 关）→ 输入 `done` 退出 | 对应引脚的 LED 亮/灭，终端显示引脚状态 |
| 摩尔斯电码 | 选择 2 → 输入文本（如 `SOS`）→ 输入 `E0` 退出 | DB0 引脚 LED 按摩尔斯时序闪烁（点=100ms，划=300ms） |
| 写字节 | 选择 3 | 向端口写入 `0x00`（所有引脚关闭） |
| 读字节 | 选择 4 | 终端打印读取的字节值，如 `Read 1 bytes: 0x00` |

### 无硬件时的验证

如果没有 FT245R 设备，程序在 `FT_Open` 就会报错退出。此时只能验证编译是否通过：
```bash
gcc *.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller
echo $?   # 返回 0 表示编译成功
```

## 架构

程序是一个菜单驱动的 CLI 应用，由三个源文件编译链接而成：

- **`controller.c`** — 入口（`main`）、设备初始化、主菜单循环。通过 `FT_Open` 打开设备，用 `FT_SetBitMode`（掩码 `0xFF`，所有引脚设为输出）配置 bit-bang 模式，然后分发到各功能模块。也处理原始字节的读写操作。
- **`LED_Project.c`** — LED 交互控制（`controlLED`）。用户可以逐个切换引脚（0-7）的开关状态，内部通过位掩码操作单字节输出缓冲区。
- **`morse_Project.c`** — 摩尔斯电码发送（`sendMorseCode`）。将用户输入的文本转换为摩尔斯码，通过定时调用 `FT_Write` + `usleep` 在引脚 0 上控制 LED 闪烁（点=100ms，划=300ms）。

三个文件共享 `controller.c::initializeDevice()` 中获取的 `FT_HANDLE`。跨平台通过 `#ifdef _WIN32` 条件编译实现（Windows 用 `Sleep`，Unix 用 `usleep`）。

## 关键依赖

- **FTDI D2XX 库** — `ftd2xx.h` 头文件 + 平台对应的静态库（macOS: `libftd2xx.a`，Windows: `i386/ftd2xx.lib` 或 `amd64/ftd2xx.lib`）。这是 FTDI 厂商提供的库，不是用户代码。
- **`WinTypes.h`** — 为非 Windows 平台提供 Windows 类型定义（`DWORD`、`BYTE`、`HANDLE` 等），使 `ftd2xx.h` 能在 macOS/Linux 上编译。
- **`ftd2xx.cfg`** — 驱动配置文件，包含厂商/产品 ID `0403:6001` 的 `ConfigFlags`。

## 硬件说明

FT245R 芯片提供 8 个数据引脚（DB0-DB7）。在 bit-bang 模式下，通过 `FT_Write` 写入的字节中每一位对应一个引脚。必须连接物理 FT245R USB 设备才能运行程序（否则 `FT_Open` 启动即失败）。
