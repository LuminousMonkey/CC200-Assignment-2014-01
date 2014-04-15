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

/*
 * Setup queue
 *
 * Sets the queue up ready to be used. Has to be called before any
 * other functions to operate on the queue.
 */
void setup_queue(struct PacketQueue *const queue);

/*
 * Add packet
 *
 * Adds packet to end of the queue.
 *
 * This will copy the packet to the list, so the packet that's added
 * does not need to be kept around.
 */
void add_to_queue(struct PacketQueue *const queue,
                  const struct Packet *const to_be_added,
                  const size_t length);

/*
 * Get next packet
 *
 * Remove the next packet from the queue.
 *
 * It will copy the packet into Packet struct given by out_packet
 * pointer. If there is no packet, then the function will return
 * false.
 */
size_t next_packet(struct PacketQueue *const queue,
                   struct Packet *const out_packet);

#endif
