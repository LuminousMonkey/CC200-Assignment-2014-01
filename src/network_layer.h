/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Network Layer
 *
 * Description:
 *
 *  For taking the messages from the application, and figuring out how
 *  to get them to the needed host. Wraps the application message into
 *  packets.
 */

#ifndef NETWORK_LAYER_H_
#define NETWORK_LAYER_H_

#include "application_layer.h"

struct PACKET {
  CnetAddr destination_address;
  CnetAddr source_address;

  size_t length;
  struct MESSAGE message;
};

/* Declarations */

/*
 * Take the message from the application and process it so it can be
 * routed through the network.
 */
void application_down_to_network(CnetAddr destination_address,
                                 CnetAddr source_address,
                                 struct MESSAGE *message, int length);

#endif
