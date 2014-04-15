/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 */

#include <cnet.h>

#include "data_link_layer.h"

/*
 * Physical Ready
 *
 * Nothing much to this event, just grab the data from the physical
 * layer and pass it up.
 *
 * Either this node will accept it, pass it up further the layers, or
 * forward it back down, it's safe to have the in_frame as a stack
 * allocated variable, since it will end up being copied or processed
 * before this function ends.
 */
EVENT_HANDLER(physical_ready) {
  struct Frame in_frame;
  int in_link;
  size_t length = sizeof(struct Frame);

  // Read in the frame from the physical link.
  CHECK(CNET_read_physical(&in_link, &in_frame, &length));

  // Pass it up to the datalink layer.
  up_to_datalink_from_physical(in_link, &in_frame, length);
}
