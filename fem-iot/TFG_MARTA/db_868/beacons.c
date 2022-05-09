#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include "dades.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/

static struct etimer et;
uint8_t beacon [3];

/*---------------------------------------------------------------------------*/

PROCESS(beacons, "beacons");
AUTOSTART_PROCESSES(&beacons);

/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){

  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  memcpy(&datas.temperature, &bytebuf[1], 2);
  memcpy(&datas.humidity, &bytebuf[3], 2);
  memcpy(&datas.noise, &bytebuf[5], 2);

  printf(",\"Temperature\": %02d.%02d", datas.temperature / 10, datas.temperature % 10);
  printf(", \"Humidity\": %02d.%02d", datas.humidity / 10, datas.humidity % 10);
  printf(", \"Noise\": %u}", datas.noise);

  free(bytebuf);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(beacons, ev, data){

  PROCESS_BEGIN();

  while(1){

    etimer_set(&et, CLOCK_SECOND * 10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    uint8_t beacon[3] = {0x00, 0x01, 0x34};
    nullnet_buf = beacon;
    nullnet_len = 3;

    NETSTACK_NETWORK.output(NULL);

    nullnet_set_input_callback(input_callback);
    
    printf("\n");
  }

  
  
  PROCESS_END();
}
