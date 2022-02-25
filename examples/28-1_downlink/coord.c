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
#define SEND_INTERVAL (8 * CLOCK_SECOND)

uint8_t stimer =0; 

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

void serial_handler( (char*) data) {

  if(strcmp(data, "1") == 0){
    printf("1\n");
    uint8_t 
    memcpy(nullnet_buf, &diff, sizeof(diff));
    nullnet_len = sizeof(diff);
    
    NETSTACK_NETWORK.output(&coordinator_addr);
    printf("diff: %" PRIu32 "\n", diff);
  }
  else if(strcmp(data, "2") == 0){
    printf("2\n");
    timtim = clock_time();
    printf("timtim: %" PRIu32 "\n", timtim);
  }
  else if(strcmp(data, "3") == 0){
    printf("3\n");
    clock_time_t diff = clock_time() - timtim;
    memcpy(nullnet_buf, &diff, sizeof(diff));
    nullnet_len = sizeof(diff);
    
    NETSTACK_NETWORK.output(&coordinator_addr);
    printf("diff: %" PRIu32 "\n", diff);
  }
  else if(strcmp(data, "4") == 0){
    printf("4\n");
    clock_time_t diff = clock_time() - timtim;
    memcpy(nullnet_buf, &diff, sizeof(diff));
    nullnet_len = sizeof(diff);
    
    NETSTACK_NETWORK.output(&coordinator_addr);
    printf("diff: %" PRIu32 "\n", diff);
  }
  else if(strcmp(data, "5") == 0){
    printf("5\n");
    clock_time_t diff = clock_time() - timtim;
    memcpy(nullnet_buf, &diff, sizeof(diff));
    nullnet_len = sizeof(diff);
    
    NETSTACK_NETWORK.output(&coordinator_addr);
    printf("diff: %" PRIu32 "\n", diff);
  }
  else if(strcmp(data, "6") == 0








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
      printf("data is %s\n", (char*) data);
      count =0; 
      serial_handler(); 

    }
    if(etimer_expired(&periodic_timer)) {
      count++;
      printf("count: %d\n", count);
      etimer_reset(&periodic_timer);
    }
    //Choose between 3 random values for count
    /*unsigned r = random_rand() % 5;
    count = r;
    LOG_INFO("Sending %u to ", count);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");
    
    memcpy(nullnet_buf, &count, sizeof(count));
    nullnet_len = sizeof(count);

    NETSTACK_NETWORK.output(NULL);
    etimer_reset(&periodic_timer);
    */
  }

  PROCESS_END();
}






