#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SENSOR_TYPE 1
/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

//static linkaddr_t coordinator_addr = {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x15, 0x9f, 0x57  }};

static bool flag = 0; 
unsigned int timtim = 5;
/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) {
    unsigned count;
   
    if(flag == 1){
        
        memcpy(&count, data, sizeof(count));
        LOG_INFO("Received %u from ", count);
        LOG_INFO_LLADDR(src);
        LOG_INFO_("\n");


        switch(count) {
            case 0: 
                timtim = 1;
                break;
            case 1:
                timtim = 20;
                break;
            case 2:
                timtim = 50;
                break;
            case 3:
                timtim = 50;
                break;
            case 4:
                timtim = 100;
                break;  
            }
        }

  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  static unsigned count = 0;

  PROCESS_BEGIN();

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  
  while(1) {
    printf("CLOCK: time remaining:\t %u\n", (timtim + 10));
    etimer_set(&periodic_timer, timtim * CLOCK_SECOND);  
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    flag = 1;

    etimer_set(&send_timer, 10 * CLOCK_SECOND);  
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

    flag = 0; 
    printf("rst\n");   
    
     
  }

  PROCESS_END();
}