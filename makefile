# Define compiler and flags
CC = gcc
CFLAGS = -Wall -g -I./include #-fsanitize=address,undefined

# Define target and object files
TARGET = bin/a
SRCDIR = src
OBJDIR = obj
INCDIR = include

OBJS = $(OBJDIR)/main.o $(OBJDIR)/scan.o $(OBJDIR)/ast.o $(OBJDIR)/gen.o $(OBJDIR)/stmt.o $(OBJDIR)/sym.o

# Default target
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	mkdir -p $(dir $(TARGET))
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile main.c
$(OBJDIR)/main.o: $(SRCDIR)/main.c $(INCDIR)/tokens.h $(INCDIR)/ast.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $@

# Rule to compile scan.c
$(OBJDIR)/scan.o: $(SRCDIR)/scan.c $(INCDIR)/tokens.h $(INCDIR)/ast.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/scan.c -o $@

# Rule to compile ast.c
$(OBJDIR)/ast.o: $(SRCDIR)/ast.c $(INCDIR)/tokens.h $(INCDIR)/ast.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/ast.c -o $@

# Rule to compile gen.c
$(OBJDIR)/gen.o: $(SRCDIR)/gen.c $(INCDIR)/tokens.h $(INCDIR)/ast.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/gen.c -o $@

# Rule to compile stmt.c
$(OBJDIR)/stmt.o: $(SRCDIR)/stmt.c $(INCDIR)/tokens.h $(INCDIR)/ast.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/stmt.c -o $@

# Rule to compile sym.c
$(OBJDIR)/sym.o: $(SRCDIR)/sym.c $(INCDIR)/tokens.h $(INCDIR)/ast.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/sym.c -o $@

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets (not actual files)
.PHONY: all clean
