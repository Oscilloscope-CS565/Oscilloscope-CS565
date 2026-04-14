#ifndef IO_BUFFER_H
#define IO_BUFFER_H

#include <cstddef>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include "WinTypes.h"
#endif

namespace ioBuffer {

class ioBuffer {
private:
    BYTE   *storage;
    std::size_t length;

public:
    ioBuffer();
    ~ioBuffer();

    int  create(std::size_t capacity);
    BYTE* data() const;
    std::size_t size() const;
    void destroy();
};

} // namespace ioBuffer

#endif
