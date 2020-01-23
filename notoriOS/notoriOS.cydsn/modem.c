#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "notorios.h"
#include "project.h"

/* === Modem UART Functions  */

#define MAX_STARTS 8
#define MAX_REQUEST 256
#define FIXED_HEADER "Content-Length: "
#define CHUNKED_HEADER "Transfer-Encoder: chunked"

static char chars[257] = {0};
static uint8_t num_chars = 0;

static const char *line = chars;
static const char *match = NULL;
static const char *pattern = NULL;

static void clear_chars(void) {
  Modem_UART_ClearRxBuffer();
  memset((void*)chars, 0, 256);
  num_chars = 0;
}

static bool wait_for(const char *string, uint32_t timeout) {
  pattern = string, match = NULL;
  uint32_t end = time() + timeout;
  while (!match && time() < end);
  return match != NULL;
}

CY_ISR(Modem_ISR) {
  /* Store received char in chars */
  char rx_char = Modem_UART_GetChar();
  if (rx_char) chars[num_chars++] = rx_char;

  /* Check pattern at end of line */
  if (rx_char == '\r') {
    match = strstr(line, pattern);
    line = chars + num_chars;
  }
}

/* === Modem Power === */

static bool at_write(const char *format, uint32_t timeout, ...) {
  clear_chars();
  va_list argp;
  va_start(argp, timeout);
  vdprintf(MODEM_STREAM, format, argp);
  va_end(argp);
  return wait_for("OK", timeout);
}

static bool power_on(void) {
  // Initialize UART interrupts
  Modem_UART_Start();
  Modem_ISR_StartEx(Modem_ISR);
  Modem_ON_Write(1);
  Modem_PWR_Write(1);
  // Start boot sequence
  Modem_ON_Write(0);
  wait_ms(100);
  Modem_ON_Write(1);
  return wait_for("^SYSTART", 30);
}

static void power_off(void) {
  at_write("AT^SMSO\r", 5);
  Modem_PWR_Write(0);
  Modem_ON_Write(0);
  Modem_ISR_Stop();
  Modem_UART_Stop();
}

static void reset(void) {
  Modem_RST_Write(1);
  wait_ms(200);
  Modem_RST_Write(0);
  wait_ms(5000);
}

/* == Modem Interactions == */

static bool setup(const modem_t *config) {
  // Set APN to config->apn
  return (at_write("AT+CGDCONT=3,\"IPV4V6\",\"%s\"\r", 1, config->apn) &&
          at_write("AT+CFUN=1\r", 1) && // Set functionality to full
          at_write("AT+CMEE=2\r", 1));  // Set verbose error reports
}

static bool set_context(bool activate) {
  /*
  Activate/deactivate internet service using PDP context 3
  
  Example Conversation:
  [Board] AT^SICA?
  [Modem] ^SICA: 1,1
          ^SICA: 3,0
  
          OK
  */
  if (!at_write("AT^SICA?\r", 1)) return false;
  // Check if PDP context 3 state already matches 
  unsigned pdp_state = atoi(strstr(line, "^SICA: 3,") + 9);
  if (pdp_state == activate) return true;
  return at_write("AT^SICA=%u,3\r", 60, activate);
}

static uint8_t startup(const modem_t *config) {
  for (uint8_t i = 1; i < MAX_STARTS; ++i) {
    // Turn on modem, cycle power on failure
    if (!power_on()) { reset(); continue; }
    // Attempt to connect to network
    if (setup(config) && set_context(true)) return i;
  }
  power_off(); return MAX_STARTS;
}

static bool get_meid(char *meid) {
  /*
  Return cell module ICCID

  Example Conversation:
  [Board] AT+CCID?
  [Modem] +CCID: "A1000049AB9082",""

          OK
  */
  if (!at_write("AT+CCID?\r", 1)) return false;
  const char *start = strstr(chars, "+CCID: \"") + 8;
  const char *end = strstr(start, "\",\"");
  if (!start || !end || end - start > 20) return false;
  memcpy(meid, start, end - start);
  return true;
}

static bool socket_open(const char *host) {
  // Open TCP socket to specified address
  return (at_write("AT^SISS=0,srvType,\"Socket\"\r", 20) &&
    at_write("AT^SISS=0,conID,\"3\"\r", 20) &&
    at_write("AT^SISS=0,\"address\",\"socktcp://%s\"\r", 20, host) &&
    at_write("AT^SISO=0\r", 20) && wait_for("^SISW: 0, 1", 30));
}

static bool socket_close(void) {
  // Close active TCP socket
  return at_write("AT^SISC=0\r", 1);  
}

#define BLOCK_LEN 256

static void send_request(const char *request, uint32_t len) {
  // Write request string to socket in blocks
  uint8_t n_blocks = (len + BLOCK_LEN - 1) / BLOCK_LEN;
  for (uint8_t i = 0; i < n_blocks; ++i) {
    const char *block = request + i * BLOCK_LEN;
    uint32_t write_len = (i < n_blocks - 1 ? BLOCK_LEN : len % BLOCK_LEN);
    at_write("AT^SISW=0,%u\r", 10, write_len);
    at_write("%.*s\032", 10, write_len, block);
  }
}

/*static bool read_response(const char *msg, uint32_t max_size) {
  at_write("AT^SISR=0,256\r", 10);

  // Read response length from buffer
  const char *fixed_len = strstr(modem_buf, FIXED_HEADER);
  const char *chunked = strstr(modem_buf, CHUNKED_HEADER);
  uint32_t chunk_size = 256;
  if (fixed_len) {
    fixed_len += strlen(FIXED_HEADER);
    chunk_size = atoi(fixed_len);
  } else if (!chunked) return false;

  while (start < end) {
    at_write("AT^SISR=0,256\r", 10);
  }
}*/

static void post_readings(const modem_t *config) {
  uint32_t len = num_msgs(config->port) * sizeof(reading_t);
  char req_buf[MAX_REQUEST] = {0};
  char *ptr = req_buf;

  // Write POST request headers
  ptr += sprintf(ptr, "POST /%s HTTP/1\r\n", "");
  ptr += sprintf(ptr, "Host: %s\r\nConnection: Close\r\n", config->host);
  ptr += sprintf(ptr, "Authorization: Basic %s\r\n", config->auth);
  ptr += sprintf(ptr, "Content-Type: application/octet-stream\r\n");
  ptr += sprintf(ptr, "Content-Length: %lu\r\n\r\n", len + 8);

  // Copy sensor readings to request body
  *(uint32_t*)ptr = 0xf100d000; ptr += 4;
  memcpy(ptr, config->node_id, 20); ptr += 20;
  for (uint8_t i = 0; i < num_msgs(config->port); ++i) {
    if (ptr - req_buf + sizeof(reading_t) > MAX_REQUEST) break;
    memcpy(ptr, &(recv(config->port)->reading), sizeof(reading_t));
    ptr += sizeof(reading_t);
  }
  send_request(req_buf, ptr - req_buf);
}

void run_modem(modem_t *config) {
  while (true) {
    if (num_msgs(config->port) == 0) yield();
    if (startup(config) != MAX_STARTS) {
      get_meid(config->node_id);
      socket_open(config->host);
      post_readings(config);
      //get_updates();
      socket_close();
    }
    power_off();
    sleep(config->interval);
  }
}