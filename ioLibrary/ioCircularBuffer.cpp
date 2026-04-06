#include "ioCircularBuffer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

CircularBuffer::CircularBuffer(std::size_t capacity)
    : capacity(capacity), head(0), tail(0), count(0), finished(false) {
    storage = static_cast<BYTE *>(std::malloc(capacity));
    if (storage == nullptr) {
        fprintf(stderr, "Error: Failed to allocate CircularBuffer of size %zu\n", capacity);
        std::abort();
    }
    std::memset(storage, 0, capacity);
}

CircularBuffer::~CircularBuffer() {
    if (storage != nullptr) {
        std::free(storage);
        storage = nullptr;
    }
}

bool CircularBuffer::write(const BYTE *data, std::size_t len) {
    std::unique_lock<std::mutex> lock(mtx);

    for (std::size_t i = 0; i < len; i++) {
        // Wait until there is space or we are told to stop
        notFull.wait(lock, [this]() {
            return count < capacity || finished;
        });

        if (finished) {
            return false;
        }

        storage[head] = data[i];
        head = (head + 1) % capacity;
        count++;

        notEmpty.notify_one();
    }

    return true;
}

bool CircularBuffer::read(BYTE *data, std::size_t len) {
    std::unique_lock<std::mutex> lock(mtx);

    for (std::size_t i = 0; i < len; i++) {
        // Wait until there is data or producer is done
        notEmpty.wait(lock, [this]() {
            return count > 0 || finished;
        });

        if (count == 0 && finished) {
            return false;
        }

        data[i] = storage[tail];
        tail = (tail + 1) % capacity;
        count--;

        notFull.notify_one();
    }

    return true;
}

void CircularBuffer::setDone() {
    std::lock_guard<std::mutex> lock(mtx);
    finished = true;
    notEmpty.notify_all();
    notFull.notify_all();
}

std::size_t CircularBuffer::getCount() {
    std::lock_guard<std::mutex> lock(mtx);
    return count;
}

std::size_t CircularBuffer::getCapacity() const {
    return capacity;
}
