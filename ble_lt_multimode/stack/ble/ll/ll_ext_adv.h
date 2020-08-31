/********************************************************************************************************
 * @file     ll_ext_adv.h
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Feb. 1, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#ifndef LL_ADV_EXT_H_
#define LL_ADV_EXT_H_


#include <stack/ble/hci/hci_cmd.h>
#include <stack/ble/ll/ll_adv.h>
#include <stack/ble/ll/ll_stack.h>


/**
 * @brief	Primary channel advertising packet data buffer size
 */
#define 		MAX_LENGTH_PRIMARY_ADV_PKT						44   //sizeof(rf_pkt_pri_adv_t) = 43


/**
 * @brief	Secondary channel advertising packet data buffer size
 */
#define 		MAX_LENGTH_SECOND_ADV_PKT						264   //sizeof(rf_pkt_ext_adv_t) = 261





//NOTE: this data structure must 4 bytes aligned
typedef struct
{
    u8		adv_handle;
    u8 		extAdv_en;
    u8 		adv_chn_mask;
    u8		adv_chn_num;

	u8 		own_addr_type;
	u8 		peer_addr_type;
    u8 		pri_phy;
    u8 		sec_phy;


    u8 		max_ext_adv_evt;
    u8 		run_ext_adv_evt;
    u8		unfinish_advData;
    u8		unfinish_scanRsp;


	u8		adv_filterPolicy;
    u8 		scan_req_noti_en;
    u8 		coding_ind;					//s2 or s8
    u8		param_update_flag;


	u8		with_aux_adv_ind;   //ADV_EXT_IND  with AUX_ADV_IND
	u8		with_aux_chain_ind;
	u8 		rand_adr_flg;
    u8 		adv_sid;


    // u8 		s_adv_max_skip;


	u16     adv_did; 	// BIT<11:0>
	u16 	evt_props;
	u16		advInt_use;
	u16		send_dataLen;
    u16 	maxLen_advData;			//for each ADV sets, this value can be different to save SRAM
    u16 	curLen_advData;
    u16 	maxLen_scanRsp;			//for each ADV sets, this value can be different to save SRAM
    u16 	curLen_scanRsp;

    u16		send_dataLenBackup;
    u16		rsvd_16_1;


	u32 	adv_duration_tick;
	u32 	adv_begin_tick;				//24
    u32		adv_event_tick;

	u8*		dat_extAdv;
	u8*		dat_scanRsp;                //Scan response data.
	rf_pkt_pri_adv_t*		primary_adv;
	rf_pkt_ext_adv_t*		secondary_adv;

	u8 		rand_adr[6];
	u8 		peer_addr[6];
}ll_ext_adv_t;


#define ADV_SET_PARAM_LENGTH				(sizeof(ll_ext_adv_t))   //sizeof(ll_ext_adv_t) =  ,  must 4 byte aligned









/**
 * @brief      this function is used to initialize extended advertising module
 * @param[in]  *pAdvCtrl - advertising set control buffer address
 * @param[in]  *pPriAdv - Primary channel advertising packet data buffer address
 * @param[in]	num_sets - number of advertising set
 * @return     none
 */
void 		blc_ll_initExtendedAdvertising_module(	u8 *pAdvCtrl, u8 *pPriAdv,int num_sets);


/**
 * @brief      this function is used to initialize secondary channel advertising packet buffer
 * @param[in]  *pSecAdv - secondary channel advertising packet buffer address
 * @param[in]  sec_adv_buf_len - secondary channel advertising packet buffer length
 * @return     none
 */
void 		blc_ll_initExtSecondaryAdvPacketBuffer(u8 *pSecAdv, int sec_adv_buf_len);


/**
 * @brief      initialize Advertising Data buffer for all adv_set
 * @param[in]  pExtAdvData - extended advertising data buffer address
 * @param[in]  max_len_advData - extended advertising data buffer maximum length
 * @return     none
 */
void 		blc_ll_initExtAdvDataBuffer(u8 *pExtAdvData, int max_len_advData);


/**
 * @brief      initialize Scan Response Data Buffer for all adv_set
 * @param[in]  pScanRspData - extended scan response data buffer address
 * @param[in]  max_len_scanRspData - extended scan response data buffer maximum length
 * @return     none
 */
void 		blc_ll_initExtScanRspDataBuffer(u8 *pScanRspData, int max_len_scanRspData);



/**
 * @brief      This function is used to set the advertising parameters
 * @param[in]  advHandle - advertising handle
 * @param[in]  adv_evt_prop - advertising event property
 * @param[in]  pri_advIntervalMin - primary advertising channel interval maximum value
 * @param[in]  pri_advInter_max -   primary advertising channel interval minimum value
 * @param[in]  pri_advChnMap -  primary advertising channel map
 * @param[in]  ownAddrType - own address type
 * @param[in]  peerAddrType - peer address type
 * @param[in]  *peerAddr - peer address
 * @param[in]  advFilterPolicy - advertising filter policy
 * @param[in]  adv_tx_pow - advertising TX power
 * @param[in]  pri_adv_phy - primary advertising channel PHY type
 * @param[in]  sec_adv_max_skip - secondary advertising minimum skip number
 * @param[in]  sec_adv_phy - - primary advertising channel PHY type
 * @param[in]  adv_sid - advertising set id
 * @param[in]  scan_req_noti_en -scan response notify enable
 * @return     Status - 0x00: command succeeded;
						others: failed
 */
ble_sts_t 	blc_ll_setExtAdvParam(  adv_handle_t advHandle, 		advEvtProp_type_t adv_evt_prop, u32 pri_advIntervalMin, 		u32 pri_advIntervalMax,
									u8 pri_advChnMap,	 			own_addr_type_t ownAddrType, 	u8 peerAddrType, 			u8  *peerAddr,
									adv_fp_type_t advFilterPolicy,  tx_power_t adv_tx_pow,			le_phy_type_t pri_adv_phy, 	u8 sec_adv_max_skip,
									le_phy_type_t sec_adv_phy, 	 	u8 adv_sid, 					u8 scan_req_noti_en);



/**
 * @brief      This function is used to set the data used in advertising PDU that have a data field
 * @param[in]  advHandle - advertising handle
 * @param[in]  operation - Operation type
 * @param[in]  fragment_prefer -Fragment_Preference
 * @param[in]  advData_len - advertising data length
 * @param[in]  *advData - advertising data buffer address
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t	blc_ll_setExtAdvData	(u8 advHandle, data_oper_t operation, data_fragm_t fragment_prefer, u8 adv_dataLen, 	u8 *advdata);




/**
 * @brief      This function is used to provide scan response data used in scanning response PDUs.
 * @param[in]  advHandle - advertising handle
 * @param[in]  operation - Operation type
 * @param[in]  fragment_prefer -Fragment_Preference
 * @param[in]  scanRsp_dataLen - advertising scan response data length
 * @param[in]  *scanRspData - advertising scan response data buffer address
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t 	blc_ll_setExtScanRspData(u8 advHandle, data_oper_t operation, data_fragm_t fragment_prefer, u8 scanRsp_dataLen, u8 *scanRspData);


/**
 * @brief      This function is used to request the Controller to enable or disable one or more advertising sets using the
			   advertising sets identified by the adv_handle
 * @param[in]  extAdv_en -
 * @param[in]  advHandle - advertising handle
 * @param[in]  duration -	the duration for which that advertising set is enabled
 * 							Range: 0x0001 to 0xFFFF, Time = N * 10 ms, Time Range: 10 ms to 655,350 ms
 * @param[in]  max_extAdvEvt - Maximum number of extended advertising events the Controller shall
 *                             attempt to send prior to terminating the extended advertising
 * @return     Status - 0x00: command succeeded; 0x01-0xFF: command failed
 */
ble_sts_t 	blc_ll_setExtAdvEnable_1(u32 extAdv_en, u8 sets_num, u8 advHandle, 	 u16 duration, 	  u8 max_extAdvEvt);





/**
 * @brief      used to set default S2/S8 mode for Extended advertising if Coded PHY is used, this
 * @param[in]  advHandle - advertising handle
 * @param[in]  prefer_CI - LE coding indication prefer
 * @return     none
 */
void		blc_ll_setDefaultExtAdvCodingIndication(u8 advHandle, le_ci_prefer_t prefer_CI);



/**
 * @brief      this API is used to debug, setting one auxiliary data channel
 * @param[in]  aux_chn - auxiliary data channel, must be range of 0~36
 * @return     none
 */
void        blc_ll_setAuxAdvChnIdxByCustomers(u8 aux_chn);



/**
 * @brief      this API is used to debug, setting maximum advertising random delay
 * @param[in]  max_delay_ms - maximum advertising random delay, unit :mS, only  8/4/2/1/0  available
 * @return     none
 */
void		blc_ll_setMaxAdvDelay_for_AdvEvent(u8 max_delay_ms);





#endif /* LL_ADV_EXT_H_ */
