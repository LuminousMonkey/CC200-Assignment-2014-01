/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Data link layer.
 */

#ifndef DATA_LINK_LAYER_H_
#define DATA_LINK_LAYER_H_

#include <cnet.h>
#include <stdlib.h>

#include "network_layer.h"
#include "physical_layer.h"

#define FRAME_HEADER_SIZE (sizeof(struct FRAME) -   \
                           sizeof(struct MESSAGE))
#define FRAME_SIZE(f) (FRAME_HEADER_SIZE + f.length)

/*
 * Just pass the physical frame up to the data link layer.
 */
void up_to_datalink_from_physical(int in_link,
                                  struct FRAME *in_frame,
                                  int frame_length);

/*
 * Take the packet for the network layer and send it out on the
 * required link.
 */
void down_to_datalink_from_network(int out_link,
                                   struct PACKET *out_packet,
                                   int packet_size);
#endif
