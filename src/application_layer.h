/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Application Layer
 *
 * Description:
 *   CNET provides the application layer, however we need an interface
 *   for going to and from the CNET application.
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <cnet.h>

/*
 * Messages generated by the application. We don't care about their
 * contents, and they are not to be larger than MAX_MESSAGE_SIZE.
 */
struct Message {
  char data[MAX_MESSAGE_SIZE];
};

/*
 * Application Ready
 *
 * CNET already provides the application layer, however to bring
 * application messages into our simulation, we have to handle the
 * "application_ready event."
 *
 * This is called when the application is ready to send some data to a
 * node. The address will be any node except for the current one.
 */
EVENT_HANDLER(application_ready);

/*
 * Network up to application
 *
 * Called when the network layer has determined that the message is
 * for the application running on the node.
 */
void network_up_to_application(const struct Message *const in_message,
                               const size_t length);

#endif
