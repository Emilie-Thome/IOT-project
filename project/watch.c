#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "rest-engine.h"
#include "extern_var.h"
#include "dev/acc-mag-sensor.h"
#include "dev/leds.h"

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

/* FIXME: Server address of the meteo station. */
#define SERVER_NODE_METEOSTATION(ipaddr)   uip_ip6addr(ipaddr, 0x2001, 0x660, 0x5307, 0x3110, 0, 0, 0, 0xb768)

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

extern resource_t res_meteo, res_watch_triggered;
int watch_triggered = 0;
int watch_activated = 0;
/*
 * Smart watch that signales when the runner's acceleration decreases or increases.
 * Also prints the meteo at the beginning of the running session.
 */


PROCESS(smart_watch_receiver, "Smart watch receiver");
PROCESS(smart_watch_transmitter, "Smart watch transmitter");
AUTOSTART_PROCESSES(&smart_watch_receiver, &smart_watch_transmitter);

uip_ipaddr_t meteo_station_ipaddr;

#define NUMBER_OF_URLS 1
char *service_urls[NUMBER_OF_URLS] = { "trigger_watch" };

void
client_chunk_handler(void *response)
{
  const uint8_t *chunk;

  int len = coap_get_payload(response, &chunk);

  printf("|%.*s", len, (char *)chunk);
}

double dt = CLOCK_SECOND;
double vx = 0;
double vy = 0;
double vz = 0;
double v = 0;

/* Watch functions */
static void config_watch()
{
  /* Initialize the REST engine. */
  rest_init_engine();
  rest_activate_resource(&res_meteo, "meteo");
  rest_activate_resource(&res_watch_triggered, "trigger_watch");
  /* Activate sensors. */
  SENSORS_ACTIVATE(acc_sensor);
}

static void watch_info()
{
  int ax = acc_sensor.value(ACC_MAG_SENSOR_X);
  int ay = acc_sensor.value(ACC_MAG_SENSOR_Y);
  int az = acc_sensor.value(ACC_MAG_SENSOR_Z);
  
  vx = vx + ax * dt;
  vy = vy + ay * dt;
  vz = vz + az * dt;

  double prev_v = v;
  v = sqrt(vx*vx + vy*vy + vz*vz);

  if (v > 500000){
    if (v < prev_v*0.9)
      printf("You are slowing down too much.\n");
    else if (v > prev_v*1.1)
      printf("You are speeding up too much.\n");
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smart_watch_transmitter, ev, data)
{
  PROCESS_BEGIN();

  static coap_packet_t request[1];

  SERVER_NODE_METEOSTATION(&meteo_station_ipaddr);

  /* receives all CoAP messages */
  coap_init_engine();

  static struct etimer actuator;
  etimer_set(&actuator, CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();
    etimer_restart(&actuator); 

    if (watch_triggered) {
      printf("--Toggle meteo station--\n");

      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_urls[0]);

      const char msg[] = "Toggle!";

      coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);

      PRINT6ADDR(&meteo_station_ipaddr);
      PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

      COAP_BLOCKING_REQUEST(&meteo_station_ipaddr, REMOTE_PORT, request,
                          client_chunk_handler);

      printf("\n--Done--\n");

      watch_triggered = 0;
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smart_watch_receiver, ev, data)
{
  PROCESS_BEGIN();

  config_watch();

  static struct etimer info_timer;
  etimer_set(&info_timer, dt);

  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      if (watch_activated) {
        watch_info();
      }
      etimer_restart(&info_timer);  
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
