#include "contiki.h"
#include "sys/etimer.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/leds.h"
#include "net/packetbuf.h"
#include "sys/log.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/


#define LOG_MODULE "DB_868"
#define LOG_LEVEL LOG_LEVEL_DBG


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

volatile static uint16_t len_msg;
static bool is_associated = 0; // flag to check if the node is associated

static linkaddr_t from, gw_addr; 
//static bool flag = 0;  
/*---------------------------------------------------------------------------*/
PROCESS(dualband_868, "dualband 868");
PROCESS(radio_rx_process, "rx process");
PROCESS(associator_process, "assoc process");
AUTOSTART_PROCESSES(&dualband_868, &radio_rx_process);
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
  
  
  
  linkaddr_copy(&from, src); //PREMATURO, comprobar!
  len_msg = len; 
  process_poll(&radio_rx_process);
  
  
  
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


PROCESS_THREAD(radio_rx_process, ev, data){

static struct etimer beacon_no_timer;
static bool bcon_flag = 0; 

volatile static uint8_t* datapoint;
PROCESS_BEGIN();

while(1){

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

  datapoint = malloc(len_msg);
  datapoint = packetbuf_dataptr();


  uint8_t frame_header = (datapoint[0] & 0b11100000) >> 5; 
  
  if(frame_header == 0){
    printf("beacon\n");
    
    linkaddr_copy(&gw_addr, &from);

                
    uint8_t Beacon_no = datapoint[0] & 0b00011111;

    if(bcon_flag == 0){

      bcon_flag = 1; 

      
      if(Beacon_no == 0){
        
        
        etimer_set(&beacon_no_timer, CLOCK_SECOND);
        LOG_DBG("Beacon 0 received\n");

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&beacon_no_timer));
      }
      else if(Beacon_no == 1)
      {   
       
        LOG_DBG("Beacon 1 received\n");
        etimer_set(&beacon_no_timer, CLOCK_SECOND/2 );
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&beacon_no_timer));

      }
      else if(Beacon_no ==2)
      {   
        LOG_DBG("Beacon 2 received\n");
      }
      // TODO: find a METHOD to wait for the 1.5 seconds after first beacon, or time depending on the message received first    
      else{
          
          LOG_INFO("ignoring beacon\n");
      }

     

      printf("Beacon received: %d %d %d\n", datapoint[0], datapoint[1], datapoint[2]);
      
      char string[20];

      sprintf(string, "B0, %d, %d, %d\n", datapoint[0], datapoint[1], datapoint[2]);

      uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
      printf("beacon sent UART\n");
    }
      
  }
  else if(frame_header == 1){
    LOG_DBG("association request received\n");

  }

  else if(frame_header == 2){
    LOG_DBG("association response received\n");
    
    if(linkaddr_cmp(&from, &gw_addr)) { 

          LOG_INFO("Not associated, associating now\n");
          PROCESS_CONTEXT_BEGIN(&associator_process);
          is_associated = true; 
          PROCESS_CONTEXT_END(&associator_process);
        }
        else{
            LOG_DBG("error, different adresses");
        }
    
  }

  else if(frame_header == 3){

    LOG_DBG("Sensor data received !? \n");

  }

  else{
    LOG_DBG("unknown frame header\n");
  }

}
PROCESS_END();



}


PROCESS_THREAD(associator_process, ev, data){
  PROCESS_BEGIN();

  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    printf("associating\n");

  }
  PROCESS_END();
}