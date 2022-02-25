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


/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();


  /* Initialize NullNet */

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    etimer_set(&periodic_timer, 20*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    uint16_t var = 280;
    nullnet_len = sizeof(var);
    memcpy(nullnet_buf, &var, sizeof(var));
    NETSTACK_NETWORK.output(NULL); //send broadcast
    }   
  

  PROCESS_END();
}