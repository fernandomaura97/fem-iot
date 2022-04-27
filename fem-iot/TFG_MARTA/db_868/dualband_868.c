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
PROCESS(dualband_868, "dualband 868");
AUTOSTART_PROCESSES(&dualband_868);
/*---------------------------------------------------------------------------*/

/*
La funció següent té la finalitat d'enviar bytes mitjançant UART
*/

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

/*
La funció següent serà cridada cada vegada que arribi un byte
*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  //uint8_t* bytebuf = (uint8_t*) data;
  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  printf("Data beacon: B0 %d %d %d\n", bytebuf[0], bytebuf[1], bytebuf[2]);
  
  char string[20];

  sprintf(string, "B0, %d, %d, %d\n", bytebuf[0], bytebuf[1], bytebuf[2]);

  uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_868, ev, data){

  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);

  while(1){
    
    PROCESS_YIELD();
      // Si es volgués
  }
  PROCESS_END();
}