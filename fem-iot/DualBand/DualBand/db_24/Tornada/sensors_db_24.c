#include "contiki.h"
#include "sys/etimer.h"
#include "dev/uart.h"
#include "dev/dht22.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include "dev/gpio-hal.h"
#include "dev/adc-sensors.h"
#include "lib/sensors.h"
#include "dev/serial-line.h"
#include "dev/leds.h"
#include "sys/log.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define ADC_PIN 2 // ADC1 utilitza el pin 5 i ADC3 utilitza el pin 2. Es pot veure en el datasheet
#define SENSOR_READ_INTERVAL (CLOCK_SECOND / 8)
/*---------------------------------------------------------------------------*/
static struct etimer et;

/*---------------------------------------------------------------------------*/
PROCESS(sensors_db_24, "sensors db 24");
AUTOSTART_PROCESSES(&sensors_db_24);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensors_db_24, ev, data){

  PROCESS_BEGIN();
 


  NETSTACK_NETWORK.output(NULL);

 while(1){
  etimer_set(&et, 10*CLOCK_SECOND);
  uint8_t buffer[7] = {0, 23.2, 34.5, 102};
  nullnet_buf = (uint8_t *) &buffer;
  nullnet_len = sizeof(buffer);
  printf("Que voy!\n");
  printf("%d %d %d %d", buffer[0], buffer[1], buffer[2], buffer[3]);
    PROCESS_YIELD(); 
    
  }
  PROCESS_END();
}
