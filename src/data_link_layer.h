/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Data link layer.
 */

#ifndef DATA_LINK_LAYER_H_
#define DATA_LINK_LAYER_H_

#include <cnet.h>
#include <stdlib.h>

#include "network_layer.h"

enum FRAME_TYPE {
  DL_DATA,
  DL_ACK};

struct FRAME {
  enum FRAME_TYPE type;
  size_t length;
  int checksum;
  int sequence;
  struct PACKET packet;
};

#define FRAME_HEADER_SIZE (sizeof(struct FRAME) -   \
                           sizeof(struct MESSAGE))
#define FRAME_SIZE(f) (FRAME_HEADER_SIZE + f.length)

/*
 * Just pass the physical frame up to the data link layer.
 */
void up_to_datalink_from_physical(int in_link,
                                  struct FRAME *in_frame,
                                  int frame_length);

#endif
