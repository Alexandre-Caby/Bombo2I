# Compiler for PC
Exec_dir = ../app
CC = gcc

# Cross compiler for Raspberry Pi
CC_rpi = $(source ../../../ressources/config.source) $(CC)
INCLUDES = ../../SDL2-2.0.3/include
LIBS_SDL2 = ../../SDL2-2.30.3/build
LIBS_SDL2_TTF = ../../SDL2_ttf-2.0.15/build

# Compiler flags
CFLAGS = -Wall -Wextra
LDFLAGS = -L../library/libmcs.a -lmcs -lpthread

all : build_pc build_rpi build_server build_server_rpi
	@echo "\033[32m\tAll sources built successfully!\033[0m"

build_pc : map.c
	@echo "\033[32m\tBuilding map.c for PC\033[0m"
	@$(CC) -o $(Exec_dir)/map_pc $(CFLAGS) map.c -lSDL2 -lSDL2_ttf

build_rpi : map.c
	@echo "\033[32m\tBuilding map.c for Raspberry Pi\033[0m"
# 	@$(CC_rpi) -o $(Exec_dir)/map_rpi $(CFLAGS) -I$(INCLUDES) -L$(LIBS_SDL2) -L$(LIBS_SDL2_TTF) map.c 

build_server : communication_socket.c
	@echo "\033[32m\tBuilding communication_socket.c for PC\033[0m"
	@$(CC) -o $(Exec_dir)/communication_socket $(CFLAGS) communication_socket.c $(LDFLAGS)

build_server_rpi : communication_socket.c
	@echo "\033[32m\tBuilding communication_socket.c for Raspberry Pi\033[0m"
#	@$(CC_rpi) -o $(Exec_dir)/communication_socket $(CFLAGS) communication_socket.c

clean :
	@rm -f $(Exec_dir)/* $(Exec_dir)/bombo2i

.PHONY : all build_pc build_rpi build_server build_server_rpi clean