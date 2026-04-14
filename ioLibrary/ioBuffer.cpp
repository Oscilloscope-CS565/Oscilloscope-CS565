#include "ioBuffer.h"
#include <cstdio>

namespace ioBuffer {

ioBuffer::ioBuffer() : storage(nullptr), length(0) {}

ioBuffer::~ioBuffer() {
    destroy();
}

int ioBuffer::create(std::size_t capacity) {
    storage = static_cast<BYTE *>(std::malloc(capacity * sizeof(BYTE)));
    if (storage == nullptr) {
        fprintf(stderr, "Error: Failed to allocate ioBuffer of size %zu\n", capacity);
        return -1;
    }
    std::memset(storage, 0, capacity);
    length = capacity;
    return 0;
}

BYTE* ioBuffer::data() const {
    return storage;
}

std::size_t ioBuffer::size() const {
    return length;
}

void ioBuffer::destroy() {
    if (storage != nullptr) {
        std::free(storage);
        storage = nullptr;
    }
    length = 0;
}

} // namespace ioBuffer
