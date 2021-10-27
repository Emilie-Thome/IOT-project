#include "contiki.h"
#include <string.h>
#include "rest-engine.h"
#include "extern_var.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_watch_triggered,
         "title=\"WATCH TRIGGERRED\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	if (!watch_activated)
		printf("Watch activated.\n");
	else		
		printf("Watch disactivated.\n");
 	watch_triggered = (watch_triggered + 1) % 2;
    watch_activated = (watch_activated + 1) % 2;
}
