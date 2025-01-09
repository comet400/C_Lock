# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Os -march=native -mtune=native -flto -funroll-loops -I$(HDR_DIR)

# Target executable name
TARGET = clock

# Directories
SRC_DIR = src
HDR_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source and object file locations
SRCS = $(SRC_DIR)/ast.c $(SRC_DIR)/lexer.c $(SRC_DIR)/parser.c $(SRC_DIR)/Main.c  $(SRC_DIR)/runtimeEnv.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Header files
HEADERS = $(HDR_DIR)/ast.h $(HDR_DIR)/lexer.h $(HDR_DIR)/parser.h $(HDR_DIR)/runtimeEnv.h $(HDR_DIR)/runtimeValue.h

# Default rule to build the target
all: directories $(BIN_DIR)/$(TARGET)

# Rule to link object files and create the executable
$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Rule to compile each .c file into .o (with header dependencies)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories (Windows-compatible)
directories:
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	if not exist $(BIN_DIR) mkdir $(BIN_DIR)

# Clean up build files
clean:
	-rm -rf $(BUILD_DIR) $(BIN_DIR)

# Rebuild everything from scratch
rebuild: clean all
