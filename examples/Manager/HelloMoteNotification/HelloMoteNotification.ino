/*
*     SmeIoT Library - HelloMoteNotification
*
*  This example, togheter with the HelloDustManager in the Mote examples, show the
*  comunication between the Motes and the Manager.
*
*  The mote are no more static but discovered by the manager.
*
* created 08 07 2016
      by Mik (mik@ioteam.it)

   This example is in the public domain
       https://bitbucket.org/ioteamit/arduino-dust-library
*/
#include <Arduino.h>
#include <DustManager.h>
uint8_t moteN=0;
IpMgDataModel *motes[10], *moteAll;
char brcMac[8]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};    // Broadcast MOTES
char mac[8]={0x0,0x17,0x0D,0x00,0x00,0x59,0x09,0x61};        // Put here your Mote mac

uint16_t srcPort = 61020;
uint16_t dstPort = 60000;
char msg[20]={'H','e','l','l','o',' ','D','u','s','t',' ','M','o','t','e',' ','9','9','9',' '};
unsigned long timeOut;
#define Sec3 3000

static void printHelp(void) {
	SerialUSB.println("Example for some Network information:\n");
	SerialUSB.println("0..9) send to the specific registered mote");
	SerialUSB.println("B) send Broadcast");
	SerialUSB.println("\n\nh) print this help\n");
}



static void printByte(uint8_t* payload, uint8_t length) {
	uint8_t i;

	SerialUSB.print(" ");
	for (i=0;i<length;i++) {
		SerialUSB.print(payload[i], HEX);
		if (i<length-1) {
			SerialUSB.print("-");
		}
	}
}

void eventReceiver(uint8_t eventId, uint8_t mac[8]){
	SerialUSB.print("\nEvent ");
	SerialUSB.print(eventId, HEX);
	SerialUSB.print(" received by ");
	printByte(mac, 8);

	// only operational and lost are handled by the library
	switch (eventId)
	{
		case DN_EVENTID_EVENTMOTEOPERATIONAL:
		// The mac will be copied in the internal attribute
		motes[moteN]   = new IpMgDataModel((const uint8_t*)mac, srcPort, dstPort);
		dustManager.registerMote(motes[moteN]);
		moteN++;
		break;

		case DN_EVENTID_EVENTMOTELOST:
		break;

		default:
		break;
	}


}


//=========================== "main" ==========================================

void setup() {
	SerialUSB.begin(115200);
	while (!SerialUSB) {
		;
	}

	timeOut = 0;

	moteAll = new IpMgDataModel((const uint8_t*)brcMac, srcPort, dstPort);
	
	printHelp();
	SerialUSB.println("Start Manager...\n");
	dustManager.begin(true, eventReceiver);

}

static void incrementMsg(void) {
	msg[18]++;

	if (msg[18]==':') {
		msg[17]++;
		msg[18]='0';
	}

	if (msg[17]==':'){
		msg[16]++;
		msg[17]='0';
	}

	if(msg[16]==':') {
		msg[16]='0';
	}
}


void loop() {
	dn_ipmg_getMoteConfig_rpt* reply;
	uint8_t len;
	char data=0;
	IpMgDataModel *mote;

	DustCbStatusE msgStatus = dustManager.readData();
	switch (msgStatus) {
		case DataReceived:
		SerialUSB.print("DataReceived:   ");

		mote = dustManager.getLastMessage();
		printByte((uint8_t*)mote->getDestinationMac(), 8);
		SerialUSB.print(" - ");
		SerialUSB.write(mote->fetchLastMessage(&len), len);
		SerialUSB.println();
		break;
	}


	if (SerialUSB.available()) {
		data = SerialUSB.read();
	}


	switch (data) {
		

		case 'B':
		case 'b':
					SerialUSB.println("Sending broadcast message");
		incrementMsg();

		timeOut = millis();

		moteAll->dataToSend((const uint8_t*)msg, 19);
		moteAll->setPriority(1);
		if (!dustManager.sendData(moteAll)) {
			SerialUSB.println("Error in Send Data");
		}
		break;
		
		case 'H':
		case 'h':
			printHelp();
			break;
			
		default:
		if (('0'<=data) && ('9'>=data)) {
			SerialUSB.print("Sending message to mote_");
			SerialUSB.println(data);
			incrementMsg();
			uint8_t moteId = (data-'0');
			timeOut = millis();
			if (motes[moteId]) {
				motes[moteId]->dataToSend((const uint8_t*)msg, 19);
				motes[moteId]->setPriority(1);
				if (!dustManager.sendData(motes[moteId])) {
					SerialUSB.println("Error in Send Data");
				}
			} else {
				SerialUSB.println("Mote NOT registered");
			}
		}
		break;

	}

	if ((millis() - timeOut) == Sec3) {
		timeOut =0;
	}
}
