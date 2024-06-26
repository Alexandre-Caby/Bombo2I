#include "session.h"
#include <string.h>

int STREAM = SOCK_STREAM;

/**
 * function adr2struct
 * @brief Function to convert an IP address and port number to a sockaddr_in structure
 * @param addr - pointer to the sockaddr_in structure
 * @param adrIP - IP address
 * @param port - port number
 * @return void
 */
void adr2struct(struct sockaddr_in *addr, char *adrIP, short port) {
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = PF_INET;
    addr->sin_port = htons(port);
    if (inet_pton(AF_INET, adrIP, &addr->sin_addr) <= 0) {
        printf("test in adr2struct\n");
        printf("Invalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
}

/**
 * function creerAddr_in
 * @brief Function to create a sockaddr_in structure
 * @param ip - IP address
 * @param port - port number
 * @return struct sockaddr_in
 */
struct sockaddr_in creerAddr_in(char *ip, short port) {
    struct sockaddr_in addr;

    if(ip == NULL) {
        perror("Invalid IP address");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    memset(&addr.sin_zero, 0, 8);
    return addr;
}

/**
 * function creerSocket
 * @brief Function to create a socket
 * @param mode - mode of the socket (SOCK_STREAM or SOCK_DGRAM)
 * @return socket_t
 */
socket_t creerSocket(int mode) {
    int sockfd = socket(PF_INET, mode, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    socket_t sock = {
        .fd = sockfd,
        .mode = mode
    };
    return sock;
}

/**
 * function createAndConnectToServer
 * @brief Function to create a socket and connect to the server
 * @return socket_t
 */
socket_t createAndConnectToServer() {
    // Création de la socket pour envoi de la requête
    socket_t sock = creerSocket(SOCK_STREAM);
    struct sockaddr_in svc = creerAddr_in(INADDR_SVC, PORT_SVC);
    CHECK(connect(sock.fd, (struct sockaddr *)&svc, sizeof svc), "Can't connect");
    return sock;
}

/**
 * function creerSocketAdr
 * @brief Function to create a socket with an IP address and port number
 * @param mode - mode of the socket (SOCK_STREAM or SOCK_DGRAM)
 * @param adrIP - IP address
 * @param port - port number
 * @return socket_t
 */
socket_t creerSocketAdr(int mode, char *adrIP, short port) {
    socket_t sock = creerSocket(mode);  
    struct sockaddr_in addr;
    
    if (strcmp(adrIP, "any") == 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        adr2struct(&addr, adrIP, port);
    }
    return sock;
}

/**
 * function creerSocketEcoute
 * @brief Function to create a listening socket
 * @param adrIP - IP address
 * @param port - port number
 * @return socket_t
 */
socket_t creerSocketEcoute(char *adrIP, short port) {
    socket_t sock = creerSocket(STREAM);
    struct sockaddr_in addr;
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (strcmp(adrIP, "any") == 0) { // If "any" is passed, use INADDR_ANY
        addr.sin_addr.s_addr = INADDR_ANY;
    } else if (inet_pton(AF_INET, adrIP, &addr.sin_addr) <= 0) {
        printf("test in creerSocketEcoute\n");
        printf("Invalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
    
    if (bind(sock.fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(sock.fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

/**
 * function accepterClt
 * @brief Function to accept a client connection
 * @param sockEcoute - listening socket
 * @return socket_t
 */
socket_t accepterClt(const socket_t sockEcoute) {
    struct sockaddr_in client_addr;
    socklen_t clilen = sizeof(client_addr);
    int newsockfd = accept(sockEcoute.fd, (struct sockaddr *)&client_addr, &clilen);
    if (newsockfd < 0) {
        perror("ERROR on accept");
        exit(EXIT_FAILURE);
    }
    socket_t newsock;
    newsock.fd = newsockfd;
    return newsock;
}

/**
 * function connecterClt2Srv
 * @brief Function to connect a client to a server
 * @param adrIP - IP address of the server
 * @param port - port number of the server
 * @return socket_t
 */
socket_t connecterClt2Srv(char *adrIP, short port) {
    socket_t sock = creerSocket(STREAM); // Assuming STREAM is for TCP
    struct sockaddr_in serv_addr;
    adr2struct(&serv_addr, adrIP, port);
    
    if (connect(sock.fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}