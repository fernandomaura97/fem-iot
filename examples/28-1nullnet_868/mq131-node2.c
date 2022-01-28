

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "stdlib.h"

#include "dev/MQ1312.h"


#include "dev/adc-sensors.h"
#include "dev/gpio-hal.h"
#include "lib/sensors.h"

#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

////////////IPV6 definitions

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa8 }};



typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision ;
#define SENSOR_READ_INTERVAL (120*CLOCK_SECOND)
#define ADC_PIN              2
//conve
//static dest_ipaddr_t dest_ipaddr = {{0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0xb6, 0xa8}};



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
    
    /*printf("Valor de VRL:");
    putFloat(vRL, DEC3);
    printf("\n");
*/
    float rS = (5.0 / vRL - 1.0) * 1000000.0;  // 1MOhm == R0 hardcoded value, fix with calibration

    /*printf("Valor de RS - hardc:");
    putFloat(rS, DEC3);
    printf("\n"); */ 
    return rS;

    }
void calibrate(){

  //wait until adc stabilyzed, then assign lastRsvalue to R0

  
  float fmeasurement = ReadRs();

  calib_s.R0 = fmeasurement; 
  printf("Valor de R0:");
  putFloat(calib_s.R0, DEC3);
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

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  
  static struct etimer timy;
  static char str3[100];
  PROCESS_BEGIN()
  
 

	//MQ131_sensor.configure(S/if((uint32_t)lastRsValue != (uint32_t)value && (uint32_t)lastLastRsValue != (uint32_t)value){ ////////////PROBAR PROBAR 

	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);

  printf("Calibrando...\n");
  calibrate(); 
  printf("Calibrado!\n");
    
  while(1) {
  etimer_set(&timy, SENSOR_READ_INTERVAL);
  
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timy));
     
  float rs = ReadRs();
   
  float ppm = getppm(rs);
  char ppm_[20]; 
  char fractional_part[5];

  sprintf(fractional_part, "%d", (int)((ppm - (int)ppm + 1.0) * 1000));
  snprintf(ppm_, sizeof(ppm_),"%d.%s", (int)ppm, &fractional_part[1]);
 
  /*sprintf(fractional_part, "%d", (int)((ppb - (int)ppb + 1.0) * 1000));
  snprintf(ppb_, sizeof(ppb_),"ppb: %d.%s\n", (int)ppb, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((mg_m3 - (int)mg_m3 + 1.0) * 1000));
  snprintf(mg_m3_, sizeof(mg_m3_),"mg/m3: %d.%s\n", (int)mg_m3, &fractional_part[1]); // 

  sprintf(fractional_part, "%d", (int)((ug_m3 - (int)ug_m3 + 1.0) * 1000));
  snprintf(ug_m3_, sizeof(ug_m3_),"ug/m3: %d.%s\n", (int)ug_m3, &fractional_part[1]); // 
  */
  //printf("ppm: ");
  //putFloat(ppm, DEC2); // 
  
  

  snprintf(str3, strlen(str3), "{\"nodeID\": 6, \"ppm\": %s} ", ppm_); //last character is left out? :(
  memcpy(nullnet_buf, &str3, sizeof(str3));
  nullnet_len = sizeof(str3);
      
  NETSTACK_NETWORK.output(&coordinator_addr);    

    
  }

  PROCESS_END();
}

