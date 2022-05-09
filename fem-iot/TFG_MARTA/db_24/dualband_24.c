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


char buf_in[100];
uint8_t beacon[3];
const char delimitador[2] = ",";
long int sortida[3];
char* endPtr;
static bool flag = 0;
static struct data datas; 

uint16_t counter_uart;



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

      char buffer_header[20];
      strcpy(buffer_header, buf_in);
      char *header; 
      header = strtok(buffer_header, delimitador); //agafem només "B0"

  if (!strncmp(header, "BO", sizeof("B0")) == 0){ // B0 indicarà al db_24 que el missatge és beacon

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

   
  flag = 1;  

  }
  printf("Beacon: B0, %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
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

  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  bytebuf[0] = 0;
  memcpy(&datas.temperature, &bytebuf[1], 2);
  memcpy(&datas.humidity, &bytebuf[3], 2);
  memcpy(&datas.noise, &bytebuf[5], 2);

  printf("Data sensors:  %d %02d.%02d %02d.%02d %u\n", bytebuf[0], datas.temperature / 10, datas.temperature%10, datas.humidity / 10, datas.humidity % 10, datas.noise);
  
  char string[20];

  sprintf(string, "%d %d %d %u\n", bytebuf[0], datas.temperature , datas.humidity, datas.noise);

  uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);

  free(bytebuf);
}
/*---------------------------------------------------------------------------*/
PROCESS(dualband_24, "dualband 24");
AUTOSTART_PROCESSES(&dualband_24);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_24, ev, data){

  static struct etimer et;
  PROCESS_BEGIN();

  uart_set_input(1, print_uart);

  while(1){  
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if (flag == 1){
      flag = 0;
      nullnet_buf = beacon;
      nullnet_len = 3;

      NETSTACK_NETWORK.output(NULL);
    }

    nullnet_set_input_callback(input_callback);
  }
  
  printf("\n");
  PROCESS_END();
}
