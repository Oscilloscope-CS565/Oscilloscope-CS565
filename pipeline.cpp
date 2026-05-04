#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <thread>
#include "ioByteSink.h"
#include "ioByteSource.h"
#include "ioCircularBuffer.h"
#include "ioFtdiDevice.h"
#include "ioThreadedReader.h"
#include "ioThreadedWriter.h"

static void printUsage(const char *progName) {
    printf("Usage: %s [options]\n", progName);
    printf("\nOptions:\n");
    printf("  --input-file <path>     Read bytes from file (instead of FTDI index 0)\n");
    printf("  --output-file <path>    Write to file (default: output.bin)\n");
    printf("  --output-ftdi <index>   Write to FTDI device at index\n");
    printf("  --freq <hz>             Read/write frequency in Hz (default: 10)\n");
    printf("  --duration <sec>      Run duration in seconds (default: 10)\n");
    printf("  --bufsize <bytes>       Circular buffer size in bytes (default: 1024)\n");
    printf("  --help                  Show this help message\n");
}

int main(int argc, char *argv[]) {
    const char *inputFilePath = nullptr;
    const char *outputFilePath = "output.bin";
    int outputFtdiIndex = -1;
    double frequencyHz = 10.0;
    int durationSec = 10;
    std::size_t bufSize = 1024;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input-file") == 0 && i + 1 < argc) {
            inputFilePath = argv[++i];
        } else if (strcmp(argv[i], "--output-file") == 0 && i + 1 < argc) {
            outputFilePath = argv[++i];
            outputFtdiIndex = -1;
        } else if (strcmp(argv[i], "--output-ftdi") == 0 && i + 1 < argc) {
            outputFtdiIndex = atoi(argv[++i]);
            outputFilePath = nullptr;
        } else if (strcmp(argv[i], "--freq") == 0 && i + 1 < argc) {
            frequencyHz = atof(argv[++i]);
        } else if (strcmp(argv[i], "--duration") == 0 && i + 1 < argc) {
            durationSec = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--bufsize") == 0 && i + 1 < argc) {
            bufSize = static_cast<std::size_t>(atoi(argv[++i]));
        } else if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    printf("=== Multithreaded Data Acquisition Pipeline ===\n");
    printf("Frequency: %.1f Hz\n", frequencyHz);
    printf("Duration:  %d seconds\n", durationSec);
    printf("Buffer:    %zu bytes\n", bufSize);
    if (inputFilePath != nullptr) {
        printf("Input:     file (%s)\n", inputFilePath);
    } else {
        printf("Input:     FTDI device (index 0)\n");
    }
    if (outputFilePath != nullptr) {
        printf("Output:    file (%s)\n", outputFilePath);
    } else {
        printf("Output:    FTDI device (index %d)\n", outputFtdiIndex);
    }
    printf("\n");

    ioFtdiDevice::FtdiDevice readDev;
    if (inputFilePath == nullptr) {
        if (readDev.open(0) != FT_OK) {
            fprintf(stderr, "Failed to open input FTDI device. Exiting.\n");
            return EXIT_FAILURE;
        }
    }

    ioFtdiDevice::FtdiDevice writeDev;
    if (outputFtdiIndex >= 0) {
        if (writeDev.open(outputFtdiIndex) != FT_OK) {
            fprintf(stderr, "Failed to open output FTDI device at index %d. Exiting.\n", outputFtdiIndex);
            if (inputFilePath == nullptr) {
                readDev.close();
            }
            return EXIT_FAILURE;
        }
    }

    ioCircularBuffer::CircularBuffer circBuf(bufSize);
    printf("Circular buffer created (%zu bytes).\n\n", bufSize);

    ioThreadedReader::ThreadedReader *reader = nullptr;
    if (inputFilePath != nullptr) {
        reader = new ioThreadedReader::ThreadedReader(std::make_unique<ioByteSource::FileByteSource>(inputFilePath));
    } else {
        reader = new ioThreadedReader::ThreadedReader(std::make_unique<ioByteSource::FtdiByteSource>(&readDev));
    }
    reader->configure(&circBuf, 1, frequencyHz);

    ioThreadedWriter::ThreadedWriter *writer = nullptr;
    if (outputFtdiIndex >= 0) {
        writer = new ioThreadedWriter::ThreadedWriter(std::make_unique<ioByteSink::FtdiByteSink>(&writeDev));
    } else {
        writer = new ioThreadedWriter::ThreadedWriter(std::make_unique<ioByteSink::FileByteSink>(outputFilePath));
    }
    writer->configure(&circBuf, 1, frequencyHz);

    printf("--- Starting pipeline ---\n");
    reader->start();
    writer->start();

    printf("Running for %d seconds...\n\n", durationSec);
    std::this_thread::sleep_for(std::chrono::seconds(durationSec));

    printf("\n--- Stopping pipeline ---\n");
    reader->stop();
    writer->stop();

    delete reader;
    delete writer;
    printf("\nPipeline complete.\n");

    if (inputFilePath == nullptr) {
        readDev.close();
    }
    if (outputFtdiIndex >= 0) {
        writeDev.close();
    }

    return EXIT_SUCCESS;
}
