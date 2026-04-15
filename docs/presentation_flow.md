# Presentation Flow — Step-by-Step Script

This document is the **run-of-show** for the in-class presentation. Every row = one step. Each step has:

- **Step / Time** — minute marker & what the slide/demo shows.
- **Presenter** — who is driving (A / B / C / D, assigned on the day).
- **Action** — what to click / open / run.
- **Spoken script** — a ready-to-read paragraph. Adapt freely, but this is the fallback if nerves hit.

Target total: **14 minutes talk + 2 minutes Q&A = 16 minutes**.

---

## Top-level timeline

```
0:00 ─ Title & agenda                       (A, 1 min)
1:00 ─ Problem & domain                     (A, 1 min)
2:00 ─ Literature / FT245R background       (A, 1 min)
3:00 ─ 4+1 Views — Logical (classes)        (B, 2 min)
5:00 ─ 4+1 Views — Process (seq + activity) (B, 1.5 min)
6:30 ─ 4+1 Views — Package + Deployment     (B, 1 min)
7:30 ─ 4+1 Views — Scenarios (use case+req) (B, 0.5 min)
8:00 ─ Code structure & conventions         (C, 1 min)
9:00 ─ Code ↔ UML mapping                   (C, 1 min)
10:00 ─ Live debugging (breakpoints)        (C, 1 min)
11:00 ─ Live demo — Terminal pipeline       (D, 1 min)
12:00 ─ Live demo — Qt UI scale/shift/view  (D, 2 min)
14:00 ─ Wrap: what's next + submission      (All, 0.5 min)
14:30 ─ Q&A                                 (All, 1.5 min)
```

---

## Flow diagram (ASCII)

```
  ┌─────────┐   ┌──────────┐   ┌──────────────────────────┐   ┌─────────────┐   ┌──────────┐
  │  INTRO  │──▶│ CONTEXT  │──▶│     4+1 VIEWS (30 pts)   │──▶│  CODE PART  │──▶│   DEMO   │──▶ Q&A
  │ A · 1m  │   │ A · 2m   │   │  B · 5m                  │   │  C · 3m     │   │  D · 3m  │
  └─────────┘   └──────────┘   │  Logical → Process →     │   │  Struct →   │   │  CLI →   │
                               │  Development → Physical  │   │  Mapping →  │   │  Qt UI   │
                               │   → Scenarios            │   │  Debug      │   │          │
                               └──────────────────────────┘   └─────────────┘   └──────────┘
```

---

## Step-by-step script

### 0 — Before the class starts (T-10 min)

- [ ] Plug FT245R board via a known-good USB **data** cable.
- [ ] Verify `system_profiler SPUSBDataType | grep -i 0403` shows the device.
- [ ] Open 3 terminal tabs pre-loaded with:
    1. `cd <repo>` ready to `make pipeline && ./pipeline --freq 5 --duration 5 --output-file output.bin`
    2. `cd <repo>/build` ready to `./oscilloscope_qt`
    3. A spare tab for `xxd output.bin | head`.
- [ ] Open VS Code with 3 bookmarks set:
    - `ui/ioOscilloscopeModel.cpp` — `startAcquisition()` first line.
    - `ioLibrary/ioScaleShiftPipeline.cpp` — inside `threadFunc()` after `inBuf->read`.
    - `ioLibrary/ioThreadedWriter.cpp` — branch `if (device != nullptr)`.
- [ ] Open slide deck in Presenter View on one monitor.
- [ ] Close Slack / email / notifications.

---

### 1 — Title & agenda (0:00, Presenter A, 1 min)

**Action:** Slide 1 (title) → Slide 2 (agenda).

**Script:**

> "Good afternoon. We are Group <N>, presenting our CS565 final project: an FT245R-based software oscilloscope and function generator. I'm <A>, joined by <B>, <C>, and <D>.
>
> In the next fifteen minutes we'll cover: the problem we're solving, the hardware and software background, our architecture using Kruchten's 4+1 Views, a tour of the code and how it maps back to the UML, and finally a live demo from both the terminal and a Qt GUI. Let's dive in."

---

### 2 — Problem & domain (1:00, Presenter A, 1 min)

**Action:** Slide 3 — a photo of the FT245R breadboard on the left, architecture one-liner on the right.

**Script:**

> "The FT245R is an FTDI USB-to-parallel bridge with eight GPIO pins — DB0 through DB7. Operated in synchronous bit-bang mode, every byte sent over USB becomes a pin-state snapshot, and every read samples the pins.
>
> Our project wraps this hardware with a reusable C++ I/O library and two selectable Qt user interfaces, so a student can point the tool at a signal source, stream samples through a configurable scale/shift pipeline, visualize them in real time, and optionally write the processed stream back to the device to drive an LED — or to a second FT245R board, or to a capture file."

---

### 3 — Literature / FT245R background (2:00, Presenter A, 1 min)

**Action:** Slide 4 — excerpt from `FT245R_BitBangMode.pdf` + reference list.

**Script:**

> "Our technical foundation comes from three FTDI application notes — `FT245R_BitBangMode.pdf`, the D2XX Programmer's Guide, and the FIFO Basics document. The key takeaway is that `FT_SetBitMode(0xFF, 0x01)` puts all eight pins into synchronous bit-bang output mode, and the USB packet size combined with the baud divisor determines the effective sample rate.
>
> On the software side, we drew from three patterns taught in this course: producer-consumer with a bounded ring buffer, the pipe-and-filter pipeline, and MVC as realized by Qt's signal/slot system. Each of those maps directly to a cluster of classes you're about to see."

---

### 4 — 4+1 Views: Logical view (3:00, Presenter B, 2 min)

**Action:** Slide 5 (Kruchten overview table) → Slide 6 (`class_diagram_01_overview.puml`) → Slide 7 (`class_diagram_02_iolib_pipeline.puml`) → Slide 8 (`class_diagram_ui_only.puml`).

**Script:**

> "We followed Kruchten's 4+1 Views model. This table shows which artifact we produced for each view. Let's walk through them.
>
> **(Slide 6 — overview.)** At the package level, we have two main layers — a reusable `ioLibrary` static library, and a Qt application under `ui/`. The Qt layer uses ioLibrary; there's no dependency the other way.
>
> **(Slide 7 — ioLibrary pipeline.)** Inside ioLibrary the multithreaded pipeline is a strict producer-consumer chain: a `ThreadedReader` pulls from `FtdiDevice` into the raw ring buffer; the `ScaleShiftPipeline` transforms bytes into the processed ring buffer; the `ThreadedWriter` drains it into either the device or a file. Every `CircularBuffer` reference is **aggregation** — the buffers are owned higher up.
>
> **(Slide 8 — UI MVC.)** On top of that, the Qt layer is a clean MVC. `OscilloscopeModel` inherits `QObject` and owns the whole pipeline via `unique_ptr` and `new`/`delete`. Two concrete views — Compact and Workspace — implement our `AbstractOscilloscopeView` interface; `MainWindow` owns the model and both views, and switches between them in a `QStackedWidget`."

---

### 5 — 4+1 Views: Process view (5:00, Presenter B, 1.5 min)

**Action:** Slide 9 — sequence diagram (`sequence_start_acquisition.puml`). Slide 10 — activity diagram (`activity_pipeline.puml`).

**Script:**

> "The **process view** answers: what happens at runtime when the user clicks Start?
>
> **(Slide 9 — sequence.)** MainWindow forwards to the active view, which calls `model.startAcquisition`. The model opens the FTDI device, allocates two ring buffers, instantiates the reader, pipeline, and writer, starts all three threads, and from that point forward each processed sample is posted back through a Qt `QueuedConnection` to `deliverSample` — which emits `samplesUpdated`, which the waveform widget consumes.
>
> **(Slide 10 — activity.)** Three threads run concurrently. The reader blocks on `FT_Read`, produces into the raw buffer. The pipeline blocks on the raw buffer's `read`, computes scale and shift atomically, blinks DB0 if configured, and produces into the processed buffer. The writer blocks on the processed buffer, consumes, and either calls `FT_Write` or `fwrite`. `setDone()` on either buffer unblocks everyone for a clean shutdown — no polling, no sleeps."

---

### 6 — 4+1 Views: Development + Physical views (6:30, Presenter B, 1 min)

**Action:** Slide 11 — package diagram (`package_diagram.puml`). Slide 12 — deployment diagram (`deployment_diagram.puml`).

**Script:**

> "**(Slide 11 — packages.)** At build-artifact granularity: `ioLibrary/` produces `libioLibrary.a`, consumed by three executables — `pipeline` (CLI), `blink_test` (demo), and `oscilloscope_qt` (GUI). The legacy C controller is kept as `controller.c` but is outside the graded design.
>
> **(Slide 12 — deployment.)** Physically: the host PC runs the Qt binary linked against the FTDI D2XX library; the FT245R board sits on the USB bus; DB0 drives an LED through a 220-ohm resistor back to GND. An optional second board — same driver — receives processed bytes in dual-FTDI mode."

---

### 7 — 4+1 Views: Scenarios (7:30, Presenter B, 0.5 min)

**Action:** Slide 13 — use case diagram on the left, top-5 requirements on the right.

**Script:**

> "The **scenarios** view ties it all together. One primary actor — the operator — interacts with use cases like *Start acquisition*, *Adjust scale and shift*, *Switch UI view*, and *Capture to file*. Our full enumerated requirements list is in `docs/requirements.md` — here are the top five, including the non-functional requirement to support at least two UIs, which the rubric asks for explicitly."

---

### 8 — Code structure & conventions (8:00, Presenter C, 1 min)

**Action:** Slide 14 — responsibilities table (one row per class).

**Script:**

> "Every class lives in its own header+implementation pair. We use the `io` prefix as our namespace — for example `ioScaleShiftPipeline::ScaleShiftPipeline` — because every class in the library deals with byte streams. This follows the rubric's guidance to use a consistent namespace; we chose `io` over `scp`.
>
> Each class has a single documented responsibility. `FtdiDevice` wraps every FT_ call and serializes them with a mutex. `CircularBuffer` is the only thread-synchronization primitive in the project. `ScaleShiftPipeline` is the only place numeric transformation happens. `OscilloscopeModel` is the only place the pipeline is assembled and torn down."

---

### 9 — Code ↔ UML mapping (9:00, Presenter C, 1 min)

**Action:** Slide 15 — sequence diagram arrow on the left, `ioOscilloscopeModel.cpp:107-168` on the right, line numbers visible.

**Script:**

> "Here's the payoff: every arrow in the sequence diagram maps to a specific line in the code. On the left, the arrow from `OscilloscopeModel` to `ThreadedReader.start`. On the right, `ioOscilloscopeModel.cpp` line 167 — `reader_->start()` — preceded by the construction and configuration. The aggregation relationships in the class diagram correspond to the constructor parameters — `new ThreadedReader(readDevice_.get())` takes a non-owning pointer, exactly as the UML shows."

---

### 10 — Live debugging (10:00, Presenter C, 1 min)

**Action:** Switch to VS Code. Hit breakpoint 1 → breakpoint 2.

**Script:**

> "Let me step through a live run. **(Run + breakpoint at `startAcquisition`.)** We're about to open the FTDI device. Note `readDevice_` is still null. Step over — the `unique_ptr` now holds a handle, and the device prints `opened successfully`. A few more steps and we see the three threads being spawned.
>
> **(Continue to breakpoint inside `ScaleShiftPipeline::threadFunc`.)** The pipeline thread has just pulled a byte. `b` holds the raw sample. `scale.load()` and `shift.load()` are read atomically — that's why they're `std::atomic<double>`. After `clamp`, `o` is the processed byte that gets written to the output buffer and delivered to the UI."

---

### 11 — Live demo: Terminal pipeline (11:00, Presenter D, 1 min)

**Action:** Terminal tab 1. Run the command, wait for it to finish, then `xxd output.bin | head`.

**Script:**

> "Now the CLI demo. I'm running `./pipeline --freq 5 --duration 5 --output-file output.bin`. You can see the reader and writer cycles logged in real time — `[Reader] Cycle 0`, `[Writer] Cycle 0` — each producing and consuming one byte at 5 Hz. After five seconds both threads stop cleanly and the device closes.
>
> **(Run `xxd output.bin | head`.)** The output file contains 25 bytes — five seconds times five hertz — which matches our expectation. With no driven input, the FT245R returns 0x00, which is exactly what we see."

---

### 12 — Live demo: Qt UI (12:00, Presenter D, 2 min)

**Action:** Launch `./oscilloscope_qt`. Demonstrate in this order: Start → change Scale → change Shift → View menu switch → Stop.

**Script:**

> "Next, the GUI. I'm launching `oscilloscope_qt`.
>
> **(Click Start.)** DB0 blink is on, so the pipeline is toggling bit 0 every sample. You can see the square wave plotted in real time, and if you look at the breadboard the LED is flashing at the sample rate.
>
> **(Change Scale from 1.0 to 2.0.)** The amplitude doubles. The change is atomic — no restart needed — because `scale` is an atomic double the pipeline reloads on every sample.
>
> **(Change Shift from 0 to 64.)** The waveform jumps up; this is the DC offset.
>
> **(Open View menu → Workspace view.)** Same model, different layout. The MainWindow unbound the compact view from the model and rebound the workspace view — exactly one view is wired at a time, so no double-signal issues.
>
> **(Click Stop.)** All three threads join cleanly and the device closes."

---

### 13 — Wrap: what's next + submission (14:00, All, 0.5 min)

**Action:** Slide 16 — roadmap + submission summary.

**Script (Presenter A closes):**

> "To summarize: we have the complete 4+1 architecture, a working Qt GUI with two layouts, a working CLI, live scale/shift control, and a captured output file. For the final presentation we plan to add trigger logic, a simple signal generator mode, and CSV export.
>
> Our submission includes the slide deck, a zipped source tree, and a `README-GRADER.txt` with the exact compile and run commands for both macOS and Windows. Thank you — we're happy to take questions."

---

### 14 — Q&A (14:30, All, 1.5 min)

**Pre-prepared answers** for likely questions:

| Q | Lead answerer | Key point |
|---|---|---|
| "Why a custom ring buffer and not `std::queue`?" | C | Need blocking semantics and bounded memory — `std::queue` has neither; our `CircularBuffer` blocks on `notFull`/`notEmpty` and caps at configured capacity. |
| "How do you avoid data races on the shared `FtdiDevice`?" | C | Every public method (`read`/`write`/`close`) takes an internal `std::mutex` — verified in `ioFtdiDevice.cpp`. |
| "Why MVC and not MVVM / MVP?" | B | Qt's signal/slot is the natural fit for MVC; our Model exposes signals, the View subscribes, the Controller (`MainWindow`) wires them — no view-model layer needed for this scope. |
| "What happens if the grader has no FT245R?" | D | `FT_Open` fails → `errorMessage` signal → `QMessageBox` — app stays usable, no crash. We'll show a screen recording as backup. |
| "Can the writer target both a device and a file?" | C | No — two constructors, one sink per instance. Dual output was out of scope. |
| "How thread-safe is the pipeline on shutdown?" | B | `setDone()` on each buffer unblocks both sides; threads exit their loops and are `join`-ed in the destructor. No detach, no polling. |

---

## Contingency plans

| Failure | Fallback |
|---|---|
| FTDI device not detected live | Switch to pre-recorded screen video at Slide 16-bis. |
| Qt app crashes at Start | Use CLI pipeline demo + narrate the GUI from screenshots. |
| VS Code debugger misbehaves | Skip live breakpoints; walk through the same code in read-only view and narrate as if stepping. |
| Running long (> 14 min) | Cut Step 7 (scenarios, 0.5 min) and Step 6 second half (deployment, 0.5 min), jump straight to code + demo. |
| Running short (< 12 min) | Add the object diagram slide and a second debugging breakpoint. |

---

## Final day-of checklist

- [ ] Hardware: FT245R board, USB data cable, breadboard with LED + 220Ω resistor.
- [ ] Software: laptop fully charged + charger, backup laptop if possible.
- [ ] Files on desktop: slide deck (`.pptx` + `.pdf`), screen recording fallback, zipped source.
- [ ] VS Code: workspace open, bookmarks verified, debugger once tested this morning.
- [ ] Adapter for projector (USB-C → HDMI).
- [ ] Water bottle. Do not skip this.
