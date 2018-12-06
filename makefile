all: consumer_make producer_make

consumer_make: consumer.c helpers.h
	gcc -o consumer consumer.c helpers.h -I .

producer_make: producer.c helpers.h
	gcc -o producer producer.c helpers.h -I .
