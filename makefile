# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Os -march=native -mtune=native -flto -funroll-loops -IHeaders

# Target executable name
TARGET = clock

# Source and object file locations
SRC_DIR = Source
HDR_DIR = Headers
SRCS = $(SRC_DIR)/ast.c $(SRC_DIR)/lexer.c $(SRC_DIR)/parser.c $(SRC_DIR)/Main.c
OBJS = $(SRCS:.c=.o)

# Header files
HEADERS = $(HDR_DIR)/ast.h $(HDR_DIR)/lexer.h $(HDR_DIR)/parser.h

# Default rule to build the target
all: $(TARGET)

# Rule to link object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Rule to compile each .c file into .o (with header dependencies)
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Rebuild everything from scratch
rebuild: clean all
