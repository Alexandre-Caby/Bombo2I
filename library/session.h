#ifndef SESSION_H
#define SESSION_H

/*******************************************/
/*		I N C L U D E S                    */
/*******************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*******************************************/
/*		D E F I N E S                      */
/*******************************************/
/**
 * @brief Port number for the service to connect to
 * @def PORT_SVC
 * @see INADDR_SVC
 */
#define PORT_SVC 8080
#define INADDR_SVC "192.168.144.100"

/**
 * @brief Macro to check the return value of a function
 * @def CHECK
 * @param sts - return value of the function
 * @param msg - message to print in case of error
 */
#define CHECK(sts,msg) if ((sts)==-1) {perror(msg); exit(-1);}

/**
 * @brief Macro to pause the execution of the program
 * @def PAUSE
 * @param msg - message to print
 */
#define PAUSE(msg)	printf("%s [Appuyez sur entrée pour continuer]", msg); getchar();

/*******************************************/
/*		S T R U C T U R E S                */
/*******************************************/
/**
 * @brief Structure to store the socket
 * @typedef socket_t
 * 
 */
struct socket {
	int fd;	
	int mode;
	struct sockaddr_in addrLoc;	
	struct sockaddr_in addrDst;	
};

/**
 * @brief Typedef for the socket structure
 * @typedef socket_t
 * 
 */
typedef struct socket socket_t; 

/*******************************************/
/*		F O N C T I O N S                  */
/*******************************************/
/**
 * function adr2struct
 * @brief Function to convert an IP address and port number to a sockaddr_in structure
 * @param addr - pointer to the sockaddr_in structure
 * @param adrIP - IP address
 * @param port - port number
 * @return void
 */
void adr2struct (struct sockaddr_in *addr, char *adrIP, short port);

/**
 * function creerSocket
 * @brief Function to create a socket
 * @param mode - mode of the socket (SOCK_STREAM or SOCK_DGRAM)
 * @return socket_t
 */
socket_t creerSocket (int mode);

/**
 * function createAndConnectToServer
 * @brief Function to create a socket and connect to the server
 * @return socket_t
 */
socket_t createAndConnectToServer();

/**
 * function creerSocketAdr
 * @brief Function to create a socket with an IP address and port number
 * @param mode - mode of the socket (SOCK_STREAM or SOCK_DGRAM)
 * @param adrIP - IP address
 * @param port - port number
 * @return socket_t
 */
socket_t creerSocketAdr (int mode, char *adrIP, short port);

/**
 * function creerSocketEcoute
 * @brief Function to create a listening socket
 * @param adrIP - IP address
 * @param port - port number
 * @return socket_t
 */
socket_t creerSocketEcoute (char *adrIP, short port);

/**
 * function accepterClt
 * @brief Function to accept a client connection
 * @param sockEcoute - listening socket
 * @return socket_t
 */
socket_t accepterClt (const socket_t sockEcoute);

/**
 * function connecterClt2Srv
 * @brief Function to connect a client to a server
 * @param adrIP - IP address of the server
 * @param port - port number of the server
 * @return socket_t
 */
socket_t connecterClt2Srv (char *adrIP, short port);

/**
 * function creerAddr_in
 * @brief Function to create a sockaddr_in structure
 * @param ip - IP address
 * @param port - port number
 * @return struct sockaddr_in
 */
struct sockaddr_in creerAddr_in(char *ip, short port);

#endif /* SESSION_H */
