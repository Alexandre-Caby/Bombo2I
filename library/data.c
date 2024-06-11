#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int DGRAM = SOCK_DGRAM;

/**
 * function envoyer
 * @brief function to send the message
 * @param sockEch - socket to send the message
 * @param quoi - message to send
 * @param serial - function to serialize the message
 * @param ... - arguments to serialize
 * @return void
 */
void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...)
{
    buffer_t buffer;
    // Check if serialization is needed
    if (serial != NULL)
    {
        // Call the serialization function, passing the address of buffer and the variable arguments
        va_list args;
        va_start(args, serial);
        (*serial)(buffer, args);
        va_end(args);
    }
    else
    {
        // Directly assign quoi to buffer (assumed to be a char pointer)
        strcpy(buffer, (char *)quoi);
    }

    // Send the buffer to the socket
    ssize_t bytes_written;
    CHECK(bytes_written = write(sockEch->fd, buffer, strlen(buffer) + 1), "write");
}

/**
 * function recevoir
 * @brief function to serialize the message 
 * @param sockEch - socket to receive the message
 * @param quoi - message to receive
 * @param deSerial - function to deserialize the message
 * @return void
 */
void recevoir(socket_t *sockEch, generic quoi, pFct deSerial)
{
    buffer_t buffer;
    ssize_t nread = read(sockEch->fd, buffer, MAX_BUFFER - 1);
    buffer[nread] = '\0'; // Null-terminate the received data

    // Check if deserialization is needed
    if (deSerial != NULL)
    {
        // Call the deserialization function, passing the buffer and the address of quoi
        (*deSerial)(buffer, quoi);
    }
    else
    {
        // Directly assign the buffer to quoi (assumed to be a char pointer)
        strncpy((char *)quoi, buffer, MAX_BUFFER);
    }
}

/**
 * function serial_string
 * @brief function to serialize the message
 * @param quoi - message to serialize
 * @param args - arguments to serialize
 * @return void
 */
void deserial_string(generic buffer, generic quoi)
{
    // The buffer is a char with this format: "message"
    char *message = (char *)buffer;
    strcpy((char *)quoi, message);
}

/**
 * function serial_long_int
 * @brief function to serialize the message as an integer
 * @param buffer - buffer to store the message
 * @param quoi - message to serialize
 * @return void
 */
void serial_long_int(generic buffer, generic args)
{
    // serialize the long int as a string in the buffer
    sprintf(buffer, "%ld", *(long int *)args);
}

/**
 * function deserial_long_int
 * @brief function to deserialize the message
 * @param quoi - message to serialize
 * @param args - arguments to serialize
 * @return void
 */
void deserial_long_int(generic buffer, generic quoi)
{
    // Deserialize the buffer as a long int
    sscanf(buffer, "%ld", (long int *)quoi);
}

/**
 * function deserial_map
 * @brief function to deserialize the map 
 * @param quoi - message to serialize
 * @param args - arguments to serialize
 * @return void
 */
void deserial_map(generic buffer, generic quoi)
{
    Map *map = (Map *)quoi;
    memcpy(map, buffer, sizeof(Map));
}

/**
 * funtion serial_point
 * @brief function to serialize the message as a point
 * @param buffer - buffer to store the message
 * @param args - arguments to deserialize
 * @return void
 */
void serial_point(generic buffer, generic args){
    Point *point = (Point *)args;
    memcpy(buffer, point, sizeof(Point));
}

/**
 * function deserial_point
 * @brief Function to deserialize the message as a point
 * 
 * @param buffer 
 * @param quoi 
 * @return void
 */
void deserial_point(generic buffer, generic quoi) {
    memcpy(quoi, buffer, sizeof(Point));
}
