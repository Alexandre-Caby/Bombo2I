# Define the compiler and the default flags
CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ_DIR = obj

# 'all' target should build all libraries
all: data_lib session_lib ar_lib
	@echo "\033[32m\tAll libraries built successfully!\033[0m"

# Create object directory before compiling anything
$(shell mkdir -p $(OBJ_DIR))

# Compile the data object file
data_lib: $(OBJ_DIR)/data.o

$(OBJ_DIR)/data.o: data.c
	@echo "\033[33m\tCompiling the library...\033[0m"
	@$(CC) $(CFLAGS) -c data.c -o $(OBJ_DIR)/data.o

# Compile the session object file
session_lib: $(OBJ_DIR)/session.o

$(OBJ_DIR)/session.o: session.c
	@$(CC) $(CFLAGS) -c session.c -o $(OBJ_DIR)/session.o

# Create the static library
ar_lib: $(OBJ_DIR)/session.o $(OBJ_DIR)/data.o
	@echo "\033[33m\tCreating the static library...\033[0m"
	@ar rcs libmcs.a $(OBJ_DIR)/session.o $(OBJ_DIR)/data.o

# Clean the object files and the library
clean_lib:
	@rm -f $(OBJ_DIR)/*.o libmcs.a

# Additional clean rule
clean:
	@rm -f *.e
	@rm -f $(OBJ_DIR)/*.o libmcs.a

