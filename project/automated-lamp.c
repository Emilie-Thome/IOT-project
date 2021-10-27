#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "extern_var.h"
#include "dev/light-sensor.h"
#include "dev/leds.h"

extern resource_t res_alarm_door;
int lamp_activated = 0;
struct etimer lamp_timer;

/*
 * Automated lamp: print the value of the light and turn the leds on when it is dark and someone passes.
 */

PROCESS(automated_lamp, "Automated lamp");
AUTOSTART_PROCESSES(&automated_lamp);

#ifdef IOTLAB_M3

/* Lamp functions */
static void config_lamp()
{  
  /* Initialize the REST engine. */
  rest_init_engine();
  rest_activate_resource(&res_alarm_door, "approach_alarm");
  /* Activate sensors. */
  light_sensor.configure(LIGHT_SENSOR_SOURCE, ISL29020_LIGHT__AMBIENT);
  light_sensor.configure(LIGHT_SENSOR_RESOLUTION, ISL29020_RESOLUTION__16bit);
  light_sensor.configure(LIGHT_SENSOR_RANGE, ISL29020_RANGE__1000lux);
  SENSORS_ACTIVATE(light_sensor);
}

static void turn_lamp_off()
{
  leds_off(LEDS_ALL);
  printf("The lamp is turned off.\n");
  lamp_activated = 0;
}

static void lamp_info()
{
  int light_info = light_sensor.value(0);
  printf("Light: %d lux\n", light_info);
  
  if (leds_get() == 0){
    printf("Lamp state = on.\n");
  } else {
    printf("Lamp state = off.\n");
  }
}
#endif

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(automated_lamp, ev, data)
{
  PROCESS_BEGIN();

#ifdef IOTLAB_M3

  config_lamp();

  static struct etimer info_timer;
  etimer_set(&info_timer, CLOCK_SECOND);
  while(1) {
    PROCESS_WAIT_EVENT();
    if ((ev == PROCESS_EVENT_TIMER) && etimer_expired(&info_timer)) {
      lamp_info();
      etimer_restart(&info_timer);  
    }
    if ((ev == PROCESS_EVENT_TIMER) && lamp_activated && etimer_expired(&lamp_timer)) {
      turn_lamp_off();
      etimer_restart(&lamp_timer);  
    }
  }
#endif
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

