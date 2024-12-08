#include "kermit.h"

/// @brief Inicia o fluxo de dados, recebendo os dados do pacote e escrevendo no arquivo 
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return retorna 1 se o fluxo de dados foi realizado com sucesso, 0 caso contrário
int inicia_fluxo_dados(FILE * arquivo, int socket);

/// @brief Recebe o tamanho do arquivo e verifica se há espaço suficiente para armazená-lo
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return retorna o tamanho se houver espaço suficiente, UINT64_MAX caso contrário
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
        if(errno == ENOENT) {
            fprintf(stderr, "Arquivo não encontrado\n");
        } else if(errno == EACCES) {
            fprintf(stderr, "Permissão negada\n");
        } else {
            fprintf(stderr, "Erro ao acessar o arquivo\n");
        }

        if(cria_envia_pck(ERRO, &errno, socket, sizeof(int)) < 0) {
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
    unsigned char * pacote = NULL;
    uint64_t tamanho = 0;

    // Verifica se o arquivo foi aberto corretamente
    if (arquivo != NULL) {
        #ifdef DEBUG
            printf("Arquivo aberto com sucesso\n");
        #endif
    } else {
        pacote = inicializa_pacote(ERRO, (unsigned char *) MSG_ERR_ACESSO, 1);
        insere_dados_pacote(pacote, (char *) MSG_ERR_ACESSO, 1);
        envia_pacote(pacote, socket);
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
    uint64_t data = 0;
    unsigned char * recebido_cliente = NULL;

    // espera o cliente enviar o tamanho do arquivo
    while((recebido_cliente = stop_n_wait(inicializa_pacote(OK, NULL, 0), socket)) == NULL);

    // Enquanto o pacote recebido não é do tipo correto
    while(get_tipo_pacote(recebido_cliente) != TAMANHO){
        envia_nack(socket);
        destroi_pacote(recebido_cliente);
        recebido_cliente = recebe_pacote(socket);
    }

    // este conversor de tamanho deu uma dor de cabeça...
    // mas é devido ao tamanho diferente de bytes que pode vir do cliente
    uint64_t * data_ptr = get_dados_pacote(recebido_cliente);
    data = *data_ptr;
    free(data_ptr);

    #ifdef DEBUG
        printf("\nRecebido o arquivo de tamanho %lu\n", data);
    #endif

    if(!ha_memoria_suficiente(data)) {
        destroi_pacote(recebido_cliente);
        if((cria_envia_pck(ERRO, (char *) MSG_ERR_ESPACO, socket, 0)) < 0) 
            fprintf(stderr, "Erro ao enviar o pacote de erro\n");

        fprintf(stderr, "Erro: Memória insuficiente\n");
        return UINT64_MAX;
    }

    #ifdef DEBUG
        printf("Espaço suficiente\n");
    #endif
    cria_envia_pck(OK, NULL, socket, 0);
    // liberando o pacote
    destroi_pacote(recebido_cliente);
    return data;
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
        fprintf(stderr, "trata_pacote: requisição do tipo incorreta.\n");
        break;

        if(nome_arquivo) free(nome_arquivo);
    }
}

int server(int socket) {
    unsigned char * packet = NULL;

    while(1) {
        SEQUENCIA_ENVIA = 0;
        SEQUENCIA_RECEBE = 0;
        // fica esperando pacotes
        #ifdef DEBUG
            printf("Esperando resposta do cliente... \n");
        #endif
        while(!(packet = recebe_pacote(socket)));

        trata_pacote(packet, socket);

        packet = destroi_pacote(packet);
    }
    return 0;
}

int restaura_server(char *nome_arq, int socket) {
    FILE *arquivo = NULL;
    unsigned char *recebido = NULL, *enviado = NULL;
    uint64_t tamanho = 0;

    if((arquivo = fopen(nome_arq, "r")) == NULL) {
        perror("Erro ao abrir o arquivo");
        return 0;
    }

    // vai ate o final do arquivo, e pega o tamanho
    fseeko(arquivo, 0, SEEK_END);
    tamanho = ftello(arquivo);
    rewind(arquivo);

    #ifdef DEBUG
        printf("Tamanho do arquivo: %lu\n", tamanho);
        printf("Enviando o ok + tamanho do arquivo\n");
    #endif

    // cria um pacote com o tamanho do arquivo no campo de dados
    if(!(enviado = inicializa_pacote(OK_TAMANHO, &tamanho, sizeof(uint64_t)))) {
        fprintf(stderr, "Erro ao inicializar o pacote\n");
        return 0;
    }

    // enquanto não for um ok, envia o mesmo pacote
    recebido = stop_n_wait(enviado, socket);
    while (get_tipo_pacote(recebido) != OK) {
        recebido = destroi_pacote(recebido);
        recebido = stop_n_wait(enviado, socket);
    }

    // destroi o pacote enviado e o recebido
    destroi_pacote(enviado);
    destroi_pacote(recebido);

    // se for um ok, envia o arquivo
    if(!envia_fluxo_dados(arquivo, tamanho, socket)) {
        fprintf(stderr, "Erro ao enviar o arquivo\n");
        return 0;
    }

    fclose(arquivo);
    return 1;
}
