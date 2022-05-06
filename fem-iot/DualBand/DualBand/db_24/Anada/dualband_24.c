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
static struct etimer et;
uint16_t counter_uart;
static char buf_in[100];
uint8_t beacon[3];
const char delimitador[2] = ",";
long int sortida[3];
char* endPtr;
/*---------------------------------------------------------------------------*/
PROCESS(dualband_24, "dualband 24");
AUTOSTART_PROCESSES(&dualband_24);
/*---------------------------------------------------------------------------*/

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

void serial_in(){ // Implementa la lògica a la cadena de caràcters que ha entrat al UART. node_db_24

  #ifdef DEBUG
  printf("dentro serial_in %s\n", buf_in);
  #endif

  //potser de linea 91 a 99 no és necessari
      char buffer_header[20];
      strcpy(buffer_header, buf_in);
      printf("bufheader %s\n", buffer_header);
      char *header; 
      header = strtok(buffer_header, delimitador); //agafem només "B0"
      printf("header %s\n", header);

  if (!strncmp(header, "BO", sizeof("B0")) == 0){ // B0 indicarà al db_24 que el missatge és beacon

    #ifdef DEBUG
    printf("dentro del if STRNCMP\n"); 
    #endif
    leds_toggle(LEDS_GREEN);

  
    leds_toggle(LEDS_RED);

    char *token = strtok(buf_in, delimitador);
    int i= 0; 
    
    while(token != NULL){
      token = strtok(NULL, delimitador);
      if(i == 0){
        beacon[0] = atoi(token);
      }
      else if(i == 1){
        beacon[1] = atoi(token);
      }
      else if(i == 2){
        beacon[2] = atoi(token);
      }
      i++;
    }

  printf("HOLA %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
   
    

  // Mesura sensors
  nullnet_buf = beacon;
  //memcpy(nullnet_buf, sortida, sizeof(sortida));
  nullnet_len = 3;

  NETSTACK_NETWORK.output(NULL);
  }
}

int print_uart(unsigned char c){
	buf_in[counter_uart] = c;
	counter_uart++;

	if (c == '\n'){
		printf("SERIAL DATA IN --> %s\n", (char *)buf_in); // SERIAL DATA IN --> B0, 0, 0, 0
		counter_uart = 0;
		serial_in();
	}
	
	return 1;
}

//TORNADA
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
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_24, ev, data){

  PROCESS_BEGIN();

  uart_set_input(1, print_uart);
  while(1){  
  etimer_set(&et, CLOCK_SECOND * 4);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
#ifdef DEBUG
  sprintf(buf_in, "B0, %d, %d, %d", 0, 23, 44);
  
  serial_in(); 
#endif

  //TORNADA
  //Potser es podria posar en un procès diferent...
  nullnet_set_input_callback(input_callback);

  PROCESS_END();
}
