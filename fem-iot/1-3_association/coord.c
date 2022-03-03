#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"
#include <string.h>
#include <stdio.h> 
#include "random.h"
#include "sys/clock.h"
#include <stdlib.h>


#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6
#define NODEID_PM10_1 7
#define NODEID_PM10_2 8



#define ROUTENUMBER 8
static linkaddr_t addr_stas[ROUTENUMBER]; //store sta's addresses in here, for routing and sending


/*
#define SEND_INTERVAL (10 * CLOCK_SECOND)
#define BEACON_INTERVAL (20* CLOCK_SECOND)
#define T_MM (5* CLOCK_SECOND)
#define T_GUARD (1* CLOCK_SECOND)
#define T_SLOT (10* CLOCK_SECOND)*/


/* Configuration */
#define SPEED_NW 1 //Speed >1 if we want faster beacons and times ( for debugging "quicker" without changing too much code)

#define SEND_INTERVAL (220 * CLOCK_SECOND * (1/SPEED_NW))
#define BEACON_INTERVAL (240* CLOCK_SECOND * (1/SPEED_NW))
#define T_MM (30* CLOCK_SECOND  * 1/SPEED_NW)
#define T_GUARD (1* CLOCK_SECOND * 1/SPEED_NW)
#define T_SLOT (10* CLOCK_SECOND *  1/SPEED_NW)

//struct sensor types



/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
//PROCESS(beacon_process, "beacon process");
//PROCESS(parser_process, "Parsing process");

AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("Callback \t received rx: %d\n", *(uint8_t *)data);

  uint8_t *buf1 = (uint8_t *)malloc(sizeof(uint8_t) + 2*sizeof(float)); //allocate 9bytes (maximum payload)
  memcpy(buf1, data, len);
  
 
  uint8_t i;
  uint8_t oldaddr = 0; 
  
  for (i= 0, i<ROUTENUMBER, i++){
      if addr_stas[i] == *src){
        oldaddr = 1;
        int pos = i;     
      }   //if we have this address in our list, we don't need to add it again
  }
  if(!oldaddr){
    printf("New address:")
    LOG_INFO_LLADDR(src);
    addr_stas[pos] = *src;
  }    

 } //callback

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  const uint8_t nbuf[3] = {{0x05, 0x06, 0x07}};

  
 
  PROCESS_BEGIN();
  nbuf = {{0x05, 0x06, 0x07}};
  nullnet_set_input_callback(input_callback);
  nullnet_buf = (uint8_t *)nbuf;
  nullnet_len = sizeof(nbuf);

  while(1) {
    etimer_set(&periodic_timer, 30* CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending BEACON\n");
    NETSTACK_NETWORK.output(NULL);


    printf("addresses: \n");
    for (int i= 0, i<ROUTENUMBER, i++){
      LOG_INFO_LLADR(addr_stas[0]);
    }
   
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


