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

ThreadedWriter::ThreadedWriter(std::unique_ptr<ioByteSink::ByteSink> byteSink)
    : byteSink_(std::move(byteSink)), circBuffer(nullptr), M(0), frequencyHz(0.0), running(false) {}

ThreadedWriter::~ThreadedWriter() {
    stop();
}

void ThreadedWriter::configure(ioCircularBuffer::CircularBuffer *buf, std::size_t mBytes, double hz) {
    circBuffer = buf;
    M = mBytes;
    frequencyHz = hz;
}

void ThreadedWriter::start() {
    if (running.load()) {
        return;
    }
    running.store(true);
    writerThread = std::thread(&ThreadedWriter::threadFunc, this);
    printf("[Writer] Thread started (M=%zu, freq=%.1f Hz)\n", M, frequencyHz);
}

void ThreadedWriter::stop() {
    if (!running.load()) {
        return;
    }
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

        FT_STATUS status = byteSink_->writeBytes(tempBuf, M);
        if (status != FT_OK) {
            fprintf(stderr, "[Writer] Error: sink write failed at cycle %d (status %lu)\n",
                    cycle, (unsigned long)status);
            break;
        }

        printf("[Writer] Cycle %d: wrote %zu byte(s), first=0x%02X\n", cycle, M, tempBuf[0]);
        cycle++;
        io_sleep_us(delay_us);
    }

    delete[] tempBuf;
}

} // namespace ioThreadedWriter
