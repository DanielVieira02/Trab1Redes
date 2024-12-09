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
/// @param nome_arq Nome do arquivo
/// @param socket Socket que será utilizado
/// @return Retorna 1 se o arquivo foi restaurado com sucesso, 0 caso contrário
int restaura_client(char *nome_arq, int socket);

/// @brief Função que compara os checksums do arquivo no client e no servidor
/// @param nome_arq Nome do arquivo
/// @param socket Socket que será utilizado
/// @return Retorna 1 se o checksum foi realizado com sucesso, 0 caso contrário
int verifica_client(char *nome_arq, int socket);

int verifica_client(char *nome_arq, int socket) {
    unsigned char *recebido = NULL;
    unsigned int *checksum_server = NULL, checksum_local = 0, tam_nome_arq = 0;struct stat st;

    // Verifica se o arquivo existe, ou é acessível
    if(stat(nome_arq, &st) == -1) {
        fprintf(stderr, "Erro ao obter informações do arquivo: %s\n", strerror(errno));
        return 0;
    }


    // calcula o tamanho do nome do arquivo
    tam_nome_arq = strnlen(nome_arq, TAM_CAMPO_DADOS - 2) + 1;

    // Envia e espera o pacote OK + CHECKSUM
    if(!(recebido = cria_stop_wait(VERIFICA, nome_arq, tam_nome_arq, OK_CHECKSUM, socket))) {
        fprintf(stderr, "Erro ao enviar o nome do arquivo\n");
        if(recebido) recebido = destroi_pacote(recebido);
        return 0;
    } else if (get_tipo_pacote(recebido) == ERRO) {
        imprime_erro(recebido);
        fprintf(stderr, "no servidor\n");
        recebido = destroi_pacote(recebido);
        return 0; 
    }

    // Realiza o checksum do arquivo e indica se os arquivos são iguais ou não
    checksum_server = ((unsigned int *)get_dados_pacote(recebido));

    checksum_local = realiza_checksum(nome_arq);

    // Compara os checksums
    if (*checksum_server == checksum_local) {
        printf("Checksums iguais: Arquivos são idênticos.\n");
    } else {
        printf("Checksums diferentes: Arquivos são diferentes.\n");
    }

    // Libera o pacote
    if(recebido) recebido = destroi_pacote(recebido);
    free(checksum_server);
    return 1;
}

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
    unsigned char *recebido = NULL;
    uint64_t tamanho = 0, tam_nome_arq = 0;
    struct stat st;

    #ifdef DEBUG
        printf("Começando o backup\n");
    #endif

   // Verifica se o arquivo existe, ou é acessível
    if(stat(nome_arq, &st) == -1) {
        fprintf(stderr, "Erro ao obter informações do arquivo: %s\n", strerror(errno));
        return 0;
    }

    // calcula o tamanho do nome do arquivo
    tam_nome_arq = strnlen(nome_arq, TAM_CAMPO_DADOS - 2) + 1;

    // cria e envia um pacote com o nome do arquivo no campo de dados
    // espera um pacote de OK
    #ifdef DEBUG
        printf("tipo enviado = %d\n", BACKUP);
    #endif
    if(!(recebido = cria_stop_wait(BACKUP, nome_arq, tam_nome_arq, OK, socket))) {
        fprintf(stderr, "Erro ao enviar o nome do arquivo\n");
        if(recebido) recebido = destroi_pacote(recebido);
        return 0;
    } else if (recebido && get_tipo_pacote(recebido) == ERRO) {
        fprintf(stderr, "no servidor");
        recebido = destroi_pacote(recebido);
        return 0; 
    }

    recebido = destroi_pacote(recebido);

    // pega o tamanho do arquivo
    if(!(tamanho = get_tamanho_arquivo(nome_arq))){ 
        return 0;
    }

    #ifdef DEBUG
        printf("Sinal OK recebido do servidor\n");
        printf("Enviando o tamanho do arquivo\n");
    #endif

    // envia o tamanho e espera um pacote de OK
    if(!(recebido = cria_stop_wait(TAMANHO, &tamanho, sizeof(uint64_t), OK, socket))) {
        fprintf(stderr, "Erro ao enviar o tamanho do arquivo\n");
        if(recebido) recebido = destroi_pacote(recebido);
        return 0;
    } else if (get_tipo_pacote(recebido) == ERRO) {
        imprime_erro(recebido);
        fprintf(stderr, "no servidor\n");
        recebido = destroi_pacote(recebido);
        return 0; 
    }

    recebido = destroi_pacote(recebido);
    
    // recebido o tamanho, começa o fluxo de dados
    if(!envia_fluxo_dados(arquivo, tamanho, socket)) {
        fprintf(stderr, "Erro ao enviar os dados\n");
        return 0;
    }

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
        SEQUENCIA_ENVIA = 0;
        SEQUENCIA_RECEBE = 0;
        printf("Escolha o comando \n");
        printf("[1] Backup  [2] Restaura    [3] Verifica \n");
        printf("[0] Sair\n");
        if (scanf("%d", &comando) == EOF) {
            fprintf(stderr, "Erro ao ler a linha\n");
            break;
        }

        while (getchar() != '\n');  // só pra tirar o \n do buffer

        if(comando == 0) {
            if(buffer) free(buffer);
            return 0;
        }

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
                break;
            case 2:
                if(!restaura_client(buffer, socket)){
                    fprintf(stderr, "Erro ao realizar a restauração\n");
                } else {
                    printf("Restauração realizada com sucesso\n");
                }
                // printf("TODO Restaura, mano\n");
                break;
            case 3:
			    if(!verifica_client(buffer, socket))
				    fprintf(stderr, "Erro ao realizar a verificação\n");
			    else
				    printf("Verificação realizada com sucesso.\n");
			    break;
            case 0:
                printf("Saindo\n");
                executar = 0;
                break;
            default:
                executar = 0;
                break;
        }
    }

    free(buffer);
    return 0;
}

int restaura_client(char *nome_arq, int socket) {
    FILE *arquivo = NULL;
    unsigned char *recebido = NULL;
    uint64_t tam_nome_arq = 0, *tamanho_arquivo = NULL;

    #ifdef DEBUG
        printf("Iniciando a restauração do arquivo\n");
    #endif

    // calcula o tamanho do arquivo
    tam_nome_arq = strnlen(nome_arq, TAM_CAMPO_DADOS - 2) + 1;

    if(!(recebido = cria_stop_wait(RESTAURA, nome_arq, tam_nome_arq, OK_TAMANHO, socket))) {
        fprintf(stderr, "Erro ao enviar o nome do arquivo\n");
        if(recebido) recebido = destroi_pacote(recebido);
        return 0;
    } else if (get_tipo_pacote(recebido) == ERRO) {
        imprime_erro(recebido);
        fprintf(stderr, "no servidor\n");
        recebido = destroi_pacote(recebido);
        return 0; 
    }

    tamanho_arquivo = (uint64_t *)get_dados_pacote(recebido);

    recebido = destroi_pacote(recebido);
    
    if(testa_memoria(*tamanho_arquivo, socket) == 0) return 0;
    free(tamanho_arquivo);

    // Tenta abrir o arquivo
    if((arquivo = fopen(nome_arq, "w")) == NULL) {
        perror("Erro ao abrir o arquivo");
        return 0;
    }

    // Inicia o recebimento dos dados
    if(!recebe_fluxo_dados(arquivo, socket)) {
        fprintf(stderr, "restaura_client: Erro ao receber os dados\n");
        return 0;
    }

    fclose(arquivo);
    return 1;
}