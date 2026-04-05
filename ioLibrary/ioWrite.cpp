#include "ioWrite.h"
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#define io_sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define io_sleep_us(us) usleep(us)
#endif

ioWrite::ioWrite(FtdiDevice *device)
    : device(device), buffer(nullptr), M(0), frequencyHz(0.0) {}

void ioWrite::configure(ioBuffer *buffer, std::size_t M, double frequencyHz) {
    this->buffer = buffer;
    this->M = M;
    this->frequencyHz = frequencyHz;
}

FT_STATUS ioWrite::writeLoop(int cycles) {
    FT_STATUS ftStatus = FT_OK;
    useconds_t delay_us = static_cast<useconds_t>(1000000.0 / frequencyHz);
    std::size_t bufferSize = buffer->size();
    BYTE *data = buffer->data();
    std::size_t offset = 0;

    for (int i = 0; i < cycles; i++) {
        ftStatus = device->write(&data[offset], M);
        if (ftStatus != FT_OK) {
            fprintf(stderr, "Error: write failed at cycle %d (status %lu)\n",
                    i, (unsigned long)ftStatus);
            return ftStatus;
        }

        printf("Write cycle %d: 0x%02X\n", i, data[offset]);

        offset = (offset + M) % bufferSize;
        io_sleep_us(delay_us);
    }

    return ftStatus;
}
