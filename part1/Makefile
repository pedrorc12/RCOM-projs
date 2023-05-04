dep=utils.c utils.h flags.h states.h alarm.h alarm.c data_layer.c app_layer.c

all: noncanonical writenoncanonical

noncanonical:
	gcc noncanonical.c -o receiver $(dep) -lm

writenoncanonical:
	gcc writenoncanonical.c -o sender $(dep) -lm

clean:
	rm -f sender receiver
