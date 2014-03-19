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
static int last_length = 0;

// The most recently sent message.
static struct MESSAGE *last_message;

/*
 * Application Ready
 *
 * This is called when the application is ready to send some data to a
 * node. The address will be any node except for the current one.
 */
static EVENT_HANDLER(application_ready) {
  CnetAddr destination_address;

  last_length = sizeof(struct MESSAGE);

  // Read the message from the application.
  CHECK(CNET_read_application(&destination_address,
                              (char *) last_message,
                              &last_length));

  // Disable application message generation (TODO: Why?)
  CNET_disable_application(ALLNODES);

  /* Call the network layer. */
  application_down_to_network(&destination_address,
                              nodeinfo.address,
                              last_message, last_length);
}
