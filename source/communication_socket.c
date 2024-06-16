#include "communication_socket.h"

// --- Global variables for managing the clients ---
socket_t client_sockets[MAX_CLIENTS];
pthread_mutex_t client_sockets_mutex = PTHREAD_MUTEX_INITIALIZER;
int roles_assigned[MAX_CLIENTS] = {0, 0}; // 0 = not assigned, 1 = assigned
// --- Global variables for managing the game ---
game_state_t game_state = {
    .bombCount = 0,
    .deactivatedBombCount = 0,
    .start_time = 0,
    .gameEnded = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER
};

/**
 * function handle_sigint
 * @brief Handle the SIGINT signal (Ctrl+C) to shut down the server
 * 
 * @param sig 
 * @return void
 */
void handle_sigint(int sig) {
    printf("\nServer shutting down...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i].fd != 0) {
            close(client_sockets[i].fd);
        }
    }
    exit(0);
}

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

    // Send the player data to the client
    if (send(client_socket.fd, player, sizeof(Player), 0) < 0) {
        perror("Failed to send player data");
        close(client_socket.fd);
        free(client_data);
        pthread_exit(NULL);
    }

    // Handle the client's requests
    while (1) {
        // Receive the client's request
        Point point;
        ssize_t recv_size = recv(client_socket.fd, &point, sizeof(Point), 0);

        if (recv_size <= 0) {
            if (recv_size == 0) {
                printf("Client %d disconnected.\n", client_socket.fd);
                break;
            } else {
                perror("recv");
                break;
            }
        }

        pthread_mutex_lock(&game_state.mutex);
        // Handle the request
        switch (point.state) {
            case 2:
                if (game_state.bombCount < BOMB_COUNT) {
                    setSpecialPoint(map, player->x, player->y, BOMB);
                    point.state = BOMB;
                    game_state.bombCount++;

                    char message[BUFFER_SIZE] = "A bomb has been placed by the Bomber!\n";
                    broadcastMessage(message);

                    if (game_state.bombCount == BOMB_COUNT) {
                        game_state.start_time = time(NULL);
                        char message[BUFFER_SIZE] = "All bombs are placed. The time starts now! 1 minute left!\n";
                        broadcastMessage(message);
                    }
                    // Broadcast the point to all clients
                    broadcastPoint(point);
                } else {
                    char message[BUFFER_SIZE] = "Bomb limit reached\n";
                    send(client_socket.fd, message, strlen(message), 0);
                }
                printf("Debug - Bomb count: %d\n", game_state.bombCount);
                break;
            case 3:
                setSpecialPoint(map, player->x, player->y, DEACTIVATED_BOMB);
                point.state = DEACTIVATED_BOMB;
                game_state.deactivatedBombCount++;
                // Broadcast the point to all clients
                broadcastPoint(point);
                char message[BUFFER_SIZE] = "A bomb has been deactivated by the Mine clearer!\n";
                broadcastMessage(message);
                break;
            default:
                fprintf(stderr, "Unknown request: %d\n", point.state);
                break;
        }

        if (!game_state.gameEnded) {
            if (game_state.deactivatedBombCount == 5 && game_state.start_time != 0 && time(NULL) - game_state.start_time < 60) {
                game_state.gameEnded = 1;
                char message[BUFFER_SIZE] = "Game ended: Victory for the Mine clearer!\n";

                broadcastMessage(message);
                pthread_cond_broadcast(&game_state.cond);
            } else if (game_state.bombCount > 0 && game_state.start_time != 0 && time(NULL) - game_state.start_time >= 60) {
                game_state.gameEnded = 1;
                char message[BUFFER_SIZE] = "Game ended: Victory for the Bomber!\n";
                
                broadcastMessage(message);
                pthread_cond_broadcast(&game_state.cond);
            }
        }

        pthread_mutex_unlock(&game_state.mutex);

        if (game_state.gameEnded) {
            break;
        }
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
    int map_size[2] = {map->width, map->height};

    for (int i = 0; i < num_clients; i++) {
        printf("Sending map to client %d\n", client_sockets[i].fd);
        if (client_sockets[i].fd != 0) {
            // Send the map dimensions
            send(client_sockets[i].fd, map_size, 2 * sizeof(int), 0);
            
            // Send the map cells
            send(client_sockets[i].fd, map->cells, map->width * map->height * sizeof(int), 0);
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
    // Handle ctrl+c
    signal(SIGINT, handle_sigint);

    srand(time(NULL));  // Seed the random number generator

    socket_t server_socket = creerSocketEcoute(ADDRESS_SERVER, PORT_SERVER);
    if (server_socket.fd < 0) {
        perror("Failed to create server socket");
        return 1;
    }

    // Generate the map (shared between all clients)
    Map *map = map_new(MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
    generateMap(map);

    // Message to indicate the server is running and listening for clients
    printf("Server running on %s:%d and listening for clients...\n", ADDRESS_SERVER, PORT_SERVER);

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
            connected_clients++;
        }
        pthread_mutex_unlock(&client_sockets_mutex);

        printf("Player connected (id=%d)\n", client_socket.fd);

        // Send a welcome message to the client
        envoyer(&client_socket, "\t💣Welcome to Bombo2I!💣\n", NULL);

        // Waiting for 2 clients to connect
        if (connected_clients < 2) {
            printf("Waiting for %d more players to connect...\n", 2 - connected_clients);
            continue;
        }

        printf("\tAll players connected! Game starting...\n");

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
                initPlayer(client_data->player, map, &roles_assigned[BOMBER], &roles_assigned[MINE_CLEARER]);
                pthread_create(&threads[i], NULL, handleClient, client_data);
            }

            // Wait for all threads to finish
            for (int i = 0; i < MAX_CLIENTS; i++) {
                pthread_join(threads[i], NULL);
                pthread_mutex_lock(&client_sockets_mutex);
                client_sockets[i].fd = 0;
                pthread_mutex_unlock(&client_sockets_mutex);
                // Reset the roles_assigned counter
                roles_assigned[i] = 0;
            }

            // Reset the connected_clients counter and the game state
            connected_clients = 0;
            game_state.bombCount = 0;
            game_state.deactivatedBombCount = 0;
            game_state.start_time = 0;
            game_state.gameEnded = 0;
            // Reset the map for the next game
            generateMap(map);
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
 * function broadcastPoint
 * @brief Broadcast a point to all connected clients except one
 *  
 * @param point 
 * @return void
 */
void broadcastPoint(Point point) {
    pthread_mutex_lock(&client_sockets_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i].fd != 0) {
            printf("Broadcasting point (%d, %d) with state %d to client %d\n", point.x, point.y, point.state, client_sockets[i].fd);
            send(client_sockets[i].fd, &point, sizeof(Point), 0);
        }
    }
    pthread_mutex_unlock(&client_sockets_mutex);
}

/**
 * function broadcastMessage
 * @brief Broadcast a message to all connected clients
 * 
 * @param message 
 * @return void
 */
void broadcastMessage(const char *message) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i].fd != 0) {
            if (send(client_sockets[i].fd, message, strlen(message), 0) < 0) {
                perror("send");
            }
        }
    }
}

// --- Player functions ---

/**
 * function initPlayer
 * @brief Initialize the player position and role
 * 
 * @param client_socket
 * @param player 
 * @param map 
 * @param player_id
 * @return void
 */
void initPlayer(Player *player, Map *map, int *bomber_assigned, int *mine_clearer_assigned) {
    // Assign roles based on counters
    if (*bomber_assigned == 0) {
        player->role = BOMBER;
        player->x = 1; // Initial position for BOMBER
        player->y = 1;
        (*bomber_assigned)++;
    } else if (*mine_clearer_assigned == 0) {
        player->role = MINE_CLEARER;
        player->x = map->width - 1; // Initial position for MINE_CLEARER
        player->y = map->height - 1;
        (*mine_clearer_assigned)++;
    }

    // Ensure the player is not placed on a wall
    while (map->cells[player->y * map->width + player->x] == WALL) {
        if (player->role == BOMBER) {
            player->x++;
            if (player->x >= map->width) {
                player->x = 1;
                player->y++;
            }
        } else {
            player->x--;
            if (player->x < 0) {
                player->x = map->width - 2;
                player->y--;
            }
        }
    }

    printf("Player initialized at position (%d, %d) with role %s\n", player->x, player->y, player->role == BOMBER ? "BOMBER" : "MINE_CLEARER");
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