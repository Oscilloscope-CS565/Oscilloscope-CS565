# Presentation Guidance — Final Project Progress (Group)

Breakdown of what we have to produce for the in-class presentation, mapped one-to-one against the 100-point rubric (`docs/Rubric_for_first_presentation.md`) and the assignment spec (`docs/final_project_progress_specification.md`).

Use this as the master checklist. Each section lists: **what the rubric asks → what already exists in the repo → the gap → concrete action items → who / where to put it.**

---

## 0. Deliverables at submission time

Per rubric §10 (10 pts):

- [ ] **Slide deck** (PowerPoint `.pptx` or `.pdf`).
- [ ] **Zipped source tree** (clean copy — do not include `build/`, `*.o`, `*.dSYM`, the compiled `controller` / `pipeline` / `blink_test` binaries, or `output.bin`).
- [ ] **Exact compile + run command** for the instructor's machine. Paste these verbatim into a `README-GRADER.txt` at the root of the zip:
    - macOS (Qt GUI): `brew install qt cmake && cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)" && cmake --build build && ./build/oscilloscope_qt`
    - macOS (CLI pipeline): `make clean && make pipeline blink_test && ./pipeline --output-file output.bin --freq 10 --duration 5`
    - Windows (Qt GUI): `cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.8.2\msvc2019_64 && cmake --build build --config Release && build\Release\oscilloscope_qt.exe`

---

## 1. Project Overview & Purpose — 5 pts (Rubric §1)

**Ask:** project name, topic, problem, high-level description, domain context.

**Already have:** `README.md`, `CLAUDE.md`, `docs/PLAN.md`, `docs/项目说明与演示指南.md`.

**Slides to produce (2–3 slides):**

1. **Title slide** — project name (e.g. "FT245R Oscilloscope / Function Generator — CS565"), group members, date.
2. **Problem & scope** — hardware: FT245R 8-bit USB-parallel FIFO chip in sync bit-bang mode. Goal: acquire samples from DB0–DB7, stream through a scale/shift pipeline, visualize in real time and optionally write back to the device or to a second device/file.
3. **Domain context** — where FT245R fits (USB-to-GPIO for low-speed instrumentation), why a software-defined oscilloscope/function-generator on top of it is interesting.

**Action items:**

- [ ] Decide the final one-sentence "elevator pitch" and put it on the title slide.
- [ ] Pick one screenshot of the Qt UI (compact + workspace) for the overview slide.

---

## 2. Literature Review / Topic Understanding — 5 pts (Rubric §2)

**Ask:** research, conceptual clarity, technical background.

**Already have (cite directly):**

- `FT245R_BitBangMode.pdf` — official FTDI app note on sync/async bit-bang modes.
- `FT245R_D2XX_Programmer's_Guide.pdf` — D2XX API reference.
- `FT245R_FIFO_Basics.pdf` — FIFO operating principles.

**Slides (1–2):**

1. **FT245R technical background** — summarize: synchronous bit-bang vs async, DB0–DB7 are bidirectional GPIOs, `FT_SetBitMode(0xFF, 0x01)` = all outputs, sync bit-bang. USB packet size 64 B, baud divisor sets sample rate.
2. **Software architecture references** (optional but scores well):
    - Producer/consumer + bounded ring buffer (Hoare / classic concurrency).
    - MVC (Model–View–Controller) as realized in Qt's signal/slot system.
    - Pipe-and-filter / pipeline architecture pattern (Reader → Pipeline → Writer).

**Action items:**

- [ ] Pull 3–4 key diagrams / excerpts from the FTDI PDFs (timing diagram, baud divisor formula, BitMode table) for the slide.
- [ ] Cite 1–2 textbook/online references for MVC and pipeline patterns.

---

## 3. Architecture — 4+1 Views — 30 pts (Rubric §3)

**This is the highest-weighted single section.** The rubric lists required + optional diagrams; below is how the 4+1 Views model (Kruchten) maps to our artifacts.

| 4+1 View | Diagram type | Status | Where |
|---|---|---|---|
| **Logical view** | Class diagrams | ✅ Have 6 PlantUML | `docs/class_diagram_01_overview.puml`, `…01_iolib_singlethread.puml`, `…02_iolib_pipeline.puml`, `…03_model.puml`, `…04_views.puml`, `…05_controller.puml`, `…ui_only.puml` |
| **Logical view** | Object diagram (optional) | ❌ Missing | To add: `docs/object_diagram_runtime.puml` — a runtime snapshot with 1× `MainWindow`, 1× `OscilloscopeModel`, 2× `FtdiDevice`, 2× `CircularBuffer`, 1× each of reader / pipeline / writer. |
| **Process view** | Sequence diagram | ❌ Missing | To add: `docs/sequence_start_acquisition.puml` — user clicks Start → MainWindow → view → model.startAcquisition → devices open → threads start → reader produces → pipeline transforms → writer consumes → sample callback → `samplesUpdated` signal → waveform widget redraws. |
| **Process view** | Activity diagram | ❌ Missing | To add: `docs/activity_pipeline.puml` — reader loop, pipeline loop, writer loop, with join/fork bars showing the three threads and the two ring buffers as synchronization points. |
| **Process view** | State diagram (optional) | ❌ Missing | Optional: `docs/state_acquisition.puml` — states `Idle → Opening → Running → Stopping → Idle`, plus `Error` from any state. Worth 0 pts baseline but adds polish. |
| **Development view** | Package diagram | ⚠ Partial | `class_diagram_01_overview.puml` already shows the two packages at class granularity. Add a dedicated `docs/package_diagram.puml` that shows module-level directories: `ioLibrary/`, `ui/`, and the build artefacts (`libioLibrary.a`, `oscilloscope_qt`, `pipeline`, `blink_test`, `controller`). |
| **Physical view** | Deployment diagram | ❌ Missing | To add: `docs/deployment_diagram.puml` — nodes: `Host PC` (Qt app + D2XX driver), `USB bus`, `FT245R board #1 (read)`, optional `FT245R board #2 (write)`, breadboard + LED + resistor. Artefacts deployed: `oscilloscope_qt`, `libftd2xx`, `pipeline`. |
| **Scenarios (+1)** | Use case diagram | ❌ Missing | To add: `docs/use_case_diagram.puml` — primary actor `User (student/operator)`; system boundary `FT245R Controller`; use cases: *Start acquisition*, *Stop acquisition*, *Change scale/shift*, *Switch view*, *Toggle DB0 blink*, *Dual-FTDI capture*, *Write processed data to file*; secondary actor `FTDI hardware`. |
| **Scenarios (+1)** | Enumerated requirements | ❌ Missing | To add: `docs/requirements.md` — numbered functional + non-functional requirements (e.g. `FR-01` Acquire bytes from DB0–DB7, `FR-02` Apply scale/shift, `NFR-01` Sample rate up to 500 Hz, `NFR-02` Two selectable UIs). |

**Action items (priority order):**

1. [ ] **Requirements list** (`docs/requirements.md`) — ~15 numbered entries, functional + non-functional.
2. [ ] **Use case diagram** — drives slides §1 and §3; 6–7 use cases should be enough.
3. [ ] **Sequence diagram** of start-to-first-sample flow — most commonly asked for in code-review.
4. [ ] **Activity diagram** of the three-thread pipeline.
5. [ ] **Deployment diagram** — small (3–4 nodes) but required by rubric.
6. [ ] **Package diagram** dedicated file, at directory/artifact level (not class level).
7. [ ] (Optional) Object diagram + state diagram for extra polish.

**Slide structure for §3 (6–8 slides):**

- Slide: *"Our 4+1 Views"* — the Kruchten table, mark which view each upcoming slide implements.
- Slide: *Logical view* — class diagram overview (insert `class_diagram_01_overview.puml` rendered as PNG).
- Slide: *Logical view — ioLibrary pipeline* — `class_diagram_02_iolib_pipeline.puml`.
- Slide: *Logical view — UI MVC* — `class_diagram_ui_only.puml`.
- Slide: *Process view — sequence* — insert the new sequence diagram.
- Slide: *Process view — activity* — insert the new activity diagram.
- Slide: *Development view — package* — insert `package_diagram.puml`.
- Slide: *Physical view — deployment* — insert `deployment_diagram.puml`.
- Slide: *Scenarios — use cases + requirements* — compact: use case diagram on the left, top-5 requirements listed on the right.

---

## 4. Code Structure & Quality — 10 pts (Rubric §4)

**Ask:** one class per file, meaningful namespaced names, clearly stated responsibility per class.

**Already have:**

- One class per file across `ioLibrary/` and `ui/`. ✅
- Namespaces already used (`ioFtdiDevice::`, `ioCircularBuffer::`, `ioScaleShiftPipeline::`, `ioOscilloscopeModel::`, `ioMainWindow::`, etc.). ✅
- `docs/class_implementation.md` already enumerates every class's responsibility. ✅

**Gaps:**

- [ ] The rubric explicitly cites `scpMyClass` style. Our prefix is `io*` — this is **acceptable** (the spec says "use namespace in the name… feel free to use another namespace that is not scp"). Be ready to defend the `io` prefix during Q&A: `io` = input/output, since every class deals with byte streams.
- [ ] Make sure legacy C files (`controller.c`, `LED_Project.c`, `morse_Project.c`) are **clearly labeled as legacy** in slides, not mixed in with the graded C++ design.

**Slides (1–2):**

1. **Responsibilities table** — one row per class, 3 columns: *class*, *namespace*, *responsibility (1 line)*. Copy condensed from `class_implementation.md §1`.
2. **Conventions** — one class per file, `io*` namespace rationale, header/impl split, Qt `Q_OBJECT` on `QObject`-derived classes.

---

## 5. Demo — Terminal Control — 10 pts (Rubric §5)

**Ask:** run scope / function generator from terminal. Example commands: `scope start`, `sampleTime=1ms`, `sampleFor=10s`, `stop`.

**Already have:** `pipeline` CLI with `--freq`, `--duration`, `--bufsize`, `--output-file`, `--output-ftdi`, `--help`. `blink_test` for a fixed demo. `controller` for legacy menu. ✅

**Gap vs. rubric wording:** rubric uses imperative subcommands (`scope start`), ours uses flags. Acceptable, but prepare the translation table:

| Rubric command | Our equivalent |
|---|---|
| `scope start` | `./pipeline --freq 10 --duration 10 --output-file output.bin` |
| `sampleTime=1ms` | `--freq 1000` (1 kHz ≈ 1 ms period) |
| `sampleFor=10s` | `--duration 10` |
| `stop` | Ctrl-C or wait for duration to elapse |

**Action items:**

- [ ] **Rehearse the terminal demo**: one clean run of `./pipeline --freq 5 --duration 5 --output-file output.bin`, then `xxd output.bin | head` to prove bytes were captured.
- [ ] Prepare a fallback screen-recording (`.mov`) in case hardware misbehaves during live demo.
- [ ] Optional (nice-to-have): add a tiny wrapper script `scope` that translates the rubric-style verbs to our flags. Not required but impressive.

---

## 6. Demo — Data Interaction — 10 pts (Rubric §6)

**Ask:** real-time scrolling display; input changes visibly affect output; scaling & offsetting work.

**Already have:** Qt app `oscilloscope_qt` with scrolling `WaveformWidget`, Scale / Shift spin boxes, "Toggle DB0 every sample" for a square wave, dual-FTDI mode. ✅

**Action items:**

- [ ] **Rehearse the Qt demo script**:
    1. Launch `./build/oscilloscope_qt`.
    2. Click **Start** with DB0-toggle on → square wave appears.
    3. Change **Scale** from 1.0 → 2.0 → 0.5 → show plot amplitude changing live.
    4. Change **Shift** from 0 → 64 → -32 → show vertical offset.
    5. Use **View** menu to switch between Compact and Workspace layouts (covers rubric's "show two different UIs" requirement from §5b of the spec).
    6. Click **Stop** cleanly.
- [ ] Have a clip of the LED actually blinking, in case grader watches remotely.

---

## 7. Code-to-UML Mapping — 10 pts (Rubric §7)

**Ask:** explicit linkage between diagrams and code.

**Already have:** `docs/class_implementation.md` already walks through every class file-by-file. ✅

**Slides (2):**

1. **Left side = UML (class diagram snippet), right side = code excerpt.** Pick 2 representative examples:
    - `ThreadedReader` aggregation to `CircularBuffer` ↔ `ioThreadedReader.h` field declaration + `threadFunc()` call site.
    - `OscilloscopeModel` composition of the whole pipeline ↔ `ioOscilloscopeModel.cpp::startAcquisition()`.
2. **Sequence diagram mapping** — the "Start acquisition" sequence diagram, with line numbers annotated for each arrow pointing back to `ioOscilloscopeModel.cpp` / `ioThreadedReader.cpp` / `ioScaleShiftPipeline.cpp`.

**Action items:**

- [ ] Pre-mark 3 code locations with VS Code bookmarks or comments for quick jump during presentation.
- [ ] Make sure each UML→code slide has line numbers visible.

---

## 8. Debugging Demonstration — 5 pts (Rubric §8)

**Ask:** presenter sets breakpoints and steps through live.

**Suggested breakpoints (pick 3):**

1. `ioOscilloscopeModel.cpp` → `startAcquisition()` entry → step over device `open()`, buffer construction, thread starts. Shows pipeline assembly.
2. `ioScaleShiftPipeline.cpp` → inside `threadFunc()` loop, right after `inBuf->read(&b, 1)`. Inspect `b`, `scale.load()`, `shift.load()`, `o` after clamp. Shows per-sample transform.
3. `ioThreadedWriter.cpp` → `threadFunc()` branch on `device != nullptr`. Shows the file-vs-device fork.

**Action items:**

- [ ] Build in debug mode (`cmake --build build --config Debug` or `make` with `-g` already on).
- [ ] On macOS Qt, confirm lldb works through VS Code CodeLLDB extension — test before the demo.
- [ ] Prepare one-line narration per breakpoint (*"Here, the pipeline pulls a byte off the raw buffer. Watch `scale` and `shift` being read atomically."*).

---

## 9. Presentation Quality — 5 pts (Rubric §9)

**Ask:** clear, organized, professional; every member speaks; time-boxed.

**Action items:**

- [ ] **Agree a running order** — who presents which section. Suggested split (4 members):
    - Member A: §1, §2 (2 min)
    - Member B: §3 (Architecture, 4+1 Views) (4 min)
    - Member C: §4, §7, §8 (Code + Mapping + Debugging) (3 min)
    - Member D: §5, §6 (Live demo) (3 min)
    - Group: Q&A (2 min)
- [ ] **Rehearse end-to-end at least once** with a timer.
- [ ] **Slide master** — consistent theme, dark code blocks, readable font size on projector (≥18 pt body).

---

## 10. Submission Completeness — 10 pts (Rubric §10)

Already covered at the top (§0). Double-check before the deadline:

- [ ] `README-GRADER.txt` at zip root with exact compile + run commands.
- [ ] `git clean -fdx` (dry-run first!) or manual sweep to drop build artefacts before zipping.
- [ ] Verify the zip unpacks and builds on a **second** machine that hasn't been used for development.
- [ ] File name convention e.g. `CS565_Group<N>_FinalProgress.zip` + `.pdf` version of slides alongside the `.pptx`.

---

## Consolidated "things to produce this week"

| # | Artifact | Where | Rubric section | Priority |
|---|---|---|---|---|
| 1 | Slide deck (final) | `presentation/` (new folder) | §1–§10 | **P0** |
| 2 | `requirements.md` | `docs/` | §3 (scenarios) | **P0** |
| 3 | Use case diagram | `docs/use_case_diagram.puml` | §3 | **P0** |
| 4 | Sequence diagram (start acquisition) | `docs/sequence_start_acquisition.puml` | §3, §7 | **P0** |
| 5 | Activity diagram (pipeline threads) | `docs/activity_pipeline.puml` | §3 | **P0** |
| 6 | Deployment diagram | `docs/deployment_diagram.puml` | §3 | **P0** |
| 7 | Package diagram (dedicated) | `docs/package_diagram.puml` | §3 | **P1** |
| 8 | Object diagram (optional) | `docs/object_diagram_runtime.puml` | §3 | **P2** |
| 9 | State diagram (optional) | `docs/state_acquisition.puml` | §3 | **P2** |
| 10 | `README-GRADER.txt` (compile/run) | repo root | §10 | **P0** |
| 11 | Demo rehearsal checklist | verbally agreed | §5, §6, §8, §9 | **P0** |
| 12 | Screen recording fallback | `presentation/demo.mov` | §5, §6 | **P1** |

### Suggested schedule (if presentation is next week)

- **Day 1–2**: produce all P0 diagrams + requirements doc. Run `plantuml docs/*.puml` to render PNGs.
- **Day 3**: assemble slide deck; place rendered PNGs; write talking points.
- **Day 4**: full-team rehearsal #1 with timer, live demo dry-run on presenter's laptop.
- **Day 5**: fix rough edges, rehearsal #2, freeze zip + slides.
- **Day of**: bring USB-A cable + FT245R board + backup laptop + backup screen recording.

---

## Expected rubric score if every P0 item lands

| Rubric § | Points | Confidence once P0 done |
|---|---|---|
| §1 Project overview | 5 | 5 |
| §2 Literature review | 5 | 5 |
| §3 4+1 Views | 30 | 28–30 (full if object + state diagrams included) |
| §4 Code structure | 10 | 10 (already strong) |
| §5 Terminal demo | 10 | 10 |
| §6 Data interaction | 10 | 10 |
| §7 UML ↔ code mapping | 10 | 10 |
| §8 Debugging | 5 | 5 |
| §9 Presentation quality | 5 | 4–5 (depends on rehearsal) |
| §10 Submission completeness | 10 | 10 |
| **Total** | **100** | **~97–100** |
