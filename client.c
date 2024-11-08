#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para a função strlen()
#include <sys/types.h>
#include <sys/socket.h>

#include "ConexaoRawSocket.h"
#include "kermit.h"

int envia_arquivo(char * nome_arquivo, int socket) {
    printf("Enviando arquivo...\n");

    FILE * arquivo = fopen(nome_arquivo, "rb");
    if(arquivo == NULL) {
        printf("Erro ao ler arquivo. Encerrando execução\n");
        return -1;
    }

    char *buffer;
    if(!(buffer = malloc(sizeof(char) * 255))) {
        return -1;
    }

    size_t bytesLidos;
    kermit_packet * pacote;
    unsigned char sequencia = 0;

    while((bytesLidos = fread(buffer, 1, sizeof(unsigned char) * 63, arquivo)) > 0) {
        pacote = inicializa_pacote(OK, sequencia);
        insere_dados_pacote(pacote, buffer, bytesLidos);
        envia_pacote(pacote, socket);
        pacote = destroi_pacote(pacote);
        sequencia++;
    }

    fclose(arquivo);

    return 0;
}

int ler_entrada(char * buffer) {
    if (fgets(buffer, 255, stdin) == NULL) {
        return -1;
    }
    buffer[strcspn(buffer, "\n")] = 0;
    return 0;
}

int client() {
    int socket = ConexaoRawSocket("eno1");
    char *buffer;
    if(!(buffer = malloc(sizeof(char) * 255))) {
        return -1;
    }

    int comando;
    int executar = 1;

    while(executar) {
        system("clear");
        printf("Escolha o comando \n");
        printf("[1] Backup  [2] Restaura    [3] Verifica \n");
        printf("[0] Sair\n");
        if (ler_entrada(buffer) == -1) {
            fprintf(stderr, "Erro ao ler a linha\n");
            break;
        }
        
        comando = atoi(buffer);
        
        switch(comando) {
            case 1:
                printf("Insira o nome do arquivo: \n");
                ler_entrada(buffer);
                if(envia_arquivo(buffer, socket) == -1) {
                    break;
                }
                ler_entrada(buffer);
                break;
            case 2:
                printf("Restaura TODO, mano\n");
                break;
            case 3:
                printf("Restaura TODO, mano\n");
                break;
            default:
                executar = 0;
                break;
        }
        
    }

    free(buffer);
    return 0;
}

