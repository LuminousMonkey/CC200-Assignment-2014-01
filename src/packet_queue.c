/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet_queue.h"

// Forward declarations
static struct PacketQueueNode *create_new_node(
    const struct Packet *const to_copy,
    const size_t length);

void setup_queue(struct PacketQueue *queue) {
  queue->head = NULL;
  queue->tail = NULL;
}

void add_to_queue(struct PacketQueue *const queue,
                  const struct Packet *const to_be_added,
                  const size_t length) {
  if (queue->tail != NULL) {
    // Something already in the queue, add to tail.
    queue->tail->next = create_new_node(to_be_added,length);
    queue->tail = queue->tail->next;
  } else {
    // If it's NULL, we don't have a tail, so the queue must be empty.
    queue->head = queue->tail = create_new_node(to_be_added,length);
  }
}

size_t next_packet(struct PacketQueue *const queue,
                   struct Packet *const out_packet) {
  size_t length = 0;

  if (queue->head != NULL) {
    memcpy(out_packet, &queue->head->packet, sizeof(struct Packet));
    length = queue->head->length;;

    struct PacketQueueNode *old_head_of_queue = queue->head;

    if (queue->head == queue->tail) {
      // We've removed the last item from the queue.
      queue->head = queue->tail = NULL;
    } else {
      queue->head = queue->head->next;
    }

    free(old_head_of_queue);
  }

  return length;
}

/*
 * Create New Node
 *
 * Given a pointer to the packet, will return a PacketQueueNode that
 * has the packet copied into it.
 *
 * to_copy - Packet to copy.
 * length - Size of the packet.
 */
static struct PacketQueueNode *create_new_node(
    const struct Packet *const to_copy,
    const size_t length) {

  struct PacketQueueNode *new_node;

  // Calloc so we can find errors a little easier.
  new_node = (struct PacketQueueNode *)calloc(1, sizeof(struct PacketQueueNode));

  // If we can't allocate memory for this, it's a serious problem
  // can't recover from.
  assert(new_node);

  new_node->next = NULL;
  new_node->length = length;
  memcpy(&new_node->packet, to_copy, sizeof(struct Packet));

  return new_node;
}
