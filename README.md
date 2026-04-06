# FT245R Controller - CS565

A C++ project for controlling the FTDI FT245R (UM245R) USB-to-parallel chip, built for Stevens CS565 (Software Architecture and Component-Based Design). Includes a reusable static I/O library and a multithreaded data acquisition pipeline.

Figma link: https://www.figma.com/make/zhjeJXZHEork6O5gXgszK4/Untitled?t=eDVLhgxRvg6EaR4i-1

## Project Structure

```
FT245R/
├── ioLibrary/                      # Reusable FTDI I/O library (static, C++)
│   ├── FtdiDevice.h / .cpp         # Device class: wraps all FT_* calls
│   ├── ioBuffer.h / .cpp           # Simple buffer management
│   ├── ioWrite.h  / .cpp           # Single-threaded frequency-timed write
│   ├── ioRead.h   / .cpp           # Single-threaded frequency-timed read
│   ├── CircularBuffer.h / .cpp     # Thread-safe ring buffer (mutex + condvar)
│   ├── ThreadedReader.h / .cpp     # Multithreaded FTDI reader (producer)
│   └── ThreadedWriter.h / .cpp     # Multithreaded writer to file or FTDI (consumer)
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

This compiles 7 C++ source files into `libioLibrary.a` (static library), then links two executables:
- `blink_test` — LED blink demo (single-threaded)
- `pipeline` — multithreaded data acquisition pipeline

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
