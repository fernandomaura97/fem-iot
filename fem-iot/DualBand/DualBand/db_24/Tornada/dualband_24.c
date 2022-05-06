#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint16_t counter_uart;
uint8_t beacon[3];
const char delimitador[2] = ",";
long int sortida[3];
  static struct etimer et;

char* endPtr;
/*---------------------------------------------------------------------------*/
PROCESS(dualband_24, "dualband 24");
AUTOSTART_PROCESSES(&dualband_24);
/*---------------------------------------------------------------------------*/

//TORNADA

unsigned int uart1_send_bytes(const unsigned char *s, unsigned int len){  
  unsigned int i = 0;

  while(s && *s != 0){
    if(i >= len) {
      break;
    }
    uart_write_byte(1, *s++);
    i++;
  }
  return i;
}



void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  //uint8_t* bytebuf = (uint8_t*) data;
  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  printf("Data sensors: %d %d %d %d\n", bytebuf[0], bytebuf[1], bytebuf[2], bytebuf[3]);
  
  char string[20];

  sprintf(string, "%d, %d, %d, %d\n", bytebuf[0], bytebuf[1], bytebuf[2], bytebuf[3]);

  uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
  free(bytebuf);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_24, ev, data){
  PROCESS_BEGIN();

  //TORNADA
  //Potser es podria posar en un procès diferent...
  nullnet_set_input_callback(input_callback);

  while(1){
    etimer_set(&et, 3*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
