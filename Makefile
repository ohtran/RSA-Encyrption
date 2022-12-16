CC = clang
CFLAGS = -Wall -Wpedantic -Wextra -Werror
LFLAGS = -lgmp -lm  

all: keygen encrypt decrypt

keygen: keygen.o randstate.o numtheory.o rsa.o
	$(CC) -o keygen keygen.o randstate.o numtheory.o rsa.o $(LFLAGS)

encrypt: encrypt.o randstate.o numtheory.o rsa.o
	$(CC) -o encrypt encrypt.o randstate.o numtheory.o rsa.o $(LFLAGS)

decrypt: decrypt.o randstate.o numtheory.o rsa.o
	$(CC) -o decrypt decrypt.o randstate.o numtheory.o rsa.o $(LFLAGS)

keygen.o: keygen.c 
	$(CC) $(CFLAGS) -c keygen.c

randstate.o: randstate.c 
	$(CC) $(CFLAGS) -c randstate.c 

numtheory.o: numtheory.c
	$(CC) $(CFLAGS) -c numtheory.c

rsa.o: rsa.c 
	$(CC) $(CFLAGS) -c rsa.c 

clean:
	rm -f keygen encrypt decrypt *.o

format:
	clang-format -i -style=file *.[ch]

