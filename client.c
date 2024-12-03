#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para a função strlen()
#include <sys/types.h>
#include <sys/socket.h>

#include "kermit.h"

kermit_protocol_state * fim_dados_client(unsigned char * resposta, void * dados, int socket);
kermit_protocol_state * dados_client(unsigned char * resposta, void * dados, int socket);
kermit_protocol_state * tamanho_client(unsigned char * resposta, void * dados, int socket);
void backup_client(FILE* arquivo, char * nome_arq, int socket);

kermit_protocol_state * fim_dados_client(unsigned char * resposta, void * dados, int socket) {
    return NULL;
}

kermit_protocol_state * dados_client(unsigned char * resposta, void * dados, int socket) {
    // kermit_protocol_state * next_state = NULL;
    // unsigned char * packet;
    // unsigned char* buffer;
    // size_t bytesLidos;
    // FILE* arquivo = (FILE *)dados;

    // if(!(buffer = malloc(sizeof(char) * 255))) {
    //     fclose(arquivo);
    //     return NULL;
    // }
    // switch (get_tipo_pacote(resposta)) {
    //     case ACK:            
    //         if((bytesLidos = fread(buffer, 1, 63, arquivo)) > 0) {    
    //             set_dados(packet, buffer);
    //         } else {
    //             fclose(arquivo);
    //         }            
    // }

    // free(buffer);
    return NULL;
}

kermit_protocol_state * tamanho_client(unsigned char * resposta, void * dados, int socket) {
    // kermit_protocol_state * next_state = NULL;
    // unsigned char * packet;
    // unsigned char * buffer;
    // size_t bytesLidos;
    // FILE* arquivo = (FILE *)dados;

    // if(!(buffer = malloc(sizeof(char) * 255))) {
    //     fclose(arquivo);
    //     return NULL;
    // }
    // switch (get_tipo_pacote(resposta)) {
    //     case OK:            
    //         if((bytesLidos = fread(buffer, 1, 63, arquivo)) > 0) {
    //             set_dados(packet, buffer);
    //         }
    // }

    // free(buffer);
    return NULL;
}

void backup_client(FILE *dados, char *nome_arq, int socket) {
    FILE* arquivo = dados;
    unsigned char *recebido = NULL, *enviado = NULL;
    uint64_t tamanho = 0;

    // cria um pacote com o nome do arquivo no campo de dados
    enviado = inicializa_pacote(BACKUP, 0, (unsigned char *)nome_arq);
    recebido = stop_n_wait(enviado, socket); // Envia e espera o pacote OK

    destroi_pacote(enviado);

    // pega o tamanho do arquivo
    fseek(arquivo, 0, SEEK_END);
    tamanho = ftell(arquivo);

    switch (get_tipo_pacote(recebido)) {
        case OK:
            #ifdef DEBUG
                printf("Sinal OK recebido do servidor\n");
            #endif
            enviado = inicializa_pacote(TAMANHO, 0, (unsigned char *)&tamanho);
            
            // Tamanho do campo de dados será de 8 bytes
            destroi_pacote(recebido);
            recebido = stop_n_wait(enviado, socket); // Envia e espera o pacote ack
            destroi_pacote(enviado);
            destroi_pacote(recebido);
            #ifdef DEBUG
                printf("Pacote recebido\n");
            #endif
            break;
        default:
            return;
    }
}

int ler_entrada(char * buffer) {
    if (fgets(buffer, 255, stdin) == NULL) {
        return -1;
    }
    buffer[strcspn(buffer, "\n")] = 0;
    return 0;
}

int client(int socket) {
    char *buffer;
    FILE * arquivo;
    if(!(buffer = malloc(sizeof(char) * 255))) {
        return -1;
    }

    int comando = -1;
    int executar = 1;

    while(executar) {
        printf("Escolha o comando \n");
        printf("[1] Backup  [2] Restaura    [3] Verifica \n");
        printf("[0] Sair\n");
        if (scanf("%d", &comando) == EOF) {
            fprintf(stderr, "Erro ao ler a linha\n");
            break;
        }

        while (getchar() != '\n');  // só pra tirar o \n do buffer

        switch(comando) {
            case 1:
                printf("Insira o nome do arquivo: \n");
                ler_entrada(buffer);                

                if((arquivo = fopen(buffer, "r")) == NULL) {
                    break;
                }
                
                backup_client(arquivo, buffer, socket);
                break;
            case 2:
                printf("TODO Restaura, mano\n");
                break;
            case 3:
                printf("TODO Verifica, mano\n");
                break;
            default:
                executar = 0;
                break;
        }
        printf("executar: %d\n", executar);
    }

    free(buffer);
    return 0;
}

