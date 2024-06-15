#include "map.h"

// --- Global variables ---
pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t renderer_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * function main
 * @brief Main function
 * 
 * @return int
 */
int main() {
    socket_t sock = createAndConnectToServer();
    if(sock.fd == -1) {
        printf("Could not connect to the server\n");
        return 1;
    }

    // Receive the welcome message from the server
    message_t welcome_message;
    recevoir(&sock, &welcome_message, deserial_string);
    printf("%s", welcome_message.buffer);

    // Initialize the random number generator
    srand(time(NULL));

    // Create a new map
    Map *map = map_new(MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
    // Receive map dimensions from server
    int map_size[2];
    recv(sock.fd, map_size, 2 * sizeof(int), 0);
    map->width = map_size[0];
    map->height = map_size[1];
    printf("Map received, width: %d, height: %d\n", map->width, map->height);

    // Receive map cells from server
    int *received_cells = (int *)malloc(map->width * map->height * sizeof(int));
    recv(sock.fd, received_cells, map->width * map->height * sizeof(int), 0);

    // Copy received cells to map->cells
    memcpy(map->cells, received_cells, map->width * map->height * sizeof(int));

    // Free allocated memory for received cells
    free(received_cells);

    // Receive the player role from the server
    Player player;
    ssize_t received_bytes = recv(sock.fd, &player, sizeof(Player), 0);
    if (received_bytes <= 0) {
        perror("Failed to receive player data");
        // Handle error appropriately
    } else if (received_bytes != sizeof(Player)) {
        fprintf(stderr, "Partial player data received\n");
        // Handle error appropriately
    }
    
    printf("You are a %s\n", player.role == BOMBER ? "bomber" : "mine clearer");
    sleep(1);

    // Initialize GPIO pins
    // gpioInitialise();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window and renderer
    SDL_Window *window = SDL_CreateWindow("Map", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAX_MAP_WIDTH * CELL_SIZE, MAX_MAP_HEIGHT * CELL_SIZE, 0);
    if (!window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_ttf (font rendering library)
    if (TTF_Init() == -1) {
        fprintf(stderr, "Could not initialize SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("../ressources/Minecraft.ttf", 12);
    if (!font) {
        fprintf(stderr, "Could not load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Render game elements
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    drawMap(renderer, map, font);
    renderPlayer(renderer, &player);
    SDL_RenderPresent(renderer);

    // Create the thread data for the receiveUpdates thread
    recv_thread_data_t *recv_data = malloc(sizeof(recv_thread_data_t));
    if (!recv_data) {
        fprintf(stderr, "Failed to allocate memory for thread data\n");
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    recv_data->sock_fd = sock.fd;
    recv_data->map = map;

    // Create the receiveUpdates thread
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receiveUpdates, recv_data) != 0) {
        fprintf(stderr, "Failed to create receiveUpdates thread\n");
        free(recv_data);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Uint32 time = 0;
    Uint32 bombPlacementTime = 5000;
    Uint32 bombDeactivationTime = 4000;
    int running = 1;
    while (running) {
        // Handle the input from the user and send it to the server
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    // Send a disconnect message to the server
                    int disconnect = -1;
                    send(sock.fd, &disconnect, sizeof(int), 0);
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        // Exit the game if the user closes the window or presses the ESC key
                        running = 0;
                        // Send a disconnect message to the server
                        int disconnect = -1;
                        send(sock.fd, &disconnect, sizeof(int), 0);
                        break;
                    } else {
                        // Handle player input based on the key pressed
                        int action = -1; // Default action
                        switch (event.key.keysym.sym) {
                            case SDLK_UP:
                                action = MOVE_UP;
                                break;
                            case SDLK_DOWN:
                                action = MOVE_DOWN;
                                break;
                            case SDLK_LEFT:
                                action = MOVE_LEFT;
                                break;
                            case SDLK_RIGHT:
                                action = MOVE_RIGHT;
                                break;
                            case SDLK_SPACE:
                                if (player.role == BOMBER) {
                                    if(map->cells[player.y * map->width + player.x] == BOMB) {
                                        showMessage(renderer, font, "Cannot place point: The cell already contains a bomb.");
                                        break;
                                    }
                                    if (SDL_GetTicks() - time > bombPlacementTime) {
                                        time = SDL_GetTicks();
                                        action = PLACE_BOMB;
                                    } else {
                                        showMessage(renderer, font, "Cannot place point: You must wait 5 seconds between each bombing.");
                                    }
                                } else if (player.role == MINE_CLEARER) {
                                    if(SDL_GetTicks() - time > bombDeactivationTime) {
                                        time = SDL_GetTicks();
                                        action = DEACTIVATE_BOMB;
                                    } else {
                                        showMessage(renderer, font, "Cannot deactivate bomb: You must wait 4 seconds between each deactivation.");
                                    }
                                }
                                break;
                            default:
                                // Error message for invalid key presses
                                showMessage(renderer, font, "Invalid key pressed. Use the arrow keys to move and the space bar to place a bomb.");
                                break;
                        }

                        handleInput(&player, map, action);
                        if (action == PLACE_BOMB || action == DEACTIVATE_BOMB) {
                            placePoint(map, renderer, font, player.x, player.y, action == PLACE_BOMB ? BOMB : DEACTIVATED_BOMB, sock.fd);
                        }
                    }
                    break;
                case SDL_USEREVENT:
                    switch (event.user.code) {
                        case 1:
                            // Render the updated map
                            pthread_mutex_lock(&renderer_mutex);
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                            SDL_RenderClear(renderer);
                            drawMap(renderer, map, font);
                            renderPlayer(renderer, &player);
                            SDL_RenderPresent(renderer);
                            pthread_mutex_unlock(&renderer_mutex);
                            break;
                        case 2:
                            // Display the message received from the server
                            showMessage(renderer, font, event.user.data1);
                            break;
                        case 3:
                            // Display the game ended message received from the server
                            showMessage(renderer, font, event.user.data1);
                            SDL_Delay(2000); // Display message for 2 seconds
                            running = 0;
                            break;
                    }
                    break;
            }          
        }
        if (SDL_USEREVENT != event.type) {
            // Render the updated map
            pthread_mutex_lock(&renderer_mutex);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);
            drawMap(renderer, map, font);
            renderPlayer(renderer, &player);
            SDL_RenderPresent(renderer);
            pthread_mutex_unlock(&renderer_mutex);
        }
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(sock.fd);

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
 * function drawMap
 * @brief Draw the map
 * 
 * @param renderer 
 * @param map 
 * @return void
 */
void drawMap(SDL_Renderer *renderer, Map *map, TTF_Font *font) {
    SDL_Color textColor = { 255, 255, 255, 255 }; // White
    SDL_Color bgColor = { 0, 0, 0, 255 }; // Black

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };

            // Define the background color based on the cell type
            switch (map->cells[y * map->width + x]) {
                case WALL:
                    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a); // Black for the wall
                    break;
                case PATH:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White for the path
                    break;
                case BOMB:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for the bomb
                    break;
                case DEACTIVATED_BOMB:
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for the deactivated bomb
                    break;
            }
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderDrawRect(renderer, &rect);

            // Display the coordinates on the first row and column
            if (x == 0 || y == 0) {
                char coords[4];
                if (x == 0 && y != 0) {
                    sprintf(coords, "%d", y);
                } else if (y == 0 && x != 0) {
                    sprintf(coords, "%d", x);
                }

                if ((x == 0 && y != 0) || (y == 0 && x != 0)) {
                    SDL_Surface *surface = TTF_RenderText_Solid(font, coords, textColor);
                    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
                    int text_width = surface->w;
                    int text_height = surface->h;
                    SDL_FreeSurface(surface);

                    SDL_Rect textRect = { x * CELL_SIZE + (CELL_SIZE - text_width) / 2, y * CELL_SIZE + (CELL_SIZE - text_height) / 2, text_width, text_height };
                    SDL_RenderCopy(renderer, texture, NULL, &textRect);
                    SDL_DestroyTexture(texture);
                }
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
 * function isAccessible
 * @brief Check if a cell is accessible from a path, i.e. not surrounded by walls
 * 
 * @param map 
 * @param x 
 * @param y 
 * @return int
 */
int isAccessible(Map *map, int x, int y) {
    // Check if the coordinates are within the map boundaries
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return 0;
    }

    // Check if the cell itself is a path or a bomb
    if (map->cells[y * map->width + x] != PATH && map->cells[y * map->width + x] != BOMB) {
        return 0;
    }

    // Check adjacent cells
    int numWalls = 0;
    if (x > 0 && map->cells[y * map->width + (x - 1)] == WALL) {
        numWalls++; // Left cell
    }
    if (x < map->width - 1 && map->cells[y * map->width + (x + 1)] == WALL) {
        numWalls++; // Right cell
    }
    if (y > 0 && map->cells[(y - 1) * map->width + x] == WALL) {
        numWalls++; // Upper cell
    }
    if (y < map->height - 1 && map->cells[(y + 1) * map->width + x] == WALL) {
        numWalls++; // Lower cell
    }

    // If there are 4 walls around the cell, it's not accessible
    if (numWalls == 4) {
        return 0;
    }

    // Check if the cell is connected to a path
    if (numWalls == 3) {
        // Check diagonally adjacent cells
        if ((x > 0 && y > 0 && map->cells[(y - 1) * map->width + (x - 1)] != WALL) ||
            (x < map->width - 1 && y > 0 && map->cells[(y - 1) * map->width + (x + 1)] != WALL) ||
            (x > 0 && y < map->height - 1 && map->cells[(y + 1) * map->width + (x - 1)] != WALL) ||
            (x < map->width - 1 && y < map->height - 1 && map->cells[(y + 1) * map->width + (x + 1)] != WALL)) {
            return 1;
        }
        return 0;
    }

    return 1;
}

/**
 * function placePoint
 * @brief Place a point on the map
 * 
 * @param map 
 * @param renderer 
 * @param font 
 * @param x 
 * @param y 
 * @param action 
 * @param sock 
 * @return void
 */
void placePoint(Map *map, SDL_Renderer *renderer, TTF_Font *font, int x, int y, int action, int sock) {  
    if (!isAccessible(map, x, y)) {
        showMessage(renderer, font, "Cannot place point: The cell is not accessible.");
        return;
    }

    // If it's a wall, we can't place a point
    if (map->cells[y * map->width + x] == WALL) {
        showMessage(renderer, font, "Cannot place point: The cell is a wall.");
        return;
    }
    printf("Placing point at (%d, %d)\n", x, y);
    printf("Action: %d\n", action);

    // If the player is a mine clearer, they can only deactivate bombs on a cell with a bomb state
    if (action == DEACTIVATED_BOMB && map->cells[y * map->width + x] != BOMB) {
        showMessage(renderer, font, "Cannot deactivate bomb: The cell does not contain a bomb.");
        return;
    }
    
    // Send the state and coordinates of the point to the server
    Point point = { x, y, action };
    send(sock, &point, sizeof(Point), 0);
}

/**
 * function renderText
 * @brief Render text on the screen
 * 
 * @param renderer 
 * @param font 
 * @param text 
 * @param x 
 * @param y 
 * @param color 
 * @return void
 */
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, SDL_Color bgColor) {
    TTF_SetFontSize(font, 26);
    // Render text surface
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    if (textSurface == NULL) {
        fprintf(stderr, "Unable to render text surface: %s\n", TTF_GetError());
        return;
    }

    // Create texture from surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (texture == NULL) {
        fprintf(stderr, "Unable to create texture from surface: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }

    // Get text dimensions
    int text_width = textSurface->w;
    int text_height = textSurface->h;
    SDL_FreeSurface(textSurface);

    // Create background rectangle
    SDL_Rect textRect = { x, y, text_width, text_height };
    SDL_Rect bgRect = { x - 5, y - 5, text_width + 10, text_height + 10 }; // Add padding to background

    // Render background rectangle
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Enable blending for semi-transparent color
    SDL_RenderFillRect(renderer, &bgRect);

    // Render text
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);
    TTF_SetFontSize(font, 12);
}

/**
 * function showMessage
 * @brief Display a message on the screen
 * 
 * @param renderer 
 * @param font 
 * @param message 
 * @return void
 */
void showMessage(SDL_Renderer *renderer, TTF_Font *font, const char *message) {
    SDL_Color color = { 255, 0, 0, 255 }; // Red color
    SDL_Color bgColor = { 0, 0, 0, 200 }; // Semi-transparent black background
    // Measure the text dimensions
    int textWidth, textHeight;
    if (TTF_SizeText(font, message, &textWidth, &textHeight) != 0) {
        fprintf(stderr, "Failed to measure text: %s\n", TTF_GetError());
        return;
    }

    // Calculate the position to center the text on the screen
    int screenWidth = MAX_MAP_WIDTH * CELL_SIZE;
    int screenHeight = MAX_MAP_HEIGHT * CELL_SIZE;
    int x = (screenWidth - textWidth) / 4;
    int y = (screenHeight - textHeight) / 4;

    // Render the text
    renderText(renderer, font, message, x, y, color, bgColor);

    // Present the renderer
    SDL_RenderPresent(renderer);

    // Display message for 2 seconds
    SDL_Delay(2000);

    // Clear the renderer
    SDL_RenderClear(renderer);
}

/**
 * function handleInput
 * @brief Handle the input from the user
 * 
 * @param player 
 * @param map 
 * @param event 
 * @return void
 */
void handleInput(Player *player, Map *map, int action) {
    switch (action) {
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

/**
 * function renderPlayer
 * @brief Render the player on the map
 * 
 * @param renderer 
 * @param player 
 * @return void
 */
void renderPlayer(SDL_Renderer *renderer, Player *player) {
    if (player->role == BOMBER) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue color for the player BOMBER
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // Purple color for the player MINE_CLEARER
    }
    SDL_Rect playerRect = { player->x * CELL_SIZE, player->y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    SDL_RenderFillRect(renderer, &playerRect);
}

/**
 * function gpioInitialise
 * @brief Initialize the GPIO pins
 * 
 * @return void
 */
// void gpioInitialise() {
//     wiringPiSetupGpio();
//     pinMode(GPIO_PIN_UP, INPUT);
//     pinMode(GPIO_PIN_DOWN, INPUT);
//     pinMode(GPIO_PIN_LEFT, INPUT);
//     pinMode(GPIO_PIN_RIGHT, INPUT);
//     pullUpDnControl(GPIO_PIN_UP, PUD_UP);
//     pullUpDnControl(GPIO_PIN_DOWN, PUD_UP);
//     pullUpDnControl(GPIO_PIN_LEFT, PUD_UP);
// }

// --- Communication functions ---

/**
 * function receiveUpdates
 * @brief Receive updates from the server
 * 
 * @param arg (wrapped arguments)
 * @return void*
 */
void *receiveUpdates(void *arg) {
    recv_thread_data_t *data = (recv_thread_data_t *)arg;
    int sock = data->sock_fd;
    Map *map = data->map;
    
    char buffer[BUFFER_SIZE];
    ssize_t recv_size;

    while (1) {
        recv_size = recv(sock, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            if (recv_size == 0) {
                printf("Server closed connection.\n");
            } else {
                perror("recv");
            }
            break;
        }

        buffer[recv_size] = '\0';

        // Determine the type of message received
        if (recv_size == sizeof(Point)) {
            Point *point = (Point *)buffer;
            
            pthread_mutex_lock(&map_mutex);
            setSpecialPoint(map, point->x, point->y, point->state);
            pthread_mutex_unlock(&map_mutex);

            SDL_Event event;
            event.type = SDL_USEREVENT;
            event.user.code = 1; // Code 1 for rendering the map
            SDL_PushEvent(&event);
        } else {
            printf("Received message from server: %s\n", buffer);

            if (strstr(buffer, "Game ended") != NULL) {
                SDL_Event event;
                event.type = SDL_USEREVENT;
                event.user.code = 3;
                event.user.data1 = strcpy(malloc(strlen(buffer) + 1), buffer);
                SDL_PushEvent(&event);
            }else{
                // Display the message received from the server
                SDL_Event event;
                event.type = SDL_USEREVENT;
                event.user.code = 2;
                event.user.data1 = strcpy(malloc(strlen(buffer) + 1), buffer);
                SDL_PushEvent(&event);
            }
        }
    }

    return NULL;
}
