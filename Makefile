CC = gcc
PROGS = tcpserv tcpcli
CFLAGS = -Wall
all: ${PROGS}
tcpserv:tcpserv.o head.o
	${CC} ${CFLAGS}   -o tcpserv tcpserv.o head.o  -lpthread  
tcpcli:tcpcli.o head.o
	${CC} ${CFLAGS}   -o tcpcli tcpcli.o head.o 
head.o:heads.h
clean:
	rm -f tcpserv tcpcli head.o tcpserv.o tcpcli.o
