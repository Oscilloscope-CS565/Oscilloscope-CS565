#include "ioBuffer.h"
#include <stdio.h>

int ioBuffer_create(ioBuffer *buf, size_t capacity) {
    buf->storage = (BYTE *)malloc(capacity * sizeof(BYTE));
    if (buf->storage == NULL) {
        fprintf(stderr, "Error: Failed to allocate ioBuffer of size %zu\n", capacity);
        return -1;
    }
    memset(buf->storage, 0, capacity);
    buf->length = capacity;
    return 0;
}

BYTE* ioBuffer_data(const ioBuffer *buf) {
    return buf->storage;
}

size_t ioBuffer_size(const ioBuffer *buf) {
    return buf->length;
}

void ioBuffer_destroy(ioBuffer *buf) {
    if (buf->storage != NULL) {
        free(buf->storage);
        buf->storage = NULL;
    }
    buf->length = 0;
}
