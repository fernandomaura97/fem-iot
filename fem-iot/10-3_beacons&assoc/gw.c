#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"
#include <string.h>
#include <stdio.h> 
#include "random.h"
#include "sys/clock.h"
#include <stdlib.h>


#include "sys/log.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6
#define NODEID_PM10_1 7
#define NODEID_PM10_2 8


//placeholder struct for the data of the sensors: 
typedef struct sensor_data_t {
  uint8_t nodeid;
  int16_t humidity;
  int16_t temperature;
  uint16_t pm10;
  uint32_t noise;
  float o3;
  float co;
  float no2;
} sensor_data_t;
static sensor_data_t sensors;
static uint16_t cb_len;
static linkaddr_t from; 

#define DEBUG 0

#define ROUTENUMBER 8 //for now, then it should be bigger

static linkaddr_t addr_stas[ROUTENUMBER]; //store sta's addresses in here, for routing and sending
static linkaddr_t buffer_addr; 
const linkaddr_t addr_empty = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; //placeholder address

#define SPEED_NW 1 //Speed >1 if we want faster beacons and times ( for debugging "quicker" without changing too much code)
#define SEND_INTERVAL (220 * CLOCK_SECOND * (1/SPEED_NW))
#define BEACON_INTERVAL (360* CLOCK_SECOND * (1/SPEED_NW))
#define T_MM (10* CLOCK_SECOND  * 1/SPEED_NW)
#define T_GUARD (0.5 * CLOCK_SECOND * 1/SPEED_NW)
#define T_SLOT (1.5 * CLOCK_SECOND *  1/SPEED_NW)

#define f_BEACON 0x00
#define f_POLL 0x01
#define f_DATA 0x02
#define f_ENERGEST 0x03


//static uint8_t *buf;

/*---------------------------------------------------------------------------*/

PROCESS(coordinator_process, "fem-iot coordinator process");
//PROCESS(beacon_process, "beacon process");

PROCESS(parser_process, "Parsing process");
PROCESS(association_process, "Association process");
PROCESS(callback_process,"Callback process");

AUTOSTART_PROCESSES(&coordinator_process, &parser_process, &association_process, &callback_process);

/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{   
    #if DEBUG
    LOG_DBG("Callback received from ");
    LOG_DBG_LLADDR(src);
    LOG_DBG("\n");
    #endif

    from = *src;
    cb_len = len; //save the length of the received packet


    process_poll(&callback_process);
}



/*--------------------------------------------------------------------------------*/

PROCESS_THREAD(coordinator_process, ev,data)
{
    static struct etimer periodic_timer;
    static struct etimer guard_timer;
    static struct etimer mm_timer;
    static struct etimer beacon_timer;

    static uint8_t bitmask;
    static uint8_t beaconbuf[3];

    static clock_time_t t;
    static clock_time_t dt;
    static uint8_t pollbuf[2];
    
    PROCESS_BEGIN();

    bitmask = random_rand();

    printf("bitmask = %d\n", bitmask);


    //What do we do with this???
    beaconbuf[0] = 0x00;
    beaconbuf[1] = bitmask;
    beaconbuf[2] = 0x00;

    nullnet_buf = (uint8_t*)&beaconbuf;
    nullnet_len = sizeof(beaconbuf);
    nullnet_set_input_callback(input_callback);

    //setup time
    etimer_set(&periodic_timer, 5*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    while(1)
    {
    
        etimer_set(&beacon_timer, BEACON_INTERVAL); //set the timer for the next interval
        
        static uint8_t i;

        for (i= 0; i<3; i++)
        {
            beaconbuf[1] = bitmask; 
           
            beaconbuf[0] = i; 
            nullnet_buf = (uint8_t*)&beaconbuf;
            nullnet_len = sizeof(beaconbuf);
            NETSTACK_NETWORK.output(NULL);
            LOG_INFO("beacon %d sent, bitmask %d\n", i, beaconbuf[1]);
            
            if(i<2){
            etimer_set(&guard_timer, T_GUARD); //set the timer for the next interval
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&guard_timer));
            }
        }
        
        etimer_set(&mm_timer, T_MM); //set the timer for the next interval

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&mm_timer));

        t=clock_time();
        printf("seconds since boot: %lu\n", t/CLOCK_SECOND);
        
        for(i = 1; i<9; i++)
        {
           if(bitmask & 0x01){
               bitmask = bitmask >> 1;
                pollbuf[0] = 0b01000000;
                pollbuf[1] = i;

                nullnet_buf = (uint8_t*)&pollbuf;
                nullnet_len = sizeof(pollbuf);

                dt = clock_time() - t;
                LOG_INFO("polling node %d, dt: %lu\n", i, dt/CLOCK_SECOND);
                NETSTACK_NETWORK.output(NULL);
                
                etimer_set(&periodic_timer, T_SLOT); //set the timer for the next interval
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

                etimer_set(&periodic_timer, T_GUARD);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));


           }
              else{
                bitmask = bitmask >> 1;
                dt = clock_time() - t;
                LOG_INFO("NOT polling node %d, dt: %lu\n", i, dt/CLOCK_SECOND);
                etimer_set(&periodic_timer, T_SLOT); //set the timer for the next interval
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                
                etimer_set(&periodic_timer, T_GUARD);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                
              }
        }
        LOG_INFO("Polling finished\n");
        


         //if we need to change bitmask for next loop, do it here      
         //also use this time for uplink and downlink extra communications



        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&beacon_timer));
        
        LOG_DBG("finished loop\n");
    } //while
    
    PROCESS_END();

}

/*-----------------------------------------------------------------------------------------*/




PROCESS_THREAD( parser_process, ev, data)
{
    PROCESS_BEGIN();


    uint32_t fbuf; //float buffer
    union {
        float float_variable;
        uint8_t temp_array[4];
        } u;
  
    union{
        uint32_t u32_var;
        uint8_t temp_array[4];
        } ua;

    union{
        int16_t u16_var;
        uint8_t temp_array[2];
        } ua2;


    //variables




    while(1){

        PROCESS_YIELD();

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

        uint8_t *parsebuf;
        parsebuf = packetbuf_dataptr(); //THIS NEEDS TO BE TESTED
        /* 
        Alternatives:
            memcpy(parsebuf,data,len); //Do this on the callback


        */

        //switch(buf[0] & 31){
        switch(parsebuf[0]& 0b00011111) //last 5 bits of the first byte is for NodeID?
        {
            case NODEID_MGAS1:
            case NODEID_MGAS2:
                        
            u.temp_array[0] = parsebuf[1];
            u.temp_array[1] = parsebuf[2];
            u.temp_array[2] = parsebuf[3];
            u.temp_array[3] = parsebuf[4];
            sensors.co = u.float_variable;
           

            u.temp_array[0] = parsebuf[5];
            u.temp_array[1] = parsebuf[6];
            u.temp_array[2] = parsebuf[7];
            u.temp_array[3] = parsebuf[8];
            sensors.no2 = u.float_variable;


            printf("{\"nodeID\": %d", parsebuf[0]);
            printf(",\"co\": ");
            fbuf = sensors.co * 100;
            printf("%lu.%02lu", fbuf/100, fbuf%100);
            printf(", \"no2\": ");
            fbuf = sensors.no2 * 100;
            printf("%lu.%02lu", fbuf/100, fbuf%100);
            printf("}\n");    
            break;



        case NODEID_DHT22_1:
        case NODEID_DHT22_2:
        

            ua2.temp_array[0] = parsebuf[1];
            ua2.temp_array[1] = parsebuf[2];
            
            memcpy(&sensors.temperature, &ua2.u16_var, sizeof(int16_t)); 
        

            ua2.temp_array[0] = parsebuf[3];
            ua2.temp_array[1] = parsebuf[4];
            memcpy(&sensors.humidity, &ua2.u16_var, sizeof(int16_t));
        
            ua.temp_array[0] = parsebuf[5];
            ua.temp_array[1] = parsebuf[6];
            ua.temp_array[2] = parsebuf[7];
            ua.temp_array[3] = parsebuf[8];

            memcpy(&sensors.noise, &ua.u32_var, sizeof(uint32_t));

           
            printf("{\"nodeID\": %d", parsebuf[0]);
            printf(",\"Humidity\": %d.%d", sensors.humidity/10, sensors.humidity%10);
            printf(",\"Temperature\": %d.%d", sensors.temperature/10, sensors.temperature%10);
            printf(",\"Noise\": %lu", sensors.noise);
            printf("}\n");

           
            break;
            
        case NODEID_O3_1:
        case NODEID_O3_2:
            
            ua2.temp_array[0] = parsebuf[1];
            ua2.temp_array[1] = parsebuf[2];
            
            memcpy(&sensors.temperature, &ua2.u16_var, sizeof(int16_t)); 
        

            ua2.temp_array[0] = parsebuf[3];
            ua2.temp_array[1] = parsebuf[4];
            memcpy(&sensors.humidity, &ua2.u16_var, sizeof(int16_t));

            u.temp_array[0] = parsebuf[5];
            u.temp_array[1] = parsebuf[6];
            u.temp_array[2] = parsebuf[7];
            u.temp_array[3] = parsebuf[8];
            
           
            memcpy(&sensors.o3, &u.float_variable, sizeof(float));
            fbuf = sensors.o3 * 100;
            


            printf("{\"nodeID\": %d", parsebuf[0]);
            printf(",\"ppm\": ");
            printf("%lu.%02lu", fbuf/100, fbuf%100);
           
            printf(",\"Humidity\": %d.%d", sensors.humidity/10, sensors.humidity%10);
            printf(",\"Temperature\": %d.%d", sensors.temperature/10, sensors.temperature%10);
            printf("}\n");
            break;
        case NODEID_PM10_1:
        case NODEID_PM10_2:
           
            sensors.pm10 = (parsebuf[2] << 8) | parsebuf[1];
            printf("{\"nodeID\": %d", parsebuf[0]);
            printf(",\"pm10\": %d", sensors.pm10);
            printf("}\n");
            break;
            //AOK!!
        
        default:
            /*printf("unknown nodeID %d\n", parsebuf[0]);
            printf("BYTES copied are: ");
            for (int i = 0; i < len; i++) {
            printf("%d ", parsebuf[i]);
            */        
            break;
        } //switch
    } //while
    PROCESS_END();

}


PROCESS_THREAD(association_process,ev,data){


    PROCESS_BEGIN();
    


    while(1){

        PROCESS_YIELD();

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

        uint8_t i;
        uint8_t oldaddr = 0; 

        for (i= 0; i<ROUTENUMBER; i++){
            //if (addr_stas[i] == *buffer_addr){
            if (linkaddr_cmp(&addr_stas[i], &buffer_addr)){ //if they are the same then
            oldaddr = 1;
            printf("Address already found at pos %d\n", i);
            break;     

            }   //if we have this address in our list, we don't need to add it again                
        }

        if(!oldaddr){
            
            for(i = 0; i<ROUTENUMBER; i++){

                if(linkaddr_cmp(&addr_stas[i], &addr_empty))
                    {
                    linkaddr_copy(&addr_stas[i], &buffer_addr); //if we don't have it, add it
                    printf("Address");
                    LOG_INFO_LLADDR(&buffer_addr);
                    printf(" added at pos %d\n", i);
                    break;
                    //oldaddr = 1;     
                    }
            }
        }
 }
    
    PROCESS_END();
}

PROCESS_THREAD(callback_process,ev,data){

    PROCESS_BEGIN();
    PROCESS_YIELD();

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        uint8_t *buf = (uint8_t *)malloc(cb_len);
        buf = packetbuf_dataptr();
        
        switch ((buf[0] & 224) >> 5 ) { //check the first 3 bits of the first byte

            case 0:
                LOG_INFO("Beacon received ??\n"); //this is only for stas to hear
                break;

            case 1:
                LOG_INFO("Association request received\n");

                linkaddr_copy(&buffer_addr, &from );
                process_poll(&association_process);    
                break;

            case 2:
                LOG_INFO("Association response received ??\n"); //this is only for stas to hear
                break;

            case 3: 
                LOG_INFO("Poll request received ?? \n"); //this is only for stas to hear
                break;

            case 4:
                LOG_INFO("Sensor Data received\n");
                process_poll(&parser_process);
                break;          

            case 5: 
                LOG_INFO("Energest Data received\n");
                /*Are we going to use this really? Maybe checking the consumptions in the lab through serial monitor
                should be enough for making a model, since the network is "deterministic" */
                break;

            default:
                LOG_INFO("Unknown packet received\n");
                break;
        }
    }

    PROCESS_END();
}