/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Description:
 *
 *  Cnet emulates the physical layer, however this is the wrapper
 *  functions for handling events from the physical layer and
 *  constructing frames to send across the wire.
 */

#ifndef PHYSICAL_LAYER_H_
#define PHYSICAL_LAYER_H_

#include <cnet.h>
#include <stdint.h>

#include "network_layer.h"

enum FrameType {
  DL_DATA,
  DL_ACK};

struct Frame {
  enum FrameType type;
  uint32_t checksum;
  int sequence;

  // Size of the packet.
  size_t length;
  struct Packet packet;
};

/*
 * Transmit the frame out onto the given physical link.
 */
void transmit_frame(int out_link,
                    struct Frame *out_frame,
                    enum FrameType type,
                    int sequence_no);

/*
 * The corresponding "receive_frame" is actually an event fired by
 * Cnet.
 */
EVENT_HANDLER(physical_ready);

#endif
