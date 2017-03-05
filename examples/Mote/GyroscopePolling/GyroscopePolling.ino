/*
Copyright (c) 2014, Dust Networks.  All rights reserved.

Arduino sketch which connects to a SmartMesh IP mote and periodically sends a
2-byte value to the manager. You can use the SensorDataReceiver application of
the SmartMesh SDK to see that data arrive.

Note: before you can run this sketch, you need import the sm_clib
library. To do so:
- double click on this file to open the Arduino IDE
- In Sketch > Import Library... > Add Library..., select navigate to the
sm_clib folder and click open.

Note: before running this sketch:
- configure your SmartMesh IP mote to run in slave mode
- on your SmartMesh IP mote, configure the network ID you the mote to connect
to.
- remove the battery from your SmartMesh IP mote, it will be powered by the
Arduino Due
- connect your Arduino Due board to your DC9003 SmartMesh IP mote as detailed
in the documentation.
- make sure the power switch of the DC9003 SmartMesh IP mote is in the ON
position.

To run this sketch, connect your Arduino Due board to your computer, and Select
File > Upload.

\license See attached DN_LICENSE.txt.
 */

#include <Wire.h>
#include <Arduino.h>
#include <SmeDustMote.h>

#define MIN_DIST 11
#define MAX_DIST 30

char distance =  255;
uint16_t distance_ready =  255;
bool data_ready = false;
uint16_t counter = 0;
IpMtDataModel *dataModel;
uint16_t dataStream[3];
//=========================== data generator ==================================

uint8_t* generateData(DataModel *model) {
    data_ready = false;

    dataStream[0] = 11;//smeGyroscope.readX();
    dataStream[1] = 12;//smeGyroscope.readY();
    dataStream[2] = 13;//smeGyroscope.readZ();
    model->setDatasize(sizeof(dataStream));

    SerialPrint(dataStream[0], DEC);
    SerialPrint("-");

    SerialPrint(dataStream[1], DEC);
    SerialPrint("-");

    SerialPrint(dataStream[2], DEC);

    ledBlueLight(LOW);   // turn the LED off by making the voltage LOW

    return (uint8_t*) dataStream;

}


//===========================  CONNECTION STATUS ================================
typedef void (*led_cb_t)(uint32_t light);

void status_notification (uint8_t status) {
    static uint8_t last_status = 0; // UNKNOWN STATE;
    led_cb_t       led_cb = ledRedLight;
    uint16_t       msec = 10;

    if (status != last_status) {
        if (MOTE_STATE_OPERATIONAL == status) {
            led_cb = ledGreenLight;
            msec = 500;            // wait for 300 ms
        }

        led_cb(HIGH);
        delay(msec);
        led_cb(LOW);

        last_status = status;
        SerialPrint("New State: ");
        SerialPrintln(status, DEC);
    }
}

//=========================== "main" ==========================================

void setup() {
    Wire.begin();

    while (!SerialUSB){
        ;
    }    
    dataModel = new IpMtDataModel(generateData);

    smeDMote.begin(
            60000,                         // srcPort
            (uint8_t*)ipv6Addr_manager,    // destAddr
            61020,                         // destPort
            500,                           // dataPeriod (ms)
            dataModel,                     // dataGenerator
            true,                          // polling mode
            status_notification            // get Mote status change notification
    );

    delay(100);
}

void loop() {

    // continuously send the message in polling and
    //      read every down link message
    DustCbStatusE status = smeDMote.readData();
    if (status == DataSent) {
        SerialUSB.println("SENT !!");
    }

}

