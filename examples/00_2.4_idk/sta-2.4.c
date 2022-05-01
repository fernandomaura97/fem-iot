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

#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include <stdlib.h>
#include <math.h>

#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO




//static char serbuf[300];
//static uint8_t payload[64];

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);


const linkaddr_t dest_addr = {{0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa8 }};


/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("Callback received from");
  LOG_INFO_LLADDR(src);
  printf("\n");


  uint8_t *buf = (uint8_t *)malloc(len);
  memcpy(buf, data, len);

  printf("Data: %d %d %d %d \n", buf[0], buf[1], buf[2], buf[3]);


    
  }
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  
  static struct etimer et;
  static uint8_t packetbuf[3];
  static uint8_t count = 0;
  PROCESS_BEGIN();
   
  nullnet_set_input_callback(input_callback);
  etimer_set(&et, CLOCK_SECOND * 10);
  while(1) {
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    

    count++;
    LOG_INFO("count =  %d\n", count);
    packetbuf[0] = count;
    packetbuf[1] = count;
    packetbuf[2] = count;
    nullnet_buf = &packetbuf;
    nullnet_len = sizeof(packetbuf);
    
    NETSTACK_NETWORK.OUTPUT(&dest_addr);
    LOG_INFO("Sending packet %d to", count);
    LOG_INFO_LLADDR(&dest_addr);
    LOG_INFO("\n");

    etimer_reset(&et);

  }
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
