/*
 * IpMgMtWrapper.h
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

#ifndef IP_MG_MT_WRAPPER_H
#define IP_MG_MT_WRAPPER_H

#include "commonHelper.h"
#include "Arduino.h"
#include "commonDefines.h"
#include "../dataModel/DataModel.h"


#if defined (DUST_DEBUG) |  defined(DEBUG_ON_AIR)
#define SerialPrint SerialUSB.print
#define Serialwrite SerialUSB.write
#define SerialPrintln SerialUSB.println
#else
#define SerialPrint(args...) ((void)0)
#define SerialPrintln(args...) ((void)0)
#define Serialwrite(args...) ((void)0)

#endif
//=========================== typedef =========================================

/*
 *  The enumeration that defin ethe status of the command/Data
 * 
 *  The status of the command could be:
 *  CommandSent:   Just send the command
 *  Working:   The command has more than one answer, the received answer is one of the list
 *  Completed: This is the last answer of the command
 *  DataReceived:  The received message is for a Mote/Manager.
 */
typedef enum {
    Idle,
    CommandSent,
    DataSent,
    Working,
    Completed,
    DataReceived
} DustCbStatusE;

typedef struct {
    // fsm
    TIME_T               fsmPreviousEvent;
    TIME_T               fsmDelay;
    // module
    dn_uart_rxByte_cbt   ipmgmt_uart_rxByte_cb;
    uint8_t              replyBuf[MAX_FRAME_LENGTH];   // holds notifications from ipmt or ipmg

    // Class Data model constructor defined, for the manager only the buffer is used
    DataModel            *syncDataToSend;     

    DataModel            *asyncDataToSend;             // Asyncronous Class Data model
    DustCbStatusE        cbStatus;                     // Status of the internal FSM

} mg_mt_app_vars_t;

extern mg_mt_app_vars_t    app_vars;


class IpMgMtWrapper {
public :
    IpMgMtWrapper();
    virtual void begin(Uart *serial);

public:
    static Uart    *dustSerial;

public:    
    //=== helpers
    inline const uint8_t* getLastCommand(){return app_vars.replyBuf;};
    static inline Uart*   getInternalSerial(){return IpMgMtWrapper::dustSerial;};
    inline void           setMgStatus(DustCbStatusE newStatus) {app_vars.cbStatus = newStatus;};
	void resetDBGMsg(void);
protected:
    virtual void    executeReplyCb()=0;
    /*
     * The function read byte by byte the message and automatically invoke the 
     *      correct callBack according with the type of message received
     */
    TIME_T parseMessage(void);

private:
    bool readMessageExecuteCommand();


};

#endif
