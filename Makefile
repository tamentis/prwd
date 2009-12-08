BINARY=prwd
PREFIX?=/usr/local

CFLAGS?=-Wall

CFLAGS+=-ggdb -O1

all: ${BINARY}

${BINARY}: main.o wcslcpy.o strdelim.o
	gcc -o ${BINARY} main.o wcslcpy.o strdelim.o

prwd_tests: main.c wcslcpy.o strdelim.o test.c
	gcc ${CFLAGS} -D TESTING -c main.c -o main.o
	gcc ${CFLAGS} -c test.c -o test.o
	gcc -ggdb -o run_tests main.o wcslcpy.o strdelim.o test.o

tests: prwd_tests
	./run_tests

main.o: main.c
	gcc ${CFLAGS} -c main.c -o main.o
wcslcpy.o: wcslcpy.c
	gcc ${CFLAGS} -c wcslcpy.c -o wcslcpy.o
strdelim.o: strdelim.c
	gcc ${CFLAGS} -c strdelim.c -o strdelim.o

install: ${BINARY}
	install -m 755 ${BINARY} ${PREFIX}/bin
	install -m 644 ${BINARY}.1 ${PREFIX}/man/man1

clean:
	rm -f ${BINARY} main.o wcslcpy.o strdelim.o test.o run_tests
