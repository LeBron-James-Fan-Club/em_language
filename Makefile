# Compiler and flags
CC = clang
CFLAGS = -MMD -MP -Wall -Wextra -g -I./include -fsanitize=address,undefined
LDFLAGS = -fsanitize=address,undefined

# Directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
TARGET = $(BUILD_DIR)/em

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
# Corresponding object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Default target
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@

# Rule to compile each source file to an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Include the dependency files
-include $(OBJ_FILES:.o=.d)

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean