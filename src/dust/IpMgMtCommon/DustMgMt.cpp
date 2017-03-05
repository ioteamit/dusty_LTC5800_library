/*
 * DustMgMt.cpp
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
#include "DustMgMt.h"
#include "wiring_digital.h"
#include "wiring_constants.h"
#include "variant.h"

DustMgMt::DustMgMt(){

}

void DustMgMt::begin(char CtsPin){
	
	// by default the module  is not in homologation phase
	homologation = false;
	
    // works only with a valid Arduino Pin
    if (CtsPin>1) {
        // UART_TX_CTSn must be forced to LOW
        // UART_RX_CTSn, is not handle,
        pinMode(CtsPin, OUTPUT); //PIN_ANTENNA_CTS
        digitalWrite(CtsPin, LOW);
    }
}
