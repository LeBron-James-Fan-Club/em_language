# Compiler and flags
CC = clang
CFLAGS = -MMD -MP -Wall -Wextra -g -I./include -fsanitize=address,undefined
LDFLAGS = -fsanitize=address,undefined

# Directories
SRC_DIR = src
OBJ_DIR = obj
TARGET = bin/a

# Source files
SRC_FILES :=
SRC_FILES += $(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
SRC_FILES += $(filter-out $(SRC_FILES),$(SRC_DIR)/new-em.tab.c)
SRC_FILES += $(filter-out $(SRC_FILES),$(SRC_DIR)/new-em.lex.c)

$(info $(SRC_FILES))

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

$(SRC_DIR)/new-em.lex.c: src/new-em.l
	flex -o $@ --header-file=$(patsubst %.c,%.h,$@) --debug $<

$(SRC_DIR)/new-em.tab.c: src/new-em.y
	bison -o $@ --defines=$(patsubst %.c,%.h,$@) --debug $<

# Include the dependency files
-include $(OBJ_FILES:.o=.d)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR)/ $(SRC_DIR)/new-em.lex.c $(SRC_DIR)/new-em.lex.h $(SRC_DIR)/new-em.tab.c $(SRC_DIR)/new-em.tab.h

.PHONY: all clean
