

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

////////////IPV6 definitions

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"



#define ADC_PIN1        5
#define ADC_PIN2	      2
#define SENSOR_TYPE     0

#define SEND_INTERVAL (8 * CLOCK_SECOND)



typedef struct sensordata{
    uint32_t co_s;
    uint32_t no2_s;
    uint16_t loud_s;
    uint16_t pm10_s; 
} sensordata_t;

static sensordata_t sensordata;
bool flag = 0; 
//#define SENSOR_READ_INTERVAL (10*CLOCK_SECOND)

static linkaddr_t coordinator_addr = {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x15, 0x9f, 0x57  }};
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
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
PROCESS_THREAD(udp_client_process, ev, data)
{
 //int16_t temperature, humidity;
 static struct etimer timer;
 static unsigned count = 0;


  static uint16_t pm10_value;
  uint16_t loudness;

  
  PROCESS_BEGIN();
  
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);
  
  
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


  etimer_set(&timer, SEND_INTERVAL);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    
  //Sample Multichannel gas sensor
    i2c_master_enable();
      
    float c__; 			
    uint32_t fbits = 0;  
    c__ = measure_CO();
    
    memcpy(&fbits, &c__, sizeof fbits);

    sensordata.co_s = fbits;
    printf("The concentration of CO is ");
    floatsensor(c__);
    printf(" ppm\n");
  
    
    c__ = measure_NO2();

    memcpy(&fbits, &c__, sizeof fbits);

    sensordata.no2_s = fbits;
    
  
    printf("The concentration of NO2 is ");
    floatsensor(c__);
    printf(" ppm\n");

  //Sample Loudness sensor   
  loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);
    
  if(loudness != ADC_WRAPPER_ERROR) {
      printf("%u\n", loudness);
      sensordata.loud_s = loudness;
  } else {
      printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
      PROCESS_EXIT();
  }
//Sample PM10

pm10_value = pm10.value(1);

  if(pm10_value != ADC_WRAPPER_ERROR) {
    printf("PM10 value = %u ppm\n", pm10_value);
    sensordata.pm10_s = pm10_value;
    
} else {
    printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
    PROCESS_EXIT();
}

/*if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      //printf("\"Temperature\": %02d.%02d, ", temperature / 10, temperature % 10);
      //printf("\"Humidity\": %02d.%02d ", humidity / 10, humidity % 10);
      snprintf(temp, sizeof(temp), "\"Temperature\": %02d.%02d,  ", temperature / 10, temperature % 		10);
      snprintf(hums, sizeof(hums), "\"Humidity\": %02d.%02d ", humidity / 10, humidity % 10);
    } else {
      printf("Failed to read the sensor\n");
    }
*/

//ipv6 send
     if (flag ==1){
      LOG_INFO("Sending to ");/* Send to DAG root */
      LOG_INFO("Sending request to ");
      LOG_INFO_LLADDR(&coordinator_addr);
      LOG_INFO_("\n");

     
      //snprintf(str3,sizeof(str3), "{%s,%s,%s,%s,%s,%s}", co_, no2_, temp, hums, pm10s, louds );  //prepare JSON string
	    //snprintf(str3,sizeof(str3), "{%s,%s,%s,%s}", co_, no2_, pm10s, louds );  //prepare JSON string
      
      memcpy(nullnet_buf, &sensordata, sizeof(sensordata));
      nullnet_len = sizeof(sensordata);
      NETSTACK_NETWORK.output(&coordinator_addr);
      flag = 0;  
      }    
  else {
  	    LOG_INFO("Not transmitting\n");
    }
}
  PROCESS_END();
}




