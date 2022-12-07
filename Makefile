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
TMP = $(DATA)/generated

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
# Create directory for generated bmp files
$(TMP):
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
transform-small: $(TARGET) $(TMP) $(DATA)/small-one.bmp
	$< crop-rotate $(DATA)/small-one.bmp $(TMP)/tranform-small-output.bmp 0 0 2 2
# More comprehensive test
transform-big: $(TARGET) $(TMP) $(DATA)/lena_512.bmp
	$< crop-rotate $(DATA)/lena_512.bmp $(TMP)/tranform-big-output.bmp 35 95 371 351
	
# Helper targets
$(TMP)/insert-small-output.bmp: $(TARGET) $(TMP) $(DATA)/small-one.bmp $(DATA)/key-small.txt $(DATA)/message-small.txt
	$< insert $(DATA)/small-one.bmp $(TMP)/insert-small-output.bmp $(DATA)/key-small.txt $(DATA)/message-small.txt
	
$(TMP)/insert-big-output.bmp: $(TARGET) $(TMP) $(DATA)/lena_512.bmp $(DATA)/key-big.txt $(DATA)/message-big.txt
	$< insert $(DATA)/lena_512.bmp $(TMP)/insert-big-output.bmp $(DATA)/key-big.txt $(DATA)/message-big.txt
	
# Encoding small test
insert-small: $(TMP)/insert-small-output.bmp
# Encoding big test
insert-big: $(TMP)/insert-big-output.bmp
	
# Decoding small test
extract-small: $(TARGET) $(TMP)/insert-small-output.bmp
	$< extract $(TMP)/insert-small-output.bmp $(DATA)/key-small.txt $(TMP)/extract-small-output.txt
# Decoding big test
extract-big: $(TARGET) $(TMP)/insert-big-output.bmp
	$< extract $(TMP)/insert-big-output.bmp $(DATA)/key-big.txt $(TMP)/extract-big-output.txt

# Remove $(BIN), $(OBJ) and $(TMP) directories
clean:
	rm -fr $(BIN) $(OBJ) $(TMP)

# Do not print commands by default
ifndef VERBOSE
.SILENT:
endif
