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
        (*deSerial)((generic)buffer, quoi);
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
    // The buffer is a char with this format: "code-message-arg1-arg2-...-argn"
    char *code_str = strtok((char *)buffer, "-");
    char *message = strtok(NULL, "-");
    char *args[10];
    args[0] = strtok(NULL, "-");
    int arg_count = 1;
    if (args[0] != NULL)
    {
        char *arg = strtok(NULL, "-");
        while (arg != NULL && arg_count < 10)
        {
            args[arg_count] = arg;
            arg = strtok(NULL, "-");
            arg_count++;
        }
    }

    message_t *msg = (message_t *)quoi;

    msg->code = atoi(code_str);
    strcpy(msg->message, message);
    if (args[0] != NULL)
    {
        for (int i = 0; i < arg_count; i++)
        {
            strcpy(msg->args[i], args[i]);
        }
    }
}