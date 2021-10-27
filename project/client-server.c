#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "rest-engine.h"
#include "dev/serial-line.h"
#include "extern_var.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/* FIXME: Server address of the door lamp. */
#define SERVER_NODE_DOOR(ipaddr)   uip_ip6addr(ipaddr, 0x2001, 0x660, 0x5307, 0x3110, 0, 0, 0, 0xb369)
/* FIXME: Server address of the watch. */
#define SERVER_NODE_WATCH(ipaddr)   uip_ip6addr(ipaddr, 0x2001, 0x660, 0x5307, 0x3110, 0, 0, 0, 0x3860)

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

extern resource_t res_alarm_door, res_client_approaches, res_watch_triggered, res_event;
extern char* res_serial_data;
int client_approaches = 0;
int watch_triggered = 0;
int watch_activated = 0;

PROCESS(transmitter, "Transmitter");
PROCESS(receiver, "Receiver");
AUTOSTART_PROCESSES(&receiver, &transmitter);

uip_ipaddr_t door_ipaddr;
uip_ipaddr_t watch_ipaddr;

#define NUMBER_OF_URLS 2
char *service_urls[NUMBER_OF_URLS] = { "approach_alarm" , "trigger_watch" };

void
client_chunk_handler(void *response)
{
  const uint8_t *chunk;

  int len = coap_get_payload(response, &chunk);

  printf("|%.*s", len, (char *)chunk);
}

/* Server configuration */
static void config_server()
{
  /* Initialize the REST engine. */
  rest_init_engine();
  rest_activate_resource(&res_client_approaches, "approach_door");
  rest_activate_resource(&res_watch_triggered, "trigger_watch");
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(transmitter, ev, data)
{
  PROCESS_BEGIN();

  static coap_packet_t request[1];

  SERVER_NODE_DOOR(&door_ipaddr);
  SERVER_NODE_WATCH(&watch_ipaddr);

  /* receives all CoAP messages */
  coap_init_engine();

  static struct etimer actuator;
  etimer_set(&actuator, CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();
    etimer_restart(&actuator);

    /* The client wants to approach the door. */
    if (client_approaches) {
      printf("--Toggle alarm door--\n");

      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_urls[0]);

      const char msg[] = "Toggle!";

      coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);

      PRINT6ADDR(&door_ipaddr);
      PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

      COAP_BLOCKING_REQUEST(&door_ipaddr, REMOTE_PORT, request,
                          client_chunk_handler);

      printf("\n--Done--\n");

      client_approaches = 0;
    }
    /* The client (des)activates its watch. */
    if (watch_triggered) {
      printf("--Toggle watch--\n");

      /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_urls[1]);

      const char msg[] = "Toggle!";

      coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);

      PRINT6ADDR(&watch_ipaddr);
      PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

      COAP_BLOCKING_REQUEST(&watch_ipaddr, REMOTE_PORT, request,
                          client_chunk_handler);

      printf("\n--Done--\n");

      watch_triggered = 0;
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(receiver, ev, data)
{
  PROCESS_BEGIN();

  config_server();

  while(1) {
    PROCESS_WAIT_EVENT();


    if(ev == serial_line_event_message) {
      res_serial_data = (char*)data;
      res_event.trigger();
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

