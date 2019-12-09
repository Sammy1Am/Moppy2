/*
 * MoppyUDP.h
 *
 */

#ifndef SRC_MOPPYNETWORKS_MOPPYUDP_H_
#define SRC_MOPPYNETWORKS_MOPPYUDP_H_

#include "../MoppyConfig.h"
#include "../MoppyMessageConsumer.h"
#include "Arduino.h"
#include "MoppyNetwork.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>   //Local WebServer used to serve the configuration portal
#include <ESPAsyncWiFiManager.h> // https://github.com/alanswx/ESPAsyncWiFiManager WiFi Configuration Magic
#include <stdint.h>
#include <ArduinoOTA.h>

#define MOPPY_UDP_PORT 30994
#define MOPPY_MAX_PACKET_LENGTH 259

class MoppyUDP {
public:
    MoppyUDP(MoppyMessageConsumer *messageConsumer);
    void begin();
    void readMessages();

private:
    MoppyMessageConsumer *targetConsumer;
    uint8_t messagePos = 0;     // Track current message read position
    uint8_t messageBuffer[MOPPY_MAX_PACKET_LENGTH]; // Max message length for Moppy messages is 259
    const uint8_t pongBytes[8] = {START_BYTE, 0x00, 0x00, 0x04, 0x81, DEVICE_ADDRESS, MIN_SUB_ADDRESS, MAX_SUB_ADDRESS};
    void startOTA();
    bool startUDP();
    void parseMessage(uint8_t message[], int length);
    void sendPong();
};

#endif /* SRC_MOPPYNETWORKS_MOPPYUDP_H_ */