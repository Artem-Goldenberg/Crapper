# Compiler
CC = gcc
# Compiler flags
FLAGS = -g -Wall

# Directories
HDR = include
SRC = src
OBJ = obj
BIN = bin
DATA = samples

# Target name
TARGET = $(BIN)/hw_01

# All headers in $(HDR) directory
HDRS = $(wildcard $(HDR)/*.h)
# All source files inside $(SRC) directory
SRCS = $(wildcard $(SRC)/*.c)
# All object files from those source files
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

# Main target to run with `make`
all: $(TARGET)

# Create directory for .o files
$(OBJ):
	mkdir $@
# Create directory for an executable
$(BIN):
	mkdir $@
	
# Compile all source files with $(HDR) directory included in search path
$(OBJ)/%.o: $(SRC)/%.c $(HDR)/%.h $(OBJ)
	$(CC) $(FLAGS) -c $< -o $@ -I $(HDR)
	
$(OBJ)/%.o: $(SRC)/%.c $(OBJ)
	$(CC) $(FLAGS) -c $< -o $@ -I $(HDR)
	
# With all the object files in place, compile the final target
$(TARGET): $(OBJS) $(BIN)
	$(CC) $(FLAGS) $(OBJS) -o $@
	
# A simple test
test: $(TARGET)
	$< crop-rotate $(DATA)/small-one.bmp $(DATA)/generated/output.bmp 0 0 2 2

# Remove $(BIN) and $(OBJ) directories
clean:
	rm -fr $(BIN) $(OBJ) $(DATA)/generated

# Do not print commands by default
ifndef VERBOSE
.SILENT:
endif
