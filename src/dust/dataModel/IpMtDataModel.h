/*
 * IpMtDataModel.cpp
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
#ifndef IP_MT_DATAMODEL_H
#define IP_MT_DATAMODEL_H

#include "DataModel.h"
#include "sm_clib\dn_ipmt.h"
class IpMtDataModel: public DataModel {

private:

public:
    IpMtDataModel();
    IpMtDataModel(data_generator callBack):DataModel(callBack){
        memset(&dataBuffer, 0x33, sizeof(dn_ipmt_receive_nt));
    };
    
    inline void dataToSend(const uint8_t *data, uint8_t dataLen) {
        dn_ipmt_receive_nt *buf = (dn_ipmt_receive_nt*)dataBuffer;
        memcpy(buf->payload, data, dataLen);
        buf->payloadLen = dataLen;
    };

    const uint8_t*  fetchLastMessage(uint8_t* len);

    inline const uint8_t*  getDataToSend(void) {
        dn_ipmt_receive_nt *buf = (dn_ipmt_receive_nt*)dataBuffer;
        return (const uint8_t*)buf->payload;
    };

    inline uint8_t  getDatasize(void){
        dn_ipmt_receive_nt *buf = (dn_ipmt_receive_nt*)dataBuffer;
        return buf->payloadLen;
    };

    inline void  setDatasize(uint8_t size){
        dn_ipmt_receive_nt *buf = (dn_ipmt_receive_nt*)dataBuffer;
        buf->payloadLen = size;
    };

    inline uint8_t*  getLastMessage(void) {
        dn_ipmt_receive_nt *buf = (dn_ipmt_receive_nt*)dataBuffer;
        return buf->payload;
    };

    inline uint8_t  getLastMessageLen(void) {
        dn_ipmt_receive_nt *buf = (dn_ipmt_receive_nt*)dataBuffer;
        return buf->payloadLen;
    };
};
#endif
