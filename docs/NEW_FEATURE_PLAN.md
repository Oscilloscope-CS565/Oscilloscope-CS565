# Multithreaded Data Acquisition Pipeline — Design & Implementation Plan

## 1. Overview

This document describes the design and implementation of a multithreaded data acquisition pipeline that extends the existing ioLibrary. The pipeline reads data from an FTDI device in a dedicated thread, transfers it through a thread-safe circular buffer, and writes it to either a file or another FTDI device in a separate thread.

### Data Flow

```
┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  ThreadedReader   │     │  CircularBuffer   │     │  ThreadedWriter   │
│                  │     │                  │     │                  │
│  FTDI Device ──> │ ──> │  [ring buffer]   │ ──> │ ──> File         │
│  (read thread)   │     │  (thread-safe)   │     │     or           │
│                  │     │                  │     │ ──> FTDI Device  │
└──────────────────┘     └──────────────────┘     └──────────────────┘
```

---

## 2. New Classes

Three new C++ classes are added to `ioLibrary/`:

### 2.1 CircularBuffer

**File**: `ioLibrary/CircularBuffer.h / .cpp`

Thread-safe ring buffer for producer-consumer data transfer between the read and write threads.

**Attributes**:
| Field | Type | Description |
|-------|------|-------------|
| storage | BYTE* | Heap-allocated byte array |
| capacity | size_t | Maximum buffer size |
| head | size_t | Write position (producer) |
| tail | size_t | Read position (consumer) |
| count | size_t | Current number of bytes stored |
| mtx | std::mutex | Protects all shared state |
| notEmpty | std::condition_variable | Signals consumer when data available |
| notFull | std::condition_variable | Signals producer when space available |
| finished | bool | Producer signals no more data |

**Operations**:
| Method | Signature | Description |
|--------|-----------|-------------|
| Constructor | `CircularBuffer(size_t capacity)` | Allocates storage |
| Destructor | `~CircularBuffer()` | Frees storage |
| write | `bool write(const BYTE *data, size_t len)` | Blocks if full; returns false if finished |
| read | `bool read(BYTE *data, size_t len)` | Blocks if empty; returns false if finished + empty |
| setDone | `void setDone()` | Signals producer is done, wakes all waiters |
| getCount | `size_t getCount()` | Returns current byte count (thread-safe) |
| getCapacity | `size_t getCapacity() const` | Returns maximum capacity |

**Thread Safety**: Uses mutex + condition variables for blocking synchronization. `write()` waits on `notFull` when buffer is full; `read()` waits on `notEmpty` when buffer is empty. `setDone()` broadcasts on both to unblock any waiting threads.

### 2.2 ThreadedReader

**File**: `ioLibrary/ThreadedReader.h / .cpp`

Reads N bytes from an FTDI device at a specified frequency in a dedicated thread, and pushes the data into a CircularBuffer.

**Attributes**:
| Field | Type | Relationship | Description |
|-------|------|-------------|-------------|
| device | FtdiDevice* | Composition | FTDI device to read from |
| circBuffer | CircularBuffer* | Aggregation | Buffer to write data into |
| N | size_t | — | Bytes per read operation |
| frequencyHz | double | — | Read frequency in Hz |
| readerThread | std::thread | — | The reader thread |
| running | std::atomic\<bool\> | — | Controls thread lifecycle |

**Operations**:
| Method | Description |
|--------|-------------|
| `ThreadedReader(FtdiDevice *device)` | Constructor — sets device pointer |
| `~ThreadedReader()` | Destructor — calls stop() |
| `configure(CircularBuffer*, size_t N, double freq)` | Sets buffer, read size, frequency |
| `start()` | Spawns the reader thread |
| `stop()` | Sets running=false, signals buffer done, joins thread |

**Thread Function Loop**:
```
while (running):
    device->read(tempBuf, N)        // read from FTDI
    circBuffer->write(tempBuf, N)   // push to circular buffer
    sleep(1/frequencyHz)            // wait for next cycle
```

### 2.3 ThreadedWriter

**File**: `ioLibrary/ThreadedWriter.h / .cpp`

Reads M bytes from a CircularBuffer in a dedicated thread, and writes to either a file or another FTDI device.

**Attributes**:
| Field | Type | Relationship | Description |
|-------|------|-------------|-------------|
| device | FtdiDevice* | Composition | FTDI device (nullptr if file output) |
| circBuffer | CircularBuffer* | Aggregation | Buffer to read data from |
| outputFile | FILE* | — | File handle (nullptr if FTDI output) |
| M | size_t | — | Bytes per write operation |
| frequencyHz | double | — | Write frequency in Hz |
| writerThread | std::thread | — | The writer thread |
| running | std::atomic\<bool\> | — | Controls thread lifecycle |

**Operations**:
| Method | Description |
|--------|-------------|
| `ThreadedWriter(FtdiDevice *device)` | Constructor — write to FTDI device |
| `ThreadedWriter(const char *filePath)` | Constructor — write to file |
| `~ThreadedWriter()` | Destructor — stops thread, closes file |
| `configure(CircularBuffer*, size_t M, double freq)` | Sets buffer, write size, frequency |
| `start()` | Spawns the writer thread |
| `stop()` | Sets running=false, joins thread |

**Thread Function Loop**:
```
while (running):
    circBuffer->read(tempBuf, M)    // pull from circular buffer (blocks if empty)
    if device != nullptr:
        device->write(tempBuf, M)   // write to FTDI
    else:
        fwrite(tempBuf, M, file)    // write to file
    sleep(1/frequencyHz)
```

---

## 3. UML Class Diagram

```
┌─────────────────────────────┐
│        FtdiDevice           │
├─────────────────────────────┤
│ - handle: FT_HANDLE         │
├─────────────────────────────┤
│ + open(deviceIndex): FT_STATUS  │
│ + close(): FT_STATUS            │
│ + read(bytes, n): FT_STATUS     │
│ + write(bytes, m): FT_STATUS    │
└──────────┬──────────────────┘
           │
           │ composition
           │
    ┌──────┴──────────────────────────────────────┐
    │                                             │
    ▼                                             ▼
┌─────────────────────────────┐   ┌─────────────────────────────┐
│      ThreadedReader         │   │      ThreadedWriter         │
├─────────────────────────────┤   ├─────────────────────────────┤
│ - device: FtdiDevice*       │   │ - device: FtdiDevice*       │
│ - circBuffer: CircularBuffer*│  │ - circBuffer: CircularBuffer*│
│ - N: size_t                 │   │ - outputFile: FILE*         │
│ - frequencyHz: double       │   │ - M: size_t                 │
│ - readerThread: std::thread │   │ - frequencyHz: double       │
│ - running: atomic<bool>     │   │ - writerThread: std::thread │
├─────────────────────────────┤   │ - running: atomic<bool>     │
│ + configure(buf, N, freq)   │   ├─────────────────────────────┤
│ + start()                   │   │ + configure(buf, M, freq)   │
│ + stop()                    │   │ + start()                   │
│ - threadFunc()              │   │ + stop()                    │
└──────────┬──────────────────┘   │ - threadFunc()              │
           │                      └──────────┬──────────────────┘
           │ aggregation                     │ aggregation
           │                                 │
           ▼                                 ▼
┌─────────────────────────────────────────────────┐
│              CircularBuffer                      │
├─────────────────────────────────────────────────┤
│ - storage: BYTE*                                │
│ - capacity: size_t                              │
│ - head: size_t                                  │
│ - tail: size_t                                  │
│ - count: size_t                                 │
│ - mtx: std::mutex                               │
│ - notEmpty: std::condition_variable             │
│ - notFull: std::condition_variable              │
│ - finished: bool                                │
├─────────────────────────────────────────────────┤
│ + write(data, len): bool                        │
│ + read(data, len): bool                         │
│ + setDone(): void                               │
│ + getCount(): size_t                            │
│ + getCapacity(): size_t                         │
└─────────────────────────────────────────────────┘
```

### UML Relationships

| From | To | Type | Multiplicity | Rationale |
|------|-----|------|-------------|-----------|
| ThreadedReader | FtdiDevice | Composition | 1..1 | Reader always needs exactly one device |
| ThreadedReader | CircularBuffer | Aggregation | 1..1 | Buffer created externally, passed via configure() |
| ThreadedWriter | FtdiDevice | Composition | 0..1 | Optional — nullptr if writing to file |
| ThreadedWriter | CircularBuffer | Aggregation | 1..1 | Buffer created externally, passed via configure() |

---

## 4. UML Sequence Diagram

```
    Main              Reader Thread       CircularBuffer       Writer Thread        Output
     │                     │                    │                    │                  │
     │── create devices ──>│                    │                    │                  │
     │── create buffer ────────────────────────>│                    │                  │
     │── configure ───────>│                    │                    │                  │
     │── configure ────────────────────────────────────────────────>│                  │
     │── start() ─────────>│                    │                    │                  │
     │── start() ──────────────────────────────────────────────────>│                  │
     │                     │                    │                    │                  │
     │                     │── device->read() ──┐                   │                  │
     │                     │<── data ───────────┘                   │                  │
     │                     │── write(data) ────>│                    │                  │
     │                     │                    │── notify ─────────>│                  │
     │                     │                    │                    │── read(data) ───>│
     │                     │                    │<── data ───────────│                  │
     │                     │                    │                    │── write ────────>│
     │                     │                    │                    │                  │
     │                     │ ... (repeats at frequency) ...         │                  │
     │                     │                    │                    │                  │
     │── stop reader ─────>│                    │                    │                  │
     │                     │── setDone() ──────>│                    │                  │
     │                     │                    │── notify ─────────>│                  │
     │── stop writer ──────────────────────────────────────────────>│                  │
     │                     │                    │                    │── drain ────────>│
     │                     │                    │                    │                  │
     │<── cleanup ─────────────────────────────────────────────────────────────────────│
```

---

## 5. Project Structure (After)

```
FT245R/
├── ioLibrary/                      # Reusable FTDI I/O library (static, C++)
│   ├── FtdiDevice.h / .cpp         # Device class (unchanged)
│   ├── ioBuffer.h / .cpp           # Simple buffer (unchanged)
│   ├── ioRead.h / .cpp             # Single-threaded read (unchanged)
│   ├── ioWrite.h / .cpp            # Single-threaded write (unchanged)
│   ├── CircularBuffer.h / .cpp     # NEW — thread-safe ring buffer
│   ├── ThreadedReader.h / .cpp     # NEW — multithreaded FTDI reader
│   └── ThreadedWriter.h / .cpp     # NEW — multithreaded writer (file or FTDI)
├── main.cpp                        # Blink demo app (unchanged)
├── pipeline.cpp                    # NEW — multithreaded pipeline executable
├── Makefile                        # Updated — adds new sources, -pthread flag
└── docs/
    ├── PLAN.md                     # Original specification
    ├── NEW_FEATURE.md              # Pipeline assignment specification
    └── NEW_FEATURE_PLAN.md         # This document
```

---

## 6. Build & Run

### Build

```bash
# Build the pipeline executable
make pipeline

# Build the blink test (still works)
make blink_test

# Build both
make pipeline blink_test

# Clean all
make clean
```

### Run — Output to File

```bash
./pipeline --output-file output.bin --freq 10 --duration 5
```

Reads 1 byte at 10 Hz from FTDI device 0, passes through a 1024-byte circular buffer, and writes to `output.bin` for 5 seconds (~50 bytes).

### Run — Output to FTDI Device

```bash
./pipeline --output-ftdi 1 --freq 5 --duration 10
```

Reads from FTDI device 0, writes to FTDI device 1 at 5 Hz for 10 seconds.

### Command-Line Options

| Option | Default | Description |
|--------|---------|-------------|
| `--output-file <path>` | output.bin | Write output to file |
| `--output-ftdi <index>` | — | Write output to FTDI device at index |
| `--freq <hz>` | 10 | Read/write frequency in Hz |
| `--duration <sec>` | 10 | Pipeline run duration in seconds |
| `--bufsize <bytes>` | 1024 | Circular buffer capacity |
| `--help` | — | Show usage information |

---

## 7. Expected Output

```
=== Multithreaded Data Acquisition Pipeline ===
Frequency: 10.0 Hz
Duration:  5 seconds
Buffer:    1024 bytes
Output:    file (output.bin)

Device opened successfully.
Device reset successfully.
Purged USB buffers successfully.
USB parameters set successfully.
Set synchronous bit bang mode successfully.
Circular buffer created (1024 bytes).

--- Starting pipeline ---
[Reader] Thread started (N=1, freq=10.0 Hz)
[Writer] Thread started — output: file (M=1, freq=10.0 Hz)
Running for 5 seconds...

[Reader] Cycle 0: read 1 byte(s), first=0x00
[Writer] Cycle 0: wrote 1 byte(s), first=0x00
[Reader] Cycle 1: read 1 byte(s), first=0x00
[Writer] Cycle 1: wrote 1 byte(s), first=0x00
...

--- Stopping pipeline ---
[Reader] Thread stopped.
[Writer] Thread stopped.
Device closed.

Pipeline complete.
```

---

## 8. Verification Checklist

| # | Check | How to Verify |
|---|-------|---------------|
| 1 | Pipeline builds | `make pipeline` succeeds, no errors |
| 2 | blink_test still works | `make blink_test` succeeds |
| 3 | Device opens | "Device opened successfully." in output |
| 4 | Reader thread runs | "[Reader] Thread started" + cycle logs |
| 5 | Writer thread runs | "[Writer] Thread started" + cycle logs |
| 6 | File output works | `ls -la output.bin` shows bytes; `xxd output.bin \| head` shows data |
| 7 | Threads stop cleanly | "[Reader] Thread stopped." + "[Writer] Thread stopped." |
| 8 | Clean exit | "Pipeline complete." + "Device closed." |
| 9 | CircularBuffer thread-safe | No crashes, no data corruption under concurrent access |
