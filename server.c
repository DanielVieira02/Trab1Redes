#include "ConexaoRawSocket.h"
#include "kermit.h"

void backup(kermit_packet * packet, int socket) {
    FILE * arquivo = fopen(get_dados_pacote(packet), "w");
    kermit_packet * pacote;

    if (arquivo != NULL) {
        pacote = inicializa_pacote(OK, 0);
    } else {
        pacote = inicializa_pacote(ERRO, 0);
        insere_dados_pacote(packet, 1, 1);
    }
     
    kermit_packet * resposta = envia_pacote(pacote, socket);

    if (arquivo != NULL) {
        tamanho(resposta, arquivo, socket);
    }
}

void tamanho(kermit_packet * packet, FILE * arquivo, int socket) {
    char * tamanho = get_dados_pacote(packet);

    // Analisa o tamanho disponível

    kermit_packet * pacote = inicializa_pacote(OK, 1);
    kermit_packet * resposta = envia_pacote(pacote, socket);

    dados(resposta, arquivo, socket);
}

void dados(kermit_packet * packet, FILE * arquivo, int socket) {
    kermit_packet * resposta = packet;
    kermit_packet * pacote;

    int sequencia = 2;

    while(get_tipo_pacote(resposta) != FIM_DADOS) {
        fprintf(arquivo, get_dados_pacote(resposta));

        pacote = inicializa_pacote(ACK, sequencia++);
        resposta = envia_pacote(pacote, socket);
        pacote = destroi_pacote(pacote);
    }

    pacote = inicializa_pacote(ACK, sequencia++);
    resposta = envia_pacote(pacote, socket);
    pacote = destroi_pacote(pacote);

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
