#COMMON.H - LUCIANO OTONI MILEN [2012079754]

all: cliente servidor

servidor: tp_socket.o common.o servidor.o
	gcc tp_socket.o common.o servidor.o -o servidor

cliente: tp_socket.o common.o cliente.o
	gcc tp_socket.o common.o cliente.o -o cliente

servidor.o: servidor.c
	gcc -c -g -Wall servidor.c

cliente.o: cliente.c
	gcc -c -g -Wall cliente.c

common.o: common.c
	gcc -c -g -Wall common.c

tp_socket.o: tp_socket.c
	gcc -c -g -Wall tp_socket.c

clean:
	rm -f *.o cliente servidor common tp_socket
