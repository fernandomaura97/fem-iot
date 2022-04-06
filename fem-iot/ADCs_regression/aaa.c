#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "cpu/cc2538/dev/uart.h"
#include "dev/button-sensor.h"
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#include "dev/serial-line.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define ON 1
#define OFF 0
/*---------------------------------------------------------------------------*/
static struct etimer et;
uint16_t counter_uart;
char buf_out[100];
char buf_in[100];
/*---------------------------------------------------------------------------*/
PROCESS(cc2538_uart_demo_process, "cc2538 uart demo");
AUTOSTART_PROCESSES(&cc2538_uart_demo_process);
/*---------------------------------------------------------------------------*/
unsigned int
uart1_send_bytes(const unsigned char *s, unsigned int len)
{
  unsigned int i = 0;

  while(s && *s != 0) {
    if(i >= len) {
      break;
    }
    uart_write_byte(1, *s++);
    i++;
  }
  return i;
}

void serial_in(){

  if (strncmp(buf_in, "OK", 2) == 0){
    // Switch ON
    printf("SWITCH ON\n");

    leds_toggle(LEDS_GREEN);

    sprintf(buf_out, "OK\r\n");
    uart1_send_bytes((uint8_t *)buf_out, sizeof(buf_out)-1);
    printf("SERIAL DATA OUT --> %s", (char *) buf_out);
    printf("***********************\n");
  }
 
}

int print_uart(unsigned char c){
	buf_in[counter_uart] = c;
	counter_uart++;
	//uart_write_byte(0,c++);
	//printf("%c",c);

	if (c == '\n'){
		printf("SERIAL DATA IN --> %s\n", (char *)buf_in);
		counter_uart = 0;
		serial_in();
	}
  return 1; 
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2538_uart_demo_process, ev, data)
{
  
  PROCESS_BEGIN();
  uart_set_input(1, print_uart);
  etimer_set(&et, CLOCK_SECOND * 4);
  leds_toggle(LEDS_RED);

  PROCESS_END();
}
