#include "..\IpMtWrapper\IpMtWrapper.h"
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
#ifndef DUSTMGMT_H_
#define DUSTMGMT_H_

class DustMgMt {
	public:
		DustMgMt();
	
	protected:
		// attributes
		bool homologation;
		radioTestE txType;
		uint16_t chanMask;
		int8_t power;
	
		// methods
		void begin(char CtsPin);
	
		// Manager & Mote common method
	public:	
			inline void setHomologationData(bool homologation, radioTestE txType, uint16_t chanMask, int8_t power)
			{
				this->homologation = homologation;
				this->txType = txType;
				this->chanMask = chanMask;
				this->power = power;
			};	
};
#endif
