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
#include "IpMtDataModel.h"
#include "sm_clib\dn_ipmt.h"


IpMtDataModel::IpMtDataModel(){

}

const uint8_t* IpMtDataModel::fetchLastMessage(uint8_t* len) {
    dn_ipmt_receive_nt *buf = (dn_ipmt_receive_nt*)dataBuffer;
    *len = buf->payloadLen;
    
    return buf->payload;
};