/*
      SmeIoT Library - DustySensor.ino

   This example work togher with the DustySensor designed by IOTEAM (www.ioteam.it)

  created 08 07 2017
      by Mik (mik@ioteam.it)

   This example is in the public domain
 *      https://github.com/ioteamit/dusty_LTC5800_library

*/
#include <Arduino.h>
#include <DustManager.h>

#pragma 1
typedef struct {
  uint16_t xDust;
  uint16_t yDust;
  uint16_t tempDust;
} GyroDust_t;

typedef struct {
  uint16_t xDust;
  uint16_t yDust;
  uint16_t zDust;
  uint16_t tempDust;
} AccelerometerDust_t;

typedef struct {
  uint16_t tempDust;
} TempDust_t;

typedef struct {
  GyroDust_t gyroMsg;
  AccelerometerDust_t accelerometerMsg;
  TempDust_t tempMsg;
  boolean    fromButton;
}  DustySensorMsg_t;

uint8_t moteN = 0;
IpMgDataModel *motes[10];

uint16_t srcPort = 61020;
uint16_t dstPort = 60000;
unsigned long timeOut;
#define Sec3 3000

static void printByte(const uint8_t* payload, uint8_t length) {
  uint8_t i;

  SerialUSB.print(" ");
  for (i = 0; i < length; i++) {
    SerialUSB.print(payload[i], HEX);
    if (i < length - 1) {
      SerialUSB.print("-");
    }
  }
}

void eventReceiver(uint8_t eventId, uint8_t mac[8]) {
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
  SerialUSB.println("Start Manager...\n");
  dustManager.begin(true, eventReceiver);

}

uint16_t debug2;
void loop() {
  dn_ipmg_getMoteConfig_rpt* reply;
  uint8_t len;
  const uint8_t* dataRcv;
  char data = 0;
  IpMgDataModel *mote;
  DustySensorMsg_t msgStr;

  DustCbStatusE msgStatus = dustManager.readData();
  switch (msgStatus) {
    case DataReceived:
      SerialUSB.print("\n\rDataReceived:   ");

      mote = dustManager.getLastMessage();
      printByte((uint8_t*)mote->getDestinationMac(), 8);
      dataRcv = mote->fetchLastMessage(&len);
      SerialUSB.print(" = [");
      SerialUSB.print(len, DEC);
      SerialUSB.print("] ");
      printByte(dataRcv, len);


memcpy(&msgStr, dataRcv, sizeof(DustySensorMsg_t));
      SerialUSB.println("\n\r****Gyro");
      SerialUSB.print("Gyro x = ");
	  debug2 = msgStr.gyroMsg.xDust;
      SerialUSB.println(debug2, DEC);
      SerialUSB.print("Gyro y = ");
      SerialUSB.println(msgStr.gyroMsg.yDust, DEC);
      SerialUSB.print("Gyro temp = ");
      SerialUSB.println(msgStr.gyroMsg.tempDust, DEC);
      SerialUSB.println("Gyro****");
      SerialUSB.println();

      SerialUSB.println("\n\r****Accelerometer");
      SerialUSB.print("Accelerometer x = ");
      SerialUSB.println(msgStr.accelerometerMsg.xDust, DEC);
      SerialUSB.print("Accelerometer y = ");
      SerialUSB.println(msgStr.accelerometerMsg.yDust, DEC);
      SerialUSB.print("Accelerometer z = ");
      SerialUSB.println(msgStr.accelerometerMsg.zDust, DEC);
      SerialUSB.print("Accelerometer temp = ");
      SerialUSB.println(msgStr.accelerometerMsg.tempDust, DEC);
      SerialUSB.println("Accelerometer****");
      SerialUSB.println();

      SerialUSB.println("\n\r****Ambiental");
      SerialUSB.println(msgStr.tempMsg.tempDust, DEC);
      SerialUSB.println("Ambiental****");
      SerialUSB.println();


      SerialUSB.println("\n\r****Button");
      if (msgStr.fromButton)
        SerialUSB.println("Button is pressed");
      else
        SerialUSB.println("Button is NOT pressed");
      SerialUSB.println("Button****");
      SerialUSB.println();
      break;
  }


  if (SerialUSB.available()) {
    data = SerialUSB.read();
  }



  if ((millis() - timeOut) == Sec3) {
    timeOut = 0;
  }
}
