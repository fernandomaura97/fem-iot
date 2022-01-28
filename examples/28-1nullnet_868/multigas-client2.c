

/*---------------------------------------------------------------------------*/

#include "contiki.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "sys/log.h"

#include "dev/i2c.h"
#include "dev/multigas.h"

#include "dev/dht22.h"
#include "dev/pm10-sensor.h"

#include "dev/adc-sensors.h"
#include "lib/sensors.h"

#include "net/nullnet/nullnet.h"
#include "net/netstack.h"



static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa8 }};

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#define ADC_PIN1        5
#define ADC_PIN2	      2

static uint32_t avg_loud;
static int im = 0;
static float c__;


//#define SENSOR_READ_INTERVAL (10*CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
 //int16_t temperature, humidity;
  static struct etimer timer;
 
  static char str3[320];
  char pm10s[20];
  char louds[20];

  static uint16_t pm10_value;


  PROCESS_BEGIN();
  

  etimer_set(&timer, CLOCK_SECOND * 20);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));  
  
  /* Setup a periodic timer that expires after 10 seconds. Give some time to join DODAG */
  
//Setup Multichannel gas sensor  
  i2c_master_enable();
  mgsbegin(0x04);
  mgspowerOn();
 
//Setup DHT22
//SENSORS_ACTIVATE(dht22);

//Setup PM10 GP2Y10
 
  pm10.configure(SENSORS_ACTIVE, ADC_PIN2);
//Setup Noise sensor
  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1);


   
while(1) {


  etimer_set(&timer, CLOCK_SECOND * 110);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));


   avg_loud = 0; 
   
    for(im = 0; im < 100; im++)
    { 
      etimer_set(&timer, CLOCK_SECOND * 0.1);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      avg_loud += (adc_sensors.value(ANALOG_GROVE_LOUDNESS));
    }

    printf(" avg_loud %lu\n", avg_loud/100);
    snprintf(louds, sizeof(louds), "\"Noise\": %lu", avg_loud/100);

    
  //Sample Multichannel gas sensor
    i2c_master_enable();
      
     			
    char co_[20];
    char no2_[20];
    char fractional_part[5];
    
    c__ = measure_CO();

    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(co_, sizeof(co_),"\"co\": %d.%s", (int)c__, &fractional_part[1]);
    
    c__ = measure_NO2();
    /*printf("The concentration of NO2 is ");
    floatsensor(c__);
    printf(" ppm\n");
*/
    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(no2_, sizeof(no2_),"\"no2\": %d.%s", (int)c__, &fractional_part[1]);
    
  //Sample Loudness sensor   
   
  pm10_value = pm10.value(1);

  if(pm10_value != ADC_WRAPPER_ERROR) {
    printf("PM10 value = %u ppm\n", pm10_value);
    snprintf(pm10s, sizeof(pm10s), "\"PM10\": %u", pm10_value);
    } else {
    printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
    PROCESS_EXIT();
    }

    
      LOG_INFO("Sending to ");/* Send to DAG root */
      LOG_INFO("Sending request to ");
      LOG_INFO_LLADDR(&coordinator_addr);
      LOG_INFO_("\n");
      //snprintf(str3,sizeof(str3), "{%s,%s,%s,%s,%s,%s}", co_, no2_, temp, hums, pm10s, louds );  //prepare JSON string
	    snprintf(str3,sizeof(str3), "{\"nodeID\": \"3\",%s,%s,%s,%s}", co_, no2_, pm10s, louds );  //prepare JSON string
      


      memcpy(nullnet_buf, &str3, sizeof(str3));
      nullnet_len = sizeof(str3);
      
      NETSTACK_NETWORK.output(&coordinator_addr);
      
  }    
  PROCESS_END();
}