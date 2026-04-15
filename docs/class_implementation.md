# Class Implementation and Relationships

This document enumerates every implemented class in the project and describes how they relate to each other. Only **classes actually implemented in this repository** are listed — the vendor `FT_HANDLE` / `FT_*` API from `ftd2xx.h` is treated as an external dependency, not a project class.

Relationship legend:

| Notation | Meaning |
|---|---|
| **Inheritance** (`A ◁── B`) | `B` derives from `A` (is-a). |
| **Composition** (`A ◆── B`) | `A` owns `B`'s lifetime; `B` cannot outlive `A`. |
| **Aggregation** (`A ◇── B`) | `A` references `B` but does not own it; `B` is created elsewhere and outlives the reference. |
| **Usage / Dependency** (`A --> B`) | `A` uses `B` transiently (parameter, signal target, callback), no ownership. |

---

## 1. Inventory of Implemented Classes

### `ioLibrary/` — reusable static C++ library (`libioLibrary.a`)

| Class | File | Role |
|---|---|---|
| `ioFtdiDevice::FtdiDevice` | `ioFtdiDevice.h/.cpp` | Encapsulates a single FTDI handle; serializes all `FT_Read`/`FT_Write`/`FT_Close` through an internal `std::mutex`. |
| `ioBuffer::ioBuffer` | `ioBuffer.h/.cpp` | Heap byte buffer (`malloc`/`free`) of fixed capacity used by single-threaded readers/writers. |
| `ioRead::ioRead` | `ioRead.h/.cpp` | Single-threaded blocking read loop at a configured frequency. |
| `ioWrite::ioWrite` | `ioWrite.h/.cpp` | Single-threaded blocking write loop at a configured frequency. |
| `ioCircularBuffer::CircularBuffer` | `ioCircularBuffer.h/.cpp` | Thread-safe ring buffer (mutex + two condition variables + `finished` flag). |
| `ioThreadedReader::ThreadedReader` | `ioThreadedReader.h/.cpp` | Producer thread: reads from an `FtdiDevice` and pushes bytes into a `CircularBuffer`. |
| `ioThreadedWriter::ThreadedWriter` | `ioThreadedWriter.h/.cpp` | Consumer thread: pulls bytes from a `CircularBuffer` and writes them to either an `FtdiDevice` or a file. |
| `ioScaleShiftPipeline::ScaleShiftPipeline` | `ioScaleShiftPipeline.h/.cpp` | Intermediate stage thread: `CircularBuffer → round(scale·b + shift) [+ optional DB0 toggle] → CircularBuffer`. |

### `ui/` — Qt 6 application (`oscilloscope_qt`)

| Class | File | Role |
|---|---|---|
| `ioOscilloscopeModel::OscilloscopeModel` | `ioOscilloscopeModel.h/.cpp` | **Model.** Owns FTDI device(s), ring buffers, reader, pipeline, writer; exposes Qt signals `samplesUpdated` / `logLine` / `errorMessage`. |
| `ioAbstractOscilloscopeView::AbstractOscilloscopeView` | `ioAbstractOscilloscopeView.h` | **Pure-virtual view interface.** `asWidget`, `viewTitle`, `bindModel`, `unbindModel`. |
| `ioCompactOscilloscopeView::CompactOscilloscopeView` | `ioCompactOscilloscopeView.h/.cpp` | Concrete view — single-column compact layout. |
| `ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView` | `ioWorkspaceOscilloscopeView.h/.cpp` | Concrete view — `QSplitter` layout with large plot + side parameter panel. |
| `ioWaveformWidget::WaveformWidget` | `ioWaveformWidget.h/.cpp` | Custom `QWidget` that paints up to 512 normalized samples as a polyline. |
| `ioMainWindow::MainWindow` | `ioMainWindow.h/.cpp` | **Controller.** Owns the model, hosts both views in a `QStackedWidget`, wires the View menu, shows device-error dialogs. |

> The legacy C files (`controller.c`, `LED_Project.c`, `morse_Project.c`) expose **free functions only** — no classes — so they are outside this diagram.

---

## 2. UML Class Diagram (overall)

```
                                 ┌───────────────────────────────────────────┐
                                 │              ioLibrary (core)              │
                                 └───────────────────────────────────────────┘

                       ┌────────────────┐                ┌────────────┐
                       │  FtdiDevice    │                │  ioBuffer  │
                       │────────────────│                │────────────│
                       │ - handle       │                │ - storage  │
                       │ - accessMutex  │                │ - length   │
                       │ + open/close   │                │ + create() │
                       │ + read/write   │                │ + data()   │
                       └───────▲────────┘                └─────▲──────┘
                               │ composition (ptr, not owned)         │ aggregation (ptr, not owned)
                               │                                      │
               ┌───────────────┼───────────────┐       ┌──────────────┴──────────────┐
               │ ◆             │               │ ◆     │ ◇                           │ ◇
       ┌───────┴──────┐ ┌──────┴──────┐ ┌──────┴──────┐│    ┌────────────────┐       │
       │   ioRead     │ │  ioWrite    │ │ThreadedRead.││    │    ioRead      │       │
       │──────────────│ │─────────────│ │ThreadedWrit.││    │    ioWrite     │───────┘
       │ - device*    │ │ - device*   │ │ - device*   ││    └────────────────┘
       │ - buffer*  ◇ │ │ - buffer* ◇ │ │ - circBuf* ◇│└── see CircularBuffer below
       │ - N,freq     │ │ - M,freq    │ │ - thread    │
       │ + readLoop() │ │ + writeLoop()│ │ + start/stop│
       └──────────────┘ └─────────────┘ └──────┬──────┘
                                                │
                                                │ aggregation
                                                ▼
                                       ┌──────────────────┐
                                       │ CircularBuffer   │ ◁── shared producer/consumer medium
                                       │──────────────────│
                                       │ - storage        │
                                       │ - mtx, notEmpty, │
                                       │   notFull        │
                                       │ - finished       │
                                       │ + write/read     │
                                       │ + setDone()      │
                                       └────────▲─────────┘
                                                │ aggregation (two instances: raw + processed)
                                                │
                                       ┌────────┴───────────┐
                                       │ ScaleShiftPipeline │
                                       │────────────────────│
                                       │ - inBuf*, outBuf*  │
                                       │ - scale,shift      │
                                       │ - blinkDb0         │
                                       │ - worker (thread)  │
                                       │ + start/stopJoin() │
                                       │ + setSampleCallback│
                                       └────────────────────┘


                                 ┌───────────────────────────────────────────┐
                                 │              ui/ (Qt MVC)                  │
                                 └───────────────────────────────────────────┘

   ┌──────────────────┐               ┌────────────────────────────────────┐
   │   MainWindow     │ ◆──────────── │         OscilloscopeModel          │
   │  (QMainWindow)   │  owns model   │          (QObject)                 │
   │──────────────────│               │────────────────────────────────────│
   │ - model_         │               │  unique_ptr<FtdiDevice>  readDev_  │ ◆
   │ - stack_ (QSW)   │               │  unique_ptr<FtdiDevice>  writeDev_ │ ◆
   │ - compactView_ ◆ │               │  unique_ptr<CircularBuffer> raw_   │ ◆
   │ - workspaceView_ ◆               │  unique_ptr<CircularBuffer> proc_  │ ◆
   │ - currentView_ ◇ │               │  ThreadedReader*      reader_      │ ◆ (new/delete)
   └───────┬──────────┘               │  ScaleShiftPipeline*  pipeline_    │ ◆ (new/delete)
           │                          │  ThreadedWriter*      writer_      │ ◆ (new/delete)
           │ composition              │  signals: samplesUpdated, logLine, │
           │  (two concrete views)    │           errorMessage             │
           ▼                          └──────────────┬─────────────────────┘
   ┌────────────────────────┐                        │
   │ AbstractOscilloscopeView│ ◁── inheritance       │ Qt signal/slot (usage, QueuedConnection)
   │  (abstract interface)  │                        ▼
   └───────▲────────▲───────┘              ┌──────────────────────┐
           │        │                      │  concrete view slots │
   ┌───────┴──┐  ┌──┴───────────┐          │  onSamplesUpdated()  │
   │ Compact  │  │ Workspace    │          │  onLogLine()         │
   │ View     │  │ View         │          └──────────┬───────────┘
   │ (QWidget)│  │ (QWidget)    │                     │ usage
   │          │  │              │                     ▼
   │ waveform_│  │ waveform_    │            ┌──────────────────┐
   │   ◆──────┼──┼──────◆──────►│            │  WaveformWidget  │
   │          │  │              │            │   (QWidget)      │
   └──────────┘  └──────────────┘            └──────────────────┘
```

(Symbols: `◆` = composition, `◇` = aggregation, `▲` = inheritance arrowhead, `-->` / textual "usage" = plain dependency.)

---

## 3. Relationships — by class

### 3.1 `FtdiDevice`
- **Inheritance:** none.
- **Composition:** owns an `FT_HANDLE` (external, opaque) and an internal `std::mutex`.
- **Aggregation / Usage:** used (non-owning) by `ioRead`, `ioWrite`, `ThreadedReader`, `ThreadedWriter`, and stored in `std::unique_ptr` by `OscilloscopeModel` (Model owns → composition **from Model's perspective**).

### 3.2 `ioBuffer`
- **Inheritance:** none.
- **Composition:** owns a heap `BYTE*` (`malloc`/`free`).
- **Aggregation / Usage:** passed by pointer to `ioRead::configure` / `ioWrite::configure`; they do **not** own it.

### 3.3 `ioRead`
- **Inheritance:** none.
- **Composition:** — (holds only raw pointers to external objects).
- **Aggregation:** `FtdiDevice *device` (non-owning, must outlive `ioRead`) and `ioBuffer *buffer` (non-owning, set via `configure`).
- **Usage:** calls `device->read()`; uses `usleep`/`Sleep` for timing.

### 3.4 `ioWrite`
- Mirrors `ioRead`: aggregates `FtdiDevice*` and `ioBuffer*`, advances an internal offset and loops at configured frequency.

### 3.5 `CircularBuffer`
- **Inheritance:** none.
- **Composition:** owns its byte storage, `std::mutex`, and two `std::condition_variable`s.
- **Aggregation / Usage:** shared medium — producers (`ThreadedReader`, `ScaleShiftPipeline` outBuf side) and consumers (`ThreadedWriter`, `ScaleShiftPipeline` inBuf side) hold non-owning pointers; its lifetime is controlled by the enclosing context (`OscilloscopeModel` owns two via `unique_ptr`; `pipeline.cpp` owns one on the stack).

### 3.6 `ThreadedReader`
- **Inheritance:** none.
- **Composition:** owns a `std::thread` and a `std::atomic<bool> running`.
- **Aggregation:** `FtdiDevice *device` (non-owning) and `CircularBuffer *circBuffer` (non-owning).
- **Usage:** allocates a transient `BYTE[]` per run; calls `device->read()` and `circBuffer->write()`; calls `circBuffer->setDone()` on stop.

### 3.7 `ThreadedWriter`
- **Inheritance:** none.
- **Composition:** owns a `std::thread`, `std::atomic<bool> running`, and — when constructed with the file-path overload — a `FILE*` (closed in destructor).
- **Aggregation:** `FtdiDevice *device` (optional, non-owning, 0..1) and `CircularBuffer *circBuffer` (non-owning).
- **Usage:** has two constructors (FTDI-target vs. file-target); branches inside `threadFunc` accordingly.

### 3.8 `ScaleShiftPipeline`
- **Inheritance:** none.
- **Composition:** owns a worker `std::thread`, three `std::atomic`s (`scale`, `shift`, `blinkDb0`), a `running` flag, a `SampleCallback` (`std::function<void(unsigned char)>`), and `sampleCounter`.
- **Aggregation:** `CircularBuffer *inBuf` and `CircularBuffer *outBuf` (both non-owning).
- **Usage:** invokes the registered `SampleCallback` on each processed byte (the Model uses this to post samples to the Qt event loop); calls `outBuf->setDone()` when stopping.

### 3.9 `OscilloscopeModel` (inherits `QObject`)
- **Inheritance:** `QObject` ◁── `OscilloscopeModel` (enables signals/slots via `Q_OBJECT`).
- **Composition (ownership):**
  - `std::unique_ptr<FtdiDevice> readDevice_`, `writeDevice_`
  - `std::unique_ptr<CircularBuffer> rawBuffer_`, `processedBuffer_`
  - Raw-pointer members managed manually with `new`/`delete` inside `start/stopAcquisition`: `ThreadedReader* reader_`, `ScaleShiftPipeline* pipeline_`, `ThreadedWriter* writer_`.
  - `QMutex displayMutex_`, `QVector<double> displaySamples_`.
- **Aggregation / Usage:**
  - Hands non-owning pointers (device, buffers) to reader/pipeline/writer — these are **compositional from the Model's perspective** (lifetimes nested) but **aggregational from the helpers' perspective** (they just reference).
  - Installs a `SampleCallback` into `ScaleShiftPipeline` that does `QMetaObject::invokeMethod(... Qt::QueuedConnection ...)` onto its own `deliverSample` slot — this is a usage/dependency on Qt's meta-object system, safe across thread boundaries.
- **Signals (outbound usage):** `samplesUpdated(QVector<double>)`, `logLine(QString)`, `errorMessage(QString)`.

### 3.10 `AbstractOscilloscopeView`
- **Inheritance:** none (pure-virtual interface in its own namespace; **not** derived from `QObject` or `QWidget`).
- **Composition / Aggregation:** none — interface only.
- **Usage:** declared in `MainWindow` as `AbstractOscilloscopeView *compactView_`, `workspaceView_`, `currentView_`; forward-declares `OscilloscopeModel` for `bindModel/unbindModel`.

### 3.11 `CompactOscilloscopeView` and `WorkspaceOscilloscopeView`
- **Inheritance (multiple):**
  - `QWidget` ◁── (concrete view)
  - `AbstractOscilloscopeView` ◁── (concrete view)
  - Both also use `Q_OBJECT` for Qt slots.
- **Composition (Qt parent-owned children):** each view owns (via Qt parent/child, i.e., composition) a `WaveformWidget *waveform_`, a `QTextEdit *log_`, spin boxes (`QDoubleSpinBox`, `QSpinBox`), check boxes (`QCheckBox`), and buttons (`QPushButton`).
- **Aggregation:** `OscilloscopeModel *model_` (non-owning — set in `bindModel`, cleared in `unbindModel`); `QVector<QMetaObject::Connection> connections_` tracks temporary Qt connections.
- **Usage:** Qt signal/slot connections — spin/checkbox/button signals → the view's own `apply*` / `handle*` slots → `model_->setXxx()` / `model_->startAcquisition()`. Model signals `samplesUpdated` / `logLine` → view slots `onSamplesUpdated` / `onLogLine` → `waveform_->setSamples()` / `log_->append()`.

### 3.12 `WaveformWidget` (inherits `QWidget`)
- **Inheritance:** `QWidget` ◁── `WaveformWidget` (`Q_OBJECT`).
- **Composition:** owns a `QVector<double> samples_` (up to 512 values).
- **Usage:** overrides `paintEvent` (uses `QPainter`, `QPen`, `QPaintEvent`); `setSamples()` is called by the concrete views.

### 3.13 `MainWindow` (inherits `QMainWindow`)
- **Inheritance:** `QMainWindow` ◁── `MainWindow` (`Q_OBJECT`).
- **Composition (Qt parent-owned):**
  - `OscilloscopeModel *model_` (created with `this` as `QObject` parent → owned).
  - `QStackedWidget *stack_`, `CompactOscilloscopeView *compactView_`, `WorkspaceOscilloscopeView *workspaceView_`, menu actions, status bar.
- **Aggregation:** `AbstractOscilloscopeView *currentView_` — a **non-owning pointer** that aliases whichever concrete view is active (the views themselves are owned as above).
- **Usage:** connects `model_->errorMessage` → `MainWindow::onModelError` (shows `QMessageBox`); on view switch calls `currentView_->unbindModel()` then the new one's `bindModel(model_)`.

---

## 4. End-to-end Relationship Summary

### 4.1 Ownership chain inside the Qt app (top-down composition)

```
MainWindow
 ├─◆ OscilloscopeModel          (Qt parent)
 │   ├─◆ unique_ptr<FtdiDevice>        readDevice_
 │   ├─◆ unique_ptr<FtdiDevice>        writeDevice_  (optional, dual-FTDI mode)
 │   ├─◆ unique_ptr<CircularBuffer>    rawBuffer_
 │   ├─◆ unique_ptr<CircularBuffer>    processedBuffer_
 │   ├─◆ ThreadedReader*               reader_        (new/delete)
 │   ├─◆ ScaleShiftPipeline*           pipeline_      (new/delete)
 │   └─◆ ThreadedWriter*               writer_        (new/delete)
 ├─◆ QStackedWidget
 ├─◆ CompactOscilloscopeView
 │   ├─◆ WaveformWidget + QTextEdit + spin/check/button widgets
 │   └─◇ OscilloscopeModel*            (alias only)
 └─◆ WorkspaceOscilloscopeView
     ├─◆ WaveformWidget + QTextEdit + spin/check/button widgets
     └─◇ OscilloscopeModel*            (alias only)
```

### 4.2 Helper-to-resource aggregation inside the pipeline

```
ThreadedReader      ─◇─► FtdiDevice (read)       ─◇─► rawBuffer
ScaleShiftPipeline  ─◇─► rawBuffer (in)          ─◇─► processedBuffer (out)
ThreadedWriter      ─◇─► processedBuffer         ─◇─► FtdiDevice (write) OR FILE*
```

### 4.3 Inheritance hierarchies

```
QObject                                    (Qt)
 └─ OscilloscopeModel

QWidget                                    (Qt)
 ├─ WaveformWidget
 ├─ CompactOscilloscopeView     ──┐ also implements
 └─ WorkspaceOscilloscopeView   ──┤
                                  ▼
                      AbstractOscilloscopeView  (pure interface, no Qt base)

QMainWindow                                (Qt)
 └─ MainWindow
```

No other project class uses inheritance — all ioLibrary classes are standalone; cross-file coupling is purely **composition (resources they own)** plus **aggregation (pointers to external resources passed in via `configure`/constructor)**, with a few **callback / signal-slot usages** at the seams.
