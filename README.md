# Bombo2I
Welcome to Bombo2I ! This guide will help you set up and run the Bombo2I project on your local machine.

## Prerequisites
- [SDL2](https://www.libsdl.org/download-2.0.php)
- [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)
- [WiringPi](http://wiringpi.com/) (only for Raspberry Pi)
- A Raspberry Pi or Joy-PI
- A keyboard (optional)

## Installation
1. Unzip the project
```sh
unzip Bombo2I.zip
```

2. Install the dependencies
```sh
sudo apt-get install libsdl2-dev libsdl2-ttf-dev
```
```sh
sudo apt-get install wiringpi
```

Note: To use the project on a Raspberry Pi, you need to install the dependencies on the Raspberry Pi itself. Because no static libraries are provided, you will need to build the project on the Raspberry Pi.

3. Build the project
```sh
make
```

## Usage
1. Run the game, launch the server first and then the clients on each Raspberry (2 players required)
```sh
./app/communication_socket
```
```sh
./map_rpi
```

2. Use the arrow keys to move the player or the GPIOs on the Raspberry Pi

## Authors
- [Bombo2I](Alexandre Caby)
- [Bombo2I](Jérôme Devienne)