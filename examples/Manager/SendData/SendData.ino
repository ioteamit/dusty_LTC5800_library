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
uint16_t srcPort = 61020;
uint16_t dstPort = 60000;

//=========================== "main" ==========================================

void setup() {
    memset(listOfMac, 0 , sizeof(listOfMac));
    fsm = NoCommand;
    while (!SerialUSB) {
        ;
    }

    // The data model for the Manager is used only to send data to Mote
    data = new IpMgDataModel((const uint8_t*)mac, srcPort, dstPort);

    SerialUSB.println("Start");
    smeDManager.begin(35, &Serial1);

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

    DustCbStatusE msgStatus = smeDManager.readData();
    switch (msgStatus) {
    case DataSent:
        SerialUSB.println("Data SENT");
        break;
    }

    if (isButtonOnePressed()){
        data->dataToSend((const uint8_t*)"Hello", 5);
        data->setPriority(1);
        smeDManager.sendData(data);
    }
}



