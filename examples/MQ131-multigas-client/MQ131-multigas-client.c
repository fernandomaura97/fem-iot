

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
 static struct etimer timm;
  
  //static char str[200];
  //static char str[100];
  //static char str2[200];
  static char str3[500];
  uip_ipaddr_t dest_ipaddr;
  uint8_t is_reachable, root_found;

  PROCESS_BEGIN();
  SENSORS_ACTIVATE(MQ131_sensor);

  

  /* Setup a periodic timer that expires after 10 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 10);

//Setup Multichannel gas sensor  
  i2c_master_enable();
  mgsbegin(0x04);
  mgspowerOn();
 
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
	//MQ_calibrate();
	printf("Calibrado!\n");

clock_delay(0.1*CLOCK_SECOND);
		
///end setup


simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
   
  while(1) {
    
///Sample MQ131
		  
  //static unsigned count;
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
  snprintf(ppm_, sizeof(ppm_),"\"ppm\" : %d.%s", (int)ppm, &fractional_part[1]);
 
  sprintf(fractional_part, "%d", (int)((ppb - (int)ppb + 1.0) * 1000));
  snprintf(ppb_, sizeof(ppb_),"\"ppb\": %d.%s", (int)ppb, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((mg_m3 - (int)mg_m3 + 1.0) * 1000));
  snprintf(mg_m3_, sizeof(mg_m3_),"\"mg_m3\" : %d.%s", (int)mg_m3, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((ug_m3 - (int)ug_m3 + 1.0) * 1000));
  snprintf(ug_m3_, sizeof(ug_m3_),"\"ug_m3\": %d.%s", (int)ug_m3, &fractional_part[1]); // 


////////////////////////////EXPERIMENTAL///////////////////////

//////////////////////////EXPERIMENTAAAAAAAAAAAAAL&////////&
  
  /*printf("ppm: %d.%s\n", (int)ppm, &fractional_part[1]); // 
  printf("ppb: %d.%s\n", (int)ppb, &fractional_part[1]); // 
  printf("mg/m3: %d.%s\n", (int)mg_m3, &fractional_part[1]); // 
  printf("ug/m3: %d.%s\n", (int)ug_m3, &fractional_part[1]); // 
  */

//Sample Multichannel gas sensor
    i2c_master_enable();
    
	float c__; 			
	char nh3_[20];
	char co_[20];
	char no2_[20];
	char c3h8_[20]; 
	char c4h10_[20];
	char ch4_[20];
	char h2_[20];
	char c2h5oh_[20];

  
    c__ = measure_NH3();
    /*printf("The concentration of NH3 is ");
    floatsensor(c__);
    printf(" ppm\n");
    */
    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(nh3_, sizeof(nh3_)," \"nh3\": %d.%s", (int)c__, &fractional_part[1]);


    c__ = measure_CO();
    /*printf("The concentration of CO is ");
    floatsensor(c__);
    printf(" ppm\n");
*/
    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(co_, sizeof(co_),"\"co\": %d.%s", (int)c__, &fractional_part[1]);
    
    c__ = measure_NO2();
    //printf("The concentration of NO2 is ");
    //floatsensor(c__);
    //printf(" ppm\n");

    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(no2_, sizeof(no2_),"\"no2\": %d.%s", (int)c__, &fractional_part[1]);
    
    c__ = measure_C3H8();
    //printf("The concentration of C3H8 is ");
    //floatsensor(c__);
    //printf(" ppm\n");

    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(c3h8_, sizeof(c3h8_),"\"c3h8\": %d.%s", (int)c__, &fractional_part[1]);
    
    c__ = measure_C4H10();
    //printf("The concentration of C4H10 is ");
    //floatsensor(c__);
    //printf(" ppm\n");
    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(c4h10_, sizeof(c4h10_),"\"c4h10\": %d.%s", (int)c__, &fractional_part[1]);

    
    c__ = measure_CH4();
    //printf("The concentration of CH4 is ");
    //floatsensor(c__);
    //printf(" ppm\n");

    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(ch4_, sizeof(ch4_),"\"ch4\": %d.%s", (int)c__, &fractional_part[1]);
    
    c__ = measure_H2();
    //printf("The concentration of H2 is ");
    //floatsensor(c__);
    //printf(" ppm\n");

    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(h2_, sizeof(h2_),"\"h2\": %d.%s", (int)c__, &fractional_part[1]);
    //printf("H2!!!!! %s \n\n", h2_);
    
    c__ = measure_C2H5OH();
    //printf("The concentration of C2H5OH is ");
    //floatsensor(c__);
    //printf(" ppm\n");

    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(c2h5oh_, sizeof(c2h5oh_),"\"c2h5oh\": %d.%s", (int)c__, &fractional_part[1]);


etimer_set(&timm, CLOCK_SECOND * 2);

PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timm));
etimer_reset(&timm);


//ipv6 send
	is_reachable = NETSTACK_ROUTING.node_is_reachable();
    	root_found = NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);

    if(is_reachable && root_found) {
      /* Send to DAG root */
      LOG_INFO("Sending request to ");
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
     
      
      //snprintf(str3, sizeof(str3), "MQ131: %s %s %s %s \n", ppm_, ppb_, mg_m3_, ug_m3_);
      //snprintf(str, sizeof(str), "Multigas: %s %s %s %s %s %s %s %s\n", nh3_, co_,no2_, c3h8_, c4h10_, ch4_, h2_, c2h5oh_);

	////EXPERIMENTAL, PROBAR///
      
	snprintf(str3, sizeof(str3), "{ %s , %s , %s , %s , %s, %s, %s, %s, %s, %s, %s, %s}\n"
	, ppm_, ppb_, mg_m3_, ug_m3_, nh3_, co_,no2_, c3h8_, c4h10_, ch4_, h2_, c2h5oh_);


      printf("%s",str3);
      //printf("\n %s \n",str);

    
      //simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      //clock_delay(0.5*CLOCK_SECOND);
      simple_udp_sendto(&udp_conn, str3, strlen(str3), &dest_ipaddr);
      
    } else {
      LOG_INFO("Not reachable yet\n");
    }
  }

  PROCESS_END();
}




