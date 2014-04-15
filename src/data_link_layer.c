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

// Forward declarations
static void process_ack(struct Frame *in_frame, CnetTimerID last_timer, int in_link);
static void process_data(struct Frame *in_frame, int in_link);
static void transmit_frame(int out_link,
                           struct Frame *frame_to_transmit,
                           enum FrameType type,
                           int sequence_no);
static void send_off_queued_packet(int out_link);
static size_t frame_size(struct Frame *frame);

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

void init_data_link_layer() {
  for (int i = 0; i < nodeinfo.nlinks; ++i) {
    setup_queue(&packet_queue[i]);
  }
}

void up_to_datalink_from_physical(int in_link,
                                  struct Frame *in_frame,
                                  size_t frame_length) {

  // Checksum was calculated with the checksum field of 0. So save the
  // incoming checksum first and clear the checksum so we can
  // calculate it.
  uint32_t in_checksum = in_frame->checksum;
  in_frame->checksum = 0;

  // Check if the packet is corrupted.
  if (CNET_crc32((void *) in_frame, frame_length) == in_checksum) {
    // Good checksum!
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

void down_to_datalink_from_network(int out_link,
                                   struct Packet *out_packet,
                                   size_t length) {
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
 * Build and send frame.
 *
 * Builds the frame up to be transmitted by the physical layer. The
 * data link layer keeps a copy of the last transmitted frame, because
 * the last frame will need to be transmitted if there's a timeout
 * waiting for an ACK.
 *
 * This function takes care of that, however, beaware that this will
 * override the previous frame, so this function should only be called
 * if the node is not waiting for an ACK on the given link.
 *
 * out_link - Link to send frame out on.
 * out_packet - Packet to be sent out.
 * length - Size of the packet passed in.
 */
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
    build_and_send_frame(out_link, &next_packet_to_send, length);
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

      CnetTime timeout = frame_size(frame_to_transmit) *
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

  size_t last_length = frame_size(frame_to_transmit);
  frame_to_transmit->checksum =
      CNET_crc32((unsigned char *) frame_to_transmit, (int) last_length);
  CHECK(CNET_write_physical(out_link,
                            (void *) frame_to_transmit,
                            &last_length));
}

void debug_data_link_layer() {
  printf("Status for links.\n");
  printf("+------+------------------+----------------+--------------------+\n");
  printf("| Link | Ack Seq Expected | Next Frame Seq | Frame Seq Expected |\n");
  printf("+------+------------------+----------------+--------------------+\n");
  for (int current_link = 0; current_link < nodeinfo.nlinks; current_link++) {
    printf("|  %d   |        %d         |        %d       |         %d          |\n",
           current_link + 1,
           ack_expected[current_link],
           next_frame_to_send[current_link],
           frame_expected[current_link]);
    printf("+------+------------------+----------------+--------------------+\n");
  }
}

/*
 * Frame size
 *
 * Given a pointer to the frame, it will return the total used size
 * for the frame. This is why the Packet must be at the end of the
 * Frame struct.
 */
static size_t frame_size(struct Frame *frame) {
  const size_t frame_header_size = sizeof(struct Frame) -
      sizeof(struct Packet);

  return frame_header_size + frame->length;
}
