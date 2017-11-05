/*
 * commonHelper.h
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
#ifndef COMMON_HELPER_H
#define COMMON_HELPER_H

#include "sm_clib/dn_uart.h"
#include "variant.h"
#include "commonDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

void dn_uart_init(dn_uart_rxByte_cbt rxByte_cb);

void dn_uart_txByte(uint8_t byte);

void dn_uart_txFlush();

//===== definition of interface declared in lock.h

void dn_lock() ;

void dn_unlock();
//===== definition of interface declared in endianness.h

void dn_write_uint16_t(uint8_t* ptr, uint16_t val);

void dn_write_uint32_t(uint8_t* ptr, uint32_t val);

void dn_read_uint16_t(uint16_t* to, uint8_t* from);
void dn_read_uint32_t(uint32_t* to, uint8_t* from);

#ifdef __cplusplus
}
#endif

#ifdef DEBUG_ON_AIR
extern uint8_t debugMsg[2000];
extern uint16_t rcvPtr;
extern uint16_t sendPtr;
#endif

    void debugOnAir(uint8_t byte, uint8_t dir);
    void printState(uint8_t state, bool manager);
    void printByteArray(uint8_t* payload, uint8_t length);
    
#endif
