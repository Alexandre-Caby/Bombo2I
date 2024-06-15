#ifndef DATA_H
#define DATA_H

/*******************************************/
/*		I N C L U D E S                    */
/*******************************************/
#include "session.h"

/*******************************************/
/*		D E F I N E S                      */
/*******************************************/
/**
 * @brief Global variable to store the session
 * 
 */
#define MAX_BUFFER 1024
#define MAX_MAP_WIDTH 48
#define MAX_MAP_HEIGHT 24
#define MAX_MAP_SIZE MAX_MAP_WIDTH * MAX_MAP_HEIGHT
#define CELL_SIZE 24

/*******************************************/
/*		S T R U C T U R E S                */
/*******************************************/
/**
 * @brief structure to store the message
 * @typedef message_t
 * 
 */
typedef struct
{
    char buffer[MAX_BUFFER];
    int size;
} message_t;

typedef struct {
    int width;
    int height;
    int cells[MAX_MAP_SIZE];
} Map;

typedef struct {
    int x;
    int y;
    int state;
} Point;

/**
 * @brief structure to store the socket
 * @typedef socket_t
 * 
 */
typedef char buffer_t[MAX_BUFFER];

/**
 * @brief structure to store the socket
 * @typedef socket_t
 * 
 */
typedef void *generic;

/**
 * @brief structure to store the socket
 * @typedef socket_t
 * 
 */
typedef void (*pFct)(generic, generic);


/*******************************************/
/*		F O N C T I O N S                  */
/*******************************************/
/**
 * function envoyer
 * @brief Function to send the message
 * @param sockEch - socket to send the message
 * @param quoi - message to send
 * @param serial - function to serialize the message
 * @param ... - arguments to serialize
 * @return void
 */
void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...);

/**
 * function recevoir
 * @brief Function to receive the message
 * @param sockEch - socket to receive the message
 * @param quoi - message to receive
 * @param deSerial - function to deserialize the message
 * @return void
 */
void recevoir(socket_t *sockEch, generic quoi, pFct deSerial);

/**
 * function serial_string
 * @brief Function to serialize the message
 * @param buffer - buffer to store the message
 * @param quoi - message to serialize
 * @return void
 */
void deserial_string(generic buffer, generic quoi);

/**
 * function serial_int
 * @brief Function to serialize the message as an integer
 * @param buffer - buffer to store the message
 * @param args - arguments to serialize
 * @return void
 */
void serial_long_int(generic buffer, generic args);

/**
 * function deserial_int
 * @brief Function to deserialize the message as an integer
 * @param buffer - buffer to store the message
 * @param quoi - message to deserialize
 * @return void
 */
void deserial_long_int(generic buffer, generic quoi);

/**
 * funtion serial_point
 * @brief Function to serialize the message as a point
 * 
 * @param buffer 
 * @param args 
 * @return void
 */
void serial_point(generic buffer, generic args);

/**
 * function deserial_point
 * @brief Function to deserialize the message as a point
 * 
 * @param buffer 
 * @param quoi
 * @return void
 */
void deserial_point(generic buffer, generic quoi);

#endif // DATA_H