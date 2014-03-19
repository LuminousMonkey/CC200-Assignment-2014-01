/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Network Layer
 *
 * Description:
 */

#include <cnet.h>
#include <string.h>

#include "network_layer.h"

void application_down_to_network(CnetAddr destination_address,
                                 CnetAddr source_address,
                                 struct MESSAGE *message, int length) {

  // Check we can route?

  // Create the packet.
  // Wrap the message in the packet.
  struct PACKET outgoing_packet;

  outgoing_packet.destination_address = destination_address;
  outgoing_packet.source_address = source_address;
  outgoing_packet.length = length;
  memcpy(&outgoing_packet.message, message, length);

  // Look up routing table.
  // Send packet to correct link.
}
