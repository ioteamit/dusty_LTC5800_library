/*
 * Dustmote.h
 *
 *  Created on: Apr 26, 2015
 * by mik (mik@ioteam.it)
 *
 * License Information
 * -------------------
 *
 * Copyright (c) IOteam S.r.l. All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DUSTMOTE_H_
#define DUSTMOTE_H_



#include <stdint-gcc.h>
#include <Arduino.h>
#include "dust/IpMtWrapper/IpMtDefines.h"
#include "dust/IpMtWrapper/IpMtWrapper.h"
#include "dust/dataModel/IpMtDataModel.h"
#include "dust/IpMgMtCommon/DustMgMt.h"



// external variable used by the sketches
extern IpMtWrapper  ipmtwrapper;

class DustMote : public DustMgMt {

public:
	DustMote(){};
	virtual ~DustMote(){};

private:	
	
	// library API
public:
#ifdef MKR_SAMD_LINEAR
	void begin (uint16_t srcPort, uint8_t* destAddr, uint16_t destPort,
			TIME_T dataPeriod, IpMtDataModel *dataToSend,
			boolean polling=true, status_update  statusUpd_cb = NULL,
			char CtsPin = PIN_ANTENNA_CTS, Uart *serial=&serialAntenna,
			uint16_t			networkId=DEFAULT_NETWORK_ID);
#elif IOTEAM_SAMD_DUSTINO
	void begin (uint16_t srcPort, uint8_t* destAddr, uint16_t destPort,
	TIME_T dataPeriod, IpMtDataModel *dataToSend,
	boolean polling=true, status_update  statusUpd_cb = NULL,
	char CtsPin = PIN_DUST_CTS, Uart *serial=&SerialDust,
	uint16_t networkId=DEFAULT_NETWORK_ID);
#else
	//by default use one common Arduino PIN
	void begin (uint16_t srcPort, uint8_t* destAddr, uint16_t destPort,
			TIME_T dataPeriod, IpMtDataModel *dataToSend,
			boolean polling=true, status_update  statusUpd_cb = NULL,
			char CtsPin = PIN_LED, Uart *serial=&Serial1,
			uint16_t			networkId=DEFAULT_NETWORK_ID); 
#endif
	void start(void);
	DustCbStatusE readData (void);
	void sendData (DataModel *sendData=NULL);
	inline const uint8_t* getLastCommand(){return ipmtwrapper.getLastCommand();};
	void retrieveNetworkId();
	void retrieveNetworkInfo();
	inline const char* getMac(void) {
		return ipmtwrapper.getMacAddress();
	}
	inline bool	getHomologationResult(){return ipmtwrapper.getHomologationResult();};
	inline bool	getHomologationRunning(){return ipmtwrapper.getHomologationRunning();};
	

};

extern DustMote  dustMote;


#endif /* SMEDUST_H_ */
