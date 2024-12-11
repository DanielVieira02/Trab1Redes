#ifndef FRONT_H
#define FRONT_H
#include <termios.h>
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

void imprime_barra_progresso(double progresso);
void imprime_recebendo(uint64_t quantidade_bytes);

#endif