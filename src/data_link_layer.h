/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Data link layer.
 */

#ifndef DATA_LINK_LAYER_H_
#define DATA_LINK_LAYER_H_

#include <stdlib.h>

#include "cnet.h"

enum FRAME_TYPE {
  DL_DATA,
  DL_ACK};

struct MESSAGE {
  char data[MAX_MESSAGE_SIZE];
};

struct FRAME {
  enum FRAME_TYPE type;
  size_t length;
  int checksum;
  int sequence;
  struct MESSAGE message;
};

#define FRAME_HEADER_SIZE (sizeof(struct FRAME) -   \
                           sizeof(struct MESSAGE))
#define FRAME_SIZE(f) (FRAME_HEADER_SIZE + f.length)

#endif
