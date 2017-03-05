/*
 * commonHelper.cpp
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
#include "commonHelper.h"
#include "IpMgMtWrapper.h"
#include "commonDefines.h"
#include "..\IpMtWrapper\IpMtDefines.h"
#include "..\IpMgWrapper\IpMgDefines.h"

//=========================== port to Arduino =================================

//===== definition of interface declared in uart.h

void dn_uart_init(dn_uart_rxByte_cbt rxByte_cb) {
    // remember function to call back
    app_vars.ipmgmt_uart_rxByte_cb = rxByte_cb;

    // open the serial 1 port on the Arduino Due
    IpMgMtWrapper::getInternalSerial()->begin(BAUDRATE_API);
}

void debugOnAir(uint8_t byte, uint8_t dir) {
#ifdef DEBUG_ON_AIR
    SerialPrint(byte, HEX);
    Serialwrite(dir);
#endif
}

uint8_t debugMsg[2000];
uint16_t rcvPtr=1000;
uint16_t sendPtr=0;

void dn_uart_txByte(uint8_t byte) {
    // write to the serial 1 port on the Arduino Due
    IpMgMtWrapper::getInternalSerial()->write(byte);
	if (sendPtr<1000)
		debugMsg[sendPtr++] = byte;
    debugOnAir(byte, '+');
}

void dn_uart_txFlush() {
    // nothing to do since Arduino Due serial 1 driver is byte-oriented
}

//===== definition of interface declared in lock.h

void dn_lock() {
    // this sample Arduino code is single threaded, no need to lock.
}

void dn_unlock() {
    // this sample Arduino code is single threaded, no need to lock.
}

//===== definition of interface declared in endianness.h

void dn_write_uint16_t(uint8_t* ptr, uint16_t val) {
    // arduino Due is a little-endian platform
    ptr[0]     = (val>>8)  & 0xff;
    ptr[1]     = (val>>0)  & 0xff;
}

void dn_write_uint32_t(uint8_t* ptr, uint32_t val) {
    // arduino Due is a little-endian platform
    ptr[0]     = (val>>24) & 0xff;
    ptr[1]     = (val>>16) & 0xff;
    ptr[2]     = (val>>8)  & 0xff;
    ptr[3]     = (val>>0)  & 0xff;
}

void dn_read_uint16_t(uint16_t* to, uint8_t* from) {
    // arduino Due is a little endian platform
    *to        = 0;
    *to       |= (from[1]<<0);
    *to       |= (from[0]<<8);
}
void dn_read_uint32_t(uint32_t* to, uint8_t* from) {
    // arduino Due is a little endian platform
    *to        = 0;
    *to       |= (from[3]<<0);
    *to       |= (from[2]<<8);
    *to       |= (from[1]<<16);
    *to       |= (from[0]<<24);
}


//=========================== helpers =========================================

void printState(uint8_t state, bool manager) {
    if (manager) {
        switch (state) {
        case MOTE_STATE_IDLE:
            SerialPrintln("MOTE_STATE_IDLE");
            break;
        case MOTE_STATE_SEARCHING:
            SerialPrintln("MOTE_STATE_SEARCHING");
            break;
        case MOTE_STATE_NEGOCIATING:
            SerialPrintln("MOTE_STATE_NEGOCIATING");
            break;
        case MOTE_STATE_CONNECTED:
            SerialPrintln("MOTE_STATE_CONNECTED");;
            break;
        case MOTE_STATE_OPERATIONAL:
            SerialPrintln("MOTE_STATE_OPERATIONAL");
            break;
        default:
            SerialPrintln("<unknown>");
            break;
        }
    } else {
        switch (state) {
        case MANAGER_STATE_LOST:
            SerialPrintln("MOTE_STATE_LOST");
            break;
        case MANAGER_STATE_NEGOTIATING:
            SerialPrintln("MOTE_STATE_NEGOTIATING");
            break;
        case MANAGER_STATE_OPERATIONAL:
            SerialPrintln("MOTE_STATE_OPERATIONAL");
            break;
        default:
            SerialPrintln("<unknown>");
            break;
        }
    }
}

void printByteArray(uint8_t* payload, uint8_t length) {
    uint8_t i;

    SerialPrint(" ");
    for (i=0;i<length;i++) {
        SerialPrint(payload[i], HEX);
        if (i<length-1) {
            SerialPrint(".");
        }
    }
}
