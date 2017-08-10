/*
 * DustManager.cpp
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

#include "DustManager.h"
DustManager  dustManager;

boolean DustManager::begin (bool eventNotification, eventCallBack eventFunct,
		char CtsPin, Uart *serial)
{
    
	if (eventNotification && eventFunct == NULL) {
		SerialPrint("\n\nWARNING: You set the request to have notification without add a callback to handle it.\n\n");
		SerialPrintln();
		SerialPrintln();
	}

	this->eventFunct = eventFunct;
	DustMgMt::begin(CtsPin);        

	//initialize the Motes array
	uint8_t i;
	motePosition = 0;

	for(i=0; i<MAX_MOTE_NUMBER;i++)
		motes[i] = 0;
        
	// it could be ignored
	return ipmgwrapper.begin(serial, eventNotification);
}


DustCbStatusE DustManager::readData (void) {
	return ipmgwrapper.main();
}

boolean DustManager::sendData (DataModel *sendData) {
	return ipmgwrapper.sendData(sendData);
};

boolean DustManager::registerMote(IpMgDataModel *mote){
	boolean ret = false;
	if (motePosition < MAX_MOTE_NUMBER) {
		motes[motePosition] = mote;
		motePosition++;
		ret = true;
	}

	return ret;
}


boolean DustManager::listOfMac(boolean start) {
	if (start)
		ipmgwrapper.resetCurrentMac(); 

	ipmgwrapper.api_getNextMoteConfig();
}

void DustManager::listOfPath(char mac[8], char pathFilter) {
	
	// set the strt mac
	ipmgwrapper.resetCurrentPathId(mac, pathFilter);
	ipmgwrapper.api_getNextPathInfo();
}

void DustManager::eventNotification(byte* notifEventStr, uint8_t subCmdId) {    
	uint8_t mac[8];
	dn_ipmg_eventMoteOperational_nt *notif_eventMoteOperational;
	dn_ipmg_eventMoteLost_nt        *notif_eventMoteLost;

	SerialPrint("INFO:     DN_NOTIFID_NOTIFEVENT");
	SerialPrint(" SubEventId = ");
	SerialPrintln(subCmdId, HEX);

	switch (subCmdId)
	{
	case DN_EVENTID_EVENTMOTEOPERATIONAL:
		notif_eventMoteOperational = (dn_ipmg_eventMoteOperational_nt*)notifEventStr;
		if (eventFunct)
			eventFunct(subCmdId, notif_eventMoteOperational->macAddress);
		break;

	case DN_EVENTID_EVENTMOTELOST:
		notif_eventMoteLost = (dn_ipmg_eventMoteLost_nt*)notifEventStr;
		if (eventFunct)
			eventFunct(subCmdId, notif_eventMoteLost->macAddress);
		break;


	case DN_EVENTID_EVENTMOTEDELETE:
		SerialPrint(" SubEventId = ");
		SerialPrintln(subCmdId, HEX);
		break;

	default:

		break;
	}


}

bool DustManager::dispatchMessage(dn_ipmg_notifData_nt* notifData) {
	int i;
	SerialPrint("Manager   DN_NOTIFID_NOTIFDATA ");
	printByteArray(notifData->macAddress, sizeof(notifData->macAddress));
	SerialPrint(" Data len = ");
	SerialPrint(notifData->dataLen);
	SerialPrint(" value = ");
	printByteArray(notifData->data, notifData->dataLen);

	for (i=0; i<MAX_MOTE_NUMBER; i++) {
		if (motes[i]->isThisMote(notifData->macAddress)) {
			motes[i]->setMsgReceived(notifData);
			lastReceiveMsg = motes[i];
			SerialPrintln();
			return true;
		}
	}

	SerialPrintln("NOT REGISTERED Mote !!");
	SerialPrintln();
	return false;
}

void DustManager::retrieveNetworkInfo(void) {
	ipmgwrapper.api_getNetworkInfo();
}

void DustManager::retrieveNetworkConfig()
{
	ipmgwrapper.api_getNetworkConfig();
}

void DustManager::retrieveMoteInfo(const byte mac[8])
{
	ipmgwrapper.api_getMoteInfo(mac);
}

/**
 * \brief 
 *      Send The message to the manager to retrieve its information
 *      It should be read with the following structure dn_ipmg_getSystemInfo_rpt
 *      dn_ipmg_getSystemInfo_rpt *data = (dn_ipmg_getSystemInfo_rpt*) this->getLastCommand()
 * 
 * \param 
 * 
 * \return void
 */
void DustManager::retrieveManagerInfo(void)
{
	ipmgwrapper.api_getSystemInfo();
}

void DustManager::retrieveManagerIp(void)
{
	ipmgwrapper.api_getManagerIp();
}



void DustManager::changeNetworkConfig(const dn_ipmg_getNetworkConfig_rpt *netMsg)
{
	ipmgwrapper.api_changeNetworkConfig(netMsg);
}