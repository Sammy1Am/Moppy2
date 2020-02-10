#include <Arduino.h>

/**********
 * MoppyInstruments handle the sound-creation logic for your setup.  The
 * instrument class provides a systemMessage handler function and a deviceMessage
 * handler function for handling messages received by the network.
 *
 * Uncomment the appropriate instrument class for your setup
 */

// Floppy drives directly connected to the Arduino's digital pins
#include "src/MoppyInstruments/FloppyDrives.h"
FloppyDrives instrument = FloppyDrives();


//Uncomment the 2 next lines and comment the 2 lines above this comment to switch to L298N mode
//#include "src/MoppyInstruments/L298N.h"
//L298N instrument = L298N(); // please see src/MoppyInstruments/L298N.h for pinout and additionnal info


// A single device (e.g. xylophone, drums, etc.) connected to shift registers
//#include "src/MoppyInstruments/ShiftRegister.h"
//ShiftRegister instrument = ShiftRegister();

/**********
 * MoppyNetwork classes receive messages sent by the Controller application,
 * parse them, and use the data to call the appropriate handler as implemented
 * in the instrument class defined above.
 *
 * Uncomment the appropriate networking class for your setup
 */

// Standard Arduino HardwareSerial implementation
#include "src/MoppyNetworks/MoppySerial.h"
MoppySerial network = MoppySerial(instrument.systemMessage, instrument.deviceMessage);

// Standard Arduino HardwareSerial implementation  (Still WIP / Maintaned by Magnusm1709)
//#include "src/MoppyNetworksMidi/MoppyMidi.h"
//MoppyMidi network = MoppyMidi(instrument.deviceMessage);



//// UDP Implementation using some sort of network stack?  (Not implemented yet)
// #include "src/MoppyNetworks/MoppyUDP.h"
// MoppyUDP network = MoppyUDP(instrument.systemMessage, instrument.deviceMessage);


//The setup function is called once at startup of the sketch
void setup()
{
	// Call setup() on the instrument to allow to to prepare for action
    instrument.setup();

    // Tell the network to start receiving messages
    network.begin();
}

// The loop function is called in an endless loop
void loop()
{
	// Endlessly read messages on the network.  The network implementation
	// will call the system or device handlers on the intrument whenever a message is received.
    network.readMessages();
}
