#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>   
#include <SDL2/SDL_ttf.h>
#include <time.h>

// --- Constants ---
#define MAX_MAP_WIDTH 40
#define MAX_MAP_HEIGHT 40
#define MAX_MAP_SIZE MAX_MAP_WIDTH * MAX_MAP_HEIGHT
#define CELL_SIZE 24

// --- Structures ---
typedef struct {
    int width;
    int height;
    int cells[MAX_MAP_SIZE];
} Map;

typedef enum {
    WALL,
    PATH,
    START,
    END
} Cell;

typedef struct {
    int x;
    int y;
} Point;

// --- Functions ---
Map* map_new(int width, int height);
void drawMap(SDL_Renderer *renderer, Map *map, TTF_Font *font);
void carvePathFrom(int x, int y, Map *map);
void generateMap(Map *map);
void setSpecialPoint(Map *map, int x, int y, int state);
int isPath(Map *map, int x, int y);
void placePoint(Map *map, int x, int y, int state);