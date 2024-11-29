#include "kermit.h"

kermit_protocol_state * cria_estrutura_estado(kermit_packet * packet, kermit_protocol_state * (*procedure)(kermit_packet *, void *, int), int socket) {
    kermit_protocol_state * state;
    
    if(!(state = malloc(sizeof(kermit_protocol_state)))) {
        return NULL;
    }

    state->packet = packet;
    state->procedure = procedure;
    state->socket = socket;
    state->procedure_params = NULL;

    return state;
}

kermit_protocol_state * destroi_estrutura_estado(kermit_protocol_state * state) {
    if (state->packet != NULL) {
        destroi_pacote(state->packet);
    }
    free(state);
    return NULL;
}

void invoca_estado(kermit_protocol_state * state) {
    kermit_packet * resposta = stop_n_wait(state->packet, state->socket);
    kermit_protocol_state * next_state = state->procedure(resposta, state->procedure_params, state->socket);

    destroi_estrutura_estado(state);
    
    if(next_state != NULL) {
        invoca_estado(next_state);
    }
}

void define_parametros_procedimento_estado(kermit_protocol_state * state, void * procedure_params){
    if (state != NULL && procedure_params != NULL) {
        state->procedure_params = procedure_params;
    }
}

kermit_packet * inicializa_pacote(char tipo, char sequencia) {
    kermit_packet * packet;

    if(!(packet = malloc(sizeof(kermit_packet)))) {
        return NULL;
    }

    packet->tipo = tipo;
    packet->sequencia = sequencia;
    packet->tamanho = 0;
    packet->crc = 'D';
    packet->ptr_dados = packet->dados;

    memset(packet->dados, '\0', 64);
    
    return packet;
}

int insere_dados_pacote(kermit_packet * packet, char * dados, int tamanho) {
    for (int index = 0; index < tamanho && packet->tamanho < 63; index++) {
        *packet->ptr_dados = dados[index];
        packet->ptr_dados++;
        packet->tamanho++;
    }

    return packet->tamanho < 63;
}

kermit_packet * converte_bytes_para_pacote(char * dados) {
    if (dados[0] != MARCADOR_INICIO) {
        printf("Marcador de Início está incorreto. Pacote foi recebido incorretamente\n");
        return NULL;
    }

    int tamanho = dados[1];
    kermit_packet * packet;

    packet = inicializa_pacote(dados[3], dados[2]);

    for (int index = 0; index < tamanho; index++) {
        insere_dados_pacote(packet, &dados[4 + index], 1);
    }

    packet->crc = dados[5 + tamanho];

    return packet;
}

char * converte_pacote_para_bytes(kermit_packet * packet) {
    int tamanho = get_tamanho_pacote(packet);
    char * dados = malloc(sizeof(char) * (tamanho + 5));

    dados[0] = MARCADOR_INICIO;
    dados[1] = tamanho;
    dados[2] = packet->sequencia;
    dados[3] = packet->tipo;

    for (int index = 0; index < tamanho + 1; index++) {
        dados[4 + index] = packet->dados[index];
    }

    return dados;
}

kermit_packet * destroi_pacote(kermit_packet * packet) {
    free(packet);
    return NULL;
}

int get_tipo_pacote(kermit_packet * packet) {
    if (packet == NULL)
        return -1;
    return (int)packet->tamanho;
}

unsigned char * get_dados_pacote(kermit_packet * packet) {
    return packet->dados;
}

unsigned int get_tamanho_pacote(kermit_packet * packet) {
    return (unsigned int) (packet->tamanho % (1 << 5)); // 5 bits apenas
}

void print_pacote(kermit_packet * packet) {
    printf("Tamanho: %d\n", packet->tamanho);
    printf("Sequência: %d\n", packet->sequencia);
    printf("Tipo: %d\n", packet->tipo);
    
    printf("Dados: \n");
    for (int index = 0; index < (int)packet->tamanho; index++) {
        printf("%c", packet->dados[index]);
    }
    printf("\n");

    printf("CRC: %d\n", packet->crc);
}

kermit_packet * recebe_pacote(int socket) {
    char * buffer = (char *)malloc(65);
    int buffer_length;
    kermit_packet * packet = NULL;

    buffer_length = recv(socket, buffer, 68, 0);
	if (buffer_length == -1) {
	    return NULL;
	}
    packet = converte_bytes_para_pacote(buffer);

    /*
        Analisa CRC
        Errado?
            envia_pacote(NACK)
            return NULL
    */

	return packet;
}

int envia_pacote(kermit_packet * packet, int socket) {
    char * dados = converte_pacote_para_bytes(packet);
    int bytes_enviados = 0;

    bytes_enviados = send(socket, (char*)dados, 100, 0);

    free(dados);
    return bytes_enviados;
}

int espera_pacote(kermit_packet * resposta, int socket) {
    // Se nao for possivel ler, retorna -1 (timeout estourado)
    if(!(resposta = recebe_pacote(socket))) {
        // trata CRC aqui 
        return 0;
    }
    return 1;
}

kermit_packet * stop_n_wait(kermit_packet * packet, int socket) {
    kermit_packet * resposta = NULL;

    envia_pacote(packet, socket);
    if(espera_pacote(resposta, socket)) return resposta;
    
    return NULL;
}

unsigned char crc(kermit_packet *packet) {
    unsigned char gerador = 0x87; // Gerador polinomial
    unsigned char crc = 0x00;     // Valor inicial do CRC
    unsigned char data[packet->tamanho + 3];

    // Preparando os dados para o cálculo do CRC
    data[0] = packet->sequencia;
    data[1] = packet->tipo;
    data[2] = packet->tamanho;
    memcpy(&data[3], packet->dados, packet->tamanho);

    // Calculo do CRC-8
    for (int i = 0; i < packet->tamanho + 3; i++) {
        crc ^= data[i]; // XOR byte a byte
        for (int j = 0; j < 8; j++) { // Processa cada bit
            if (crc & 0x80) crc = (crc << 1) ^ gerador; // Shift e XOR com o gerador
            else crc <<= 1; // Apenas shift se não houver bit mais significativo
        }
    }

    return crc;
}

int insere_envia_pck(kermit_packet * packet, char * dados, int tamanho, int socket) {
    insere_dados_pacote(packet, dados, tamanho);
    return envia_pacote(packet, socket);
}

int cria_envia_pck(char tipo, char sequencia, char * dados, int tamanho, int socket) {
    kermit_packet * packet = inicializa_pacote(tipo, sequencia);
    int retorno = 1;
    insere_dados_pacote(packet, dados, tamanho);

    if(!envia_pacote(packet, socket)) {
        fprintf(stderr, "Erro ao enviar pacote\n");
        retorno = -1;
    }

    destroi_pacote(packet);
    return retorno;
}

int envia_ack(int socket) {
    return cria_envia_pck(ACK, 0, NULL, 0, socket);
}

int envia_nack(int socket) {
    return cria_envia_pck(NACK, 0, NULL, 0, socket);
}

char *get_ethernet_interface_name() {
    struct ifaddrs *ifaddr, *ifa;
    char *interface_name = NULL;

    // Obter lista de interfaces
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return NULL;
    }

    // Iterar pelas interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        // Ignorar interfaces que não são IPv4
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;

        // Ignorar interfaces de loopback
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue;

        // Verificar se a interface está ativa
        if (ifa->ifa_flags & IFF_UP) {
            // Copiar o nome da interface (primeira interface Ethernet encontrada)
            interface_name = strdup(ifa->ifa_name);
            break;
        }
    }

    freeifaddrs(ifaddr);
    return interface_name;
}