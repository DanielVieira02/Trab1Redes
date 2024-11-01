CC = gcc
CFLAGS = -Wall -g
PROGRAM = main
LIBS = ConexaoRawSocket

all: $(PROGRAM)

$(PROGRAM): $(PROGRAM).o $(LIBS).o
	$(CC) $(CFLAGS) $(LIBS).o $(PROGRAM).o -o $(PROGRAM)

$(LIBS).o: $(LIBS).h $(LIBS).c
	$(CC) $(CFLAGS) -c $(LIBS).c

clean:
	rm -f *.o $(PROGRAM)
