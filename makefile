all: listener_make producer_make

listener_make: listener.c helpers.h
	gcc -o listener.out listener.c helpers.h -I .

producer_make: producer.c helpers.h
	gcc -o producer.out producer.c helpers.h -I .
