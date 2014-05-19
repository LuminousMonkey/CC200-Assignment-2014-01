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
 * The corresponding "receive_frame" is actually an event fired by
 * Cnet.
 */
EVENT_HANDLER(physical_ready);

#endif
