# 项目大白话解读

不讲术语，不讲理论，就讲"这玩意到底在干嘛"。

---

## 一句话总结

> 我们写了一个程序，通过 USB 线控制一块小芯片上的 8 个开关，可以让 LED 灯亮灭，也可以读回开关状态画成波形图——本质上就是一个**超低配版示波器**。

---

## 硬件是什么？

想象一个 **带 8 个开关的遥控器**：

```
你的电脑  ──── USB 线 ──── FT245R 芯片（小板子）
                                │
                        8 个引脚（DB0 ~ DB7）
                                │
                           LED 灯 / 其他设备
```

- **FT245R** 就是这个小板子，插在 USB 口上。
- 它有 **8 个引脚**（DB0 到 DB7），每个引脚就像一个开关，可以打开（高电平=1）或关闭（低电平=0）。
- 你往芯片写一个字节（8 位二进制数），每一位对应一个引脚的开/关。
  - 比如写 `0x01`（二进制 `00000001`）→ 只有 DB0 打开 → DB0 上的 LED 亮了。
  - 写 `0xFF`（二进制 `11111111`）→ 全部 8 个引脚都开 → 8 个 LED 全亮。
  - 写 `0x00` → 全关。
- 你也可以反过来**读**芯片 → 芯片告诉你现在 8 个引脚各自是高还是低。

**就这么简单：写=控制灯，读=看灯的状态。**

---

## 软件在干嘛？

我们的代码分**三代**，一代比一代高级：

### 第一代：最原始的 C 程序（legacy，已不是重点）

文件：`controller.c`、`LED_Project.c`、`morse_Project.c`

就是一个**菜单程序**：

```
你想干嘛？
1. 控制 LED（手动选引脚，手动开关）
2. 发摩尔斯电码（输入 SOS → 灯按点划闪）
3. 写一个字节
4. 读一个字节
5. 退出
```

**打比方**：这就像用遥控器一个按钮一个按钮地按，纯手动。

---

### 第二代：C++ 库（ioLibrary）— 自动化 + 多线程

文件：`ioLibrary/` 文件夹里所有 `.h` 和 `.cpp`

**问题**：手动一个字节一个字节读写太累了，能不能自动？

**解决**：我们造了一套"工具箱"（静态库），里面有 8 个工具类：

#### 单线程工具（简单循环）

| 工具 | 打比方 | 干嘛的 |
|------|--------|--------|
| `FtdiDevice` | USB 遥控器本身 | 封装了"打开设备、读、写、关闭"四个动作。其他所有工具都通过它跟芯片打交道。 |
| `ioBuffer` | 一块白板 | 在内存里开一小块空间，用来暂存要写的数据或刚读回来的数据。 |
| `ioRead` | 一个定时读的机器人 | 你告诉它"每秒读 5 次，每次读 1 字节"，它就乖乖循环执行。 |
| `ioWrite` | 一个定时写的机器人 | 同上，但是往芯片写。 |

**打比方**：从"人手按遥控器"升级成了"设了闹钟的机器人按遥控器"。

#### 多线程工具（流水线 / 管道）

这是项目的**核心亮点**。

想象一个**工厂流水线**：

```
                     传送带 A            传送带 B
采集工人 ──→ [ 原料箱 ] ──→ 加工工人 ──→ [ 成品箱 ] ──→ 打包工人
(Reader)   (rawBuffer)  (Pipeline)  (processedBuf) (Writer)
```

| 工具 | 流水线角色 | 干嘛的 |
|------|-----------|--------|
| `CircularBuffer` | 传送带上的箱子 | 一个**环形缓冲区**（就像旋转寿司的传送带），生产者往里放，消费者从里拿。放满了就等，空了也等——不会丢数据也不会爆。 |
| `ThreadedReader` | 采集工人 | 一个单独的线程，不停地从芯片读数据，放进"原料箱"。 |
| `ScaleShiftPipeline` | 加工工人 | 从"原料箱"拿一个字节，做一个数学运算 `结果 = round(scale × 原始值 + shift)`，限制在 0~255 之间，然后放进"成品箱"。还可以选择让 DB0 每次交替亮灭（闪灯模式）。 |
| `ThreadedWriter` | 打包工人 | 从"成品箱"拿数据，要么写到**文件**里（存盘），要么写到**另一块芯片**上（双设备模式）。 |

**三个工人同时干活（三个线程并行）**，通过两个传送带（两个 `CircularBuffer`）协调。

**打比方**：从"一个机器人"升级成了"三个工人同时在流水线上干活的工厂"。

---

### 第三代：Qt 图形界面（UI）

文件：`ui/` 文件夹 + `qt_main.cpp`

**问题**：流水线跑起来了，但全是终端打印，能不能有个**图形界面**看波形、调参数？

**解决**：用 Qt（一个 C++ GUI 框架）做了一个窗口程序。

架构是经典的 **MVC（模型-视图-控制器）**，用大白话说就是：

```
┌────────────────────────────────────────────────┐
│ MainWindow（控制器 / 大管家）                    │
│  "用户点了什么按钮，我就通知谁去干活"             │
│                                                 │
│  ┌───────────────────┐  ┌────────────────────┐  │
│  │ OscilloscopeModel │  │   View（视图）      │  │
│  │  （模型 / 后厨）    │  │  （前台 / 展示窗）  │  │
│  │                   │  │                    │  │
│  │  拥有整条流水线：   │  │  两套外观可选：      │  │
│  │  Reader           │  │  - Compact View    │  │
│  │  Pipeline         │  │    （紧凑型）       │  │
│  │  Writer           │  │  - Workspace View  │  │
│  │  两个传送带        │  │    （宽屏型）       │  │
│  │  FTDI 设备        │  │                    │  │
│  │                   │  │  都有：             │  │
│  │  处理好的数据通过   │  │  - 波形图           │  │
│  │  信号发给 View     │  │  - Scale/Shift 旋钮│  │
│  └───────────────────┘  │  - Start/Stop 按钮 │  │
│                         │  - 日志区域         │  │
│                         └────────────────────┘  │
└────────────────────────────────────────────────┘
```

| 角色 | 类名 | 打比方 |
|------|------|--------|
| 大管家 | `MainWindow` | 饭店经理：客人（用户）说要换座位（换视图），经理安排；后厨（Model）出错了，经理出来跟客人解释。 |
| 后厨 | `OscilloscopeModel` | 厨房：拥有所有灶台（流水线），接到"开工"指令就生火做菜，做好的菜（波形数据）通过传菜窗口（Qt 信号）端出去。 |
| 菜单接口 | `AbstractOscilloscopeView` | 一份"菜单模板"：规定了所有餐厅必须有的功能（绑定后厨、解绑后厨、显示自己的窗口）。 |
| 前台 A | `CompactOscilloscopeView` | 小餐厅：所有控件竖着排一列，紧凑。 |
| 前台 B | `WorkspaceOscilloscopeView` | 大餐厅：左边大屏幕放波形，右边放控制面板，适合宽屏。 |
| 波形屏幕 | `WaveformWidget` | 餐厅墙上的电视：实时滚动显示最近 512 个数据点画成的折线图。 |

**用户操作流程**（实际点按钮时发生的事）：

1. 打开程序 → 看到 Compact View。
2. 点 **Start** → `MainWindow` 告诉 `Model` "开工" → Model 打开 USB 设备、创建传送带、启动三个工人线程 → 数据开始流 → 波形图开始滚动。
3. 拧 **Scale** 旋钮从 1.0 到 2.0 → Model 把新值写进 `atomic<double>` → Pipeline 工人下一次加工时就用新值 → 波形幅度变大。
4. 拧 **Shift** 旋钮 → 同理，波形上下移动。
5. 菜单栏 **View → Workspace** → MainWindow 把 Model 从 Compact View 上"拔下来"，插到 Workspace View 上 → 画面变成宽屏布局，波形数据无缝继续。
6. 点 **Stop** → 三个工人线程依次停下、USB 设备关闭。

---

## 类之间的关系，用人话说

### "拥有"（组合 / Composition）= 老板和员工

老板不在了，员工也散了。

- `MainWindow` **拥有** `OscilloscopeModel`、两个 View → 窗口关了，全都释放。
- `OscilloscopeModel` **拥有** `FtdiDevice`、`CircularBuffer`、三个工人线程 → Model 停了，流水线全拆。
- 每个 View **拥有** 自己的 `WaveformWidget` → 视图销毁，波形控件也没了。

### "借用"（聚合 / Aggregation）= 租工具

工具是别人的，我只是用一下。

- `ThreadedReader` **借用** `FtdiDevice`（不是它创建的）和 `CircularBuffer`（也不是它创建的）。
- 两个 View **借用** `OscilloscopeModel`（Model 是 MainWindow 创建的，View 只是拿到一个指针用一下）。

### "继承"（Inheritance）= 师傅和徒弟

徒弟继承师傅的手艺，再加上自己的特色。

- `CompactOscilloscopeView` 和 `WorkspaceOscilloscopeView` 都**实现了** `AbstractOscilloscopeView` 这个"模板" → 所以 `MainWindow` 可以不管你是哪种 View，统一调用 `bindModel` / `unbindModel`。

---

## 构建和运行，用人话说

### CLI 管道（不需要图形界面）

```bash
make pipeline          # 编译
./pipeline --freq 5 --duration 5 --output-file output.bin
# 意思：每秒采 5 次，采 5 秒，结果存到 output.bin
```

### Qt 图形界面

```bash
brew install qt cmake            # 装 Qt 和 CMake（只需一次）
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build              # 编译
./build/oscilloscope_qt          # 启动窗口程序
```

### 没有硬件怎么办？

程序会在打开设备那一步报错弹窗："Failed to open FTDI device"——然后什么都不会崩，只是没数据。

---

## 文件对照表（"这个文件是干嘛的？"）

| 文件 | 一句话说明 |
|------|-----------|
| `controller.c` | 最早的菜单程序（选 1 控灯，选 2 发摩尔斯），已弃用 |
| `LED_Project.c` | 手动逐个开关引脚的函数 |
| `morse_Project.c` | 把文字变成摩尔斯电码闪灯的函数 |
| `ioLibrary/ioFtdiDevice.*` | 封装 USB 芯片的"遥控器"类 |
| `ioLibrary/ioBuffer.*` | 一块临时存数据的内存 |
| `ioLibrary/ioRead.*` | 定时读芯片的机器人 |
| `ioLibrary/ioWrite.*` | 定时写芯片的机器人 |
| `ioLibrary/ioCircularBuffer.*` | 旋转寿司传送带（线程安全环形缓冲区） |
| `ioLibrary/ioThreadedReader.*` | 流水线上的"采集工人" |
| `ioLibrary/ioThreadedWriter.*` | 流水线上的"打包工人" |
| `ioLibrary/ioScaleShiftPipeline.*` | 流水线上的"加工工人"（scale × 值 + shift） |
| `ui/ioOscilloscopeModel.*` | 后厨：拥有并管理整条流水线 |
| `ui/ioMainWindow.*` | 饭店经理：管视图切换和错误弹窗 |
| `ui/ioAbstractOscilloscopeView.h` | 菜单模板：规定所有视图必须实现的接口 |
| `ui/ioCompactOscilloscopeView.*` | 紧凑型餐厅前台 |
| `ui/ioWorkspaceOscilloscopeView.*` | 宽屏型餐厅前台 |
| `ui/ioWaveformWidget.*` | 墙上的波形电视 |
| `main.cpp` | LED 闪烁 demo 入口（1Hz + 2Hz） |
| `pipeline.cpp` | CLI 多线程管道入口（命令行参数控制） |
| `qt_main.cpp` | Qt 窗口程序入口（就三行：创建窗口、显示、运行） |
| `Makefile` | 告诉编译器怎么把 .cpp 变成可执行文件 |
| `CMakeLists.txt` | 告诉 CMake 怎么编译 Qt 程序 |
| `ftd2xx.h` / `libftd2xx.a` | FTDI 厂商提供的驱动库（不是我们写的） |
| `WinTypes.h` | 让 FTDI 的头文件在 macOS/Linux 上也能编译的补丁 |

---

## 最后的最后：这个项目到底证明了什么？

这是一门**软件架构**课，教授要看的不是"灯能不能亮"，而是：

1. **分层**：我们把"跟硬件打交道"和"画界面"彻底分开了（ioLibrary vs ui）。
2. **设计模式**：生产者-消费者（Reader/Writer + CircularBuffer）、管道过滤器（Pipeline）、MVC（Model/View/Controller）。
3. **可替换**：想换界面？实现 `AbstractOscilloscopeView` 就行，Model 和 ioLibrary 一行不用改。
4. **线程安全**：三个线程同时跑，靠 `mutex` + `condition_variable` + `atomic` 协调，不崩不丢不死锁。
5. **UML ↔ 代码一一对应**：每条 UML 关系线都能在代码里找到对应的字段声明或函数调用。

把这五点理解了，presentation 的时候不管教授怎么问，你都能答上来。





  ---                                                                                                                                                                     
  图 1 — Controller 层（MainWindow 怎么管两个 View）
                                                                                                                                                                          
  - MainWindow（Controller / 大管家）：程序的主窗口，继承自 QMainWindow                                                                                                   
  - 它拥有三样东西（黑色菱形 = composition）：                                                                                                                            
    - OscilloscopeModel — 数据和逻辑的核心（后厨）                                                                                                                        
    - CompactOscilloscopeView — 紧凑布局视图                                                                                                                              
    - WorkspaceOscilloscopeView — 宽屏布局视图                                                                                                                            
  - currentView_（空心菱形 = aggregation）是一个指针，指向当前正在用的那个 View                                                                                           
  - AbstractOscilloscopeView（标了 I = Interface）：定义了 bindModel() / unbindModel() 接口，两个具体 View 都实现了它（三角箭头 = inheritance）                           
  - 为什么这么设计：MainWindow 不认识具体是哪种 View，只通过接口操作 → 切换视图时只需要 unbind 旧的、bind 新的，一行 Model 代码都不用改                                   
                                                                                                                                                                          
  ---                                                                                                                                                                     
  图 2 — Model 层（OscilloscopeModel 拥有什么）                                                                                                                           
                                                                                                                                                                          
  - OscilloscopeModel（继承 QObject）：整条流水线的老板                                                                                                                   
  - 五条黑色菱形线 = composition（组合），意思是 Model 创建并拥有这些东西：                                                                                               
    - FtdiDevice — USB 芯片驱动（×2，一个读一个写）                                                                                                                       
    - CircularBuffer — 线程安全环形缓冲区（×2，原料箱和成品箱）
    - ThreadedReader — 读线程                                                                                                                                             
    - ScaleShiftPipeline — 加工线程                                                                                                                                       
    - ThreadedWriter — 写线程                                                                                                                                             
  - 为什么这么设计：所有流水线组件的生命周期由 Model 统一管理 → 点 Start 全部创建，点 Stop 全部销毁，不会有资源泄漏                                                       
                                                                                                                                                                          
  ---                                                                                                                                                                     
  图 3 — Pipeline 数据流（数据怎么从左流到右）              
                                                                                                                                                                          
  - 这是流水线的内部连接图，从左到右就是数据流动方向：      
    - ThreadedReader →（producer）→ rawBuffer : CircularBuffer →（inBuf）→ ScaleShiftPipeline →（outBuf）→ processedBuffer : CircularBuffer →（consumer）→ ThreadedWriter 
  - FtdiDevice 在最右边，ThreadedReader 和 ThreadedWriter 都通过 device* 指针引用它                                                                                       
  - 所有连线都是空心菱形（aggregation）= 这些组件之间只是借用彼此的引用，不负责创建和销毁（创建和销毁是上一张图 Model 的事）
  - 三个 note 说明了关键行为：                                                                                                                                            
    - CircularBuffer：setDone() 解除阻塞，让线程安全退出                                                                                                                  
    - ScaleShiftPipeline：clamp(round(scale·b + shift)) + DB0 toggle + 回调给 UI                                                                                          
    - ThreadedWriter：输出目标可以是设备或文件，由构造函数决定                                                                                                            
  - 为什么这么设计：经典的 pipe-and-filter（管道过滤器） 模式 → 每个组件只做一件事，中间用线程安全缓冲区连接，可以独立替换任何一个环节                                    
                                                                                                                                                                          
  ---                                                                                                                                                                     
  三张图的展示逻辑：从外到内逐层放大                                                                                                                                      
  1. 图 1：最外层——Controller 怎么管 View 和 Model                                                                                                                        
  2. 图 2：中间层——Model 里有什么零件                                                                                                                                     
  3. 图 3：最内层——零件之间数据怎么流动   