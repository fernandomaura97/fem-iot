#include "contiki.h"
#include "dev/adc-sensors.h"
#include <stdint.h>
#include "sys/log.h"
#include "dev/gpio-hal.h"
#include "lib/sensors.h"
#include <inttypes.h>
#include "dev/MQ131.h"
#include <math.h>


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"



#define ADC_PIN              2

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
static struct simple_udp_connection udp_conn;

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
PROCESS(MQ131_TEST, "MQ131 TEST process");
AUTOSTART_PROCESSES(&MQ131_TEST);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(MQ131_TEST, ev, data)
{
  static struct etimer timer;
  uip_ipaddr_t dest_ipaddr;
  uint8_t is_reachable, root_found;

  PROCESS_BEGIN();
  //SENSORS_ACTIVATE(MQ131_sensor);
	
/*----------------------------------------------*/
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
	printf("tiempo de calientamiento = %ld  segundos", MQ_getTimeToRead());
	etimer_set(&timer, CLOCK_SECOND *20);
	
	simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

	while(1) {
		 

		  //int val = MQ131_SENSOR.value();
	          //printf("%d", val);		  
		  MQ_sample();
		  

		  float ppm = MQ_getO3(PPM);
 		  
		  char ppm_[20]; 
		//char ppb_[20]; 
		//char mg_m3_[20];
		//char ug_m3_[20];
		char fractional_part[5];

		sprintf(fractional_part, "%d", (int)((ppm - (int)ppm + 1.0) * 1000));
		snprintf(ppm_, sizeof(ppm_),"\"ppm\" : %d.%s", (int)ppm, &fractional_part[1]);
		printf("%s\n", ppm_);
		 
		
		PROCESS_PAUSE();



			
    /* Wait for the periodic timer to expire and then restart the timer. */
   //ipv6 send
	is_reachable = NETSTACK_ROUTING.node_is_reachable();
	root_found = NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);
	if(is_reachable && root_found) {
    /* Send to DAG root */
		LOG_INFO("Sending request to ");
		LOG_INFO_6ADDR(&dest_ipaddr);
		LOG_INFO_("\n");
    
        char str3[100]; 
    	snprintf(str3, sizeof(str3), "{ %s }\n"
    	, ppm_);

        simple_udp_sendto(&udp_conn, str3, strlen(str3), &dest_ipaddr);
        
      } else {
        LOG_INFO("Not reachable yet\n");
      }
  }
  PROCESS_END();
}



/*---------------------------------------------------------------------------*/
