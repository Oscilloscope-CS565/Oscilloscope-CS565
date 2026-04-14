#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <cstddef>
#include <mutex>
#include <condition_variable>

#ifdef _WIN32
#include <windows.h>
#else
#include "WinTypes.h"
#endif

namespace ioCircularBuffer {

class CircularBuffer {
private:
    BYTE  *storage;
    std::size_t capacity;
    std::size_t head;
    std::size_t tail;
    std::size_t count;
    std::mutex mtx;
    std::condition_variable notEmpty;
    std::condition_variable notFull;
    bool finished;

public:
    CircularBuffer(std::size_t capacity);
    ~CircularBuffer();

    bool write(const BYTE *data, std::size_t len);
    bool read(BYTE *data, std::size_t len);
    void setDone();
    std::size_t getCount();
    std::size_t getCapacity() const;
};

} // namespace ioCircularBuffer

#endif
