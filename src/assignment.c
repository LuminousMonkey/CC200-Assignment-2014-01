/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 */

#include <cnet.h>
#include <stdlib.h>

#include "application_layer.h"
#include "data_link_layer.h"
#include "physical_layer.h"

EVENT_HANDLER(draw_frame);

EVENT_HANDLER(reboot_node) {
  // Allocate the memory for the application message.
  CHECK(CNET_set_handler(EV_APPLICATIONREADY, application_ready, 0));
  CHECK(CNET_set_handler(EV_PHYSICALREADY, physical_ready, 0));
  CHECK(CNET_set_handler(EV_TIMER1, timeouts, 0));
  CHECK(CNET_set_handler(EV_DRAWFRAME, draw_frame, 0));

  // Start the traffic
  if (nodeinfo.nodenumber == 0) {
    CNET_enable_application(ALLNODES);
  }
}

EVENT_HANDLER(draw_frame) {
  CnetDrawFrame *draw_frame = (CnetDrawFrame *)data;
  struct Frame *frame = (struct Frame *)draw_frame->frame;

  draw_frame->colours[0] = (frame->sequence == 0) ? "red" : "purple";
  draw_frame->pixels[0] = 10;

  switch (frame->type) {
    case DL_ACK:
      draw_frame->nfields = 1;
      sprintf(draw_frame->text, "ack=%d", frame->sequence);
      break;
    case DL_DATA:
      draw_frame->nfields = 2;
      draw_frame->colours[1] = "green";
      draw_frame->pixels[1] = 30;
      sprintf(draw_frame->text, "data=%d", frame->sequence);
      break;
    default:
      draw_frame->nfields = 1;
      sprintf(draw_frame->text, "Unknown");
  }
}
