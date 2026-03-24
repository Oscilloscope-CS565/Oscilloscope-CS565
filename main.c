#include <stdio.h>
#include <stdlib.h>
#include "ioBuffer.h"
#include "FtdiDevice.h"
#include "ioWrite.h"
#include "ioRead.h"

void runBlink1Hz(ioWrite *writer, ioBuffer *buf) {
    printf("\n--- Blinking LEDs at 1 Hz (10 cycles, ~10 seconds) ---\n");
    ioWrite_configure(writer, buf, 1, 1.0);
    FT_STATUS status = ioWrite_writeLoop(writer, 10);
    if (status != FT_OK) {
        fprintf(stderr, "Write test at 1 Hz failed.\n");
    }
}

void runBlink2Hz(ioWrite *writer, ioBuffer *buf) {
    printf("\n--- Blinking LEDs at 2 Hz (20 cycles, ~10 seconds) ---\n");
    ioWrite_configure(writer, buf, 1, 2.0);
    FT_STATUS status = ioWrite_writeLoop(writer, 20);
    if (status != FT_OK) {
        fprintf(stderr, "Write test at 2 Hz failed.\n");
    }
}

int main(void) {
    FtdiDevice dev;
    FT_STATUS ftStatus;

    // Open and initialize the FTDI device
    ftStatus = FtdiDevice_open(&dev, 0);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Failed to open device. Exiting.\n");
        return EXIT_FAILURE;
    }

    // Create a 2-byte buffer for the blink pattern: 0xFF (all ON), 0x00 (all OFF)
    ioBuffer buf;
    if (ioBuffer_create(&buf, 2) != 0) {
        FtdiDevice_close(&dev);
        return EXIT_FAILURE;
    }
    ioBuffer_data(&buf)[0] = 0xFF;
    ioBuffer_data(&buf)[1] = 0x00;

    // Create writer with composition to device
    ioWrite writer;
    writer.device = &dev;

    // Run blink tests
    runBlink1Hz(&writer, &buf);
    runBlink2Hz(&writer, &buf);

    // Optional: demonstrate ioRead
    printf("\n--- Reading from device (5 cycles at 1 Hz) ---\n");
    ioBuffer readBuf;
    if (ioBuffer_create(&readBuf, 1) == 0) {
        ioRead reader;
        reader.device = &dev;
        ioRead_configure(&reader, &readBuf, 1, 1.0);
        ioRead_readLoop(&reader, 5);
        printf("Last read value: 0x%02X\n", ioBuffer_data(&readBuf)[0]);
        ioBuffer_destroy(&readBuf);
    }

    // Cleanup
    ioBuffer_destroy(&buf);
    FtdiDevice_close(&dev);

    return EXIT_SUCCESS;
}
