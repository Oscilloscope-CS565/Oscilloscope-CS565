#include "ioByteSource.h"
#include "ioFtdiDevice.h"
#include <cstdio>

namespace ioByteSource {

FtdiByteSource::FtdiByteSource(ioFtdiDevice::FtdiDevice *device) : device_(device) {}

FT_STATUS FtdiByteSource::readBytes(BYTE *buffer, std::size_t byteCount) {
    if (device_ == nullptr) {
        return FT_IO_ERROR;
    }
    return device_->read(buffer, byteCount);
}

FileByteSource::FileByteSource(const char *path, const char *mode)
    : file_(std::fopen(path, mode)), ownsFile_(true) {
    if (file_ == nullptr) {
        std::fprintf(stderr, "[FileByteSource] Failed to open '%s'\n", path);
    }
}

FileByteSource::FileByteSource(FILE *file, bool takeOwnership) : file_(file), ownsFile_(takeOwnership) {}

FileByteSource::~FileByteSource() {
    if (ownsFile_ && file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
}

FT_STATUS FileByteSource::readBytes(BYTE *buffer, std::size_t byteCount) {
    if (file_ == nullptr || byteCount == 0U) {
        return FT_IO_ERROR;
    }
    const std::size_t got = std::fread(buffer, 1U, byteCount, file_);
    if (got != byteCount) {
        return FT_IO_ERROR;
    }
    return FT_OK;
}

} // namespace ioByteSource
