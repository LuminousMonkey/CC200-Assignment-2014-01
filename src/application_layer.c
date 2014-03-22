/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Application Layer
 *
 * Description:
 *
 *  Cnet provides the application, this is just the layer to pass the
 *  generated application events from Cnet down to the network layer.
 */

#include <cnet.h>
#include <stdio.h>

#include "application_layer.h"
#include "network_layer.h"

/* Node variables */

// The length of the most recently send message.
static size_t last_length = 0;

/*
 * Application Ready
 *
 * This is called when the application is ready to send some data to a
 * node. The address will be any node except for the current one.
 */
EVENT_HANDLER(application_ready) {
  CnetAddr destination_address;

  last_length = sizeof(struct MESSAGE);

  // Read the message from the application.
  CHECK(CNET_read_application(&destination_address,
                              (char *) last_message,
                              &last_length));

  // Disable application message generation (TODO: Why?)
  CNET_disable_application(ALLNODES);

  /* Call the network layer. */
  application_down_to_network(destination_address,
                              last_message, last_length);
}

void network_up_to_application(struct MESSAGE *in_message, size_t length) {
  printf("Application received message.\n");
  CHECK(CNET_write_application((char *)in_message, &length));
}
