#include "kermit.h"

kermit_packet * inicializa_pacote(char tipo, char sequencia) {
    kermit_packet * packet;

    if(!(packet = malloc(sizeof(kermit_packet)))) {
        return NULL;
    }

    packet->tipo = tipo;
    packet->sequencia = sequencia;
    packet->tamanho = 0;

    packet->ptr_dados = packet->dados;
    
    return packet;
}

void insere_dados_pacote(kermit_packet * packet, char * dados, int tamanho) {
    for (int index = 0; index < tamanho; index++) {
        *packet->ptr_dados = dados[index];
        packet->ptr_dados++;
    }

    packet->tamanho += (unsigned char)tamanho;
}


// TODO: Método não está pronto
kermit_packet * converte_bytes_para_pacote(char * dados) {
    int tamanho = (int)dados[1];
    kermit_packet * packet;

    packet = inicializa_pacote(dados[3], dados[2]);

    return packet;
}

kermit_packet * destroi_pacote(kermit_packet * packet) {
    free(packet);
    return NULL;
}

int get_tipo_pacote(kermit_packet * packet) {
    return (int)packet->tamanho;
}

char * get_dados_pacote(kermit_packet * packet) {
    return packet->dados;
}


void print_pacote(kermit_packet * packet) {
    printf("Tamanho: %d\n", packet->tamanho);
    printf("Sequência: %d\n", packet->sequencia);
    printf("Tipo: %d\n", packet->tipo);
    
    printf("Dados: \n");
    for (int index = 0; index < (int)packet->tamanho; index++) {
        printf("%d\n", packet->dados[index]);
    }

    printf("CRC: %d\n", packet->crc);
}

int main() {
    kermit_packet * packet = inicializa_pacote(OK, 1);
    print_pacote(packet);

    unsigned char * dados = malloc(sizeof(unsigned char) * 3);
    unsigned char * pacote = malloc(sizeof(unsigned char));

    *dados = 0x01;
    *(dados + 1) = 0x09;
    *(dados + 2) = 0X0F;

    for (int index = 0; index < 3; index++) {
        insere_dados_pacote(packet, dados, 3);
    }

    printf("===================================\n");

    printf("Pacote bruto: \n");

    pacote = get_dados_pacote(packet);
    for (int index = 0; index < packet->tamanho; index++) {
        printf("%d ", pacote[index]);
    }

    printf("\n===================================\n");
    print_pacote(packet);

    free(dados);

    destroi_pacote(packet);

    return 0;
}