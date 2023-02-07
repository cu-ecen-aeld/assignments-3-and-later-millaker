CROSS_COMPILE?=
CC=gcc
CFLAG=-g -Wall -Werror
NAME=aesdsocket
all: ${NAME}.c
	${CROSS_COMPILE}${CC} ${NAME}.c ${CFLAG} -o ${NAME}

.PHONY: clean
clean:
	rm -rf ${NAME}.o ${NAME}
