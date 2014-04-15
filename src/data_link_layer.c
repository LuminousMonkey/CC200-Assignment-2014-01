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

/*
 * To save from dynamically allocating memory for the ack, next frame,
 * and frame expected sequence information, just use a static array.
 * Just allocate the max and leave the others unused.
 */
#define MAX_NO_LINKS 4

/*
 * Each link keeps track of a timer for ACK timeouts, if we receive
 * the correct ACK, then we remove the timer.
 */
static CnetTimerID timers[MAX_NO_LINKS] = {NULLTIMER,
                                           NULLTIMER,
                                           NULLTIMER,
                                           NULLTIMER};

static int ack_expected[MAX_NO_LINKS] = {0,0,0,0};
static int next_frame_to_send[MAX_NO_LINKS] = {0,0,0,0};
static int frame_expected[MAX_NO_LINKS] = {0,0,0,0};

/*
 * We have to hold the last frame sent out on a link, we do this so we
 * can retransmit it if we don't receive an ACK before the timer runs
 * out.
 */
static struct Frame outgoing_frame[MAX_NO_LINKS];

/*
 * Outgoing packet queue, packets are placed in a FIFO queue until the
 * link is free to send (i.e. we're not waiting on an ACK).
 */
static struct PacketQueue packet_queue[MAX_NO_LINKS];

// Forward declarations
static void process_ack(const struct Frame *in_frame,
                        const CnetTimerID last_timer,
                        const int in_link);
static void process_data(const struct Frame *const in_frame,
                         const int in_link);
static void transmit_frame(const int out_link,
                           const enum FrameType type,
                           const int sequence_no);
static void send_off_queued_packet(const int out_link);
static size_t frame_size(const struct Frame *const frame);

/*
 * Init data link layer
 *
 * Here we allocate and initialise our structures for the data link,
 * the packet queue is the only dynamically allocated structure we
 * use.
 */
void init_data_link_layer() {
  for (int i = 0; i < nodeinfo.nlinks; ++i) {
    setup_queue(&packet_queue[i]);
  }
}

/*
 * Up to datalink from physical
 *
 * Check header file for details.
 */
void up_to_datalink_from_physical(const int in_link,
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
    // Bad checksum, naughty checksum, go to bed.
    printf("\t\t\t\tBAD checksum - frame ignored.\n");
  }
}

/*
 * Down to datalink from network
 *
 * Check header file for details.
 *
 * Globals:
 *   packet_queue - out_packet added to queue, and top most packet
 *                  sent if link free.
 */
void down_to_datalink_from_network(const int out_link,
                                   const struct Packet *const out_packet,
                                   const size_t length) {
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
 * The event handler that is called when a timer expires waiting for
 * an ACK. When the timer event fires, the number of the link is
 * passed in via the CNET data variable. Resend the frame again, and
 * setup a new timer.
 */
EVENT_HANDLER(timeouts) {
  int link_timeout = (int)data;

  printf("Timeout, DATA(%d) out on link: %d\n",
         ack_expected[link_timeout - 1], link_timeout);

  transmit_frame(link_timeout, DL_DATA, ack_expected[link_timeout - 1]);
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
 *
 * Globals:
 *   outgoing_frame - Updated on corresponding link and transmitted.
 */
static void build_and_send_frame(const int out_link,
                                 const struct Packet *const out_packet,
                                 const size_t length) {
  // Build the frame
  outgoing_frame[out_link - 1].length = length;
  memcpy(&outgoing_frame[out_link - 1].packet, out_packet, length);

  // This is a static one as we need to keep a copy around incase of retransmit.
  transmit_frame(out_link, DL_DATA, next_frame_to_send[out_link - 1]);
  next_frame_to_send[out_link - 1] = 1 - next_frame_to_send[out_link - 1];
}

/*
 * Send off queued packet
 *
 * With the queue, when we are ready to send data, we always send off
 * the first queued packet.
 *
 * out_link - Link which to check queue and send packet out on.
 *
 * Globals:
 *   packet_queue - Top packet removed from queue.
 */
static void send_off_queued_packet(const int out_link) {
  struct Packet next_packet_to_send;

  const size_t length = next_packet(&packet_queue[out_link - 1],
                                    &next_packet_to_send);

  if (length != 0) {
    build_and_send_frame(out_link, &next_packet_to_send, length);
  }
}

/*
 * Process ACK
 *
 * Called when a node receives an ACK frame. Will check that the ACK
 * is the expected sequence number, if it is, great, the other node
 * got the data frame. If it isn't, then, it could mean that it just
 * took awhile for an ACK to get to us and we send a duped DATA frame.
 *
 * in_frame - ACK frame we've received.
 * last_timer - Matching timer for the link we received the ACK.
 * in_link - Link we received the ACK on.
 *
 * Globals:
 *   ack_expected - Updated to next sequence number.
 *
 * Assumed that frame has already had its checksum checked.
 */
static void process_ack(const struct Frame *in_frame,
                        const CnetTimerID last_timer,
                        const int in_link) {

  if (in_frame->sequence == ack_expected[in_link - 1]) {
    // ACK sequence expected.
    printf("\t\t\t\tACK received. Link: %d, sequence: %d.\n",
           in_link, in_frame->sequence);

    // Stop the timer so we don't send out a dup DATA frame.
    CNET_stop_timer(last_timer);
    ack_expected[in_link - 1] = 1 - ack_expected[in_link - 1];

    // Not waiting for ACK anymore, so try to send off another packet
    // for that link.
    send_off_queued_packet(in_link);
  } else {
    printf("\t\t\t\tIncorrect ACK. Link: %d, sequence: %d, expected %d\n",
           in_link, in_frame->sequence, ack_expected[in_link - 1]);
  }
}

/*
 * Process Data
 *
 * Called when a node receives a DATA frame.
 *
 * Check that the frame sequence matches the sequence number we're
 * expecting. If it does, great, pass it up for the network layer,
 * otherwise just ignore it.
 *
 * Regardless, always send an ACK back with the same sequence number
 * so the node at the other end of the link knows we received it.
 *
 * in_frame - Frame that's been received.
 * in_link - Link that the frame came in on.
 *
 * Globals:
 *   frame_expected - Updated to new sequence number.
 */
static void process_data(const struct Frame *const in_frame,
                         const int in_link) {
  if (in_frame->sequence == frame_expected[in_link - 1]) {
    printf("\t\t\t\tDATA received. Link: %d, sequence: %d.\n",
           in_link,
           in_frame->sequence);

    // Expected, switch to next frame seq number and send the packet
    // in this frame up to the network layer.
    frame_expected[in_link - 1] = 1 - frame_expected[in_link - 1];
    datalink_up_to_network(&in_frame->packet);
  } else {
    printf("\t\t\t\tDATA received. Link: %d, sequence: %d, expected %d\n",
           in_link, in_frame->sequence, frame_expected[in_link - 1]);
    printf("\t\t\t\tIgnored\n");
  }

  transmit_frame(in_link, DL_ACK, in_frame->sequence);
}

/*
 * Transmit frame
 *
 * Send the given frame out on the link.
 *
 * out_link - Link to send the frame out on.
 * type - Type of frame, ACK, or DATA.
 * sequence_no - Sequence number to go out on the frame.
 *
 * Globals:
 *   linkinfo - Provided by CNET.
 *   outgoing_frame - Updated to frame to be sent out.
 *   timers - Updated to new ACK timer.
 */
static void transmit_frame(const int out_link,
                           const enum FrameType type,
                           const int sequence_no) {

  outgoing_frame[out_link - 1].type = type;
  outgoing_frame[out_link - 1].sequence = sequence_no;
  outgoing_frame[out_link - 1].checksum = 0;

  switch (type) {
    case DL_ACK:
      printf("ACK(%d) sent out on link %d.\n", sequence_no, out_link);
      break;
    case DL_DATA:
      printf("DATA(%d) sent out on link %d.\n", sequence_no, out_link);

      CnetTime timeout = frame_size(&outgoing_frame[out_link - 1]) *
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

  // Send it off onto the physical link.
  size_t length = frame_size(&outgoing_frame[out_link - 1]);
  outgoing_frame[out_link - 1].checksum =
      CNET_crc32((unsigned char *) &outgoing_frame[out_link - 1], (int) length);
  CHECK(CNET_write_physical(out_link,
                            (void *) &outgoing_frame[out_link - 1],
                            &length));
}

/*
 * Frame size
 *
 * Given a pointer to the frame, it will return the total used size
 * for the frame. This is why the Packet must be at the end of the
 * Frame struct.
 */
static size_t frame_size(const struct Frame *const frame) {
  const size_t frame_header_size = sizeof(struct Frame) -
      sizeof(struct Packet);

  return frame_header_size + frame->length;
}
