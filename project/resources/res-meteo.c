#include "contiki.h"

#if PLATFORM_HAS_PRESSURE

#include <string.h>
#include "rest-engine.h"
#include "dev/pressure-sensor.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_meteo,
         "title=\"METEO\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /* Activate sensors. */
  SENSORS_ACTIVATE(pressure_sensor);
  
  int pressure = pressure_sensor.value(0);
  pressure = pressure / PRESSURE_SENSOR_VALUE_SCALE;

  if (pressure > 1020) 
    printf("Good or fairly good weather. Warm days, cool nights, morning frosts.\n");
  else if (pressure > 1006)
    printf("Low or high rainfall, of short duration. Cool temperatures.\n");
  else 
    printf("Rain or snow with wind. Low temperatures.\n");
}
#endif /* PLATFORM_HAS_PRESSURE */
