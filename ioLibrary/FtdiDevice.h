#ifndef FTDI_DEVICE_H
#define FTDI_DEVICE_H

#include "ftd2xx.h"
#include <stddef.h>

typedef struct {
    FT_HANDLE handle;
} FtdiDevice;

FT_STATUS FtdiDevice_open(FtdiDevice *dev, int deviceIndex);
FT_STATUS FtdiDevice_close(FtdiDevice *dev);
FT_STATUS FtdiDevice_read(FtdiDevice *dev, BYTE *bytes, size_t n);
FT_STATUS FtdiDevice_write(FtdiDevice *dev, BYTE *bytes, size_t m);

#endif
