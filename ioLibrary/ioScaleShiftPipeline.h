#ifndef IO_SCALE_SHIFT_PIPELINE_H
#define IO_SCALE_SHIFT_PIPELINE_H

#include "ioCircularBuffer.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>

namespace ioScaleShiftPipeline {

class ScaleShiftPipeline {
public:
    using SampleCallback = std::function<void(unsigned char)>;

    ScaleShiftPipeline(ioCircularBuffer::CircularBuffer *inBuf,
                       ioCircularBuffer::CircularBuffer *outBuf);
    ~ScaleShiftPipeline();

    void setSampleCallback(SampleCallback cb);

    void start();
    void stopJoin();

    std::atomic<double> scale;
    std::atomic<double> shift;

    /** When true, DB0 toggles every processed sample (same cadence as acquisition / sampleHz). */
    std::atomic<bool> blinkDb0;

private:
    void threadFunc();

    ioCircularBuffer::CircularBuffer *inBuf;
    ioCircularBuffer::CircularBuffer *outBuf;
    SampleCallback callback;
    std::thread worker;
    std::atomic<bool> running;
    std::uint64_t sampleCounter;
};

} // namespace ioScaleShiftPipeline

#endif
