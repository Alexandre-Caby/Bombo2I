#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <wiringPi.h>

// --- Constants ---
#define MAX_MAP_WIDTH 45
#define MAX_MAP_HEIGHT 20
#define MAX_MAP_SIZE MAX_MAP_WIDTH * MAX_MAP_HEIGHT
#define CELL_SIZE 24
#define GPIO_PIN_UP 37 // GPIO pin for the up button
#define GPIO_PIN_DOWN 33 // GPIO pin for the down button
#define GPIO_PIN_LEFT 22 // GPIO pin for the left button
#define GPIO_PIN_RIGHT 35 // GPIO pin for the right button

// --- Structures ---
typedef struct {
    int width;
    int height;
    int cells[MAX_MAP_SIZE];
} Map;

typedef enum {
    WALL,
    PATH,
    BOMB,
    DEACTIVATED_BOMB,
} Cell;

typedef enum {
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    ORANGE,
    PURPLE
} CellColor;

typedef enum {
    BOMBER,
    MINE_CLEARER
} Role;

typedef struct {
    int x;
    int y;
    Role role;
} Player;

// --- Functions ---
Map* map_new(int width, int height);
void drawMap(SDL_Renderer *renderer, Map *map, TTF_Font *font);
void carvePathFrom(int x, int y, Map *map);
void generateMap(Map *map);
void setSpecialPoint(Map *map, int x, int y, int state);
int isAccessible(Map *map, int x, int y);
void placePoint(Map *map, SDL_Renderer *renderer, TTF_Font *font, int x, int y, int state);
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, SDL_Color bgColor);
void showMessage(SDL_Renderer *renderer, TTF_Font *font, const char *message);
void initPlayer(Player *player, Map *map);
void movePlayer(Player *player, Map *map, int dx, int dy);
void handleInput(Player *player, Map *map, SDL_Event event);
void renderPlayer(SDL_Renderer *renderer, Player *player);
void gpioInitialise();