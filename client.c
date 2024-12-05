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

    // cria um pacote com o nome do arquivo no campo de dados
    enviado = inicializa_pacote(BACKUP, 0, (unsigned char *)nome_arq, strnlen(nome_arq, TAM_CAMPO_DADOS) + 1);
    recebido = stop_n_wait(enviado, socket); // Envia e espera o pacote OK

    tipo = get_tipo_pacote(recebido);
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
                    destroi_pacote(enviado);
                    break;
                }
                destroi_pacote(recebido);
                recebido = stop_n_wait(enviado, socket);
            }

            destroi_pacote(recebido);
            // recebido o tamanho, começa o fluxo de dados
            if(!inicia_envio_dados(arquivo, tamanho, socket)) {
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

        switch(comando) {
            case 1:
                printf("Insira o nome do arquivo: \n");
                ler_entrada(buffer);                

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

int inicia_envio_dados(FILE * arquivo, uint64_t tamanho, int socket) {
    unsigned char * recebido = NULL, * enviado = NULL, * buffer = NULL;
    uint64_t chunks = tamanho / MAX_DADOS; // 63 bytes por pacote
    buffer = calloc(1, MAX_DADOS);

    // Envia o pacote de dados
    for(int seq = 0; seq < chunks; seq++) {
        #ifdef DEBUG
            printf("Enviando o pacote %u\n", seq);
        #endif
        // lê 64 bytes do arquivo
        if(fread(buffer, 1, MAX_DADOS, arquivo) == 0){
            fprintf(stderr, "Erro ao ler o arquivo\n");
            free(buffer);
            return 0;    
        }
        // cria o pacote de dados
        if(!(enviado = inicializa_pacote(DADOS, seq % (1 << 5), buffer, MAX_DADOS))) {
            fprintf(stderr, "Erro ao inicializar o pacote\n");
            free(buffer);
            return 0;
        }

        recebido = stop_n_wait(enviado, socket); // Envia e espera o pacote ACK

        // enquanto der erro no CRC, timeout ou o pacote recebido não for do tipo correto
        while(recebido == NULL || get_tipo_pacote(recebido) != ACK) {
            enviado = inicializa_pacote(DADOS, seq, buffer, MAX_DADOS);
            recebido = stop_n_wait(enviado, socket);
        }

        // destroi os pacotes
        destroi_pacote(recebido);
    }

    // escreve o resto dos dados
    if(tamanho % MAX_DADOS != 0) {
        #ifdef DEBUG
            printf("Resto dos dados\n");
        #endif
        if(!fread(buffer, 1, tamanho % MAX_DADOS, arquivo)){
            fprintf(stderr, "Erro ao ler o arquivo\n");
            return 0;
        }

        if(!(enviado = inicializa_pacote(DADOS, chunks % (1 << 5), buffer, tamanho % MAX_DADOS))) {
            fprintf(stderr, "Erro ao inicializar o pacote\n");
            destroi_pacote(enviado);
            free(buffer);
            return 0;
        }

        recebido = stop_n_wait(enviado, socket);

        while(recebido == NULL || get_tipo_pacote(recebido) != ACK) {
            enviado = inicializa_pacote(DADOS, chunks, buffer, tamanho % 64);
            recebido = stop_n_wait(enviado, socket);
        }

        destroi_pacote(recebido);
    }

    // envia um ultimo pacote vazio
    enviado = inicializa_pacote(FIM_DADOS, 0, NULL, 0);

    while((envia_pacote(&enviado, socket)) < 0) {
        fprintf(stderr, "Erro ao enviar o último pacote, enviando novamente.\n");
    }

    return 1;
}