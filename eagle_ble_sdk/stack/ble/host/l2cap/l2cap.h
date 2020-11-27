/********************************************************************************************************
 * @file	l2cap.h
 *
 * @brief	This is the header file for BLE SDK
 *
 * @author	BLE GROUP
 * @date	2020.06
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef STACK_BLE_L2CAP_L2CAP_H_
#define STACK_BLE_L2CAP_L2CAP_H_




//RF_LEN_MAX:255: MIC(4)+LL_MAX_LEN(251) => LL_MAX_LEN:l2cap_len(2)+cid(2)+ATT_MTU_MAX(247).
//l2cap buffer max: dma(4)+header(2)+l2cap_len(2)+cid(2)+ATT_MTU_MAX(247).
#define	L2CAP_RX_BUFF_LEN_MAX			  260//257 = 10+247,align 4, here we use 260
//dma(4)+header(2)+l2cap_len(2)+cid(2)+Attribute_data[ATT_MTU]
#define	ATT_RX_MTU_SIZE_MAX		  		  (L2CAP_RX_BUFF_LEN_MAX - 10)



/**
 * @brief	Telink defined L2CAP connect update response callback
 */
typedef int (*l2cap_conn_update_rsp_callback_t) (u8 id, u16 result);




typedef enum{
	CONN_PARAM_UPDATE_ACCEPT = 0x0000,
	CONN_PARAM_UPDATE_REJECT = 0x0001,
}conn_para_up_rsp;




/**
 * @brief	This function is used to set connect request parameter for updating connect parameter
 * @param	min_interval - connect interval minimum
 * @param	max_interval - connect interval maximum
 * @param	latency - connect latency
 * @param	timeout - connect timeout
 * @return	none.
 */
void		bls_l2cap_requestConnParamUpdate (u16 min_interval, u16 max_interval, u16 latency, u16 timeout);  //Slave


/**
 * @brief	This function is used to set the minimal time for send connect parameter update request after connect created
 * @param	time_ms - the unit is millisecond
 * @return	none.
 */
void        bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(int time_ms);


/**
 * @brief	This function is used to register the function for handler L2CAP data
 * @param	*p - the pointer direct to blc_l2cap_handler
 * @return	none.
 */
void		blc_l2cap_register_handler (void *p);


/**
 * @brief	This function is used to handler L2CAP data
 * @param	connHandle - connection handle
 * @param	*p - the pointer of l2cap data
 * @return	0
 */
int 		blc_l2cap_packet_receive (u16 connHandle, u8 * p);


/**
 * @brief	This function is used to register the function to process L2CAP SIG CHANNEL packet
 * @param	*p - the pointer of l2cap data
 * @return	none.
 */
void 		blc_l2cap_reg_att_sig_hander(void *p);//signaling pkt proc


/**
 * @brief	This function is used to register the function to process L2CAP ATTRIBUTE PROCTOCOL packet
 * @param	*p - the pointer of l2cap data
 * @return	none.
 */
void 		blc_l2cap_reg_att_cli_hander(void *p);

/**
 * @brief	This function is used to send connect parameter update response
 * @param	connHandle - connection handle
 * @param	req_id - Request packet identifier
 * @param	result - connect parameter update result
 * @return	none.
 */
void  		blc_l2cap_SendConnParamUpdateResponse(u16 connHandle, u8 req_id, conn_para_up_rsp result);


/**
 * @brief	This function is used to register the callback function for response connect parameter update
 * @param	cb - the pointer of callback function
 * @return	none.
 */
void 		blc_l2cap_registerConnUpdateRspCb(l2cap_conn_update_rsp_callback_t cb);










#endif /* STACK_BLE_L2CAP_L2CAP_H_ */
