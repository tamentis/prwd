BINARY=prwd

all: ${BINARY}

${BINARY}: ${BINARY}.c
	gcc -ggdb -Wall -o ${BINARY} ${BINARY}.c

install: ${BINARY}
	install -m 755 ${BINARY} /usr/local/bin

clean:
	rm -f ${BINARY}
