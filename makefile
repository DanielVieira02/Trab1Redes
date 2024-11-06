CC = gcc
CFLAGS = -Wall -g
LIBS = ConexaoRawSocket

# Alvo all compila ambos os executáveis: client e server
all: client server

# Compilação do cliente
client: client.o $(LIBS).o
	$(CC) $(CFLAGS) client.o $(LIBS).o -o client

# Compilação do servidor
server: server.o $(LIBS).o
	$(CC) $(CFLAGS) server.o $(LIBS).o -o server

# Compilar a biblioteca ConexaoRawSocket
$(LIBS).o: $(LIBS).c $(LIBS).h
	$(CC) $(CFLAGS) -c $(LIBS).c

# Compilação do arquivo objeto para o cliente
client.o: client.c $(LIBS).h
	$(CC) $(CFLAGS) -c client.c

# Compilação do arquivo objeto para o servidor
server.o: server.c $(LIBS).h
	$(CC) $(CFLAGS) -c server.c

# Limpar os arquivos gerados
clean:
	rm -f *.o client server

