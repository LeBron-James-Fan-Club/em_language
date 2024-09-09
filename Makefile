# Compiler and flags
CC = clang++
INCDIR = ./bin/include
CFLAGS = -MMD -MP -Wall -Wextra -g -fsanitize=address,undefined -DINCDIR=\"$(INCDIR)\"
LDFLAGS = -fsanitize=address,undefined

# Directories
SRC_DIR = src
OBJ_DIR = obj
TARGET = bin/a

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/**/*.cpp $(SRC_DIR)/*.cpp)
# Corresponding object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Default target
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@

# Rule to compile each source file to an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Include the dependency files
-include $(OBJ_FILES:.o=.d)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) bin/a

.PHONY: all clean