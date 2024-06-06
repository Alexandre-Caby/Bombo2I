Exec_dir = ../app
CC = gcc
CFLAGS = -Wall -Wextra

all : build_map
	@echo "\033[32m\tAll sources built successfully!\033[0m"

build_map : map.c
	@echo "\033[32m\tBuilding map.c\033[0m"
	@$(CC) -o $(Exec_dir)/map $(CFLAGS) map.c -lSDL2 -lSDL2_ttf

clean :
	@rm -f $(Exec_dir)/* $(Exec_dir)/bombo2i