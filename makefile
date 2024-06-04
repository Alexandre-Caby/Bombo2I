all: build_src build_lib build_maze
	@echo "\033[32m\nBombo2I built successfully!\033[0m"

MAKEFLAGS += --no-print-directory

# Build source folder
build_src:
	@echo "Building sources..."
	@cd source && make --always-make

# Build library folder
build_lib:
	@echo "Building libraries..."
	@cd library && make --always-make

# Build maze folder
build_maze:
	@echo "Building maze..."
	@cd maze && make --always-make

clean:
	@echo "Cleaning sources..."
	@cd source && make clean
	@echo "Cleaning libraries..."
	@cd library && make clean
	@echo "Cleaning maze..."
	@cd maze && make clean
	@echo "\033[32m\tCleaning clear!\033[0m"