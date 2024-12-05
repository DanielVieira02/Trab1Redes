#include "kermit.h"

/// @brief Inicia o fluxo de dados, recebendo os dados do pacote e escrevendo no arquivo 
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return sem retorno
int inicia_fluxo_dados(FILE * arquivo, int socket);

/// @brief Recebe o tamanho do arquivo e verifica se há espaço suficiente para armazená-lo
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return retorna o tamanho se houver espaço suficiente, -1 caso contrário
uint64_t recebe_tamanho(int socket);

/// @brief Realiza o backup do arquivo
/// @param packet pacote com o nome do arquivo
/// @param socket socket que será utilizado
/// @return 1 se o arquivo foi salvo corretamente, 0 caso contrário
int backup(unsigned char * packet, int socket);

int backup(unsigned char * packet, int socket) {
    FILE * arquivo = fopen((char *) "mensagem1", "w");
    unsigned char * pacote = NULL;
    uint64_t tamanho = 0;

    // Verifica se o arquivo foi aberto corretamente
    if (arquivo != NULL) {
        #ifdef DEBUG
            printf("Arquivo aberto com sucesso\n");
        #endif
        pacote = inicializa_pacote(OK, 0, NULL, 0);
    } else {
        pacote = inicializa_pacote(ERRO, 0, (unsigned char *) MSG_ERR_ACESSO, 1);
        insere_dados_pacote(pacote, (char *) MSG_ERR_ACESSO, 1);
    }
     
    if(envia_pacote(&pacote, socket) < 0) {
        fprintf(stderr, "server_backup: Erro ao enviar pacote\n");
        return 0;
    }

    #ifdef DEBUG
        printf("Pacote enviado\n");
    #endif
    // Se o arquivo foi aberto corretamente, inicia o fluxo de dados
    if (arquivo != NULL) {
        while((tamanho = recebe_tamanho(socket)) < 0){
            fprintf(stderr, "server_backup: Erro ao receber o tamanho do arquivo\n");
        }
        if(inicia_fluxo_dados(arquivo, socket)) {
            fprintf(stderr, "server_backup: Erro durante o fluxo de dados\n");
            return 0;
        }
        return 1;
    }
    return 0;
}

uint64_t recebe_tamanho(int socket) {
    uint64_t data = 0, espaco = 0;
    unsigned char * recebido_cliente = NULL;
    void * raw_data = NULL;
    struct statvfs fs;

    // espera o cliente enviar o tamanho do arquivo
    while((recebido_cliente = recebe_pacote(socket)) == NULL);

    // Enquanto o pacote recebido não é do tipo correto
    while(get_tipo_pacote(recebido_cliente) != TAMANHO){
        envia_nack(socket);
        destroi_pacote(recebido_cliente);
        recebido_cliente = recebe_pacote(socket);
    }

    // este conversor de tamanho deu uma dor de cabeça...
    // mas é devido ao tamanho diferente de bytes que pode vir do cliente
    raw_data = get_dados_pacote(recebido_cliente);
	for(int i = 0; i < get_tamanho_pacote(recebido_cliente); ++i) {
		data |= ((uint64_t)((uint8_t *)raw_data)[i] << (i * 8));
	}

    #ifdef DEBUG
        printf("\nRecebido o arquivo de tamanho %lu\n", data);
    #endif

    // Obtém informações sobre o sistema de arquivos
    if (statvfs("/", &fs) == -1) {
        fprintf(stderr, "server_backup: Erro ao obter informações sobre o sistema de arquivos\n");
        return 0;
    }

    espaco = fs.f_bsize * fs.f_bavail;  // Calcula espaço disponível em bytes

    if (espaco < data) { // Verifica se há espaço suficiente
        cria_envia_pck(ERRO, 0, (char *) MSG_ERR_ESPACO, socket, 1);
        fprintf(stderr, "server_backup: Espaço insuficiente\n");
        return 0;
    }

    #ifdef DEBUG
        printf("Espaço suficiente\n");
    #endif
    cria_envia_pck(OK, 0, NULL, socket, 0);
    // liberando o pacote
    destroi_pacote(recebido_cliente);
    return data;
}

int inicia_fluxo_dados(FILE * arquivo, int socket) {
    unsigned char * recebido = NULL;
    uint64_t sequencia = 0;

    #ifdef DEBUG
        printf("Iniciando o fluxo de dados\n");
    #endif
    if(!(recebido = recebe_pacote(socket))) {
        fprintf(stderr, "server_backup: Erro ao receber primeiro pacote no fluxo de dados\n");
        return 0;
    }
    #ifdef DEBUG
        printf("Recebido o primeiro pacote\n");
    #endif
    // Enquanto não for o fim dos dados
    while(get_tipo_pacote(recebido) != FIM_DADOS) { 
        // Verifica se o pacote recebido é do tipo correto
        while(get_tipo_pacote(recebido) != DADOS){
            envia_nack(socket);
            destroi_pacote(recebido);
            recebido = recebe_pacote(socket);
        }

        // Verifica se o pacote recebido é da sequencia esperada
        while(get_sequencia_pacote(recebido) != sequencia) {
            envia_nack(socket);
            destroi_pacote(recebido);
            recebido = recebe_pacote(socket);
        }

        // Se tudo der certo, escreve no arquivo
        if((fwrite(get_dados_pacote(recebido), sizeof(char), get_tamanho_pacote(recebido), arquivo)) == 0) {
            fprintf(stderr, "server_backup: Erro ao escrever no arquivo\n");
            return 0;
        }

        // Envia ACK e recebe o próximo pacote
        destroi_pacote(recebido);
        envia_ack(socket);
        recebido = recebe_pacote(socket);
        sequencia++;
    }

    // Verifica se o último pacote é do tipo correto
    while(get_tipo_pacote(recebido) != FIM_DADOS) {
        fprintf(stderr, "server_backup: Ultimo pacote não é do tipo FIM_DADOS\n");
        envia_nack(socket);
        destroi_pacote(recebido);
        recebido = recebe_pacote(socket);
    }

    // como o ultimo pacote é vazio, não é necessário escrever no arquivo
    // manda apenas um ack
    envia_ack(socket);
    destroi_pacote(recebido);       // libera o pacote
    fclose(arquivo);
    return 1;
}

void trata_pacote(unsigned char * packet, int socket) {
    switch (get_tipo_pacote(packet)) {
    case BACKUP:
        backup(packet, socket);
        break;
    case RESTAURA:
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
        packet = recebe_pacote(socket);

	    if (packet != NULL) {
            trata_pacote(packet, socket);
        }
    }
    return 0;
}
