#include "../library/data.h"
#include "../library/session.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include <unistd.h>
#include <pthread.h>

// --- Constants ---
#define PORT_SERVER 8080
#define ADDRESS_SERVER "192.168.1.100"
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

// --- Structures ---
typedef struct {
    int socket;
    char *buffer[BUFFER_SIZE];
    int buffer_size;
} CommunicationSocket;

// --- Functions ---



