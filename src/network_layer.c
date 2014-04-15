/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Network Layer
 *
 * Description:
 *   For details about what public functions are for, check the header
 *   file.
 */

#include <cnet.h>
#include <string.h>

#include "application_layer.h"
#include "network_layer.h"
#include "data_link_layer.h"

/*
 * Routing Table
 *
 * There's no need for dynamic routing, so we will just have a
 * hardcoded static table for routing. This, and the NUM_NODES
 * constant must be updated if there's any changes in the topography
 * file.
 *
 * The routing table is a simple matrix, the first index is the
 * current node address, and the second index is the destination node
 * address. This will return the link number to send the packet out
 * onto.
 */
#define NUM_NODES 5

static const int routing_table[NUM_NODES][NUM_NODES] = {{0, 1, 2, 2, 2},
                                                        {1, 0, 2, 2, 2},
                                                        {1, 2, 0, 3, 4},
                                                        {2, 2, 2, 0, 1},
                                                        {2, 2, 2, 1, 0}};

/*
 * Forward function declarations.
 */
static int link_to_use(const struct Packet *const in_packet);
static size_t packet_size(const struct Packet *const packet);

void application_down_to_network(const CnetAddr destination_address,
                                 const struct Message *const message,
                                 const size_t length) {

  // Stack automatic safe because it gets copied.
  struct Packet outgoing_packet;

  // Build the packet.
  outgoing_packet.destination_address = destination_address;
  outgoing_packet.source_address = nodeinfo.address;
  outgoing_packet.length = length;
  memcpy(&outgoing_packet.message, message, length);

  // Routing table lookup.
  down_to_datalink_from_network(link_to_use(&outgoing_packet),
                                &outgoing_packet,
                                packet_size(&outgoing_packet));
}

void datalink_up_to_network(const struct Packet *const in_packet) {
  if (in_packet->destination_address == nodeinfo.address) {
    // Packet is for this node.
    network_up_to_application(&in_packet->message, in_packet->length);
  } else {
    // Not for this node, forward it on.
    printf("Forwarding packet for Node: %d\n",
           in_packet->destination_address);

    down_to_datalink_from_network(link_to_use(in_packet),
                                  in_packet,
                                  packet_size(in_packet));
  }
}

/*
 * Link to use
 *
 * Takes a packet and will return which link to send that packet out
 * onto.
 */
static int link_to_use(const struct Packet *const packet) {
  return routing_table[nodeinfo.address][packet->destination_address];
}

/*
 * Packet size
 *
 * Given a pointer to the packet, it will return the total used size
 * for the packet. This is why the application message must be at the
 * end of the Package struct.
 *
 * This was a macro, but application_down_to_network had a struct
 * directly while datalink_up_to_network uses a pointer. This resulted
 * in a messy (*in_packet) inside the macro args.
 */
static size_t packet_size(const struct Packet *const packet) {
  const size_t packet_header_size = sizeof(struct Packet) -
      sizeof(struct Message);

  return packet_header_size + packet->length;
}
