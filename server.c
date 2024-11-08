#include "ConexaoRawSocket.h"
#include "kermit.h"

void server_routine() {
    int socket = ConexaoRawSocket("eno1");
    char * buffer = (char *)malloc(100);

    int buffer_length;
    kermit_packet * packet = NULL;

    while(1) {
    	buffer_length = recv(socket, (char*) buffer, 68, 0);
	if (buffer_length == -1) {
	    printf("Erro ao receber pacote.\n");	   
	}
    }
	packet = converte_bytes_para_pacote(buffer);

	if (packet != NULL)
		print_pacote(packet);

    free(buffer);
}

int server(int argc, char * argv[]) {
    server_routine();
    return 0;
}
