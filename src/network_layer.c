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
#include "data_link_layer.h"

// Number of nodes in network.
#define NUM_NODES 5

#define PACKET_HEADER_SIZE (sizeof(struct Packet) - \
                            sizeof(struct Message))
#define PACKET_SIZE(p) (PACKET_HEADER_SIZE + p.length)

/*
 * Routing Table
 *
 * There's no need for dynamic routing, so we will just have a
 * hardcoded static table for routing.
 *
 * Array based, first index is the current node number, then the
 * target node number, the number given will be the link number to
 * route the packet out onto.
 */
static int routing_table[NUM_NODES][NUM_NODES] = {{0, 1, 2, 2, 2},
                                                  {1, 0, 2, 2, 2},
                                                  {1, 2, 0, 3, 4},
                                                  {2, 2, 2, 0, 1},
                                                  {2, 2, 2, 1, 0}};

static int link_to_use(struct Packet *in_packet);

void application_down_to_network(CnetAddr destination_address,
                                 struct Message *message, size_t length) {

  struct Packet outgoing_packet;

  // Check we can route?
  outgoing_packet.destination_address = destination_address;
  outgoing_packet.source_address = nodeinfo.address;
  outgoing_packet.length = length;

  memcpy(&outgoing_packet.message, message, length);

  // Look up routing table.
  // Send packet to correct link.
  int outgoing_link = link_to_use(&outgoing_packet);

  printf("Sending packet out on link: %d.\n", outgoing_link);

  down_to_datalink_from_network(outgoing_link, &outgoing_packet,
                                PACKET_SIZE(outgoing_packet));
}

void datalink_up_to_network(struct Packet *in_packet) {
  // Check the destination address.
  // If it's for us, move the message up to the application layer.
  // Otherwise find out which link to push it off to and send it.

  if (in_packet->destination_address == nodeinfo.address) {
    printf("\t\t\t\tSource address: %d\n", in_packet->source_address);
    network_up_to_application(&in_packet->message, in_packet->length);
  } else {
    int outgoing_link = link_to_use(in_packet);

    printf("Packet for Node: %d\n", in_packet->destination_address);
    down_to_datalink_from_network(outgoing_link, in_packet,
                                  PACKET_SIZE((*in_packet)));
  }
}

/*
 * Given a packet, return the link to use.
 */
static int link_to_use(struct Packet *packet) {
  return routing_table[nodeinfo.address][packet->destination_address];
}
