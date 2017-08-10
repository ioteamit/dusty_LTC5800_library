/*

      by Mik (mik@ioteam.it)

   This example is in the public domain
       https://bitbucket.org/ioteamit/arduino-dust-library
*/

#include <Wire.h>
#include <Arduino.h>
#include <DustMote.h>
#include "dust\dataModel\StreamDataModel.h"

StreamDataModel *dataModel;
const uint8_t *dataStream;   // 523147314231 = R1G1B1
char led;
volatile uint8_t len=0;

//=========================== data generator ==================================

uint8_t* generateData(void) {
    
    return (uint8_t*) dataStream;
    
}



void ledLight(boolean _light)
{
    char light = HIGH;

    if (!_light)
    light = LOW;

    switch (led) {

        case 'G':
        case 'g':
        ledGreenLight(light);
        break;

        case 'R':
        case 'r':
        ledRedLight(light);
        break;

        case 'B':
        case 'b':
        ledBlueLight(light);
        break;
    }

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
    
    dataModel = new StreamDataModel(generateData);

    smeDMote.begin(
    60000,                          // srcPort
    (uint8_t*)ipv6Addr_manager,     // destAddr
    61030,                          // destPort
    10000,                          // dataPeriod (ms)
    dataModel,                      // dataGenerator
    true,                           // polling mode
    status_notification             // get Mote status change notification
    );
    
    delay(100);
    
    led = 0;
    // LED & User Button are already initialized by the SME core.
}

void loop() {
    
    // continuously send the message in polling and
    //      read every down link message
    smeDMote.readData();
    
    
    // Write the received message
    uint8_t len=0;
    if (dataModel->isMessageReceived()) {
        SerialPrint("Received Msg = ");
        dataStream = dataModel->fetchLastMessage(&len);
        
        for (char i=0; i<len; i++) {
            
            switch (dataStream[i]) {
                case '1':
                ledLight(true);
                break;

                case '0':
                ledLight(false);
                break;

                default:
                led = dataStream[i];
                break;

            }
        }
    }
}

