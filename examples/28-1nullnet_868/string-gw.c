/*---------------------------------------------------------------------*/
/*
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

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_NONE

static char serbuf[300];
PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

typedef struct dht22_data_t {
  uint8_t nodeid;
  uint16_t humidity;
  uint16_t temperature;
} dht22_data_t;
static struct dht22_data_t dht22_data;


static char nodeid1;
static uint16_t humidity1;
static uint16_t temperature1;




/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("received %d bytes \n", len);

  /*if(len == sizeof(dht22_data_t)) {
    memcpy(&dht22_data, data, sizeof(dht22_data_t));
    memcpy(&nodeid1, data, sizeof(uint8_t));
    memcpy(&humidity1, data+1, sizeof(uint16_t));
    memcpy(&temperature1, data+3, sizeof(uint16_t));


    printf("nodeid: %d\n", nodeid1);
    printf("humidity: %d\n", humidity1);
    printf("temperature: %d\n", temperature1);

    printf("end callback\n");
    
  }
*/
  
  //snprintf(serbuf, len, "%s", (char*)data);
  //copy the contents of data to serbuf, as a string
  memcpy( &serbuf, data, len); 

  printf("%s", serbuf);
  serbuf[0]='\0'; //empty the string
   
  printf("%s\n", serbuf); //check if string is empty
  
  /*memcpy(&payload, data, sizeof(data));
  memcpy(&nodeid1, data, sizeof(uint8_t));
  memcpy(&humidity1, data+1, sizeof(uint16_t));
  memcpy(&temperature1, data+3, sizeof(uint16_t));
  printf("nodeid: %d\n", nodeid1);
  printf("humidity: %d\n", humidity1);
  printf("temperature: %d\n", temperature1);
  */

    
  }
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

  static struct etimer et;
  PROCESS_BEGIN();



  nullnet_set_input_callback(input_callback);
  etimer_set(&et, CLOCK_SECOND * 10);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("buckle up buckaroo\n");
    printf("String: %s\n", serbuf);
    printf( "struct1: nodeid: %d, humidity: %d, temperature: %d\n",
      dht22_data.nodeid, dht22_data.humidity, dht22_data.temperature);
    printf( "struct2: nodeid: %d, humidity: %d, temperature: %d\n", nodeid1, humidity1, temperature1);
    printf("bye bye\n");

    etimer_reset(&et);
  }
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
