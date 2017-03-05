/*
 * IpMgDefines.h
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

#ifndef IPMGDEFINES_H
#define IPMGDEFINES_H
//=========================== defines =========================================

// service types
#define SERVICE_TYPE_BW           0x00

// subscription
#define SUBSC_FILTER_EVENT        0x02
#define SUBSC_FILTER_LOG          0x02
#define SUBSC_FILTER_DATA         0x10
#define SUBSC_FILTER_IPDATA       0x20
#define SUBSC_FILTER_HR           0x40

// mote state
#define MANAGER_STATE_LOST           0x00
#define MANAGER_STATE_NEGOTIATING    0x01
#define MANAGER_STATE_OPERATIONAL    0x04

#define TIMEOUT_5Sec              5000 // 5*1000 mSec.
#define TIMEOUT_2Sec              2000 // 5*1000 mSec.

#endif