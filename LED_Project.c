#include <stdio.h>      // Standard input/output library
#include <stdlib.h>     // Standard library for memory allocation, process control, etc.
#include <string.h>     // String handling library
#ifdef _WIN32           // Preprocessor directive to include specific headers for Windows
#include <windows.h>    // Windows API functions
#else                   // If not Windows, then assume Unix-like system
#include <unistd.h>     // Unix standard library for POSIX API
#endif
#include "ftd2xx.h"     // FTDI library for communication with the UM245R chip

void displayPinStates(BYTE outputBuffer, int numberOfPins) {
    for (int pin = 0; pin < numberOfPins; ++pin) {
        printf("Pin %d = %s\n", pin, (outputBuffer & (1 << pin)) ? "ON" : "OFF");
    }
}

void controlLED(FT_HANDLE ftHandle) {
    const int BUFFER_LENGTH = 256; // Buffer length for input
    const int NUMBER_OF_PINS = 8;  // Total number of pins available
    const int PIN_ID_FIRST = 0;    // First pin ID
    const int PIN_ID_LAST = 7;     // Last pin ID
    const int PIN_STATE_ON = 1;    // State value for ON
    const int PIN_STATE_OFF = 0;   // State value for OFF

    char input[256];     // Buffer to store user input
    int pin;                       // Variable to store pin number
    int state;                     // Variable to store pin state
    FT_STATUS ftStatus;            // Variable to store FTDI function status
    DWORD bytesWritten;            // Variable to store the number of bytes written
    BYTE outputBuffer = 0x00;      // Initialize with all pins off

    while (1) {                    // Infinite loop to continuously ask for user input
        // Get pin number from the user
        printf("\nEnter the pin number (0-7), 'reset' to turn off all pins, or 'done' to finish:\n> ");
        fgets(input, sizeof(input), stdin);

        if (strncmp(input, "done", strlen("done")) == 0) {  // Check if the user typed 'done'
            printf("Exiting LED control...\n");
            break; // Exit loop if user types 'done'
        }

        if (strncmp(input, "reset", strlen("reset")) == 0) {  // Check if the user typed 'reset'
            outputBuffer = 0x00;  // Turn off all pins
            printf("All pins reset to OFF.\n");

            // Write the output buffer to the FTDI chip
            ftStatus = FT_Write(ftHandle, &outputBuffer, sizeof(outputBuffer), &bytesWritten);
            if (ftStatus != FT_OK) {  // Check if the write operation was successful
                fprintf(stderr, "Error: FT_Write failed\n");  // Print error message
                FT_Close(ftHandle);  // Close the FTDI handle
                exit(EXIT_FAILURE);  // Exit the program with a failure status
            }
            continue;  // Continue to the next iteration of the loop
        }

        pin = atoi(input); // Convert input to integer

        if (pin < PIN_ID_FIRST || pin > PIN_ID_LAST) {  // Check if the pin number is valid
            printf("Error: Invalid pin number %d. Please enter a value between %d and %d.\n", pin, PIN_ID_FIRST, PIN_ID_LAST);
            continue; // Ask for the pin number again
        }

        // Display current states of all pins
        printf("Current pin states:\n");
        displayPinStates(outputBuffer, NUMBER_OF_PINS);

        // Get pin state from the user
        printf("Enter the new state for pin %d (1 for ON, 0 for OFF):\n> ", pin);
        fgets(input, sizeof(input), stdin);

        state = atoi(input); // Convert input to integer

        if (state != PIN_STATE_OFF && state != PIN_STATE_ON) {  // Validate the state value
            printf("Error: Invalid state %d for pin %d. Please enter %d for OFF or %d for ON.\n", state, pin, PIN_STATE_OFF, PIN_STATE_ON);
            continue; // Ask for the pin state again
        }

        // Update the output buffer based on the state
        if (state == PIN_STATE_ON) {    // If the state is ON
            outputBuffer |= (1 << pin); // Set the bit corresponding to the pin to 1
            printf("Pin %d is ON.\n", pin);
        } else {                        // If the state is OFF
            outputBuffer &= ~(1 << pin); // Set the bit corresponding to the pin to 0
            printf("Pin %d is OFF.\n", pin);
        }

        // Write the output buffer to the FTDI chip
        ftStatus = FT_Write(ftHandle, &outputBuffer, sizeof(outputBuffer), &bytesWritten);
        if (ftStatus != FT_OK) {  // Check if the write operation was successful
            fprintf(stderr, "Error: FT_Write failed\n");  // Print error message
            FT_Close(ftHandle);  // Close the FTDI handle
            exit(EXIT_FAILURE);  // Exit the program with a failure status
        }
    }
    printf("Final LED states updated. Wrote %d bytes: 0x%02X\n", bytesWritten, outputBuffer);  // Print the result
}