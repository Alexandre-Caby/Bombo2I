#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <pthread.h>
//#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "../library/data.h"
#include "../library/session.h"

// --- Constants ---
#define BUFFER_SIZE 1024

// #define GPIO_PIN_UP 37 // GPIO pin for the up button
// #define GPIO_PIN_DOWN 33 // GPIO pin for the down button
// #define GPIO_PIN_LEFT 22 // GPIO pin for the left button
// #define GPIO_PIN_RIGHT 35 // GPIO pin for the right button

// --- Structures ---
typedef enum {
    WALL,
    PATH,
    BOMB,
    DEACTIVATED_BOMB,
} Cell;

typedef enum {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    PLACE_BOMB,
    DEACTIVATE_BOMB
} Action;

typedef enum {
    BOMBER,
    MINE_CLEARER
} Role;

typedef struct {
    int x;
    int y;
    Role role;
} Player;

typedef struct {
    int sock_fd;
    Map *map;
} recv_thread_data_t;

// --- Functions ---
Map* map_new(int width, int height);
void drawMap(SDL_Renderer *renderer, Map *map, TTF_Font *font);
void setSpecialPoint(Map *map, int x, int y, int state);
int isAccessible(Map *map, int x, int y);
void placePoint(Map *map, SDL_Renderer *renderer, TTF_Font *font, int x, int y, int action, int sock);
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, SDL_Color bgColor);
void showMessage(SDL_Renderer *renderer, TTF_Font *font, const char *message);
void movePlayer(Player *player, Map *map, int dx, int dy);
void handleInput(Player *player, Map *map, int action);
void movePlayer(Player *player, Map *map, int dx, int dy);
void renderPlayer(SDL_Renderer *renderer, Player *player);
// void gpioInitialise();
void chrono(int fd);
void display7segments(int fd, int sec, int min);
void *receiveUpdates(void *arg);