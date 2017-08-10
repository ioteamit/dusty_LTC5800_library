/*
      by Mik (mik@ioteam.it)

   This example is in the public domain
       https://bitbucket.org/ioteamit/arduino-dust-library
*/

#include <Wire.h>
#include <Arduino.h>
#include <DustMote.h>


bool bounce = false;
bool ledOneStatus = false;
bool ledTwoStatus = false;

uint32_t referenceTime;

StreamDataModel *dataModel;
uint8_t dataStream[2];

//=========================== data generator ==================================

uint8_t* generateData(void) {
    
    dataStream[0] = ledOneStatus;
    dataStream[1] = ledTwoStatus;
    
    SerialPrint("Dust Sending: ");
    SerialPrint(dataStream[0], DEC);
    SerialPrint("-");
    
    SerialPrintln(dataStream[1], DEC);

    ledBlueLight(HIGH);   // turn the LED On by making the voltage HIGH
    delay(30);            // wait for 30 ms
    ledBlueLight(LOW);    // turn the LED off by making the voltage LOW
    
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
    

    #ifdef DUST_DEBUG
    while (!SerialUSB) {
        ;
    };
    #endif
    
    dataModel = new StreamDataModel(generateData, 2);

    smeDMote.begin(60000,                          // srcPort
    (uint8_t*)ipv6Addr_manager,     // destAddr
    61002,                          // destPort
    10000,                          // dataPeriod (ms)
    dataModel,                      // dataGenerator
    true,                           // polling mode
    status_notification             // get Mote status change notification
    );
    
    delay(100);
    referenceTime = millis();
    
    ledYellowOneLight(ledOneStatus);
    ledYellowTwoLight(ledTwoStatus);
    // RGB LED & User Buttons are already initialized by the SME core.
}

void loop() {
    
    delay(10);              // wait for 10 ms

    // continuously send the message in polling and
    // on button status change
    smeDMote.readData();
    
    if (isButtonOnePressed() && !bounce) {
        bounce = true;

        ledOneStatus = !ledOneStatus;
        SerialPrint("Yellow Led ONE ->  ");
        SerialPrintln(ledOneStatus ? "ON" : "OFF");
        ledYellowOneLight(ledOneStatus);
        smeDMote.sendData();
        referenceTime = millis();
        
        } else if (isButtonTwoPressed() && !bounce) {
        bounce = true;
        
        ledTwoStatus = !ledTwoStatus;
        SerialPrint("Yellow Led TWO ->  ");
        SerialPrintln(ledTwoStatus ? "ON" : "OFF");
        ledYellowTwoLight(ledTwoStatus);
        smeDMote.sendData();
        referenceTime = millis();
    }
    
    if ((millis() - referenceTime) > 1000) {
        bounce = false;
        referenceTime = millis();
    }
}

