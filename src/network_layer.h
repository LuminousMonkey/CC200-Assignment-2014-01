/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Network Layer
 *
 * Description:
 *
 *  For taking the messages from the application, and figuring out how
 *  to get them to the needed host. Wraps the application message into
 *  packets.
 */

#ifndef NETWORK_LAYER_H_
#define NETWORK_LAYER_H_

#include "application_layer.h"

struct Packet {
  CnetAddr destination_address;
  CnetAddr source_address;

  int message_number;

  // Length of the message.
  size_t length;
  struct Message message;
};

/* Declarations */

/*
 * Take the message from the application and process it so it can be
 * routed through the network.
 */
void application_down_to_network(CnetAddr destination_address,
                                 struct Message *message, size_t length);

/*
 * Take the packet from the datalink layer, either it's for us, or we
 * forward it onto the next node.
 */
void datalink_up_to_network(struct Packet *in_packet);

#endif
