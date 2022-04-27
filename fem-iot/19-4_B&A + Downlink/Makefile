CONTIKI_PROJECT = gw sta

all: $(CONTIKI_PROJECT)

#MODULES += os/services/shell


CONTIKI_TARGET_SOURCEFILES += dht22.c
CONTIKI=../..
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include
