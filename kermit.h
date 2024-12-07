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
#include <errno.h>
#include <math.h>
#include <inttypes.h>
#define _FILE_OFFSET_BITS 64

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

#define TAM_CAMPO_MARCADOR      8
#define TAM_CAMPO_TAM           6
#define TAM_CAMPO_SEQ           5
#define TAM_CAMPO_TIPO          5
#define TAM_CAMPO_DADOS         64
#define TAM_CAMPO_CRC           8
#define TAM_PACOTE              544
#define TAM_PACOTE_BYTES        68

#define OFFSET_MARCADOR         0
#define OFFSET_TAM              8
#define OFFSET_SEQ              14
#define OFFSET_TIPO             19
#define OFFSET_DADOS            24
#define OFFSET_CRC              88

#define DIVISOR_CRC             0x119
#define MAX_DADOS               63

#define ERRO_CRC                -1
#define ERRO_ALLOC              -2
#define ERRO_RECV               -3
#define ERRO_MARCADOR           -4
extern uint64_t SEQUENCIA;
int client(int socket);
int server(int socket);

void aumenta_sequencia();
void diminui_sequencia();

/// @brief Aloca e inicializa os valores do pacote
/// @param packet
/// @return Retorna o pacote inicializado, caso contrário retorna NULL
unsigned char * inicializa_pacote(char tipo, void * dados, int tamanho);

/// @brief Insere os dados no pacote e define o valor do tamanho da estrutura
/// @param packet 
/// @return Retorna 1 se os dados foram inseridos corretamente, 0 se o pacote atingiu o limite máximo de tamanho
int insere_dados_pacote(unsigned char * packet, char * dados, int tamanho);

/// @brief Pega o conjunto de dados e transforma na estrutura unsigned char
/// @param dados 
/// @return Estrutura unsigned char composta das informações em dados
unsigned char * converte_bytes_para_pacote(char * dados);

/// @brief Desaloca o pacote
/// @param packet 
/// @return Retorna NULL
unsigned char * destroi_pacote(unsigned char * packet);

/// @brief imprime o pacote
/// @param packet
void print_pacote(unsigned char * packet);

/// @brief Recebe o pacote do socket
/// @param socket
/// @return Retorna o pacote recebido, NULL se ocorrer erro no CRC ou timeout
unsigned char * recebe_pacote(int socket);

/// @brief tamanho do pacote (5 bits)
/// @param packet 
/// @return Retorna o tamanho do pacote
unsigned char get_tamanho_pacote(unsigned char * packet);

/// @brief Insere os dados no pacote e o envia (deixar o codigo mais limpo)
/// @param packet a estrutura do pacote que sera montada
/// @param dados os dados que serao enviados
/// @param tamanho o tamanho dos dados
/// @param socket o socket que sera utilizado
/// @return Retorna 1 se o pacote foi enviado, -1 por erro no envio (função send)
int insere_envia_pck(unsigned char * packet, char * dados, int tamanho, int socket);

/// @brief Cria e envia o pacote, no final destroi o pacote
/// @param tipo tipo do pacote
/// @param sequencia qual é o indice do pacote
/// @param dados dados que serão enviados
/// @param tamanho tamanho do campo de dados
/// @param socket socket que será utilizado
/// @return Retorna 1 se o pacote foi enviado, -1 por erro no envio (função send)
int cria_envia_pck(char tipo, char sequencia, char * dados, int socket, int tamanho);

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
unsigned char * stop_n_wait(unsigned char * packet, int socket);

/// @brief Envia um pacote
/// @param packet pacote que será enviado
/// @param socket socket que será utilizado
/// @return Retorna > 1 se o pacote foi enviado, -1 por erro no envio (função send)
int envia_pacote(unsigned char * packet, int socket);

//TODO: Definir o CRC porque eu ainda não entendi pra que isso serve
unsigned char crc(unsigned char * packet, int tamanho);

char *get_ethernet_interface_name();


/// @brief Função que lê bytes entre um intervalo deles
/// @param inicio byte inicial, no qual se inicia a leitura
/// @param posicao posicao do primeiro bit a ser lido
/// @param quantidade quantidade de bits lidos
unsigned char * le_intervalo_bytes(unsigned char * src, unsigned int inicio, unsigned int posicao, unsigned int quantidade);

/// @brief Seta o marcador do pacote
/// @param package pacote em que será escrito o marcador
/// @param marcador o marcador em si
void set_marcador(unsigned char * package, unsigned char marcador);

/// @brief Seta o tamanho do pacote
/// @param package pacote em que será escrito o tamanho
/// @param tamanho o tamanho em si 
void set_tamanho(unsigned char * package, uint8_t tamanho);

/// @brief Seta o numero da sequencia do pacote
/// @param package pacote em que será escrito a sequencia
/// @param sequencia a sequencia em si
void set_sequencia(unsigned char * package, uint8_t sequencia);

/// @brief Seta o tipo do pacote
/// @param package pacote em que será escrito o tipo
/// @param tipo o tipo em si
void set_tipo(unsigned char * package, unsigned char tipo);

/// @brief Seta os dados do pacote
/// @param package pacote em que será escrito os dados
/// @param dados os dados em si
void set_dados(unsigned char * package, void * dados);

/// @brief Seta o CRC, é necessário que o campo tamanho já esteja setado.
/// @param package pacote em que será escrito o CRC
void set_crc(unsigned char * package);

/// @brief Função que escreve os bytes dados em src em um intervalo específico
/// @param src De onde virá os dados
/// @param dest destino dos dados
/// @param byte_inicial bytes onde se iniciará os dados em dest
/// @param posicao posicao do bit no byte_inicial em dest
/// @param tamanho quantidade de bits que serão escritos
void escreve_bytes_intervalo(unsigned char * src, unsigned char * dest, unsigned int byte_inicial, unsigned int posicao, unsigned int tamanho);

/// @brief Função que pega o marcador do pacote
/// @param packet pacote que será lido
/// @return Retorna o marcador do pacote 
unsigned char get_marcador_pacote(unsigned char * packet);

/// @brief Função que pega o tamanho do pacote
/// @param packet pacote que será lido
/// @return Retorna o tamanho do pacote
unsigned int get_sequencia_pacote(unsigned char * packet);

/// @brief Função que pega a sequencia do pacote
/// @param packet pacote que será lido
/// @return Retorna a sequencia do pacote
unsigned char get_tipo_pacote(unsigned char * packet);

/// @brief Função que gera uma cópia dos dados do pacote
/// @param packet pacote que será lido
/// @return Retorna os dados do pacote
void * get_dados_pacote(unsigned char * packet);

/// @brief Função que pega o CRC do pacote
/// @param packet pacote que será lido
/// @return Retorna o CRC do pacote
unsigned char get_CRC(unsigned char * packet);

/// @brief Função que realiza a divisão do pacote pelo polinômio gerador, uma divisão módulo 2
/// @param dividendo valor que será dividido
/// @param tamanho_dividendo quantidade de bytes do dividendo
unsigned char divisao_mod_2(unsigned char *dividendo, unsigned int tamanho_dividendo);

void print_byte(uint64_t byte, int fim, int comeco);

/// @brief Função que recebe os dados via socket
/// @param arquivo arquivo que será enviado
/// @param tamanho tamanho do arquivo
int recebe_fluxo_dados(FILE * arquivo, int socket);

/// @brief Função que envia os dados via socket
/// @param arquivo descritor do arquivo que será enviado
/// @param tamanho tamanho do arquivo
/// @param socket socket que será utilizado
/// @return Retorna 1 se o envio foi realizado com sucesso, 0 caso contrário
int envia_fluxo_dados(FILE * arquivo, uint64_t tamanho, int socket);

/// @brief Função que ajusta e trunca o pacote, se necessário
/// @param packet pacote que será ajustado
/// @return Retorna 1 se o pacote foi ajustado, 0 caso contrário
int ajusta_pacote(unsigned char ** packet);

/// @brief Função que verifica se há memória suficiente para alocar um pacote
/// @param tamanho tamanho do pacote
/// @return Retorna 1 se houver memória suficiente, 0 caso contrário
u_int64_t ha_memoria_suficiente(u_int64_t tamanho);


size_t calcula_tamanho_pacote(unsigned char * packet);
#endif
