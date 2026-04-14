#include "ioThreadedWriter.h"
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#define io_sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define io_sleep_us(us) usleep(us)
#endif

namespace ioThreadedWriter {

ThreadedWriter::ThreadedWriter(ioFtdiDevice::FtdiDevice *device)
    : device(device), circBuffer(nullptr), outputFile(nullptr),
      M(0), frequencyHz(0.0), running(false) {}

ThreadedWriter::ThreadedWriter(const char *filePath)
    : device(nullptr), circBuffer(nullptr), outputFile(nullptr),
      M(0), frequencyHz(0.0), running(false) {
    outputFile = fopen(filePath, "wb");
    if (outputFile == nullptr) {
        fprintf(stderr, "[Writer] Error: Failed to open file '%s' for writing\n", filePath);
    }
}

ThreadedWriter::~ThreadedWriter() {
    stop();
    if (outputFile != nullptr) {
        fclose(outputFile);
        outputFile = nullptr;
    }
}

void ThreadedWriter::configure(ioCircularBuffer::CircularBuffer *buf, std::size_t M, double frequencyHz) {
    this->circBuffer = buf;
    this->M = M;
    this->frequencyHz = frequencyHz;
}

void ThreadedWriter::start() {
    if (running.load()) return;
    running.store(true);
    writerThread = std::thread(&ThreadedWriter::threadFunc, this);
    if (device != nullptr) {
        printf("[Writer] Thread started — output: FTDI device (M=%zu, freq=%.1f Hz)\n", M, frequencyHz);
    } else {
        printf("[Writer] Thread started — output: file (M=%zu, freq=%.1f Hz)\n", M, frequencyHz);
    }
}

void ThreadedWriter::stop() {
    if (!running.load()) return;
    running.store(false);
    if (writerThread.joinable()) {
        writerThread.join();
    }
    printf("[Writer] Thread stopped.\n");
}

void ThreadedWriter::threadFunc() {
    useconds_t delay_us = static_cast<useconds_t>(1000000.0 / frequencyHz);
    BYTE *tempBuf = new BYTE[M];
    int cycle = 0;

    while (running.load()) {
        if (circBuffer == nullptr || !circBuffer->read(tempBuf, M)) {
            break;
        }

        if (device != nullptr) {
            FT_STATUS status = device->write(tempBuf, M);
            if (status != FT_OK) {
                fprintf(stderr, "[Writer] Error: FTDI write failed at cycle %d (status %lu)\n",
                        cycle, (unsigned long)status);
                break;
            }
        } else if (outputFile != nullptr) {
            std::size_t written = fwrite(tempBuf, 1, M, outputFile);
            if (written != M) {
                fprintf(stderr, "[Writer] Error: file write failed at cycle %d\n", cycle);
                break;
            }
            fflush(outputFile);
        }

        printf("[Writer] Cycle %d: wrote %zu byte(s), first=0x%02X\n", cycle, M, tempBuf[0]);
        cycle++;
        io_sleep_us(delay_us);
    }

    delete[] tempBuf;
}

} // namespace ioThreadedWriter
