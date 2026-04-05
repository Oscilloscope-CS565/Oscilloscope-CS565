#include "ioRead.h"
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#define io_sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define io_sleep_us(us) usleep(us)
#endif

ioRead::ioRead(FtdiDevice *device)
    : device(device), buffer(nullptr), N(0), frequencyHz(0.0) {}

void ioRead::configure(ioBuffer *buffer, std::size_t N, double frequencyHz) {
    this->buffer = buffer;
    this->N = N;
    this->frequencyHz = frequencyHz;
}

FT_STATUS ioRead::readLoop(int cycles) {
    FT_STATUS ftStatus = FT_OK;
    useconds_t delay_us = static_cast<useconds_t>(1000000.0 / frequencyHz);
    BYTE *data = buffer->data();

    for (int i = 0; i < cycles; i++) {
        ftStatus = device->read(data, N);
        if (ftStatus != FT_OK) {
            fprintf(stderr, "Error: read failed at cycle %d (status %lu)\n",
                    i, (unsigned long)ftStatus);
            return ftStatus;
        }

        printf("Read cycle %d: 0x%02X\n", i, data[0]);

        io_sleep_us(delay_us);
    }

    return ftStatus;
}
