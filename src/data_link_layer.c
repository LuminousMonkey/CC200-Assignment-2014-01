/*
 * CC200 Assignment
 *
 * Author: Mike Aldred
 *
 * Data link layer.
 */

#include <cnet.h>
#include <stdio.h>
#include <string.h>

#include "data_link_layer.h"
#include "physical_layer.h"

// Forward declarations
static void process_ack(struct FRAME in_frame, CnetTimerID last_timer);
static void process_data(struct FRAME in_frame);

// Local node variables
static CnetTimerID last_timer = NULLTIMER;

static int ack_expected = 0;
//static int next_frame_to_send = 0;
static int frame_expected = 0;
static int last_link = 0;

static struct FRAME *last_frame = NULL;
static size_t last_length = 0;

/*
 * Takes the raw physical frame, and start processing it.
 */
void up_to_datalink_from_physical(int in_link,
                                  struct FRAME *in_frame,
                                  int frame_length) {

  int in_checksum = in_frame->checksum;

  // Checksum was calculated with the checksum field of 0.
  in_frame->checksum = 0;

  // Check if the packet is corrupted.
  if (CNET_crc32((unsigned char *)&in_frame,
                 (int) frame_length) == in_checksum) {

    // Process depending on the frame type.
    switch (in_frame->type) {
      case DL_ACK:
        process_ack(*in_frame, last_timer);
        break;
      case DL_DATA:
        process_data(*in_frame);
        break;
      default:
        printf("Error: Unexpected frame type.\n");
    }
  } else {
    printf("\t\t\t\tBAD checksum - frame ignored.\n");
  }
}

void down_to_datalink_from_network(int out_link,
                                   struct PACKET *out_packet,
                                   int packet_size) {
  // Take the packet.
  // Build the frame.
  // Save as last sent.
  // Set timer.
  // Send out on link.
  // Wait for ACK or timer expire.
  // If timer expire, resend frame.
}

/*
 * Event handler for frame ACKs that are timing out.
 */
static EVENT_HANDLER(timeouts) {
    printf("timeout, seq=%d\n", ack_expected);
    transmit_frame(last_link, last_frame, DL_DATA, last_length, ack_expected);
}

static void process_ack(struct FRAME in_frame, CnetTimerID last_timer) {
  if (in_frame.sequence == ack_expected) {
    printf("\t\t\t\tACK received, sequence: %d.\n", in_frame.sequence);
    CNET_stop_timer(last_timer);
    ack_expected = 1 - ack_expected;
    CNET_enable_application(ALLNODES);
  }
}

static void process_data(struct FRAME in_frame) {
  if (in_frame.sequence == frame_expected) {
    printf("Up to application.\n");
    size_t frame_length = in_frame.length;
    CHECK(CNET_write_application((char *) &in_frame.packet, &frame_length));
    frame_expected = 1 - frame_expected;
  } else {
    printf("Ignored\n");
  }

  transmit_frame(NULL, DL_ACK, 0, in_frame.sequence);
}

static void transmit_frame(struct PACKET *packet,
                           enum FRAME_TYPE type,
                           size_t length,
                           int seqno) {
  struct FRAME frame_to_transmit;
  int link = 1;

  frame_to_transmit.type = type;
  frame_to_transmit.sequence = seqno;
  frame_to_transmit.checksum = 0;
  frame_to_transmit.length = length;

  switch (type) {
    case DL_ACK:
      printf("ACK transmitted, sequence: %d.\n", seqno);
      break;
    case DL_DATA:
      printf("DATA transmitted, sequence: %d.\n", seqno);
      memcpy(&frame_to_transmit.packet, (char *) packet, (int) length);

      CnetTime timeout = FRAME_SIZE(frame_to_transmit) *
          ((CnetTime) 8000000 / linkinfo[link].bandwidth) +
          linkinfo[link].propagationdelay;

      last_timer = CNET_start_timer(EV_TIMER1, 3 * timeout, 0);
      break;
    default:
      printf("Unexpected frame type.\n");
  }

  size_t out_frame_length = FRAME_SIZE(frame_to_transmit);
  frame_to_transmit.checksum =
      CNET_crc32((unsigned char *) &frame_to_transmit, (int) out_frame_length);
  CHECK(CNET_write_physical(link,
                            (char *) &frame_to_transmit,
                            &out_frame_length));
}
