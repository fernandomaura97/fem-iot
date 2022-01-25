

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "sys/log.h"

#include "dev/adc-sensors.h"
#include "lib/sensors.h"

////////////IPV6 definitions

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#define ADC_PIN1  5

static uint32_t avg_loud;
static int im = 0;

//#define SENSOR_READ_INTERVAL (10*CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
static struct etimer timer;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
 //int16_t temperature, humidity;
  

  uint16_t loudness;


  PROCESS_BEGIN();
  

  etimer_set(&timer, CLOCK_SECOND * 10);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));  
  
  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1);

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    
    loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);
    printf("instant loudness: %d\n", loudness);

    avg_loud = 0; 
   
    for(im = 0; im < 100; im++)
    { 
      etimer_set(&timer, CLOCK_SECOND * 0.1);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);
      printf("instant loudness: %d\n", loudness);
      avg_loud += (adc_sensors.value(ANALOG_GROVE_LOUDNESS));
          
    }

  printf(" avg_loud %lu\n", avg_loud/100);

  /*if(loudness != ADC_WRAPPER_ERROR) {
      printf("%u\n", loudness);
  } else {
      printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
      PROCESS_EXIT();
  }
  */
}
  PROCESS_END();
}




