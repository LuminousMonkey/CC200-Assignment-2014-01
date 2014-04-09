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

#define FRAME_HEADER_SIZE (sizeof(struct Frame) -   \
                           sizeof(struct Packet))
#define FRAME_SIZE(f) (FRAME_HEADER_SIZE + f->length)

// Forward declarations
static void process_ack(struct Frame *in_frame, CnetTimerID last_timer);
static void process_data(struct Frame *in_frame, int in_link);

// Local node variables
static CnetTimerID last_timer = NULLTIMER;

static int ack_expected = 0;
static int next_frame_to_send = 0;
static int frame_expected = 0;

// We need to keep track of the last things send.
static int last_link = 0;
static struct Frame outgoing_frame;
static size_t last_length = 0;

/*
 * Takes the raw physical frame, and start processing it.
 */
void up_to_datalink_from_physical(int in_link,
                                  struct Frame *in_frame,
                                  size_t frame_length) {

  uint32_t in_checksum = in_frame->checksum;

  // Checksum was calculated with the checksum field of 0.
  in_frame->checksum = 0;

  // Check if the packet is corrupted.
  if (CNET_crc32((void *) in_frame,
                 frame_length) == in_checksum) {

    // Process depending on the frame type.
    switch (in_frame->type) {
      case DL_ACK:
        process_ack(in_frame, last_timer);
        break;
      case DL_DATA:
        process_data(in_frame, in_link);
        break;
      default:
        printf("Error: Unexpected frame type.\n");
    }
  } else {
    printf("CRC in is: %d\n", in_checksum);

    printf("Expected is: %d\n", CNET_crc32((unsigned char *)&in_frame, frame_length));
    printf("\t\t\t\tBAD checksum - frame ignored.\n");
  }
}

void down_to_datalink_from_network(int out_link, struct Packet *out_packet,
                                   size_t length) {
  // Build the frame.
  outgoing_frame.length = length;
  memcpy(&outgoing_frame.packet, out_packet, length);

  // This is a static one as we need to keep a copy around incase of retransmit.
  transmit_frame(out_link, &outgoing_frame, DL_DATA, next_frame_to_send);
}

/*
 * Event handler for frame ACKs that are timing out.
 */
EVENT_HANDLER(timeouts) {
  printf("timeout, seq=%d\n", ack_expected);

  // Retry sending
  transmit_frame(last_link, &outgoing_frame, DL_DATA, ack_expected);
}

static void process_ack(struct Frame *in_frame, CnetTimerID last_timer) {
  if (in_frame->sequence == ack_expected) {
    printf("\t\t\t\tACK received, sequence: %d.\n", in_frame->sequence);
    CNET_stop_timer(last_timer);
    ack_expected = 1 - ack_expected;
    CNET_enable_application(ALLNODES);
  }
}

static void process_data(struct Frame *in_frame, int in_link) {
  if (in_frame->sequence == frame_expected) {
    printf("Up to Network Layer.\n");
    datalink_up_to_network(&in_frame->packet);
    frame_expected = 1 - frame_expected;
  } else {
    printf("Ignored\n");
  }

  transmit_frame(in_link, &outgoing_frame, DL_ACK, in_frame->sequence);
}

void transmit_frame(int out_link,
                    struct Frame *frame_to_transmit,
                    enum FrameType type,
                    int sequence_no) {

  frame_to_transmit->type = type;
  frame_to_transmit->sequence = sequence_no;
  frame_to_transmit->checksum = 0;

  switch (type) {
    case DL_ACK:
      printf("ACK transmitted, sequence: %d.\n", sequence_no);
      break;
    case DL_DATA:
      printf("DATA transmitted, sequence: %d.\n", sequence_no);

      CnetTime timeout = FRAME_SIZE(frame_to_transmit) *
          ((CnetTime) 8000000 / linkinfo[out_link].bandwidth) +
          linkinfo[out_link].propagationdelay;

      last_timer = CNET_start_timer(EV_TIMER1, 3 * timeout, 0);
      last_link = out_link;
      break;
    default:
      printf("Unexpected frame type.\n");
  }

  last_length = FRAME_SIZE(frame_to_transmit);
  frame_to_transmit->checksum =
      CNET_crc32((unsigned char *) frame_to_transmit, (int) last_length);
  printf("CRC is: %d\n", frame_to_transmit->checksum);
  CHECK(CNET_write_physical_reliable(out_link,
                            (void *) frame_to_transmit,
                            &last_length));
}
