#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para a função strlen()
#include <sys/types.h>
#include <sys/socket.h>

#include "ConexaoRawSocket.h"
#include "kermit.h"

kermit_protocol_state * fim_dados_client(kermit_packet * resposta, void * dados, int socket);
kermit_protocol_state * dados_client(kermit_packet * resposta, void * dados, int socket);
kermit_protocol_state * tamanho_client(kermit_packet * resposta, void * dados, int socket);
kermit_protocol_state * backup_client(kermit_packet * resposta, void * dados, int socket);

kermit_protocol_state * fim_dados_client(kermit_packet * resposta, void * dados, int socket) {
    return NULL;
}

kermit_protocol_state * dados_client(kermit_packet * resposta, void * dados, int socket) {
    kermit_protocol_state * next_state = NULL;
    kermit_packet * packet;
    char* buffer;
    size_t bytesLidos;
    FILE* arquivo = (FILE *)dados;

    if(!(buffer = malloc(sizeof(char) * 255))) {
        fclose(arquivo);
        return NULL;
    }
    switch (get_tipo_pacote(resposta)) {
        case ACK:            
            if((bytesLidos = fread(buffer, 1, 63, arquivo)) > 0) {
                packet = inicializa_pacote(DADOS, 2);
                insere_dados_pacote(packet, buffer, bytesLidos);
                next_state = cria_estrutura_estado(packet, dados_client, socket);
                define_parametros_procedimento_estado(next_state, (void *)arquivo);
            } else {
                packet = inicializa_pacote(FIM_DADOS, 2);
                next_state = cria_estrutura_estado(packet, fim_dados_client, socket);
                fclose(arquivo);
            }            
    }

    free(buffer);
    return next_state;
}

kermit_protocol_state * tamanho_client(kermit_packet * resposta, void * dados, int socket) {
    kermit_protocol_state * next_state = NULL;
    kermit_packet * packet;
    char* buffer;
    size_t bytesLidos;
    FILE* arquivo = (FILE *)dados;

    if(!(buffer = malloc(sizeof(char) * 255))) {
        fclose(arquivo);
        return NULL;
    }
    switch (get_tipo_pacote(resposta)) {
        case OK:            
            if((bytesLidos = fread(buffer, 1, 63, arquivo)) > 0) {
                packet = inicializa_pacote(DADOS, 2);
                insere_dados_pacote(packet, buffer, bytesLidos);
            }
            next_state = cria_estrutura_estado(packet, dados_client, socket);
            define_parametros_procedimento_estado(next_state, (void *)arquivo);
            
    }

    free(buffer);
    return next_state;
}

kermit_protocol_state * backup_client(kermit_packet * resposta, void * dados, int socket) {
    kermit_protocol_state * next_state;
    FILE* arquivo = (FILE *)dados;
    switch (get_tipo_pacote(resposta)) {
        case OK:            
            kermit_packet * packet = inicializa_pacote(TAMANHO, 1); // Inicializa pacote para ser enviado no próximo estado

            fseek(arquivo, 0, SEEK_END); // Tamanho do arquivo
            insere_dados_pacote(packet, (char *)ftell(arquivo), 63);
            next_state = cria_estrutura_estado(packet, tamanho_client, socket);
            define_parametros_procedimento_estado(next_state, dados);
            break;
        default:
            return NULL;
    }

    return next_state;
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
    FILE * arquivo;
    kermit_packet * packet;
    kermit_protocol_state * initial_state;
    if(!(buffer = malloc(sizeof(char) * 255))) {
        return -1;
    }

    int comando = -1;
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

        

        switch(comando) {
            case 1:
                printf("Insira o nome do arquivo: \n");
                ler_entrada(buffer);                

                if((arquivo = fopen(buffer, "r")) == NULL) {
                    break;
                }                // se o arquivo existe, cria um pacote e troca o estado dele
                packet = inicializa_pacote(BACKUP, 0);
                initial_state = cria_estrutura_estado(packet, backup_client, socket);
                define_parametros_procedimento_estado(initial_state, (void *)arquivo);
                invoca_estado(initial_state);
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
        
    }

    free(buffer);
    return 0;
}

