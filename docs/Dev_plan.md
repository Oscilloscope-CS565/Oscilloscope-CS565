# Dev Plan: ioLibrary Refactor

## Context

Per `docs/PLAN.md` Task 2, the FTDI read/write functionality is separated into a reusable static library called `ioLibrary`. The library contains four classes (struct + functions in C): `FtdiDevice`, `ioBuffer`, `ioRead`, and `ioWrite`. A sample application links the library and blinks LEDs at 1 Hz and 2 Hz to verify correctness.

---

## 1. File Structure

```
FT245R/
├── ioLibrary/
│   ├── FtdiDevice.h / .c     — Device class: open, close, read, write (wraps all FT_* calls)
│   ├── ioBuffer.h / .c        — Buffer management (malloc/free)
│   ├── ioWrite.h  / .c        — Frequency-timed write loop
│   └── ioRead.h   / .c        — Frequency-timed read loop
├── main.c                      — Sample app (BlinkDemoApp: LED blink test)
├── Makefile                    — Builds libioLibrary.a and links blink_test
├── controller.c                — Legacy app (preserved for reference)
├── LED_Project.c               — Legacy (preserved)
├── morse_Project.c             — Legacy (preserved)
└── (ftd2xx.h, WinTypes.h, libftd2xx.a unchanged)
```

---

## 2. Struct Definitions

### FtdiDevice (`ioLibrary/FtdiDevice.h`)
```c
typedef struct {
    FT_HANDLE handle;           // Opaque FTDI device handle
} FtdiDevice;
```

### ioBuffer (`ioLibrary/ioBuffer.h`)
```c
typedef struct {
    BYTE   *storage;            // Heap-allocated byte array
    size_t  length;             // Buffer capacity in bytes
} ioBuffer;
```

### ioRead (`ioLibrary/ioRead.h`)
```c
typedef struct {
    FtdiDevice *device;         // Composition: holds FtdiDevice pointer
    ioBuffer   *buffer;         // Aggregation: externally-created ioBuffer
    size_t      N;              // Bytes to read per cycle
    double      frequencyHz;    // Read frequency in Hz
} ioRead;
```

### ioWrite (`ioLibrary/ioWrite.h`)
```c
typedef struct {
    FtdiDevice *device;         // Composition: holds FtdiDevice pointer
    ioBuffer   *buffer;         // Aggregation: externally-created ioBuffer
    size_t      M;              // Bytes to write per cycle
    double      frequencyHz;    // Write frequency in Hz
} ioWrite;
```

---

## 3. Function Signatures

### FtdiDevice
```c
FT_STATUS FtdiDevice_open(FtdiDevice *dev, int deviceIndex);   // Open + init (reset, purge, set bit-bang)
FT_STATUS FtdiDevice_close(FtdiDevice *dev);                   // Close device
FT_STATUS FtdiDevice_read(FtdiDevice *dev, BYTE *bytes, size_t n);   // Read n bytes
FT_STATUS FtdiDevice_write(FtdiDevice *dev, BYTE *bytes, size_t m);  // Write m bytes
```
All `FT_*` calls (FT_Open, FT_Close, FT_Read, FT_Write, FT_Purge, etc.) reside exclusively here. On error, returns `FT_STATUS` instead of calling `exit()` — the caller decides how to handle it.

### ioBuffer
```c
int     ioBuffer_create(ioBuffer *buf, size_t capacity);   // malloc + memset
BYTE*   ioBuffer_data(const ioBuffer *buf);                // Get data pointer
size_t  ioBuffer_size(const ioBuffer *buf);                // Get capacity
void    ioBuffer_destroy(ioBuffer *buf);                   // Free memory
```

### ioRead
```c
void      ioRead_configure(ioRead *reader, ioBuffer *buffer, size_t N, double frequencyHz);
FT_STATUS ioRead_readLoop(ioRead *reader, int cycles);     // Blocking loop: read -> sleep -> repeat
```

### ioWrite
```c
void      ioWrite_configure(ioWrite *writer, ioBuffer *buffer, size_t M, double frequencyHz);
FT_STATUS ioWrite_writeLoop(ioWrite *writer, int cycles);  // Blocking loop: write buffer[i % size] -> sleep -> repeat
```

---

## 4. Implementation Steps (Dependency Order)

| Step | Files | Depends On | Description |
|------|-------|-----------|-------------|
| 1 | `ioLibrary/` directory | — | Create subdirectory |
| 2 | `ioBuffer.h` + `ioBuffer.c` | None | Pure memory management, no FTDI dependency |
| 3 | `FtdiDevice.h` + `FtdiDevice.c` | `ftd2xx.h` | Extract init logic from `controller.c:89-135`, add `read()`/`write()` |
| 4 | `ioWrite.h` + `ioWrite.c` | ioBuffer, FtdiDevice | Write loop + frequency delay, calls `FtdiDevice_write()` |
| 5 | `ioRead.h` + `ioRead.c` | ioBuffer, FtdiDevice | Read loop + frequency delay, calls `FtdiDevice_read()` |
| 6 | `Makefile` | All above | Compile .o -> `ar rcs libioLibrary.a` -> link main |
| 7 | `main.c` | All ioLibrary headers | BlinkDemoApp with `runBlink1Hz()` and `runBlink2Hz()` |
| 8 | Update `.gitignore` | — | Add `blink_test`, `libioLibrary.a`, `ioLibrary/*.o` |

---

## 5. main.c Core Logic (BlinkDemoApp)

```
 1. FtdiDevice_open(&dev, 0)                          — Open device
 2. ioBuffer_create(&buf, 2)                           — Create 2-byte buffer
 3. buf[0] = 0xFF; buf[1] = 0x00                      — Fill blink pattern
 4. writer.device = &dev                               — Composition: wire device to writer
 5. runBlink1Hz(&writer, &buf)                         — Configure 1 Hz, run writeLoop(10)
 6. runBlink2Hz(&writer, &buf)                         — Configure 2 Hz, run writeLoop(20)
 7. ioBuffer_destroy(&buf)                             — Free buffer
 8. FtdiDevice_close(&dev)                             — Close device
```

Aggregation: `buf` is created and destroyed in `main()`, writer only holds a pointer.
Composition: `dev` is created in `main()`, writer holds a pointer to it.

---

## 6. Build Commands

**Build with Makefile (one command):**
```bash
make blink_test
```

**Manual build on macOS:**
```bash
gcc -c ioLibrary/ioBuffer.c ioLibrary/ioRead.c ioLibrary/ioWrite.c ioLibrary/FtdiDevice.c -I. -g
ar rcs libioLibrary.a ioBuffer.o ioRead.o ioWrite.o FtdiDevice.o
gcc main.c libioLibrary.a libftd2xx.a -I. -IioLibrary -L. \
    -framework CoreFoundation -framework IOKit -o blink_test
```

---

## 7. Frequency-to-Delay Mapping

Delay calculation inside `ioWrite_writeLoop` / `ioRead_readLoop`:
```c
useconds_t delay_us = (useconds_t)(1000000.0 / frequencyHz);
usleep(delay_us);   // Windows: Sleep(1000 / frequencyHz)
```

| Frequency | Delay per cycle | Cycles for ~10s | LED Effect |
|-----------|----------------|-----------------|------------|
| 1 Hz | 1 second | 10 (5 on/off) | Slow blink |
| 2 Hz | 0.5 seconds | 20 (10 on/off) | Fast blink |

---

## 8. Verification

1. **Build**: `make clean && make blink_test` — zero errors, zero warnings
2. **Hardware test**: Run `./blink_test`, observe LED blinks slow (1 Hz) then fast (2 Hz)
3. **Terminal output**: Confirm `0xFF`/`0x00` alternating in write cycle logs
4. **Symbol check**: `nm libioLibrary.a | grep " T "` — verify all 12 functions present
5. **Without hardware**: Build succeeds; `FtdiDevice_open` returns error but architecture is validated

---

## 9. Key Design Decisions

- **FtdiDevice class**: All FTDI calls (`FT_Open`, `FT_Read`, `FT_Write`, `FT_Close`, `FT_Purge`) reside exclusively in `FtdiDevice.c`. ioRead/ioWrite never call `FT_*` functions directly.
- **Composition**: ioRead/ioWrite hold a `FtdiDevice *device` pointer. The FtdiDevice is created in `main()` and passed in.
- **Aggregation**: ioBuffer is created/destroyed by `main()`. ioRead/ioWrite only hold a pointer to it (they do not own it).
- **Blocking execution**: No threading. `writeLoop`/`readLoop` are blocking loops with `usleep`.
- **Legacy code preserved**: `controller.c`, `LED_Project.c`, `morse_Project.c` remain for reference but are not part of the new build.
