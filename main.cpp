#include <cstdio>
#include <cstdlib>
#include "ioBuffer.h"
#include "FtdiDevice.h"
#include "ioWrite.h"
#include "ioRead.h"

void runBlink1Hz(ioWrite &writer, ioBuffer &buf) {
    printf("\n--- Blinking LEDs at 1 Hz (10 cycles, ~10 seconds) ---\n");
    writer.configure(&buf, 1, 1.0);
    FT_STATUS status = writer.writeLoop(10);
    if (status != FT_OK) {
        fprintf(stderr, "Write test at 1 Hz failed.\n");
    }
}

void runBlink2Hz(ioWrite &writer, ioBuffer &buf) {
    printf("\n--- Blinking LEDs at 2 Hz (20 cycles, ~10 seconds) ---\n");
    writer.configure(&buf, 1, 2.0);
    FT_STATUS status = writer.writeLoop(20);
    if (status != FT_OK) {
        fprintf(stderr, "Write test at 2 Hz failed.\n");
    }
}

int main() {
    // Open and initialize the FTDI device
    FtdiDevice dev;
    if (dev.open(0) != FT_OK) {
        fprintf(stderr, "Failed to open device. Exiting.\n");
        return EXIT_FAILURE;
    }

    // Create a 2-byte buffer for the blink pattern: 0xFF (all ON), 0x00 (all OFF)
    ioBuffer buf;
    if (buf.create(2) != 0) {
        return EXIT_FAILURE;
    }
    buf.data()[0] = 0xFF;
    buf.data()[1] = 0x00;

    // Create writer with composition to device
    ioWrite writer(&dev);

    // Run blink tests
    runBlink1Hz(writer, buf);
    runBlink2Hz(writer, buf);

    // Demonstrate ioRead
    printf("\n--- Reading from device (5 cycles at 1 Hz) ---\n");
    ioBuffer readBuf;
    if (readBuf.create(1) == 0) {
        ioRead reader(&dev);
        reader.configure(&readBuf, 1, 1.0);
        reader.readLoop(5);
        printf("Last read value: 0x%02X\n", readBuf.data()[0]);
    }

    return EXIT_SUCCESS;
}
