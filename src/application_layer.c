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

/*
 * Application Ready
 *
 * This is called when the application is ready to send some data to a
 * node. The address will be any node except for the current one.
 */
EVENT_HANDLER(application_ready) {
  CnetAddr destination_address;

  /*
   * Outgoing message.
   *
   * We're not worried about it being cleaned up when this function
   * exits because we'll just copy the message between every layer.
   * It's just easier.
   */
  struct Message outgoing_message;
  size_t length = sizeof(struct Message);

  // Read the message from the application.
  CHECK(CNET_read_application(&destination_address,
                              &outgoing_message,
                              &length));

  printf("Generated message for node: %d\n", destination_address);

  /* Call the network layer. */
  application_down_to_network(destination_address,
                              &outgoing_message, length);
}

void network_up_to_application(struct Message *in_message, size_t length) {
  printf("\t\t\t\tApplication received message.\n");
  CHECK(CNET_write_application((char *)in_message, &length));
}
