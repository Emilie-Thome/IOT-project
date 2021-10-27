
#include "contiki.h"

#if PLATFORM_HAS_LIGHT

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "extern_var.h"
#include "dev/light-sensor.h"
#include "dev/leds.h"

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static void turn_lamp_on();

RESOURCE(res_alarm_door,
         "title=\"ALARM\"",
         res_post_handler,
         NULL,
         NULL,
         NULL);

static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  printf("Someone approaches.\n");
  turn_lamp_on();
}

static void turn_lamp_on()
{
  /* Activate sensors. */
  light_sensor.configure(LIGHT_SENSOR_SOURCE, ISL29020_LIGHT__AMBIENT);
  light_sensor.configure(LIGHT_SENSOR_RESOLUTION, ISL29020_RESOLUTION__16bit);
  light_sensor.configure(LIGHT_SENSOR_RANGE, ISL29020_RANGE__1000lux);
  SENSORS_ACTIVATE(light_sensor);
  
  etimer_set(&lamp_timer, 5*CLOCK_SECOND);
  float light = (float)light_sensor.value(0);
  light = light / LIGHT_SENSOR_VALUE_SCALE;
  printf("light = %f\n", light);
  if ((!lamp_activated) && (light < 0.1)) {
    leds_on(LEDS_ALL);
    printf("The lamp is on for 5 secondes.\n");
    lamp_activated = 1;
    etimer_restart(&lamp_timer);
  }
}
#endif /* PLATFORM_HAS_LIGHT */
