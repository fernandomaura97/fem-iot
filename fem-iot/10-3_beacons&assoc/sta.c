#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include "dev/radio.h"

#include "net/packetbuf.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/*
#define f_BEACON 0x00
#define f_POLL 0x01
#define f_DATA 0x02
#define f_ENERGEST 0x03
*/


#define NODEID  1

#define T_MDB  (10 * CLOCK_SECOND)
#define T_SLOT  (1.5 * CLOCK_SECOND)
#define T_GUARD  (0.5 * CLOCK_SECOND)
#define T_BEACON (360 * CLOCK_SECOND)
/*---------------------------------------------------------------------------*/

static linkaddr_t coordinator_addr; 
static struct mydatas {
        uint8_t NodeID;
        int16_t temperature;
        int16_t hum;
        float co ;
        float no2 ;
        float o3 ;
        uint32_t noise ; 
        uint16_t pm10;

    } mydata;

static bool txflag = 0;


void datasender( uint8_t id )  
{
    uint8_t megabuf[9];
    mydata.co = 1.23;
    mydata.no2 = 2.34;
    mydata.o3 = 3.45;
    mydata.noise = 4560;
    mydata.pm10 = 45;
    mydata.hum = 560;
    mydata.temperature = 670;

    nullnet_buf = (uint8_t *)&megabuf;
    nullnet_len = sizeof(megabuf);        
    switch(id) {
        case 1:
        case 3:
            printf("Node %d, multigas\n", id);

            megabuf[0] = id;
            memcpy(&megabuf[1], &mydata.co, sizeof(mydata.co));
            memcpy(&megabuf[5], &mydata.no2, sizeof(mydata.no2));
            printf("Sending %d %d %d %d %d %d %d %d %d\n", megabuf[0], megabuf[1], megabuf[2], megabuf[3], megabuf[4], megabuf[5], megabuf[6], megabuf[7], megabuf[8]);

            //make sure it's correct data
            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(NULL); 
            

            break;
        case 2:
        case 4: 
            printf("Node %d\n, dht22", id);

            megabuf[0] = id;
            memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
            memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));
            memcpy(&megabuf[5], &mydata.noise, sizeof(mydata.noise));

            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(NULL); 
            break;
        case 5:
        case 6:
          printf("Node %d\n, Ozone\n", id);
          union {
            float float_variable;
            uint8_t temp_array[4];
          } u;

          u.float_variable = mydata.o3;

          megabuf[5] = u.temp_array[0];
          megabuf[6] = u.temp_array[1];
          megabuf[7] = u.temp_array[2];
          megabuf[8] = u.temp_array[3];
          megabuf[0] = id;

          memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
          memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));

          nullnet_buf = (uint8_t *)&megabuf;
          nullnet_len = sizeof(megabuf);
          NETSTACK_NETWORK.output(NULL); 
          break;
        case 7:
        case 8:
            printf("Node %d, PM10\n", id);

            megabuf[0] = id;
            memcpy(&megabuf[1], &mydata.pm10, sizeof(mydata.pm10));

            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(NULL); 
                  
            break;
        default:
            printf("?");
          
            break;
    }
}



/*---------------------------------------------------------------------------*/

PROCESS(sta_process, "STA process");
PROCESS(associator_process,"associator process");
AUTOSTART_PROCESSES(&sta_process, &associator_process);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  LOG_DBG("Callback received from ");
  LOG_DBG_LLADR(src);
  LOG_DBG("\n");  
  
  uint8_t *buf = (uint8_t *)malloc(len);
  packetbuf_dataptr(buf, data, len); //TEST THIS
  
  switch((buf[0] & 224) >>5) {

    case 0:
        LOG_INFO("Beacon received\n");
        coordinator_addr = *src;
        process_poll(&associator_process);
        break;

    case 1:
        LOG_INFO("Association request received ??\n"); //not supposed to be hearing these
        break;

    case 2: 
        LOG_INFO("Association response received\n");
        
        /// ASSOC_ACK = TRUE         
        break;

    case 3:
        LOG_INFO("Poll request received\n");
        //process_poll(&datasender);         TODO
        break;

    case 4: 
        LOG_INFO("Sensor data received??\n"); //not supposed to be hearing these
        break;

    case 5: 
        LOG_INFO("Energest data received\n");
        break;

    default:
        LOG_INFO("Unknown packet received\n");
        break;
  } //switch
    free(buf);
}//callback


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(sta_process, ev,data){

    PROCESS_BEGIN();

    while(1){
            //TODO



    }



    PROCESS_END();



}

PROCESS_THREAD(associator_process, ev,data){

    PROCESS_BEGIN();

    while(1){

        PROCESS_YIELD();

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

        //CODE STARTS HERE
    }

    PROCESS_END();

}




