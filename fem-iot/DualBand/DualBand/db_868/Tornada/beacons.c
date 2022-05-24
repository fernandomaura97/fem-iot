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

/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  //uint8_t* bytebuf = (uint8_t*) data;
  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  printf("Data sensors: %d %d %d %d\n", bytebuf[0], bytebuf[1], bytebuf[2], bytebuf[3]);

  //Hauriem de pujar-les al núvol o el que volguessim fer amb elles.
  
}

/*---------------------------------------------------------------------------*/

PROCESS(beacons, "beacons");
AUTOSTART_PROCESSES(&beacons);

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(beacons, ev, data){

  PROCESS_BEGIN();

  //TORNADA
  nullnet_set_input_callback(input_callback);

  PROCESS_END();
}
