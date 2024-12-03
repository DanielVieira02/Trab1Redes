#include "kermit.h"
#include "ConexaoRawSocket.h"

int main(int argc, char * argv[]) {
    int socket = -1;
    if (argc < 2) {
        printf("Usage: ./main <IS_CLIENT> \n");
        return -1;
    }

    int isClient = (atoi(argv[1]) == 1);
    char *nome_socket = get_ethernet_interface_name();
    printf("Socket: %s\n", nome_socket);
    #ifndef LOOP
        socket = ConexaoRawSocket(nome_socket);
    #endif
    if(isClient) {
        #ifdef LOOP
            socket = conexaoDebug(nome_socket, IP_A);
        #endif
        client(socket);
    } else {
        #ifdef LOOP
            socket = conexaoDebug(nome_socket, IP_B);
        #endif
        server(socket);
    }

    return 0;
}