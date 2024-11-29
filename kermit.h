#ifndef KERMIT_H
#define KERMIT_H
#include "kermit_structs.h"

#define _POSIX_C_SOURCE 200809L // para usar a função strdup
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
#include <sys/statvfs.h>
#include <ifaddrs.h>

#define MARCADOR_INICIO 0b01111110

#define ACK                     0b00000
#define NACK                    0b00001
#define OK                      0b00010
#define BACKUP                  0b00100
#define RESTAURA                0b00101
#define VERIFICA                0b00110
#define OK_CHECKSUM             0b01101
#define OK_TAMANHO              0b01110
#define TAMANHO                 0b01111
#define DADOS                   0b10000
#define FIM_DADOS               0b10001
#define ERRO                    0b11111
#define MSG_ERR_ACESSO          1
#define MSG_ERR_ESPACO          2
#define MSG_ERR_NAO_ENCONTRADO  3
#define MSG_ERR_SEQUENCIA       4           // o campo sequencia indica o pacote esperado

int client(int socket);
int server(int socket);

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
/// @return Retorna NULL
kermit_packet * destroi_pacote(kermit_packet * packet);

/// @brief Captura o tipo do pacote
/// @param packet
/// @return Retorna o tipo do pacote
int get_tipo_pacote(kermit_packet * packet);

/// @brief imprime o pacote
/// @param packet
void print_pacote(kermit_packet * packet);

/// @brief Recebe o pacote do socket
/// @param socket
/// @return Retorna o pacote recebido
kermit_packet * recebe_pacote(int socket);

/// @brief 
/// @param packet 
unsigned char * get_dados_pacote(kermit_packet * packet);

/// @brief tamanho do pacote (5 bits)
/// @param packet 
/// @return Retorna o tamanho do pacote
unsigned int get_tamanho_pacote(kermit_packet * packet);

/// @brief Insere os dados no pacote e o envia (deixar o codigo mais limpo)
/// @param packet a estrutura do pacote que sera montada
/// @param dados os dados que serao enviados
/// @param tamanho o tamanho dos dados
/// @param socket o socket que sera utilizado
/// @return Retorna 1 se o pacote foi enviado, -1 por erro no envio (função send)
int insere_envia_pck(kermit_packet * packet, char * dados, int tamanho, int socket);

/// @brief Cria e envia o pacote, no final destroi o pacote
/// @param tipo tipo do pacote
/// @param sequencia qual é o indice do pacote
/// @param dados dados que serão enviados
/// @param tamanho tamanho do campo de dados
/// @param socket socket que será utilizado
/// @return Retorna 1 se o pacote foi enviado, -1 por erro no envio (função send)
int cria_envia_pck(char tipo, char sequencia, char * dados, int tamanho, int socket);

/// @brief Envia o pacote ACK, que é um pacote vazio
/// @param socket socket que será utilizado
/// @return Retorna 1 se o pacote foi enviado, -1 por erro no envio (função send)
int envia_ack(int socket);

/// @brief Envia o pacote NACK, que é um pacote vazio
/// @param socket socket que será utilizado
/// @return Retorna 1 se o pacote foi enviado, -1 por erro no envio (função send)
int envia_nack(int socket);

/// @brief  Função que implementa o protocolo stop-and-wait
/// @param packet pacote que será enviado
/// @param socket socket que será utilizado
/// @return Retorna a resposta do pacote enviado, NULL se ocorrer erro no CRC ou timeout
kermit_packet * stop_n_wait(kermit_packet * packet, int socket);

/// @brief Função que espera um pacote
/// @param resposta pacote que será recebido
/// @param socket socket que será utilizado
/// @return Retorna 1 se o pacote foi recebido, 0 por erro no CRC
int espera_pacote(kermit_packet * resposta, int socket);

/// @brief Envia um pacote
/// @param packet pacote que será enviado
/// @param socket socket que será utilizado
/// @return Retorna 1 se o pacote foi enviado, -1 por erro no envio (função send)
int envia_pacote(kermit_packet * packet, int socket);

//TODO: Definir o CRC porque eu ainda não entendi pra que isso serve
unsigned char crc(kermit_packet * packet);

char *get_ethernet_interface_name();
#endif
