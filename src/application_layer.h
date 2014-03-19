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

#endif
