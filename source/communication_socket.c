#include "communication_socket.h"

// Global variables for managing clients
socket_t client_sockets[MAX_CLIENTS];
pthread_mutex_t client_sockets_mutex = PTHREAD_MUTEX_INITIALIZER;
int roles_assigned[MAX_CLIENTS] = {0, 0}; // 0 = not assigned, 1 = assigned

/**
 * function handleClient
 * @brief Handle the client requests
 * 
 * @param socket_desc 
 * @return void* 
 */
void *handleClient(void *data) {
    // Get the client socket from the data
    client_data_t *client_data = (client_data_t *)data;
    if (client_data == NULL) {
        fprintf(stderr, "Error: client_data is NULL\n");
        pthread_exit(NULL);
    }

    socket_t client_socket = client_data->client_socket;
    Map *map = client_data->map;
    Player *player = client_data->player;

    if (player == NULL) {
        fprintf(stderr, "Error: player is NULL\n");
        pthread_exit(NULL);
    }
    if (map == NULL) {
        fprintf(stderr, "Error: map is NULL\n");
        pthread_exit(NULL);
    }

    printf("Debug: Client socket: %d\n", client_socket.fd);
    printf("Debug: Player initial position: (%d, %d)\n", player->x, player->y);

    // Send the player's initial position to the client
    Point point = {player->x, player->y};
    envoyer(&client_socket, &point, NULL);

    // Send the player's initial position to all other clients
    broadcastPoint(client_socket, point);

    // Handle the client's requests
    while (1) {
        // Receive the client's request
        int request;
        recevoir(&client_socket, &request, NULL);

        printf("Debug: Received request: %d\n", request);

        // Handle the request
        switch (request) {
            case MOVE_UP:
                movePlayer(player, map, 0, -1);
                break;
            case MOVE_DOWN:
                movePlayer(player, map, 0, 1);
                break;
            case MOVE_LEFT:
                movePlayer(player, map, -1, 0);
                break;
            case MOVE_RIGHT:
                movePlayer(player, map, 1, 0);
                break;
            case PLACE_BOMB:
                setSpecialPoint(map, player->x, player->y, BOMB);
                break;
            case DEACTIVATE_BOMB:
                setSpecialPoint(map, player->x, player->y, DEACTIVATED_BOMB);
                break;
            default:
                fprintf(stderr, "Unknown request: %d\n", request);
                break;
        }

        // Broadcast the player's new position to all other clients
        Point point = {player->x, player->y};
        broadcastPoint(client_socket, point);
    }

    // Close the client socket
    close(client_socket.fd);
    free(client_data);
    pthread_exit(NULL);

    return NULL;
}

/**
 * function sendMap
 * @brief Send the map to all clients
 * 
 * @param client_socket 
 * @param num_clients
 * @param map 
 * @return void
 */
void sendMap(socket_t client_sockets[], int num_clients, Map *map) {
    for (int i = 0; i < num_clients; i++) {
        printf("Sending map to client %d\n", client_sockets[i].fd);
        if (client_sockets[i].fd != 0) {
            // Send the map dimensions
            // Send the map dimensions and cells in one request
            int map_data[MAX_MAP_WIDTH * MAX_MAP_HEIGHT + 2];
            map_data[0] = map->width;
            map_data[1] = map->height;
            int index = 2;
            for (int y = 0; y < map->height; y++) {
                for (int x = 0; x < map->width; x++) {
                    map_data[index++] = map->cells[y * map->width + x];
                }
            }
            envoyer(&client_sockets[i], map_data, serial_long_int);
        }
    }
}

/**
 * function main
 * @brief Main function to start the server
 * 
 * @return int
 */
int main() {
    srand(time(NULL));  // Seed the random number generator

    socket_t server_socket = creerSocketEcoute(ADDRESS_SERVER, PORT_SERVER);
    if (server_socket.fd < 0) {
        perror("Failed to create server socket");
        return 1;
    }

    // Generate the map (shared between all clients)
    Map *map = map_new(MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
    generateMap(map);

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        client_sockets[i].fd = 0;
    }

    int connected_clients = 0;

    while (1) {
        socket_t client_socket = accepterClt(server_socket);
        if (client_socket.fd < 0) {
            perror("Failed to accept client connection");
            continue;
        }

        // Find an empty slot in the client_sockets array and assign the client socket
        int empty_slot = -1;
        pthread_mutex_lock(&client_sockets_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i].fd == 0) {
                empty_slot = i;
                break;
            }
        }
        if (empty_slot != -1) {
            client_sockets[empty_slot] = client_socket;
        }
        pthread_mutex_unlock(&client_sockets_mutex);

        connected_clients++;

        printf("Player connected (id=%d)\n", client_socket.fd);

        // Send a welcome message to the client
        envoyer(&client_socket, "\t💣Welcome to Bombo2I!💣\n", NULL);

        // waiting for 2 clients to connect
        if(connected_clients < 2) {
            printf("Waiting for %d more players to connect...\n", 2 - connected_clients);
            sleep(1);
            continue;
        }

        printf ("\tAll players connected ! Game starting...\n");

        // Send the map to all clients when all clients are connected and initialize the players
        if (connected_clients == MAX_CLIENTS) {
            sendMap(client_sockets, MAX_CLIENTS, map);

            // Create a thread for each client
            pthread_t threads[MAX_CLIENTS];
            for (int i = 0; i < MAX_CLIENTS; i++) {
                client_data_t *client_data = malloc(sizeof(client_data_t));
                client_data->client_socket = client_sockets[i];
                client_data->map = map;
                client_data->player = malloc(sizeof(Player));
                initPlayer(client_data->player, map, i);
                pthread_create(&threads[i], NULL, handleClient, client_data);
            }

            // Wait for all threads to finish
            for (int i = 0; i < MAX_CLIENTS; i++) {
                pthread_join(threads[i], NULL);
            }

            // Reset the roles_assigned array
            for (int i = 0; i < MAX_CLIENTS; i++) {
                roles_assigned[i] = 0;
            }

            // Reset the connected_clients counter
            connected_clients = 0;
        }   
    }

    return 0;
}

/**
 * function map_new
 * @brief Create a new map
 * 
 * @param width 
 * @param height 
 * @return Map*
 */
Map* map_new(int width, int height) {
    Map *map = malloc(sizeof(Map));
    if (map == NULL) {
        fprintf(stderr, "Could not allocate memory for map\n");
        exit(1);
    }
    map->width = width;
    map->height = height;
    for (int i = 0; i < width * height; i++) {
        map->cells[i] = WALL;
    }
    return map;
}

/**
 * function generateMap
 * @brief Generate a map with multiple paths (and on some paths no walls)
 * 
 * @param map 
 * @return void
 */
void generateMap(Map *map) {
    // Initialize the map with walls
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            map->cells[y * map->width + x] = WALL;
        }
    }

    // Create multiple paths by carving out a grid-like pattern
    for (int y = 1; y < map->height; y += 2) {
        for (int x = 1; x < map->width; x += 2) {
            map->cells[y * map->width + x] = PATH;
            if (x + 1 < map->width) {
                map->cells[y * map->width + x + 1] = PATH; // carve right
            }
            if (y + 1 < map->height) {
                map->cells[(y + 1) * map->width + x] = PATH; // carve down
            }
        }
    }

    // Create open areas without walls
    for (int y = 3; y < map->height; y += 4) {
        for (int x = 3; x < map->width; x += 4) {
            map->cells[y * map->width + x] = PATH;
            if (x + 1 < map->width) {
                map->cells[y * map->width + x + 1] = PATH; // open right
            }
            if (y + 1 < map->height) {
                map->cells[(y + 1) * map->width + x] = PATH; // open down
            }
            if (x - 1 > 0) {
                map->cells[y * map->width + x - 1] = PATH; // open left
            }
            if (y - 1 > 0) {
                map->cells[(y - 1) * map->width + x] = PATH; // open up
            }
        }
    }

    // Add some random obstacles 
    for (int y = 1; y < map->height; y++) {
        for (int x = 1; x < map->width; x++) {
            if (map->cells[y * map->width + x] == PATH && rand() % 100 < 4) {
                map->cells[y * map->width + x] = WALL;
            }
        }
    }
}

/**
 * function setSpecialPoint
 * @brief Set a special point on the map
 * 
 * @param map 
 * @param x 
 * @param y 
 * @param state 
 * @return void
 */
void setSpecialPoint(Map *map, int x, int y, int state) {
    if (x >= 0 && x < map->width && y >= 0 && y < map->height) {
        map->cells[y * map->width + x] = state;
    }
}

/**
 * function broadcastPoint
 * @brief Broadcast a point to all connected clients except one
 * 
 * @param exclude_socket 
 * @param point 
 * @return void
 */
void broadcastPoint(socket_t exclude_socket, Point point) {
    pthread_mutex_lock(&client_sockets_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i].fd != 0 && client_sockets[i].fd != exclude_socket.fd) {
            envoyer(&client_sockets[i], &point, NULL);
        }
    }
    pthread_mutex_unlock(&client_sockets_mutex);
}

// --- Player functions ---

/**
 * function initPlayer
 * @brief Initialize the player position and role
 * 
 * @param player 
 * @param map 
 * @param player_id
 * @return void
 */
void initPlayer(Player *player, Map *map, int player_id) {
    // Assign role randomly but ensure each role is assigned only once
    if (roles_assigned[BOMBER] == 0 && roles_assigned[MINE_CLEARER] == 0) {
        player->role = rand() % 2 == 0 ? BOMBER : MINE_CLEARER;
    } else if (roles_assigned[BOMBER] == 0) {
        player->role = BOMBER;
    } else {
        player->role = MINE_CLEARER;
    }
    roles_assigned[player->role] = 1;

    // Define the player's position based on their role
    if(player->role == BOMBER) {
        player->x = 1;
        player->y = 1;
    } else {
        player->x = map->width - 2;
        player->y = map->height - 2;
    }
    
    while (map->cells[player->y * map->width + player->x] == WALL) {
        player->x++;
    }
}

/**
 * function movePlayer
 * @brief Move the player on the map with the arrow keys or gpio buttons
 * 
 * @param map
 * @param player
 * @param dx
 * @param dy
 * @return void
 */
void movePlayer(Player *player, Map *map, int dx, int dy) {
    int newX = player->x + dx;
    int newY = player->y + dy;

    // Check if the new position is within the map boundaries and is not a wall
    if (newX >= 0 && newX < map->width && newY >= 0 && newY < map->height && map->cells[newY * map->width + newX] != WALL) {
        player->x = newX;
        player->y = newY;
    }
}