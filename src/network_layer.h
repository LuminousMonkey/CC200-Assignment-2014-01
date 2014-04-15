/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Network Layer
 *
 * Description:
 *
 *  Called from the application layer, wraps the message in a packet
 *  and determines which link the packet needs to be pushed out onto.
 *
 */

#ifndef NETWORK_LAYER_H_
#define NETWORK_LAYER_H_

#include "application_layer.h"

/*
 * The network layer needs to know where something is going in order
 * to send it off on the correct link. In this implimentation we don't
 * need the source address, but it's included here for debugging.
 *
 * The application's message is copied into the frame, so we don't
 * have to worry.
 */
struct Packet {
  CnetAddr destination_address;
  CnetAddr source_address;

  size_t length; // Length of the message.

  // Be sure to keep this last in the struct, check the package_size
  // function for details why.
  struct Message message;
};

/*
 * Take the message from the application and process it so it can be
 * routed through the network.
 */
void application_down_to_network(const CnetAddr destination_address,
                                 const struct Message *const message,
                                 const size_t length);

/*
 * Take the packet from the datalink layer, either it's for us, or we
 * forward it onto the next node.
 */
void datalink_up_to_network(const struct Packet *const in_packet);

#endif
