#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "notorios.h"
#include "project.h"

/* === Modem UART Functions  */

#define MAX_RETRY 8
#define MAX_REQUEST 1024

static char chars[257] = {0};
static uint8_t num_chars = 0;
static bool error = false;

static char *line = chars;
static const char *match = NULL;
static const char *pattern = NULL;
int _write(int file, char *buf, int len);

static void clear_chars(void) {
  printf("[%x]\n", time());
  _write(1, chars, num_chars);
  Modem_UART_ClearRxBuffer();
  memset((void*)chars, 0, 256);
  num_chars = 0;
}

static bool wait_for(const char *string, uint32_t timeout) {
  pattern = string, match = NULL, error = false;
  uint32_t end = time() + timeout;
  while (!match && !error && time() < end);
  return match != NULL;
}

CY_ISR(Modem_ISR) {
  /* Store received char in chars */
  char rx_char = Modem_UART_GetChar();
  if (rx_char) chars[num_chars++] = rx_char;

  /* Check pattern at end of line */
  if (rx_char == '\r') {
    error = (error || strstr(line, "ERROR") != NULL);
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
  /* Initialize UART interrupts */
  Modem_UART_Start();
  Modem_ISR_StartEx(Modem_ISR);
  Modem_ON_Write(1);
  Modem_PWR_Write(1);

  /* Start boot sequence */
  Modem_ON_Write(0);
  wait_ms(100);
  Modem_ON_Write(1);

  /* Poll for response */
  for (uint8_t i = 0; i < 30; ++i)
    if (at_write("AT\r", 1)) return true;
  return false;
}

static void power_off(void) {
  at_write("AT^SMSO\r", 2);
  Modem_PWR_Write(0);
  Modem_ON_Write(0);
  Modem_ISR_Stop();
  Modem_UART_Stop();
}

static void reset(void) {
  at_write("AT+CFUN=1,1\r", 2);
  Modem_RST_Write(1);
  wait_ms(200);
  Modem_RST_Write(0);
  wait_ms(10000);
}

/* == Modem Interactions == */

static bool setup(const modem_t *config) {
  /* Set configured APN, full functionality, verbose errors */
  return (at_write("AT+CGDCONT=3,\"IPV4V6\",\"%s\"\r", 2, config->apn) &&
    at_write("AT+CFUN=1\r", 2) && at_write("AT+CMEE=2\r", 2));
}

static bool registered(void) {
  /* Poll until registered to home network */
  for (uint8_t i = 0; i < 30; ++i, wait_ms(1000)) {
    at_write("AT+CEREG?\r", 2);
    if (strstr(chars, "+CEREG: 0,0")) return false;
    if (strstr(chars, "+CEREG: 0,1")) return true;
  }
  return false;
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
  if (!registered() || !at_write("AT^SICA?\r", 2)) return false;
  unsigned pdp_state = atoi(strstr(chars, "^SICA: 3,") + 9);
  if (pdp_state == activate) return true;
  if (!at_write("AT^SICA=%u,3\r", 60, activate))
    return at_write("AT+CEER\r", 2), false;
  return true;
}

static bool get_meid(char *meid) {
  /*
  Return cell module ICCID

  Example Conversation:
  [Board] AT+CCID?
  [Modem] +CCID: "A1000049AB9082",""

          OK
  */
  if (!at_write("AT+CCID?\r", 2)) return false;
  const char *start = strstr(chars, "+CCID: \"") + 8;
  const char *end = strstr(start, "\",\"");
  if (!start || !end || end - start > 24) return false;
  memcpy(meid, start, end - start);
  return true;
}

static bool socket_connected(void) {
  /*
  Checks status of internet service profile 0 is 4 (Up)
    
  Example Conversation:
  [Board] AT^SISI?
  [Modem] ^SISI: 0,4,20,3,3,0
    
          OK
  */
  for (uint8_t i = 0; i < 15; ++i, wait_ms(1000)) {
    at_write("AT^SISI=0\r", 2);
    if (strstr(chars, "^SISI: 0,6")) return false;
    if (strstr(chars, "^SISI: 0,4")) return true;
  }
  return false;
}

static bool socket_open(const char *host) {
  /* Open TCP socket to specified address */
  return (at_write("AT^SISS=0,srvType,\"Socket\"\r", 2) &&
    at_write("AT^SISS=0,conID,\"3\"\r", 2) &&
    at_write("AT^SISS=0,address,\"socktcp://%s:%u\"\r", 2, host, 80) &&
    at_write("AT^SISO=0\r", 2) && socket_connected());
}

static bool socket_close(void) {
  /* Close active TCP socket */
  return at_write("AT^SISC=0\r", 2);
}

/* == High Level Interface == */

static char req_buf[MAX_REQUEST] = {0};

static bool send_request(char *request, uint32_t len) {
  /* Write request string to socket in blocks */
  uint8_t n_blocks = (len + 255) / 256;
  for (uint8_t i = 0; i < n_blocks; ++i) {
    uint32_t wlen = (i == n_blocks - 1 ? len % 256 : 256);
    at_write("AT^SISW=0,%u,0\r", 5, wlen);
    wlen = _write(MODEM_STREAM, request + i * 256, wlen);
    if (!socket_connected()) return false;
  }
  return at_write("AT^SISW=0,0,1\r");
}

static bool read_status() {
  /* Check response code in 200 range */
  at_write("AT^SISR=0,224\r", 5);
  const char *resp = strstr(chars, "HTTP/1.1");
  return resp && resp[9] == '2';
}

static uint32_t read_response(char *buf, uint32_t max_len) {
  /* Read response length */
  if (!read_status()) return 0;
  uint32_t len = atoi(strstr(chars, "Content-Length: "));
  len = (len > max_len ? max_len : len);

  /* Discard remaining headers */
  const char *resp = 0;
  while (!(resp = strstr(chars, "\n\r")))
    at_write("AT^SISR=0,224\r", 5);
  const char *end = strstr(resp, "\r\n");

  /* Keep copying chars to buf */
  uint32_t clen = end - resp;
  for (uint32_t i = 0; i < len; i += clen) {
    memcpy(buf + i, resp, clen);
    at_write("AT^SISR=0,224\r", 5);
    resp = strstr(chars, "\r\n") + 2;
    end = strstr(resp, "\r\n");
  }
  return len;
}

static char *write_post(const modem_t *config, const char *path, uint32_t len) {
  char *ptr = (char*)memset(req_buf, 0, MAX_REQUEST);
  ptr += sprintf(ptr, "POST %s HTTP/1.1\r\n", path);
  ptr += sprintf(ptr, "Host: %s\r\nConnection: Close\r\n", config->host);
  ptr += sprintf(ptr, "X-Api-Key: %s\r\n", config->auth);
  ptr += sprintf(ptr, "Content-Type: application/octet-stream\r\n");
  ptr += sprintf(ptr, "Accept: application/octet-stream\r\n");
  return ptr + sprintf(ptr, "Content-Length: %u\r\n\r\n", len);
}

static bool write_readings(const modem_t *config) {
  if (!socket_open(config->host)) return socket_close(), false;
  uint32_t len = num_msgs(config->port_in);
  uint32_t size = sizeof(header_t) + len * sizeof(reading_t);
  char *ptr = write_post(config, "/write", size);

  /* Copy sensor readings to request body */
  memcpy(ptr, &(config->header), sizeof(header_t));
  ptr += sizeof(header_t);
  for (uint8_t i = 0; i < len; ++i) {
    if (ptr - req_buf + sizeof(reading_t) > MAX_REQUEST) break;
    memcpy(ptr, &(recv(config->port_in)->reading), sizeof(reading_t));
    ptr += sizeof(reading_t);
  }

  /* Check response OK, otherwise unack msgs */
  bool ok = send_request(req_buf, ptr - req_buf) && read_status();
  if (!ok) printf("Failed upload\n"), unack(config->port_in, len);
  return socket_close() && ok;
}

static bool read_updates(const modem_t *config) {
  if (!socket_open(config->host)) return socket_close(), false;
  char *ptr = write_post(config, "/read/flag", field_size(header_t, id));
  memcpy(ptr, config->header.id, field_size(header_t, id));
  send_request(req_buf, ptr + field_size(header_t, id) - req_buf);

  /* Verify magic bytes in header_t */
  uint32_t len = read_response(req_buf, MAX_REQUEST);
  if (((header_t*)req_buf)->magic != 0xf100d100) {
    printf("Invalid download\n");
    return socket_close(), false;
  }
  
  /* Copy messages to local port */
  msg_t msg_out;
  for (uint32_t i = sizeof(header_t); i < len; i += sizeof(reading_t)) {
    memcpy(&msg_out.reading, req_buf + i, sizeof(reading_t));
    send(config->port_out, &msg_out);
  }
  return socket_close();
}

void run_modem(modem_t *config) {
  while (true) {
    if (num_msgs(config->port_in) == 0) yield();
    for (uint8_t i = 0; i < MAX_RETRY; ++i, reset()) {
      if (power_on() &&
        setup(config) &&
        set_context(true) &&
        get_meid(config->header.id) &&
        write_readings(config) &&
        read_updates(config)) break;
    }
    power_off();
    sleep(config->interval);
  }
}
