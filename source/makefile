# Compiler for PC
Exec_dir = ../app
CC = gcc

# Cross compiler for Raspberry Pi
USER_dir = /home/alexandre/Bureau/IG2I/La1
CC_rpi = $(USER_dir)/ObjetCo/sources/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc
INCLUDES_SDL2 = ../../SDL2-2.30.3/target_SDL2/include
LIBS_SDL2 = ../../SDL2-2.30.3/target_SDL2/lib
INCLUDES_SDL2_TTF = ../../SDL2_ttf-2.22.0/target_SDL2_ttf/include
LIBS_SDL2_TTF = ../../SDL2_ttf-2.22.0/target_SDL2_ttf/lib
INCLUDES_SDL2_RPI = -I$(INCLUDES_SDL2) -I$(INCLUDES_SDL2_TTF)
LIBS_SDL2_RPI = -L$(LIBS_SDL2) -L$(LIBS_SDL2_TTF)

INCLUDE_WIRINGPI = -I../wiringPi/target-rpi/include
LIBS_WIRINGPI = -L../wiringPi/target-rpi/lib

OBJECT_SERVER = ../library/obj/data.o ../library/obj/session.o
OBJECT_CLIENT = ../library/obj/data.o ../library/obj/session.o

# Compiler flags
CFLAGS = -Wall -std=c99 
LDFLAGS = -lpthread


all : build_pc build_rpi build_server build_server_rpi
	@echo "\033[32m\tAll sources built successfully!\033[0m"

build_pc : map.c
	@echo "\033[32m\tBuilding map.c for PC\033[0m"
#	@$(CC) -o $(Exec_dir)/map_pc $(CFLAGS) map.c $(OBJECT_CLIENT) -lSDL2 -lSDL2_ttf

build_rpi : map.c
	@echo "\033[32m\tBuilding map.c for Raspberry Pi\033[0m"
#	@$(CC_rpi) -o $(Exec_dir)/map_rpi map.c $(CFLAGS) $(INCLUDES_SDL2_RPI) $(LIBS_SDL2_RPI) $(INCLUDE_WIRINGPI) $(LIBS_WIRINGPI) -lSDL2 -lSDL2_ttf -lwiringPi
	@gcc -o ../app/map_rpi map.c $(OBJECT_CLIENT) -Wall -std=c99 -I../../SDL2-2.30.3/target_SDL2/include -I../../SDL2_ttf-2.22.0/target_SDL2_ttf/include -L../../SDL2-2.30.3/target_SDL2/lib -L../../SDL2_ttf-2.22.0/target_SDL2_ttf/lib -L../../wiringPi/target-rpi/lib -lSDL2 -lSDL2_ttf -lwiringPi $(LDFLAGS)

build_server : communication_socket.c
	@echo "\033[32m\tBuilding communication_socket.c for PC\033[0m"
#	@$(CC) -o $(Exec_dir)/communication_socket $(CFLAGS) communication_socket.c $(OBJECT_SERVER) $(LDFLAGS)

build_server_rpi : communication_socket.c
	@echo "\033[32m\tBuilding communication_socket.c for Raspberry Pi\033[0m"
	@gcc -o $(Exec_dir)/communication_socket $(CFLAGS) communication_socket.c $(OBJECT_SERVER) $(LDFLAGS)

clean :
	@rm -f $(Exec_dir)/* $(Exec_dir)/bombo2i

.PHONY : all build_pc build_rpi build_server build_server_rpi clean