#include "ThreadedReader.h"
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#define io_sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define io_sleep_us(us) usleep(us)
#endif

ThreadedReader::ThreadedReader(FtdiDevice *device)
    : device(device), circBuffer(nullptr), N(0), frequencyHz(0.0), running(false) {}

ThreadedReader::~ThreadedReader() {
    stop();
}

void ThreadedReader::configure(CircularBuffer *buf, std::size_t N, double frequencyHz) {
    this->circBuffer = buf;
    this->N = N;
    this->frequencyHz = frequencyHz;
}

void ThreadedReader::start() {
    if (running.load()) return;
    running.store(true);
    readerThread = std::thread(&ThreadedReader::threadFunc, this);
    printf("[Reader] Thread started (N=%zu, freq=%.1f Hz)\n", N, frequencyHz);
}

void ThreadedReader::stop() {
    if (!running.load()) return;
    running.store(false);
    circBuffer->setDone();
    if (readerThread.joinable()) {
        readerThread.join();
    }
    printf("[Reader] Thread stopped.\n");
}

void ThreadedReader::threadFunc() {
    useconds_t delay_us = static_cast<useconds_t>(1000000.0 / frequencyHz);
    BYTE *tempBuf = new BYTE[N];
    int cycle = 0;

    while (running.load()) {
        FT_STATUS status = device->read(tempBuf, N);
        if (status != FT_OK) {
            fprintf(stderr, "[Reader] Error: read failed at cycle %d (status %lu)\n",
                    cycle, (unsigned long)status);
            break;
        }

        if (!circBuffer->write(tempBuf, N)) {
            break;
        }

        printf("[Reader] Cycle %d: read %zu byte(s), first=0x%02X\n", cycle, N, tempBuf[0]);
        cycle++;
        io_sleep_us(delay_us);
    }

    delete[] tempBuf;
}
