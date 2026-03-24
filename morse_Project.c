#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#define usleep(x) Sleep((x) / 1000)
#else
#include <unistd.h>
#endif
#include "ftd2xx.h"

// Define Morse code for each letter and number
const char *morse_code[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

// Function to send a Morse code character
void sendMorseCodeCharacter(FT_HANDLE ftHandle, char c) {
    FT_STATUS ftStatus;       // Variable to store FTDI function status
    DWORD bytesWritten;       // Variable to store the number of bytes written
    BYTE outputBuffer = 0x01; // Assume LED on pin 0
    const char *morse;        // Pointer to store Morse code for the character

    // Determine Morse code for the character
    if (c >= 'A' && c <= 'Z') {
        morse = morse_code[c - 'A'];
    } else if (c >= 'a' && c <= 'z') {
        morse = morse_code[c - 'a'];
    } else if (c >= '0' && c <= '9') {
        morse = morse_code[c - '0' + 26];
    } else {
        return; // Unsupported character
    }

    // Print Morse code for the character
    printf("Morse code for '%c': %s\n", c, morse);

    // Send Morse code to the FTDI chip
    while (*morse) {
        if (*morse == '.') {
            outputBuffer = 0x01; // Turn LED on for dot
            ftStatus = FT_Write(ftHandle, &outputBuffer, sizeof(outputBuffer), &bytesWritten);
            usleep(100000); // 1 unit for dot
        } else if (*morse == '-') {
            outputBuffer = 0x01; // Turn LED on for dash
            ftStatus = FT_Write(ftHandle, &outputBuffer, sizeof(outputBuffer), &bytesWritten);
            usleep(300000); // 3 units for dash
        }
        outputBuffer = 0x00; // Turn LED off
        ftStatus = FT_Write(ftHandle, &outputBuffer, sizeof(outputBuffer), &bytesWritten);
        usleep(100000); // 1 unit space between parts of the same letter
        morse++;
    }
    usleep(200000); // 3 units space between letters
}

// Function to send a Morse code message
void sendMorseCode(FT_HANDLE ftHandle) {
    char input[256];  // Buffer to store user input
    printf("Enter your message (type 'E0' to finish): ");

    // Loop to read user input
    while (1) {
        fgets(input, sizeof(input), stdin);  // Read user input
        if (strncmp(input, "E0", 2) == 0) {  // Check if the user typed 'E0'
            printf("Exiting Morse code sender...\n");
            break; // Exit loop if user types 'E0'
        }

        // Loop through each character in the input
        for (int i = 0; input[i] != '\0' && input[i] != '\n'; ++i) {
            if (input[i] == 'E' && input[i+1] == '0' && (input[i+2] == '\0' || input[i+2] == '\n')) {
                break; // Stop processing if 'E0' is found at the end of input
            }
            sendMorseCodeCharacter(ftHandle, input[i]);  // Send Morse code for each character
        }
    }
    printf("\nMorse code message sent.\n");
}
