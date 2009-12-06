BINARY=prwd
PREFIX?=/usr/local

CFLAGS?=-Wall

#CFLAGS+=-ggdb

all: ${BINARY}

${BINARY}: main.o strtonum.o strlcpy.o strdelim.o
	gcc -o ${BINARY} main.o strtonum.o strlcpy.o strdelim.o

main.o: main.c
	gcc ${CFLAGS} -c main.c -o main.o
strtonum.o: strtonum.c
	gcc ${CFLAGS} -c strtonum.c -o strtonum.o
strlcpy.o: strlcpy.c
	gcc ${CFLAGS} -c strlcpy.c -o strlcpy.o
strdelim.o: strdelim.c
	gcc ${CFLAGS} -c strdelim.c -o strdelim.o

install: ${BINARY}
	install -m 755 ${BINARY} ${PREFIX}/bin
	install -m 644 ${BINARY}.1 ${PREFIX}/man/man1

clean:
	rm -f ${BINARY} main.o strtonum.o strlcpy.o strdelim.o
