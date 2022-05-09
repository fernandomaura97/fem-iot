#include "contiki.h"
#include "sys/etimer.h"
#include "dev/uart.h"
#include "dev/dht22.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include "dev/gpio-hal.h"
#include "dev/adc-sensors.h"
#include "lib/sensors.h"
#include "dev/serial-line.h"
#include "dev/leds.h"
#include "sys/log.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
typedef struct data{
  int16_t temperature, humidity;
  uint16_t noise;
}
