#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para a função strlen()
#include "ConexaoRawSocket.h"
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char * argv[]) {
    int socket;
    char *buffer = malloc(100);

    socket = ConexaoRawSocket("eno1");

    while(1){
        // Usando fgets para ler uma linha completa
        if (fgets(buffer, 100, stdin) == NULL) {
            // Se a leitura falhar, sair do loop
            fprintf(stderr, "Erro ao ler a linha\n");
            break;
        }

        // Remove o caractere de nova linha '\n' se ele existir
        buffer[strcspn(buffer, "\n")] = 0;

        printf("A mensagem digitada foi: %s\n", buffer);

        if (send(socket, buffer, strlen(buffer) + 1, 0) == -1) {
            fprintf(stderr, "Erro ao enviar mensagem\n");
        }
    }

    free(buffer);  // Não se esqueça de liberar a memória alocada

    return 0;
}

