

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "stdlib.h"
#include "dev/MQ1312.h"
#include "dev/adc-sensors.h"
#include "lib/sensors.h"

#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

////////////IPV6 definitions

//#define LOG_MODULE "dbug"
//#define LOG_LEVEL LOG_LEVEL_INFO

//static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa4 }};

#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6

#define OWN_NODEID 5

typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision ;

#define SENSOR_READ_INTERVAL 15*CLOCK_SECOND
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
    if(vRL<1.0){
      printf("error: vRL<1.0\n");
      break;
    }
    float rS = (5.0 / vRL - 1.0) * 1000000.0;  // 1MOhm == R0 hardcoded value, fix with calibration

    printf("Valor de RS - hardc:");
    putFloat(rS, DEC3);
    printf("\n");  
    return rS;

    }

float calibrate( float rs){

  //wait until adc stabilyzed, then assign lastRsvalue to R0

  //DO SMTH IN HERE


  float fmeasurement = rs;

  //calib_s.R0 = fmeasurement; 
  //printf("Valor de R0: ");
  //putFloat(calib_s.R0, DEC3);
  return fmeasurement;
}

float getppm(float rs){

  float valueR0 = calib_s.R0;
  float ratio = rs / valueR0 * 0.9906; //Where 0.9906 is ratio, to be calculated through getEnvCorrectRatio()
  float ppb = 9.4783 * pow(ratio, 2.3348);
  float ppm2 = ppb / 1000.0;

  //printf("putfloat ppb \n");
  //putFloat( ppb, DEC3 );

  //printf("putfloat ppm \n");
  //putFloat( ppm2, DEC3 );

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
  static uint8_t buf[(sizeof(uint8_t)+sizeof(float))]; //size: nodeid + ppm

  PROCESS_BEGIN()
   
 

	//MQ131_sensor.configure(S/if((uint32_t)lastRsValue != (uint32_t)value && (uint32_t)lastLastRsValue != (uint32_t)value){ ////////////PROBAR PROBAR 
  printf("Configuring MQ131 sensor\n");
	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);

  printf("Calibrando ahora ...\n");
  float rs_c = ReadRs();

  float r0_c = calibrate(rs_c); 
  
    
  while(1) {

    printf("loop\n");

    etimer_set(&timy, SENSOR_READ_INTERVAL);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timy));
      
    float rs = ReadRs();
    
    float ppm = getppm(rs);
    
    buf[0] = OWN_NODEID;

    
    union {
      float float_variable;
      uint8_t temp_array[4];
    } u;

    u.float_variable = ppm;
    putFloat(ppm, DEC3);
    //printf("float bytes = %02x %02x %02x %02x\n", u.temp_array[0], u.temp_array[1], u.temp_array[2], u.temp_array[3]);
    printf("\nbytes in decimal: %d %d %d %d\n", u.temp_array[0], u.temp_array[1], u.temp_array[2], u.temp_array[3]);

    memcpy(buf+1, u.temp_array, sizeof(float));

    printf("\nbytes in decimal: %d %d %d %d\n", buf[1], buf[2], buf[3], buf[4]);
    
    
    memcpy(nullnet_buf, buf, sizeof(buf));
    nullnet_len = sizeof(buf);


    printf("\nnullnet buf:  %d %d %d %d\n", nullnet_buf[0], nullnet_buf[1], nullnet_buf[2], nullnet_buf[3]);
        
    NETSTACK_NETWORK.output(NULL);    //send packet

    
  }

  PROCESS_END();
}

