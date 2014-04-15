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

/*
 * Event handler for timeouts when waiting for an ACK.
 */
EVENT_HANDLER(timeouts);

/*
 * The data link later has some queues that need to be setup before
 * they can be used.
 */
void init_data_link_layer();

/*
 * Just pass the physical frame up to the data link layer.
 */
void up_to_datalink_from_physical(int in_link,
                                  struct Frame *in_frame,
                                  size_t frame_length);

/*
 * Take the packet for the network layer and send it out on the
 * required link.
 */
void down_to_datalink_from_network(int out_link,
                                   struct Packet *out_packet,
                                   size_t length);

/*
 * For printing out debug information about the data link layer.
 */
void debug_data_link_layer();

#endif
