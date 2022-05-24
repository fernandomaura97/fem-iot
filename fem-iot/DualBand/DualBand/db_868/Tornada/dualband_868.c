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

//TORNADA
static struct etimer et;
uint16_t counter_uart;
static char buf_in[100];
uint8_t buffer[4];
const char delimitador[2] = ",";
long int sortida[3];
char* endPtr;

/*---------------------------------------------------------------------------*/
PROCESS(dualband_868, "dualband 868");
AUTOSTART_PROCESSES(&dualband_868);
/*---------------------------------------------------------------------------*/



//TORNADA

void serial_in(){ 

  #ifdef DEBUG
  printf("dentro serial_in %s\n", buf_in);
  #endif

  //potser de linea 91 a 99 no és necessari
      char buffer_header[20];
      strcpy(buffer_header, buf_in);
      printf("bufheader %s\n", buffer_header);
      char *header; 
      header = strtok(buffer_header, delimitador);
      printf("header %s\n", header);

//INDICAR HEADER TORNADA EN COMPTES DE B0, PODRIA SER ID?

  if (!strncmp(header, "0", sizeof("0")) == 0){

    buffer[0] = atoi(header);
    #ifdef DEBUG
    printf("dentro del if STRNCMP\n"); 
    #endif

    char *token = strtok(buf_in, delimitador);
    int i= 0; 
    
    while(token != NULL){
      token = strtok(NULL, delimitador);
      if(i == 0){
        buffer[1] = atoi(token);
      }
      else if(i == 1){
        buffer[2] = atoi(token);
      }
      else if(i == 2){
        buffer[3] = atoi(token);
      }
      i++;
    }

  printf("HOLA %d %d, %d, %d\n", buffer[0], buffer[1], buffer[2], buffer[3]);
   
    

  // Mesura sensors
  nullnet_buf = buffer;
  //memcpy(nullnet_buf, sortida, sizeof(sortida));
  nullnet_len = sizeof(buffer);

  NETSTACK_NETWORK.output(NULL);
  }
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
 
  //TORNADA
  uart_set_input(1, print_uart);
  while(1){
    
  etimer_set(&et, CLOCK_SECOND * 4);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
#ifdef DEBUG
  sprintf(buf_in, "B0, %d, %d, %d", 0, 23, 44);
  
  serial_in(); 
#endif

  PROCESS_END();
}
