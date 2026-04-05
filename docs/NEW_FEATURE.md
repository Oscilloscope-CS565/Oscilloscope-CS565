# Assignment: Multithreaded Data Acquisition Pipeline
## Overview
In real-time data acquisition systems, data must be transferred continuously and efficiently between components. A common design for this type of system uses a dedicated read thread, a dedicated write thread, and a circular buffer to safely pass data between them.

We will discuss multithreaded pipeline design and circular buffer implementation in more detail next week. However, this assignment is being given now so that you may begin planning and working on it in advance.

## Objective
Extend your existing library to support a basic multithreaded data acquisition and processing pipeline.

## Requirements
Modify your library so that it includes the following components:

a multithreaded read mechanism

a circular buffer

a multithreaded write mechanism

Then, create a command-line executable that uses these components together to perform the following tasks:

read data from an FTDI device

store and transfer the data through a circular buffer

write the data either to:

a file, or

another FTDI device

In addition, the new design must be documented using UML. This documentation must be added as a new chapter in your architecture document titled: "Multithreaded Data Acquisition Pipeline"

 

## Expected Functionality
Your solution should demonstrate a working pipeline in which:

A read thread acquires data from an FTDI device.

The acquired data is placed into a circular buffer.

A write thread removes data from the circular buffer.

The write thread outputs the data to either a file or another FTDI device.

 

## Documentation Requirement
Your architecture document must include a new chapter titled: "Multithreaded Data Acquisition Pipeline"

This chapter should describe the new design and include appropriate UML diagrams. At minimum, the documentation should clearly show:

the major software components involved

the relationship between the read thread, circular buffer, and write thread

the flow of data through the system

the responsibilities of the main classes

You may use UML class diagrams, sequence diagrams, activity diagrams, or other appropriate UML representations.

 

## Deliverables
Submit the following:

updated library source code

a command-line executable implementing the pipeline

updated architecture document including the chapter titled Multithreaded Data Acquisition Pipeline

UML diagrams describing the design

a video showing you running the code from the command line

 

## Notes
Additional design guidance will be provided next week during class.

At this stage, focus on creating a clear, correct, and well-structured implementation.

You are encouraged to begin discussing and planning your approach with your group before the full in-class discussion.

 

## Grading Rubric (100 points)
 

### 1. Multithreaded Read Implementation — 20 points
20–17: Read thread is correctly implemented, well integrated, and functions reliably.

16–13: Read thread is mostly correct, with minor issues in structure or behavior.

12–9: Read thread is partially implemented or inconsistently functional.

8–0: Read thread is missing or not functional.

 

### 2. Circular Buffer Design and Integration — 20 points
20–17: Circular buffer is correctly implemented and clearly supports thread-safe data transfer.

16–13: Circular buffer works with minor design or correctness issues.

12–9: Circular buffer is incomplete or only partially functional.

8–0: Circular buffer is missing or not functional.

 

### 3. Multithreaded Write Implementation — 20 points
20–17: Write thread is correctly implemented and reliably writes to file or another FTDI device.

16–13: Write thread is mostly correct, with minor issues.

12–9: Write thread is partially implemented or inconsistently functional.

8–0: Write thread is missing or not functional.

 

### 4. Command-Line Executable and End-to-End Functionality — 20 points
20–17: Executable builds and runs successfully, demonstrating a complete end-to-end pipeline.

16–13: Executable works with minor issues in usage, integration, or reliability.

12–9: Executable is incomplete or demonstrates only part of the required workflow.

8–0: Executable is missing, does not build, or does not demonstrate the required pipeline.

 

### 5. Architecture Documentation and UML — 20 points
20–17: Architecture document includes a clear chapter titled Multithreaded Data Acquisition Pipeline with appropriate UML diagrams and strong explanation of the design.

16–13: Documentation is present and useful, but missing some clarity, detail, or UML completeness.

12–9: Documentation is limited, incomplete, or only partially explains the design.

8–0: Required chapter is missing, UML is missing, or documentation does not meaningfully describe the new design.

