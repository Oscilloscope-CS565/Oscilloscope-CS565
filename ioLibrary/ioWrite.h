#ifndef IO_WRITE_H
#define IO_WRITE_H

#include "ioFtdiDevice.h"
#include "ioBuffer.h"
#include <cstddef>

class ioWrite {
private:
    FtdiDevice *device;   // composition — uses but does not own the device
    ioBuffer   *buffer;   // aggregation — uses but does not own the buffer
    std::size_t M;
    double      frequencyHz;

public:
    ioWrite(FtdiDevice *device);

    void      configure(ioBuffer *buffer, std::size_t M, double frequencyHz);
    FT_STATUS writeLoop(int cycles);
};

#endif
