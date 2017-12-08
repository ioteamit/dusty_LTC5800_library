/*
 * IpMgDataModel.h
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

#ifndef IP_MG_DATAMODEL_H
#define IP_MG_DATAMODEL_H

#include "DataModel.h"
#include "../../sm_clib/dn_ipmg.h"

class IpMgDataModel: public DataModel {

private:
    uint8_t  options;
    uint8_t  priority;
    uint16_t dstPort;
    uint16_t srcPort;
    uint8_t  macAddress[8];


public:
    IpMgDataModel(){options=0, priority=0;};
    /**
     * 
     * configure the outgoing an incoming port used by the Mote, identified by its mac.
     *
     **/
    IpMgDataModel(const uint8_t *mac, uint16_t srcPort, uint16_t dstPort);



public:
    /**
     * data	: the pointer to the stream of bytes want to send
     * dataLen	: the Length of the same stream.
     *
     **/
    inline void dataToSend(const uint8_t *data, uint8_t dataLen) {
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        memcpy(buf->data, data, dataLen);
        buf->dataLen = dataLen;
    };

    inline uint8_t  getDatasize(void){
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        return buf->dataLen;
    };

    inline void  setDatasize(uint8_t size){
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        buf->dataLen = size;
    };

    inline const uint8_t*  fetchLastMessage(uint8_t* len) {
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        *len = buf->dataLen;
        return buf->data;
    };

    inline const uint8_t*  getDataToSend(void) {
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        return buf->data;
    };

    inline uint8_t*  getLastMessage(void) {
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        return buf->data;
    };

    inline uint8_t  getLastMessageLen(void) {
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        return buf->dataLen;
    };  


    inline const uint8_t* getDestinationMac()
    {
        return macAddress;
    }

    inline uint8_t getOption() {
        return options;
    };

    inline uint8_t getPriority() {
        return priority;
    };

    inline void setOption(uint8_t opt) {
        options = opt;
    };

    /**
     * Set the priority of this datamodel
     *  Used on all its message till next set
     **/
    inline void setPriority(uint8_t prio) {
        priority = prio;
    };

    inline uint16_t getDstPort() {
        return dstPort;
    };

    inline uint16_t getSrcPort() {
        return srcPort;
    };

    inline uint8_t  getDataLen() {
        dn_ipmg_notifData_nt *buf = (dn_ipmg_notifData_nt*)dataBuffer;
        return buf->dataLen;
    };

    bool isThisMote(const uint8_t *macAddress);
    void setMsgReceived(dn_ipmg_notifData_nt* notifData_notif);
};

#endif
