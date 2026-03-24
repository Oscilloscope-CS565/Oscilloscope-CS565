#ifndef IO_READ_H
#define IO_READ_H

#include "FtdiDevice.h"
#include "ioBuffer.h"
#include <stddef.h>

typedef struct {
    FtdiDevice *device;
    ioBuffer   *buffer;
    size_t      N;
    double      frequencyHz;
} ioRead;

void      ioRead_configure(ioRead *reader, ioBuffer *buffer, size_t N, double frequencyHz);
FT_STATUS ioRead_readLoop(ioRead *reader, int cycles);

#endif
