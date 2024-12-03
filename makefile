CC = gcc
CCFLAGS = -lm -Wall -g -std=c99     #Flags de compilação
SOURCES = $(wildcard *.c)       #Arquivos .c
OBJECTS = $(SOURCES: .c=.o)     #Arquivos .o
TARGET  = main                  #Executavel

#Regra default (primeira regra)
all: $(TARGET)

lo: CCFLAGS += -DLOOP
lo: debug

debug: CCFLAGS += -DDEBUG
debug: all

$(TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

#Remove arquivos temporarios
clean:
	rm -f *~ *.o

#Remove o que não for o codigo fonte original
purge:
	rm $(TARGET)