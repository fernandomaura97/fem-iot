CONTIKI_PROJECT = dht22-TS1 dht22-TS2 mq131-node1 mq131-node2 multigas-client1 multigas-client2 udp-gw string-gw dht22-dbg gw-dbg

all: $(CONTIKI_PROJECT)

MODULES += os/services/shell

CONTIKI_TARGET_SOURCEFILES += dht22.c multigas.c pm10-sensor.c adc-sensors.c
MAKE_NET = MAKE_NET_NULLNET
CONTIKI = ../..

include $(CONTIKI)/Makefile.include



