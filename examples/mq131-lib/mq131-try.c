

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "stdlib.h"

#include "sys/log.h"
#include "dev/MQ1312.h"


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


typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision ;
#define SENSOR_READ_INTERVAL (10*CLOCK_SECOND)
#define ADC_PIN              2
//conve
//static dest_ipaddr_t dest_ipaddr = {{0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0xb6, 0xa8}};

static struct simple_udp_connection udp_conn;

static struct {
  float R0;
  uint16_t stable_time;
} calib_s; //calibration struct

void putLong(long x)
{
    if(x < 0)
    {
        putchar('-');
        x = -x;
    }
    if (x >= 10) 
    {
        putLong(x / 10);
    }
    putchar(x % 10+'0');
}


void putFloat( float f, tPrecision p )
{
    long i = (long)f ;
    putLong( i ) ;
    f = (f - i) * p ;
    i = abs((long)f) ;
    if( fabs(f) - i >= 0.5f )
    {
        i++ ;
    }
    putchar('.') ;
    putLong( i ) ;
    putchar('\n') ;
}

float ReadRs(){

    uint16_t valueSensor= adc_zoul.value(ZOUL_SENSORS_ADC3);
    printf("Valor de sensor: %d\n", valueSensor);

    float fvaluesensor = valueSensor;
    float vRL = (fvaluesensor / 16384.0)* 5.0 ; //16384 bits ven adc cc25385/470
    
    printf("Valor de VRL:");
    putFloat(vRL, DEC3);
    printf("\n");

    float rS = (5.0 / vRL - 1.0) * 1000000.0;  // 1MOhm == R0 hardcoded value, fix with calibration

    printf("Valor de RS - hardc:");
    putFloat(rS, DEC3);
    printf("\n");  
    return rS;
}
void calibrate(){

  uint16_t measurement = adc_zoul.value(ZOUL_SENSORS_ADC3);
  float fmeasurement = measurement;
  calib_s.R0 = fmeasurement; 
  

  //wait until adc stabilyzed, then assign lastRsvalue to R0

  /*float lastRsvalue = 0;
  float lastlastRsvalue = 0;
  uint8_t countrow = 0;
  uint8_t count = 0; //counts how many times we have to iterate
  while(countrow < 15){

     if((abs(lastRsValue - value)> delta) && ((abs(lastLastRsValue - value))> delta)){ ////////////PROBAR PROBAR
	    lastLastRsValue = lastRsValue;
	    lastRsValue = value;
		if (countReadInRow<= 8){
			countReadInRow = 0;
		}
		else
			{countReadInRow = countReadInRow -5;}

      //printf("timmmmm: %u\n", countReadInRow);
      
			}	
			
	else { countReadInRow++;}
	count++;
	clock_wait(CLOCK_SECOND);
  calib_s.R0 = lastvalue_r; 
  calib_s.stable_time = count;
    
*/
  }


float getppm(float rs){

  float valueR0 = calib_s.R0;
  float ratio = rs / valueR0 * 0.9906; //Where 0.9906 is ratio, to be calculated through getEnvCorrectRatio()
  float ppb = 9.4783 * pow(ratio, 2.3348);
  float ppm2 = ppb / 1000.0;

  printf("putfloat ppb \n");
  putFloat( ppb, DEC3 );

  printf("putfloat ppm \n");
  putFloat( ppm2, DEC3 );

  return ppm2;  
}

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
  
  //static struct etimer timy;
  static char str3[100];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN()
  
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
  

	//MQ131_sensor.configure(S/if((uint32_t)lastRsValue != (uint32_t)value && (uint32_t)lastLastRsValue != (uint32_t)value){ ////////////PROBAR PROBAR 

	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);
    
  while(1) {
   
  //Sample MQ131

  
  MQbegin(2,2,1000000);


  printf("Calibrando...\n");
  MQ_calibrate();
  printf("Calibrado!\n");
	  
  static unsigned count;


  /*uint16_t valueSensor= adc_zoul.value(ZOUL_SENSORS_ADC3);
  printf("Valor de sensor: %d\n", valueSensor);

  float fvaluesensor = valueSensor;
  float vRL = (fvaluesensor / 16384.0)* 5.0 ; //16384 bits ven adc cc25385/470
  
  printf("Valor de VRL:");
  putFloat(vRL, DEC3);
  printf("\n");

  float rS = (5.0 / vRL - 1.0) * 1000000.0;  // 1MOhm == R0 hardcoded value, fix with calibration

  printf("Valor de RS - hardc:");
  putFloat(rS, DEC3);
  printf("\n");


  printf("valor de RS- lib:");
  putFloat(MQ_readRs(), DEC3);


  float ratio = rS / valueR0 * 0.9906; //Where 0.9906 is ratio, to be calculated through getEnvCorrectRatio()
  float ppb = 9.4783 * pow(ratio, 2.3348);
  float ppm2 = ppb / 1000.0;


  printf("putfloat ppb \n");
  putFloat( ppb, DEC3 );

  printf("putfloat ppm \n");
  putFloat( ppm2, DEC3 );
	*/
  float ppm = MQ_getO3(PPM,rS);
  char ppm_[20]; 
  char fractional_part[10];

  sprintf(fractional_part, "%d", (int)((ppm - (int)ppm + 1.0) * 1000));
  snprintf(ppm_, sizeof(ppm_)," \"ppm\": %d.%s\n", (int)ppm, &fractional_part[1]);
 
  /*sprintf(fractional_part, "%d", (int)((ppb - (int)ppb + 1.0) * 1000));
  snprintf(ppb_, sizeof(ppb_),"ppb: %d.%s\n", (int)ppb, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((mg_m3 - (int)mg_m3 + 1.0) * 1000));
  snprintf(mg_m3_, sizeof(mg_m3_),"mg/m3: %d.%s\n", (int)mg_m3, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((ug_m3 - (int)ug_m3 + 1.0) * 1000));
  snprintf(ug_m3_, sizeof(ug_m3_),"ug/m3: %d.%s\n", (int)ug_m3, &fractional_part[1]); // 
  */
  printf("ppm: %d.%s\n", (int)ppm, &fractional_part[1]); // 
  
  
//ipv6 send
if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      LOG_INFO("Sending request %u to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
     
      
      snprintf(str3, sizeof(str3), "MQ131: %s \n", ppm_);
      simple_udp_sendto(&udp_conn, str3, strlen(str3), &dest_ipaddr);
      //simple_udp_sendto(&udp_conn, str2, strlen(str2), &dest_ipaddr);
      count++;
    } else {
      LOG_INFO("Not reachable yet\n");
    }
    
  }

  PROCESS_END();
}




