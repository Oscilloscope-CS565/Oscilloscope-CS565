# FT245R Controller - CS565

A C++ project for controlling the FTDI FT245R (UM245R) USB-to-parallel chip, built for Stevens CS565 (Software Architecture and Component-Based Design). Includes a reusable static I/O library and a multithreaded data acquisition pipeline.

Figma link: https://www.figma.com/make/zhjeJXZHEork6O5gXgszK4/Untitled?t=eDVLhgxRvg6EaR4i-1

## Project Structure

```
FT245R/
├── ioLibrary/                      # Reusable FTDI I/O library (static, C++)
│   ├── ioFtdiDevice.h / .cpp       # Device class: wraps all FT_* calls
│   ├── ioBuffer.h / .cpp           # Simple buffer management
│   ├── ioWrite.h  / .cpp           # Single-threaded frequency-timed write
│   ├── ioRead.h   / .cpp           # Single-threaded frequency-timed read
│   ├── ioCircularBuffer.h / .cpp   # Thread-safe ring buffer (mutex + condvar)
│   ├── ioThreadedReader.h / .cpp   # Multithreaded FTDI reader (producer)
│   ├── ioThreadedWriter.h / .cpp   # Multithreaded writer to file or FTDI (consumer)
│   └── ioScaleShiftPipeline.h / .cpp # Pipeline stage: scale + shift between two ring buffers
├── ui/                             # Qt MVC GUI (CMake target oscilloscope_qt)
│   ├── ioOscilloscopeModel.*       # Model: FTDI + dual circular buffers + pipeline thread
│   ├── ioMainWindow.*              # Controller: view switching + model wiring
│   ├── ioAbstractOscilloscopeView.h # View interface (input / output / waveform)
│   ├── ioCompactOscilloscopeView.* / ioWorkspaceOscilloscopeView.* # Two concrete Qt views
│   └── ioWaveformWidget.*          # Signal display widget
├── qt_main.cpp                     # Qt application entry
├── CMakeLists.txt                  # Qt 6 + Widgets + libioLibrary + libftd2xx.a
├── main.cpp                        # Blink demo: LED at 1Hz and 2Hz
├── pipeline.cpp                    # Multithreaded data acquisition pipeline
├── Makefile                        # Build system (g++ -std=c++11 -pthread)
├── controller.c                    # Legacy: menu-driven controller (C)
├── LED_Project.c                   # Legacy: interactive LED pin control (C)
├── morse_Project.c                 # Legacy: Morse code via LED (C)
├── ftd2xx.h                        # FTDI D2XX vendor header
├── WinTypes.h                      # Windows type definitions for macOS/Linux
├── libftd2xx.a                     # FTDI vendor static library (macOS)
├── i386/ & amd64/                  # FTDI vendor libraries (Windows)
└── docs/
    ├── PLAN.md                     # Project specification
    ├── Dev_plan.md                 # Development plan for ioLibrary
    ├── NEW_FEATURE.md              # Pipeline assignment specification
    └── NEW_FEATURE_PLAN.md         # Pipeline design document with UML
```

## Prerequisites

### Compiler

- **macOS**: `xcode-select --install` or `brew install gcc` (g++ is included)
- **Windows**: Install [MSVC Build Tools](https://visualstudio.microsoft.com/downloads/#remote-tools-for-visual-studio-2022) (select "Build Tools for Visual Studio 2022", check Desktop Development with C++)

### Hardware

- UM245R (FT245R) USB module
- USB **data** cable (not a charge-only cable)
- LED + 220-330 ohm resistor
- Breadboard + jumper wires

## Hardware Setup

Wire the LED to **DB0** on the UM245R module:

```
UM245R DB0 ──── jumper wire ──── resistor (220 ohm) ──── LED long leg (+)
                                                          LED short leg (-) ──── jumper wire ──── UM245R GND
```

**Important**: The circuit must form a closed loop: DB0 -> resistor -> LED -> GND.

## Build & Run

### Build All

```bash
make clean && make pipeline blink_test
```

This compiles the I/O library sources into `libioLibrary.a` (static library), then links two executables:
- `blink_test` — LED blink demo (single-threaded)
- `pipeline` — multithreaded data acquisition pipeline

### Qt GUI (`oscilloscope_qt`, optional)

Requires **Qt 6** (Widgets module), **CMake** 3.16+, and a **C++17** compiler. The app uses the D2XX static library shipped in this repo (`libftd2xx.a` on macOS; `amd64/ftd2xx.lib` or `i386/ftd2xx.lib` on Windows — selected automatically by `CMakeLists.txt`).

#### Architecture (short)

- **Model** (`ioOscilloscopeModel`): opens FTDI device(s), runs `ThreadedReader` → raw ring buffer → `ScaleShiftPipeline` (scale/shift + optional DB0 toggle) → second ring buffer → `ThreadedWriter` (file, same device write-back, or second FTDI). Emits waveform samples and log lines.
- **Controller** (`ioMainWindow`): switches between views, connects model signals (errors, samples, log).
- **Views**: `CompactOscilloscopeView` and `WorkspaceOscilloscopeView` implement `AbstractOscilloscopeView` — same controls, different layout. **View** menu toggles them.

#### Qt UI — control reference

| Control | Purpose |
|--------|---------|
| **Scale** | Per-byte pipeline gain: `round(scale × byte + shift)` clamped to 0–255. |
| **Shift** | Constant offset added after scaling (same formula). |
| **Sample rate (Hz)** | How often the reader samples the FTDI (and writer consumes processed bytes at the same rate). |
| **Ring buffer size** | Capacity in bytes for **each** of the two circular buffers (raw + processed). |
| **Read device index** | `FT_Open` index for the device that supplies input bytes (usually `0` if only one board is attached). |
| **Write device index** | Used when **Dual FTDI** is checked — second `FT_Open` index for the write-side board. |
| **Dual FTDI** | When on, processed data goes to the **second** device (`Write device index`). When off, you can write back to the read device or to a file (see below). |
| **Single device: write processed data back** | When on (and not dual FTDI), `ThreadedWriter` targets the **same** `FtdiDevice` as the reader so DB0 can follow the pipeline output; D2XX access is mutex-protected. |
| **Toggle DB0 every sample** | Pipeline forces DB0 to alternate each processed sample (LED cadence follows sample rate; square-wave period ≈ `2 / sampleRate` seconds). |
| **Start / Stop** | Start/stop the acquisition threads. **Stop** before unplugging hardware is recommended. |
| **Signal display** | Plots recent normalized samples; when DB0 toggle is enabled, bit0 is shown full-scale so the square wave is visible. |
| **Log** | Status text from the model (open paths, mode summary). |

Without a compatible FTDI device, **Start** will fail at `FT_Open` and an error dialog appears — this is expected on machines with no hardware.

#### Build and run — macOS

Install Qt 6 and CMake (example with Homebrew):

```bash
brew install qt cmake
cd /path/to/Oscilloscope-CS565
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
./build/oscilloscope_qt
```

If CMake cannot find Qt, set `CMAKE_PREFIX_PATH` to your Qt install, e.g. `~/Qt/6.8.0/macos`.

#### Build and run — Windows

1. Install **Visual Studio 2022** (Desktop development with C++) or **Build Tools**, **CMake**, and **Qt 6** (MSVC 64-bit kit, e.g. `Qt 6.x.x` → MSVC 2019 64-bit).
2. Open **x64 Native Tools Command Prompt for VS** (or PowerShell with MSVC in `PATH`).
3. Point CMake at your Qt kit (adjust the path to match your install):

```bat
cd C:\path\to\Oscilloscope-CS565
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.8.2\msvc2019_64
cmake --build build --config Release
```

4. Run the executable (Visual Studio generator places it under `Release`):

```bat
build\Release\oscilloscope_qt.exe
```

**Notes (Windows):** Use the **64-bit** toolchain with `amd64\ftd2xx.lib`. If you use **Ninja** instead of Visual Studio generator, the binary is typically `build\oscilloscope_qt.exe` after `cmake --build build`. Install the official **FTDI D2XX** driver so `FT_Open` succeeds.

### Blink Test (blink_test)

```bash
make blink_test
./blink_test
```

Blinks LEDs at 1 Hz (10 cycles) then 2 Hz (20 cycles), followed by a read demo.

### Data Acquisition Pipeline (pipeline)

```bash
make pipeline
./pipeline --help
```

```
Usage: ./pipeline [options]

Options:
  --output-file <path>    Write to file (default: output.bin)
  --output-ftdi <index>   Write to FTDI device at index
  --freq <hz>             Read/write frequency in Hz (default: 10)
  --duration <sec>        Run duration in seconds (default: 10)
  --bufsize <bytes>       Circular buffer size in bytes (default: 1024)
  --help                  Show this help message
```

**Write to file:**
```bash
./pipeline --output-file output.bin --freq 10 --duration 5
```

**Write to another FTDI device (requires 2 devices):**
```bash
./pipeline --output-ftdi 1 --freq 5 --duration 10
```

**Verify output:**
```bash
xxd output.bin | head
```

### Legacy Controller App (C)

```bash
make controller
./controller
```

Menu-driven app with LED control, Morse code, and raw byte read/write.

## macOS Driver Note

If `FT_Open` fails, macOS may have a built-in FTDI kernel driver claiming the device. Unload it:

```bash
sudo kextunload -b com.apple.driver.AppleUSBFTDI
```

On newer macOS versions (26+), this driver may not exist. Verify the device is detected:

```bash
system_profiler SPUSBDataType | grep -A 5 -i "FT245\|FTDI\|0403"
```

If no output, check your USB cable (must be a data cable, not charge-only).

## Testing the Blink Demo

### Step 1: Connect Hardware

1. Plug the UM245R module into the breadboard
2. Wire DB0 -> resistor -> LED -> GND (see Hardware Setup above)
3. Connect USB data cable to your Mac

### Step 2: Build and Run

```bash
make clean && make blink_test
./blink_test
```

### Step 3: Expected Output

**Device initialization:**
```
Device opened successfully.
Device reset successfully.
Purged USB buffers successfully.
USB parameters set successfully.
Set synchronous bit bang mode successfully.
```

**1 Hz blink test (~10 seconds):**
```
--- Blinking LEDs at 1 Hz (10 cycles, ~10 seconds) ---
Write cycle 0: 0xFF     <- LED ON,  wait 1s
Write cycle 1: 0x00     <- LED OFF, wait 1s
...
Write cycle 9: 0x00     <- LED OFF
```

**2 Hz blink test (~10 seconds):**
```
--- Blinking LEDs at 2 Hz (20 cycles, ~10 seconds) ---
Write cycle 0: 0xFF     <- LED ON,  wait 0.5s
Write cycle 1: 0x00     <- LED OFF, wait 0.5s
...
Write cycle 19: 0x00    <- LED OFF
```

**Read test (~5 seconds):**
```
--- Reading from device (5 cycles at 1 Hz) ---
Read cycle 0: 0x00
...
Read cycle 4: 0x00
Last read value: 0x00
Device closed.
```

## Testing the Pipeline

### Step 1: Build and Run

```bash
make clean && make pipeline
./pipeline --output-file output.bin --freq 5 --duration 5
```

### Step 2: Expected Output

```
=== Multithreaded Data Acquisition Pipeline ===
Frequency: 5.0 Hz
Duration:  5 seconds
Buffer:    1024 bytes
Output:    file (output.bin)

Device opened successfully.
...
Circular buffer created (1024 bytes).

--- Starting pipeline ---
[Reader] Thread started (N=1, freq=5.0 Hz)
[Writer] Thread started — output: file (M=1, freq=5.0 Hz)
Running for 5 seconds...

[Reader] Cycle 0: read 1 byte(s), first=0x00
[Writer] Cycle 0: wrote 1 byte(s), first=0x00
...

--- Stopping pipeline ---
[Reader] Thread stopped.
[Writer] Thread stopped.
Device closed.

Pipeline complete.
```

### Step 3: Verify Output File

```bash
xxd output.bin | head
```
expected result:
00000000: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000010: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000020: 0000 0000 0000 0000 0000 0000 0000 0000  ................

This output is expected for your current setup.

  - pipeline only does read from FTDI -> write to file; it does not generate a blink pattern
    (0xFF/0x00) by itself.
  - The device is configured as FT_SetBitMode(..., 0xFF, 0x01) (all 8 pins as outputs). With no
    changing external signal, the read value stays at 0x00.
  - In xxd, 0000 is just display formatting for two consecutive bytes: 00 00.

### Verification Checklist

| # | Check | How to Confirm |
|---|-------|----------------|
| 1 | Build succeeds | `make pipeline blink_test` returns 0, no errors |
| 2 | Device opens | "Device opened successfully." printed |
| 3 | Blink 1Hz works | LED toggles every 1 second |
| 4 | Blink 2Hz works | LED toggles every 0.5 second, visibly faster |
| 5 | Pipeline reader runs | "[Reader] Thread started" + cycle logs |
| 6 | Pipeline writer runs | "[Writer] Thread started" + cycle logs |
| 7 | File output works | `xxd output.bin` shows byte data |
| 8 | Threads stop cleanly | "[Reader] Thread stopped." + "[Writer] Thread stopped." |
| 9 | Clean exit | "Pipeline complete." + "Device closed." |

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| `FT_Open failed (status 3)` | Device not detected | Check USB cable is a data cable; try a different cable |
| Terminal output correct but LED dark | Wiring issue | Verify DB0 -> resistor -> LED(+) -> LED(-) -> GND closed loop |
| LED stays on, never blinks | Connected to VCC instead of DB0 | Rewire to DB0 |
| LED very dim | Resistor too large | Use 220 ohm resistor |
| Pipeline hangs on stop | Deadlock in circular buffer | Should not happen; file a bug |

## Architecture

### ioLibrary — 7 C++ Classes

The library is organized into two layers:

**Single-threaded I/O (Phase 1):**

- **FtdiDevice** — Encapsulates the FTDI device handle. All `FT_*` calls (`FT_Open`, `FT_Read`, `FT_Write`, `FT_Close`, etc.) reside exclusively in this class. Destructor calls `close()` automatically.
- **ioBuffer** — Manages a heap-allocated byte buffer. Created by the caller and passed into readers/writers (aggregation).
- **ioWrite** — Takes a `FtdiDevice*` (composition) and `ioBuffer*` (aggregation). Writes M bytes at a configured frequency in a blocking loop.
- **ioRead** — Takes a `FtdiDevice*` (composition) and `ioBuffer*` (aggregation). Reads N bytes at a configured frequency in a blocking loop.

**Multithreaded Pipeline (Phase 2):**

- **CircularBuffer** — Thread-safe ring buffer using `std::mutex` and `std::condition_variable`. Supports blocking `write()` (producer) and `read()` (consumer). `setDone()` signals the producer is finished so the consumer can drain and exit.
- **ThreadedReader** — Runs a `std::thread` that reads from an FTDI device and pushes data into a CircularBuffer. Composition to FtdiDevice, aggregation to CircularBuffer.
- **ThreadedWriter** — Runs a `std::thread` that pulls data from a CircularBuffer and writes to either a file or another FTDI device. Two constructors support both output targets.

### Data Flow

```
┌──────────────┐       ┌──────────────┐       ┌──────────────┐
│ThreadedReader │       │CircularBuffer│       │ThreadedWriter │
│              │       │              │       │              │
│ FTDI Dev ──> │ ────> │ [ring buffer]│ ────> │ ──> File     │
│ (read thread)│       │ (thread-safe)│       │     or       │
│              │       │              │       │ ──> FTDI Dev │
└──────────────┘       └──────────────┘       └──────────────┘
    Producer               Pipeline              Consumer
```

### UML Relationships

| From | To | Type | Description |
|------|----|------|-------------|
| ioWrite / ioRead | FtdiDevice | Composition | Uses device, does not own it |
| ioWrite / ioRead | ioBuffer | Aggregation | Buffer created externally |
| ThreadedReader | FtdiDevice | Composition | Uses device, does not own it |
| ThreadedReader | CircularBuffer | Aggregation | Buffer created externally |
| ThreadedWriter | FtdiDevice | Composition (0..1) | Optional, nullptr if writing to file |
| ThreadedWriter | CircularBuffer | Aggregation | Buffer created externally |
