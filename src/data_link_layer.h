/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Description:
 *
 *   This is where the majority of the hard work is done. As per the
 *   assignment specs, this impliments a stop and wait protocol.
 *   Additionally, each link on a node has a boundless queue, this
 *   means that any new packets will be queued if the node is waiting
 *   for an ACK to be received.
 *
 *   This was done to remove the need to have
 *   CNET_(enable/disable)_application function calls. This
 *   implimentation supports all node applications generating messages
 *   fulltime. However, given the efficiency of the stop and wait
 *   protocol, these queues will keep growing. Since CNET runs for 5
 *   minutes only as a default, this shouldn't be a problem.
 */

#ifndef DATA_LINK_LAYER_H_
#define DATA_LINK_LAYER_H_

#include "network_layer.h"
#include "physical_layer.h"

/*
 * Init Data Link Layer
 *
 * Since the data link layer has a queue per link, we need to make
 * sure they get initialised first.
 */
void init_data_link_layer();

/*
 * Up to datalink from physical
 *
 * When the physical layer receives a frame, it will pass it up via
 * this function. We need to know which link it came in on (because to
 * keep track of expected sequences on the link), the frame itself,
 * and it's length.
 *
 * in_link - Link that frame arrived on.
 * in_frame - Pointer to frame that arrived.
 * frame_length - Size of the frame.
 */
void up_to_datalink_from_physical(const int in_link,
                                  struct Frame *in_frame,
                                  size_t frame_length);

/*
 * Down to datalink from network
 *
 * Since we have a queue, just add any packets from the network layer
 * onto that queue straight away. Then we will test to see if that
 * link is waiting on an ACK, if it isn't, then send off the first
 * packet in the queue.
 *
 * This will most likly be the packet we just added.
 *
 * out_link - Link that packet is to go out on.
 * out_packet - Pointer to packet to be sent.
 * length - Size of the packet.
 */
void down_to_datalink_from_network(const int out_link,
                                   const struct Packet *const out_packet,
                                   const size_t length);

/*
 * Timeouts
 *
 * A timer is set to go off if we don't receive an ACK from a node. If
 * the timer is fired, then the frame needs to be resent on the link
 * again, and the timer is set again.
 *
 * This will keep happening until it works, or the zombie apocalypse.
 */
EVENT_HANDLER(timeouts);


/*
 * For printing out debug information about the data link layer.
 */
void debug_data_link_layer();

#endif
