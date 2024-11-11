#include "kermit.h"

kermit_packet * inicializa_pacote(char tipo, char sequencia) {
    kermit_packet * packet;

    if(!(packet = malloc(sizeof(kermit_packet)))) {
        return NULL;
    }

    packet->tipo = tipo;
    packet->sequencia = sequencia;
    packet->tamanho = 0;
    packet->crc = 'D';
    packet->ptr_dados = packet->dados;

    memset(packet->dados, '\0', 64);
    
    return packet;
}

int insere_dados_pacote(kermit_packet * packet, char * dados, int tamanho) {
    for (int index = 0; index < tamanho && packet->tamanho < 63; index++) {
        *packet->ptr_dados = dados[index];
        packet->ptr_dados++;
        packet->tamanho++;
    }

    return packet->tamanho < 63;
}

kermit_packet * converte_bytes_para_pacote(char * dados) {
    if (dados[0] != MARCADOR_INICIO) {
        printf("Marcador de Início está incorreto. Pacote foi recebido incorretamente\n");
        return NULL;
    }

    int tamanho = dados[1];
    kermit_packet * packet;

    packet = inicializa_pacote(dados[3], dados[2]);

    for (int index = 0; index < tamanho; index++) {
        insere_dados_pacote(packet, &dados[4 + index], 1);
    }

    packet->crc = dados[5 + tamanho];

    return packet;
}

char * converte_pacote_para_bytes(kermit_packet * packet) {
    unsigned char tamanho = packet->tamanho;
    char * dados = malloc(sizeof(char) * (tamanho + 5));

    dados[0] = MARCADOR_INICIO;
    dados[1] = tamanho;
    dados[2] = packet->sequencia;
    dados[3] = packet->tipo;

    for (int index = 0; index < tamanho + 1; index++) {
        dados[4 + index] = packet->dados[index];
    }

    return dados;
}

kermit_packet * destroi_pacote(kermit_packet * packet) {
    free(packet);
    return NULL;
}

int get_tipo_pacote(kermit_packet * packet) {
    if (packet == NULL)
        return -1;
    return (int)packet->tamanho;
}

unsigned char * get_dados_pacote(kermit_packet * packet) {
    return packet->dados;
}

void print_pacote(kermit_packet * packet) {
    printf("Tamanho: %d\n", packet->tamanho);
    printf("Sequência: %d\n", packet->sequencia);
    printf("Tipo: %d\n", packet->tipo);
    
    printf("Dados: \n");
    for (int index = 0; index < (int)packet->tamanho; index++) {
        printf("%c", packet->dados[index]);
    }
    printf("\n");

    printf("CRC: %d\n", packet->crc);
}

kermit_packet * recebe_pacote(int socket) {
    char * buffer = (char *)malloc(65);
    int buffer_length;
    kermit_packet * packet = NULL;

    buffer_length = recv(socket, 65, 68, 0);
	if (buffer_length == -1) {
	    return NULL;
	}
    packet = converte_bytes_para_pacote(buffer);

    /*
        Analisa CRC
        Errado?
            envia_pacote(NACK)
            return NULL
    */

	return packet;
}

kermit_packet * envia_pacote(kermit_packet * packet, int socket) {
    unsigned char * dados = converte_pacote_para_bytes(packet);
    kermit_packet * resposta = NULL;

    while (resposta != NULL) {
        if (send(socket, (char*)dados, 100, 0) == -1) {
            fprintf(stderr, "Erro ao enviar mensagem\n");
        }

        resposta = recebe_pacote(socket);
    }
    
    free(dados);
    return resposta;
}
