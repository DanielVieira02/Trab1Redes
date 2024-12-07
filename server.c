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
int backup(unsigned char * packet, int socket);

/// @brief Restaura o arquivo presente no servidor no cliente
/// @param nomeArq nome do arquivo
/// @param socket socket que será utilizado
/// @return 1 se o arquivo foi restaurado corretamente, 0 caso contrário
int restaura_server(char *nomeArq, int socket);

int backup(unsigned char * packet, int socket) {
    char * nome_arquivo = (char *) get_dados_pacote(packet);
    FILE * arquivo = fopen(nome_arquivo, "w");
    free(nome_arquivo);
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
        if((cria_envia_pck(ERRO, 0, (char *) MSG_ERR_ESPACO, socket, 0)) < 0) 
            fprintf(stderr, "Erro ao enviar o pacote de erro\n");

        fprintf(stderr, "Erro: Memória insuficiente\n");
        return UINT64_MAX;
    }

    #ifdef DEBUG
        printf("Espaço suficiente\n");
    #endif
    cria_envia_pck(OK, 0, NULL, socket, 0);
    // liberando o pacote
    destroi_pacote(recebido_cliente);
    return data;
}

void trata_pacote(unsigned char * packet, int socket) {
    switch (get_tipo_pacote(packet)) {
    case BACKUP:
        if(!backup(packet, socket)) 
            fprintf(stderr, "Erro ao realizar o backup\n");
        else 
            printf("Backup completo.\n");
        
        break;
    case RESTAURA:
        char * nome_arquivo = (char *) get_dados_pacote(packet);
        if(!(restaura_server(nome_arquivo, socket))) 
            fprintf(stderr, "Erro ao realizar o backup\n");
        else 
            printf("Backup realizado com sucesso\n");
        free(nome_arquivo);
        break;
    case VERIFICA:
        break;
    default:
        break;
    }
}

int server(int socket) {
    unsigned char * packet = NULL;

    while(1) {
        // fica esperando pacotes
        #ifdef DEBUG
            printf("Esperando resposta do cliente... \n");
        #endif
        while(!(packet = recebe_pacote(socket)));

        trata_pacote(packet, socket);
    }
    return 0;
}

int restaura_server(char *nomeArq, int socket) {
    FILE *arquivo = NULL;
    unsigned char *recebido = NULL, *enviado = NULL;
    uint64_t tamanho = 0;

    if((arquivo = fopen(nomeArq, "r")) == NULL) {
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
    return 0;
}
