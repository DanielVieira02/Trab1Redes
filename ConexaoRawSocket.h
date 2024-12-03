#ifndef CONEXAO_RAW_SOCKET_H
#define CONEXAO_RAW_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#define IP_A "127.0.0.1"
#define IP_B "127.0.0.2"
int ConexaoRawSocket(char * device);
int conexaoDebug(char * device, char * ip_destino);
#endif
