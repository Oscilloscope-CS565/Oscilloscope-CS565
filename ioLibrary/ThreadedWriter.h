#ifndef THREADED_WRITER_H
#define THREADED_WRITER_H

#include "FtdiDevice.h"
#include "CircularBuffer.h"
#include <cstddef>
#include <cstdio>
#include <thread>
#include <atomic>

class ThreadedWriter {
private:
    FtdiDevice     *device;       // composition — nullptr if writing to file
    CircularBuffer *circBuffer;   // aggregation — uses but does not own the buffer
    FILE           *outputFile;   // nullptr if writing to FTDI device
    std::size_t     M;            // bytes per write
    double          frequencyHz;
    std::thread     writerThread;
    std::atomic<bool> running;

    void threadFunc();

public:
    ThreadedWriter(FtdiDevice *device);       // write to FTDI device
    ThreadedWriter(const char *filePath);      // write to file
    ~ThreadedWriter();

    void configure(CircularBuffer *buf, std::size_t M, double frequencyHz);
    void start();
    void stop();
};

#endif
