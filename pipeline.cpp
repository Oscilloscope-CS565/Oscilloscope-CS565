#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>
#include "ioFtdiDevice.h"
#include "ioCircularBuffer.h"
#include "ioThreadedReader.h"
#include "ioThreadedWriter.h"

static void printUsage(const char *progName) {
    printf("Usage: %s [options]\n", progName);
    printf("\nOptions:\n");
    printf("  --output-file <path>    Write to file (default: output.bin)\n");
    printf("  --output-ftdi <index>   Write to FTDI device at index\n");
    printf("  --freq <hz>             Read/write frequency in Hz (default: 10)\n");
    printf("  --duration <sec>        Run duration in seconds (default: 10)\n");
    printf("  --bufsize <bytes>       Circular buffer size in bytes (default: 1024)\n");
    printf("  --help                  Show this help message\n");
}

int main(int argc, char *argv[]) {
    // Default parameters
    const char *outputFilePath = "output.bin";
    int outputFtdiIndex = -1;
    double frequencyHz = 10.0;
    int durationSec = 10;
    std::size_t bufSize = 1024;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--output-file") == 0 && i + 1 < argc) {
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
    if (outputFilePath != nullptr) {
        printf("Output:    file (%s)\n", outputFilePath);
    } else {
        printf("Output:    FTDI device (index %d)\n", outputFtdiIndex);
    }
    printf("\n");

    // 1. Open input FTDI device
    FtdiDevice readDev;
    if (readDev.open(0) != FT_OK) {
        fprintf(stderr, "Failed to open input FTDI device. Exiting.\n");
        return EXIT_FAILURE;
    }

    // 2. Optionally open output FTDI device
    FtdiDevice writeDev;
    if (outputFtdiIndex >= 0) {
        if (writeDev.open(outputFtdiIndex) != FT_OK) {
            fprintf(stderr, "Failed to open output FTDI device at index %d. Exiting.\n", outputFtdiIndex);
            return EXIT_FAILURE;
        }
    }

    // 3. Create circular buffer
    CircularBuffer circBuf(bufSize);
    printf("Circular buffer created (%zu bytes).\n\n", bufSize);

    // 4. Create and configure reader
    ThreadedReader reader(&readDev);
    reader.configure(&circBuf, 1, frequencyHz);

    // 5. Create and configure writer
    ThreadedWriter *writer = nullptr;
    if (outputFtdiIndex >= 0) {
        writer = new ThreadedWriter(&writeDev);
    } else {
        writer = new ThreadedWriter(outputFilePath);
    }
    writer->configure(&circBuf, 1, frequencyHz);

    // 6. Start pipeline
    printf("--- Starting pipeline ---\n");
    reader.start();
    writer->start();

    // 7. Run for specified duration
    printf("Running for %d seconds...\n\n", durationSec);
    std::this_thread::sleep_for(std::chrono::seconds(durationSec));

    // 8. Stop pipeline: reader first (signals done), then writer (drains remaining data)
    printf("\n--- Stopping pipeline ---\n");
    reader.stop();
    writer->stop();

    // 9. Cleanup
    delete writer;
    printf("\nPipeline complete.\n");

    return EXIT_SUCCESS;
}
