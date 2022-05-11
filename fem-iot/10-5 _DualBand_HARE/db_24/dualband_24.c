#include "contiki.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <string.h>

#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"


PROCESS(dualband_24, "dualband 24");
PROCESS(sender, "sender");
AUTOSTART_PROCESSES(&dualband_24, &sender);
/*---------------------------------------------------------------------------*/


typedef struct data_t{
  int16_t temperature, humidity;
  uint16_t noise;
} data_t;

static struct data_t datas; 

char buf_in[100];
uint8_t beacon[3];
const char delimitador[2] = ",";
long int sortida[3];
char* endPtr;
//static bool flag = 0;

uint16_t counter_uart = 0;

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

    process_poll(&sender);
   
  //flag = 1;  
 
  }

 else if(!srncmp(header, "P0", sizeof("P0")) == 0){ // P0 indicarà al db_24 que el missatge és un poll

  printf("received poll message\n");
  char *token = strtok(buf_in, delimitador);
  
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
 }

  printf("Beacon: B0, %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
}

int print_uart(unsigned char c){
	buf_in[counter_uart] = c;
	counter_uart++;

	if (c == '\n'){
		printf("SERIAL DATA IN --> %s", (char *)buf_in); // SERIAL DATA IN --> B0, 0, 0, 0
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
  memcpy(&datas.temperature, &bytebuf[1], sizeof(int16_t));
  memcpy(&datas.humidity, &bytebuf[3], sizeof(int16_t));
  memcpy(&datas.noise, &bytebuf[5], sizeof(uint16_t));

  printf("Data sensors:  %d %02d.%02d %02d.%02d %u\n", bytebuf[0], datas.temperature / 10, datas.temperature%10, datas.humidity / 10, datas.humidity % 10, datas.noise);
  
  char string[20];

  sprintf(string,"%d %d %d %u\n", bytebuf[0], datas.temperature , datas.humidity, datas.noise);
  //printf("string sent: %s; END\n", string);

  uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
  free(bytebuf);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_24, ev, data){

  //static struct etimer et;
  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);
  uart_set_input(1, print_uart);

  while(1){  
    PROCESS_YIELD();

    /*if (flag == 1){
      flag = 0;
      nullnet_buf = beacon;
      nullnet_len = 3;

      printf("broadcasting BEACON: %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);

      NETSTACK_NETWORK.output(NULL);
    }*/
  } 
  PROCESS_END();
}

PROCESS_THREAD(sender, ev,data){
  PROCESS_BEGIN();

  while(1){

    PROCESS_WAIT_EVENT_UNTIL(PROCESS_EVENT_POLL);

    nullnet_buf = (uint8_t* )beacon;
    nullnet_len = sizeof(beacon);

    printf("broadcasting BEACON: %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
    NETSTACK_NETWORK.output(NULL);
 }
  PROCESS_END();
}
