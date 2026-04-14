#ifndef IO_WRITE_H
#define IO_WRITE_H

#include "ioFtdiDevice.h"
#include "ioBuffer.h"
#include <cstddef>

namespace ioWrite {

class ioWrite {
private:
    ioFtdiDevice::FtdiDevice *device;
    ioBuffer::ioBuffer       *buffer;
    std::size_t M;
    double      frequencyHz;

public:
    ioWrite(ioFtdiDevice::FtdiDevice *device);

    void      configure(ioBuffer::ioBuffer *buffer, std::size_t M, double frequencyHz);
    FT_STATUS writeLoop(int cycles);
};

} // namespace ioWrite

#endif
