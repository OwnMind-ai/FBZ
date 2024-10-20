# Compiler
CC = clang

# Compiler flags
CFLAGS = -lm

# Name of the output executable
TARGET = fbz

# Source files
SRCS = $(wildcard *.c) $(wildcard parser/*.c) $(wildcard parser/*.h) 

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
# all: $(TARGET)
all: $(TARGET)

# Link the target executable
$(TARGET): 
	$(CC) $(CFLAGS) $(SRCS)

clean:
	rm -f build/$(OBJS) build/$(TARGET)

# Phony targets
.PHONY: all clean
