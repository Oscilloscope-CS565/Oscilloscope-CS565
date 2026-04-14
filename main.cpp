#include <cstdio>
#include <cstdlib>
#include "ioBuffer.h"
#include "ioFtdiDevice.h"
#include "ioWrite.h"
#include "ioRead.h"

void runBlink1Hz(ioWrite::ioWrite &writer, ioBuffer::ioBuffer &buf) {
    printf("\n--- Blinking LEDs at 1 Hz (10 cycles, ~10 seconds) ---\n");
    writer.configure(&buf, 1, 1.0);
    FT_STATUS status = writer.writeLoop(10);
    if (status != FT_OK) {
        fprintf(stderr, "Write test at 1 Hz failed.\n");
    }
}

void runBlink2Hz(ioWrite::ioWrite &writer, ioBuffer::ioBuffer &buf) {
    printf("\n--- Blinking LEDs at 2 Hz (20 cycles, ~10 seconds) ---\n");
    writer.configure(&buf, 1, 2.0);
    FT_STATUS status = writer.writeLoop(20);
    if (status != FT_OK) {
        fprintf(stderr, "Write test at 2 Hz failed.\n");
    }
}

int main() {
    ioFtdiDevice::FtdiDevice dev;
    if (dev.open(0) != FT_OK) {
        fprintf(stderr, "Failed to open device. Exiting.\n");
        return EXIT_FAILURE;
    }

    ioBuffer::ioBuffer buf;
    if (buf.create(2) != 0) {
        return EXIT_FAILURE;
    }
    buf.data()[0] = 0xFF;
    buf.data()[1] = 0x00;

    ioWrite::ioWrite writer(&dev);

    runBlink1Hz(writer, buf);
    runBlink2Hz(writer, buf);

    printf("\n--- Reading from device (5 cycles at 1 Hz) ---\n");
    ioBuffer::ioBuffer readBuf;
    if (readBuf.create(1) == 0) {
        ioRead::ioRead reader(&dev);
        reader.configure(&readBuf, 1, 1.0);
        reader.readLoop(5);
        printf("Last read value: 0x%02X\n", readBuf.data()[0]);
    }

    return EXIT_SUCCESS;
}
