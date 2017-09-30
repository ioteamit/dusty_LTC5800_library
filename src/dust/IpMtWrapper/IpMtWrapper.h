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

#ifndef IPMTWRAPPER_H
#define IPMTWRAPPER_H


#include <Arduino.h>
#include "../../sm_clib/dn_ipmt.h"
#include "../dataModel/IpMtDataModel.h"
#include "../IpMgMtCommon/IpMgMtWrapper.h"


#define NOT_USE_NETWORK_ID 0xFFFF
#define DEFAULT_NETWORK_ID 0x04CD

//=========================== typedef =========================================

typedef enum  {
	packet,
	cm,
	cw,
	pkcca
	} radioTestE;

class IpMtWrapper;
typedef void (IpMtWrapper::*fsm_mt_timer_callback)(void);
typedef void (IpMtWrapper::*fsm_mt_reply_callback)(void);

typedef void(*status_update)(uint8_t state);


typedef struct {
	// reply
	fsm_mt_reply_callback   replyCb;
	fsm_mt_timer_callback   fsmCb;
	// api
	uint8_t              socketId;                          // ID of the mote's UDP socket
	uint16_t             srcPort;                           // UDP source port
	uint8_t              destAddr[IPv6ADDR_LEN];            // IPv6 destination address
	uint16_t             destPort;                          // UDP destination port
	TIME_T               dataPeriod;                        // number of ms between transmissions
	boolean              polling;                           // true if the system does not send message
	//       at dataPeriod time
	status_update        statusUpdCb;                       // callback notifying status update
} mt_app_vars_t;

//=========================== IpMtWrapper object ==============================

class IpMtWrapper : public IpMgMtWrapper{
	public:
	//===== methods
	IpMtWrapper();
	void                   begin(
	uint16_t            srcPort,
	uint8_t*            destAddr,
	uint16_t            destPort,
	TIME_T              dataPeriod,
	IpMtDataModel       *dataToSend,
	boolean             polling,
	status_update       statusUpd_cb,
	Uart                *serial,
	uint16_t			networkId=DEFAULT_NETWORK_ID
	);
	void				start(void);
	DustCbStatusE       main();
	void                addAsyncronousSend(DataModel * sendData);
	void                deleteAsyncronousSend();
	void                api_sendTo();
	void                api_getMoteStatus();
	void                fsm_scheduleEvent(uint16_t delay, fsm_mt_timer_callback cb);
	void                api_getServiceInfo();
	void                executeStatusUpdCb(uint8_t    state);
	void                executeReplyCb();
	void				getConfiguredNetworkId();
	void			    retrieveNetworkInfo();
	void				api_getNetworkInfo(void);
	void				getMac();
	void				startHomologation(radioTestE txType, uint16_t chanMask, int8_t power);
	inline bool		    getHomologationResult(){return homologationResult;};
	inline bool		    getHomologationRunning(){return homologationRunning;};
	const char*         getMacAddress(void);
	
	private:
	//===== attributes
	radioTestE	radioTest;
	uint16_t	channelTest;
	int8_t		powerTest;
	bool		homologationResult;
	bool		homologationRunning;
	
	//===== methods
	//=== fsm
	void                fsm_cancelEvent();
	void                fsm_setCallback(fsm_mt_reply_callback cb);

	//=== api
	void                api_openSocket();
	void                api_bindSocket();
	void                api_join();
	void                api_join_reply();
	void                api_requestService();
	void                api_DummyStatus();
	void				api_configureNetworkId();
	void				api_getMac();
	void				api_setNetworkId();
	void				api_setRatioTestTx();

	//=== helpers

	void                api_response_timeout();
	void                api_getServiceInfo_reply();
	void                api_bindSocket_reply();
	void                api_openSocket_reply();
	void                api_getMoteStatus_reply();
	void                api_requestService_reply();
	void                api_sendTo_reply();
	void				api_configureNetworkId_reply();
	void				api_getMac_reply();
	void				api_setNetworkId_reply();
	void				api_setRatioTestTx_reply();
	void				api_getNetworkInfo_reply();

	private:
	//===== attributes
	mt_app_vars_t          ipmt_app_vars;
	uint16_t			   networkId;
	dn_ipmt_getParameter_netInfo_rpt networkInfo;
};


#endif
