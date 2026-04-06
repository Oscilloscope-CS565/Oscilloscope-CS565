#ifndef THREADED_READER_H
#define THREADED_READER_H

#include "ioFtdiDevice.h"
#include "ioCircularBuffer.h"
#include <cstddef>
#include <thread>
#include <atomic>

class ThreadedReader {
private:
    FtdiDevice     *device;       // composition — uses but does not own the device
    CircularBuffer *circBuffer;   // aggregation — uses but does not own the buffer
    std::size_t     N;            // bytes per read
    double          frequencyHz;
    std::thread     readerThread;
    std::atomic<bool> running;

    void threadFunc();

public:
    ThreadedReader(FtdiDevice *device);
    ~ThreadedReader();

    void configure(CircularBuffer *buf, std::size_t N, double frequencyHz);
    void start();
    void stop();
};

#endif
