#include "mjson.h"  // Sketch -> Add File -> Add mjson.h

static int ledOn = 0;  // Current LED status

// Gets called by the RPC engine to send a reply frame
static int sender(const char *frame, int frame_len, void *privdata) {
  return Serial.write(frame, frame_len);
}

static void reportState(void) {
  mjson_printf(sender, NULL,
               "{\"method\":\"Shadow.Report\",\"params\":{\"on\":%s}}",
               ledOn ? "true" : "false");
}

static void shadowDeltaHandler(struct jsonrpc_request *r) {
  int ledOn = 0;
  mjson_get_bool(r->params, r->params_len, "$.on", &ledOn);
  digitalWrite(LED_BUILTIN,
               ledOn);              // Set LED to the "on" value
  reportState();                    // Let shadow know our new state
  jsonrpc_return_success(r, NULL);  // Report success
}

void setup() {
  jsonrpc_init(NULL, NULL);
  jsonrpc_export("Shadow.Delta", shadowDeltaHandler);

  pinMode(LED_BUILTIN, OUTPUT);  // Configure LED pin
  Serial.begin(115200);          // Init serial comms
  reportState();                 // Let shadow know our state
}

static void process_byte(unsigned char ch) {
  static char buf[256];  // Buffer that holds incoming frame
  static size_t len;     // Current frame length

  if (len >= sizeof(buf)) len = 0;  // Handle overflow - just reset
  buf[len++] = ch;                  // Append to the buffer
  if (ch == '\n') {                 // On new line, parse frame
    jsonrpc_process(buf, len, sender, NULL, NULL);
    len = 0;
  }
}

void loop() {
  if (Serial.available() > 0) process_byte(Serial.read());
}
