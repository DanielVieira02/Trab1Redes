#ifndef KERMIT_STRUCTS_H
#define KERMIT_STRUCTS_H

typedef struct kermit_packet {
    unsigned char tamanho;
    unsigned char sequencia;
    unsigned char tipo;
    unsigned char crc;
    unsigned char dados[64];
    unsigned char *ptr_dados;
} kermit_packet;


/**
 * Estrutura que representa o estado do protocolo Kermit e tambem uma maquina de estados.
 * cada ação X com um pacote terá seu próximo estado já predefinido dentro da função que exerceu a ação X.
 * usamos uma função que sempre retorna um ponteiro para o próximo estado, assim, a cada ação, o estado é atualizado.
 */
typedef struct kermit_protocol_state {
    int socket;                         // socket de comunicação
    void * procedure_params;            // parametros da função que será executada
    kermit_packet * packet;             // pacote que será enviado
    struct kermit_protocol_state * (*procedure)(kermit_packet *, void *, int); // função que será executada no proximo estado com o pacote
} kermit_protocol_state;

#endif