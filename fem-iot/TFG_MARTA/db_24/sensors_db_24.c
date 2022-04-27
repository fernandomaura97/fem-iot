#include "contiki.h"
#include "sys/etimer.h"
#include "dev/uart.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
PROCESS(sensors_db_24, "sensors db 24");
AUTOSTART_PROCESSES(&sensors_db_24);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  //uint8_t* bytebuf = (uint8_t*) data;
  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  printf("Data beacon: B0 %d %d %d\n", bytebuf[0], bytebuf[1], bytebuf[2]);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensors_db_24, ev, data){

  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);

  while(1){

    PROCESS_YIELD(); 
    
  }
  PROCESS_END();
}
