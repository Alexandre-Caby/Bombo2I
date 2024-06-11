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
    socket_t client_socket;
    Map *map;
    Player *player;
} client_data_t;



// --- Functions ---
void *handleClient(void *socket_desc);
Map* map_new(int width, int height);
void generateMap(Map *map);
void sendMap(socket_t client_sockets[], int num_clients, Map *map) ;
void setSpecialPoint(Map *map, int x, int y, int state);
void broadcastPoint(socket_t client_sockets, Point point); 
void initPlayer(Player *player, Map *map, int player_id);
void movePlayer(Player *player, Map *map, int dx, int dy) ;