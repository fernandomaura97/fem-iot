

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "sys/log.h"
#include "dev/MQ131.h"


#include "dev/adc-sensors.h"
#include "dev/gpio-hal.h"
#include "lib/sensors.h"

////////////IPV6 definitions

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678


static struct simple_udp_connection udp_conn;


#define SENSOR_READ_INTERVAL (60*CLOCK_SECOND)


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
 
 //static struct etimer timer;
 static struct etimer timm;
  
  //static char str[200];
  //static char str[100];
  //static char str2[200];
  
  PROCESS_BEGIN();
  //SENSORS_ACTIVATE(MQ131_sensor);

//Setup MQ131

	// Init the sensor
	// - Heater control on pin 2
	// - Sensor analog read on pin A0
	// - Model LOW_CONCENTRATION
	// - Load resistance RL of 1MOhms (1000000 Ohms)


	//MQ131_sensor.configure(SENSORS_ACTIVE, ADC_PIN);
	//adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);
  //adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);
  
  etimer_set(&timm, CLOCK_SECOND * 15);

  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timm));

	//printf("ADC3 = %d raw\n", adc_zoul.value(ZOUL_SENSORS_ADC3));

	MQbegin(2,2,1000000);

  //calibrate using temp, humidity and switch on heater...
		


simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);


while(1) {
etimer_set(&timm, SENSOR_READ_INTERVAL);

PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timm));   

MQ_sample();
float ppm;
ppm = MQ_getO3(PPM);

char ppm_[20]; 
 
char fractional_part[10];
sprintf(fractional_part, "%d", (int)((ppm - (int)ppm + 1.0) * 1000));
snprintf(ppm_, sizeof(ppm_),"\"ppm\" : %d.%s", (int)ppm, &fractional_part[1]);
 

if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      LOG_INFO("Sending request to ");
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      
	snprintf(str3, sizeof(str3), "{\"NodeID \":6, %s }\n"
	, ppm_);
  printf("%s",str3);
  simple_udp_sendto(&udp_conn, str3, strlen(str3), &dest_ipaddr);   
} 
else {
      LOG_INFO("Not reachable yet\n");
    }
  }

  PROCESS_END();
}




