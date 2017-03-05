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

//#define DUST_DEBUG

#include <Wire.h>
#include <Arduino.h>
#include <SmeDustMote.h>
//#include <VL6180.h>


bool bounce = false;
uint32_t referenceTime;

char distance_sent =  255;
char distance_ready =  255;

StreamDataModel *dataModel;
uint8_t dataStream[1];

//=========================== data generator ==================================

uint8_t* generateData(void) {
  
    dataStream[0] = distance_ready;
   
    distance_sent = distance_ready;

    SerialPrint("Dust Sending: ");
    SerialPrintln(dataStream[0], DEC);

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
    	Wire.begin();

    	if (!smeProximity.begin()) {
        	while(1){
            	; // endless loop due to error on VL6180 initialization
        	}
    	}

#ifdef DUST_DEBUG                            
      while (!SerialUSB) {
        ;
      };
#endif  

      dataModel = new StreamDataModel(generateData, 1);
      smeDMote.begin(
              60000,                           // srcPort
              (uint8_t*)ipv6Addr_manager,      // destAddr
              61001,                           // destPort
              10000,                           // dataPeriod (ms)
              dataModel,                       // dataGenerator
              true,                            // polling mode
              status_notification              // get Mote status change notification
              );
              
      referenceTime = millis();
}

//volatile unsigned char data;
uint16_t counter = 0;
      
void loop() {

    smeDMote.readData();
    
    char distance = smeProximity.rangePollingRead();

    if (!(counter % 100)) {
        if (distance == 255) {
            SerialPrintln("Infinity");
        } else {
            SerialPrint(distance, DEC);
            SerialPrintln(" mm");
        }        
    } 
    
    if (abs(distance_sent - distance) > 10  && !bounce) {
        bounce = true;
        distance_ready = distance;
        smeDMote.sendData();   
        referenceTime = millis();    
    }  else {
        distance_ready = distance;
    }

    delay(10);              // wait for 10 ms


    if ((millis() - referenceTime) > 1000) {
        bounce = false;
        referenceTime = millis();
    }
    counter++;
}
