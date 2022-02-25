#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include "dev/radio.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count = 0;
  static radio_value_t txpower;

  PROCESS_BEGIN();

  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  

  count = 69;

 
  
  while(1) {
    
    etimer_set(&periodic_timer, 5 * CLOCK_SECOND);  
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    nullnet_buf = (uint8_t *)&count;
    nullnet_len = sizeof(count);


    NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &txpower);
    printf("txpower inicial: %d\n", txpower);

    etimer_set(&periodic_timer, 10 * CLOCK_SECOND);  
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

  
    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, -5);

    NETSTACK_NETWORK.output(NULL);


    NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &txpower);
    printf("txpower cambio 1: %d\n", txpower);

     etimer_set(&periodic_timer, 10 * CLOCK_SECOND);  
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, 0);
    NETSTACK_NETWORK.output(NULL);


    NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &txpower);
    printf("txpower 0: %d\n", txpower);

  }

  PROCESS_END();
}