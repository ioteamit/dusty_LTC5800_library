/*
 *     SmeIoT Library - HelloDustManager
 * 
 *  This example, togheter with the HelloDustMote in the Manager examples, show the 
 *  comunication between the Motes and the Manager.
 *  
 * 
 * created 08 07 2016
      by Mik (mik@ioteam.it)

   This example is in the public domain
       https://bitbucket.org/ioteamit/arduino-dust-library
 */

#include <Arduino.h>
#include <DustMote.h>

#define MIN_DIST 11
#define MAX_DIST 30

IpMtDataModel *dataModel;
char *hello="Hello Dust Manager";
uint8_t dataStream[50];
uint8_t len=0;
//=========================== data generator ==================================

uint8_t* generateData(DataModel *model) {

    memcpy(dataStream, hello, 18);
    model->setDatasize(18);

    return (uint8_t*) dataStream;
}


//===========================  CONNECTION STATUS ================================
void status_notification (uint8_t status) {
    static uint8_t last_status = 0; // UNKNOWN STATE;
    uint16_t    msec = 10;

    if (status != last_status) {
        if (MOTE_STATE_OPERATIONAL == status) {
#ifdef ARDUINO_SAMD_SMARTEVERYTHING_FOX
            ledGreenLight(HIGH);
#endif
            msec = 500;            // wait for 300 ms
        }

#ifdef ARDUINO_SAMD_SMARTEVERYTHING_FOX
        ledRedLight(HIGH);
        delay(msec);
        ledRedLight(LOW);      
#endif

        last_status = status;
        SerialUSB.print("New State: ");
        SerialUSB.println(status, DEC);
    }
}

//=========================== "main" ==========================================

void setup() {
    SerialUSB.begin(115200);
    while (!SerialUSB){
        ;
    } 
    SerialUSB.println("start Mote...\n");

    dataModel = new IpMtDataModel(generateData);

    dustMote.begin(
            60000,                         // srcPort
            (uint8_t*)ipv6Addr_manager,    // destAddr
            61020,                         // destPort
            500,                           // dataPeriod (ms)
            dataModel,                     // dataGenerator
            false,                          // polling mode
            status_notification            // get Mote status change notification
    );

    delay(100);
}

void loop() {

    char data=0;
    // continuously send the message in polling and
    //      read every down link message
    DustCbStatusE status = dustMote.readData();
    if (status == DataReceived) {
        SerialUSB.print("DataReceived:   ");
        SerialUSB.write(dataModel->fetchLastMessage(&len), len);
        SerialUSB.println();
    }

#ifdef ARDUINO_SAMD_SMARTEVERYTHING_FOX
    if (isButtonOnePressed()){
#else 
        if (SerialUSB.available()) {
            data = SerialUSB.read();
            if ('.'==data)
#endif
                dustMote.sendData(dataModel);
        }
    }
