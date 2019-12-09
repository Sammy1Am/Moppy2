#include "MoppyUDP.h"

WiFiUDP UDP;

/*
 * Serial communications implementation for Arduino.  Instrument
 * has its handler functions called for device and system messages
 */
MoppyUDP::MoppyUDP(MoppyMessageConsumer *messageConsumer) {
    targetConsumer = messageConsumer;
}

void MoppyUDP::begin() {
    Serial.begin(115200); // For debugging

    // Setup and connect to WiFi
    AsyncWebServer server(80);
    DNSServer dns;
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.autoConnect("FloppyDrives", "m0ppydrives");
    startUDP();
}

// connect to UDP – returns true if successful or false if not
bool MoppyUDP::startUDP() {
    bool connected = false;

    Serial.println("");
    Serial.println("Connecting to UDP");

    if (UDP.beginMulticast(WiFi.localIP(), IPAddress(239,2,2,7), MOPPY_UDP_PORT) == 1) {
        Serial.println("Connection successful");
        connected = true;
    } else {
        Serial.println("Connection failed");
    }

    return connected;
}

void MoppyUDP::readMessages() {
    int packetSize = UDP.parsePacket();
    if (packetSize) {
        // Serial.println("");
        // Serial.print("Received packet of size ");
        // Serial.println(packetSize);
        // Serial.print("From ");
        // IPAddress remote = UDP.remoteIP();
        // for (int i = 0; i < 4; i++) {
        //     Serial.print(remote[i], DEC);
        //     if (i < 3) {
        //         Serial.print(".");
        //     }
        // }
        // Serial.print(", port ");
        // Serial.println(UDP.remotePort());

        // read the packet into messageBuffer
        int messageLength = UDP.read(messageBuffer, MOPPY_MAX_PACKET_LENGTH);
        // Parse
        parseMessage(messageBuffer, messageLength);

        UDP.flush(); // Just incase we got a really long packet
    }

    // udpLoopCounter++;

    // if (udpLoopCounter >= 50000) {
    //     Serial.println("Sending packet");
    //     udpLoopCounter = 0;
    //     UDP.beginPacket(IPAddress(192, 168, 7, 169), 64460);
    //     UDP.write(ReplyBuffer);
    //     UDP.endPacket();
    // }
}

/* MoppyMessages contain the following bytes:
 *  0    - START_BYTE (always 0x4d)
 *  1    - Device address (0x00 for system-wide messages)
 *  2    - Sub address (Ignored for system-wide messages)
 *  3    - Size of message body (number of bytes following this one)
 *  4    - Command byte
 *  5... - Optional payload
 */
void MoppyUDP::parseMessage(uint8_t message[], int length) {
    if (length < 5 || message[0] != START_BYTE || length != (4 + message[3])) {
        return; // Message is too short, not a Moppy Message, or wrongly sized
    }

    // Only worry about this if it's addressed to us
    if (message[1] == SYSTEM_ADDRESS ) {
        if (messageBuffer[4] == NETBYTE_SYS_PING) {
            sendPong(); // Respond with pong if requested
                            } else {
                                targetConsumer->handleSystemMessage(messageBuffer[4], &messageBuffer[5]);
                            }
    } else if (message[1] == DEVICE_ADDRESS) {
        targetConsumer->handleDeviceMessage(messageBuffer[2], messageBuffer[4], &messageBuffer[5]);
    }
}

void MoppyUDP::sendPong() {
    UDP.beginPacket(IPAddress(239,2,2,7), 30994);
    UDP.write(pongBytes, sizeof(pongBytes));
    UDP.endPacket();
}
