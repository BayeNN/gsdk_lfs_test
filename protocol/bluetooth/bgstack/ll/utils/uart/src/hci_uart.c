#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <em_device.h>
#include <em_gpio.h>
#include <em_core.h>
#include "sl_hci_common_transport.h"
#include "../inc/sl_btctrl_hci_uart.h"
#include "uart.h"

#define RX_BUFFER_LEN 2550 // TODO: Determine buffer size

SL_ALIGN(4)
static uint8_t hci_rx_buffer[RX_BUFFER_LEN] SL_ATTRIBUTE_ALIGN(4);
/* buffer pointer for transferring bytes to hci_rx_buffer */
static uint8_t *buf_byte_ptr;
/* buffer pointer for extracting metadata (packet type, cmd length) */
static uint8_t *buf_meta_ptr;
static enum hci_uart_state state;
static enum hci_packet_type packet_type;
static uint16_t bytes_remaining; // first message byte contains packet type
static uint16_t total_bytes_read;

static void reset(void)
{
  memset(hci_rx_buffer, 0, RX_BUFFER_LEN);
  state = hci_uart_state_read_packet_type;
  buf_byte_ptr = hci_rx_buffer;
  buf_meta_ptr = hci_rx_buffer;
  bytes_remaining = 1;
  total_bytes_read = 0;
}

static void reception_failure(void)
{
  hci_common_transport_receive(NULL, 0);
  reset();
}

/**
 *  Called from sl_service_process_event(). Immediately returns if no data
 *  available to read. Reading of data consists of three phases: packet type,
 *  header, and data. The amount of data read during each phase is dependent
 *  upon the command. For a given phase, an attempt is made to read the full
 *  amount of data pertaining to that phase. If less than this amount is in
 *  the buffer, the amount read is subtracted from the remaining amount and
 *  function execution returns. When all data for a phase has been read, the
 *  next phase is started, or hci_common_transport_receive() is called if all
 *  data has been read.
 */
void sl_btctrl_hci_uart_read(void)
{
  uint16_t bytes_read;

  /* Check if data available */
  if (uart_rx_buffered_length() <= 0) {
    return;
  }

  bytes_read = uart_read(buf_byte_ptr, bytes_remaining);
  buf_byte_ptr += bytes_read;
  total_bytes_read += bytes_read;
  bytes_remaining -= bytes_read;

  if (bytes_remaining > 0) {
    return;
  }

  switch (state) {
    case hci_uart_state_read_packet_type:
    {
      packet_type = *buf_meta_ptr;
      buf_meta_ptr++;

      switch (packet_type) {
        case hci_packet_type_command:
        {
          bytes_remaining = hci_command_header_size;
          break;
        }
        case hci_packet_type_acl_data:
        {
          bytes_remaining = hci_acl_data_header_size;
          break;
        }
        default:
        {
          reception_failure();
          return;
        }
      }

      state = hci_uart_state_read_header;
      break;
    }
    case hci_uart_state_read_header:
    {
      switch (packet_type) {
        case hci_packet_type_command:
        {
          hci_command_t *cmd = (hci_command_t *) buf_meta_ptr;
          bytes_remaining = cmd->param_len;
          break;
        }
        case hci_packet_type_acl_data:
        {
          acl_packet_t *packet = (acl_packet_t *) buf_meta_ptr;
          bytes_remaining = packet->length;
          break;
        }
        default:
        {
          reception_failure();
          return;
        }
      }

      if (bytes_remaining == 0) {
        hci_common_transport_receive(hci_rx_buffer, total_bytes_read);
        reset();
        return;
      } else {
        state = hci_uart_state_read_data;
      }
      break;
    }
    case hci_uart_state_read_data:
    {
      hci_common_transport_receive(hci_rx_buffer, total_bytes_read);
      reset();
      break;
    }
    default:
    {
      reception_failure();
      return;
    }
  }
}

int16_t hci_common_transport_transmit(uint8_t *data, int16_t len)
{
  return uart_write(data, len); // TODO: Confirm return value with HCI!!
}

void hci_common_transport_init(void)
{
  reset();
  uart_init();
}
