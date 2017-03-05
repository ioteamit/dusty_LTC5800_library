/*
 * IpMgDataModel.cpp
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
#include "IpMgDataModel.h"

IpMgDataModel::IpMgDataModel(const uint8_t *mac, uint16_t srcPort, uint16_t dstPort) {
    memcpy(macAddress, mac, 8);
    this->srcPort = srcPort;
    this->dstPort = dstPort;
    options=0;
    priority=0;
}

bool IpMgDataModel::isThisMote(const uint8_t *externalMac)
{
    return ((macAddress[5] == externalMac[5]) &&
            (macAddress[6] == externalMac[6]) &&
            (macAddress[7] == externalMac[7]));
}

void IpMgDataModel::setMsgReceived(dn_ipmg_notifData_nt* notifData_notif)
{
    memcpy(dataBuffer, notifData_notif, sizeof(dn_ipmg_notifData_nt));
}
