#include "kermit.h"
#include "ConexaoRawSocket.h"

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Usage: ./main <IS_CLIENT> \n");
        return -1;
    }

    int isClient = (atoi(argv[1]) == 1);
    char *nome_socket = get_ethernet_interface_name();
    printf("Socket: %s\n", nome_socket);
    int socket = ConexaoRawSocket("lo");
    if(isClient) {
        client(socket);
    } else {
        server(socket);
    }

    return 0;
}