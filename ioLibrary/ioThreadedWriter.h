#ifndef THREADED_WRITER_H
#define THREADED_WRITER_H

#include "ioFtdiDevice.h"
#include "ioCircularBuffer.h"
#include <cstddef>
#include <cstdio>
#include <thread>
#include <atomic>

namespace ioThreadedWriter {

class ThreadedWriter {
private:
    ioFtdiDevice::FtdiDevice     *device;
    ioCircularBuffer::CircularBuffer *circBuffer;
    FILE           *outputFile;
    std::size_t     M;
    double          frequencyHz;
    std::thread     writerThread;
    std::atomic<bool> running;

    void threadFunc();

public:
    ThreadedWriter(ioFtdiDevice::FtdiDevice *device);
    ThreadedWriter(const char *filePath);
    ~ThreadedWriter();

    void configure(ioCircularBuffer::CircularBuffer *buf, std::size_t M, double frequencyHz);
    void start();
    void stop();
};

} // namespace ioThreadedWriter

#endif
