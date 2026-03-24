#ifndef IO_BUFFER_H
#define IO_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#else
#include "WinTypes.h"
#endif

typedef struct {
    BYTE   *storage;
    size_t  length;
} ioBuffer;

int     ioBuffer_create(ioBuffer *buf, size_t capacity);
BYTE*   ioBuffer_data(const ioBuffer *buf);
size_t  ioBuffer_size(const ioBuffer *buf);
void    ioBuffer_destroy(ioBuffer *buf);

#endif
