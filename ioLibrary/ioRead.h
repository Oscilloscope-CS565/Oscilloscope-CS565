#ifndef IO_READ_H
#define IO_READ_H

#include "ioFtdiDevice.h"
#include "ioBuffer.h"
#include <cstddef>

class ioRead {
private:
    FtdiDevice *device;   // composition — uses but does not own the device
    ioBuffer   *buffer;   // aggregation — uses but does not own the buffer
    std::size_t N;
    double      frequencyHz;

public:
    ioRead(FtdiDevice *device);

    void      configure(ioBuffer *buffer, std::size_t N, double frequencyHz);
    FT_STATUS readLoop(int cycles);
};

#endif
