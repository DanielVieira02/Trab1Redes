#include "ConexaoRawSocket.h"

int main(int argc, char * argv[]) {
    int socket = ConexaoRawSocket("eno1");
    unsigned char * buffer = (unsigned char *)malloc(100);

    int buffer_length;

    while(1) {
    	buffer_length = recv(socket, (char*) buffer, sizeof(buffer), 0);
	if (buffer_length == -1) {
	   printf("Erro ao receber pacote.\n");	   
	}
	printf("%s\n", buffer);
    }

    free(buffer);

    return 0;
}
