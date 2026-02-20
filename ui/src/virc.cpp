#if (SIMULATOR == 0) && (SENSCAP_EN == 0)

#define DECODE_NEC // Decodes only NEC format data and neglects all other IR formats data
#include "jsonrpc2.h"
#include <Arduino.h>    // Arduino 1.8.19
#include <IRremote.hpp> //https://github.com/Arduino-IRremote/Arduino-IRremote/tree/v3.6.1

#define IR_RECEIVE_PIN 10

void virc_setup() {

  pinMode(IR_RECEIVE_PIN, INPUT);

  // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin
  // from the internal boards definition as default feedback LED
  IrReceiver.begin(IR_RECEIVE_PIN, false);
}

void read_virc_data() {
  /*
   * Check if received data is available and if yes, try to decode it.
   * Decoded result is in the IrReceiver.decodedIRData structure.
   *
   * E.g. command is in IrReceiver.decodedIRData.command
   * address is in command is in IrReceiver.decodedIRData.address
   * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
   */
  if (IrReceiver.decode()) {
    uint32_t ir_raw = IrReceiver.decodedIRData.decodedRawData;
    if (ir_raw != 0) {

      // SERIAL_PORT.println(IrReceiver.decodedIRData.decodedRawData);
      JsonDocument doc;
      JsonObject params = doc["params"].to<JsonObject>();

      params["value"] = IrReceiver.decodedIRData.decodedRawData;
      serializeJsonRpcRequest(0, CMD_VIRC, params);
      if (lv_disp_get_inactive_time(NULL) > INACTIVE_TIMEOUT) {
        // SERIAL_PORT.println("VIRC: Waking up from sleep mode");
        // This flag is made false, so that first touch event is considered without neglecting
        flg_disp_sleep = false;
      }
      // trigger dummy activity to come out of sleep mode if the display is in sleep mode
      lv_disp_trig_activity(NULL);
    }

    /*
     * !!!Important!!! Enable receiving of the next value,
     * since receiving has stopped after the end of the current received data
     * packet.
     */
    IrReceiver.resume(); // Enable receiving of the next value
  }
}

#endif // SIMULATOR