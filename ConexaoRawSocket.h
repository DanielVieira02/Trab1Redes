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

int ConexaoRawSocket(char * device);
#endif
