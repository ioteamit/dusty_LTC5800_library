/*
 * DataModel.h
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

#ifndef _DATAMODEL_H
#define _DATAMODEL_H

#include <stdint-gcc.h>
#include "../../sm_clib/dn_ipmg.h"
#include "../IpMgMtCommon/commonDefines.h"

typedef class DataModel;
typedef uint8_t* (*data_generator)(DataModel *model);

class DataModel {

protected:

    const char brcMac[MAC_LEN];
    data_generator      specificDataGenerator;

    /*
     * For the data buffer a unique array is used.
     * This is possible because it could be not possible to have a Mote that is also a Manager, and vice versa
     *
     * Because of that the size of the buffer is equal to the biggest structure of both the case.
     */
    char  dataBuffer[sizeof(dn_ipmg_notifData_nt)];        // notifications buffer internal to ipmg

protected:
    DataModel(): brcMac({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}){};
    DataModel(data_generator callBack):specificDataGenerator(callBack),
     brcMac({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}){};

    /*-------------- abstract methods --------------------*/
public:
    virtual uint8_t         getDatasize(void)=0;
    virtual void            setDatasize(uint8_t size)=0;
    virtual void            dataToSend(const uint8_t *data, uint8_t dataLen)=0;
    /**
     *
     * Return the pointer to the latest message received and its length
     *
     **/
    virtual const uint8_t*  fetchLastMessage(uint8_t* len)=0;
    virtual const uint8_t*  getDataToSend(void)=0;
    virtual uint8_t*        getLastMessage(void)=0;
    virtual uint8_t         getLastMessageLen(void) =0;


    /*-------------- end abstract methods --------------------*/

protected:


public:
    /* ------------- USER PUBLIC FUNCTION ---------------*/
    virtual uint8_t* getPayload(void){return specificDataGenerator(this);};
    inline uint8_t* getNotifyBuffer(void) { return (uint8_t*)&dataBuffer;};
    inline uint8_t  getNotifyBufferLen(void) {return sizeof(dn_ipmg_notifData_nt);};
};

#endif /*_DATAMODEL_H*/
