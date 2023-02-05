CROSS_COMPILE?=
CC=${CROSS_COMPILE}gcc
all: writer.o
	${CC} writer.o -o writer
writer.o: writer.c
	${CC} -Wall writer.c -c
.PHONY: clean run
clean:
	rm -rf *.o writer
run: all
	@./writer
