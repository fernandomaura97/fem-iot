/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-dht22-test DHT22 temperature and humidity sensor test
 *
 * Demonstrates the use of the DHT22 digital temperature and humidity sensor
 * @{
 *
 * \file
 *         A quick program for testing the DHT22 temperature and humidity sensor
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/dht22.h"
#include <stdlib.h>
#include "dbg.h"
#include <math.h>

#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "sys/log.h"
#include "stdlib.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision ;

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


/*void float2Bytes(float val,uint8_t* bytes_array){
  // Create union of shared memory space
  union {
    float float_variable;
    uint8_t temp_array[4];
  } u;
  // Overwrite bytes of union with float variable
  u.float_variable = val;
  // Assign bytes to input array
  memcpy(bytes_array, u.temp_array, 4);
}*/

typedef union {
  struct s{
    uint8_t nodeid;
    uint16_t humidity;
    uint16_t temperature;
    /*uint16_t pm10;
    uint16_t noise;
    float o3;
    float co;
    float no2;*/
    }old;
  uint8_t data[sizeof(struct s)];
} my_data;



//static struct dht22_data_t dht22_data;
//static char msms[70]; //32?

//static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa4 }};


/*---------------------------------------------------------------------------*/
PROCESS(remote_dht22_process, "DHT22 test");
AUTOSTART_PROCESSES(&remote_dht22_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_dht22_process, ev, data)
{ 
  static my_data dht22_data; 
  static int16_t temperature, humidity;
  static uint8_t buf2[(sizeof(uint8_t)+2*sizeof(uint16_t)+sizeof(float))];

  
  PROCESS_BEGIN();
  
  //if( sizeof(my_data) != (sizeof(uint8_t) + 2*sizeof(uint16_t))){printf("unwanted padding");} //""""assert"""""

  nullnet_buf = (uint8_t *)&buf2; //initialize the pointer to the buffer
  nullnet_len = sizeof(buf2); //initialize the length of the buffer

  printf("sizeof(buffer) = %d\n", sizeof(buf2));

  etimer_set(&et, CLOCK_SECOND * 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  SENSORS_ACTIVATE(dht22);

  /* Let it spin and read sensor data */

  while(1) {
    etimer_set(&et, 5*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      //printf("\"Temperature\": %02d.%d, ", temperature / 10, temperature % 10);
      //printf("\"Humidity\": %02d.%d ", humidity / 10, humidity % 10);

      dht22_data.old.temperature = temperature;
      dht22_data.old.humidity = humidity;     
      dht22_data.old.nodeid = 5;

      

      printf("nodeid = %d\n", dht22_data.old.nodeid);

      buf2[0] = 5;
      buf2[1] = temperature & 0xFF;
      buf2[2] = (temperature >> 8) & 0xFF;
      buf2[3] = humidity & 0xFF;
      buf2[4] = (humidity >> 8) & 0xFF;


      /*buf2[5] = (uint8_t)(fvar & 0xFF);
      buf2[6] = (uint8_t)((fvar >> 8) & 0xFF);
      buf2[7] = (uint8_t)((fvar >> 16) & 0xFF);
      buf2[8] = (uint8_t)((fvar >> 24) & 0xFF);
    */

    
      //printf("float bytes = %d %d %d %d\n", buf2[5], buf2[6], buf2[7], buf2[8]);
      float fvar = 1.378;
      union {
        float float_variable;
        uint8_t temp_array[4];
      } u;

      u.float_variable = fvar;
      putFloat(fvar, DEC3);
      printf("float bytes = %02x %02x %02x %02x\n", u.temp_array[0], u.temp_array[1], u.temp_array[2], u.temp_array[3]);
      printf("\nbytes in decimal: %d %d %d %d\n", u.temp_array[0], u.temp_array[1], u.temp_array[2], u.temp_array[3]);

      memcpy(buf2+5, u.temp_array, 4);
    
      //float2Bytes(fvar, &bytes[0]);
      
      //remake the float
      //float fvar2 = (buf2[5] | (buf2[6] << 8) | (buf2[7] << 16) | (buf2[8] << 24));
      
      //remake the float
      /*float fvar2 = (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
      printf("float = ");
      putFloat(fvar2, DEC3);
      printf("\n");
      */      
      printf("temp = %d\t", temperature);
      printf("hum = %d\n", humidity);
      
      
      printf("\nbuf2 = %d %d %d %d %d\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);

      nullnet_buf = (uint8_t *)&buf2; //initialize the pointer to the buffer
      nullnet_len = sizeof(buf2); //initialize the length of the buffer

      printf("\nnullnet buffer = %d %d %d %d %d\n ", nullnet_buf[0], nullnet_buf[1], nullnet_buf[2], nullnet_buf[3], nullnet_buf[4]);

      NETSTACK_NETWORK.output(NULL);
      /*dht22_data.old.nodeid = 0;
      printf("\nid:0 \t");
      log_bytes(dht22_data.data, sizeof(dht22_data.data));
      
      dht22_data.old.nodeid = 255;
       printf("\nid:1 \t");
      log_bytes(dht22_data.data, sizeof(dht22_data.data));*/
         
      //printf("dht22_data.temperature: %d\n", dht22_data.old.temperature);
      //printf("dht22_data.humidity: %d\n", dht22_data.old.humidity);
      
    } else {
      printf("Failed to read the sensor\n");
    }
    
    
  }
  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

