/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Application Layer
 *
 * Description:
 *   Look at the header file for details.
 */

#include <cnet.h>
#include <stdio.h>

#include "application_layer.h"
#include "network_layer.h"

EVENT_HANDLER(application_ready) {
  CnetAddr destination_address;

  /*
   * We can just use a stack allocated variable here because the lower
   * layers would have copied the message before this function exits.
   */
  struct Message outgoing_message;
  size_t length = sizeof(struct Message);

  CHECK(CNET_read_application(&destination_address,
                              &outgoing_message,
                              &length));

  application_down_to_network(destination_address,
                              &outgoing_message, length);
}

void network_up_to_application(const struct Message *const in_message,
                               size_t length) {
  CHECK(CNET_write_application((char *)in_message, &length));
}
