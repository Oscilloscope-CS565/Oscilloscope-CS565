#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "ftd2xx.h"

// Function prototypes
void initializeDevice(FT_HANDLE *ftHandle);
void controlLED(FT_HANDLE ftHandle);
void sendMorseCode(FT_HANDLE ftHandle);

int main() {

    //printf("Attach debugger now. Press ENTER to continue...\n");
    //getchar();  // Wait for debugger to attach

    FT_HANDLE ftHandle;
    int choice;

    initializeDevice(&ftHandle);

    // variables for writing
    FT_STATUS ftStatus;
    unsigned char byteToWrite = 0x00;
    DWORD bytesWritten;

    // variables for reading
    unsigned char byteRead = 0;
    DWORD numberOfBytesRead;

    while (1) {
        printf("\nControl Menu\n");
        printf("1. Control LEDs\n");
        printf("2. Send Morse Code\n");
        printf("3. Write byte to port\n");
        printf("4. Read byte from port\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // to consume the newline character left by scanf
        //choice = 2;

        if (choice == 5) {
            break;
        }

        switch (choice) {
            case 1:
                controlLED(ftHandle);
                break;
            case 2:
                sendMorseCode(ftHandle);
                break;
            case 3:
                ftStatus=
                FT_Write(ftHandle,&byteToWrite,sizeof(byteToWrite),&bytesWritten);
                break;
            case 4:
                //- 0    all fine
                //- 1    read pins failed
                //- 2    USB device unavailable
                FT_Purge(ftHandle, FT_PURGE_RX);
                ftStatus = FT_Read(ftHandle,
                                    &byteRead,
                                    sizeof(byteRead),
                                    &numberOfBytesRead);
                if (ftStatus != FT_OK){
                    // report error;
                }else{
                    // report OK;
                }
                printf("Read %d bytes: 0x%02X\n", numberOfBytesRead, byteRead); 
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    // Close the device
    FT_Close(ftHandle);
    printf("Device closed.\n");

    return EXIT_SUCCESS;
}

void initializeDevice(FT_HANDLE *ftHandle) {
    FT_STATUS ftStatus;

    // Open the device
    ftStatus = FT_Open(0, ftHandle);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_Open failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Device opened successfully.\n");

    // Reset the device
    ftStatus = FT_ResetDevice(*ftHandle);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_ResetDevice failed\n");
        FT_Close(*ftHandle);
        exit(EXIT_FAILURE);
    }
    printf("Device reset successfully.\n");

    // Purge USB receive and transmit buffers
    ftStatus = FT_Purge(*ftHandle, FT_PURGE_RX | FT_PURGE_TX);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_Purge failed\n");
        FT_Close(*ftHandle);
        exit(EXIT_FAILURE);
    }
    printf("Purged USB buffers successfully.\n");

    // Set the USB parameters
    ftStatus = FT_SetUSBParameters(*ftHandle, 64, 0);
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_SetUSBParameters failed\n");
        FT_Close(*ftHandle);
        exit(EXIT_FAILURE);
    }
    printf("USB parameters set successfully.\n");

    // Set the device to synchronous bit bang mode
    ftStatus = FT_SetBitMode(*ftHandle, 0xFF, 0x01); // All pins as output, synchronous bit bang mode
    if (ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_SetBitMode failed\n");
        FT_Close(*ftHandle);
        exit(EXIT_FAILURE);
    }
    printf("Set synchronous bit bang mode successfully.\n");
}
