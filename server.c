#include "ConexaoRawSocket.h"
#include "kermit.h"

void salva_dados(kermit_packet * packet, int socket) {
    /*
    Enquanto tipo do pacote não for FIM_DADOS
        Escreve dados no arquivo
        pacote = recebe_pacote()
        envia_pacote(ACK)
    */
}

void trata_pacote(kermit_packet * packet, int socket) {
    /*
        Analisa CRC
        Se erro:
            envia_pacote(NACK);
    */

    switch (get_tipo_pacote(packet))
    {
    case BACKUP:
        /*  Tem acesso?
                envia_pacote(OK);
            Se não:
                envia_pacote(ERRO);
         */
        break;
    case TAMANHO:
        /*  Tem espaço disponível?
                envia_pacote(OK);
            Se não:
                envia_pacote(ERRO);
         */
        break;
    case DADOS:
        salva_dados(packet, socket):
        break;
    default:
        break;
    }
}

void server_routine() {
    int socket = ConexaoRawSocket("eno1");
    kermit_packet * packet = NULL;

    while(1) {
        packet = recebe_pacote(socket);

	    if (packet != NULL) {
            trata_pacote(packet, socket);
        }   
    }
}

int server(int argc, char * argv[]) {
    server_routine();
    return 0;
}
