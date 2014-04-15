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
static void transmit_frame(int out_link,
                           struct Frame *frame_to_transmit,
                           enum FrameType type,
                           int sequence_no);

// Stop and wait sequence numbers, need to be organised on a per link
// basis. Be sure to +1 because links start at 1.
#define MAX_NO_LINKS 4

// Local node variables
static CnetTimerID timers[MAX_NO_LINKS] = {NULLTIMER,
                                           NULLTIMER,
                                           NULLTIMER,
                                           NULLTIMER};

static int ack_expected[MAX_NO_LINKS] = {0,0,0,0};
static int next_frame_to_send[MAX_NO_LINKS] = {0,0,0,0};
static int frame_expected[MAX_NO_LINKS] = {0,0,0,0};

/* We have to hold the last frame sent out on a link, we do this so we
 * can retransmit it if we don't receive an ACK before the timer runs
 * out.
 */
static struct Frame outgoing_frame[MAX_NO_LINKS];

/*
 * Queue per link, for packets to wait while waiting for a pending
 * ACK.
 */
static struct PacketQueue packet_queue[MAX_NO_LINKS];

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
        process_ack(in_frame, timers[in_link - 1], in_link);
        break;
      case DL_DATA:
        process_data(in_frame, in_link);
        break;
      default:
        printf("Error: Unexpected frame type.\n");
    }
  } else {
    printf("\t\t\t\tBAD checksum - frame ignored.\n");
  }
}

static void build_and_send_frame(int out_link,
                                 struct Packet *out_packet,
                                 size_t length) {
  // Build the frame
  outgoing_frame[out_link - 1].length = length;
  memcpy(&outgoing_frame[out_link - 1].packet, out_packet, length);

  // This is a static one as we need to keep a copy around incase of retransmit.
  transmit_frame(out_link, &outgoing_frame[out_link - 1],
                 DL_DATA, next_frame_to_send[out_link - 1]);
  next_frame_to_send[out_link - 1] = 1 - next_frame_to_send[out_link - 1];
}

/*
 * With the queue, when we are ready to send data, we always send off
 * the first queued packet.
 */
static void send_off_queued_packet(int out_link) {
  /*
   * Check the queue for any pending packets, and send the first one
   * in the queue.
   */
  struct Packet next_packet_to_send;
  size_t length = next_packet(&packet_queue[out_link - 1], &next_packet_to_send);

  if (length != 0) {
    // Just call down to datalink again, double copying, but easier to follow.
    printf("Sending off queued packet.\n");
    build_and_send_frame(out_link, &next_packet_to_send, length);
  } else {
    printf("No packets to send.\n");
  }
}

void down_to_datalink_from_network(int out_link,
                                   struct Packet *out_packet,
                                   size_t length) {

  /*
   * Always add the packet to the queue, then send off the first
   * packet on that queue.
   */
  printf("Adding packet to queue.\n");
  add_to_queue(&packet_queue[out_link - 1], out_packet, length);

  /*
   * If we're waiting on an ACK, we don't send the packet yet, we will
   * send it when the ACK is received.
   */
  if (ack_expected[out_link - 1] == next_frame_to_send[out_link - 1]) {
    send_off_queued_packet(out_link);
  }
}

/*
 * Event handler for frame ACKs that are timing out.
 */
EVENT_HANDLER(timeouts) {

  int link_timeout = (int)data;

  // Retry sending
  printf("Timeout, DATA(%d) out on link: %d\n", ack_expected[link_timeout - 1], link_timeout);

  transmit_frame(link_timeout, &outgoing_frame[link_timeout - 1],
                 DL_DATA, ack_expected[link_timeout - 1]);
}

static void process_ack(struct Frame *in_frame, CnetTimerID last_timer, int in_link) {
  if (in_frame->sequence == ack_expected[in_link - 1]) {
    printf("\t\t\t\tACK received. Link: %d, sequence: %d.\n",
           in_link, in_frame->sequence);
    CNET_stop_timer(last_timer);
    ack_expected[in_link - 1] = 1 - ack_expected[in_link - 1];
    send_off_queued_packet(in_link);
  } else {
    printf("\t\t\t\tIncorrect ACK. Link: %d, sequence: %d, expected %d\n",
           in_link, in_frame->sequence, ack_expected[in_link - 1]);
  }
}

static void process_data(struct Frame *in_frame, int in_link) {
  if (in_frame->sequence == frame_expected[in_link - 1]) {
    printf("\t\t\t\tDATA received. Link: %d, sequence: %d.\n",
           in_link,
           in_frame->sequence);
    frame_expected[in_link - 1] = 1 - frame_expected[in_link - 1];
    datalink_up_to_network(&in_frame->packet);
  } else {
    printf("\t\t\t\tDATA received. Link: %d, sequence: %d, expected %d\n",
           in_link, in_frame->sequence, frame_expected[in_link - 1]);
    printf("\t\t\t\tIgnored\n");
  }

  printf("Transmitting ACK(%d) to link: %d\n", in_frame->sequence, in_link);
  transmit_frame(in_link, &outgoing_frame[in_link - 1],
                 DL_ACK, in_frame->sequence);
}

static void transmit_frame(int out_link,
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

      /*
       * There's a timer per link, set the timer and set which link
       * the timer is for so if it expires we know which link to send
       * the packet back out on.
       */
      timers[out_link - 1] = CNET_start_timer(EV_TIMER1, 4 * timeout, out_link);
      break;
    default:
      printf("Unexpected frame type.\n");
  }

  size_t last_length = FRAME_SIZE(frame_to_transmit);
  frame_to_transmit->checksum =
      CNET_crc32((unsigned char *) frame_to_transmit, (int) last_length);
  CHECK(CNET_write_physical(out_link,
                            (void *) frame_to_transmit,
                            &last_length));
}

/*
 *  Prints out the data link layer info.
 */
void debug_data_link_layer() {
  for (int current_queue = 0; current_queue < MAX_NO_LINKS; current_queue++) {
    printf("Queue for Link: %d\n", current_queue + 1);
    printf("------------------\n");

    debug_print_queue(&packet_queue[current_queue]);
  }
}
