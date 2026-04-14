#ifndef FTDI_DEVICE_H
#define FTDI_DEVICE_H

#include "ftd2xx.h"
#include <cstddef>
#include <mutex>

namespace ioFtdiDevice {

class FtdiDevice {
private:
    FT_HANDLE handle;
    std::mutex accessMutex;

public:
    FtdiDevice();
    ~FtdiDevice();

    FT_STATUS open(int deviceIndex);
    FT_STATUS close();
    FT_STATUS read(BYTE *bytes, std::size_t n);
    FT_STATUS write(BYTE *bytes, std::size_t m);
};

} // namespace ioFtdiDevice

#endif
