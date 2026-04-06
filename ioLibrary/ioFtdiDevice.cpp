#include "ioFtdiDevice.h"
#include <cstdio>

FtdiDevice::FtdiDevice() : handle(nullptr) {}

FtdiDevice::~FtdiDevice() {
    if (handle != nullptr) {
        close();
    }
}

FT_STATUS FtdiDevice::open(int deviceIndex) {
    FT_STATUS ftStatus;

    ftStatus = FT_Open(deviceIndex, &handle);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_Open failed (status %lu)\n", (unsigned long)ftStatus);
        return ftStatus;
    }
    printf("Device opened successfully.\n");

    ftStatus = FT_ResetDevice(handle);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_ResetDevice failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(handle);
        handle = nullptr;
        return ftStatus;
    }
    printf("Device reset successfully.\n");

    ftStatus = FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_Purge failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(handle);
        handle = nullptr;
        return ftStatus;
    }
    printf("Purged USB buffers successfully.\n");

    ftStatus = FT_SetUSBParameters(handle, 64, 0);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_SetUSBParameters failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(handle);
        handle = nullptr;
        return ftStatus;
    }
    printf("USB parameters set successfully.\n");

    ftStatus = FT_SetBitMode(handle, 0xFF, 0x01);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_SetBitMode failed (status %lu)\n", (unsigned long)ftStatus);
        FT_Close(handle);
        handle = nullptr;
        return ftStatus;
    }
    printf("Set synchronous bit bang mode successfully.\n");

    return FT_OK;
}

FT_STATUS FtdiDevice::close() {
    FT_STATUS ftStatus = FT_Close(handle);
    if (ftStatus == FT_OK) {
        handle = nullptr;
        printf("Device closed.\n");
    }
    return ftStatus;
}

FT_STATUS FtdiDevice::read(BYTE *bytes, std::size_t n) {
    DWORD bytesRead;
    FT_Purge(handle, FT_PURGE_RX);
    return FT_Read(handle, bytes, (DWORD)n, &bytesRead);
}

FT_STATUS FtdiDevice::write(BYTE *bytes, std::size_t m) {
    DWORD bytesWritten;
    return FT_Write(handle, bytes, (DWORD)m, &bytesWritten);
}
