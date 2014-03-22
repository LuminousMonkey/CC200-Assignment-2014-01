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

#include "application_layer.h"
#include "network_layer.h"

// Useful macros for sizing
#define PACKET_HEADER_SIZE (sizeof(struct PACKET) - sizeof(struct MESSAGE))
#define PACKET_SIZE(p) (PACKET_HEADER_SIZE + p.length)

void application_down_to_network(CnetAddr destination_address,
                                 struct MESSAGE *message, int length) {

  // Check we can route?

  // Create the packet.
  // Wrap the message in the packet.
  struct PACKET outgoing_packet;

  outgoing_packet.destination_address = destination_address;
  outgoing_packet.source_address = nodeinfo.address;
  outgoing_packet.length = length;
  memcpy(&outgoing_packet.message, message, length);

  // Look up routing table.
  // Send packet to correct link.
}

void datalink_up_to_network(struct PACKET *in_packet) {
  // Check the destination address.
  // If it's for us, move the message up to the application layer.
  // Otherwise find out which link to push it off to and send it.

  if (1 == 1) {
    network_up_to_application(&in_packet->message, in_packet->length);
  }
}
