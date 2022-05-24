#include <stdio.h>
#include "cpu.h"
#include "contiki.h"
#include "dev/leds.h"
#include "dev/adc-sensors.h"
#include "dev/zoul-sensors.h"
#include "lib/sensors.h"
#include "dev/sys-ctrl.h"
#include "dev/pm10-sensor.h"
/*---------------------------------------------------------------------------*/
#define ADC_PIN              2
#define SENSOR_READ_INTERVAL (CLOCK_SECOND)

PROCESS(test_pm10_sensor_process, "Test PM10 sensor process");
AUTOSTART_PROCESSES(&test_pm10_sensor_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_pm10_sensor_process, ev, data){


    
    PROCESS_BEGIN();

    //adc_sensors.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL );
    adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);



    while(1){
    etimer_set(&et, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    int16_t value = adc_zoul.value(ZOUL_SENSORS_ADC1);
   //ADC1
    printf("ADC1 : %d RAW\n", value);
    int16_t mv = value / 9928 * 1000;
    printf("ADC1 : %d mV\n", mv);
    
    //ADC3
    value = adc_zoul.value(ZOUL_SENSORS_ADC3);
    printf("ADC3 : %d RAW\n\n\n",value);
            
    float mv2 = value * 1000.0 / 3333.33 - 278; //CLOSE ENOUGH, ERROR +- 0.1V ss
    printf("ADC3 : %d mV\n", mv2);


   
    }


    //float V = (float)value * (3.3 / 4096);

    


    PROCESS_END();



}



