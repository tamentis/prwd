BINARY=prwd
PREFIX?=/usr/local

all: ${BINARY}

${BINARY}: ${BINARY}.c
	gcc -ggdb -Wall -o ${BINARY} ${BINARY}.c

install: ${BINARY}
	install -m 755 ${BINARY} ${PREFIX}/bin
	install -m 644 ${BINARY}.1 ${PREFIX}/man/man1

clean:
	rm -f ${BINARY}
