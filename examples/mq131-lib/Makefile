CONTIKI_PROJECT = mq131-multigas-thingspeak mq131- mq131-try
CONTIKI = ../..

MODULES += os/services/shell
CONTIKI_TARGET_SOURCEFILES += MQ1312.c

MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC

#TARGET_LIBFILES += -L multigas.c multigas.h

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include

