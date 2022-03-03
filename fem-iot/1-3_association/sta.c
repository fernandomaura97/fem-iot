#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include "dev/radio.h"

#include "net/packetbuf.h"

/* Log configuration */
#include "sys/log.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

typedef enum {HIGH, MID, LOW} tx_power_t
const uint8_t beacon_[3] == {{0x05, 0x06, 0x07}}; 
const uint8_t ass_req[3] == {{0x08, 0x09, 0x0A}};
static linkaddr_t coordinator_addr = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}; //placeholder address
/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
PROCESS(association_process, "assoc xd");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  uint8_t rxbuf[len];
  memcpy(rxbuf, data, len);
  if(rxbuf == beacon_)
  {
    printf("Assoc. Beacon received");
    process_poll(&association_process);
  }



  }
  
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  //static unsigned count = 0;
  //static radio_value_t txpower;

  PROCESS_BEGIN();
  nullnet_set_input_callback(input_callback);
  while(1) {
    etimer_set(&periodic_timer, 10*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
   
  }

  PROCESS_END();
}

PROCESS_THREAD(association_process, ev, data)
{
    PROCESS_BEGIN();

    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, RADIO_TXPOWER_MIN);
    nullnet_buf = (uint8_t *)ass_req;
    nullnet_len = sizeof(ass_req);
    NETSTACK_NETWORK.output(NULL);
    while(!response_received)
    {
      //augment TX POWER and send again. If no response, "poison"
      
    }
     
    PROCESS_END();

}
