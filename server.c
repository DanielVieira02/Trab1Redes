#include "kermit.h"

/// @brief Inicia o fluxo de dados, recebendo os dados do pacote e escrevendo no arquivo 
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return retorna 1 se o fluxo de dados foi realizado com sucesso, 0 caso contrário
int inicia_fluxo_dados(FILE * arquivo, int socket);

/// @brief Recebe o tamanho do arquivo e verifica se há espaço suficiente para armazená-lo
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return retorna o tamanho se houver espaço suficiente, 0 caso contrário
uint64_t recebe_tamanho(int socket);

/// @brief Realiza o backup do arquivo
/// @param packet pacote com o nome do arquivo
/// @param socket socket que será utilizado
/// @return 1 se o arquivo foi salvo corretamente, 0 caso contrário
int backup(char * nome_arq, int socket);

/// @brief Restaura o arquivo presente no servidor no cliente
/// @param nome_arq nome do arquivo
/// @param socket socket que será utilizado
/// @return 1 se o arquivo foi restaurado corretamente, 0 caso contrário
int restaura_server(char *nome_arq, int socket);

/// @brief Manda o checksum do arquivo para o cliente
/// @param nome_arq nome do arquivo
/// @param socket socket que será utilizado
/// @return 1 se o checksum foi enviado corretamente, 0 caso contrário
unsigned int verifica_server(char *nome_arq, int socket);

unsigned int verifica_server(char *nome_arq, int socket) {
    unsigned int checksum = 0;

    // verifica se o arquivo é acessível
    if(access(nome_arq, F_OK) == -1) {
        perror("Erro ao acessar o arquivo");
		if(errno == EACCES)
			errno = MSG_ERR_ACESSO;
		else if(errno == ENOENT)
			errno = MSG_ERR_NAO_ENCONTRADO;

		if(cria_envia_pck(ERRO, &errno, socket, sizeof(int)) < 0){
            fprintf(stderr, "Erro ao enviar o pacote de erro\n");
        }
        return 0;
    }

    // realiza a checksum do arquivo
    if((checksum = realiza_checksum(nome_arq)) == 0){
        fprintf(stderr, "Erro ao calcular o checksum do arquivo.\n");
    }

    if(cria_envia_pck(OK_CHECKSUM, &checksum, socket, sizeof(int)) < 0) {
        fprintf(stderr, "Erro ao enviar o checksum do arquivo.\n");
    }

    return 1;
}

int backup(char * nome_arq, int socket) {
    FILE * arquivo = fopen(nome_arq, "w");
    uint64_t tamanho = 0;

    // Verifica se o arquivo foi aberto corretamente
    if (access(nome_arq, F_OK) == -1) {
        perror("Erro ao abrir o arquivo");
		if(errno == EACCES)
			errno = MSG_ERR_ACESSO;
		else if(errno == ENOENT)
			errno = MSG_ERR_NAO_ENCONTRADO;

		if(cria_envia_pck(ERRO, &errno, socket, 0) < 0) {
            fprintf(stderr, "Erro ao enviar o pacote de erro\n");
        }
        return 0;
    }

    #ifdef DEBUG
        printf("Pacote enviado\n");
    #endif
    // Se o arquivo foi aberto corretamente, inicia o fluxo de dados
    if (arquivo != NULL) {
        while((tamanho = recebe_tamanho(socket)) == UINT64_MAX){
            fprintf(stderr, "server_backup: Erro ao receber o tamanho do arquivo\n");
        }
        if(!recebe_fluxo_dados(arquivo, socket)) {
            fprintf(stderr, "server_backup: Erro durante o fluxo de dados\n");
            return 0;
        }
    }

    fclose(arquivo);
    return 1;
}

uint64_t recebe_tamanho(int socket) {
    uint64_t tamanho_arquivo = 0;
    unsigned char * recebido_cliente = NULL;

    // envia um ok e espera o cliente enviar o tamanho do arquivo
    while((recebido_cliente = cria_stop_wait(OK, NULL, 0, TAMANHO, socket)) == NULL) {
        return 0;
    }

    // recebe o tamanho 
    tamanho_arquivo = *(uint64_t *)get_dados_pacote(recebido_cliente); 
    recebido_cliente = destroi_pacote(recebido_cliente);

    #ifdef DEBUG
        printf("\nRecebido o arquivo de tamanho_arquivo %lu\n", tamanho_arquivo);
    #endif

   if(!testa_memoria(tamanho_arquivo, socket)) return 0;

    #ifdef DEBUG
        printf("Espaço suficiente\n");
    #endif

    return tamanho_arquivo;
}

void trata_pacote(unsigned char * packet, int socket) {
    char * nome_arquivo = NULL;
    switch (get_tipo_pacote(packet)) {
    case BACKUP:
        nome_arquivo = (char *) get_dados_pacote(packet);
        if(!backup(nome_arquivo, socket)) 
            fprintf(stderr, "Erro ao realizar o backup\n");
        else 
            printf("Backup completo.\n");
        break;
    case RESTAURA:
        nome_arquivo = (char *) get_dados_pacote(packet);
        if(!restaura_server(nome_arquivo, socket)) 
            fprintf(stderr, "Erro ao realizar a restauração\n");
        else 
            printf("Restauração realizada com sucesso\n");
        break;
    case VERIFICA:
        nome_arquivo = (char *) get_dados_pacote(packet);
        if(!verifica_server(nome_arquivo, socket)) 
            fprintf(stderr, "Erro ao realizar a verificação\n");
        else
            printf("Verificação realizada com sucesso.\n");
        break;
    default:
        fprintf(stderr, "trata_pacote: requisição não é do tipo esperado.\n");
        envia_nack(socket);
        break;

        if(nome_arquivo) free(nome_arquivo);
    }

    return;
}

int server(int socket) {
    unsigned char * packet = (unsigned char *) calloc(TAM_MAX_PACOTE_SUJO, 1);

    while(1) {
        SEQUENCIA_ENVIA = 0;
        SEQUENCIA_RECEBE = 0;
        // fica esperando pacotes
        printf("Esperando resposta do cliente... \n");
        if(!(packet = espera_pacote(socket, REQUISICAO_CLIENT, 0))) {
            fprintf(stderr, "server: Erro ao esperar pacote\n");
        }

        trata_pacote(packet, socket);

        packet = destroi_pacote(packet);
    }
    return 0;
}

int restaura_server(char *nome_arq, int socket) {
    FILE *arquivo = NULL;
    unsigned char *recebido = NULL;
    uint64_t tamanho = 0, stts = 0;

    if((stts = testa_arquivo(nome_arq, socket)) != 0) {
        return 0;
    }


    tamanho = get_tamanho_arquivo(nome_arq);

    #ifdef DEBUG
        printf("Tamanho do arquivo: %lu\n", tamanho);
        printf("Enviando o ok + tamanho do arquivo\n");
    #endif

    // cria um pacote com o tamanho do arquivo no campo de dados
    // enquanto não for um ok, envia o mesmo pacote
    if(!(recebido = cria_stop_wait(OK_TAMANHO, &tamanho, sizeof(uint64_t), OK, socket))) {
        fprintf(stderr, "Erro ao enviar o tamanho do arquivo\n");
        if(recebido) recebido = destroi_pacote(recebido);
        return 0;
    } else if (get_tipo_pacote(recebido) == ERRO) {
        imprime_erro(recebido);
        fprintf(stderr, "no cliente\n");
        recebido = destroi_pacote(recebido);
        return 0; 
    }

    // destroi o pacote recebido
    recebido = destroi_pacote(recebido);

    if(!(arquivo = fopen(nome_arq, "r"))) {
		perror("Erro ao abrir o arquivo");
		return 0;
	}

    // se for um ok, envia o arquivo
    if(!envia_fluxo_dados(arquivo, tamanho, socket)) {
        fprintf(stderr, "Erro ao enviar o arquivo\n");
        return 0;
    }

    fclose(arquivo);
    return 1;
}
