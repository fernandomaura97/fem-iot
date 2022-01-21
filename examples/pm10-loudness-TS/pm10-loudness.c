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
#include "dev/adc-sensors.h"
#include "dev/zoul-sensors.h"
#include "lib/sensors.h"
#include "dev/sys-ctrl.h"
#include "dev/pm10-sensor.h"


#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"

#include "sys/log.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define ADC_PIN1              5
#define ADC_PIN2	      2


char pm10s[20];
char louds[20];
char msms[100];
static struct simple_udp_connection udp_conn;
/*---------------------------------------------------------------------------*/
PROCESS(remote_pm10, "pm10 + loudness sensor");
AUTOSTART_PROCESSES(&remote_pm10);
/*---------------------------------------------------------------------------*/
static struct etimer et;

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
PROCESS_THREAD(remote_pm10, ev, data)
{
  
  uip_ipaddr_t dest_ipaddr;
   uint8_t is_reachable, root_found;


  PROCESS_BEGIN();

  
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
  
  static uint16_t pm10_value;
  uint16_t loudness;
  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1);
  pm10.configure(SENSORS_ACTIVE, ADC_PIN2);

  /* Let it spin and read sensor data */

  while(1) {
    etimer_set(&et, 8*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);
    
    if(loudness != ADC_WRAPPER_ERROR) {
      printf("%u\n", loudness);
      snprintf(louds, sizeof(louds), "\"Noise\": %u", loudness);
    } else {
      printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
      PROCESS_EXIT();
    }

    pm10_value = pm10.value(1);
    
    
    if(pm10_value != ADC_WRAPPER_ERROR) {
      printf("PM10 value = %u ppm\n", pm10_value);
       snprintf(pm10s, sizeof(pm10s), "\"PM10\": %u", pm10_value);
    } else {
      printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
      PROCESS_EXIT();
    }
    
     is_reachable = NETSTACK_ROUTING.node_is_reachable();
    root_found = NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);

    if(is_reachable && root_found) {
    
    	    snprintf(msms,sizeof(msms), "{%s , %s} ", louds, pm10s);  //prepare JSON string
	    simple_udp_sendto(&udp_conn, msms, strlen(msms), &dest_ipaddr); 
    }    else {
  	    LOG_INFO("Not reachable yet\n");
    }
}
  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

