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
EVENT_HANDLER(showstate);

EVENT_HANDLER(reboot_node) {
  init_data_link_layer();

  CHECK(CNET_set_handler(EV_APPLICATIONREADY, application_ready, 0));
  CHECK(CNET_set_handler(EV_PHYSICALREADY, physical_ready, 0));
  CHECK(CNET_set_handler(EV_TIMER1, timeouts, 0));
  CHECK(CNET_set_handler(EV_DEBUG0, showstate, 0));
  CHECK(CNET_set_debug_string(EV_DEBUG0, "Show status"));
  CHECK(CNET_set_handler(EV_DRAWFRAME, draw_frame, 0));

  // Start the traffic
  CNET_enable_application(ALLNODES);
}

EVENT_HANDLER(draw_frame) {
  CnetDrawFrame *draw_frame = (CnetDrawFrame *)data;
  struct Frame *frame = (struct Frame *)draw_frame->frame;

  draw_frame->colours[0] = (frame->sequence == 0) ? "red" : "purple";
  draw_frame->pixels[0] = 20;

  switch (frame->type) {
    case DL_ACK:
      draw_frame->nfields = 1;
      sprintf(draw_frame->text, "A:%d", frame->sequence);
      break;
    case DL_DATA:
      draw_frame->nfields = 2;
      draw_frame->colours[1] = "green";
      draw_frame->pixels[1] = 60;
      sprintf(draw_frame->text, "Dst:%d, D:%d",
              frame->packet.destination_address,
              frame->sequence);
      break;
    default:
      draw_frame->nfields = 1;
      sprintf(draw_frame->text, "Unknown");
  }
}

EVENT_HANDLER(showstate) {
  debug_data_link_layer();
}
