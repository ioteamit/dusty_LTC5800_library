/*
 * DustManager.h
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

#ifndef DUSTMANAGER_H_
#define DUSTMANAGER_H_



#include <stdint-gcc.h>
#include <Arduino.h>
#include "dust/IpMgWrapper/IpMgWrapper.h"
#include "dust/IpMgMtCommon/DustMgMt.h"
#include "dust/IpMgWrapper/IpMgWrapper.h"
#include "DustDefines.h"

#define ALL_PATH 0
#define UPSTREAM_PATH 1

// external variable used by the sketches
extern IpMgWrapper  ipmgwrapper;
typedef void (*eventCallBack) (uint8_t eventId, uint8_t mac[8]);

class DustManager : public DustMgMt {

public:
	DustManager(){};
	virtual ~DustManager(){};

private:
	uint8_t motePosition;
	IpMgDataModel*  motes[MAX_MOTE_NUMBER];
	IpMgWrapper     ipmgwrapper;
	IpMgDataModel*  lastReceiveMsg;
	eventCallBack eventFunct;

	// User function
public:
	/**
	 * begin
	 *
	 * This is the initialization method 
	 * eventNotification = true if we wnat to trap every manager event
	 * eventCallBack     = the function that will handle the event if the eventNotification parameter is true
	 * CtsPin            = the pin that will handle the CTS LTC5800 pin
	 * RtsPin            = the pin that will handle the RTS LTC5800 pin
	 * serial            = the pointer to the Serial object used for the internal comunication with the LTC5800 (IOTEAM & ARROW boards do not need this)
	 **/ 
#ifdef MKR_SAMD_LINEAR
	boolean begin (bool eventNotification=false, eventCallBack eventFunct=NULL,
			char CtsPin = PIN_ANTENNA_CTS, char RtsPin=-1, Uart *serial=&serialAntenna);
#elif ARDUINO_SAMD_SMARTEVERYTHING_DRAGONFLY
boolean begin (bool eventNotification=false, eventCallBack eventFunct=NULL,
			  char CtsPin = PIN_DUST_CTS,  char RtsPin=-1, Uart *serial=&SerialDust); // the CtsPin in Dragonfly is keep high by Hw
#else
	boolean begin (bool eventNotification=false, eventCallBack eventFunct=NULL,
			char CtsPin = PIN_LED, char RtsPin=-1, Uart *serial=&Serial1); //by default use one common Arduino PIN
#endif
	/**
	 * this is the main method used by the manager to move the internal FSM, 
                  it shall be always called because return the latest received DataModel 
                  or the complete of the interanl comand
         *
         **/
	DustCbStatusE readData (void);

        /**
	 *
	 * Send the information contains in the DataModel
	 *
	 **/
	boolean sendData(DataModel *sendData=NULL);
	boolean listOfMac(boolean start=TRUE);
	void listOfPath(char mac[8], char direction=UPSTREAM_PATH);
	inline const uint8_t* getLastCommand(){return ipmgwrapper.getLastCommand();};

	/**
	 * used to register a the dataModel of a specific mote. 
	 *     only the registered mote will be handle.
         **/
	boolean registerMote(IpMgDataModel *mote);

	/**
	 * Return the DataModel of the latest message received
	 *
	 **/
	IpMgDataModel *getLastMessage(void){return lastReceiveMsg;};
	void retrieveNetworkInfo(void);
	void retrieveNetworkConfig();
	void retrieveMoteInfo(const byte mac[8]);
    void retrieveManagerInfo(void);
    void retrieveManagerIp(void);
	void changeNetworkConfig(const dn_ipmg_getNetworkConfig_rpt *netMsg);

	// Library Function
public:
	bool dispatchMessage(dn_ipmg_notifData_nt* notifData);
	void eventNotification(byte* notifEvent, uint8_t subCmdId);
	IpMgWrapper* getWrapper(void){return &ipmgwrapper;};
};
extern DustManager  dustManager;

#endif /* SMEDUST_H_ */
