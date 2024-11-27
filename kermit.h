#ifndef KERMIT_H
#define KERMIT_H
#include "kermit_structs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
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

int client();
int server();

/// @brief Cria a estrutura que define o estado do pacote Kermit
/// @param packet Pacote que será enviado
/// @param procedure Procedimento que será executado
/// @param socket Socket que será utilizado
kermit_protocol_state * cria_estrutura_estado(kermit_packet * packet, kermit_protocol_state * (*procedure)(kermit_packet *, void *, int), int socket);

/// @brief Destroi a estrutura que define o estado do pacote Kermit
/// @param state Estrutura que será destruída
/// @return Retorna NULL
kermit_protocol_state * destroi_estrutura_estado(kermit_protocol_state * state);

/// @brief Define os parâmetros do procedimento para determinado estado do pacote
/// @param state Estado do pacote
/// @param procedure_params Parâmetros do procedimento
void define_parametros_procedimento_estado(kermit_protocol_state * state, void * procedure_params);

/// @brief Uma função recursiva que invoca o estado do pacote, recebe os novos estados e os invoca novamente, até que o estado seja final
/// @param state Estado do pacote
void invoca_estado(kermit_protocol_state * state);

/// @brief Aloca e inicializa os valores do pacote
/// @param packet
/// @return Retorna 1 se o pacote foi inicializado corretamente, 0 caso contrário 
kermit_packet * inicializa_pacote(char tipo, char sequencia);

/// @brief Insere os dados no pacote e define o valor do tamanho da estrutura
/// @param packet 
/// @return Retorna 1 se os dados foram inseridos corretamente, 0 se o pacote atingiu o limite máximo de tamanho
int insere_dados_pacote(kermit_packet * packet, char * dados, int tamanho);

/// @brief Pega o conjunto de dados e transforma na estrutura kermit_packet
/// @param dados 
/// @return Estrutura kermit_packet composta das informações em dados
kermit_packet * converte_bytes_para_pacote(char * dados);

/// @brief Desaloca o pacote
/// @param packet 
/// @return Retorna 1 se o pacote foi destruído corretamente, 0 caso contrário 
kermit_packet * destroi_pacote(kermit_packet * packet);

/// @brief Captura o tipo do pacote
/// @param packet
/// @return Retorna o tipo do pacote
int get_tipo_pacote(kermit_packet * packet);

/// @brief imprime o pacote
/// @param packet
void print_pacote(kermit_packet * packet);

/// @brief Envia o pacote para o socket
/// @param packet
/// @param socket
/// @return Retorna o pacote recebido como resposta, nulo se ocorrer erro no CRC
kermit_packet * envia_pacote(kermit_packet * packet, int socket);

/// @brief Recebe o pacote do socket
/// @param socket
/// @return Retorna o pacote recebido
kermit_packet * recebe_pacote(int socket);

/// @brief 
/// @param packet 
unsigned char * get_dados_pacote(kermit_packet * packet);

//TODO: Definir o CRC porque eu ainda não entendi pra que isso serve
unsigned char crc(kermit_packet * packet);

#endif
