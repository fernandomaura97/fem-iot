#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"


PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);



 typedef struct sensor_data_t {  
          uint8_t id1;
          uint8_t length1;
          int16_t value_temperature1;
          int16_t value_humidity1;

          uint8_t id2;
          uint8_t length2;
          int16_t value_temperature2;
          int16_t value_humidity2;
          
          } sensor_data_t;

  static struct sensor_data_t sensor_data;





/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{   //char *header;
    
  PROCESS_BEGIN();
    char delimitador[] = ",";
    char string[] = "P0,1,5,245,0,5,3,2,5,231,0,189,3"; //TODO : LENGTH IS -1 THAN IT SHOULD
    //char string2[] = "1,5,245";
    printf("string: %s\n", string);
    uint8_t buffer_aggregated[12];
    //char buffer_header[12];
    //divide the string into tokens and assign the values into the buffer
    
    char *token = strtok(string, delimitador);
    

    printf("token: %s\n", token);
   
    int i=0;
    while(token != NULL) {
        // printf("token: %s \n", token);
        token = strtok(NULL, delimitador);
        buffer_aggregated[i] = atoi(token);
        printf("buffer_aggregated[%d]: %d\n ", i, buffer_aggregated[i]);
        i++;
        if(i==12){break;}

    }
   
  
  printf("\n");

    memcpy(&sensor_data.id1, &buffer_aggregated[0], sizeof(buffer_aggregated[0]));
    memcpy(&sensor_data.length1, &buffer_aggregated[1], sizeof(buffer_aggregated[1]));
    memcpy(&sensor_data.value_temperature1, &buffer_aggregated[2], sizeof(buffer_aggregated[2]));
    memcpy(&sensor_data.value_humidity1, &buffer_aggregated[4], sizeof(buffer_aggregated[4]));
    
    memcpy(&sensor_data.id2, &buffer_aggregated[6], sizeof(buffer_aggregated[6]));
    memcpy(&sensor_data.length2, &buffer_aggregated[7], sizeof(buffer_aggregated[7]));
    memcpy(&sensor_data.value_temperature2, &buffer_aggregated[8], sizeof(buffer_aggregated[8]));
    memcpy(&sensor_data.value_humidity2, &buffer_aggregated[10], sizeof(buffer_aggregated[10]));

    printf("ID1: %d\n", sensor_data.id1);
    printf("Length1: %d\n", sensor_data.length1);
    printf("Value_temperature1: %02d.%02d\n", (sensor_data.value_temperature1)/10, (sensor_data.value_temperature1)%10);
    printf("Value_humidity1: %02d.%02d\n", (sensor_data.value_humidity1)/10, (sensor_data.value_humidity1)%10);
    
    printf("ID2: %d\n", sensor_data.id2);
    printf("Length2: %d\n", sensor_data.length2);
    printf("Value_temperature2: %02d.%02d\n", (sensor_data.value_temperature2)/10, (sensor_data.value_temperature2)%10);
    printf("Value_humidity2: %02d.%02d\n", (sensor_data.value_humidity2)/10, (sensor_data.value_humidity2)%10);

  
  

PROCESS_END();
}