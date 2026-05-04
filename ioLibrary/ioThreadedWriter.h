#ifndef THREADED_WRITER_H
#define THREADED_WRITER_H

#include "ioByteSink.h"
#include "ioCircularBuffer.h"
#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>

namespace ioThreadedWriter {

/**
 * Blocking loop: drain `M` bytes per period from a circular buffer into a `ByteSink`.
 * Timing / threading only — transport policy lives in `ByteSink` implementations.
 */
class ThreadedWriter {
private:
    std::unique_ptr<ioByteSink::ByteSink> byteSink_;
    ioCircularBuffer::CircularBuffer *circBuffer;
    std::size_t M;
    double frequencyHz;
    std::thread writerThread;
    std::atomic<bool> running;

    void threadFunc();

public:
    explicit ThreadedWriter(std::unique_ptr<ioByteSink::ByteSink> byteSink);
    ~ThreadedWriter();

    void configure(ioCircularBuffer::CircularBuffer *buf, std::size_t M, double frequencyHz);
    void start();
    void stop();
};

} // namespace ioThreadedWriter

#endif
