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
    int code;
    char message[MAX_BUFFER];
    char args[MAX_BUFFER][10];
} message_t;

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

#endif // DATA_H