CXX = g++
CXXFLAGS = -I. -IioLibrary -g -std=c++11 -pthread
LDFLAGS = -L. -framework CoreFoundation -framework IOKit -pthread

IO_SRCS = ioLibrary/ioBuffer.cpp ioLibrary/ioRead.cpp ioLibrary/ioWrite.cpp \
          ioLibrary/ioFtdiDevice.cpp ioLibrary/ioCircularBuffer.cpp \
          ioLibrary/ioThreadedReader.cpp ioLibrary/ioThreadedWriter.cpp
IO_OBJS = $(IO_SRCS:.cpp=.o)

# Build the pipeline executable (default target)
pipeline: pipeline.cpp libioLibrary.a
	$(CXX) $(CXXFLAGS) pipeline.cpp libioLibrary.a libftd2xx.a $(LDFLAGS) -o $@

# Build the blink test sample application
blink_test: main.cpp libioLibrary.a
	$(CXX) $(CXXFLAGS) main.cpp libioLibrary.a libftd2xx.a $(LDFLAGS) -o $@

# Build the static library
libioLibrary.a: $(IO_OBJS)
	ar rcs $@ $^

# Compile ioLibrary source files
ioLibrary/%.o: ioLibrary/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build the old controller (preserved for reference)
controller: controller.c LED_Project.c morse_Project.c
	gcc $^ libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o $@ -g

clean:
	rm -f ioLibrary/*.o libioLibrary.a blink_test pipeline

.PHONY: clean
