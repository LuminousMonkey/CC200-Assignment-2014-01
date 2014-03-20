/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 */

#include <cnet.h>
#include <stdlib.h>

#include "application_layer.h"

EVENT_HANDLER(reboot_node) {
  // Allocate the memory for the application message.
  last_message = calloc(1, sizeof(struct MESSAGE));
}
