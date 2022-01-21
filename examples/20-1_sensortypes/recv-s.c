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

static linkaddr_t coordinator_addr = {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x15, 0x9f, 0x57  }};

typedef struct sensordata{
    uint16_t temp_s;
    uint16_t hum_s;
} sensordata_t;

static sensordata_t sensordata;

bool flag = 0; 

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) {
    unsigned count;
    memcpy(&count, data, sizeof(count));
    LOG_INFO("Received %u from ", count);
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");


    switch(count) {
        case 0: 
            printf("Caso 0\n");
            break;
        case 1:
            printf("Caso 1\n");
            break;
        case 2:
            printf("Caso 2\n");
            break;
    }
    if (count == SENSOR_TYPE)  {
        printf("beacon rx, sending data \n");
        flag = 1;
    }

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
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    if (flag ==1){
        printf("Sending now\t");
        uint16_t temperature = 24 - (random_rand() % 5);
        uint16_t humidity = random_rand() % 100;
        printf("%u\t%u\n", temperature, humidity);

        sensordata.temp_s = temperature;
        sensordata.hum_s = humidity;
        memcpy(nullnet_buf, &sensordata, sizeof(sensordata));
        nullnet_len = sizeof(sensordata);
        NETSTACK_NETWORK.output(&coordinator_addr);
        flag = 0; 
    }
    
    
   
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}