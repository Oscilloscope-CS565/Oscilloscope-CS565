#ifndef THREADED_READER_H
#define THREADED_READER_H

#include "ioByteSource.h"
#include "ioCircularBuffer.h"
#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>

namespace ioThreadedReader {

/**
 * Blocking loop: pull `N` bytes per period from a `ByteSource` into a circular buffer.
 * Timing / threading only — transport policy lives in `ByteSource` implementations.
 */
class ThreadedReader {
private:
    std::unique_ptr<ioByteSource::ByteSource> byteSource_;
    ioCircularBuffer::CircularBuffer *circBuffer;
    std::size_t N;
    double frequencyHz;
    std::thread readerThread;
    std::atomic<bool> running;

    void threadFunc();

public:
    explicit ThreadedReader(std::unique_ptr<ioByteSource::ByteSource> byteSource);
    ~ThreadedReader();

    void configure(ioCircularBuffer::CircularBuffer *buf, std::size_t N, double frequencyHz);
    void start();
    void stop();
};

} // namespace ioThreadedReader

#endif
