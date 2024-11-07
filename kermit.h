#include <stdio.h>

#define MARCADOR_INICIO 0x7E

#define ACK         0X00
#define NACK        0X01
#define OK          0X02
#define BACKUP      0X04
#define RESTAURA    0X05
#define VERIFICA    0X06
#define OK_CHECKSUM 0X0D
#define OK_TAMANHO  0X1E
#define TAMANHO     0X1F
#define DADOS       0x20
#define FIM_DADOS   0X21
#define ERRO        0X3F

typedef struct krm_pack {
    unsigned char tamanho[6];
    unsigned char sequencia[5];
    unsigned char tipo[5];
    unsigned char crc[8];
    unsigned char *dados;
} kermit_packet;

/// @brief Aloca e inicializa os valores do pacote
/// @param packet
/// @return Retorna 1 se o pacote foi inicializado corretamente, 0 caso contrário 
int inicializa_pacote(kermit_packet * packet, char tipo, int sequencia);

/// @brief Insere os dados no pacote e define o valor do tamanho da estrutura
/// @param packet 
void insere_dados_pacote(kermit_packet * packet);

/// @brief "Converte" o pacote para um formato pronto para ser enviado para a rede
/// @param packet 
/// @return Ponteiro de bytes contendo os dados prontos para serem enviados para a rede 
char * converte_pacote_para_bytes(kermit_packet * packet);

/// @brief Desaloca o pacote
/// @param packet 
/// @return Retorna 1 se o pacote foi destruído corretamente, 0 caso contrário 
int destroi_pacote(kermit_packet * packet);