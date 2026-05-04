#include "ioByteSink.h"
#include "ioFtdiDevice.h"
#include <cstdio>

namespace ioByteSink {

FtdiByteSink::FtdiByteSink(ioFtdiDevice::FtdiDevice *device) : device_(device) {}

FT_STATUS FtdiByteSink::writeBytes(const BYTE *buffer, std::size_t byteCount) {
    if (device_ == nullptr) {
        return FT_IO_ERROR;
    }
    return device_->write(const_cast<BYTE *>(buffer), byteCount);
}

FileByteSink::FileByteSink(const char *path) : file_(std::fopen(path, "wb")), ownsFile_(true) {
    if (file_ == nullptr) {
        std::fprintf(stderr, "[FileByteSink] Failed to open '%s' for writing\n", path);
    }
}

FileByteSink::FileByteSink(FILE *file, bool takeOwnership) : file_(file), ownsFile_(takeOwnership) {}

FileByteSink::~FileByteSink() {
    if (ownsFile_ && file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
}

FT_STATUS FileByteSink::writeBytes(const BYTE *buffer, std::size_t byteCount) {
    if (file_ == nullptr || byteCount == 0U) {
        return FT_IO_ERROR;
    }
    const std::size_t wrote = std::fwrite(buffer, 1U, byteCount, file_);
    if (wrote != byteCount) {
        return FT_IO_ERROR;
    }
    std::fflush(file_);
    return FT_OK;
}

} // namespace ioByteSink
