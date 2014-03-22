/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Application Layer
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <cnet.h>

/*
 * Application Messages
 *
 * Only the Application layer and the network layer should know about
 * these.
 */
struct MESSAGE {
  char data[MAX_MESSAGE_SIZE];
};

// Declarations
EVENT_HANDLER(application_ready);
void network_up_to_application(struct MESSAGE *in_message, size_t length);

extern struct MESSAGE *last_message;

#endif
