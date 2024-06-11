#include "map.h"

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

    // Receive the map dimensions from the server
    // int map_width, map_height;
    // recevoir(&sock, &map_width, deserial_long_int);
    // recevoir(&sock, &map_height, deserial_long_int);
    // printf("Map dimensions: %d x %d\n", map_width, map_height);
    // // Create a new map based on the received dimensions
    Map *map = map_new(MAX_MAP_WIDTH, MAX_MAP_HEIGHT);

    // // Receive the map cells from the server
    // for (int y = 0; y < map->height; y++) {
    //     for (int x = 0; x < map->width; x++) {
    //         int cell;
    //         recevoir(&sock, &cell, deserial_long_int);
    //         map->cells[y * map->width + x] = cell;
    //     }
    // }

    recevoir(&sock, map, deserial_long_int);
    printf("Map received\n");

    // Receive the player role from the server
    Player player;
    recevoir(&sock, &player, deserial_string);
    printf("You are a %s\n", player.role == BOMBER ? "BOMBER" : "MINE_CLEARER");

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

    Uint32 time = 0;
    Uint32 bombPlacementTime = 5000;
    while (1) {
        // Handle the input from the user and send it to the server
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
                // Exit the game if the user closes the window or presses the ESC key
                break;
            } else if (event.type == SDL_KEYDOWN) {
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
                        if (map->cells[player.y * map->width + player.x] == PATH) {
                            if (player.role == BOMBER) {
                                if (SDL_GetTicks() - time > bombPlacementTime) {
                                    time = SDL_GetTicks();
                                    action = PLACE_BOMB;
                                } else {
                                    showMessage(renderer, font, "Cannot place point: You must wait 5 seconds between each bombing.");
                                }
                            } else {
                                action = DEACTIVATE_BOMB;
                            }
                        }
                        break;
                    default:
                        // Error message for invalid key presses
                        showMessage(renderer, font, "Invalid key pressed. Use the arrow keys to move and the space bar to place a bomb.");
                        break;
                }

                // Send the action to the server
                if (action != -1) {
                    envoyer(&sock, &action, NULL);
                }
            }
        }

        // Receive the updated player position from the server
        recevoir(&sock, &player, NULL);

        // Render game elements
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawMap(renderer, map, font);
        renderPlayer(renderer, &player);
        SDL_RenderPresent(renderer);

        // Condition to exit the game : 
        // all bombs are deactivated = victory for the mine clearer 
        // time is up (1 minute) = victory for the bomber
        // Prerequisite : 5 bombs must be placed on the map before counting the time 
        // if the player is a bomber and the time is up, the bomber wins
        // if the player is a mine clearer and all bombs are deactivated within the time limit, the mine clearer wins
        int bombCount = 0;
        if (player.role == MINE_CLEARER) {
            int numBombs = 0;
            for (int i = 0; i < map->width * map->height; i++) {
                if (map->cells[i] == DEACTIVATED_BOMB) {
                    numBombs++;
                }
            }
            if (numBombs == 5) {
                showMessage(renderer, font, "All bombs are deactivated. Victory for the mine clearer !");
                break;
            }
        }

        if(player.role == BOMBER) {
            for (int i = 0; i < map->width * map->height; i++) {
                if (map->cells[i] == BOMB) {
                    bombCount++;
                }
            }
        }
        if (bombCount == 5) {
            showMessage(renderer, font, "All bombs are placed. The time starts now !");
            // start the timer
            Uint32 startTime = SDL_GetTicks();

            if (SDL_GetTicks() - startTime > 60000) {
                showMessage(renderer, font, "Time is up. Victory for the bomber !");
                break;
            }
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
 * function carvePathFrom
 * @brief Carve a path from a given point
 * 
 * @param x 
 * @param y 
 * @param map
 * @return void
 */
void carvePathFrom(int x, int y, Map *map) {
    int directions[] = { 0, 1, 2, 3 };
    // Shuffle the directions array
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        int temp = directions[i];
        directions[i] = directions[r];
        directions[r] = temp;
    }

    for (int i = 0; i < 4; i++) {
        int dx = 0, dy = 0;
        switch (directions[i]) {
            case 0: dy = -1; break;
            case 1: dy = 1; break;
            case 2: dx = -1; break;
            case 3: dx = 1; break;
        }

        int nx = x + 2 * dx;
        int ny = y + 2 * dy;

        if (nx >= 0 && nx < map->width && ny >= 0 && ny < map->height && map->cells[ny * map->width + nx] == WALL) {
            map->cells[(ny - dy) * map->width + (nx - dx)] = PATH;
            map->cells[ny * map->width + nx] = PATH;
            carvePathFrom(nx, ny, map);
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

    // Check if the cell itself is a path
    if (map->cells[y * map->width + x] != PATH) {
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
 * @param x 
 * @param y 
 * @param state 
 * @return void
 */
void placePoint(Map *map, SDL_Renderer *renderer, TTF_Font *font, int x, int y, int state) {
    int count = 0;
    for (int i = 0; i < map->width * map->height; ++i) {
        if (map->cells[i] == BOMB || map->cells[i] == DEACTIVATED_BOMB) {
            ++count;
        }
    }
    
    if (count >= 5) {
        showMessage(renderer, font, "Cannot place point: Too many points ! The maximum allowed is 5.");
        return;
    }
    
    if (!isAccessible(map, x, y)) {
        showMessage(renderer, font, "Cannot place point: The cell is not accessible.");
        return;
    }

    // if it's a wall, we can't place a point
    if (map->cells[y * map->width + x] == WALL) {
        showMessage(renderer, font, "Cannot place point: The cell is a wall.");
        return;
    }
    
    setSpecialPoint(map, x, y, state);
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
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int text_width = textSurface->w;
    int text_height = textSurface->h;
    SDL_FreeSurface(textSurface);

    SDL_Rect textRect = { x, y, text_width, text_height };
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &textRect);
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
    // a backgournd with less opacity
    SDL_Color bgColor = { 0, 0, 0, 150 }; // Black color
    // Center the message on the screen dynamically based on the message length 
    int x = MAX_MAP_WIDTH * CELL_SIZE / 2 + 100 - strlen(message) * 5;
    int y = MAX_MAP_HEIGHT * CELL_SIZE / 4;
    renderText(renderer, font, message, x, y, color, bgColor);
    SDL_RenderPresent(renderer);
    SDL_Delay(2000); // Display message for 2 seconds
    SDL_RenderClear(renderer); // Clear the message after 2 seconds
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
// void handleInput(Player *player, Map *map, SDL_Event event) {
//     if (event.type == SDL_KEYDOWN) {
//         switch (event.key.keysym.sym) {
//             case SDLK_UP:
//                 movePlayer(player, map, 0, -1);
//                 break;
//             case SDLK_DOWN:
//                 movePlayer(player, map, 0, 1);
//                 break;
//             case SDLK_LEFT:
//                 movePlayer(player, map, -1, 0);
//                 break;
//             case SDLK_RIGHT:
//                 movePlayer(player, map, 1, 0);
//                 break;
//         }
//     }
//     // if (event.type == SDL_USEREVENT+1) {
//     //     if (event.user.code == GPIO_PIN_UP) {
//     //         printf("Pressed up\n");
//     //         movePlayer(player, map, 0, -1);
//     //     } else if (event.user.code == GPIO_PIN_DOWN) {
//     //         printf("Pressed down\n");
//     //         movePlayer(player, map, 0, 1);
//     //     } else if (event.user.code == GPIO_PIN_LEFT) {
//     //         printf("Pressed left\n");
//     //         movePlayer(player, map, -1, 0);
//     //     } else if (event.user.code == GPIO_PIN_RIGHT) {
//     //         printf("Pressed right\n");
//     //         movePlayer(player, map, 1, 0);
//     //     }
//     // }
// }

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