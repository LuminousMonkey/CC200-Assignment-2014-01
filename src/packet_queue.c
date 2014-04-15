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

void setup_queue(struct PacketQueue *queue) {
  queue->head = NULL;
  queue->tail = NULL;
}

static struct PacketQueueNode *create_new_node(struct Packet *to_copy,
                                               size_t length) {
  struct PacketQueueNode *new_node;
  new_node = (struct PacketQueueNode *)malloc(sizeof(struct PacketQueueNode));

  // If we can't allocate memory for this, it's a serious problem
  // can't recover from.
  assert(new_node);

  new_node->next = NULL;
  new_node->length = length;
  memcpy(&new_node->packet, to_copy, sizeof(struct Packet));

  return new_node;
}

void add_to_queue(struct PacketQueue *queue,
                  struct Packet *to_be_added,
                  size_t length) {
  if (queue->tail != NULL) {
    queue->tail->next = create_new_node(to_be_added,length);
    queue->tail = queue->tail->next;
  } else {
    // If it's NULL, we don't have a tail, so the queue must be empty.
    queue->head = queue->tail = create_new_node(to_be_added,length);
  }
}

size_t next_packet(struct PacketQueue *queue, struct Packet *out_packet) {
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

void debug_print_queue(struct PacketQueue *queue) {
  struct PacketQueueNode *current_node;

  assert(queue != NULL);
  current_node = queue->head;

  printf("-------\n");

  int position = 0;

  while (current_node != NULL) {
    printf("%d: Message ID: %d\n", position++,
           current_node->packet.message_number);
    current_node = current_node->next;
  }

  printf("-------\n");
}
