/********************************************************************************************************
 * @file	log.c
 *
 * @brief	This is the source file for B91
 *
 * @author	Ble Group
 * @date	2019
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *          
 *          Redistribution and use in source and binary forms, with or without
 *          modification, are permitted provided that the following conditions are met:
 *          
 *              1. Redistributions of source code must retain the above copyright
 *              notice, this list of conditions and the following disclaimer.
 *          
 *              2. Unless for usage inside a TELINK integrated circuit, redistributions 
 *              in binary form must reproduce the above copyright notice, this list of 
 *              conditions and the following disclaimer in the documentation and/or other
 *              materials provided with the distribution.
 *          
 *              3. Neither the name of TELINK, nor the names of its contributors may be 
 *              used to endorse or promote products derived from this software without 
 *              specific prior written permission.
 *          
 *              4. This software, with or without modification, must only be used with a
 *              TELINK integrated circuit. All other usages are subject to written permission
 *              from TELINK and different commercial license may apply.
 *
 *              5. Licensee shall be solely responsible for any claim to the extent arising out of or 
 *              relating to such deletion(s), modification(s) or alteration(s).
 *         
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *          DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 *          DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *         
 *******************************************************************************************************/
#include "../../../../tl_common.h"
#include "log.h"


#ifndef __LOG_RT_ENABLE__
#define __LOG_RT_ENABLE__		0
#endif


#if (__LOG_RT_ENABLE__)

#ifndef WIN32	
// the printf will mess the realtime log
// the realtime log use usb printer endpoint
//STATIC_ASSERT((!__DEBUG_PRINT__) && (USB_PRINTER_ENABLE));
#endif

#ifdef WIN32	
#include <stdio.h>
#include <io.h>
FILE *hFile = 0;
#endif

/*
	ID == -1 is invalid
	if you want to shut down logging a specified id,  assigne -1 to it
*/
#if LOG_IN_RAM
_attribute_ram_code_
#endif
static void log_write(int id, int type, u32 dat){
	if(-1 == id) return;
	u8 r = irq_disable();
	reg_usb_ep8_dat = (dat & 0xff);
	reg_usb_ep8_dat = ((dat >> 8) & 0xff);
	reg_usb_ep8_dat = ((dat >> 16)& 0xff);
	reg_usb_ep8_dat = (type)|(id);
	irq_restore(r);

#ifdef WIN32		// write to file directly
	if(!hFile){		// file not open yet
		foreach(i, 10){
			char fn[20];
			sprintf(fn, "vcd_%d.log", i);
			if(access(fn, 0)){		// file not exist
				hFile = fopen(fn, "w");
				assert(hFile);
				if(!hFile){
					while(1);
				}
				break;
			}
		}
		if(!hFile){
			hFile = fopen("vcd_0.log", "w");
		}
	}
	if(hFile){
		char temp[4];
		temp[0] = (dat & 0xff);
		temp[1] = ((dat >> 8) & 0xff);
		temp[2] = ((dat >> 16)& 0xff);
		temp[3] = (type)|(id);
		fwrite(&temp[0], 1, 4, hFile);
	}
#endif	
}

#if LOG_IN_RAM
_attribute_ram_code_
#endif
void log_task_begin(int id){
	log_write(id,LOG_MASK_BEGIN,clock_time());
}


#if LOG_IN_RAM
_attribute_ram_code_
#endif
void log_task_end(int id){
	log_write(id,LOG_MASK_END,clock_time());
}


#if LOG_IN_RAM
_attribute_ram_code_
#endif
void log_event(int id){
	log_write(id,LOG_MASK_TGL,clock_time());
}


#if LOG_IN_RAM
_attribute_ram_code_
#endif
void log_data(int id, u32 dat){
	log_write(id,LOG_MASK_DAT,dat);
}

#endif

