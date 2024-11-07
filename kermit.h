#include <stdio.h>
#include <stdlib.h>

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
    unsigned char tamanho;
    unsigned char sequencia;
    unsigned char tipo;
    unsigned char crc;
    unsigned char dados[100];
    unsigned char *ptr_dados;
} kermit_packet;

/// @brief Aloca e inicializa os valores do pacote
/// @param packet
/// @return Retorna 1 se o pacote foi inicializado corretamente, 0 caso contrário 
kermit_packet * inicializa_pacote(char tipo, char sequencia);

/// @brief Insere os dados no pacote e define o valor do tamanho da estrutura
/// @param packet 
void insere_dados_pacote(kermit_packet * packet, char * dados, int tamanho);

/// @brief Pega o conjunto de dados e transforma na estrutura kermit_packet
/// @param dados 
/// @return Estrutura kermit_packet composta das informações em dados
kermit_packet * converte_bytes_para_pacote(char * dados);

/// @brief Desaloca o pacote
/// @param packet 
/// @return Retorna 1 se o pacote foi destruído corretamente, 0 caso contrário 
kermit_packet * destroi_pacote(kermit_packet * packet);

int get_tipo_pacote(kermit_packet * packet);

/// @brief 
/// @param packet 
/// @param receiver Armazena os dados do pacote no ponteiro indicado
/// @return Sequência do pacote
char * get_dados_pacote(kermit_packet * packet);

//TODO: Definir o CRC porque eu ainda não entendi pra que isso serve