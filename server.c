#include "kermit.h"

/// @brief Inicia o fluxo de dados, recebendo os dados do pacote e escrevendo no arquivo 
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return sem retorno
void inicia_fluxo_dados(FILE * arquivo, int socket);

/// @brief Recebe o tamanho do arquivo e verifica se há espaço suficiente para armazená-lo
/// @param arquivo arquivo que será escrito
/// @param socket socket que será utilizado
/// @return retorna 1 se houver espaço suficiente, 0 caso contrário
int recebe_tamanho(FILE * arquivo, int socket);

/// @brief Realiza o backup do arquivo
/// @param packet pacote com o nome do arquivo
/// @param socket socket que será utilizado
/// @return 1 se o arquivo foi salvo corretamente, 0 caso contrário
int backup(unsigned char * packet, int socket);

int backup(unsigned char * packet, int socket) {
    FILE * arquivo = fopen((char *) "mensagem1", "w");
    unsigned char * pacote;

    // Verifica se o arquivo foi aberto corretamente
    if (arquivo != NULL) {
        #ifdef DEBUG
            printf("Arquivo aberto com sucesso\n");
        #endif
        pacote = inicializa_pacote(OK, 0, NULL);
    } else {
        pacote = inicializa_pacote(ERRO, 0, (unsigned char *) MSG_ERR_ACESSO);
        insere_dados_pacote(packet, (char *) MSG_ERR_ACESSO, 1);
    }
     
    if(envia_pacote(pacote, socket) < 0) {
        fprintf(stderr, "server_backup: Erro ao enviar pacote\n");
        return 0;
    }

    destroi_pacote(pacote);
    // Se o arquivo foi aberto corretamente, inicia o fluxo de dados
    if (arquivo != NULL) {
        if(recebe_tamanho(arquivo, socket)){
            // inicia_fluxo_dados(arquivo, socket);
            return 1;
        } else 
            return 0;
    }
    return 0;
}

int recebe_tamanho(FILE * arquivo, int socket) {
    uint64_t data = 0, espaco = 0;
    unsigned char * recebido_cliente = NULL;
    void * raw_data = NULL;
    struct statvfs fs;

    recebido_cliente = recebe_pacote(socket); // espera o cliente enviar o tamanho do arquivo

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
    
    // Enquanto o pacote recebido não é do tipo correto
    while(get_tipo_pacote(recebido_cliente) != TAMANHO){
        envia_nack(socket);
        destroi_pacote(recebido_cliente);
        recebido_cliente = recebe_pacote(socket);
    }

    if (espaco < data) { // Verifica se há espaço suficiente
        cria_envia_pck(ERRO, 0, (char *) MSG_ERR_ESPACO, socket);
        fprintf(stderr, "server_backup: Espaço insuficiente\n");
        return 0;
    }

    cria_envia_pck(OK, 0, NULL, socket);
    return 1;
}

void inicia_fluxo_dados(FILE * arquivo, int socket) {
    unsigned char * recebido_cliente = NULL;
    int tipo_cliente = 0, sequencia = 0;

    recebido_cliente = recebe_pacote(socket);
    tipo_cliente =  get_tipo_pacote(recebido_cliente);

    // Enquanto o pacote recebido não é do tipo correto
    while (tipo_cliente != DADOS) {
        insere_envia_pck(recebido_cliente, (char *) NACK, 1, socket);

        // Destroi o pacote recebido e espera o próximo
        destroi_pacote(recebido_cliente);
        recebido_cliente = recebe_pacote(socket);
    }

    // Enquanto não for o fim dos dados
    while(tipo_cliente == DADOS && tipo_cliente != FIM_DADOS) { 
        // Verifica se o pacote recebido é do tipo correto
        if(get_sequencia_pacote(recebido_cliente) != sequencia){
            cria_envia_pck(ERRO, sequencia, (char *) MSG_ERR_SEQUENCIA, socket);

            // Destroi o pacote recebido e espera o próximo
            destroi_pacote(recebido_cliente);
            recebido_cliente = recebe_pacote(socket);
            continue; // Pula para a próxima iteração, com o pacote atualizado
        }

        // Se tudo der certo, escreve no arquivo
        fwrite(get_dados_pacote(recebido_cliente), sizeof(char), get_tamanho_pacote(recebido_cliente), arquivo); 

        // Envia ACK e recebe o próximo pacote
        destroi_pacote(recebido_cliente);
        envia_ack(socket);
        sequencia++;
    }

    // como o ultimo pacote é vazio, não é necessário escrever no arquivo
    // manda apenas um ack
    envia_ack(socket);

    fclose(arquivo);
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
