/*
 * IpMgMtWrapper.cpp
 *
 *  Created on: Apr 26, 2015
 * by Mik (mik@ioteam.it)
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
#include "IpMgMtWrapper.h"
#include "../IpMgWrapper/IpMgWrapper.h"
#include "../IpMgWrapper/IpMgDefines.h"
#include "../IpMtWrapper/IpMtDefines.h"

mg_mt_app_vars_t    app_vars;


IpMgMtWrapper::IpMgMtWrapper() {
}

void IpMgMtWrapper::begin(Uart *serial) {
    IpMgMtWrapper::dustSerial = serial;
    // reset local variables
    memset(&app_vars,    0, sizeof(mg_mt_app_vars_t));

    // initialize the serial port connected to the computer
    //SerialUSB.begin(BAUDRATE_CLI);
}

volatile uint8_t byteRead;

bool IpMgMtWrapper::readMessageExecuteCommand(void){

    while (IpMgMtWrapper::dustSerial->available()) {

        // read a serial byte
        byteRead = uint8_t(IpMgMtWrapper::dustSerial->read());

#ifdef DEBUG_ON_AIR		
		if (rcvPtr < 2000)
			debugMsg[rcvPtr++] = byteRead;
			else
			debugMsg[rcvPtr-1] = byteRead;
        debugOnAir(byteRead, '-');
#endif		

        // hand over byte to ipmg module
        app_vars.ipmgmt_uart_rxByte_cb(byteRead);
    }

}

TIME_T IpMgMtWrapper::parseMessage(void){
    // the status will be automatically changed by the CallBack
    setMgStatus(Idle);

    // receive and react to HDLC frames
    readMessageExecuteCommand();

    // kick the fsm
    return  millis();
}

void IpMgMtWrapper::resetDBGMsg(void){
#ifdef DEBUG_ON_AIR
	memset(debugMsg,0,2000);
	rcvPtr=0;
#endif
}
// need the static variable allocation
Uart* IpMgMtWrapper::dustSerial=&Serial1;
