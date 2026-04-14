#ifndef IO_READ_H
#define IO_READ_H

#include "ioFtdiDevice.h"
#include "ioBuffer.h"
#include <cstddef>

namespace ioRead {

class ioRead {
private:
    ioFtdiDevice::FtdiDevice *device;
    ioBuffer::ioBuffer       *buffer;
    std::size_t N;
    double      frequencyHz;

public:
    ioRead(ioFtdiDevice::FtdiDevice *device);

    void      configure(ioBuffer::ioBuffer *buffer, std::size_t N, double frequencyHz);
    FT_STATUS readLoop(int cycles);
};

} // namespace ioRead

#endif
