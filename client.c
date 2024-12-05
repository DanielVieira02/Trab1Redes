#define _POSIX_C_SOURCE 200809L // para usar a função strdup

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para a função strlen()
#include <sys/types.h>
#include <sys/socket.h>

#include "kermit.h"

kermit_protocol_state * fim_dados_client(unsigned char * resposta, void * dados, int socket);

/// @brief Função que inicia o envio de dados
/// @param arquivo Arquivo que será enviado
/// @param socket Socket que será utilizado
/// @return Retorna 1 se o envio foi realizado com sucesso, 0 caso contrário
int inicia_envio_dados(FILE *arquivo, uint64_t tamanho, int socket);
kermit_protocol_state * tamanho_client(unsigned char * resposta, void * dados, int socket);

/// @brief Função que realiza o backup do arquivo
/// @param arquivo Descritor do arquivo que será enviado
/// @param nome_arq Nome do arquivo
/// @param socket Socket que será utilizado
/// @return Retorna 1 se o backup foi realizado com sucesso, 0 caso contrário
int backup_client(FILE* arquivo, char * nome_arq, int socket);

/// @brief Função que restaura o arquivo presente no servidor
/// @param nomeArq Nome do arquivo
/// @param socket Socket que será utilizado
/// @return Retorna 1 se o arquivo foi restaurado com sucesso, 0 caso contrário
int restaura_client(char *nomeArq, int socket);

kermit_protocol_state * fim_dados_client(unsigned char * resposta, void * dados, int socket) {
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

int backup_client(FILE *dados, char *nome_arq, int socket) {
    FILE* arquivo = dados;
    unsigned char *recebido = NULL, *enviado = NULL;
    uint64_t tamanho = 0;
    int tipo = 0;

    #ifdef DEBUG
        printf("Começando o backup\n");
    #endif

    // cria um pacote com o nome do arquivo no campo de dados
    if(!(enviado = inicializa_pacote(BACKUP, 0, (unsigned char *)nome_arq, strnlen(nome_arq, TAM_CAMPO_DADOS- 2) + 1))) {
        fprintf(stderr, "Erro ao inicializar o pacote\n");
        return 0;
    }

    recebido = stop_n_wait(enviado, socket); // Envia e espera o pacote OK
    while(get_tipo_pacote(recebido) != OK) {
        recebido = destroi_pacote(recebido);    
        recebido = stop_n_wait(enviado, socket);
    }

    tipo = get_tipo_pacote(recebido);

    enviado = destroi_pacote(enviado);
    recebido = destroi_pacote(recebido);
    // pega o tamanho do arquivo
    fseeko(arquivo, 0, SEEK_END);
    tamanho = ftello(arquivo);

    rewind(arquivo); // volta o ponteiro pro inicio do arquivo
    switch (tipo) {
        case OK:
            #ifdef DEBUG
                printf("Sinal OK recebido do servidor\n");
                printf("Enviando o tamanho do arquivo\n");
            #endif
            if(!(enviado = inicializa_pacote(TAMANHO, 0, &tamanho, sizeof(uint64_t)))) {
                fprintf(stderr, "Erro ao inicializar o pacote\n");
                return 0;
            }
            #ifdef DEBUG
                // testar get_dados_pacote(enviado), que deve retornar o tamanho do arquivo
                unsigned char * teste = get_dados_pacote(enviado);
                printf("Dados do pacote enviado: %ls\n", (unsigned int *)teste);
                free(teste);
            #endif
            // Envia e espera o pacote ok
            if(!(recebido = stop_n_wait(enviado, socket))){
                fprintf(stderr, "Erro ao enviar o tamanho do arquivo\n");
                return 0;
            }

            enviado = destroi_pacote(enviado);

            // enquanto o pacote recebido não for do tipo correto
            #ifdef DEBUG
                printf("Verificando se o tipo do pacote é OK\n");
            #endif

            tipo = get_tipo_pacote(recebido);
            while(tipo == NACK || tipo != OK) {
                #ifdef DEBUG
                    printf("Pacote recebido não é do tipo correto\n");
                #endif
                if(tipo == ERRO) {
                    // seria necessario aqui uma função que printa a mensagem de erro respectiva pra cada codigo
                    fprintf(stderr, "backup_client: erro específico do servidor\n");
                    enviado = destroi_pacote(enviado);
                    break;
                }
                destroi_pacote(recebido);
                enviado = destroi_pacote(enviado);
                recebido = stop_n_wait(enviado, socket);

            }

            enviado = destroi_pacote(enviado);
            recebido = destroi_pacote(recebido);
            // recebido o tamanho, começa o fluxo de dados
            if(!envia_fluxo_dados(arquivo, tamanho, socket)) {
                fprintf(stderr, "Erro ao enviar os dados\n");
                return 0;
            }
            #ifdef DEBUG
                printf("Pacote recebido\n");
            #endif
            break;
        default:
            return 0;
    }

    #ifdef DEBUG
        printf("Backup completo.\n");
    #endif

    return 1;
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
    if(!(buffer = calloc(sizeof(char) * 255, 1))) {
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

        printf("Insira o nome do arquivo: \n");
        ler_entrada(buffer);                

        switch(comando) {
            case 1:
                if((arquivo = fopen(buffer, "r")) == NULL) {
                    perror("Erro ao abrir o arquivo");
                    break;
                }
                
                if(!backup_client(arquivo, buffer, socket)){
                    fprintf(stderr, "Erro ao realizar o backup\n");
                } else {
                    printf("Backup realizado com sucesso\n");
                }

                fclose(arquivo);
                executar = 0;
                break;
            case 2:
                if(!restaura_client(buffer, socket)){
                    fprintf(stderr, "Erro ao realizar o backup\n");
                } else {
                    printf("Backup realizado com sucesso\n");
                }
                // printf("TODO Restaura, mano\n");
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

int restaura_client(char *nomeArq, int socket) {
    FILE *arquivo = NULL;
    unsigned char *recebido = NULL, *enviado = NULL;

    #ifdef DEBUG
        printf("Iniciando a restauração do arquivo\n");
    #endif

    // envia um pacote pedindo para restaurar o arquivo
    if(!(enviado = inicializa_pacote(RESTAURA, 0, (unsigned char *)nomeArq, strnlen(nomeArq, TAM_CAMPO_DADOS - 2) + 1))) {
        fprintf(stderr, "Erro ao inicializar o pacote\n");  
        return 0;
    }

    // Envia e espera o pacote OK_TAMANHO
    do {
        recebido = stop_n_wait(enviado, socket);
    } while(get_tipo_pacote(recebido) != OK_TAMANHO);

    enviado = destroi_pacote(enviado);

    if(!ha_memoria_suficiente(*(uint64_t *)get_dados_pacote(recebido))) {
        destroi_pacote(recebido);
        if(!(enviado = inicializa_pacote(ERRO, 0, (char *) MSG_ERR_ESPACO, 0)))
            return 0;

        if(envia_pacote(enviado, socket) < 0) {
            perror("restaura_client: Erro ao enviar o pacote de erro\n");
            destroi_pacote(enviado);
            return 0;
        }

        fprintf(stderr, "Erro: Memória insuficiente\n");
        destroi_pacote(enviado);
        return 0;
    }

    destroi_pacote(recebido);

    // Tenta abrir o arquivo
    if((arquivo = fopen(nomeArq, "w")) == NULL) {
        perror("Erro ao abrir o arquivo");
        return 0;
    }

    // Envia o pacote OK
    if(!(enviado = inicializa_pacote(OK, 0, NULL, 0))) {
        fprintf(stderr, "Erro ao inicializar o pacote\n");
        return 0;
    }

    if(envia_pacote(enviado, socket) < 0) {
        fprintf(stderr, "Erro ao enviar o pacote\n");
        return 0;
    }

    // Inicia o recebimento dos dados
    if(!recebe_fluxo_dados(arquivo, socket)) {
        fprintf(stderr, "restaura_client: Erro ao receber os dados\n");
        return 0;
    }

    return 1;
}