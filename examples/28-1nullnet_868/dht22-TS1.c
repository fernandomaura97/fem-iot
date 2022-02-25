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

#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "sys/log.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


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
  int16_t temperature, humidity;
  
  PROCESS_BEGIN();
  
  if( sizeof(my_data) != (sizeof(uint8_t) + 2*sizeof(uint16_t))){printf("unwanted padding");} //""""assert"""""

  nullnet_buf = (uint8_t *)&dht22_data; //initialize the pointer to the buffer
  nullnet_len = sizeof(my_data);

  printf("sizeof(my_data) = %d\n", sizeof(my_data));
  printf("suma: = %d\n", (sizeof(char)+2*sizeof(uint16_t)));

  etimer_set(&et, CLOCK_SECOND * 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  SENSORS_ACTIVATE(dht22);

  /* Let it spin and read sensor data */

  while(1) {
    etimer_set(&et, 5*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* The standard sensor API may be used to read sensors individually, using
     * the `dht22.value(DHT22_READ_TEMP)` and `dht22.value(DHT22_READ_HUM)`,
     * however a single read operation (5ms) returns both values, so by using
     * the function below we save one extra operation
     */

    nullnet_buf = 0;
    nullnet_len = 0; 
    if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      printf("\"Temperature\": %02d.%d, ", temperature / 10, temperature % 10);
      printf("\"Humidity\": %02d.%d ", humidity / 10, humidity % 10);

      
      dht22_data.old.temperature = temperature;
      dht22_data.old.humidity = humidity;

     
      dht22_data.old.nodeid = 5;

      /*dht22_data.old.nodeid = 0;
      printf("\nid:0 \t");
      log_bytes(dht22_data.data, sizeof(dht22_data.data));
      
      dht22_data.old.nodeid = 255;
       printf("\nid:1 \t");
      log_bytes(dht22_data.data, sizeof(dht22_data.data));*/
     
      
      printf("dht22_data.temperature: %d\n", dht22_data.old.temperature);
      printf("dht22_data.humidity: %d\n", dht22_data.old.humidity);

      
    } else {
      printf("Failed to read the sensor\n");
    }
    //if want to send struct, memcpy dht22_data to nullnet_buf
      printf("sending\n");
      int i;

      uint8_t *buf1 = (uint8_t *)malloc(sizeof(my_data));
      memcpy(buf1, &dht22_data, sizeof(my_data));
      printf("copied is: ");
      for (i = 0; i < sizeof(my_data); i++) {
        printf("%d ", buf1[i]);
      }
      printf("\n");

      //snprintf(msms,sizeof(msms), "{\"nodeID\": \"2\",  %u.%u , %u.%u}", dht22_data.temperature/10, dht22_data.temperature%10, dht22_data.humidity/10, dht22_data.humidity%10);  //prepare JSON string
      memcpy(nullnet_buf, &buf1, sizeof(my_data));
      nullnet_len = sizeof(my_data);
      printf("actual buffer is: ");
      for (i = 0; i < nullnet_len; i++) {
        printf("%d ", nullnet_buf[i]);
      }
      printf("\n");
      free(buf1);

      //printf("bytes as hex: \t");
      //log_bytes(nullnet_buf, nullnet_len);
      //printf("\n");

      
      NETSTACK_NETWORK.output(NULL);
    
  }
  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

