#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"


#include <dev/serial-line.h>
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (15 * CLOCK_SECOND)

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

static unsigned tx_var = 0;

void serial_handler( char* data) {

  char* buf = (char*) data;



  if(strcmp(buf, "0") == 0){
    printf("caso 0: 10 segundos \n");
    tx_var = 0;
  }

  else if(strcmp(buf, "1") == 0){
    printf("caso 1: 20 segundos \n");
    tx_var = 1;
  }

  else if(strcmp(buf, "2") == 0){
    printf("caso 2: 50 segundos \n");
    tx_var = 2;
  }

  else if(strcmp(buf, "3") == 0){
    printf("caso 3: 50 segundos \n");
    tx_var = 3;
  }

  else if(strcmp(buf, "4") == 0){
    printf("caso 4: 100 segundos \n");
    tx_var = 4;
  }
  else{ printf("bad data :( \n");    }

}

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

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback); //Comment if we don't need a rcv callback

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    


    PROCESS_YIELD();


    if (ev == serial_line_event_message)
    {
      serial_handler( (char*) data);

    }
    if(etimer_expired(&periodic_timer)) {
      LOG_INFO("Sending %u", tx_var);
      LOG_INFO_("\n");
      
      memcpy(nullnet_buf, &tx_var, sizeof(tx_var));
      nullnet_len = sizeof(tx_var);

      NETSTACK_NETWORK.output(NULL);
      etimer_reset(&periodic_timer);
    }
  }

  PROCESS_END();
}






