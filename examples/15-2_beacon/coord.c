#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"



/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

#define BEACON_INTERVAL (240* CLOCK_SECOND)





/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(uint8_t)) {

    printf("%d\n", *(uint8_t *)data);

    LOG_INFO_LLADDR(src);
    LOG_INFO_(" address \n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count = 0;

  PROCESS_BEGIN();

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback); //Comment if we don't need a rcv callback

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    //Choose between 3 random values for count
    unsigned r = random_rand() % 5;
    count = r;
    LOG_INFO("Sending %u to ", count);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");
    
    memcpy(nullnet_buf, &count, sizeof(count));
    nullnet_len = sizeof(count);

    NETSTACK_NETWORK.output(NULL);
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}