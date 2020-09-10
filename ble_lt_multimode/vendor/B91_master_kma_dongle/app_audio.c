/********************************************************************************************************
 * @file	app_audio.c
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "application/audio/tl_audio.h"
#include "application/audio/audio_config.h"
#include "app_audio.h"

#if (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_TELINK)			//GATT Telink

u8		att_mic_rcvd = 0;
u32		tick_adpcm;
u8		buff_mic_adpcm[MIC_ADPCM_FRAME_SIZE];

u32		tick_iso_in;
int		mode_iso_in;



_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		DBG_CHN6_TOGGLE;

		abuf_dec_usb ();
	}

}

void	att_mic (u16 conn, u8 *p)
{
	att_mic_rcvd = 1;
	memcpy (buff_mic_adpcm, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)buff_mic_adpcm);
}

_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 200000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}

	//DBG_CHN5_TOGGLE;
	abuf_mic_dec ();
}

#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)			//GATT GOOGLE
u8		att_mic_rcvd = 0;
u32		tick_adpcm;
u8		buff_mic_adpcm[MIC_ADPCM_FRAME_SIZE];

u32		tick_iso_in;
int		mode_iso_in;

volatile u8 google_audio_start;


_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

void	att_mic (u16 conn, u8 *p)
{
	att_mic_rcvd = 1;
	memcpy (buff_mic_adpcm, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)buff_mic_adpcm);
}

void app_audio_data(u8 * data, u16 length)
{
	static u8 audio_buffer_serial;

	if(!google_audio_start)
	{
		return ;
	}
	if(audio_buffer_serial < 6 && length == 20)
	{
		memcpy(buff_mic_adpcm+audio_buffer_serial*20,data,length);
		audio_buffer_serial++;
	}
	else if(audio_buffer_serial==6 && length ==14)
	{
		memcpy(buff_mic_adpcm+audio_buffer_serial*20,data,length);
		abuf_mic_add ((u32 *)buff_mic_adpcm);
		audio_buffer_serial = 0;
	}
	else
	{
		audio_buffer_serial = 0;
	}

}

_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 200000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}
	abuf_mic_dec ();
}
#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID_DONGLE_TO_STB)		//HID Service,ADPCM,Dongle to STB,STB decode

//#include "application/usbstd/usbhw_i.h"
#include "application/usbstd/usbhw.h"

#define MIC_BUFF_NUM	32
#define MIC_DATA_LEN	20

unsigned char mic_dat_buff[MIC_BUFF_NUM][MIC_DATA_LEN];

u8 usb_mic_wptr= 0;
u8 usb_mic_rptr= 0;
u8 audio_id = 0;

_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{

	if (reg_usb_irq & BIT(7)) {

		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

	}

}

unsigned char usb_report_hid_mic(u8* data, u8 report_id)
{
	if(usbhw_is_ep_busy(USB_EDP_AUDIO_IN))
		return 0;

	reg_usb_ep_ptr(USB_EDP_AUDIO_IN) = 0;

	reg_usb_ep_dat(USB_EDP_AUDIO_IN) = report_id;
	foreach(i, 20){
		reg_usb_ep_dat(USB_EDP_AUDIO_IN) = data[i];
	}

	reg_usb_ep_ctrl(USB_EDP_AUDIO_IN) = FLD_EP_DAT_ACK;		// ACK
	return 1;
}

void mic_packet_reset(void)
{
	audio_id = 0;
	usb_mic_wptr = 0;
	usb_mic_rptr = 0;
}

void push_mic_packet(unsigned char *p)
{
	memcpy(mic_dat_buff[usb_mic_wptr], p, MIC_DATA_LEN);

	usb_mic_wptr =  (usb_mic_wptr+1)&(MIC_BUFF_NUM-1);
}

_attribute_ram_code_ void proc_audio (void)
{
	if(usb_mic_wptr != usb_mic_rptr)
	{
		static u32 cnt = 0;
		u8 *pdat = mic_dat_buff[usb_mic_rptr];
		if(usb_report_hid_mic(pdat, (audio_id + 10))==1)
		{
			audio_id += 1;
			audio_id = audio_id%3;
			cnt += 1;
			usb_mic_rptr =  (usb_mic_rptr+1)&(MIC_BUFF_NUM-1);
		}
	}
	return;
}

#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID)		//HID Service,ADPCM,Dongle decode

u8		att_mic_rcvd = 0;
u32		tick_adpcm;

u32		tick_iso_in;
int		mode_iso_in;


extern u8 tmp_mic_data[];
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

void	att_mic (u16 conn, u8 *p)
{
	memcpy (tmp_mic_data, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)tmp_mic_data);
}

_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 3*1000*1000))
	{
		tick_adpcm = clock_time ();
	}
	abuf_mic_dec ();
}
#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB)		//HID Service,Dongle to STB,STB decode
//#include "application/usbstd/usbhw_i.h"
#include "application/usbstd/usbhw.h"

#define MIC_BUFF_NUM	32
#define MIC_DATA_LEN	20

unsigned char mic_dat_buff[MIC_BUFF_NUM][MIC_DATA_LEN];

u8 usb_mic_wptr= 0;
u8 usb_mic_rptr= 0;
u8 audio_id = 0;

_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{

	if (reg_usb_irq & BIT(7)) {

		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

	}

}

unsigned char usb_report_hid_mic(u8* data, u8 report_id)
{
	if(usbhw_is_ep_busy(USB_EDP_AUDIO_IN))
		return 0;

	reg_usb_ep_ptr(USB_EDP_AUDIO_IN) = 0;

	reg_usb_ep_dat(USB_EDP_AUDIO_IN) = report_id;
	foreach(i, 20){
		reg_usb_ep_dat(USB_EDP_AUDIO_IN) = data[i];
	}

	reg_usb_ep_ctrl(USB_EDP_AUDIO_IN) = FLD_EP_DAT_ACK;		// ACK
	return 1;
}

void mic_packet_reset(void)
{
	audio_id = 0;
	usb_mic_wptr = 0;
	usb_mic_rptr = 0;
}

void push_mic_packet(unsigned char *p)
{
	memcpy(mic_dat_buff[usb_mic_wptr], p, MIC_DATA_LEN);

	usb_mic_wptr =  (usb_mic_wptr+1)&(MIC_BUFF_NUM-1);
}

_attribute_ram_code_ void proc_audio (void)
{
	if(usb_mic_wptr != usb_mic_rptr)
	{
		static u32 cnt = 0;
		u8 *pdat = mic_dat_buff[usb_mic_rptr];
		if(usb_report_hid_mic(pdat, (audio_id + 10))==1)
		{
			audio_id += 1;
			audio_id = audio_id%3;
			cnt += 1;
			usb_mic_rptr =  (usb_mic_rptr+1)&(MIC_BUFF_NUM-1);
		}
	}
	return;
}

#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID)			//HID Service,SBC,Dongle decode

u8		att_mic_rcvd = 0;
u32		tick_adpcm;

u32		tick_iso_in;
int		mode_iso_in;


extern u8 tmp_mic_data[];
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

void	att_mic (u16 conn, u8 *p)
{
	att_mic_rcvd = 1;
	memcpy (tmp_mic_data, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)tmp_mic_data);
}

_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 500*1000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}
	abuf_mic_dec ();
}
#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_MSBC_HID)				//HID Service, MSBC, Dongle decode

u8		att_mic_rcvd = 0;
u32		tick_adpcm;

u32		tick_iso_in;
int		mode_iso_in;


extern u8 tmp_mic_data[];
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

void	att_mic (u16 conn, u8 *p)
{
	att_mic_rcvd = 1;
	memcpy (tmp_mic_data, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)tmp_mic_data);
}

_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 500*1000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}
	abuf_mic_dec ();
}
#elif(TL_AUDIO_MODE == TL_AUDIO_DONGLE_OPUS_GATT_AMAZON)			//HID Service,SBC,Dongle decode

u8		att_mic_rcvd = 0;
u32		tick_adpcm;
u8		buff_mic_adpcm[MIC_ADPCM_FRAME_SIZE];

u32		tick_iso_in;
int		mode_iso_in;



_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		DBG_CHN6_TOGGLE;

		abuf_dec_usb ();
	}

}

void	att_mic (u16 conn, u8 *p)
{
	att_mic_rcvd = 1;
	tmemcpy (buff_mic_adpcm, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)buff_mic_adpcm);
}

_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 200000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}

	//DBG_CHN5_TOGGLE;
	abuf_mic_dec ();
}

#endif

