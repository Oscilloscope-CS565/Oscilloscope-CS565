#include "FtdiDevice.h"
#include <stdio.h>

FT_STATUS FtdiDevice_open(FtdiDevice *dev, int deviceIndex) {
    FT_STATUS ftStatus;

    ftStatus = FT_Open(deviceIndex, &dev->handle);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_Open failed (status %lu)\n", (unsigned long)ftStatus);
        return ftStatus;
    }
    printf("Device opened successfully.\n");

    ftStatus = FT_ResetDevice(dev->handle);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_ResetDevice failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(dev->handle);
        return ftStatus;
    }
    printf("Device reset successfully.\n");

    ftStatus = FT_Purge(dev->handle, FT_PURGE_RX | FT_PURGE_TX);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_Purge failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(dev->handle);
        return ftStatus;
    }
    printf("Purged USB buffers successfully.\n");

    ftStatus = FT_SetUSBParameters(dev->handle, 64, 0);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_SetUSBParameters failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(dev->handle);
        return ftStatus;
    }
    printf("USB parameters set successfully.\n");

    ftStatus = FT_SetBitMode(dev->handle, 0xFF, 0x01);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_SetBitMode failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(dev->handle);
        return ftStatus;
    }
    printf("Set synchronous bit bang mode successfully.\n");

    return FT_OK;
}

FT_STATUS FtdiDevice_close(FtdiDevice *dev) {
    FT_STATUS ftStatus = FT_Close(dev->handle);
    if (ftStatus == FT_OK) {
        printf("Device closed.\n");
    }
    return ftStatus;
}

FT_STATUS FtdiDevice_read(FtdiDevice *dev, BYTE *bytes, size_t n) {
    DWORD bytesRead;
    FT_Purge(dev->handle, FT_PURGE_RX);
    return FT_Read(dev->handle, bytes, (DWORD)n, &bytesRead);
}

FT_STATUS FtdiDevice_write(FtdiDevice *dev, BYTE *bytes, size_t m) {
    DWORD bytesWritten;
    return FT_Write(dev->handle, bytes, (DWORD)m, &bytesWritten);
}
