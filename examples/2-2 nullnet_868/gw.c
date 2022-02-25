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
#include <stdlib.h>
#include <math.h>

#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG




//static char serbuf[300];
//static uint8_t payload[64];

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

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

/*typedef struct dht22_data_t {
  uint8_t nodeid;
  uint16_t humidity;
  uint16_t temperature;
  uint16_t pm10;
  uint16_t noise;
  float o3;
  float co;
  float no2;
} dht22_data_t;
static struct dht22_data_t dht22_data;
*/

typedef union {
  struct s{
    uint8_t nodeid;
    uint16_t humidity;
    uint16_t temperature;
    uint16_t pm10;
    uint16_t noise;
    float o3;
    float co;
    float no2;
    }old;
  uint8_t data[sizeof(struct s)];
} my_data;


static char nodeid1;
static int16_t humidity1;
static int16_t temperature1;




/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("received %d bytes: \n", len);
  //log_bytes(data, len);
  
  int i;
  uint8_t *buf1 = (uint8_t *)malloc(len);
  memcpy(buf1, data, len);
  
  printf("\ncopied is: ");
  for (i = 0; i < len; i++) {
        printf("%d ", buf1[i]);
      }
  
  
  printf("\n");

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

  //memcpy(&payload, data, sizeof(data));
  memcpy(&nodeid1, data, sizeof(uint8_t));
  memcpy(&humidity1, data+1, sizeof(int16_t));
  memcpy(&temperature1, data+3, sizeof(int16_t));
  printf("nodeid: %d\n", nodeid1);
  printf("humidity: %d\n", humidity1);
  printf("temperature: %d\n", temperature1);

  union {
        float float_variable;
        uint8_t temp_array[4];
      } u;
  memcpy(&u.temp_array, data+5, sizeof(float));
  printf("o3: ");
  putFloat(u.float_variable, DEC3);



    
  }
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

  static struct etimer et;
  PROCESS_BEGIN();
  //static my_data dht22_data; 


  nullnet_set_input_callback(input_callback);
  etimer_set(&et, CLOCK_SECOND * 10);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    /*printf("buckle up buckaroo\n");
    printf("String: %s\n", serbuf);
    printf( "struct1: nodeid: %d, humidity: %d, temperature: %d\n",
      dht22_data.old.nodeid, dht22_data.old.humidity, dht22_data.old.temperature);
    printf( "struct2: nodeid: %d, humidity: %d, temperature: %d\n", nodeid1, humidity1, temperature1);
    printf("bye bye\n");
    */
    etimer_reset(&et);
  }
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
