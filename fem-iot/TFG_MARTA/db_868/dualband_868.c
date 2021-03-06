#include "contiki.h"
#include "sys/etimer.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/leds.h"
#include "net/packetbuf.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/


typedef struct data_t{
  int16_t temperature, humidity;
  uint16_t noise;
} data_t;

static struct data_t datas; 

//static struct etimer et;
uint16_t counter_uart;
static char buf_in[100];
static uint8_t buffer[7];
const char delimitador[3] = " ";
long int sortida[3];
char* endPtr;
//static bool flag = 0;  
/*---------------------------------------------------------------------------*/
PROCESS(dualband_868, "dualband 868");
AUTOSTART_PROCESSES(&dualband_868);
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

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  printf("Beacon received: %d %d %d\n", bytebuf[0], bytebuf[1], bytebuf[2]);
  
  char string[20];

  sprintf(string, "B0, %d, %d, %d\n", bytebuf[0], bytebuf[1], bytebuf[2]);

  uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
  printf("beacon sent UART\n");

  free(bytebuf);
}

void serial_in(){

    char buffer_header[20];
    strcpy(buffer_header, buf_in);
    char *header; 
    header = strtok(buffer_header, delimitador);

    buffer[0] = atoi(header);
  
    char *token = strtok(buf_in, delimitador);
    printf("tokenx: %s\n", token);
    int i= 0; 
    
    while(token != NULL){
      
      token = strtok(NULL, delimitador);
      printf("token%d: %s\n",i, token);

      if(i == 0){
        
        datas.temperature = atoi(token);
        
      }
      else if(i == 1){

        	datas.humidity = atoi(token);
      }
      else if(i == 2){
          datas.noise = atoi(token);
      }
      i++;
    }

  printf("Data sensors arriba: %d %02d.%02d %02d.%02d %u\n", buffer[0], datas.temperature / 10, datas.temperature%10, datas.humidity / 10, datas.humidity % 10, datas.noise);

   
  memcpy(&buffer[1], &datas.temperature, sizeof(datas.temperature));
  memcpy(&buffer[3], &datas.humidity, sizeof(datas.humidity));
  memcpy(&buffer[5], &datas.noise, sizeof(datas.noise));


  process_poll(&dualband_868); //process_wait_till_event();
}


int print_uart(unsigned char c){

	buf_in[counter_uart] = c;
	counter_uart++;

	if (c == '\n'){
		printf("SERIAL DATA IN --> %s\n", (char *)buf_in);
		counter_uart = 0;
		serial_in();
	}
	
	return 1;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_868, ev, data){
 
  PROCESS_BEGIN();
  nullnet_set_input_callback(input_callback);
 
  uart_set_input(1, print_uart);
  while(1){
        
      
      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

      nullnet_buf = (uint8_t *) &buffer;
      nullnet_len = sizeof(buffer);

      NETSTACK_NETWORK.output(NULL);
      printf("Data sensors surt: %d %02d.%02d %02d.%02d %u\n", buffer[0], datas.temperature / 10, datas.temperature%10, datas.humidity / 10, datas.humidity % 10, datas.noise);
	
      }
  PROCESS_END();
}
