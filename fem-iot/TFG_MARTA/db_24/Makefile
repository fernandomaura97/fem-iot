CONTIKI_PROJECT = dualband_24 sensors_db_24

all: $(CONTIKI_PROJECT)

#MODULES += os/services/shell

CONTIKI_TARGET_SOURCEFILES += tsl256x.c sht25.c bmpx8x.c motion-sensor.c
CONTIKI_TARGET_SOURCEFILES += adc-sensors.c weather-meter.c grove-gyro.c
CONTIKI_TARGET_SOURCEFILES += rgb-bl-lcd.c pm10-sensor.c iaq.c zonik.c relay.c
CONTIKI_TARGET_SOURCEFILES += dht22.c servo.c ac-dimmer.c bme280-arch.c

CONTIKI = ../../..
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include



