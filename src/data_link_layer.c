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
static void process_ack(struct Frame *in_frame, CnetTimerID last_timer, int in_link);
static void process_data(struct Frame *in_frame, int in_link);

// Local node variables
static CnetTimerID last_timer = NULLTIMER;

// Stop and wait sequence numbers, need to be organised on a per link
// basis.
#define MAX_NO_LINKS 5

static int ack_expected[MAX_NO_LINKS] = {0,0,0,0,0};
static int next_frame_to_send[MAX_NO_LINKS] = {0,0,0,0,0};
static int frame_expected[MAX_NO_LINKS] = {0,0,0,0,0};

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
        process_ack(in_frame, last_timer, in_link);
        break;
      case DL_DATA:
        process_data(in_frame, in_link);
        break;
      default:
        printf("Error: Unexpected frame type.\n");
    }
  } else {
    printf("-------------------------------------------------\n");
    printf("\t\t\t\tBAD checksum - frame ignored.\n");
    printf("-------------------------------------------------\n");
  }
}

void down_to_datalink_from_network(int out_link, struct Packet *out_packet,
                                   size_t length) {
  // Build the frame.
  outgoing_frame.length = length;
  memcpy(&outgoing_frame.packet, out_packet, length);

  printf("Network -> Data Link. Seq: %d\n", next_frame_to_send[out_link]);

  // This is a static one as we need to keep a copy around incase of retransmit.
  transmit_frame(out_link, &outgoing_frame, DL_DATA, next_frame_to_send[out_link]);
  next_frame_to_send[out_link] = 1 - next_frame_to_send[out_link];

  printf("Network -> Data Link. Updated Seq: %d\n", next_frame_to_send[out_link]);
}

/*
 * Event handler for frame ACKs that are timing out.
 */
EVENT_HANDLER(timeouts) {
  printf("Timeout, seq=%d\n", ack_expected[last_link]);

  // Retry sending
  transmit_frame(last_link, &outgoing_frame, DL_DATA, ack_expected[last_link]);
}

static void process_ack(struct Frame *in_frame, CnetTimerID last_timer, int in_link) {
  if (in_frame->sequence == ack_expected[in_link]) {
    printf("\t\t\t\tACK received, sequence: %d.\n", in_frame->sequence);
    CNET_stop_timer(last_timer);
    ack_expected[in_link] = 1 - ack_expected[in_link];
  } else {
    printf("Incorrect ACK sequence received: %d, expected %d\n",
           in_frame->sequence, ack_expected[in_link]);
  }
}

static void process_data(struct Frame *in_frame, int in_link) {
  printf("Processing Data packet\n");
  if (in_frame->sequence == frame_expected[in_link]) {
    printf("Up to Network Layer.\n");
    datalink_up_to_network(&in_frame->packet);
    frame_expected[in_link] = 1 - frame_expected[in_link];
  } else {
    printf("Incorrect Frame sequence received: %d, expected %d\n",
           in_frame->sequence, frame_expected[in_link]);
    printf("Ignored\n");
  }

  printf("Transmitting ACK to link: %d\n", in_link);
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
  CHECK(CNET_write_physical(out_link,
                            (void *) frame_to_transmit,
                            &last_length));
}
