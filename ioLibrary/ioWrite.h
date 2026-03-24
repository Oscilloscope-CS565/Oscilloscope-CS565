#ifndef IO_WRITE_H
#define IO_WRITE_H

#include "FtdiDevice.h"
#include "ioBuffer.h"
#include <stddef.h>

typedef struct {
    FtdiDevice *device;
    ioBuffer   *buffer;
    size_t      M;
    double      frequencyHz;
} ioWrite;

void      ioWrite_configure(ioWrite *writer, ioBuffer *buffer, size_t M, double frequencyHz);
FT_STATUS ioWrite_writeLoop(ioWrite *writer, int cycles);

#endif
