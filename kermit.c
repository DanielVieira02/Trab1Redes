#include "kermit.h"

unsigned char * inicializa_pacote(char tipo, unsigned char sequencia, unsigned char * dados) {
    unsigned char * packet = NULL;
    unsigned int tamanho_dados = strnlen((char *)dados, TAM_CAMPO_DADOS);

    if(!(packet = calloc((OFFSET_DADOS+TAM_CAMPO_CRC) / 8 + tamanho_dados, 1))) {
        return NULL;
    }

    set_tamanho(packet, tamanho_dados);
    set_marcador(packet, MARCADOR_INICIO);
    set_tipo(packet, tipo);
    set_sequencia(packet, sequencia);
    set_dados(packet, dados);

    set_crc(packet, 0);
    
    return packet;
}

int insere_dados_pacote(unsigned char * packet, char * dados, int tamanho) {
    return 1;
}

unsigned char * converte_bytes_para_pacote(char * dados) {
    // if (dados[0] != MARCADOR_INICIO) {
    //     fprintf(stderr, "Marcador de Início está incorreto. Pacote foi recebido incorretamente\n");
    //     return NULL;
    // }

    // int tamanho = dados[1];
    // unsigned char * packet;

    // packet = inicializa_pacote(dados[3], dados[2]);

    // for (int index = 0; index < tamanho; index++) {
    //     insere_dados_pacote(packet, &dados[4 + index], 1);
    // }

    // packet->crc = dados[5 + tamanho];

    return NULL;
}

char * converte_pacote_para_bytes(unsigned char * packet) {
    // int tamanho = get_tamanho_pacote(packet);
    // char * dados = malloc(sizeof(char) * (tamanho + 5));

    // dados[0] = MARCADOR_INICIO;
    // dados[1] = tamanho;
    // dados[2] = packet->sequencia;
    // dados[3] = packet->tipo;

    // for (int index = 0; index < tamanho + 1; index++) {
    //     dados[4 + index] = packet->dados[index];
    // }

    return NULL;
}

unsigned char * destroi_pacote(unsigned char * packet) {
    free(packet);
    return NULL;
}

void print_pacote(unsigned char * packet) {
    // printf("Marcador: %d\n", (unsigned int)get_marcador_pacote(packet));
    // printf("Tamanho: %d\n", get_tamanho_pacote(packet));
    // printf("Sequência: %d\n", get_sequencia_pacote(packet));
    // printf("Tipo: %d\n", get_tipo_pacote(packet));
    // unsigned char * dados = get_dados_pacote(packet);
    // printf("Dados: \n");
    // for (int index = 0; index < (int)get_tamanho_pacote(packet); index++) {
    //     printf("%c", dados[index]);
    // }
    // printf("\n");

    // printf("CRC: %d\n", get_CRC(packet));
    // printf("Pacote em bits:\n");
    int byte = 2, offset = 0;
    // printf("pacote: ");
    for(int i = 0; i < 8; i++){
        printf("%d", packet[0] & (1<<i) ? 1 : 0);
    }
    printf(" ");
    for(int i = 0; i < 6; i++){
        printf("%d", packet[1] & (1<<i) ? 1: 0);
    }
    printf(" ");
    for(int i = 6; i < 8; i++){
        printf("%d", packet[1] & (1<<i) ? 1: 0);
    }
    for(int i = 0; i < 3; i++){
        printf("%d", packet[2] & (1<<i) ? 1: 0);
    }
    printf(" ");
    for(int i = 3; i < 8; i++){
        printf("%d", packet[2] & (1<<i) ? 1: 0);
    }
    printf(" ");
    for(int j = 0; j < get_tamanho_pacote(packet) * 8; offset++, j++){
        if(offset % 8) {
            byte++;
            offset= 0;
        }
        printf("%d", packet[byte] & (1<<offset) ? 1: 0);
    }
    printf(" ");
    for(int i = 0; i < 8; i++){
        printf("%d", packet[byte] & (1<<offset) ? 1: 0);
    }
    printf("\n");
}

unsigned char * recebe_pacote(int socket) {
    unsigned char * buffer = (unsigned char *) malloc(TAM_PACOTE);
    int buffer_length;

    buffer_length = recv(socket, buffer, TAM_PACOTE, 0);
	if (buffer_length == -1 || get_marcador_pacote(buffer) != MARCADOR_INICIO) {
	    return NULL;
	}
    #ifdef DEBUG
        printf("Pacote recebido\n");
    #endif
    // packet = converte_bytes_para_pacote(buffer);

    /*
        Analisa CRC
        Errado?
            envia_pacote(NACK)
            return NULL
    */

	return buffer;
}

int envia_pacote(unsigned char * packet, int socket) {
    int bytes_enviados = 0;

    bytes_enviados = send(socket, packet, TAM_PACOTE, 0);

    return bytes_enviados;
}

unsigned char * stop_n_wait(unsigned char * packet, int socket) {
    unsigned char * resposta = NULL;

    envia_pacote(packet, socket);
    while((resposta = recebe_pacote(socket)) == NULL);
    
    return resposta;
}

unsigned char crc(unsigned char *packet) {
    unsigned char gerador = 0x87; // Gerador polinomial
    unsigned char crc = 0x00;     // Valor inicial do CRC
    unsigned char data[get_tamanho_pacote(packet) + 3]; // Dados do pacote

    // Preparando os dados para o cálculo do CRC
    data[0] = get_tamanho_pacote(packet);
    data[1] = get_sequencia_pacote(packet);
    data[2] = get_tipo_pacote(packet);
    memcpy(&data[3], get_dados_pacote(packet), get_tamanho_pacote(packet));

    // Calculo do CRC-8
    for (int i = 0; i < data[0] + 3; i++) {
        crc ^= data[i]; // XOR byte a byte
        for (int j = 0; j < 8; j++) { // Processa cada bit
            if (crc & 0x80) crc = (crc << 1) ^ gerador; // Shift e XOR com o gerador
            else crc <<= 1; // Apenas shift se não houver bit mais significativo
        }
    }

    return crc;
}

int insere_envia_pck(unsigned char * packet, char * dados, int tamanho, int socket) {
    insere_dados_pacote(packet, dados, tamanho);
    return envia_pacote(packet, socket);
}

int cria_envia_pck(char tipo, char sequencia, char * dados, int socket) {
    // tamanho = tamamnho do campo de dados
    unsigned char * packet = inicializa_pacote(tipo, 0, (unsigned char *)dados);
    int retorno = 1;
    insere_dados_pacote(packet, dados, strlen(dados));

    if(!envia_pacote(packet, socket)) {
        fprintf(stderr, "Erro ao enviar pacote\n");
        retorno = -1;
    }

    destroi_pacote(packet);
    return retorno;
}

int envia_ack(int socket) {
    return cria_envia_pck(ACK, 0, NULL, socket);
}

int envia_nack(int socket) {
    return cria_envia_pck(NACK, 0, NULL, socket);
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
        if (ifa->ifa_flags) {
            // Copiar o nome da interface (primeira interface Ethernet encontrada)
            interface_name = strdup(ifa->ifa_name);
            break;
        }
    }

    freeifaddrs(ifaddr);
    return interface_name;
}

unsigned char get_marcador_pacote(unsigned char * packet){
    int byte = OFFSET_MARCADOR/8; // o byte atual é este.
    int posicao = OFFSET_MARCADOR % 8;   // a posicao dentro do byte é esta

    return le_intervalo_bytes(packet, byte, posicao, TAM_CAMPO_MARCADOR)[0];
}

unsigned char get_tamanho_pacote(unsigned char * packet) {
    int byte = OFFSET_TAM/8; // o byte atual é este.
    int posicao = OFFSET_TAM % 8;   // a posicao dentro do byte é esta

    return le_intervalo_bytes(packet, byte, posicao, TAM_CAMPO_TAM)[0];
}

unsigned int get_sequencia_pacote(unsigned char * packet) {
    int byte = OFFSET_SEQ/8; // o byte atual é este.
    int posicao = OFFSET_SEQ % 8;   // a posicao dentro do byte é esta

    return le_intervalo_bytes(packet, byte, posicao, TAM_CAMPO_SEQ)[0];
}

unsigned char get_tipo_pacote(unsigned char * packet) {
    int byte = OFFSET_TIPO/8; // o byte atual é este.
    int posicao = OFFSET_TIPO % 8;   // a posicao dentro do byte é esta

    return le_intervalo_bytes(packet, byte, posicao, TAM_CAMPO_TIPO)[0];
}


unsigned char * get_dados_pacote(unsigned char * packet) {
    int byte = OFFSET_DADOS/8; // o byte atual é este.
    int posicao = OFFSET_DADOS % 8;   // a posicao dentro do byte é esta

    return le_intervalo_bytes(packet, byte, posicao, TAM_CAMPO_DADOS);
}

unsigned char get_CRC(unsigned char * packet) {
    unsigned int byte_inicial = OFFSET_DADOS + get_tamanho_pacote(packet) /8; // o byte atual é este.
    unsigned int posicao = OFFSET_DADOS + get_tamanho_pacote(packet) % 8;   // a posicao dentro do byte é esta

    return le_intervalo_bytes(packet, byte_inicial, posicao, TAM_CAMPO_CRC)[0];
}

unsigned char * le_intervalo_bytes(unsigned char * src, unsigned int inicio, unsigned int posicao, unsigned int quantidade){
    unsigned char * intervalo;
    int bytes_andados = 0;
  
    if(quantidade % 8 == 0) {
        intervalo = (unsigned char *) malloc(quantidade / 8);
    } else {
        intervalo = (unsigned char *) malloc((quantidade / 8) + 1);
    }

    if(posicao == 0) {
        bytes_andados--;
    }
    for (int pos = posicao, bits_lidos = 0; bits_lidos < quantidade; pos++, bits_lidos++) {
        if(pos % 8 == 0) {
            bytes_andados++;
            pos = 0;
        }
        intervalo[bytes_andados] |= (src[inicio + bytes_andados] & (1 << (7 - pos)));
    }

    return intervalo;
}

void set_marcador(unsigned char * package, unsigned char marcador){
    escreve_bytes_intervalo(&marcador, package, OFFSET_MARCADOR/8, OFFSET_MARCADOR % 8, TAM_CAMPO_MARCADOR);
}

void set_tamanho(unsigned char * package, unsigned char tamanho){
    escreve_bytes_intervalo(&tamanho, package, OFFSET_TAM/8, OFFSET_TAM % 8, TAM_CAMPO_TAM);
}

void set_sequencia(unsigned char * package, unsigned char sequencia){
    escreve_bytes_intervalo(&sequencia, package, OFFSET_SEQ/8, OFFSET_SEQ % 8, TAM_CAMPO_SEQ);
}

void set_tipo(unsigned char * package, unsigned char tipo){
    escreve_bytes_intervalo(&tipo, package, OFFSET_TIPO/8, OFFSET_TIPO % 8, TAM_CAMPO_TIPO);
}

void set_dados(unsigned char * package, unsigned char * dados){
    escreve_bytes_intervalo(dados, package, OFFSET_DADOS/8, OFFSET_DADOS % 8, TAM_CAMPO_DADOS * 8);
}

void set_crc(unsigned char * package, unsigned char crc){
    escreve_bytes_intervalo(&crc, package, (OFFSET_DADOS + get_tamanho_pacote(package) /8), (OFFSET_DADOS + get_tamanho_pacote(package)) % 8, TAM_CAMPO_CRC);
}

void escreve_bytes_intervalo(unsigned char * src, unsigned char * dest, unsigned int byte_inicial, unsigned int posicao, unsigned int tamanho) {
    unsigned int bytes_andados = 0, pos = posicao;
    
    if(posicao == 0) {
        bytes_andados--;
    }

    printf("novo campo\n");
    for (size_t bits_lidos = 0; bits_lidos < tamanho; bits_lidos++, pos++) {
        if(pos % 8 == 0) {
            bytes_andados++;
            pos = 0;
        }

        // dar um jeito de colocar o bit na posicao correta
        dest[byte_inicial + bytes_andados] |= (src[bytes_andados] & (1 << pos));
    }
}