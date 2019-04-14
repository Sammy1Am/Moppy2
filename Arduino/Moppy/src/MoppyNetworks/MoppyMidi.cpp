#include "../MoppyNetwork/MoppyMidi.h"
/*
 * Serial communications implementation for Arduino.  Handler
 * functions are called to consume system and device messages received from
 * midi devices.
 */


byte actPlayingNote[MAX_SUB_ADDRESS]; // MAX_SUB_ADDRESS = Number of Drives
MoppyMidi::MoppyMidi(handleDeviceMessage dev) {
    deviceHandler = dev;
}

void MoppyMidi::begin(long baud) {
    Serial.begin(baud); // need to be 31250 for midi
}

void MoppyMidi::readMessages() {
    
    // Check if minimum needed data availabel
    // if minimum neede data is not reached it stops the function
    if(Serial.available() < 3 && ONLY3BYTE){
        return;
    }
    
    // Check if the byte is a "firstByte" (because they are bigger than 127 (128...255))
    // if it is not a first byte restart the routine
    byte firstByte = Serial.read(); // also called statusByte
    if(firstByte < 128){
        Serial.write(firstByte);
        return;
    }
    
    
    // Check if the byte is a "secondByte"/"thirdByte" (because they are smaller than 128  (0...127))
    byte secondByte = Serial.read(); // Databyte
    if(secondByte > 127){
        Serial.write(firstByte);
        Serial.write(secondByte);
        return;
    }
    
    
    // Can be deleted because data is not needed
    byte thirdByte = Serial.read(); // dataByte
    if(thirdByte > 127){
        Serial.write(firstByte);
        Serial.write(secondByte);
        Serial.write(thirdByte);

        return;
    }
    
    // start docode
    boolean firstBits[8];// = getBits(firstByte); // Not used because of limitations of C++/INO
    
    
    
    for( int i = 0; i < 8; i++){
        firstBits[i] = bitRead(firstByte, 7-i);
    }
    
    // Debug:
    //Serial.println(String(firstByte) + " : " + String(secondByte) + " : " + String(thirdByte));
    
    // Not needed, information will be get direct out of byte
    //boolean secondBits[8] = getBits(secondByte);
    
    //Not needed, Floppy has no dynmaics ("loudness")
    //boolean thirdBits[8] = getBits(thirdByte);
    
    
    // Check if noteOn command          if not -> stop function
    // TODO also check NoteOff
    if( (firstBits[1] == 0 && firstBits[2] == 0 && firstBits[3] == 1)){
        if(secondByte == 0){
            firstBits[3] = 0;
        }else{
            if(STEREO){
                for(int i = 0; i <= MAX_SUB_ADDRESS/2; i++){
                    if(actPlayingNote[i] == 0){
                        actPlayingNote[i] = secondByte;
                        actPlayingNote[i+MAX_SUB_ADDRESS/2] = secondByte;
                        deviceHandler(i+1, 0x09, &secondByte);
                        deviceHandler(i+1+MAX_SUB_ADDRESS/2, 0x09, &secondByte);
                        i = MAX_SUB_ADDRESS + 1;
                        return;
                    }
                }
            }else{
                for(int i = 0; i <= MAX_SUB_ADDRESS; i++){
                    if(actPlayingNote[i] == 0){
                        actPlayingNote[i] = secondByte;
                        deviceHandler(i+1, 0x09, &secondByte);
                        i = MAX_SUB_ADDRESS + 1;
                        return;
                    }
                }
            }
        }
    }
    if( (firstBits[1] == 0 && firstBits[2] == 0 && firstBits[3] == 0)){
        for (int i = 0; i <= MAX_SUB_ADDRESS; i++){
            if(secondByte == actPlayingNote[i]){
                actPlayingNote[i] = 0;
                deviceHandler(i+1, 0x08, &secondByte);
                return;
            }
        }
    }
    Serial.write(firstByte);
    Serial.write(secondByte);
    Serial.write(thirdByte);
}
