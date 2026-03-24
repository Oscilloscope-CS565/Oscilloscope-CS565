#include "ioWrite.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define io_sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define io_sleep_us(us) usleep(us)
#endif

void ioWrite_configure(ioWrite *writer, ioBuffer *buffer, size_t M, double frequencyHz) {
    writer->buffer = buffer;
    writer->M = M;
    writer->frequencyHz = frequencyHz;
}

FT_STATUS ioWrite_writeLoop(ioWrite *writer, int cycles) {
    FT_STATUS ftStatus = FT_OK;
    useconds_t delay_us = (useconds_t)(1000000.0 / writer->frequencyHz);
    size_t bufferSize = ioBuffer_size(writer->buffer);
    BYTE *data = ioBuffer_data(writer->buffer);
    size_t offset = 0;

    for (int i = 0; i < cycles; i++) {
        ftStatus = FtdiDevice_write(writer->device, &data[offset], writer->M);
        if (ftStatus != FT_OK) {
            fprintf(stderr, "Error: FtdiDevice_write failed at cycle %d (status %lu)\n",
                    i, (unsigned long)ftStatus);
            return ftStatus;
        }

        printf("Write cycle %d: 0x%02X\n", i, data[offset]);

        offset = (offset + writer->M) % bufferSize;
        io_sleep_us(delay_us);
    }

    return ftStatus;
}
