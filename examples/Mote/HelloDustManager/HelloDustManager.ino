/*
 *      SmeIoT Library - HelloDustManager
 *
 *   This example, togheter with the HelloDustMote in the Manager examples, show the
 *   comunication between the Motes and the Manager.
 *
 *
 *  3 are the main classes that made this possible:
 *  1) the object dustManager that is the Manager of the network
 *  2) The object dustMote that is the Mote of the network
 *  3) The class IpMgDataModel and IpMtDataModel that are the classes used to send/receive data.
 *  
 *  Describe here better the mote class and its datamodel
 *  
 *  dustMote:
 *    begin = srcPort = the port for incoming message
 *            dstPort = the port for the outgoing message
 *            dataPeriod = the millisecond to wait for the call to the periodic callback that prepare the message to send
 *            dataToSend = the pointer to the data model prepared by the callback
 *            polling    = true if we want the automatic periodic message
 *            statusUpd_cb = the callback that receive the status event of the Mote 
 *  
 *    readData = this is the main method used by the manager to move the internal FSM, 
 *                 it shall be always called because return the latest received DataModel 
 *                 or the complete of the interanl comand.
 *                 return status described in IpMgMtWrapper.h file
 *  
 *    getLastCommand = return the information of the last command (need to be cast the the command structure defined in dn_ipmt.h) 
 *                 
 *    sendData = Send the information contains in the DataModel.
 *                 
 *                 
 *  IpMtDataModel:           
 *    constructor  = set the callBack that prepare the datamodel for the polling mode
 *    fetchLastMessage =  Return the pointer to the latest message received and its length.
 *  
 *  created 08 07 2016
 *     by Mik (mik@ioteam.it)
 *
 *  This example is in the public domain
 *      https://github.com/ioteamit/dusty_LTC5800_library
 */

#include <Arduino.h>
#include <DustMote.h>

#define MIN_DIST 11
#define MAX_DIST 30

IpMtDataModel *dataModel;
char *hello = "Hello Dust Manager";
uint8_t dataStream[50];
uint8_t len = 0;
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
      msec = 500;            // wait for 300 ms
    }

    last_status = status;
    SerialUSB.print("New State: ");
    SerialUSB.println(status, DEC);
  }
}

//=========================== "main" ==========================================

void setup() {
  SerialUSB.begin(115200);
  while (!SerialUSB) {
    ;
  }
  SerialUSB.println("start Mote...\n");

  dataModel = new IpMtDataModel(generateData);

  dustMote.begin(
    60000,                         // srcPort
    61020,                         // destPort
    500,                           // dataPeriod (ms)
    dataModel,                     // dataGenerator
    false,                          // polling mode
    status_notification            // get Mote status change notification
  );

  delay(100);
}

void loop() {
    dn_ipmt_getParameter_time_rpt* timeInfo;

  char data = 0;
  // continuously send the message in polling and
  //      read every down link message
  DustCbStatusE status = dustMote.readData();
  switch (status) {
    case DataReceived:
    SerialUSB.print("DataReceived:   ");
    SerialUSB.write(dataModel->fetchLastMessage(&len), len);
    SerialUSB.println();
    break;

    case Completed:
    SerialUSB.println ("Comman Executed cast the getLastCommand to the correct comand struct");
    timeInfo = (dn_ipmt_getParameter_time_rpt*)dustMote.getLastCommand();
    break;
  }


  if (SerialUSB.available()) {
    data = SerialUSB.read();
    if ('.' == data)
      dustMote.sendData(dataModel);

    if (',' == data)
      dustMote.retrieveTime();
  }
}
