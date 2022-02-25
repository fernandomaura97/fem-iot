#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include <inttypes.h>
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

/*typedef struct sensordata{
    float co;
    float no2;
    uint16_t loudness;
    uint16_t pm10; 
} sensordata_t;

static sensordata_t sensordata;
*/

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
 LOG_INFO("received %u bytes\n", len);

  if(len == 4) {
    uint16_t temp;
    uint16_t hum;
    memcpy(&temp, data, 2);
    memcpy(&hum, data+2, 2);
    LOG_INFO("temperature %u and humidity %u from ", temp, hum);
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");
  }
  else if (len ==12){

    uint32_t co;
    uint32_t no2;
    uint16_t loudness, pm10_value; 
    memcpy(&co, data, sizeof(float));
    memcpy(&no2, data+4, sizeof(float));
    memcpy(&loudness, data+8, sizeof(uint16_t));
    memcpy(&pm10_value, data+10, sizeof(  uint16_t));
    
    LOG_INFO("CO %" PRIu32 "and NO2 %" PRIu32 "and loudness %" PRIu16 "and pm10 %" PRIu16 "from ", co, no2, loudness, pm10_value);
    //it won't print floats...
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");

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
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    //Choose between 3 random values for count
    unsigned r = random_rand() % 2;
    
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