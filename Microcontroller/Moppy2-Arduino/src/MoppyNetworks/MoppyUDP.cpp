#include "MoppyUDP.h"

#ifdef ARDUINO_ARCH_ESP8266 // For now, this will only work with ESP8266

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
    startOTA();
    startUDP();
}

// connect to UDP – returns true if successful or false if not
bool MoppyUDP::startUDP() {
    bool connected = false;

    Serial.println("");
    Serial.println("Connecting to UDP");

    if (UDP.beginMulticast(WiFi.localIP(), IPAddress(239, 2, 2, 7), MOPPY_UDP_PORT) == 1) {
        Serial.println("Connection successful");
        connected = true;
    } else {
        Serial.println("Connection failed");
    }

    return connected;
}

void MoppyUDP::startOTA() {
    ArduinoOTA.setPort(8377);
    ArduinoOTA.setPassword("flashdrive");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
}

void MoppyUDP::readMessages() {
    // Handle OTA
    ArduinoOTA.handle();

    // Handle UDP packets
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
    if (message[1] == SYSTEM_ADDRESS) {
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
    UDP.beginPacket(IPAddress(239, 2, 2, 7), 30994);
    UDP.write(pongBytes, sizeof(pongBytes));
    UDP.endPacket();
}
#endif /* ARDUINO_ARCH_ESP8266 */
