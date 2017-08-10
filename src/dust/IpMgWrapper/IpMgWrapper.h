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

#ifndef IPMGWRAPPER_H
#define IPMGWRAPPER_H


#include <Arduino.h>
#include "../../sm_clib/dn_ipmt.h"
#include "../IpMgMtCommon/IpMgMtWrapper.h"
#include "../dataModel/IpMgDataModel.h"

//=========================== typedef =========================================

// forward declaration needed here
class IpMgWrapper;
typedef void (IpMgWrapper::*fsm_mg_timer_callback)(void);
typedef void (IpMgWrapper::*fsm_mg_reply_callback)(void);

typedef struct {
	// reply
	fsm_mg_reply_callback   replyCb;
	fsm_mg_timer_callback   fsmCb;
	bool                    fsmArmed;
	// api
	uint8_t                 currentMac[8];
	char					pathFilter;

} mg_app_vars_t;

//=========================== IpMgWrapper object ==============================

class IpMgWrapper : public IpMgMtWrapper {
public:
	//===== methods
	IpMgWrapper();

	/*
	 *
	 * The begin cover
	 *      the initiateConnect
	 *      and the subscribe phase
	 *
	 */
	bool    begin(Uart          *serial,
			bool eventNotification);
	DustCbStatusE    main();
	//===== attributes
	//===== methods
	//=== fsm
	void                   fsm_scheduleEvent(
			uint16_t            delay,
			fsm_mg_timer_callback  cb
	);
	void            fsm_setCallback(fsm_mg_reply_callback cb);
	void            fsm_cancelEvent();
	void            executeReplyCb();

	//=== api
	void            api_initiateConnect();
	void            api_subscribe();
	void            api_getNextMoteConfig();
	void            api_getNextPathInfo(void);
	inline void     resetCurrentMac() {memset(ipmg_app_vars.currentMac,0, sizeof(ipmg_app_vars.currentMac));}; // start from manager
	void            resetCurrentPathId(char mac[8], char pathFilter);
	bool            sendData(DataModel *sendData);
	void            api_getNetworkInfo(void);
	void            api_getNetworkConfig();
	void            api_getMoteInfo(const uint8_t mac[8]);
    void            api_getSystemInfo(void);
	void			api_getManagerIp(void);
	void			api_changeNetworkConfig(const dn_ipmg_getNetworkConfig_rpt *netMsg);


private:
	//=== api
	void            oap_response_timeout();
	void            api_subscribe_reply();
	void            api_sendData_reply();
	void            api_getNextMoteConfig_reply();
	void            api_getNextPathInfo_reply();
	void            api_response_timeout();
	void            api_getNetworkInfo_reply();
	void            api_getNetworkConfig_reply();
	void            api_getMoteInfo_reply();
    void            api_getSystemInfo_reply();
    void            api_getManagerIp_reply();
	void			api_changeNetworkConfig_reply();

private:
	//===== attributes
	mg_app_vars_t   ipmg_app_vars;
	bool            eventNotification;
	
};


#endif
