#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"

#include "dev/i2c.h"
#include "dev/multigas.h"
#include "dev/MQ131.h"
#include "dev/adc-sensors.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (20 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;

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
  static struct etimer periodic_timer;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
//Setup MQ131
 	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);

	MQbegin(2,2,1000000);
	MQ_calibrate();
	
        char ppm_[5]; char *ppm__ = ppm_;
        /*char ppb_[5]; char *ppb__ = ppb_;
   	char mg_m3_[5]; char *mg_m3__ = mg_m3_;
   	char ug_m3_[5]; char *ug_m3__ = ug_m3_;
*/

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    static unsigned count2;
       
    MQ_sample();
    clock_delay(0.1*CLOCK_SECOND);
    
    float ppm = MQ_getO3(PPM);
    /*float ppb = MQ_getO3(PPB);
    float mg_m3 = MQ_getO3(MG_M3);	
    float ug_m3 = MQ_getO3(UG_M3);
*/
   

  
    printf("ppm=");
    *ppm__ = floatsensor(ppm);   //printf the data formatted correctly and stores it in string format
    /*printf("\n ppb = "); 
    *ppb__ = floatsensor(ppb);
    printf("\n mg/m³ = ");
    *mg_m3__ = floatsensor(mg_m3);
    printf("\n ug/m³ = ");
    *ug_m3__ = floatsensor(ug_m3);
*/
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
	/* Send to DAG root */
	LOG_INFO("Sending request %u to ", count2);
	LOG_INFO_6ADDR(&dest_ipaddr);
	LOG_INFO_("\n");
	snprintf(str, sizeof(str), "hello %d", count2);
	simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
	count2++;
	} else {
	LOG_INFO("Not reachable yet\n");
	}



/* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
    }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
