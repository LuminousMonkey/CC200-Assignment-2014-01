/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Description:
 *   Queue for the datalink layer to queue up pending packet
 *   transmissions. This is being used instead of stopping the
 *   application layer from generating messages via the
 *   CNET_(enable/disable)_application functions. Because it bothers
 *   me.
 */

#ifndef PACKET_QUEUE_H_
#define PACKET_QUEUE_H_

#include "network_layer.h"

struct PacketQueue {
  struct PacketQueueNode *head;
  struct PacketQueueNode *tail;
};

struct PacketQueueNode {
  struct PacketQueueNode *next;
  size_t length;
  struct Packet packet;
};

void setup_queue(struct PacketQueue *queue);

/*
 * Add packet
 *
 * Adds packet to end of the queue.
 *
 * This will copy the packet to the list, so the packet that's added
 * does not need to be kept around.
 */
void add_to_queue(struct PacketQueue *queue,
                  struct Packet *to_be_added,
                  size_t length);

/*
 * Get next packet
 *
 * Remove the next packet from the queue.
 *
 * It will copy the packet into Packet struct given by out_packet
 * pointer. If there is no packet, then the function will return false.
 */
size_t next_packet(struct PacketQueue *queue, struct Packet *out_packet);

/*
 * Print Debug Info
 *
 * When called, will output the current information about the given queue.
 */
void debug_print_queue(struct PacketQueue *queue);

#endif
