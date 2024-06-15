#include "../library/data.h"
#include "../library/session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// --- Constants ---
#define PORT_SERVER 8080
#define ADDRESS_SERVER "0.0.0.0"
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024
#define BOMB_COUNT 5

// --- Structures ---
// typedef struct {
//     int width;
//     int height;
//     int cells[MAX_MAP_SIZE];
// } Map;

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
    socket_t client_socket;
    Map *map;
    Player *player;
} client_data_t;

typedef struct {
    int bombCount;
    int deactivatedBombCount;
    time_t start_time;
    int gameEnded;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} game_state_t;

// --- Functions ---
void *handleClient(void *socket_desc);
Map* map_new(int width, int height);
void generateMap(Map *map);
void sendMap(socket_t client_sockets[], int num_clients, Map *map) ;
void setSpecialPoint(Map *map, int x, int y, int state);
void broadcastPoint(Point point); 
void broadcastMessage(const char *message);
void initPlayer(Player *player, Map *map, int *bomber_assigned, int *mine_clearer_assigned);
