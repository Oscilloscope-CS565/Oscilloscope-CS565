#ifndef IO_BYTE_SINK_H
#define IO_BYTE_SINK_H

#include "ftd2xx.h"
#include <cstddef>
#include <cstdio>

namespace ioFtdiDevice {
class FtdiDevice;
}

namespace ioByteSink {

/** Consumes fixed-size byte chunks from the pipeline (device, file capture, etc.). */
class ByteSink {
public:
    virtual ~ByteSink() = default;
    virtual FT_STATUS writeBytes(const BYTE *buffer, std::size_t byteCount) = 0;
};

/** Writes via existing `FtdiDevice` (does not own the device). */
class FtdiByteSink final : public ByteSink {
public:
    explicit FtdiByteSink(ioFtdiDevice::FtdiDevice *device);
    FT_STATUS writeBytes(const BYTE *buffer, std::size_t byteCount) override;

private:
    ioFtdiDevice::FtdiDevice *device_;
};

/**
 * Appends raw bytes to a FILE (e.g. capture.bin).
 * Flushes after each successful write so data survives crashes during demos.
 */
class FileByteSink final : public ByteSink {
public:
    /** Opens path with fopen("wb"); owns the FILE. */
    explicit FileByteSink(const char *path);
    /** Wrap an existing handle; if takeOwnership, fclose in destructor. */
    explicit FileByteSink(FILE *file, bool takeOwnership);
    ~FileByteSink() override;

    FileByteSink(const FileByteSink &) = delete;
    FileByteSink &operator=(const FileByteSink &) = delete;

    FT_STATUS writeBytes(const BYTE *buffer, std::size_t byteCount) override;

private:
    FILE *file_;
    bool ownsFile_;
};

} // namespace ioByteSink

#endif
