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

#define DEBUG 0
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


static linkaddr_t from;
static linkaddr_t coordinator_addr = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
static clock_time_t time_until_poll;
static struct etimer radiotimer;
static struct etimer next_beacon_etimer;
static struct etimer btimer;    
static uint16_t len_msg;
static bool is_associated;

static uint8_t bitmask;

const uint8_t power_levels[3] = {0x46, 0x71, 0x7F}; // -11dB, 8dB, 14dB


/*---------------------------------------------------------------------------*/
 
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
    }DBG
}



/*---------------------------------------------------------------------------*/

PROCESS(sta_process, "STA process");
PROCESS(associator_process,"associator process");
PROCESS(radio_process, "Radio process");

AUTOSTART_PROCESSES(&sta_process, &associator_process, &radio_process);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{   
  #if DEBUG
  LOG_INFO("Callback received from ");
  LOG_INFO_LLADR(src);
  LOG_INFO("\n");
  #endif
  from = *src;
  len_msg = len;
  //uint8_t *buf = (uint8_t *)malloc(len);
  //packetbuf_dataptr(buf, data, len); //TEST THIS
  
  process_poll(&radio_process);


  
}//callback
PROCESS_THREAD(radio_process,ev,data)
{   

    static uint8_t* datapoint;
    

    PROCESS_BEGIN();
    PROCESS_YIELD();
    while(1)
    {
        
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

        //uint8_t *buf = (uint8_t *)malloc(len_msg);
        //packetbuf_dataptr(buf, data, len); //TEST THIS

        datapoint = packetbuf_dataptr();
        
        LOG_INFO("Parsing...\n");
        uint8_t frame_header = (datapoint[0] & 224) >> 5;
        uint8_t frame_type = datapoint[0] & 31; //WHAT TO DO WITH THIS¿¿¿??


        switch(frame_header ) {

        case 0:
            LOG_INFO("Beacon received\n");
            coordinator_addr = from;
            process_poll(&associator_process);
            break;

        case 1:
            LOG_INFO("Association request received ??\n"); //not supposed to be hearing these
            break;

        case 2: 
            LOG_INFO("Association response received\n");
        
            if(from == coordinator_addr) {
                is_associated = true;
                printf("associated nowwww\n");
            }
            
            /// ASSOC_ACK = TRUE         
            break;

        case 3:
        printf("poll request received\n");
        if(is_associated) {
            process_poll(&sta_process);    
            }
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
        free(datapoint);
    } //while
    PROCESS_END();
    
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(sta_process, ev,data){

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
    mydata.co = 1.23;
    mydata.no2 = 2.34;
    mydata.o3 = 3.45;
    mydata.noise = 4560;
    mydata.pm10 = 45;
    mydata.hum = 560;
    mydata.temperature = 670;


    PROCESS_BEGIN();

    PROCESS_YIELD();
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
       

    printf("POLL frame!\n");
    printf("received poll for %d, I am node %d\n", buf[1], NODEID);
    if(buf[1] == NODEID)
        {
            //printf("I'm transmitting in the %dth slot\n", buf[1]);
            datasender(buf[1]);
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&next_beacon_etimer));
            NETSTACK_RADIO.on();
            printf("radio back on, beacon in ~2s \n");
        }
        else
        {
            printf("Error: I'm awake in the %d slot and I am %d \n", buf[1], NODEID);
        }


    PROCESS_END();



}

PROCESS_THREAD(associator_process, ev,data){
    
    

    PROCESS_BEGIN();

    
    
    while(1){

        PROCESS_YIELD();

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL); //wait until beacon 

        if(B_n==0){
            etimer_set(&btimer, CLOCK_SECOND);
            }
        else if(B_n==1){
            etimer_set(&btimer, CLOCK_SECOND*0.5);
            }

        bitmask = datapoint[1];
        printf("beacon is asking for ");
        
        if(txflag == 0){
            for (i = 0; i < 8; i++) {
                if (bitmask & (1 << i)) {
                    printf("%d \t", (i+1));
                    if((i+1) == NODEID)
                    {   
                        i_buf = i;
                        txflag = 1;
                    }
                }          
              
            }
        printf("\n");
        }
        while(!is_associated)
            {
            //augment TX POWER and send again. If no response, "poison"
            NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, power_levels[ix]);
            printf("tx power: %d\n", power_levels[ix]);

            /*---------------------------------------------------------------------------*/
            //WHAT TO SEND IN HERE? association request: 0x0

            /*---------------------------------------------------------------------------*/
            nullnet_buf = (uint8_t *)buf;
            nullnet_len = sizeof(*buf);
            NETSTACK_NETWORK.output(&coordinator_addr);

            ix++;
            if(ix >= 2)
            {
                LOG_INFO("Association process failed\n");
                //POISON!
                /*---------------------------------------------------------------------------*/
                break; 
            }
            etimer_set(&asotimer, 5* CLOCK_SECOND);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&asotimer));
   
            } //while
    
        if(B_n != 2) 
        {
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&btimer));

        }
        clock_time_t bufvar = 357*CLOCK_SECOND;
        printf("setting timer for %lu ticks, %lu seconds (+3) until beacon\n", bufvar, (bufvar/CLOCK_SECOND));

        etimer_set(&next_beacon_etimer, (357*CLOCK_SECOND)); //use rtimer maybe?

        uint8_t *buf = (uint8_t *)malloc(len_msg);

        if(txflag) {
            printf("I'm transmitting in the %dth slot\n", (i_buf+1));
            
            time_until_poll = T_MDB + ((NODEID-1) * (T_SLOT + T_GUARD)) - T_GUARD;
            printf("radio off, time until radio on: %lu ticks, %lu seconds\n", time_until_poll ,time_until_poll/CLOCK_SECOND);              
            //NETSTACK_RADIO.off();
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            etimer_set( &radiotimer, time_until_poll);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));
            
            NETSTACK_RADIO.on();
            
            
            //NETSTACK_MAC.on(); //test this
            
            //NETSTACK_RADIO.on();
            printf("radio back on\n");
            txflag = 0;

        }
        else{
            printf("Radio off until the next beacon\n");
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            etimer_set( &radiotimer, T_BEACON - 2*CLOCK_SECOND);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));

            NETSTACK_RADIO.on();

            printf("radio back on, beacon in ~2s\n");
        }        
        
        //CODE STARTS HERE
    }

    PROCESS_END();

}



