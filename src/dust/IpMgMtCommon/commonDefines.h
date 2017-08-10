/*
 * commonDefines.h
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
#ifndef COMMON_DEFINES_H
#define  COMMON_DEFINES_H

//=========================== defines =========================================
#ifndef DUST_DEBUG
//#define DUST_DEBUG
#endif

#ifndef DEBUG_ON_AIR
//#define DEBUG_ON_AIR
#endif

#define TIME_T                    unsigned long

#define BAUDRATE_CLI              9600
#define BAUDRATE_API              115200

#define IPv6ADDR_LEN              16
#define DEFAULT_SOCKETID          22
#define MAC_LEN                   8


#define RC_OK               0  //The application layer has processed the command correctly
#define RC_INVALID_COMMAND  1  //Invalid command
#define RC_INVALID_ARGUMENT 2  //Invalid argument
#define RC_END_OF_LIST      11 //End of list is returned when an iteration reaches the end of the list of objects
#define RC_NO_RESOURCES     12 // Reached maximum number of items
#define RC_IN_PROGRESS      13 // Operation is in progress
#define RC_NACK             14 //Negative acknowledgment
#define RC_WRITE_FAIL       15 //Flash write failed
#define RC_VALIDATION_ERROR 16 //Parameter validation error
#define RC_INV_STATE        17 //Object has inappropriate state
#define RC_NOT_FOUND        18 //Object is not found
#endif
