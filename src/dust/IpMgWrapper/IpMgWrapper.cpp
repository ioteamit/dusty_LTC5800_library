/*
Copyright (c) 2014, Dust Networks.  All rights reserved.

Arduino library to connect to a SmartMesh IP manager.

This library is an Arduino "wrapper" around the generic SmartMesh C library.

This library will:
- Connect to the SmartMesh IP manager.
- Subscribe to data notifications.
- Get the MAC address of all nodes in the network.
- Send an OAP command to blink each node's LED in a round-robin fashion.

\license See attached DN_LICENSE.txt.
*/

#include "Arduino.h"
#include "IpMgWrapper.h"
#include "sm_clib\dn_serial_mg.h"
#include "IpMgDefines.h"
#include "..\..\DustManager.h"

//=========================== define ==========================================
#define CMD_PERIOD                0          // number of ms between two commands being sent
#define INTER_FRAME_PERIOD        1          // min number of ms between two TX frames over serial port
#define SERIAL_RESPONSE_TIMEOUT   100        // max number of ms to wait for serial response
#define BACKOFF_AFTER_TIMEOUT     1000       // number of ms to back off after a timeout occurs
#define OAP_RESPONSE_TIMEOUT      10000      // max number of ms to wait for OAP response


//=========================== prototypes ======================================

#ifdef __cplusplus
extern "C" {
	#endif

	void dn_ipmg_notif_cb(uint8_t cmdId, uint8_t subCmdId);
	void dn_ipmg_reply_cb(uint8_t cmdId);
	void dn_ipmg_status_cb(uint8_t newStatus);

	#ifdef __cplusplus
}
#endif


//=========================== variables =======================================

extern IpMgWrapper ipmgwrapper;

//=========================== typedef =========================================

//=========================== public ==========================================

/**
\brief Constructor.
*/
IpMgWrapper::IpMgWrapper() {
	app_vars.cbStatus = Idle;
}

/**
\brief Setting up the instance.
*/
bool IpMgWrapper::begin(Uart    *serial,
bool    eventNotification) {

	uint8_t* bufAddr;
	uint8_t  bufLen=2;

	// store if the subscribe need the event Notification
	this->eventNotification = eventNotification;
	// reset local variables
	memset(&ipmg_app_vars,    0, sizeof(mg_app_vars_t));

	//setup common variable
	IpMgMtWrapper::begin(serial);

	// initialize internal data Model for the receiving message
	app_vars.syncDataToSend        = new IpMgDataModel();
	bufAddr = app_vars.syncDataToSend->getNotifyBuffer();
	bufLen = app_vars.syncDataToSend->getNotifyBufferLen();

	// store params
	// initialize the ipmg module
	dn_ipmg_init(
	dn_ipmg_notif_cb,   // notifCb
	bufAddr,            // notifBuf
	bufLen,             // notifBufLen
	dn_ipmg_reply_cb,                // replyCb
	dn_ipmg_status_cb                // statusCb
	);

	// print banner
	SerialPrint("IpMgWrapper Library, version ");
	SerialPrint(VER_MAJOR);
	SerialPrint(".");
	SerialPrint(VER_MINOR);
	SerialPrint(".");
	SerialPrint(VER_PATCH);
	SerialPrint(".");
	SerialPrint(VER_BUILD);
	SerialPrintln(" (c) Dust Networks, 2014.");
	SerialPrintln("");

	api_initiateConnect();

	uint32_t  currentTime, startTime = millis();
	bool timeoutExpired=false;
	DustCbStatusE exit= Idle;

	// internally remain in this status till all the begin commands
	// required for the initialization has been processed
	// or Exit in Timeout
	do{
		exit = main();
		currentTime = millis();
		if ((currentTime-startTime) > TIMEOUT_5Sec) {
			timeoutExpired = true;
		}
	} while((exit != Completed) && !timeoutExpired);
	
	return !timeoutExpired;
}

DustCbStatusE IpMgWrapper::main() {
	TIME_T  currentTime;

	currentTime = parseMessage();

	if (ipmg_app_vars.fsmArmed==TRUE && (currentTime-app_vars.fsmPreviousEvent>app_vars.fsmDelay)) {
		// cancel event
		ipmg_app_vars.fsmArmed=FALSE;

		// handle event (by calling the callback if present)
		if (ipmg_app_vars.fsmCb != NULL) {
			(this->*ipmg_app_vars.fsmCb)();
		}
	}

	return app_vars.cbStatus;

}

//=========================== private =========================================

//===== fsm

void IpMgWrapper::fsm_scheduleEvent(uint16_t delay, fsm_mg_timer_callback cb) {
	ipmg_app_vars.fsmArmed    = TRUE;
	app_vars.fsmPreviousEvent = millis();
	app_vars.fsmDelay         = delay;
	ipmg_app_vars.fsmCb       = cb;
}

void IpMgWrapper::fsm_cancelEvent() {
	app_vars.fsmDelay    = 0;
	ipmg_app_vars.fsmCb       = NULL;
	app_vars.cbStatus = Completed;
}

void IpMgWrapper::fsm_setCallback(fsm_mg_reply_callback cb) {
	ipmg_app_vars.replyCb     = cb;
}

void IpMgWrapper::api_response_timeout(void) {

	// log
	SerialPrint("ERROR:    serial timeout!");

	// issue cancel command
	dn_ipmg_cancelTx();

	// schedule first event
	fsm_scheduleEvent(
	BACKOFF_AFTER_TIMEOUT,
	&IpMgWrapper::api_initiateConnect
	);
}


void IpMgWrapper::resetCurrentPathId(char mac[8])
{
	memcpy(ipmg_app_vars.currentMac, mac, 8);
	ipmg_app_vars.currentPathId = 0;
}

void IpMgWrapper::oap_response_timeout(void) {

	// log
	SerialPrint("\nERROR:    OAP timeout!\n");

	// schedule first event
	fsm_scheduleEvent(
	BACKOFF_AFTER_TIMEOUT,
	&IpMgWrapper::api_getNextMoteConfig
	);
}

//===== api_initiateConnect

void IpMgWrapper::api_initiateConnect(void) {
	dn_err_t err;

	// log
	SerialPrintln("");
	SerialPrint("INFO:     dn_ipmg_initiateConnect... returns ");

	// schedule
	err = dn_ipmg_initiateConnect();

	// log
	SerialPrintln(err);
}

//===== api_subscribe

void IpMgWrapper::api_subscribe(void) {
	dn_err_t err;

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_subscribe... returns ");


	// arm callback
	fsm_setCallback(&IpMgWrapper::api_subscribe_reply);

	// issue function
	uint32_t notification = SUBSC_FILTER_DATA;
	if (this->eventNotification)
	notification |= SUBSC_FILTER_EVENT;
	err = dn_ipmg_subscribe(
	notification,                              // filter
	0x00000000,                                     // unackFilter
	(dn_ipmg_subscribe_rpt*)(app_vars.replyBuf)     // reply
	);

	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(
	SERIAL_RESPONSE_TIMEOUT,
	&IpMgWrapper::api_response_timeout
	);
}

void IpMgWrapper::api_subscribe_reply() {
	dn_ipmg_subscribe_rpt* reply;

	// log
	SerialPrintln("INFO:     api_subscribe_reply");

	// cast reply
	reply = (dn_ipmg_subscribe_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);

	// remove the Timeout Event
	fsm_cancelEvent();
}

//===== api_getNextMoteConfig

void IpMgWrapper::api_getNextMoteConfig(void) {
	dn_err_t err;
	app_vars.cbStatus = Idle;

	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getNextMoteConfig MAC=");
	printByteArray(ipmg_app_vars.currentMac,sizeof(ipmg_app_vars.currentMac));
	SerialPrint("... returns ");

	// arm callback
	fsm_setCallback(&IpMgWrapper::api_getNextMoteConfig_reply);

	// issue function
	err = dn_ipmg_getMoteConfig(
	ipmg_app_vars.currentMac,                            // macAddress
	TRUE,                                           // next
	(dn_ipmg_getMoteConfig_rpt*)(app_vars.replyBuf) // reply
	);

	app_vars.cbStatus = CommandSent;
	// log
	SerialPrintln(err);

	// schedule timeout event
	fsm_scheduleEvent(
	SERIAL_RESPONSE_TIMEOUT,
	&IpMgWrapper::api_response_timeout
	);
}


void IpMgWrapper::api_getNextMoteConfig_reply() {
	dn_ipmg_getMoteConfig_rpt* reply;

	// log
	SerialPrintln("INFO:     api_getNextMoteConfig_reply");

	// cast reply
	reply = (dn_ipmg_getMoteConfig_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
	SerialPrint("INFO:     MAC=");
	printByteArray(reply->macAddress, sizeof(reply->macAddress));
	SerialPrint("\n");
	SerialPrint("INFO:     state=");
	printState(reply->state, true);
	SerialPrint("INFO:     isAP=");
	SerialPrintln(reply->isAP);
	if (reply->RC  == RC_OK) {
		app_vars.cbStatus = Working;

		// remember current MAC
		memcpy(ipmg_app_vars.currentMac,reply->macAddress,sizeof(reply->macAddress));

		// schedule next event
		dustManager.getWrapper()->fsm_scheduleEvent(
		CMD_PERIOD,
		&IpMgWrapper::api_getNextMoteConfig
		);

		} else if (reply->RC == RC_END_OF_LIST){
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}

//===== api_getNextMoteConfig

void IpMgWrapper::api_getNextPathInfo(void) {
	dn_err_t err;

	app_vars.cbStatus = Idle;
	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getNextPathInfo MAC=");
	printByteArray(ipmg_app_vars.currentMac,sizeof(ipmg_app_vars.currentMac));
	SerialPrint("... returns ");

	// issue function
	err = dn_ipmg_getNextPathInfo(
	ipmg_app_vars.currentMac,       // macAddress
	0,                              // up and Downstream
	ipmg_app_vars.currentPathId,    // next
	(dn_ipmg_getNextPathInfo_rpt*)(app_vars.replyBuf) // reply
	);

	// log
	SerialPrintln(err);

	if (err == DN_ERR_NONE) {
		app_vars.cbStatus = CommandSent;
		// arm callback
		fsm_setCallback(&IpMgWrapper::api_getNextPathInfo_reply);
		// schedule timeout event
		fsm_scheduleEvent(
		SERIAL_RESPONSE_TIMEOUT,
		&IpMgWrapper::api_response_timeout
		);
	}
}


void IpMgWrapper::api_getNextPathInfo_reply() {
	dn_ipmg_getNextPathInfo_rpt* reply;
	// log
	SerialPrintln("INFO:     api_getPathMote_reply");

	// cast reply
	reply = (dn_ipmg_getNextPathInfo_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
	SerialPrint("          source=");
	printByteArray(reply->source, 8);
	SerialPrintln();
	SerialPrint("          dest=");
	printByteArray(reply->dest,8);
	SerialPrintln();
	SerialPrint("          direction=");
	SerialPrintln(reply->direction);
	SerialPrint("          numLinks=");
	SerialPrintln(reply->numLinks);
	SerialPrint("          quality=");
	SerialPrintln(reply->quality);

	if (reply->RC  == RC_OK) {
		// remember current link
		ipmg_app_vars.currentPathId = reply->pathId;

		// schedule next event
		dustManager.getWrapper()->fsm_scheduleEvent(
		CMD_PERIOD,
		&IpMgWrapper::api_getNextPathInfo
		);

		app_vars.cbStatus = Working;
		} else if (reply->RC == RC_END_OF_LIST){
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}


void IpMgWrapper::executeReplyCb()
{
	(dustManager.getWrapper()->*ipmg_app_vars.replyCb)();
}

void IpMgWrapper::api_sendData_reply() {
	dn_ipmg_sendData_rpt* reply;

	// log
	SerialPrintln("INFO:     api_sendData_reply");

	// cast reply
	reply = (dn_ipmg_sendData_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
	SerialPrint(" CallBackId=");
	SerialPrintln(reply->callbackId);


	// remove the Timeout Event
	fsm_cancelEvent();
}

bool IpMgWrapper::sendData(DataModel *sendData) {
	dn_err_t   err;
	IpMgDataModel *mngData = (IpMgDataModel*)sendData;

	// issue function
	err = dn_ipmg_sendData(
	(uint8_t*) mngData->getDestinationMac(),    // macAddress
	mngData->getPriority(),                     // priority
	mngData->getSrcPort(),                      // srcPort
	mngData->getDstPort(),                      // dstPort
	mngData->getOption(),                       // options
	(uint8_t*) mngData->getDataToSend(),        // data
	mngData->getDataLen(),                      // dataLen
	(dn_ipmg_sendData_rpt*)(app_vars.replyBuf)  // reply
	);

	// arm callback
	fsm_setCallback(&IpMgWrapper::api_sendData_reply);

	return (err == DN_ERR_NONE);
}

void IpMgWrapper::api_getNetworkInfo(void) {
	dn_err_t err;

	app_vars.cbStatus = Idle;
	// log
	SerialPrintln("");
	SerialPrint("INFO:     dn_ipmg_getNetworkInfo");

	// issue function
	err = dn_ipmg_getNetworkInfo(
	(dn_ipmg_getNetworkInfo_rpt*)(app_vars.replyBuf) // reply
	);

	// log
	SerialPrintln(err);

	if (err == DN_ERR_NONE) {
		app_vars.cbStatus = CommandSent;
		// arm callback
		fsm_setCallback(&IpMgWrapper::api_getNetworkInfo_reply);
		// schedule timeout event
		fsm_scheduleEvent(
		SERIAL_RESPONSE_TIMEOUT,
		&IpMgWrapper::api_response_timeout
		);
	}
}


void IpMgWrapper::api_getNetworkInfo_reply() {
	dn_ipmg_getNetworkInfo_rpt* reply;
	// log
	SerialPrintln("INFO:     api_getNetworkInfo_reply");

	// cast reply
	reply = (dn_ipmg_getNetworkInfo_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
	SerialPrint("          numMotes=");
	SerialPrintln(reply->numMotes);

	SerialPrint("          netReliability=");
	SerialPrintln(reply->netReliability);

	SerialPrint("          netPathStability=");
	SerialPrintln(reply->netPathStability);

	SerialPrint("          netLatency=");
	SerialPrintln(reply->netLatency);

	SerialPrint("          netState=");
	SerialPrintln(reply->netState);

	if (reply->RC  == RC_OK) {
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}

void IpMgWrapper::api_getNetworkConfig(void)
{
	dn_err_t err;

	app_vars.cbStatus = Idle;
	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getNetworkConfig");

	// issue function
	err = dn_ipmg_getNetworkConfig(
	(dn_ipmg_getNetworkConfig_rpt*)(app_vars.replyBuf) // reply
	);

	// log
	SerialPrintln(err);

	if (err == DN_ERR_NONE) {
		app_vars.cbStatus = CommandSent;
		// arm callback
		fsm_setCallback(&IpMgWrapper::api_getNetworkConfig_reply);

		// schedule timeout event
		fsm_scheduleEvent(
		SERIAL_RESPONSE_TIMEOUT,
		&IpMgWrapper::api_response_timeout
		);
	}
}


void IpMgWrapper::api_getNetworkConfig_reply(void)
{
	dn_ipmg_getNetworkConfig_rpt* reply;
	
	// log
	SerialPrintln("INFO:     api_getNetworkConfig_reply");

	// cast reply
	reply = (dn_ipmg_getNetworkConfig_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
	SerialPrint("          numMotes=");
	SerialPrintln(reply->maxMotes);

	SerialPrint("          apTxPower=");
	SerialPrintln(reply->apTxPower);

	SerialPrint("          autoStartNetwork=");
	SerialPrintln(reply->autoStartNetwork);

	SerialPrint("          channelList=");
	SerialPrintln(reply->channelList);

	SerialPrint("          autoStartNetwork=");
	SerialPrintln(reply->autoStartNetwork);

	if (reply->RC  == RC_OK) {
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}



void IpMgWrapper::api_getSystemInfo(void)
{
	dn_err_t err;

	app_vars.cbStatus = Idle;
	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getSystemInfo");

	// issue function
	err = dn_ipmg_getSystemInfo(
	(dn_ipmg_getSystemInfo_rpt*)(app_vars.replyBuf) // reply
	);

	// log
	SerialPrintln(err);

	if (err == DN_ERR_NONE) {
		app_vars.cbStatus = CommandSent;
		// arm callback
		fsm_setCallback(&IpMgWrapper::api_getSystemInfo_reply);

		// schedule timeout event
		fsm_scheduleEvent(
		SERIAL_RESPONSE_TIMEOUT,
		&IpMgWrapper::api_response_timeout
		);
	}
}



void IpMgWrapper::api_getSystemInfo_reply()
{
	dn_ipmg_getSystemInfo_rpt* reply;

	// log
	SerialPrintln("INFO:     api_getNetworkConfig_reply");

	// cast reply
	reply = (dn_ipmg_getSystemInfo_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     MAC= ");
	SerialPrint(reply->macAddress[0], HEX);
	SerialPrint(":");
	SerialPrint(reply->macAddress[1], HEX);
	SerialPrint(":");
	SerialPrint(reply->macAddress[2], HEX);
	SerialPrint(":");
	SerialPrint(reply->macAddress[3], HEX);
	SerialPrint(":");
	SerialPrint(reply->macAddress[4], HEX);
	SerialPrint(":");
	SerialPrint(reply->macAddress[5], HEX);
	SerialPrint(":");
	SerialPrint(reply->macAddress[6], HEX);
	SerialPrint(":");
	SerialPrint(reply->macAddress[7], HEX);

	SerialPrint("          hwModel=");
	SerialPrintln(reply->hwModel);

	SerialPrint("          hwRev=");
	SerialPrintln(reply->hwRev);

	SerialPrint("          SW=");
	SerialPrint(reply->swMajor);
	SerialPrint(".");
	SerialPrint(reply->swMinor);
	SerialPrint(".");
	SerialPrintln(reply->swPatch);

	SerialPrint("          swBuild=");
	SerialPrintln(reply->swBuild,HEX);

	if (reply->RC  == RC_OK) {
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}

void IpMgWrapper::api_getMoteInfo(const uint8_t mac[8])
{
	dn_err_t err;

	app_vars.cbStatus = Idle;
	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getMoteInfo");

	// issue function
	err = dn_ipmg_getMoteInfo(
	(uint8_t*)mac,
	(dn_ipmg_getMoteInfo_rpt*)(app_vars.replyBuf) // reply
	);

	// log
	SerialPrintln(err);

	if (err == DN_ERR_NONE) {
		app_vars.cbStatus = CommandSent;
		// arm callback
		fsm_setCallback(&IpMgWrapper::api_getMoteInfo_reply);

		// schedule timeout event
		fsm_scheduleEvent(
		SERIAL_RESPONSE_TIMEOUT,
		&IpMgWrapper::api_response_timeout
		);
	}
}

void IpMgWrapper::api_getMoteInfo_reply() {
	dn_ipmg_getMoteInfo_rpt* reply;
	// log
	SerialPrintln("INFO:     api_getNetworkConfig_reply");

	// cast reply
	reply = (dn_ipmg_getMoteInfo_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC=");
	SerialPrintln(reply->RC);
	SerialPrint("          numMotes=");
	SerialPrintln(reply->numGoodNbrs);

	SerialPrint("          packetsLost=");
	SerialPrintln(reply->packetsLost);

	SerialPrint("          packetsReceived=");
	SerialPrintln(reply->packetsReceived);

	SerialPrint("          avgLatency=");
	SerialPrintln(reply->avgLatency);

	SerialPrint("          assignedBw=");
	SerialPrintln(reply->assignedBw);

	if (reply->RC  == RC_OK) {
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}

//=========================== callback functions for ipmg =====================

void dn_ipmg_notif_cb(uint8_t cmdId, uint8_t subCmdId) {

	if (cmdId==DN_NOTIFID_NOTIFDATA) {

		if (dustManager.dispatchMessage((dn_ipmg_notifData_nt*)app_vars.syncDataToSend->getNotifyBuffer())) {
			dustManager.getWrapper()->setMgStatus(DataReceived);
			} else {
			// msg received but from a not registered mote
			dustManager.getWrapper()->setMgStatus(Idle);
		}

		} else if (cmdId==DN_NOTIFID_NOTIFEVENT) {
		dustManager.eventNotification(app_vars.syncDataToSend->getNotifyBuffer(), subCmdId);
	}
}

extern "C" void dn_ipmg_reply_cb(uint8_t cmdId) {
	dustManager.getWrapper()->executeReplyCb();
}

extern "C" void dn_ipmg_status_cb(uint8_t newStatus) {
	switch (newStatus) {
		case DN_SERIAL_ST_CONNECTED:

		// log
		SerialPrintln("INFO:     connected");

		// schedule next event
		dustManager.getWrapper()->fsm_scheduleEvent(
		INTER_FRAME_PERIOD,
		&IpMgWrapper::api_subscribe
		);
		break;
		case DN_SERIAL_ST_DISCONNECTED:
		// log
		SerialPrintln("WARNING:  disconnected");

		// schedule first event
		dustManager.getWrapper()->fsm_scheduleEvent(
		INTER_FRAME_PERIOD,
		&IpMgWrapper::api_initiateConnect
		);

		break;
		default:
		SerialPrintln("ERROR:    unexpected newStatus");
	}
}

void IpMgWrapper::api_getManagerIp(void)
{
	dn_err_t err;

	app_vars.cbStatus = Idle;
	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_getManagerIp");

	// issue function
	err = dn_ipmg_getIPConfig(
	(dn_ipmg_getIPConfig_rpt*)(app_vars.replyBuf) // reply
	);

	// log
	SerialPrintln(err);

	if (err == DN_ERR_NONE) {
		app_vars.cbStatus = CommandSent;
		// arm callback
		fsm_setCallback(&IpMgWrapper::api_getManagerIp_reply);

		// schedule timeout event
		fsm_scheduleEvent(
		SERIAL_RESPONSE_TIMEOUT,
		&IpMgWrapper::api_response_timeout
		);
	}
}



void IpMgWrapper::api_getManagerIp_reply()
{
	dn_ipmg_getIPConfig_rpt* reply;

	// log
	SerialPrintln("INFO:     api_getManagerIp_reply");

	// cast reply
	reply = (dn_ipmg_getIPConfig_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     IP= ");
	SerialPrint(reply->ipv6Address[0], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[1], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[2], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[3], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[4], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[5], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[6], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[7], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[8], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[9], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[10], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[11], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[12], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[13], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[14], HEX);
	SerialPrint(":");
	SerialPrint(reply->ipv6Address[15], HEX);

	SerialPrint("          mask=");
	SerialPrint(reply->mask[0], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[1], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[2], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[3], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[4], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[5], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[6], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[7], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[8], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[9], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[10], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[11], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[12], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[13], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[14], HEX);
	SerialPrint(":");
	SerialPrint(reply->mask[15], HEX);

	if (reply->RC  == RC_OK) {
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}

void IpMgWrapper::api_changeNetworkId(const dn_ipmg_getNetworkConfig_rpt *netMsg, bool wholeNet)
{
	dn_err_t err;

	app_vars.cbStatus = Idle;
	// log
	SerialPrintln("");
	SerialPrint("INFO:     api_changeNetworkId");

	// issue function
	err = dn_ipmg_setNetworkConfig (netMsg->networkId, netMsg->apTxPower, netMsg->frameProfile,
	netMsg->maxMotes, netMsg->baseBandwidth, netMsg->downFrameMultVal,
	netMsg->numParents,netMsg->ccaMode,netMsg->channelList,netMsg->autoStartNetwork,
	netMsg->locMode, netMsg->bbMode, netMsg->bbSize,
	netMsg->isRadioTest, netMsg->bwMult, netMsg->oneChannel,
	(dn_ipmg_setNetworkConfig_rpt*)(app_vars.replyBuf) // reply
	);

	// log
	SerialPrintln(err);

	if (err == DN_ERR_NONE) {
		app_vars.cbStatus = CommandSent;
		// arm callback
		fsm_setCallback(&IpMgWrapper::api_changeNetworkId_reply);

		// schedule timeout event
		fsm_scheduleEvent(
		SERIAL_RESPONSE_TIMEOUT,
		&IpMgWrapper::api_response_timeout
		);
	}
}

void IpMgWrapper::api_changeNetworkId_reply()
{
	dn_ipmg_setNetworkConfig_rpt* reply;

	// log
	SerialPrintln("INFO:     api_changeNetworkId_reply");

	// cast reply
	reply = (dn_ipmg_setNetworkConfig_rpt*)app_vars.replyBuf;

	// log
	SerialPrint("INFO:     RC= ");
	SerialPrint(reply->RC, DEC);
	

	if (reply->RC  == RC_OK) {
		app_vars.cbStatus = Completed;

		// clear timeout
		fsm_cancelEvent();

		// end of list or problem reading
		// reset next mote to ask
		SerialPrintln("INFO:     END LIST");
	}
}
