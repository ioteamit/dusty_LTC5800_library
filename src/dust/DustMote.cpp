/*
 * SmeDustMote.h
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

#include "DustMote.h"
IpMtWrapper  ipmtwrapper;
DustMote  dustMote;

void DustMote::begin (uint16_t srcPort, uint8_t* destAddr, uint16_t destPort,
		TIME_T dataPeriod, IpMtDataModel *dataToSend,
		boolean polling,  status_update  statusUpd_cb, char CtsPin, Uart *serial,
		uint16_t			networkId)
{	
#if defined (DUST_DEBUG)
    #ifdef MKR_SAMD_LINEAR
    if (serialAntenna == *serial) {
        SerialPrint("Serial of ");
        SerialPrintln("MKR_SAMD_LINEAR");
    } else   
    #endif
     if (Serial1 == *serial) {
        SerialPrint("Serial of ");
        SerialPrintln("Serial1");
    }  
    else if (Serial == *serial) {
        SerialPrint("Serial of ");
        SerialPrintln("Serial1");
    }
#endif

	DustMgMt::begin(CtsPin);

	// it could be ignored
	ipmtwrapper.begin(srcPort, destAddr, destPort, dataPeriod,
			dataToSend, polling, statusUpd_cb, serial, networkId);		
}

void DustMote::start(void)
{
	if (!homologation) {
		ipmtwrapper.start();
	} else {
		ipmtwrapper.startHomologation(txType, chanMask, power);
	}
}

DustCbStatusE DustMote::readData (void) {
	return ipmtwrapper.main();
}

void DustMote::sendData (DataModel *sendData) {
	if (sendData!= NULL)
		ipmtwrapper.addAsyncronousSend(sendData);

	ipmtwrapper.api_sendTo();

	if (sendData!= NULL)
		ipmtwrapper.deleteAsyncronousSend();
};

void DustMote::retrieveNetworkId(void) {
	ipmtwrapper.getConfiguredNetworkId();
}

void DustMote::retrieveMac(void) {
	ipmtwrapper.getMac();
}