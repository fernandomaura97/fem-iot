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
#include "net/netstack.h"

#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "sys/log.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

static struct dht22_data_t {
  uint8_t nodeid;
  uint16_t humz;
  uint16_t tempz;
} dht22_data;

static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa4 }};


/*---------------------------------------------------------------------------*/
PROCESS(remote_dht22_process, "DHT22 test");
AUTOSTART_PROCESSES(&remote_dht22_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_dht22_process, ev, data)
{ 

  //static char msms[50]; //32?

  int16_t temperature, humidity;

  uint8_t payload[64] = { 0 };
  
  PROCESS_BEGIN();

  dht22_data.nodeid = 5; //nodeid
  
  etimer_set(&et, CLOCK_SECOND * 1);
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
    if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      //printf("\"Temperature\": %02d.%02d, ", temperature / 10, temperature % 10);
      //printf("\"Humidity\": %02d.%02d ", humidity / 10, humidity % 10);
      dht22_data.tempz = temperature;
      dht22_data.humz = humidity;
      
      printf("Temperature %02d.%02d ºC, ", temperature / 10, temperature % 10);
      printf("Humidity %02d.%02d RH\n", humidity / 10, humidity % 10);
      printf("%d %u %u\n", dht22_data.nodeid, dht22_data.tempz, dht22_data.humz);
    } else {
      printf("Failed to read the sensor\n");
    }
    //if want to send struct, memcpy dht22_data to nullnet_buf
      /*printf("sending\n");
      snprintf(msms,sizeof(msms),"{\"nodeID\": \"2\",  %u.%u , %u.%u \n}", (dht22_data->temperature)/10, (dht22_data->temperature)%10, (dht22_data->humidity)/10, dht22_data->humidity%10);  //prepare JSON string
      memcpy(nullnet_buf, &msms, sizeof(msms));
      nullnet_len = sizeof(msms);
      
      NETSTACK_NETWORK.output(&coordinator_addr);
      */

      etimer_set(&et, CLOCK_SECOND * 5);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      printf("sending struct\n");
      
      
      /*
      nullnet_len = 5;
      printf("sending %d bytes\n", nullnet_len);
      memcpy(nullnet_buf, (uint8_t *)&dht22_data,5);

      printf("nullnet_buf: %s\n", nullnet_buf);
      */

      payload[0] = dht22_data.nodeid;
      memcpy(&payload[1], &dht22_data.tempz, 2);
      memcpy(&payload[3], &dht22_data.humz, 2);
      memcpy(nullnet_buf, &payload, 5);
      nullnet_len = 5; 

      printf("nullnet_buf: %s\n", payload);

      NETSTACK_NETWORK.output(&coordinator_addr); 
  
  }
  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

