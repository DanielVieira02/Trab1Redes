#include "ConexaoRawSocket.h"
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
int backup(kermit_packet * packet, int socket);

int backup(kermit_packet * packet, int socket) {
    FILE * arquivo = fopen((char *) get_dados_pacote(packet), "w");
    kermit_packet * pacote;

    // Verifica se o arquivo foi aberto corretamente
    if (arquivo != NULL) {
        pacote = inicializa_pacote(OK, 0);
    } else {
        pacote = inicializa_pacote(ERRO, 0);
        insere_dados_pacote(packet, (char *) MSG_ERR_ACESSO, 1);
    }
     
    if(!envia_pacote(pacote, socket)) {
        fprintf(stderr, "server_backup: Erro ao enviar pacote\n");
        return 0;
    }

    // Se o arquivo foi aberto corretamente, inicia o fluxo de dados
    if (arquivo != NULL) {
        if(recebe_tamanho(arquivo, socket)){
            inicia_fluxo_dados(arquivo, socket);
            return 1;
        } else 
            return 0;
    }
    return 0;
}

int recebe_tamanho(FILE * arquivo, int socket) {
    char * data = NULL;
    unsigned long long espaco;
    kermit_packet * recebido_cliente = NULL;
    struct statvfs fs;

    recebido_cliente = recebe_pacote(socket); // espera o cliente enviar o tamanho do arquivo

    data = (char *) get_dados_pacote(recebido_cliente);

    // Obtém informações sobre o sistema de arquivos
    if (statvfs("/", &fs) == -1) {
        fprintf(stderr, "server_backup: Erro ao obter informações sobre o sistema de arquivos\n");
        return 0;
    }

    espaco = fs.f_bsize * fs.f_bavail;  // Calcula espaço disponível em bytes
    
    // Enquanto o pacote recebido não é do tipo correto
    while(get_tipo_pacote(recebido_cliente) != TAMANHO){
        envia_nack(socket);
        espera_pacote(recebido_cliente, socket);
    }

    if (espaco < atoi(data)) { // Verifica se há espaço suficiente
        cria_envia_pck(ERRO, 0, (char *) MSG_ERR_ESPACO, 1, socket);
        fprintf(stderr, "server_backup: Espaço insuficiente\n");
        return 0;
    }

    cria_envia_pck(OK, 0, NULL, 0, socket);
    return 1;
}

void inicia_fluxo_dados(FILE * arquivo, int socket) {
    kermit_packet * recebido_cliente = NULL;
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
        if(recebido_cliente->sequencia != sequencia){
            cria_envia_pck(ERRO, sequencia, (char *) MSG_ERR_SEQUENCIA, 1, socket);

            // Destroi o pacote recebido e espera o próximo
            destroi_pacote(recebido_cliente);
            espera_pacote(recebido_cliente, socket);
            continue; // Pula para a próxima iteração, com o pacote atualizado
        }

        // Se tudo der certo, escreve no arquivo
        fwrite(get_dados_pacote(recebido_cliente), sizeof(char), recebido_cliente->tamanho, arquivo); 

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

void trata_pacote(kermit_packet * packet, int socket) {
    /*
        Analisa CRC
        Se erro:
            envia_pacote(NACK);
    */

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

void server_routine() {
    int socket = ConexaoRawSocket("eno1");
    kermit_packet * packet = NULL;

    while(1) {
        // fica esperando pacotes
        packet = recebe_pacote(socket);

	    if (packet != NULL) {
            trata_pacote(packet, socket);
        }   
    }
}

int server(int argc, char * argv[]) {
    server_routine();
    return 0;
}
