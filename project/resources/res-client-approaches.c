#include "contiki.h"
#include <string.h>
#include "rest-engine.h"
#include "extern_var.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_client_approaches,
         "title=\"CLIENT APPROACHES\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	printf("Client approaches the door.\n");
	client_approaches = 1;
}
