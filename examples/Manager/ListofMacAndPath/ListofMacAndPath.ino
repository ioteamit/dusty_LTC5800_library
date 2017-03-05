#include <DustManager.h>

typedef enum {
    NoCommand,
    ListOfMac,
    ListOfPath
}FSMtestE;

uint8_t listOfMac[8][10];
FSMtestE fsm;
IpMgDataModel *data;
char mac[8]={0x0,0x17,0x0D,0x00,0,0x30,0x8A,0x2A};      // NEW MOTE
//char mac[8]={0x0,0x17,0x0D,0x00,0,0x30,0x89,0x3c};    // OLD MOTE
uint16_t srcPort = 0x61020;
uint16_t dstPort = 0x60000;

//=========================== "main" ==========================================

void setup() {
    memset(listOfMac, 0 , sizeof(listOfMac));
    fsm = NoCommand;
    while (!SerialUSB) {
        ;
    }

    IpMgDataModel*  model = new IpMgDataModel();
    SerialUSB.println("Start");
    dustManager.begin(model);

    data = new IpMgDataModel((const uint8_t*)mac, srcPort, dstPort);
}

void printByte(uint8_t* payload, uint8_t length) {
    uint8_t i;

    SerialUSB.print(" ");
    for (i=0;i<length;i++) {
        SerialUSB.print(payload[i], HEX);
        if (i<length-1) {
            SerialUSB.print("-");
        }
    }
}

void loop() {
    dn_ipmg_getMoteConfig_rpt* reply;

    DustCbStatusE msgStatus = dustManager.readData();
    switch (msgStatus) {
        case Idle:
        break;
        case CommandSent:
        SerialUSB.println("Started");
        break;

        case Working:
        switch (fsm) {
            SerialUSB.println("Working");

            case ListOfMac:
            reply =(dn_ipmg_getMoteConfig_rpt*) dustManager.getLastMessage();
            if (reply->isAP==FALSE) {
                SerialUSB.print("Mote Mac = ");
                printByte(reply->macAddress, sizeof(reply->macAddress));
                } else {
                SerialUSB.print("Manager Mac = ");
                printByte(reply->macAddress, sizeof(reply->macAddress));

            }
            break;

            case ListOfPath:
            break;
        }

        break;

        case Completed:
        SerialUSB.println("Completed");
        switch (fsm) {
            SerialUSB.println("Working");

            case ListOfMac:
            fsm = ListOfPath;
            dustManager.listOfPath(mac);
            break;

            case ListOfPath:
            break;
        }
        break;

        case DataReceived:
        SerialUSB.println("DataMote");
        break;

        default:
        break;
    }

    if (isButtonOnePressed()){
        fsm = ListOfMac;
        dustManager.listOfMac();
    }

    if (isButtonTwoPressed()){
        data->dataToSend((const uint8_t*)"Hello", 5);
        dustManager.sendData(data);
    }
}
