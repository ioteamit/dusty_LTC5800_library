/*
      SmeIoT Library - SmartMeshIpCommand.ino

   This example shows some useful comand of the SmartMeshIp network

  created 08 12 2017
      by Mik (mik@ioteam.it)

   This example is in the public domain

      https://github.com/ioteamit/dusty_LTC5800_library

*/
#include <DustManager.h>

typedef enum {
	NoCommand,
	ListOfMac,
	ListOfPath,
	NetworkConfig,
	ManagerInfo
}FSMtestE;

char startMac[8];
FSMtestE fsm;

//=========================== "main" ==========================================

static void printByte(const uint8_t* payload, uint8_t length) {
	uint8_t i;

	SerialUSB.print(" ");
	for (i=0;i<length;i++) {
		SerialUSB.print(payload[i], HEX);
		if (i<length-1) {
			SerialUSB.print("-");
		}
	}
}

void notificationCB (uint8_t eventId, uint8_t mac[8]) {

	// only operational and lost are handled by the library
	switch (eventId)
	{
		case DN_EVENTID_EVENTMOTEOPERATIONAL:
		SerialUSB.print("Join mote = ");
		printByte(mac, 8);
		memcpy(startMac, mac ,8); // use the new Mac as start for listing
		break;

		case DN_EVENTID_EVENTMOTELOST:
		SerialUSB.print("Lost mote = ");
		printByte(mac, 8);
		break;

		default:
		break;
	}

}

static void printHelp(void) {
	SerialUSB.println("Example for some Network information:\n");
	SerialUSB.println("1) show List of Mote");
	SerialUSB.println("2) show List of Path");
	SerialUSB.println("3) getNetworkConfig");
	SerialUSB.println("4) getManagerInfo");
	SerialUSB.println("\n\nh) print this help");
}

void setup() {
	fsm = NoCommand;
	SerialUSB.begin(15200);
	
  while (!SerialUSB) {;};
	
	printHelp();
	dustManager.begin(false, NULL, PIN_DUST_CTS);

}
const dn_ipmg_getNetworkInfo_rpt* networkInfo;
const dn_ipmg_getSystemInfo_rpt* managerInfo;
const dn_ipmg_getMoteConfig_rpt*	moteInfo;
const dn_ipmg_getNextPathInfo_rpt*  pathInfo;

void loop() {
	
	char option;

	DustCbStatusE msgStatus = dustManager.readData();
	switch (msgStatus) {
		case Idle:
		break;

		case Working:
		SerialUSB.println("\n\r ");
		switch (fsm) {

			case ListOfMac:
			moteInfo = (const dn_ipmg_getMoteConfig_rpt*)dustManager.getLastCommand();
			// log
			SerialUSB.println("INFO:");
			SerialUSB.print("     MAC=");
			printByte(moteInfo->macAddress, sizeof(moteInfo->macAddress));
			SerialUSB.print("\n\r");
			SerialUSB.print("     state=");
			SerialUSB.println(moteInfo->state);
			if (moteInfo->isAP)
			SerialUSB.print("     is Manager");
			else
			SerialUSB.print("     is Mote");
			break;

			case ListOfPath:
			pathInfo = (const dn_ipmg_getNextPathInfo_rpt*)dustManager.getLastCommand();
			// log
			SerialUSB.println("INFO:");
			SerialUSB.print("          source=");
			printByte(pathInfo->source, 8);
			SerialUSB.println();
			SerialUSB.print("          dest=");
			printByte(pathInfo->dest,8);
			SerialUSB.println();
			SerialUSB.print("          direction=");
			SerialUSB.println(pathInfo->direction);
			SerialUSB.print("          numLinks=");
			SerialUSB.println(pathInfo->numLinks);
			SerialUSB.print("          quality=");
			SerialUSB.println(pathInfo->quality);
			SerialUSB.print("          pathId=");
			SerialUSB.println(pathInfo->pathId);
			break;
		}

		break;

		case Completed:
		SerialUSB.println("\n\rCompleted");
		switch (fsm) {

			case ListOfPath:
			break;

      case NetworkConfig:
      /*
       * 
       * from dn_ipmg.h
       * 
       * typedef struct {
       * uint8_t    RC;
       * uint16_t   numMotes;
       * uint16_t   asnSize;
       * uint8_t    advertisementState;
       * uint8_t    downFrameState;
       * uint8_t    netReliability;
       * uint8_t    netPathStability;
       * uint32_t   netLatency;
       * uint8_t    netState;
       * uint8_t    ipv6Address[16];
       * } dn_ipmg_getNetworkInfo_rpt
       */
			networkInfo = (const dn_ipmg_getNetworkInfo_rpt*)dustManager.getLastCommand();
      SerialUSB.println("INFO:");
      SerialUSB.print("Number of Mote = ");
      SerialUSB.println(networkInfo->numMotes);
			break;
			
			case ManagerInfo:
     /*
      * typedef struct {
      *    uint8_t    RC;
      *    uint8_t    macAddress[8];
      *    uint8_t    hwModel;
      *    uint8_t    hwRev;
      *    uint8_t    swMajor;
      *    uint8_t    swMinor;
      *    uint8_t    swPatch;
      *    uint16_t   swBuild;
      * } dn_ipmg_getSystemInfo_rpt
      */
			managerInfo = (dn_ipmg_getSystemInfo_rpt*)dustManager.getLastCommand();
      SerialUSB.println("INFO:");
      SerialUSB.print("SW Major = ");
      SerialUSB.println(managerInfo->swMajor);
      SerialUSB.print("SW Minor = ");
      SerialUSB.println(managerInfo->swMinor);
      SerialUSB.print("SW Patch = ");
      SerialUSB.println(managerInfo->swPatch);
      SerialUSB.print("SW Build = ");
      SerialUSB.println(managerInfo->swBuild);
			break;
		}
		fsm = NoCommand;
		break;

		case DataReceived:
		SerialUSB.println("DataMote");
		break;

		default:
		break;
	}

	if (SerialUSB.available()) {
		option = SerialUSB.read();
		
		switch (option) {
			case '1':
			fsm = ListOfMac;
			dustManager.listOfMac();
			break;
			
			case '2':
			fsm = ListOfPath;
			dustManager.listOfPath(startMac);
			break;
			
			case '3':
			fsm = NetworkConfig;
			dustManager.retrieveNetworkInfo();
			break;
			
			case '4':
			fsm = ManagerInfo;
			dustManager.retrieveManagerInfo();
			break;
			
			case 'h':
			printHelp();
			break;
			
			default:
			break;
		}
	}
}
