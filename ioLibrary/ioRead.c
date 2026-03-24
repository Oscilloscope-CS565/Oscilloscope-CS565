#include "ioRead.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define io_sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define io_sleep_us(us) usleep(us)
#endif

void ioRead_configure(ioRead *reader, ioBuffer *buffer, size_t N, double frequencyHz) {
    reader->buffer = buffer;
    reader->N = N;
    reader->frequencyHz = frequencyHz;
}

FT_STATUS ioRead_readLoop(ioRead *reader, int cycles) {
    FT_STATUS ftStatus = FT_OK;
    useconds_t delay_us = (useconds_t)(1000000.0 / reader->frequencyHz);
    BYTE *data = ioBuffer_data(reader->buffer);

    for (int i = 0; i < cycles; i++) {
        ftStatus = FtdiDevice_read(reader->device, data, reader->N);
        if (ftStatus != FT_OK) {
            fprintf(stderr, "Error: FtdiDevice_read failed at cycle %d (status %lu)\n",
                    i, (unsigned long)ftStatus);
            return ftStatus;
        }

        printf("Read cycle %d: 0x%02X\n", i, data[0]);

        io_sleep_us(delay_us);
    }

    return ftStatus;
}
