/********************************************************************************************************
 * @file	gl_audio.c
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
#include 	"../../tl_common.h"
#include 	"../../drivers.h"
#include	"audio_config.h"
#include	"adpcm.h"
#include 	"gl_audio.h"

//#include    "../../vendor/B91_feature/feature_audio/app_att.h"

#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)

#include 	"stack/ble/ble.h"

_attribute_data_retention_ u32 app_audio_timer = 0;
_attribute_data_retention_ u32 app_audio_waitack_timer = 0;

_attribute_data_retention_ u8 app_audio_key_flags = 0; 	//bit[7]:1-press,0-release;
													   //bit[0]:1-report press; bit[1]:1-report release
													  //bit[2]:1-report search for google
_attribute_data_retention_ u16 app_audio_sync_serial = 0;

_attribute_data_retention_ u8 htt_audio_model_enable_flags = 0;
_attribute_data_retention_ u8 on_request_audio_model_enable_flags = 0;
_attribute_data_retention_ u8 ptt_audio_model_enable_flags = 0;

_attribute_data_retention_ u8 htt_audio_model_key_press_flags = 0;
_attribute_data_retention_ u8 ptt_audio_model_key_press_flags = 0;

_attribute_data_retention_ u8 g_stream_id = 1;
_attribute_data_retention_ u8 app_audio_status = 0;


unsigned char app_audio_key_start(unsigned char isPress)
{
	u8 temp = 0;
	u8 sendBuff[20]  = {0};

	if(isPress)
	{
		if(htt_audio_model_enable_flags || ptt_audio_model_enable_flags)
		{
			// htt model


			//htt audio start
			sendBuff[0] = ATV_MIC_CHAR_RSP_OPEN;
			if(htt_audio_model_enable_flags )
			{
				htt_audio_model_key_press_flags = 1;
				sendBuff[1] = REASON_HTT; //reason :HTT Audio transfer is triggered by ��Assistant�� button press and will stop once the button is released.
			}
			else
			{
				//ptt model
				ptt_audio_model_key_press_flags = 1;
				sendBuff[1] = REASON_PTT; //reason :PTT Audio transfer is triggered by ��Assistant�� button press and will stop once the button is released.
			}

			sendBuff[2] = CS_16K16B; //codec used :ADPCM 16kHz/16bit
            if(g_stream_id > 0x80)
            {
            	g_stream_id = 1;
            }
			sendBuff[3] = g_stream_id; //streamid :1 - 0x80;

			g_stream_id ++;

			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 4))
			{
				return APP_AUDIO_PROCESS_ERR;
			}

			temp = APP_AUDIO_ENABLE;//enable audio

		}
		else if(on_request_audio_model_enable_flags)
		{
			//on request model
			if(app_audio_key_flags & APP_AUDIO_KEY_FLAG_PRESS) {
				return APP_AUDIO_PROCESS_ERR;
			}
			app_audio_key_flags |= APP_AUDIO_KEY_FLAG_PRESS;

			app_audio_waitack_timer = clock_time() | 1;

			app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_PRESS;

			if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_PRESS))
			{
				ble_sts_t ret;
				u16 search_key = 0x0221;
				ret = bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, (u8*)&search_key, 2);
				if(ret != BLE_SUCCESS) {
					return APP_AUDIO_PROCESS_ERR;
				}
				app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_PRESS;
				app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_RELEASE;

			}
			if(app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_RELEASE)
			{
				ble_sts_t ret;
				u16 consumerKey_release = 0x0000;
				ret = bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, (u8*)&consumerKey_release, 2);
				if(ret != BLE_SUCCESS) {
					return APP_AUDIO_PROCESS_ERR;
				}
				app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_RELEASE;
				app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_START;

			}

			if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_START))
			{
				ble_sts_t ret;
				u8 search = 0x08;
				ret = bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, &search, 1);
				if(ret != BLE_SUCCESS) {
					return APP_AUDIO_PROCESS_ERR;
				}
				app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_START;

			}
		}
		else
		{
			return APP_AUDIO_PROCESS_ERR;
		}

	}
	else
	{
		if(htt_audio_model_enable_flags)
		{
			htt_audio_model_key_press_flags = 0;
			//disable audio
			temp = APP_AUDIO_DISABLE;
			//send audio stop to  master
			ble_sts_t ret;
			sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
			sendBuff[1] = AS_RELEASE_BUTTON; //triggered by releasing an Assistant button during HTT interaction;
			ret = bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2);
			if(ret != BLE_SUCCESS) {
				return APP_AUDIO_PROCESS_ERR;
			}

		}
		else if(on_request_audio_model_enable_flags)
		{
			app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_PRESS;
		}
	}
	return temp;
}

extern void ui_enable_mic(int en);

int app_audio_timeout_proc(void){
	if(app_audio_timer != 0 && clock_time_exceed(app_audio_timer, APP_AUDIO_GOOGLE_TIMEOUT2))
	{
		ui_enable_mic(0);

		ble_sts_t ret;
		u8 sendBuff[2] = {0};
		sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
		sendBuff[1] = AS_TIMEOUT; //triggered by �� Audio Transfer Timeout ��;
		ret = bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2);
		if(ret != BLE_SUCCESS) {
			return 1;
		}
	}

	if(on_request_audio_model_enable_flags)
	{
		if(app_audio_waitack_timer != 0 && clock_time_exceed(app_audio_waitack_timer, APP_AUDIO_GOOGLE_TIMEOUT1))
		{
			app_audio_waitack_timer = 0;
			app_audio_key_flags = 0;
			ui_enable_mic(0);
			return	1;
		}
	}
	return 0;
}

int app_auido_google_callback(void* p)
{
	u8 sendBuff[20]  = {0};
	rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

	if(pw->dat[0] == AUDIO_GOOGLE_CMD_OPEN)
	{
		if(htt_audio_model_enable_flags)
		{
			//when htt model ongoing mic open comes send mic allready open error (0x0f01) and continue
			sendBuff[0] = ATV_MIC_CHAR_RSP_MIC_OPEN_ERROR;
			sendBuff[1] = MIC_OPEN_ERROR_HIGH;
			sendBuff[2] = MIC_ALREADY_OPEN_LOW;
			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 3))
			{
				return 0;
			}
		}
		else if(on_request_audio_model_enable_flags)
		{
			//on request model -- audio start
			sendBuff[0] = ATV_MIC_CHAR_RSP_OPEN;
			sendBuff[1] = REASON_MICOPEN; //reason :Audio transfer is triggered by MIC_OPEN request;
			sendBuff[2] = CS_16K16B; //codec used :ADPCM 16kHz/16bit
			sendBuff[3] = 0x00; //streamid = 0;
			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 4))
			{
				return 0;
			}

			ui_enable_mic(1);

			app_audio_waitack_timer = 0;

		}

	}
	else if(pw->dat[0] == AUDIO_GOOGLE_CMD_CLOSE)
	{
		u8 streamid = pw->dat[1];

		if(htt_audio_model_enable_flags || ptt_audio_model_enable_flags )
		{
			//htt model or ptt model
			if(streamid != 0)
			{
				ui_enable_mic(0);
				sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
				sendBuff[1] = AS_MICCLOSE; //triggered by MIC_CLOSE message;
				if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2))
				{
					return 0;
				}
			}
		}
		else
		{
			//on request model
			ui_enable_mic(0);
			sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
			sendBuff[1] = AS_MICCLOSE; //triggered by MIC_CLOSE message;
			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2))
			{
				return 0;
			}
		}
	}
	else if(pw->dat[0] == AUDIO_GOOGLE_CMD_CAP)
	{
		u8 model = pw->dat[5];

		if(ON_REQUEST == model)
		{
			//on request model
			on_request_audio_model_enable_flags = 1;
		}

		else if (PTT == model)
		{
			//htt model
			ptt_audio_model_enable_flags = 1;
		}

		else if (HTT == model)
		{
			//htt model
			htt_audio_model_enable_flags = 1;
		}
//		STB,Android o
//		on_request_audio_model_enable_flags = 1;
//		ptt_audio_model_enable_flags = 0;
//		htt_audio_model_enable_flags = 0;


		if(htt_audio_model_enable_flags || ptt_audio_model_enable_flags)
		{
			sendBuff[0] = ATV_MIC_CHAR_RSP_CAP;
			sendBuff[1] = VERSION_10_HIGH; // version high
			sendBuff[2] = VERSION_10_LOW; // version low 1.0
			sendBuff[3] = CS_16K16B; //codecs_supproted 16Khz(ADPCM,16bits)
			if(htt_audio_model_enable_flags)
			{
				sendBuff[4] = HTT; //assistant interaction model ON_Request(0x00)HTT(0x03)
			}
			else
			{
				sendBuff[4] = PTT; //assistant interaction model ON_Request(0x00)HTT(0x03)
			}

			sendBuff[5] = BYTE_20_HIGH; //frame lengths high
			sendBuff[6] = BYTE_20_LOW; //frame lengths low -- 20byte
			sendBuff[7] = EcReserved; // not used
			sendBuff[8] = 0x00; //not used
		}
		else
		{
			//on request
			sendBuff[0] = ATV_MIC_CHAR_RSP_CAP;
			sendBuff[1] = VERSION_04_HIGH; //major version
			sendBuff[2] = VERSION_04_LOW; //minor version
			sendBuff[3] = v4CS_BOTH_HIGH; //codecs_supproted high
			sendBuff[4] = v4CS_BOTH_LOW; //codecs_supproted low:  0x0001-8Khz(ADPCM,16bits), 0x0003-8K/16Khz(ADPCM,16bits)
			sendBuff[5] = v4BF_HIGH; //frame lengths high
			sendBuff[6] = v4BF_LOW; //frame lengths low -- 134Byts
			sendBuff[7] = BYTE_20_HIGH; //packet lengths high
			sendBuff[8] = BYTE_20_LOW; //packet lengths low -- 20 Bytes

		}

		if(bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 9))
		{
			return 0;
		}

	}
	else if(pw->dat[0] == AUDIO_GOOGLE_CMD_EXTEND)
	{
		//fresh the timeout
		app_audio_timer = clock_time() | 1;
	}

	return 0;

}


#else

#endif
