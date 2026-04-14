#ifndef THREADED_READER_H
#define THREADED_READER_H

#include "ioFtdiDevice.h"
#include "ioCircularBuffer.h"
#include <cstddef>
#include <thread>
#include <atomic>

namespace ioThreadedReader {

class ThreadedReader {
private:
    ioFtdiDevice::FtdiDevice     *device;
    ioCircularBuffer::CircularBuffer *circBuffer;
    std::size_t     N;
    double          frequencyHz;
    std::thread     readerThread;
    std::atomic<bool> running;

    void threadFunc();

public:
    ThreadedReader(ioFtdiDevice::FtdiDevice *device);
    ~ThreadedReader();

    void configure(ioCircularBuffer::CircularBuffer *buf, std::size_t N, double frequencyHz);
    void start();
    void stop();
};

} // namespace ioThreadedReader

#endif
