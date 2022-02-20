#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"
#include <string.h>
#include <stdio.h> 
#include "random.h"
#include "sys/clock.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


/* Configuration */
#define SEND_INTERVAL (220 * CLOCK_SECOND)
#define BEACON_INTERVAL (240* CLOCK_SECOND)
#define T_MM (30* CLOCK_SECOND)
#define T_GUARD (1* CLOCK_SECOND)
#define T_SLOT (10* CLOCK_SECOND)
//struct sensor types



/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
PROCESS(parser_process, "Parsing process");

AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(uint8_t)) {

    printf("%d\n", *(uint8_t *)data);

    LOG_INFO_LLADDR(src);
    LOG_INFO_(" address \n");
    //process_poll(&parser_process); //not yet
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  //static unsigned count = 0;
  //static uint16_t sensor_nodes; 
  
  union {
    uint8_t bytes[2];
    uint16_t value;
  } sensor_type;
  
  PROCESS_BEGIN();

  random_init(299);
 
  sensor_type.value = random_rand(); //Create random sensor type
  //printf ("%d\n", sensor_type.value);
  printf("R0: %d\t, R1: %d\n", sensor_type.bytes[0], sensor_type.bytes[1]);
 
  static uint8_t sensor_type_byte;  //Get random uint8_t
  memcpy(&sensor_type_byte, &sensor_type.bytes[0], sizeof(sensor_type.bytes[0]));
  printf("Bit mask: %d\n", sensor_type_byte);

  nullnet_buf = (uint8_t *)&sensor_type_byte;
  nullnet_len = sizeof(sensor_type_byte);
  //nullnet_set_input_callback(input_callback); //Comment if we don't need a rcv callback



  
  while(1) {

    etimer_set(&periodic_timer, BEACON_INTERVAL);  //Set timer for beacon interval
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    /*--------------------------------------------------------------*/
    //Send 3 beacons
    static uint8_t i;
    for (i=0; i<3; i++) { 
      etimer_set(&periodic_timer, T_GUARD); //Time between beacons
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      NETSTACK_NETWORK.output(NULL); 
      printf("Beacon %d sent, data %d\n", (i+1), sensor_type_byte);
    }
    
    printf("waiting for measurements\n");


    etimer_set (&periodic_timer, T_MM); //Set timer for measurement interval
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    //Now, poll every node in sensor_type_byte bitmask in order to receive a measurement


    static clock_time_t t;
    static clock_time_t dt;
    t = clock_time();
    printf("seconds since boot: %lu\n", t/CLOCK_SECOND);
    for (i=0; i<8; i++) {
      if (sensor_type_byte & 0x01) {

        sensor_type_byte = sensor_type_byte >> 1;
        //printf("Polling node %d\n", i);


        nullnet_buf = (uint8_t *)&i;
        nullnet_len = sizeof(i);
        NETSTACK_NETWORK.output(NULL);
        dt = clock_time()-t;
        printf("Polling node %d, dt: %lu\n", i, dt/CLOCK_SECOND);


        etimer_set(&periodic_timer, T_SLOT); 
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
        etimer_set(&periodic_timer, T_GUARD); //Time between polls
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

      }
      else{
        sensor_type_byte = sensor_type_byte >> 1;
        dt = clock_time()-t;
        printf("Not Polling node %d, dt: %lu\n", i, dt/CLOCK_SECOND);


        etimer_set(&periodic_timer, T_SLOT); 
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
        etimer_set(&periodic_timer, T_GUARD); //Time between polls
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

      }
    }

    printf("All finished\n\n\n\n");

     sensor_type.value = random_rand(); //Create random sensor type
  //printf ("%d\n", sensor_type.value);
    printf("new R0: %d\t, R1: %d\n", sensor_type.bytes[0], sensor_type.bytes[1]);
 
     //Get random uint8_t
    memcpy(&sensor_type_byte, &sensor_type.bytes[0], sizeof(sensor_type.bytes[0]));
    printf("new Bit mask: %d\n", sensor_type_byte);

    //Choose between 3 random values for count
    /*
    unsigned r = random_rand() % 5;
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



