# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

This is a C project that uses the FTDI D2XX driver to control an FT245R USB FIFO chip. The chip runs in synchronous bit-bang mode; over USB it drives eight GPIO pins (DB0–DB7). It is a Stevens CS565 (software architecture and component design) course project.

## Build commands

**macOS:**
```bash
gcc *.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller
```
Add `-g` for debug symbols.

**Windows (MSVC, run in Developer PowerShell):**
```bash
cl *.c /Zi /EHsc /nologo /link i386/ftd2xx.lib /out:controller.exe    # x86
cl *.c /Zi /EHsc /nologo /link amd64/ftd2xx.lib /out:controller.exe   # x64
```

**Windows (GCC/MinGW):**
```bash
gcc *.c -lftd2xx -o controller.exe -g
```

The VS Code task `build-controller` is defined in `.vscode/tasks.json`.

## Run and test

### Prerequisites

1. **Compiler**
   - macOS: `brew install gcc` or `xcode-select --install` for Xcode command-line tools
   - Windows: install [MSVC Build Tools](https://visualstudio.microsoft.com/downloads/#remote-tools-for-visual-studio-2022) (e.g. “Build Tools for Visual Studio 2022” with desktop development workload)

2. **Hardware** — plug in the FT245R USB device. On startup the program calls `FT_Open`; if no device is present it exits with an error.

3. **macOS driver** — if `FT_Open` fails, you may need to unload Apple’s FTDI kext:
   ```bash
   sudo kextunload -b com.apple.driver.AppleUSBFTDI
   ```

### Build and run

```bash
# macOS build
gcc *.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller

# run
./controller
```

### Program flow

After launch you get an interactive menu:

```
Control Menu
1. Control LEDs        — toggle individual pins (0–7)
2. Send Morse Code     — type text; LED blinks Morse on DB0
3. Write byte to port  — write one byte (all eight pins)
4. Read byte from port — read one byte from the port
5. Exit                — quit
```

### Feature checks

| Feature | Steps | Expected |
|--------|--------|----------|
| LED control | Choose 1 → pin (e.g. `0`) → state (`1` on / `0` off) → `done` | LED follows pin; terminal shows state |
| Morse | Choose 2 → text (e.g. `SOS`) → `E0` to exit | DB0 blinks Morse timing (see below) |
| Write byte | Choose 3 | Writes `0x00` (all pins low) |
| Read byte | Choose 4 | Prints read byte, e.g. `Read 1 bytes: 0x00` |

### Morse code details

#### Supported characters

- Letters A–Z (case-insensitive)
- Digits 0–9
- Other characters (spaces, punctuation) are skipped silently

#### Timing (`morse_Project.c`)

| Element | DB0 | Duration | Code |
|--------|-----|----------|------|
| Dot `.` | HIGH `0x01` | 100 ms | `usleep(100000)` |
| Dash `-` | HIGH `0x01` | 300 ms | `usleep(300000)` |
| Intra-symbol gap | LOW `0x00` | 100 ms | `usleep(100000)` |
| Inter-letter gap | LOW `0x00` | +200 ms (300 ms total) | `usleep(200000)` |

#### Flow

1. Menu option `2` (Send Morse Code)
2. Prompt `Enter your message (type 'E0' to finish):`
3. Enter text; each line is converted and sent
4. Multiple lines allowed
5. `E0` exits Morse mode (can appear at end of line, e.g. `HELLOE0`)

#### Example terminal output for `SOS`

```
Morse code for 'S': ...
Morse code for 'O': ---
Morse code for 'S': ...
```

#### Expected LED timing for `SOS`

```
S: 100ms on 100ms off ×3 [+200ms letter gap]
O: 300ms on 100ms off ×3 [+200ms letter gap]
S: same as first S [+200ms letter gap]
```
Total roughly ~3000 ms.

#### Morse table (built into code)

```
A .-      N -.      0 -----
B -...    O ---     1 .----
C -.-.    P .--.    2 ..---
D -..     Q --.-    3 ...--
E .       R .-.     4 ....-
F ..-.    S ...     5 .....
G --.     T -       6 -....
H ....    U ..-     7 --...
I ..      V ...-    8 ---..
J .---    W .--     9 ----.
K -.-     X -..-
L .-..    Y -.--
M --      Z --..
```

### Without hardware

Without an FT245R, `FT_Open` fails immediately. You can still verify the build:
```bash
gcc *.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller
echo $?   # 0 means success
```

## Architecture

Menu-driven CLI built from three sources:

- **`controller.c`** — `main`, device init, menu loop. Opens the device with `FT_Open`, sets bit-bang via `FT_SetBitMode` (mask `0xFF`, all outputs), dispatches features, raw byte read/write.
- **`LED_Project.c`** — `controlLED`: interactive per-pin (0–7) on/off via bitmask on the output byte.
- **`morse_Project.c`** — `sendMorseCode`: text to Morse, `FT_Write` + `usleep` on pin 0 (dot 100 ms, dash 300 ms).

All share the `FT_HANDLE` from `controller.c::initializeDevice()`. Cross-platform: `#ifdef _WIN32` uses `Sleep` on Windows and `usleep` on Unix.

## Key dependencies

- **FTDI D2XX** — `ftd2xx.h` plus the platform static library (macOS: `libftd2xx.a`; Windows: `i386/ftd2xx.lib` or `amd64/ftd2xx.lib`). Vendor library, not project code.
- **`WinTypes.h`** — Windows types (`DWORD`, `BYTE`, `HANDLE`, …) on non-Windows so `ftd2xx.h` compiles on macOS/Linux.
- **`ftd2xx.cfg`** — driver config including VID/PID `0403:6001` `ConfigFlags`.

## Hardware

FT245R exposes eight data pins (DB0–DB7). In bit-bang mode each bit in the byte written with `FT_Write` maps to a pin. A physical FT245R must be connected or `FT_Open` fails at startup.
