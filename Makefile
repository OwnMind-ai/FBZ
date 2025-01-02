# Compiler
CC = clang

# Compiler flags
CFLAGS = -lm -Wall -Wextra -Wpedantic -Wc23-extensions 

# Name of the output executable
TARGET = fbz

# Source files
SRCS = $(wildcard *.c) $(wildcard parser/*.c) $(wildcard parser/*.h) $(wildcard runtime/*.c) $(wildcard runtime/*.h) 


# Object files
OBJS = $(SRCS:.c=.o)

# Default target
# all: $(TARGET)
all: $(TARGET)

# Link the target executable
$(TARGET): 
	$(CC) $(CFLAGS) $(SRCS)

# Phony targets
.PHONY: all clean
