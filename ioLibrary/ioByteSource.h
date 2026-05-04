#ifndef IO_BYTE_SOURCE_H
#define IO_BYTE_SOURCE_H

#include "ftd2xx.h"
#include <cstddef>
#include <cstdio>

namespace ioFtdiDevice {
class FtdiDevice;
}

namespace ioByteSource {

/** Produces fixed-size byte chunks for acquisition (device, file replay, etc.). */
class ByteSource {
public:
    virtual ~ByteSource() = default;
    virtual FT_STATUS readBytes(BYTE *buffer, std::size_t byteCount) = 0;
};

/** Reads via existing `FtdiDevice` (does not own the device). */
class FtdiByteSource final : public ByteSource {
public:
    explicit FtdiByteSource(ioFtdiDevice::FtdiDevice *device);
    FT_STATUS readBytes(BYTE *buffer, std::size_t byteCount) override;

private:
    ioFtdiDevice::FtdiDevice *device_;
};

/**
 * Reads raw bytes from a FILE stream (e.g. recorded capture).
 * A short read or I/O error yields FT_IO_ERROR so the reader thread can stop cleanly.
 */
class FileByteSource final : public ByteSource {
public:
    /** Opens path with fopen(mode); owns the FILE. */
    explicit FileByteSource(const char *path, const char *mode = "rb");
    /** Wrap an existing handle; if takeOwnership, fclose in destructor. */
    explicit FileByteSource(FILE *file, bool takeOwnership);
    ~FileByteSource() override;

    FileByteSource(const FileByteSource &) = delete;
    FileByteSource &operator=(const FileByteSource &) = delete;

    FT_STATUS readBytes(BYTE *buffer, std::size_t byteCount) override;

private:
    FILE *file_;
    bool ownsFile_;
};

} // namespace ioByteSource

#endif
