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

#define f_BEACON 0x00
#define f_POLL 0x01
#define f_DATA 0x02
#define f_ENERGEST 0x03

#define NODEID  2

#define T_MDB  (10 * CLOCK_SECOND)
#define T_SLOT  (1.5 * CLOCK_SECOND)
#define T_GUARD  (0.5 * CLOCK_SECOND)
#define T_BEACON (360 * CLOCK_SECOND)





/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

static linkaddr_t coordinator_addr ; 
/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
PROCESS(parser_process, "Parsing process");
AUTOSTART_PROCESSES(&nullnet_example_process, &parser_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("Callback received rx: %d\n", *(uint8_t *)data);
  if(len == sizeof(uint8_t)) {
    //printf("%d\n", *(uint8_t *)data);
    //LOG_INFO_LLADDR(src);
    //LOG_INFO_(" address \n");
    coordinator_addr = *src;
    process_poll(&parser_process);


  }
  
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  
  //static unsigned count = 0;
  //static radio_value_t txpower;

  PROCESS_BEGIN();
  nullnet_set_input_callback(input_callback);
  while(1) {
    etimer_set(&periodic_timer, 10*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
   
  }

  PROCESS_END();
}

PROCESS_THREAD(parser_process, ev, data)
{

    static struct etimer radiotimer;
    PROCESS_BEGIN();
    /*static struct mydatas {
        uint8_t NodeID;
        int16_t temperature;
        int16_t hum;
        float co ;
        float no2 ;
        float o3 ;
        uint32_t noise ; 
        uint16_t pm10;

    } mydata;

    mydata.co = 1.23;
    mydata.no2 = 2.34;
    mydata.o3 = 3.45;
    mydata.noise = 4560;
    mydata.pm10 = 45;
    mydata.hum = 560;
    mydata.temperature = 670;
*/

    static uint8_t megabuf[9];

    nullnet_buf = (uint8_t *)&megabuf;
    nullnet_len = sizeof(megabuf);

    while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    printf("\n\n");
    
    //?????????????????????? ZONE
    uint8_t* buf;
    buf = packetbuf_dataptr();	
    uint8_t* sensor_reading = (uint8_t*)buf;


    printf("PROCESS: \tReceived %d from  ", *sensor_reading);
    LOG_INFO_LLADDR(&coordinator_addr);
    LOG_INFO_(" address \n");



    uint8_t frame = buf[0];
    //uint8_t* data = buf + 1;
    uint8_t B_f = (frame & 0xC0) >> 6;
    uint8_t B_timer = frame & 0x3f; 

    printf("Beacon_frame: %d\n", B_f);
    printf("Beacon_timer: %d\n", B_timer);

    //TODO: do something with the timer, synchronize with the coordinator (r = 0.1s, 0.5s, 1s...? )

    switch(B_f) {
        case f_BEACON:
            printf("Beacon frame\n");
            
            uint8_t bitmask = buf[1];

            printf("Bitmask: %d\n", bitmask);

            bool txflag = 0;
            int i;
            for (i = 0; i < 8; i++) {
                if (bitmask & (1 << i)) {
                    printf("asking %d ", (i+1));
                    if((i+1) == NODEID)
                    {
                      printf("I'm transmitting in the %dth slot\n", (i+1));
                      txflag = 1; 
                    }
                }
            
            }
            if(txflag) {
                
                clock_time_t time_until_poll = T_MDB + ((NODEID-1) * (T_SLOT + T_GUARD)) - 0.5*CLOCK_SECOND;

                printf("radio off, time until radio on: %lu\n", time_until_poll);              
                NETSTACK_RADIO.off();
                etimer_set( &radiotimer, time_until_poll);
                PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
                NETSTACK_RADIO.on();
                printf("radio back on\n");

            }
            else{
                printf("Radio off until the next beacon\n");
                NETSTACK_RADIO.off();
                etimer_set( &radiotimer, T_BEACON - 2*CLOCK_SECOND);
                PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
                NETSTACK_RADIO.on();
                printf("radio back on, about to receive beacons\n");
            }
            break;


        case f_POLL:
            printf("Poll frame\n");


            //Here, depending on our nodeid we will send the requested data to the coord 



            break;


        case f_DATA:
            printf("Data frame\n");
            
            // STAS do nothing here
            break;
        case f_ENERGEST:
            printf("Energest frame\n");
            
            //instead of sending data, send energest metrics
            break; 

        default:
            printf("Unknown frame\n");
            break; 

    }
  
    /*      
          
    switch(buf[0]) {
        case 1:
        case 3:
            printf("Node %d, multigas\n", buf[0]);

            megabuf[0] = buf[0];
            memcpy(&megabuf[1], &mydata.co, sizeof(mydata.co));
            memcpy(&megabuf[5], &mydata.no2, sizeof(mydata.no2));
            printf("Sending %d %d %d %d %d %d %d %d %d\n", megabuf[0], megabuf[1], megabuf[2], megabuf[3], megabuf[4], megabuf[5], megabuf[6], megabuf[7], megabuf[8]);

            //make sure it's correct data
            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(&coordinator_addr); 
            

            break;
        case 2:
        case 4: 
            printf("Node %d\n, dht22", buf[0]);

            megabuf[0] = buf[0];
            memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
            memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));
            memcpy(&megabuf[5], &mydata.noise, sizeof(mydata.noise));

            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(&coordinator_addr); 
            break;
        case 5:
        case 6:
          printf("Node %d\n, Ozone\n", buf[0]);
          union {
            float float_variable;
            uint8_t temp_array[4];
          } u;

          u.float_variable = mydata.o3;

          megabuf[5] = u.temp_array[0];
          megabuf[6] = u.temp_array[1];
          megabuf[7] = u.temp_array[2];
          megabuf[8] = u.temp_array[3];
          megabuf[0] = buf[0];

          memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
          memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));

          nullnet_buf = (uint8_t *)&megabuf;
          nullnet_len = sizeof(megabuf);
          NETSTACK_NETWORK.output(&coordinator_addr); 
          break;
        case 7:
        case 8:
            printf("Node %d, PM10\n", buf[0]);

            megabuf[0] = buf[0];
            memcpy(&megabuf[1], &mydata.pm10, sizeof(mydata.pm10));

            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(&coordinator_addr); 
                  
            break;
        default:
            printf("?");
            break;
    }
*/
    }


    PROCESS_END();

}
