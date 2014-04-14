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
#include "packet_queue.h"
#include "physical_layer.h"

#define FRAME_HEADER_SIZE (sizeof(struct Frame) - \
                           sizeof(struct Packet))
#define FRAME_SIZE(f) (FRAME_HEADER_SIZE + f->length)

// Forward declarations
static void process_ack(struct Frame *in_frame, CnetTimerID last_timer, int in_link);
static void process_data(struct Frame *in_frame, int in_link);

// Local node variables
static CnetTimerID last_timer = NULLTIMER;

// Stop and wait sequence numbers, need to be organised on a per link
// basis. Be sure to +1 because links start at 1.
#define MAX_NO_LINKS 5

static int ack_expected[MAX_NO_LINKS] = {0,0,0,0,0};
static int next_frame_to_send[MAX_NO_LINKS] = {0,0,0,0,0};
static int frame_expected[MAX_NO_LINKS] = {0,0,0,0,0};

/*
 * Queue per link, for packets to wait while waiting for a pending
 * ACK.
 */
static struct PacketQueue packet_queue[MAX_NO_LINKS];

// We need to keep track of the last things send.
static int last_link = 0;
static struct Frame outgoing_frame;
static size_t last_length = 0;

/*
 * Init data link layer structures.
 */
void init_data_link_layer() {
  for (int i = 0; i < MAX_NO_LINKS; ++i) {
    setup_queue(&packet_queue[i]);
  }
}

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
    printf("BAD checksum - frame ignored.\n");
    printf("-------------------------------------------------\n");
  }
}

void down_to_datalink_from_network(int out_link,
                                   struct Packet *out_packet,
                                   size_t length) {

  /*
   * If we're waiting on an ACK, then just queue up the packet and
   * we'll process it later.
   */
  if (ack_expected[out_link] != next_frame_to_send[out_link]) {
    printf("Waiting on ACK. Queuing packet.\n");
    add_to_queue(&packet_queue[out_link], out_packet, length);
  } else {
    // Build the frame.
    outgoing_frame.length = length;
    memcpy(&outgoing_frame.packet, out_packet, length);

    // This is a static one as we need to keep a copy around incase of retransmit.
    transmit_frame(out_link, &outgoing_frame, DL_DATA, next_frame_to_send[out_link]);
    next_frame_to_send[out_link] = 1 - next_frame_to_send[out_link];
  }
}

/*
 * Event handler for frame ACKs that are timing out.
 */
EVENT_HANDLER(timeouts) {
  // Retry sending
  printf("Timeout, DATA(%d) out on link: %d\n", ack_expected[last_link], last_link);
  transmit_frame(last_link, &outgoing_frame, DL_DATA, ack_expected[last_link]);
}

static void process_ack(struct Frame *in_frame, CnetTimerID last_timer, int in_link) {
  if (in_frame->sequence == ack_expected[in_link]) {
    printf("\t\t\t\tACK received. Link: %d, sequence: %d.\n",
	   in_link, in_frame->sequence);
    CNET_stop_timer(last_timer);
    ack_expected[in_link] = 1 - ack_expected[in_link];

    /*
     * Check the queue for any pending packets, and send the first one
     * in the queue.
     */
    struct Packet next_packet_to_send;
    size_t length = next_packet(&packet_queue[in_link], &next_packet_to_send);

    if (length != 0) {
      // Just call down to datalink again, double copying, but easier to follow.
      printf("Sending off queued packet.\n");
      down_to_datalink_from_network(in_link, &next_packet_to_send, length);
    }
  } else {
    printf("\t\t\t\tIncorrect ACK. Link: %d, sequence: %d, expected %d\n",
           in_link, in_frame->sequence, ack_expected[in_link]);
  }
}

static void process_data(struct Frame *in_frame, int in_link) {
  if (in_frame->sequence == frame_expected[in_link]) {
    printf("\t\t\t\tDATA received. Link: %d, sequence: %d.\n",
	   in_link,
	   in_frame->sequence);
    frame_expected[in_link] = 1 - frame_expected[in_link];
    datalink_up_to_network(&in_frame->packet);
  } else {
    printf("\t\t\t\tDATA received. Link: %d, sequence: %d, expected %d\n",
           in_link, in_frame->sequence, frame_expected[in_link]);
    printf("Ignored\n");
  }

  printf("Transmitting ACK(%d) to link: %d\n", in_frame->sequence, in_link);
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
      printf("ACK(%d) sent out on link %d.\n", sequence_no, out_link);
      break;
    case DL_DATA:
      printf("DATA(%d) sent out on link %d.\n", sequence_no, out_link);

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
  CHECK(CNET_write_physical_reliable(out_link,
                            (void *) frame_to_transmit,
                            &last_length));
}
