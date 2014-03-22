/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 */

#include <cnet.h>
#include <stdlib.h>

#include "application_layer.h"
#include "physical_layer.h"

EVENT_HANDLER(reboot_node) {
  // Allocate the memory for the application message.
  last_message = calloc(1, sizeof(struct MESSAGE));

  if (last_message != NULL) {

    CHECK(CNET_set_handler(EV_APPLICATIONREADY, application_ready, 0));
    CHECK(CNET_set_handler(EV_PHYSICALREADY, physical_ready, 0));

    // Start the traffic
    if (nodeinfo.nodenumber == 0) {
      CNET_enable_application(ALLNODES);
    }
  } else {
    printf("Error allocating message memory.\n");
  }
}
