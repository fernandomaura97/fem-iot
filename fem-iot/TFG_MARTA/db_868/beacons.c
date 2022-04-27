#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
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

PROCESS_THREAD(beacons, ev, data){

  PROCESS_BEGIN();
  while(1){
  etimer_set(&et, CLOCK_SECOND * 60);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  uint8_t beacon[3] = {0x00, 0x01, 0x34};
  nullnet_buf = beacon;
  nullnet_len = 3;

  NETSTACK_NETWORK.output(NULL);
  
  }

  PROCESS_END();
}