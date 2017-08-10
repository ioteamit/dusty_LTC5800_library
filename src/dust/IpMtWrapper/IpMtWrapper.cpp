/*
Copyright (c) 2014, Dust Networks.  All rights reserved.

Arduino library to connect to a SmartMesh IP mote and periodically send data.

This library is an Arduino "wrapper" around the generic SmartMesh C library.

This library will:
- Connect to the SmartMesh IP mote over its serial port.
- Have the SmartMesh IP mote connect to a SmartMesh IP network, open and bind a
UDP socket
- Periodically, invoke a data generation function and send the generated
payload to the specified IPv6 address and UDP port.

\license See attached DN_LICENSE.txt.
*/

#include "Arduino.h"
#include "IpMtWrapper.h"
#include "IpMtDefines.h"

//=========================== define ==========================================
#define CMD_PERIOD                1000      // number of ms between two commands being sent
#define SERIAL_RESPONSE_TIMEOUT    500      // max number of ms to wait for response

//=========================== prototypes ======================================

#ifdef __cplusplus
extern "C" {
	#endif

	void dn_ipmt_notif_cb(uint8_t cmdId, uint8_t subCmdId);
	void dn_ipmt_reply_cb(uint8_t cmdId);

	#ifdef __cplusplus
}
#endif

//=========================== variables =======================================

extern IpMtWrapper	ipmtwrapper;
bool				networkSet;  // used in case of changing of the networkId

//=========================== typedef =========================================

//=========================== public ==========================================

/**
\brief Constructor.
*/
IpMtWrapper::IpMtWrapper() {
	homologationRunning = false;
	networkSet = false;
}

/**
\brief Setting up the instance.
*/
void IpMtWrapper::begin(
uint16_t       srcPort,
uint8_t*       destAddr,
uint16_t       destPort,
TIME_T         dataPeriod,
IpMtDataModel     *dataToSend,
boolean        polling,
status_update  statusUpd_cb,
Uart           *serial,
uint16_t		networkId
) {
	uint8_t* bufAddr;
	uint8_t   bufLen=2;
	
	this->networkId = networkId;
	memset(&ipmt_app_vars, 0, sizeof(mt_app_vars_t));

	//setup common variable
	IpMgMtWrapper::begin(serial);

	// store params
	ipmt_app_vars.srcPort          = srcPort;
	memcpy(ipmt_app_vars.destAddr,destAddr,IPv6ADDR_LEN);
	ipmt_app_vars.destPort         = destPort;
	ipmt_app_vars.dataPeriod       = dataPeriod;
	app_vars.syncDataToSend        = dataToSend;
	ipmt_app_vars.polling          = polling;
	ipmt_app_vars.statusUpdCb      = statusUpd_cb;

	bufAddr = app_vars.syncDataToSend->getNotifyBuffer();
	bufLen = app_vars.syncDataToSend->getNotifyBufferLen();

	// initialize the ipmt module
	dn_ipmt_init(
	dn_ipmt_notif_cb,   // notifCb
	bufAddr,            // notifBuf
	bufLen,
	dn_ipmt_reply_cb    // replyCb
	);

	// print banner
	SerialPrint("IpMtWrapper Library, version ");
	SerialPrint(VER_MAJOR);
	SerialPrint(".");
	SerialPrint(VER_MINOR);
	SerialPrint(".");
	SerialPrint(VER_PATCH);
	SerialPrint(".");
	SerialPrint(VER_BUILD);
	SerialPrintln(" (c) Dust Networks, 2014.");
	SerialPrintln("");
}



void IpMtWrapper::start(void)
{
	//schedule first event
	//fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_getMoteStatus);
	getConfiguredNetworkId();	
}

void IpMtWrapper::startHomologation(radioTestE txType,
									uint16_t chanMask,
									int8_t power)
{
	this->radioTest = txType;
	this->channelTest = chanMask;
	this->powerTest = power;
	
	api_setRatioTestTx();
	//fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_setRatioTestTx);
}

DustCbStatusE IpMtWrapper::main() {
	TIME_T  currentTime;

	currentTime = parseMessage();

	if (app_vars.fsmDelay>0 && (currentTime-app_vars.fsmPreviousEvent>app_vars.fsmDelay)) {
		// cancel event
		app_vars.fsmDelay=0;

		// handle event (by calling the callback if present)
		if (ipmt_app_vars.fsmCb)
		(this->*ipmt_app_vars.fsmCb)();
	}

	return app_vars.cbStatus;
}

//=========================== private =========================================

//===== fsm
void IpMtWrapper::fsm_scheduleEvent(uint16_t delay, fsm_mt_timer_callback cb) {
	app_vars.fsmDelay    = delay;
	ipmt_app_vars.fsmCb       = cb;
}

void IpMtWrapper::fsm_cancelEvent() {
	app_vars.fsmDelay    = 0;
	ipmt_app_vars.fsmCb       = NULL;
}

void IpMtWrapper::fsm_setCallback(fsm_mt_reply_callback cb) {
	ipmt_app_vars.replyCb     = cb;
}

void IpMtWrapper::api_response_timeout(void) {

	// log
	SerialPrint("ERROR:    timeout!");

	// issue cancel command
	dn_ipmt_cancelTx();

	// schedule first event
	if (!homologationRunning)
		fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_getMoteStatus);
	else
		fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_setRatioTestTx);
}

//===== getMoteStatus

void IpMtWrapper::api_getMoteStatus(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getMoteStatus... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_getMoteStatus_reply);

	// issue function
	err = dn_ipmt_getParameter_moteStatus(
	(dn_ipmt_getParameter_moteStatus_rpt*)(app_vars.replyBuf)
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_getMoteStatus_reply() {
	dn_ipmt_getParameter_moteStatus_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_getMoteStatus_reply");

	reply = (dn_ipmt_getParameter_moteStatus_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     state=");
	printState(reply->state, false);

	if (ipmt_app_vars.statusUpdCb)
	ipmt_app_vars.statusUpdCb(reply->state);

	switch (reply->state) {
		case MOTE_STATE_IDLE:
		fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_openSocket);
		break;
		case MOTE_STATE_OPERATIONAL:
		// the API currently does not allow to find out what the open sockets are
		ipmt_app_vars.socketId = DEFAULT_SOCKETID;

		// mote already operational, send data
		fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_getServiceInfo);
		break;
		default:
		fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_getMoteStatus);
		break;
	}
}

//===== openSocket
void IpMtWrapper::api_openSocket(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_openSocket... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_openSocket_reply);

	// issue function
	err = dn_ipmt_openSocket(
	0,                                              // protocol
	(dn_ipmt_openSocket_rpt*)(app_vars.replyBuf)    // reply
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_openSocket_reply() {
	dn_ipmt_openSocket_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_openSocket_reply");

	reply = (dn_ipmt_openSocket_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     socketId=");
	SerialPrintln(reply->socketId);

	// store the socketID
	ipmt_app_vars.socketId = reply->socketId;

	// schedule next event
	fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_bindSocket);
}

//===== bindSocket

void IpMtWrapper::api_bindSocket(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_bindSocket... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_bindSocket_reply);

	// issue function
	err = dn_ipmt_bindSocket(
	ipmt_app_vars.socketId,                              // socketId
	ipmt_app_vars.srcPort,                               // port
	(dn_ipmt_bindSocket_rpt*)(app_vars.replyBuf)    // reply
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_bindSocket_reply() {
	dn_ipmt_bindSocket_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_bindSocket_reply");

	reply = (dn_ipmt_bindSocket_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);

	// schedule next event
	fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_join);
}

//===== join

void IpMtWrapper::api_join(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_join... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_join_reply);

	// issue function
	err = dn_ipmt_join(
	(dn_ipmt_join_rpt*)(app_vars.replyBuf)     // reply
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_join_reply() {
	dn_ipmt_join_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_join_reply");

	reply = (dn_ipmt_join_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
}

//===== getServiceInfo

void IpMtWrapper::api_getServiceInfo(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getServiceInfo... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_getServiceInfo_reply);

	// issue function
	err = dn_ipmt_getServiceInfo(
	0xfffe,                                              // destAddr (0xfffe==manager)
	SERVICE_TYPE_BW,                                     // type
	(dn_ipmt_getServiceInfo_rpt*)(app_vars.replyBuf)     // reply
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_getServiceInfo_reply() {
	dn_ipmt_getServiceInfo_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_getServiceInfo_reply");

	reply = (dn_ipmt_getServiceInfo_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);

	SerialPrint("INFO:     value=");
	SerialPrintln(reply->value);

	// schedule next event
	if (reply->RC!=0 || reply->value>ipmt_app_vars.dataPeriod) {
		fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_requestService);
		} else {
		if (ipmt_app_vars.polling){
			fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_sendTo);
			} else {
			fsm_cancelEvent();//fsm_scheduleEvent(CMD_PERIOD, &IpMtWrapper::api_DummyStatus);
		}
	}
}


//===== getNetworkId

void IpMtWrapper::api_configureNetworkId(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getNetworkId... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_configureNetworkId_reply);

	// issue function
	err = dn_ipmt_getParameter_networkId(
	(dn_ipmt_getParameter_networkId_rpt*)(app_vars.replyBuf)
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_configureNetworkId_reply() {
	dn_ipmt_getParameter_networkId_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_getNetworkId_reply");

	reply = (dn_ipmt_getParameter_networkId_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     networkId = 0x");
	SerialPrintln(reply->networkId, HEX);
	if (reply->networkId == networkId) {
		fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_getMoteStatus);
	} else {
		networkSet = true;
		fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_setNetworkId);
	}
}

//===== setNetworkId

void IpMtWrapper::api_setNetworkId(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_setNetworkId... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_setNetworkId_reply);

	// issue function
	err = dn_ipmt_setParameter_networkId(
	this->networkId,
	(dn_ipmt_setParameter_networkId_rpt*)(app_vars.replyBuf)
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_setNetworkId_reply() {
	dn_ipmt_setParameter_networkId_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();
	networkSet = false;

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_setNetworkId_reply");

	reply = (dn_ipmt_setParameter_networkId_rpt*)app_vars.replyBuf;

	
	if (reply->RC == RC_OK) {
		fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_getMoteStatus);
	}
}

//===== getMac

void IpMtWrapper::api_getMac(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getNetworkId... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_getMac_reply);

	// issue function
	err = dn_ipmt_getParameter_macAddress(
	(dn_ipmt_getParameter_macAddress_rpt*)(app_vars.replyBuf)
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_getMac_reply() {
	dn_ipmt_getParameter_macAddress_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_getNetworkId_reply");

	reply = (dn_ipmt_getParameter_macAddress_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     Mac = ");
	SerialPrint(reply->macAddress[0], HEX);
	SerialPrint("-");
	SerialPrint(reply->macAddress[1], HEX);
	SerialPrint("-");
	SerialPrint(reply->macAddress[2], HEX);
	SerialPrint("-");
	SerialPrint(reply->macAddress[3], HEX);
	SerialPrint("-");
	SerialPrint(reply->macAddress[4], HEX);
	SerialPrint("-");
	SerialPrint(reply->macAddress[5], HEX);
	SerialPrint("-");
	SerialPrint(reply->macAddress[6], HEX);
	SerialPrint("-");
	SerialPrint(reply->macAddress[7], HEX);

	// restart the normal schedule
	fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_getMoteStatus);		
	
	setMgStatus(Completed);
}

//===== requestService

void IpMtWrapper::api_requestService(void) {
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_requestService... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_requestService_reply);

	// issue function
	err = dn_ipmt_requestService(
	0xfffe,                                              // destAddr (0xfffe==manager)
	SERVICE_TYPE_BW,                                     // serviceType
	ipmt_app_vars.dataPeriod,                                 // value
	(dn_ipmt_requestService_rpt*)(app_vars.replyBuf)     // reply
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_requestService_reply() {
	dn_ipmt_requestService_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_requestService_reply");

	reply = (dn_ipmt_requestService_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
}

//===== sendTo

void IpMtWrapper::api_sendTo() {
	dn_err_t err;
	uint8_t  *payload;
	uint8_t  dataLen;

	// set The internal Status
	app_vars.cbStatus = DataSent;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_sendTo... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_sendTo_reply);

	// create payload
	if (app_vars.asyncDataToSend == NULL) {
		payload = app_vars.syncDataToSend->getPayload();
		dataLen = app_vars.syncDataToSend->getDatasize();
		} else {
		payload = app_vars.asyncDataToSend->getPayload();
		dataLen = app_vars.asyncDataToSend->getDatasize();
	}

	//dn_write_uint16_t(payload, dataVal);

	// issue function
	err = dn_ipmt_sendTo(
	ipmt_app_vars.socketId,                              // socketId
	ipmt_app_vars.destAddr,                              // destIP
	ipmt_app_vars.destPort,                              // destPort
	SERVICE_TYPE_BW,                                     // serviceType
	0,                                                   // priority
	0xffff,                                              // packetId
	payload,                                             // payload
	dataLen,                                             // payloadLen
	(dn_ipmt_sendTo_rpt*)(app_vars.replyBuf)             // reply
	);

	// log
	SerialPrintln(err);

	SerialPrint("INFO:     sending value: ");
	// SerialPrintln(dataVal);

	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
}

void IpMtWrapper::api_sendTo_reply() {
	dn_ipmt_sendTo_rpt* reply;

	// cancel timeout and status
	app_vars.cbStatus = Idle;
	fsm_cancelEvent();

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrintln("INFO:     api_sendTo_reply");

	reply = (dn_ipmt_sendTo_rpt*)app_vars.replyBuf;

	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);

	// schedule next transmission
	if (ipmt_app_vars.polling){
		fsm_scheduleEvent(ipmt_app_vars.dataPeriod, &IpMtWrapper::api_sendTo);
		} else {
		fsm_cancelEvent();//fsm_scheduleEvent(ipmt_app_vars.dataPeriod, &IpMtWrapper::api_DummyStatus);
	}

}


//===== Dummy when no polling required

// void IpMtWrapper::api_DummyStatus(void) {
//
//     // record time
//     app_vars.fsmPreviousEvent = millis();
//
//     // log
//     SerialPrintln("");
//     SerialPrint("INFO:     api_DummyStatus... returns ");
//
//     // arm callback
//     fsm_scheduleEvent(ipmt_app_vars.dataPeriod, &IpMtWrapper::api_DummyStatus);
// }

//=========================== helpers =========================================

void IpMtWrapper::addAsyncronousSend(DataModel * sendData)
{
	app_vars.asyncDataToSend = sendData;
}

void IpMtWrapper::deleteAsyncronousSend()
{
	app_vars.asyncDataToSend = NULL;
}

void IpMtWrapper::executeStatusUpdCb(uint8_t    state)
{
	if (ipmt_app_vars.statusUpdCb)
	ipmt_app_vars.statusUpdCb(state);
}

void IpMtWrapper::executeReplyCb()
{
	(ipmtwrapper.*ipmt_app_vars.replyCb)();
}


void IpMtWrapper::getConfiguredNetworkId()
{
	fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_configureNetworkId);
}


void IpMtWrapper::getMac()
{
	fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_getMac);
}

//=========================== callback functions for ipmt =====================

void dn_ipmt_notif_cb(uint8_t cmdId, uint8_t subCmdId) {

	dn_ipmt_events_nt* dn_ipmt_events_notif;
	if (networkSet == true)
	return;

	switch (cmdId) {
		case CMDID_EVENTS:

		SerialPrintln("");
		SerialPrintln("INFO:     notif CMDID_EVENTS");

		dn_ipmt_events_notif = (dn_ipmt_events_nt*)app_vars.syncDataToSend->getNotifyBuffer();


		SerialPrint("INFO:     state=");
		printState(dn_ipmt_events_notif->state, false);

		ipmtwrapper.executeStatusUpdCb(dn_ipmt_events_notif->state);

		switch (dn_ipmt_events_notif->state) {
			case MOTE_STATE_IDLE:
			ipmtwrapper.fsm_scheduleEvent(CMD_PERIOD,&IpMtWrapper::api_getMoteStatus);
			ipmtwrapper.executeStatusUpdCb(dn_ipmt_events_notif->state);
			break;

			case MOTE_STATE_OPERATIONAL:
			ipmtwrapper.fsm_scheduleEvent(CMD_PERIOD,&IpMtWrapper::api_getServiceInfo);
			break;
			default:
			// nothing to do
			break;
		}
		break;

		case CMDID_RECEIVE:
		// receive msg from Manager
		ipmtwrapper.setMgStatus(DataReceived);
		break;

		default:
		// nothing to do
		break;
	}
}

void IpMtWrapper::api_setRatioTestTx()
{
	dn_err_t err;

	// record time
	app_vars.fsmPreviousEvent = millis();

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_setRatioTestTx... returns ");

	// arm callback
	fsm_setCallback(&IpMtWrapper::api_setRatioTestTx_reply);

	// issue function
 	err = dn_ipmt_testRadioTxExt(
	(uint8_t)this->radioTest,
 	this->channelTest,  // chanMask  set by the user
	0,					// repeatCnt not used in Homologation
	this->powerTest,    // txPower set by the user
	0x5A, // seqSize not used 
	1,1,
	2,2,
	3,3,
	4,4,
	5,5,
	6,6,
	7,7,
	8,8, // packet & delay not used
	9,9,
	0xA,0xA,
	0xA5, // stationId not used
 	(dn_ipmt_testRadioTxExt_rpt*)(app_vars.replyBuf)
 	);

	// log
	SerialPrintln(err);
	
	// schedule timeout event
	fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, &IpMtWrapper::api_response_timeout);
	
	homologationResult = false;	
	homologationRunning = true;
}
	
volatile uint8_t    debug;
void IpMtWrapper::api_setRatioTestTx_reply()
{
	dn_ipmt_testRadioTxExt_rpt* reply;

	// cancel timeout
	fsm_cancelEvent();
	networkSet = false;

	// record time
	app_vars.fsmPreviousEvent = millis();

	SerialPrint("INFO:     api_setRatioTestTx_reply = ");

	reply = (dn_ipmt_testRadioTxExt_rpt*)app_vars.replyBuf;

	debug = reply->RC;
	if (reply->RC == RC_OK) {
		homologationResult = true;
		SerialPrintln("OK");
		// stop here do not schedule anything
		/*fsm_scheduleEvent(2*CMD_PERIOD, &IpMtWrapper::api_getMoteStatus);*/
	} else {
		SerialPrint("KO (0x");
		SerialPrint(reply->RC, HEX);
		SerialPrintln(")");
	}
	
	
	homologationRunning = false;
}


extern "C" void dn_ipmt_reply_cb(uint8_t cmdId) {
	SerialPrint("\n  on dn_ipmt_reply_cb received cmdId=");
	SerialPrintln(cmdId, HEX);
	ipmtwrapper.executeReplyCb();
}
