#include "kermit.h"

uint64_t SEQUENCIA_RECEBE = 0;
uint64_t SEQUENCIA_ENVIA = 0;

long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}

void aumenta_sequencia(int sequencia) {
    if(sequencia == ENVIA) {
        SEQUENCIA_ENVIA = (SEQUENCIA_ENVIA + 1) % (1 << 5);
    } else {
        SEQUENCIA_RECEBE = (SEQUENCIA_RECEBE + 1) % (1 << 5);
    }
}

void diminui_sequencia(int sequencia) {
    if(sequencia == ENVIA) {
        SEQUENCIA_ENVIA = (SEQUENCIA_ENVIA - 1) % (1 << 5);
    } else {
        SEQUENCIA_RECEBE = (SEQUENCIA_RECEBE - 1) % (1 << 5);
    }
}

void print_byte(u_int64_t byte, int fim, int comeco) {
    for(int i = fim; i >= comeco; i--){
        printf("%d", byte & (1<<i) ? 1 : 0);
    }
}

unsigned char * inicializa_pacote(char tipo, void * dados, int tamanho) {
    unsigned char * packet = NULL;

    #ifdef DEBUG
        printf("Inicializando pacote\n");
        printf("tamanho do campo de dados: %d\n", tamanho);
    #endif
    if(!(packet = calloc((OFFSET_DADOS + TAM_CAMPO_CRC) / 8 + tamanho, 1))) {
        return NULL;
    }
    set_marcador(packet, MARCADOR_INICIO);
    set_tamanho(packet, tamanho);
    if(tipo == NACK) set_sequencia(packet, SEQUENCIA_RECEBE);
    else set_sequencia(packet, SEQUENCIA_ENVIA);
    set_tipo(packet, tipo);
    set_dados(packet, dados);
    set_crc(packet);

    // Realoca o pacote para o tamanho correto, caso contrario libera o pacote e retorna -1
    if(!ajusta_pacote(&packet)) {
        free(packet);
        return NULL;
    }

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
    // printf("Dados: %s\n", dados); // descomente isso se for usar apenas strings como dados
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

int recebe_pacote(int socket, unsigned char *packet, int timeoutMillis, int com_timeout) {
    int bytes_lidos = 0;
    long long comeco = timestamp();
    struct timeval timeout = { .tv_sec = timeoutMillis/1000, .tv_usec = (timeoutMillis%1000) * 1000 };

    if(com_timeout)
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)); // seta o timeout do socket

    do {
		// Recebe o pacote
		bytes_lidos = recv(socket, packet, TAM_MAX_PACOTE_SUJO, 0);

		// Verifica se o marcador de início está correto
		if(bytes_lidos != -1 && get_marcador_pacote(packet) == MARCADOR_INICIO) {
            #ifdef DEBUG
                printf("Pacote recebido\n");
                print_pacote(packet);
            #endif
            break;
        }
	} while(com_timeout ? timestamp() - comeco <= timeoutMillis : 1);


	return bytes_lidos;
}

size_t calcula_tamanho_pacote(unsigned char * packet) {
	return get_tamanho_pacote(packet) + ((OFFSET_DADOS+TAM_CAMPO_CRC) / 8);
}

int envia_pacote(unsigned char * packet, int socket) {
    int tam = 0;  // tamanho do pacote

    tam = calcula_tamanho_pacote(packet) + count_TPID(packet, calcula_tamanho_pacote(packet));

    if(tam < 14) tam = 14;  // tamanho mínimo do pacote

    #ifdef DEBUG
        printf("Tentando enviar %d bytes:\n", tam);
    #endif
 
    tam = send(socket, packet, tam, 0);

    // Enviar o pacote
    if (tam == -1) {
        perror("Erro ao enviar o pacote");
        return -1;
    }
    #ifdef DEBUG
        printf("Pacote de tamanho %u enviado.\n", tam);
    #endif

    aumenta_sequencia(ENVIA);

    return tam;
}


unsigned char * stop_n_wait(unsigned char * packet, char tipo, int socket) {

    unsigned char * resposta = NULL;
    int recebendo_nacks = 1;

    while(recebendo_nacks == 1){
        if(envia_pacote(packet, socket) < 0) return NULL;
        if(!(resposta = espera_pacote(socket, tipo, 1))) return NULL;
        else if(get_tipo_pacote(resposta) != NACK) {
            recebendo_nacks = 0;
        } else if (get_tipo_pacote(resposta) == NACK){ // Ta pedindo um pacote que já foi enviado, mas nao chegou no destino
            diminui_sequencia(ENVIA);
        }
    }
    
    return resposta;
}

unsigned char crc(unsigned char *packet, int tamanho) {
    unsigned char crc = 0x00;     // Valor inicial do CRC
    unsigned char mensagem[tamanho];

    // Preparando os dados para o cálculo do CRC
    memcpy(&mensagem, &packet[OFFSET_TAM / 8], tamanho);

    // Calculando o CRC
    crc = divisao_mod_2(mensagem, tamanho);

    return crc;
}

int insere_envia_pck(unsigned char * packet, void * dados, int tamanho, int socket) {
    insere_dados_pacote(packet, dados, tamanho);
    return envia_pacote(packet, socket);
}

int cria_envia_pck(char tipo, void * dados, int socket, int tamanho) {
    // tamanho = tamamnho do campo de dados
    unsigned char * packet = NULL;
    int bytes_enviados = 0;

    if(!dados)
        packet = inicializa_pacote(tipo, NULL, 0);
    else 
        packet = inicializa_pacote(tipo, (unsigned char *)dados, tamanho);

    if((bytes_enviados = envia_pacote(packet, socket)) == -1) {
        fprintf(stderr, "Erro ao enviar pacote\n");
    }

    packet = destroi_pacote(packet);
    return bytes_enviados;
}

int envia_ack(int socket) {
    return cria_envia_pck(ACK, NULL, socket, 0);
}

int envia_nack(int socket) {
    return cria_envia_pck(NACK, NULL, socket, 0);
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

		interface_name = strdup(ifa->ifa_name);
		break;
	}

    freeifaddrs(ifaddr);
    return interface_name;
}

unsigned char get_marcador_pacote(unsigned char * packet){
    int byte = OFFSET_MARCADOR/8; // o byte atual é este.
    if(!packet) return 0;

    return packet[byte];
}

unsigned char get_tamanho_pacote(unsigned char * packet) {
    int byte = OFFSET_TAM / 8; // o byte atual é este.
    if(!packet) return 0;

    return (packet[byte] & (0b11111100)) >> 2;
}

unsigned int get_sequencia_pacote(unsigned char * packet) {
    int byte = OFFSET_SEQ/8; // o byte atual é este.
    unsigned int sequencia = 0;
    if(!packet) return 0;

    sequencia = (packet[byte] & (0b11)) << 3;
    sequencia |= (packet[byte + 1] & (0b11100000)) >> 5;
    return sequencia;
}

unsigned char get_tipo_pacote(unsigned char * packet) {
    int byte = OFFSET_TIPO/8; // o byte atual é este.
    if(!packet) return 0;

    return packet[byte] & (0b11111);
}


void * get_dados_pacote(unsigned char * packet) {
    int byte = OFFSET_DADOS/8; // o byte atual é este.
    if (packet == NULL || get_tamanho_pacote(packet) == 0) {
        return NULL;
    }
    void * dados = calloc(get_tamanho_pacote(packet), 1);
    if((memcpy(dados, &packet[byte], get_tamanho_pacote(packet))) == NULL){   // duplica o campo de dados
        return NULL;
    }
 
    return dados;
}


unsigned char get_CRC(unsigned char * packet) {
    unsigned int byte_inicial;

    if(!packet) return 0;
    byte_inicial = (OFFSET_DADOS/8) + get_tamanho_pacote(packet); // o byte atual é este.

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

void set_dados(unsigned char * package, void * dados){
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

int recebe_fluxo_dados(FILE * arquivo, int socket) {
    unsigned char * recebido = NULL, * dados = NULL;

    #ifdef DEBUG
        printf("Iniciando o fluxo de dados\n");
        printf("enviando OK\n");
    #endif

    // envia o pacote de OK
    if(!(recebido = cria_stop_wait(OK, NULL, 0, DADOS, socket))) {
        fprintf(stderr, "Erro ao receber primeiro pacote de dados\n");
        if(recebido) recebido = destroi_pacote(recebido);
        return 0;
    }

    #ifdef DEBUG
        printf("Recebido o primeiro pacote\n");
    #endif
    // Enquanto não for o fim dos dados
    while(get_tipo_pacote(recebido) != FIM_DADOS) { 
        // Se tudo der certo, escreve no arquivo
        dados = get_dados_pacote(recebido);
        if((fwrite(dados, sizeof(char), get_tamanho_pacote(recebido), arquivo)) == 0) {
            fprintf(stderr, "server_backup: Erro ao escrever no arquivo\n");
            return 0;
        }
        free(dados);

        // Envia ACK e recebe o próximo pacote
        destroi_pacote(recebido);

		do {
			if(!(recebido = cria_stop_wait(ACK, NULL, 0, QUALQUER_TIPO, socket))) {
				fprintf(stderr, "Erro ao receber pacote de dados\n");
				if(recebido) recebido = destroi_pacote(recebido);
				return 0;
			}
		} while(get_tipo_pacote(recebido) != DADOS && get_tipo_pacote(recebido) != FIM_DADOS);
	}

    // como o ultimo pacote é vazio, não é necessário escrever no arquivo
    // manda apenas um ack
    envia_ack(socket);
    destroi_pacote(recebido);       // libera o pacote
    return 1;
}

int envia_fluxo_dados(FILE * arquivo, uint64_t tamanho, int socket) {
    unsigned char * recebido = NULL, * enviado = NULL, * buffer = NULL;
    uint64_t total_chunks = tamanho / MAX_DADOS; // 63 bytes por pacote
    buffer = calloc(1, MAX_DADOS);

    // Envia o pacote de dados
    for(int chunks = 0; chunks < total_chunks; chunks++) {
        #ifdef DEBUG
            printf("São %lu pacotes\n", total_chunks);
            printf("Estamos no pacote %u\n", chunks);
        #endif
        // lê 63 bytes do arquivo
        if(fread(buffer, 1, MAX_DADOS, arquivo) == 0){
            fprintf(stderr, "Erro ao ler o arquivo\n");
            free(buffer);
            return 0;    
        }
        // cria o pacote de dados
        if(!(enviado = inicializa_pacote(DADOS, buffer, MAX_DADOS))) {
            fprintf(stderr, "Erro ao inicializar o pacote\n");
            free(buffer);
            return 0;
        }

        // Envia e espera o pacote ACK
        if(!(recebido = stop_n_wait(enviado, ACK, socket))) {
            fprintf(stderr, "Erro ao aloca memória para o pacote\n");
            free(buffer);
            return 0;
        }

		// destroi os pacotes
        recebido = destroi_pacote(recebido);
        enviado = destroi_pacote(enviado);
    }

    // escreve o resto dos dados
    if(tamanho % MAX_DADOS != 0) {
        #ifdef DEBUG
            printf("Resto dos dados\n");
        #endif
        if(!fread(buffer, 1, tamanho % MAX_DADOS, arquivo)){
            fprintf(stderr, "Erro ao ler o arquivo\n");
            return 0;
        }

        if(!(enviado = inicializa_pacote(DADOS, buffer, tamanho % MAX_DADOS))) {
            fprintf(stderr, "Erro ao inicializar o pacote\n");
            destroi_pacote(enviado);
            free(buffer);
            return 0;
        }

        // Envia e espera o pacote ACK
        if(!(recebido = stop_n_wait(enviado, ACK, socket))) {
            fprintf(stderr, "Erro ao aloca memória para o pacote\n");
            free(buffer);
            return 0;
        }

        recebido = destroi_pacote(recebido);
        enviado = destroi_pacote(enviado);
    }

    // envia um ultimo pacote vazio
    enviado = inicializa_pacote(FIM_DADOS, NULL, 0);

    // Envia e espera o pacote ACK
    if(!(recebido = stop_n_wait(enviado, ACK, socket))) {
        fprintf(stderr, "Erro ao aloca memória para o pacote\n");
        free(buffer);
        return 0;
    }

    enviado = destroi_pacote(enviado);
    recebido = destroi_pacote(recebido);
    free(buffer);
    return 1;
}

int ajusta_pacote(unsigned char **packet) {
    unsigned char *buffer1 = NULL;
    int tamanho = calcula_tamanho_pacote(*packet);

    // Se os dados não preencherem o pacote, preenche com 0
    if (tamanho < 14) { 
        buffer1 = realloc(*packet, 14);
        if (!buffer1) {
            perror("Erro ao realocar o pacote");
            destroi_pacote(*packet);
            *packet = NULL; // Garante que o ponteiro não seja usado após desalocar
            return 0;
        }

        // Atualiza o ponteiro e preenche com zeros a nova região
        *packet = buffer1;
        memset(&(*packet)[tamanho], 0, 14 - tamanho);
    }

    // Analisa e ajusta o pacote, liberando o antigo caso necessário
    if(!analisa_insere(packet)) {
        perror("Erro ao ajustar o pacote");
        *packet = destroi_pacote(*packet);
        return 0;
    }
    return 1;
}


u_int64_t ha_memoria_suficiente(u_int64_t tamanho) {
    struct statvfs fs;
    u_int64_t espaco;

    // Obtém informações sobre o sistema de arquivos
    if (statvfs("/", &fs) == -1) {
        fprintf(stderr, "server_backup: Erro ao obter informações sobre o sistema de arquivos\n");
        return errno;
    }

    espaco = fs.f_bsize * fs.f_bavail;  // Calcula espaço disponível em bytes

    if (espaco < tamanho) { // Verifica se há espaço suficiente
        fprintf(stderr, "server_backup: Espaço insuficiente\n");
        return ENOMEM;
    }

    return 0;
}

int analisa_insere(unsigned char ** packet) {
    unsigned char * copia_packet = NULL;
    int ocorrencias = 0;

    if(!(*packet)) return 0;

    // primeiro conta as ocorrencias
    for(size_t byte = 0; byte < calcula_tamanho_pacote(*packet); byte++) {
        if((*packet)[byte] == 0x81) {
            ocorrencias++;
        }
    }

    // se nao tiver nenhuma ocorrencia, apenas retorna o pacote
    if(ocorrencias == 0) return 1;

    // aloca e insere os 0xFF
    if(!(copia_packet = (unsigned char *)calloc(calcula_tamanho_pacote(*packet) + ocorrencias, 1))) {
        return 0;
    }

    for(size_t i = 0, j = 0; j < calcula_tamanho_pacote(*packet); j++) {
        copia_packet[i++] = (*packet)[j];
        
        // quando for 0x81, insere um 0xFF
        if((*packet)[j] == 0x81) {
            copia_packet[i++] = 0xFF;
        }
    }

    *packet = destroi_pacote(*packet);
    *packet = copia_packet;
    return 1;
}

int analisa_retira(unsigned char **packet) {
    unsigned char *copia_packet = NULL;

    if (!(*packet)) return 0;

    size_t tamanho = calcula_tamanho_pacote(*packet);
    
    // Aloca nova memória para o novo pacote
    copia_packet = calloc(tamanho, 1);
    if (!copia_packet) {
        perror("Erro ao alocar memória para copia_packet");
        return 0;
    }

    for (size_t i = 0, j = 0; j < TAM_MAX_PACOTE_SUJO && i < tamanho; j++) {
        if ((*packet)[j] == 0x81 && (*packet)[j + 1] == 0xFF) {
            copia_packet[i++] = (*packet)[j++];
        } else {
            copia_packet[i++] = (*packet)[j];
        }
    }

    *packet = destroi_pacote(*packet);
    *packet = copia_packet;
    
    return 1;
}

unsigned int count_TPID(unsigned char * buffer, unsigned int tamanho) {
    unsigned int counter = 0;

    for(unsigned int i = 0; i < tamanho + counter; i++)
        if(buffer[i] == 0x81) counter++;

    return counter;
}

unsigned int realiza_checksum(char * nome_arq){
    FILE *fp; // Ponteiro para o pipe
    char saida[128]; // Buffer para armazenar a saída do comando
    unsigned int checksum;

    char *comando = NULL;
        // checksum do arquivo
    comando = calloc(strlen("cksum ") + strlen(nome_arq) + 1, sizeof(char));

    if(!comando) {
        perror("Erro ao alocar memória para o comando cksum");
        return 0;
    }

    sprintf(comando, "cksum %s", nome_arq);
    // Executa o comando cksum e captura sua saída
    fp = popen(comando, "r");
    if (!fp) {
        perror("Erro ao executar o comando de checksum");
        free(comando);
        return 0;
    }

    // Lê a saída do comando e obtém o checksum local
    if (fgets(saida, sizeof(saida), fp) != NULL) {
        sscanf(saida, "%u", &checksum); // Extrai o checksum da saída
    }

    pclose(fp); // Fecha o pipe
    free(comando); // Libera a memória alocada para o comando

    return checksum;
}

int analisa_pacote(unsigned char ** packet, char tipo) {
    unsigned char crc_valor = 0;

	// retira os 0xFF
    if(!(analisa_retira((packet)))){
        perror("Erro ao realocar o pacote");
        return 0;
    }

    if(!(*packet)) return 0;
    // Analise CRC
    crc_valor = crc((*packet), get_tamanho_pacote((*packet))+3);
    if(crc_valor != 0) {
        #ifdef DEBUG
            fprintf(stderr, "Erro no CRC\n");
        #endif
        return 0;
    }

    #ifdef DEBUG
        printf("CRC OK\n\n\n");
    #endif

    // verifica se o pacote é nack, aqui nao importa a sequencia estar certa, já que ela se refere ao pacote perdido
    if(get_tipo_pacote((*packet)) == NACK) {
        #ifdef DEBUG
            printf("Pacote é um NACK\n");
        #endif
        return 1;
    }

    if(get_sequencia_pacote((*packet)) != SEQUENCIA_RECEBE) {
        #ifdef DEBUG
            printf("Esperavamos o pacote %lu, recebemos %u\n", SEQUENCIA_RECEBE, get_sequencia_pacote((*packet)));
        #endif
        return 0;
    }

    // verifica se é esperado prováveis erros e se o pacote é de erro
    if(get_tipo_pacote((*packet)) == ERRO && 
        (tipo == OK_TAMANHO || tipo == OK_CHECKSUM || tipo == OK)) {
        return 1;
    }


    // Verifica se o tipo do pacote é um dos primeiros que o servidor tem que receber
    if(tipo == REQUISICAO_CLIENT && 
        (get_tipo_pacote((*packet)) == BACKUP || get_tipo_pacote((*packet)) == RESTAURA || get_tipo_pacote((*packet)) == VERIFICA))
    {
        return 1;
    }

    // Verifica se o tipo do pacote é o esperado e se é esperado algum tipo em específico
    if(get_tipo_pacote((*packet)) != tipo && tipo != QUALQUER_TIPO) {
        #ifdef DEBUG
            printf("Tipo de pacote incorreto\n");
        #endif
        return 0;
    }

    return 1;
}

unsigned char * espera_pacote(int socket, char tipo, int com_timeout) {
    unsigned char * packet = NULL;
    int bytes_lidos = 0;
    uint64_t timeout = 1000;

    // recebe pacote até que seja do tipo correto
	while(1){
        if(!(packet = (unsigned char *) calloc(TAM_MAX_PACOTE_SUJO, 1))) {
            perror("Erro ao alocar memória para o pacote");
            return NULL;
        }
		while((bytes_lidos = recebe_pacote(socket, packet, timeout, com_timeout)) == -1) {
            fprintf(stderr, "Erro de timeout, enviando um NACK\n");
			envia_nack(socket);  // envia nack, já que houve timeout
            diminui_sequencia(ENVIA);
			timeout *= 2;        // timeout exponencial
		}

        // Se passar na analise ou o NACK pedir o que já foi enviado
		if(analisa_pacote(&packet, tipo) || (get_tipo_pacote(packet) == NACK && get_sequencia_pacote(packet) == RECEBE)){ break; }

        packet = destroi_pacote(packet); // pacote interpretado como lixo
    }

    if(get_tipo_pacote(packet) != NACK)
        aumenta_sequencia(RECEBE);

    return packet;
}


void imprime_erro(unsigned char * packet) {
    unsigned int * dados = (unsigned int *) get_dados_pacote(packet);

    fprintf(stderr, "Erro: ");
	switch(*dados) {
        case MSG_ERR_ACESSO:
            fprintf(stderr, "Erro de acesso ");
            break;
        case MSG_ERR_ESPACO:
            fprintf(stderr, "Espaço insuficiente ");
            break;
        case MSG_ERR_NAO_ENCONTRADO:
            fprintf(stderr, "Arquivo não encontrado ");
            break;
    }

    free(dados);
    return;
}

uint64_t get_tamanho_arquivo(char * nome_arq) {
    struct stat st;

    if(stat(nome_arq, &st) == -1) {
        perror("Erro ao obter informações do arquivo");
        return 0;
    }

    return st.st_size;
}

uint64_t testa_arquivo(char * nome_arq, int socket) {
    struct stat st;
    int err = 0;

    // printando localmente
    if(stat(nome_arq, &st) == -1) {
        fprintf(stderr, "Erro ao obter informações do arquivo: %s\n", strerror(errno));
        if(errno == EACCES)
            err = MSG_ERR_ACESSO;
        else if(errno == ENOENT)
            err = MSG_ERR_NAO_ENCONTRADO;
    }


	if(err == EACCES || err == MSG_ERR_NAO_ENCONTRADO){ // se houver algum tipo de erro
		if(cria_envia_pck(ERRO, &err, socket, sizeof(int)) < 0) {
			fprintf(stderr, "Erro ao enviar o pacote de erro\n");
		}
    }

    #ifdef DEBUG
        if(!err) printf("Sem erro ao acessar o arquivo\n");
        else printf("Com erro ao acessar o arquivo\n");
    #endif

	return err;
}

unsigned char * cria_stop_wait(char tipo_enviado, void * dados, int tamanho, char tipo_esperado, int socket) {
    unsigned char * enviado = NULL, * recebido = NULL;
    #ifdef DEBUG
        printf("tipo enviado = %d\n", tipo_enviado);
    #endif
    // cria um pacote com o nome do arquivo no campo de dados
    if(!(enviado = inicializa_pacote(tipo_enviado, dados, tamanho))) {
        fprintf(stderr, "Erro ao inicializar o pacote de envio\n");
        return NULL;
    }

    // Envia e espera o pacote
    if(!(recebido = stop_n_wait(enviado, tipo_esperado, socket))) {
        fprintf(stderr, "Erro ao alocar inicializar o pacote de resposta\n");
        enviado = destroi_pacote(enviado);
        if(recebido) recebido = destroi_pacote(recebido);
        return NULL;
    }

    enviado = destroi_pacote(enviado);
    return recebido;
}

int testa_memoria(uint64_t tamanho, int socket) {
    int stts = 0;
    if((stts = ha_memoria_suficiente(tamanho))) {
        if(stts == ENOMEM) {
		    fprintf(stderr, "Erro: Memória insuficiente %s\n", strerror(stts));
			// cria e envia um pacote de erro
			if((cria_envia_pck(ERRO, &stts, socket, sizeof(char)) == -1)) {
				fprintf(stderr, "Erro ao enviar o pacote de erro\n");
				return 0;
			}
        }
        return 0;
    }
    return 1;
}