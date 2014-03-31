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

/*
 * Transmit the frame out onto the given physical link.
 */
void transmit_frame(int out_link,
                    struct FRAME *out_frame,
                    enum FRAME_TYPE type,
                    size_t length,
                    int sequence_no);
#endif
