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

// Temp fix to stop clang from raising stupid error

/*
 * Application Messages
 *
 * Only the Application layer and the network layer should know about
 * these.
 */
struct Message {
  char data[MAX_MESSAGE_SIZE];
};

// Declarations
EVENT_HANDLER(application_ready);
void network_up_to_application(struct Message *in_message, size_t length);

#endif
