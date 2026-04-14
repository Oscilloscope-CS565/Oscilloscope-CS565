#include "ioScaleShiftPipeline.h"
#include <cmath>

namespace ioScaleShiftPipeline {

ScaleShiftPipeline::ScaleShiftPipeline(ioCircularBuffer::CircularBuffer *inBuf,
                                       ioCircularBuffer::CircularBuffer *outBuf)
    : inBuf(inBuf),
      outBuf(outBuf),
      running(false),
      sampleCounter(0) {
    scale.store(1.0);
    shift.store(0.0);
    blinkDb0.store(false);
}

ScaleShiftPipeline::~ScaleShiftPipeline() {
    stopJoin();
}

void ScaleShiftPipeline::setSampleCallback(SampleCallback cb) {
    callback = std::move(cb);
}

void ScaleShiftPipeline::start() {
    if (running.load()) {
        return;
    }
    running.store(true);
    worker = std::thread(&ScaleShiftPipeline::threadFunc, this);
}

void ScaleShiftPipeline::stopJoin() {
    running.store(false);
    if (worker.joinable()) {
        worker.join();
    }
}

void ScaleShiftPipeline::threadFunc() {
    BYTE b;
    sampleCounter = 0;
    while (running.load()) {
        if (!inBuf->read(&b, 1)) {
            break;
        }
        double s = scale.load();
        double sh = shift.load();
        int v = static_cast<int>(std::lround(s * static_cast<double>(b) + sh));
        if (v < 0) {
            v = 0;
        }
        if (v > 255) {
            v = 255;
        }
        BYTE o = static_cast<BYTE>(v);
        if (blinkDb0.load()) {
            // Toggle DB0 each processed sample (cadence matches UI sample rate; square period ~ 2/sampleHz)
            BYTE blinkBit = static_cast<BYTE>((sampleCounter++) % 2ULL);
            o = static_cast<BYTE>((o & static_cast<BYTE>(0xFE)) | blinkBit);
        }
        if (!outBuf->write(&o, 1)) {
            break;
        }
        if (callback) {
            callback(o);
        }
    }
    outBuf->setDone();
}

} // namespace ioScaleShiftPipeline
