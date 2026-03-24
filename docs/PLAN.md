# Specification

### 1. Oscilloscope redesign using MVC and Pipe-and-Filter

Redesign the oscilloscope so that the user interface uses the Model–View–Controller (MVC) architectural pattern, while the signal-processing portion uses the Pipe-and-Filter pattern. Assume the system includes two filters: scpScale, which scales the signal, and scpOffset, which applies an offset to the signal. Provide a UML class diagram for the redesigned system. Use standard UML relationships, including inheritance, composition, and aggregation, and clearly label any additional associations by name. Include multiplicities on all relevant relationships.

### 2. Separating FTDI functionality into a reusable library

There are several ways to separate software responsibilities across modules. One approach is to ensure that each class has a single, well-defined responsibility and is implemented in its own file. Another approach is to place related functionality into a reusable library that can be called by other programs. Such libraries may be linked either statically at compile time or dynamically at runtime.

A natural decomposition for this system is to separate the FTDI data reading and writing responsibilities from the main oscilloscope application by placing them into a dedicated library named ioLibrary. Using AI tools, determine how to redesign the FTDI input/output code as this library. Then create a small sample project that uses the library to perform FTDI reads and writes.

Test the writing functionality by sending 0xFF and 0x00 at different frequency rates (1 Hz and 2 Hz) to demonstrate LED blinking. The FTDI handling logic must reside exclusively inside ioLibrary, while the main application is responsible for configuring the frequency.

Your design should include the following components:

a) ioRead

A class in the library responsible for reading N bytes from the FTDI device at a specified frequency and storing the data in a buffer. The class should allow configuration of the buffer, N, frequency, and number of bytes. The buffer must be created outside the class and passed in, so this relationship should be modeled as aggregation.

b) ioWrite

A class in the library responsible for writing M bytes from a provided buffer at a specified frequency. The class should allow configuration of the buffer, frequency, and buffer length. The buffer must also be created outside the class and passed in, so this relationship should be modeled as aggregation.

c) ioBuffer

A class in the library responsible for creating and managing a buffer of a specified size. An ioBuffer object should be usable by both ioRead and ioWrite.

d) Main application integration

In the main() function, link the ioLibrary library, either statically or dynamically. Create the necessary ioBuffer, ioRead, and ioWrite objects, connect them appropriately, configure the required parameters, and execute the program.

Add a chapter called Library Separation and use UML to describe your code.