#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdio.h>




#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		(60 * CLOCK_SECOND)

static struct simple_udp_connection broadcast_connection;



/*---------------------------------------------------------------------------*/
PROCESS(udp_process, "UDP broadcast process");
AUTOSTART_PROCESSES(&udp_process);
/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_process, ev, data)
{
  static struct etimer periodic_timer;
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  simple_udp_register(&broadcast_connection, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, receiver);

  etimer_set(&periodic_timer, 20 * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {

    uint8_t buf[2];
    buf[0] = 5;
    buf[1] = 19; 
    printf("Sending data\n");       
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);1
    printf("Sending broadcast\n");
    uip_create_linklocal_allnodes_mcast(&addr);
    simple_udp_sendto(&broadcast_connection, buf, sizeof(buf), &addr);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
