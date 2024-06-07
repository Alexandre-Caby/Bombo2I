#include "communication_socket.h"

/**
 * function handleClient
 * @brief Handle the client connection
 * 
 * @param socket_desc 
 * @return void* 
 */
void *handleClient(void *socket_desc) {
    socket_t sd = *(socket_t*) socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    free(socket_desc);
    return 0;
}

int main(){
        socket_t se = creerSocketEcoute(PORT_SERVER, ADDRESS_SERVER);

    while (1) {
        socket_t *sd = malloc(sizeof(socket_t));  // Allocate memory to hold the socket descriptor
        *sd = accepterClt(se);

        // Send welcome message
        envoyer(sd, "Welcome to Bombo2I!\n", NULL);

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handleClient, (void*) sd) < 0) {
            perror("could not create thread");
            return 1;
        }

        // Detach the thread so that resources are automatically reclaimed upon completion
        pthread_detach(client_thread);
    }
    return 0;
}