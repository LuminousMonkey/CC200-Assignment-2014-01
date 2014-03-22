/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Description:
 */

#include <cnet.h>

#include "data_link_layer.h"

EVENT_HANDLER(physical_ready) {
  struct FRAME in_frame;
  size_t length;
  int in_link;

  length = sizeof(struct FRAME);

  // Read in the frame from the physical link.
  CHECK(CNET_read_physical(&in_link, (char *)&in_frame, &length));

  // Pass it up to the datalink layer.
  up_to_datalink_from_physical(in_link, &in_frame, length);
}
