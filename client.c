#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para a função strlen()
#include <sys/types.h>
#include <sys/socket.h>

#include "ConexaoRawSocket.h"
#include "kermit.h"

void tamanho(FILE * arquivo, int socket) {
    fseek(arquivo, 0, fseek);

    kermit_packet * pacote = inicializa_pacote(TAMANHO, 1);
    insere_dados_pacote(pacote, (char)ftell(arquivo), 63);
    kermit_packet * resposta = envia_pacote(pacote);
    destroi_pacote(pacote);

    if (resposta == NULL) {
        return;
    }

    switch (get_tipo_pacote(resposta)) {
        case OK:
            dados(arquivo, socket);
            break;
        case ERRO:
            printf("Erro de acesso\n");
        default:
            return;
    }
}

void dados(FILE * arquivo, int socket) {
    char *buffer;
    if(!(buffer = malloc(sizeof(char) * 255))) {
        return -1;
    }
    int sequencia = 2;
    kermit_packet * pacote;
    
    size_t bytesLidos;

    while((bytesLidos = fread(buffer, 1, 63, arquivo)) > 0) {
        pacote = inicializa_pacote(DADOS, ++sequencia);
        insere_dados_pacote(pacote, buffer, bytesLidos);
        envia_pacote(pacote, socket);
        pacote = destroi_pacote(pacote);
    }

    fclose(arquivo);

    fim_dados();
}

void fim_dados(int socket, int sequencia) {
    kermit_packet * pacote = inicializa_pacote(FIM_DADOS, sequencia);
    envia_pacote(pacote, socket);
    pacote = destroi_pacote(pacote);
}

void backup(char * nome_arquivo, int socket) {    
    FILE * arquivo = fopen(nome_arquivo, "rb");
    if(arquivo == NULL) {
        printf("Erro ao ler arquivo. Encerrando execução\n");
        return;
    }

    kermit_packet * pacote = inicializa_pacote(BACKUP, 0);
    insere_dados_pacote(pacote, nome_arquivo, strlen(nome_arquivo));
    kermit_packet * resposta = envia_pacote(pacote, socket);
    destroi_pacote(pacote);

    if (resposta == NULL) {
        return;
    }

    switch (get_tipo_pacote(resposta)) {
        case OK:
            tamanho(arquivo, socket);
            break;
        case ERRO:
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
                /*if(envia_arquivo(buffer, socket) == -1) {
                    break;
                }
                ler_entrada(buffer);*/
                backup(buffer, socket);
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

