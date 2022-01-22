

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "sys/log.h"

#include "dev/i2c.h"
#include "dev/multigas.h"
#include "dev/MQ131.h"


#include "dev/adc-sensors.h"
#include "dev/gpio-hal.h"
#include "lib/sensors.h"

////////////IPV6 definitions

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#include "sys/log.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678


#define ADC_PIN              2

static struct simple_udp_connection udp_conn;


//#define SENSOR_READ_INTERVAL (10*CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
 
 static struct etimer timer;
 static struct etimer timy;
  
  static char str[200];
  //static char str2[200];
  static char str3[100];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();
  SENSORS_ACTIVATE(MQ131_sensor);

  

  /* Setup a periodic timer that expires after 10 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 10);


  
 
//Setup MQ131

	// Init the sensor
	// - Heater control on pin 2
	// - Sensor analog read on pin A0
	// - Model LOW_CONCENTRATION
	// - Load resistance RL of 1MOhms (1000000 Ohms)


	//MQ131_sensor.configure(SENSORS_ACTIVE, ADC_PIN);
	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);


	//printf("ADC3 = %d raw\n", adc_zoul.value(ZOUL_SENSORS_ADC3));

	MQbegin(2,2,1000000);

	printf("Calibrando...\n");
	MQ_calibrate();
	printf("Calibrado!\n");

clock_delay(0.1*CLOCK_SECOND);
		
///end setup


simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
   
  while(1) {
    
///Sample MQ131
		  
  static unsigned count;
  MQ_sample();
  float ppm = MQ_getO3(PPM);
  float ppb = MQ_getO3(PPB);
  float mg_m3 = MQ_getO3(MG_M3);	
  float ug_m3 = MQ_getO3(UG_M3);
  
  char ppm_[20]; 
  char ppb_[20]; 
  char mg_m3_[20];
  char ug_m3_[20];
 
  char fractional_part[10];
  sprintf(fractional_part, "%d", (int)((ppm - (int)ppm + 1.0) * 1000));
  snprintf(ppm_, sizeof(ppm_),"ppm: %d.%s\n", (int)ppm, &fractional_part[1]);
 
  sprintf(fractional_part, "%d", (int)((ppb - (int)ppb + 1.0) * 1000));
  snprintf(ppb_, sizeof(ppb_),"ppb: %d.%s\n", (int)ppb, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((mg_m3 - (int)mg_m3 + 1.0) * 1000));
  snprintf(mg_m3_, sizeof(mg_m3_),"mg/m3: %d.%s\n", (int)mg_m3, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((ug_m3 - (int)ug_m3 + 1.0) * 1000));
  snprintf(ug_m3_, sizeof(ug_m3_),"ug/m3: %d.%s\n", (int)ug_m3, &fractional_part[1]); // 
  
  printf("ppm: %d.%s\n", (int)ppm, &fractional_part[1]); // 
  printf("ppb: %d.%s\n", (int)ppb, &fractional_part[1]); // 
  printf("mg/m3: %d.%s\n", (int)mg_m3, &fractional_part[1]); // 
  printf("ug/m3: %d.%s\n", (int)ug_m3, &fractional_part[1]); // 
  


 etimer_set(&timy, CLOCK_SECOND * 3);

 PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timy));
 etimer_reset(&timy);
    


//ipv6 send
if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      LOG_INFO("Sending request %u to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
     
      
      snprintf(str3, sizeof(str3), "MQ131: %s %s %s %s \n", ppm_, ppb_, mg_m3_, ug_m3_);

    
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      simple_udp_sendto(&udp_conn, str3, strlen(str3), &dest_ipaddr);
      //simple_udp_sendto(&udp_conn, str2, strlen(str2), &dest_ipaddr);
      count++;
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}




