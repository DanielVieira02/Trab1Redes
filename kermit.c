#include "kermit.h"
void print_byte(unsigned char byte, int fim, int comeco) {
    for(int i = fim; i >= comeco; i--){
        printf("%d", byte & (1<<i) ? 1 : 0);
    }
}

unsigned char * inicializa_pacote(char tipo, uint8_t sequencia, unsigned char * dados) {
    unsigned char * packet = NULL;
    uint8_t tamanho_dados = 0;

    #ifdef DEBUG
        printf("Inicializando pacote\n");
    #endif
    if(dados != NULL) 
        tamanho_dados = strnlen((char *)dados, TAM_CAMPO_DADOS);

    if(!(packet = calloc((OFFSET_DADOS+TAM_CAMPO_CRC) / 8 + tamanho_dados, 1))){
        return NULL;
	}

	set_marcador(packet, MARCADOR_INICIO);
    set_tamanho(packet, tamanho_dados);
    set_sequencia(packet, sequencia);
    set_tipo(packet, tipo);
    set_dados(packet, dados);
    set_crc(packet);

    #ifdef DEBUG
		printf("\n\nPacote inicializado\n");
        print_pacote(packet);
    #endif

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
    int byte = 0;
    unsigned char * dados = NULL;
    printf("Marcador: %d\n", get_marcador_pacote(packet));
    printf("Tamanho: %d\n", get_tamanho_pacote(packet));
    printf("Sequência: %d\n", get_sequencia_pacote(packet));
    printf("Tipo: %d\n", get_tipo_pacote(packet));
    dados = get_dados_pacote(packet);
    printf("Dados: %s\n", dados);
    printf("CRC: %d\n", get_CRC(packet));

    printf("Pacote em bits:\n");
    print_byte(packet[byte], 7, 0); // marcador
    printf(" ");
    byte++;
    print_byte(packet[byte], 7, 2); // 6 bits de tamanho
    printf(" ");
    print_byte(packet[byte], 1, 0); // 2 bits de sequencia
    byte++;
    print_byte(packet[byte], 7, 5); // 3 bits de sequencia
    printf(" ");
    print_byte(packet[byte], 4, 0); // 5 bits de tipo
    printf(" ");
    byte++;

    // Dados
    for(unsigned int i = 0; i < get_tamanho_pacote(packet); i++){
        print_byte(dados[i], 7, 0);
        printf(" ");
    }

    byte += get_tamanho_pacote(packet);
    print_byte(packet[byte], 7, 0); // CRC

    printf("\n");
    free(dados);
}

unsigned char * recebe_pacote(int socket) {
    unsigned char * buffer = (unsigned char *) calloc(TAM_PACOTE, 1);
    int buffer_length = 0, crc_valor = 0;

    buffer_length = recv(socket, buffer, TAM_PACOTE, 0);
	if (buffer_length == -1 || get_marcador_pacote(buffer) != MARCADOR_INICIO) {
	    return NULL;
	}
    #ifdef DEBUG
        printf("\n\nPacote recebido\n");
        print_pacote(buffer);
    #endif

    // Analise CRC
    crc_valor = crc(buffer, get_tamanho_pacote(buffer)+3);
    if(crc_valor != 0) {
        fprintf(stderr, "Erro no CRC\n");
        printf("CRC: %d\n", crc_valor);
        return NULL;
    }

    #ifdef DEBUG
        printf("CRC OK\n\n\n");
    #endif
	return buffer;
}

size_t calcula_tamanho_pacote(unsigned char * packet) {
	return get_tamanho_pacote(packet) + ((OFFSET_DADOS+TAM_CAMPO_CRC) / 8);
}

int envia_pacote(unsigned char * packet, int socket) {
    int bytes_enviados = 0;

    bytes_enviados = send(socket, packet, calcula_tamanho_pacote(packet), 0);

	printf("bytes enviados: %d\n", bytes_enviados);

    return bytes_enviados;
}

unsigned char * stop_n_wait(unsigned char * packet, int socket) {
    unsigned char * resposta = NULL;

    if(envia_pacote(packet, socket) < 0) return NULL;
    while((resposta = recebe_pacote(socket)) == NULL) {
		printf("esperando pacote...\n");
	}
    
    return resposta;
}

unsigned char crc(unsigned char *packet, int tamanho) {
    unsigned char crc = 0x00;     // Valor inicial do CRC
    unsigned char mensagem[tamanho], * dados = NULL; // Dados do pacote

    // Preparando os dados para o cálculo do CRC
    memcpy(&mensagem, &packet[OFFSET_TAM / 8], tamanho);

    // Calculando o CRC
    crc = divisao_mod_2(mensagem, tamanho);

    free(dados);
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

    return packet[byte];
}

unsigned char get_tamanho_pacote(unsigned char * packet) {
    int byte = OFFSET_TAM / 8; // o byte atual é este.

    return (packet[byte] & (0b11111100)) >> 2;
}

unsigned int get_sequencia_pacote(unsigned char * packet) {
    int byte = OFFSET_SEQ/8; // o byte atual é este.
    unsigned int sequencia = 0;

    sequencia = (packet[byte] & (0b11)) << 3;
    sequencia |= (packet[byte + 1] & (0b11100000)) >> 5;
    return sequencia;
}

unsigned char get_tipo_pacote(unsigned char * packet) {
    int byte = OFFSET_TIPO/8; // o byte atual é este.

    return packet[byte] & (0b11111);
}


void * get_dados_pacote(unsigned char * packet) {
    int byte = OFFSET_DADOS/8; // o byte atual é este.
    void * dados = NULL;
    if (packet == NULL) {
        return NULL;
    }
    if((dados = strndup((char *)&packet[byte], get_tamanho_pacote(packet))) == NULL){   // duplica o campo de dados
        return NULL;
    }

    return dados;
}

unsigned char get_CRC(unsigned char * packet) {
    unsigned int byte_inicial = (OFFSET_DADOS/8) + get_tamanho_pacote(packet); // o byte atual é este.
    
    return packet[byte_inicial];
}

unsigned char * le_intervalo_bytes(unsigned char * src, unsigned int inicio, unsigned int posicao, unsigned int quantidade){
    unsigned char * intervalo;
    int bytes_andados = 0;
  
    if(quantidade % 8 == 0) {
        intervalo = malloc(quantidade / 8);
        intervalo = memset(intervalo, 0, quantidade / 8);
    } else {
        intervalo = malloc((quantidade / 8) + 1);
        intervalo = memset(intervalo, 0, (quantidade / 8) + 1);
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
    memcpy(&package[0], &marcador, 1);
}

void set_tamanho(unsigned char * package, uint8_t tamanho){
    unsigned int mask;
    tamanho = tamanho % (1 << 6);        // garantir que o tamanho seja de 6 bits

    // 6 primeiros bits de package[OFFSET_TAM/8]
    mask = tamanho << 2;

    package[OFFSET_TAM/8] |= mask;
}

void set_sequencia(unsigned char * package, uint8_t sequencia){
    unsigned char mask;
    sequencia %= (1 << 5); // garantir que a sequencia seja de 5 bits
    mask = (sequencia & 0b11000) >> 3; 

    // 2 primeiros bits de package[OFFSET_SEQ/8]
    package[OFFSET_SEQ/8] |= mask;

    // 3 ultimos bits de package[OFFSET_SEQ/8 + 1]
    mask = (sequencia & 0b00111) << 5;
    memcpy(&package[OFFSET_SEQ/8 + 1], &mask, 1);
}

void set_tipo(unsigned char * package, unsigned char tipo){
    // 5 ultimos bits de package[OFFSET_TIPO/8]
    unsigned char mask = tipo % (1 << 5);
    package[OFFSET_TIPO/8] |= (mask);
}

void set_dados(unsigned char * package, unsigned char * dados){
    memcpy(&package[OFFSET_DADOS/8], dados, get_tamanho_pacote(package));
}

void set_crc(unsigned char * package){
    unsigned int codigo_crc = crc(package, get_tamanho_pacote(package) + 2);

    memcpy(&package[OFFSET_DADOS/8 + get_tamanho_pacote(package)], &codigo_crc, 1);
}


void escreve_bytes_intervalo(unsigned char * src, unsigned char * dest, unsigned int byte_inicial, unsigned int posicao, unsigned int tamanho) {
    // unsigned int bits_src = 0, bits_dest = posicao + (byte_inicial * 8),
    // shift_dest = bits_dest % 8, shift_src = tamanho % 8, bits_lidos = 0, byte_dest = byte_inicial;

    // printf("byte_inicial: %d\n", byte_inicial);
    // printf("tamanho: %d\n", tamanho);
    // printf("posicao: %d\n", posicao);
    // // src[bits_src/8] <<= 8 - (bits_dest % 8);
    // printf("\n");
    
    // printf("src:\n");
    // print_byte(src[0]);
        
    // if(posicao != 0){
    //     printf("src:\n");
    //     src[0] <<= 8 - posicao + ;
    //     print_byte(src[0]);
    //     printf("\ndest: ");
	// 	print_byte(dest[byte_dest]);

    //     for(int i = posicao; i <= 7; i++){
    //         printf("\nsrc[0] & (1 << %d): ", i);
    //         print_byte(src[0] & (1 << i));
    //         printf("\n");
    //         dest[byte_dest] |= (src[0] & (1 << i));
    //     }
	// 	printf("\ndest: ");
	// 	print_byte(dest[byte_dest]);
    //     bits_lidos = 8 - posicao;
    //     byte_dest++;
    // }

    // // if(bits_lidos < tamanho){
    // //     memcpy(&dest[byte_dest], &src[1], tamanho - (byte_dest - byte_inicial));
    // // }
	// printf("\n");

    return;
}

unsigned char divisao_mod_2(unsigned char *dividendo, unsigned int tamanho_dividendo){
    unsigned short divisor = DIVISOR_CRC; // será sempre o mesmo
    unsigned short resto = 0;

    // para cada byte do dividendo, realiza a divisão
    for(int i = 0; i < tamanho_dividendo; i++){
        resto ^= dividendo[i]; // faz o xor do byte do dividendo com o resto

        // para cada bit do byte, realiza a divisão
        for(int j = 0; j < 8; j++){
            // se o bit mais significativo do resto for 1, faz o xor com o divisor
            if(resto & (1<<7)) resto ^= divisor;

            resto = resto << 1; // shifta o resto para a esquerda
        }

    }

    return resto;
}
