# FT245R Controller - CS565

A C++ project for controlling the FTDI FT245R (UM245R) USB-to-parallel chip, built for Stevens CS565 (Software Architecture and Component-Based Design).

Figma link: https://www.figma.com/make/zhjeJXZHEork6O5gXgszK4/Untitled?t=eDVLhgxRvg6EaR4i-1

## Project Structure

```
FT245R/
├── ioLibrary/                 # Reusable FTDI I/O library (static, C++)
│   ├── FtdiDevice.h / .cpp    # Device class: open, close, read, write (wraps all FT_* calls)
│   ├── ioBuffer.h / .cpp      # Buffer management (malloc/free)
│   ├── ioWrite.h  / .cpp      # Frequency-timed write loop
│   └── ioRead.h   / .cpp      # Frequency-timed read loop
├── main.cpp                   # Sample app: LED blink test at 1Hz and 2Hz
├── Makefile                   # Build system (g++ -std=c++11)
├── controller.c               # Legacy app: menu-driven controller (C)
├── LED_Project.c              # Legacy: interactive LED pin control (C)
├── morse_Project.c            # Legacy: Morse code via LED (C)
├── ftd2xx.h                   # FTDI D2XX vendor header (C library, extern "C" compatible)
├── WinTypes.h                 # Windows type definitions for macOS/Linux
├── libftd2xx.a                # FTDI vendor static library (macOS)
├── i386/ & amd64/             # FTDI vendor libraries (Windows)
└── docs/
    ├── PLAN.md                # Project specification
    └── Dev_plan.md            # Development plan for ioLibrary
```

## Prerequisites

### Compiler

- **macOS**: `brew install gcc` or `xcode-select --install` (g++ is included)
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

### ioLibrary Sample App (blink_test)

```bash
# Build
make blink_test

# Run
./blink_test
```

This compiles 4 C++ source files into `libioLibrary.a` (static library), then links it with `main.cpp` to produce `blink_test`.

**Manual build without Make:**

```bash
# macOS
g++ -std=c++11 -I. -IioLibrary -g -c ioLibrary/ioBuffer.cpp -o ioLibrary/ioBuffer.o
g++ -std=c++11 -I. -IioLibrary -g -c ioLibrary/ioRead.cpp -o ioLibrary/ioRead.o
g++ -std=c++11 -I. -IioLibrary -g -c ioLibrary/ioWrite.cpp -o ioLibrary/ioWrite.o
g++ -std=c++11 -I. -IioLibrary -g -c ioLibrary/FtdiDevice.cpp -o ioLibrary/FtdiDevice.o
ar rcs libioLibrary.a ioLibrary/ioBuffer.o ioLibrary/ioRead.o ioLibrary/ioWrite.o ioLibrary/FtdiDevice.o
g++ -std=c++11 -I. -IioLibrary -g main.cpp libioLibrary.a libftd2xx.a -L. -framework CoreFoundation -framework IOKit -o blink_test
```

```bash
# Windows (MSVC x64, Developer PowerShell)
cl /EHsc /std:c++14 /I. /IioLibrary ioLibrary\ioBuffer.cpp ioLibrary\ioRead.cpp ioLibrary\ioWrite.cpp ioLibrary\FtdiDevice.cpp main.cpp /link amd64\ftd2xx.lib /out:blink_test.exe
```

### Legacy Controller App (C)

```bash
# macOS
gcc controller.c LED_Project.c morse_Project.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller

# Windows (MSVC x64)
cl controller.c LED_Project.c morse_Project.c /Zi /EHsc /nologo /link amd64/ftd2xx.lib /out:controller.exe

# Run
./controller
```

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

## Tutorial: Testing the ioLibrary

### Step 1: Connect Hardware

1. Plug the UM245R module into the breadboard
2. Wire DB0 -> resistor -> LED -> GND (see Hardware Setup above)
3. Connect USB data cable to your Mac

### Step 2: Build

```bash
cd FT245R
make clean && make blink_test
```

Expected output — all files compile with no errors:
```
g++ -I. -IioLibrary -g -std=c++11 -c ioLibrary/ioBuffer.cpp -o ioLibrary/ioBuffer.o
g++ -I. -IioLibrary -g -std=c++11 -c ioLibrary/ioRead.cpp -o ioLibrary/ioRead.o
g++ -I. -IioLibrary -g -std=c++11 -c ioLibrary/ioWrite.cpp -o ioLibrary/ioWrite.o
g++ -I. -IioLibrary -g -std=c++11 -c ioLibrary/FtdiDevice.cpp -o ioLibrary/FtdiDevice.o
ar rcs libioLibrary.a ioLibrary/ioBuffer.o ioLibrary/ioRead.o ioLibrary/ioWrite.o ioLibrary/FtdiDevice.o
g++ -I. -IioLibrary -g -std=c++11 main.cpp libioLibrary.a libftd2xx.a -L. -framework CoreFoundation -framework IOKit -o blink_test
```

### Step 3: Run

```bash
./blink_test
```

### Step 4: Expected Output

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
Write cycle 2: 0xFF     <- LED ON,  wait 1s
...
Write cycle 9: 0x00     <- LED OFF
```

The LED toggles every 1 second (5 on/off cycles in 10 seconds).

**2 Hz blink test (~10 seconds):**
```
--- Blinking LEDs at 2 Hz (20 cycles, ~10 seconds) ---
Write cycle 0: 0xFF     <- LED ON,  wait 0.5s
Write cycle 1: 0x00     <- LED OFF, wait 0.5s
...
Write cycle 19: 0x00    <- LED OFF
```

The LED toggles every 0.5 seconds — noticeably faster than the 1 Hz test.

**Read test (~5 seconds):**
```
--- Reading from device (5 cycles at 1 Hz) ---
Read cycle 0: 0x00
...
Read cycle 4: 0x00
Last read value: 0x00
Device closed.
```

### Step 5: Verify

| # | Check | How to Confirm |
|---|-------|----------------|
| 1 | Build succeeds | `make blink_test` returns 0, no errors |
| 2 | Device opens | Terminal prints 5 "successfully" lines |
| 3 | 1Hz values correct | Terminal alternates `0xFF` and `0x00` |
| 4 | 1Hz LED blinks | LED toggles every 1 second |
| 5 | 2Hz values correct | Terminal alternates `0xFF` and `0x00`, 20 lines |
| 6 | 2Hz LED blinks | Visibly faster than 1Hz |
| 7 | Read works | 5 read cycle lines, no errors |
| 8 | Clean exit | Last line: "Device closed." |

## Tutorial: Testing the Legacy Controller

### Run

```bash
./controller
```

### Menu Options

```
Control Menu
1. Control LEDs        — Toggle individual pins (0-7) on/off
2. Send Morse Code     — Type text, LED blinks Morse on DB0
3. Write byte to port  — Write 0x00 to all pins
4. Read byte from port — Read current pin states
5. Exit
```

### Test LED Control (Option 1)

1. Enter `1`
2. Enter pin number: `0`
3. Enter state: `1` (ON) — LED on DB0 lights up
4. Enter state: `0` (OFF) — LED turns off
5. Type `done` to return to menu

### Test Morse Code (Option 2)

1. Enter `2`
2. Type `SOS` and press Enter
3. Watch LED on DB0 blink: `... --- ...` (3 short, 3 long, 3 short)
4. Type `E0` to exit Morse mode

Morse timing: dot = 100ms, dash = 300ms, gap between letters = 300ms.

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| `FT_Open failed (status 3)` | Device not detected | Check USB cable is a data cable; try a different cable |
| Terminal output correct but LED dark | Wiring issue | Verify DB0 -> resistor -> LED(+) -> LED(-) -> GND closed loop |
| LED stays on, never blinks | Connected to VCC instead of DB0 | Rewire to DB0 |
| LED very dim | Resistor too large | Use 220 ohm resistor |
| Program hangs | Normal blocking sleep | Wait ~25 seconds for full test to complete |

## Architecture: ioLibrary

The ioLibrary separates FTDI I/O into a reusable static library with four C++ classes:

- **FtdiDevice** — Encapsulates the FTDI device handle and all hardware calls: `open()`, `close()`, `read()`, `write()`. All `FT_*` function calls (`FT_Open`, `FT_Read`, `FT_Write`, etc.) reside exclusively in this class. Constructor initializes the handle to `nullptr`; destructor calls `close()` automatically.
- **ioBuffer** — Manages a heap-allocated byte buffer (`storage`/`length`). Created by the caller and passed into ioRead/ioWrite (aggregation). Destructor calls `destroy()` automatically.
- **ioWrite** — Constructor takes a `FtdiDevice*` (composition). Holds an `ioBuffer*` set via `configure()` (aggregation). `configure()` sets buffer, byte count (M), and frequency. `writeLoop()` writes bytes at the configured frequency.
- **ioRead** — Constructor takes a `FtdiDevice*` (composition). Holds an `ioBuffer*` set via `configure()` (aggregation). `configure()` sets buffer, byte count (N), and frequency. `readLoop()` reads bytes at the configured frequency.

The sample application (`main.cpp`) creates the FtdiDevice and ioBuffer objects, passes them to ioWrite/ioRead via constructors and `configure()`, then calls `runBlink1Hz()`/`runBlink2Hz()` to demonstrate LED blinking.
