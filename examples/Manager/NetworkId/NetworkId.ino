/*
*     SmeIoT Library - NetworkID
*
*  This example shows how to change the NetworkId.
*   
*	Different NetworkId are required when more than one dust network is on the same area
*
* created 10 07 2017
      by Mik (mik@ioteam.it)

   This example is in the public domain
       https://bitbucket.org/ioteamit/arduino-dust-library
*/

#include <Arduino.h>
#include <DustManager.h>

int newId = 1234; // write here your new NetworkId
boolean readId=true;

//=========================== "main" ==========================================

void setup() {
	SerialUSB.begin(115200);
	
	while (!SerialUSB) {
		;
	}

	SerialUSB.println("Start Manager...\n");
	dustManager.begin(false, NULL);
	dustManager.retrieveNetworkConfig();

}



void loop() {
	dn_ipmg_getNetworkConfig_rpt* netMsg;
	uint8_t len;
	char data=0;
	IpMgDataModel *mote;

	DustCbStatusE msgStatus = dustManager.readData();
	switch (msgStatus) {
		case Completed:
		if (readId) {
			netMsg = (dn_ipmg_getNetworkConfig_rpt *)dustManager.getLastCommand();
			SerialUSB.print("Network Id = ");
			SerialUSB.println(netMsg->networkId);
		} else {
			readId = true;
			dustManager.retrieveNetworkConfig();
		}
		break;
	}


	if (SerialUSB.available()) {
		data = SerialUSB.read();
	}


	if (('C'==data) || ('c'==data)){
		netMsg->networkId = newId;
		readId = false;
		dustManager.changeNetworkConfig(netMsg);
	}
}


