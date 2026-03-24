CC = gcc
CFLAGS = -I. -IioLibrary -g
LDFLAGS = -L. -framework CoreFoundation -framework IOKit

IO_SRCS = ioLibrary/ioBuffer.c ioLibrary/ioRead.c ioLibrary/ioWrite.c ioLibrary/FtdiDevice.c
IO_OBJS = $(IO_SRCS:.c=.o)

# Build the sample application (default target)
blink_test: main.c libioLibrary.a
	$(CC) $(CFLAGS) main.c libioLibrary.a libftd2xx.a $(LDFLAGS) -o $@

# Build the static library
libioLibrary.a: $(IO_OBJS)
	ar rcs $@ $^

# Compile ioLibrary source files
ioLibrary/%.o: ioLibrary/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the old controller (preserved for reference)
controller: controller.c LED_Project.c morse_Project.c
	$(CC) $^ libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o $@ -g

clean:
	rm -f ioLibrary/*.o libioLibrary.a blink_test

.PHONY: clean
