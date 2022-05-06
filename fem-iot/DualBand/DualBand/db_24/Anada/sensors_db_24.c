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

static struct data{
  int16_t temperature, humidity;
  uint16_t noise;
} data;


void mesures_sensors(){

  #ifdef DEBUG
  printf("dins mesures_sensors");
  #endif


  int16_t temperature, humidity;
  uint16_t loudness;
  uint8_t buffer[7]; // 1 byte capçalera i 2 bytes per variable

  SENSORS_ACTIVATE(dht22);

  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN);
  if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR){
    
    //Emmagatzemem a l'estructura anomenada data
    data.temperature = temperature;
    data.humidity = humidity;

    printf(",\"Temperature\": %02d.%02d", temperature / 10, temperature % 10);
    printf(", \"Humidity\": %02d.%02d", humidity / 10, humidity % 10);
  }
  else{

    //Emmagatzemem a l'estructura anomenada data
    data.temperature = 0;
    data.humidity = 0;
    printf("Failed to read the sensor\n");
  }

  loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);
  if(loudness != ADC_WRAPPER_ERROR){

    //Emmagatzemem a l'estructura anomenada data
    data.noise = loudness;
    printf(", \"Noise\": %u}", loudness);
  }
  else{
    data.noise = 0;
    printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
  }
  printf("\n");

  //Afegir capçalera. Per exemple: buffer[0] = id;
  buffer[0] = 0;
  memcpy(&buffer[1], &data.temperature, sizeof(data.temperature));
  memcpy(&buffer[3], &data.humidity, sizeof(data.humidity));
  memcpy(&buffer[5], &data.noise, sizeof(data.noise));

  nullnet_buf = (uint8_t *) &buffer;
  nullnet_len = sizeof(buffer);

  NETSTACK_NETWORK.output(NULL);
}

/*---------------------------------------------------------------------------*/
PROCESS(sensors_db_24, "sensors db 24");
AUTOSTART_PROCESSES(&sensors_db_24);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  //uint8_t* bytebuf = (uint8_t*) data;
  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  printf("Data beacon: B0 %d %d %d\n", bytebuf[0], bytebuf[1], bytebuf[2]);

  mesures_sensors();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensors_db_24, ev, data){

  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);

  while(1){

    PROCESS_YIELD(); 
    
  }
  PROCESS_END();
}
