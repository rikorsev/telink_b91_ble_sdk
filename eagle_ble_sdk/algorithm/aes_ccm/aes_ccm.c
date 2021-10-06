/********************************************************************************************************
 * @file	aes_ccm.c
 *
 * @brief	This is the source file for BLE SDK
 *
 * @author	BLE GROUP
 * @date	2020.06
 *
 * @par		Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd.
 *			All rights reserved.
 *
 *          The information contained herein is confidential property of Telink
 *          Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *          of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *          Co., Ltd. and the licensee or the terms described here-in. This heading
 *          MUST NOT be removed from this file.
 *
 *          Licensee shall not delete, modify or alter (or permit any third party to delete, modify, or
 *          alter) any information contained herein in whole or in part except as expressly authorized
 *          by Telink semiconductor (shanghai) Co., Ltd. Otherwise, licensee shall be solely responsible
 *          for any claim to the extent arising out of or relating to such deletion(s), modification(s)
 *          or alteration(s).
 *
 *          Licensees are granted free, non-transferable use of the information in this
 *          file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/
#include "types.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble_common.h"
#include "stack/ble/ble_stack.h"
#include "stack/ble/ble_format.h"

#include "aes_ccm.h"

/******************************** Test case for HW AES (little or big --endian )*********************************************
//Refer to Core4.0 Spec <<BLUETOOTH SPECIFICATION Version 4.0 [Vol 6], Sample Data, Page2255
u8 KEY[16] = {0xBF, 0x01, 0xFB, 0x9D, 0x4E, 0xF3, 0xBC, 0x36, 0xD8, 0x74, 0xF5, 0x39, 0x41, 0x38, 0x68, 0x4C}; //(LSO to MSO)
u8 SKD[16] = {0x13, 0x02, 0xF1, 0xE0, 0xDF, 0xCE, 0xBD, 0xAC, 0x79, 0x68, 0x57, 0x46, 0x35, 0x24, 0x13, 0x02}; //(LSO to MSO)
u8 SK[16]  = {0x66, 0xC6, 0xC2, 0x27, 0x8E, 0x3B, 0x8E, 0x05, 0x3E, 0x7E, 0xA3, 0x26, 0x52, 0x1B, 0xAD, 0x99}; //(LSO to MSO)
u8 ASK[16] = { 0};

aes_ll_encryption(KEY, SKD, ASK); //little-endian

if(memcmp(ASK, SK, 16) == 0){
	printf("aes_ll_encryption: little-endian\n");
}

swapN(KEY, 16);
swapN(SKD, 16);
aes_encrypt(KEY, SKD, ASK); //big-endian
swapN(ASK, 16);

if(memcmp(ASK, SK, 16) == 0){
	printf("aes_encrypt: big-endian\n");
}

while(1);
******************************************************************************************************************************/

/**
 * @brief   	this function is used to encrypt the plaintext by hw aes
 * @param[in]	*key - aes key: 128 bit key for the encryption of the data, little--endian.
 * @param[in]	*plaintext - 128 bit data block that is requested to be encrypted, little--endian.
 * @param[out]	*encrypted_data - 128 bit encrypted data block, little--endian.
 * @return  	none.
 * @Note		Input data requires strict Word alignment
 */
void aes_ll_encryption(u8* key, u8* plaintext, u8 *encrypted_data)
{
    for (int i=0; i<4; i++){
    	reg_aes_key(i) = *(u32*)key;
    	aes_data_buff_ptr_get()[i] = *(u32*)plaintext;
    	key += 4;
    	plaintext += 4;
    }

    reg_aes_ptr = (u32)aes_data_buff_ptr_get();

    aes_set_mode((aes_mode_e)FLD_AES_START);      //cipher mode

    while(reg_aes_mode & FLD_AES_START){};

	u8 *ptr = (u8 *)&aes_data_buff_ptr_get()[4];
	for (int i=0; i<16; i++) {
		encrypted_data[i] = ptr[i];
	}
}


/////////////////////////////////////////////////////////////////////////////////////
//		AES_CCM
/////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief   	this function is used to set aes key for hw aes
 * @param[in]	*key - aes key: 128 bit key for the encryption of the data, big--endian.
 * @return  	none.
 * @Note		Input data requires strict Word alignment
 */
static inline void aes_init_key(u8 key[16])
{
	u8 *ptr = key + 12;
    for (int i=0; i<4; i++){
    	reg_aes_key(i) = (ptr[3]) | (ptr[2]<<8) | (ptr[1]<<16) | (ptr[0]<<24);
    	ptr -= 4;
    }
}

/**
 * @brief   	this function is used to set plaintext for hw aes
 * @param[in]	*key - aes key: 128 bit key for the encryption of the data, big--endian.
 * @return  	none.
 * @Note		Input data requires strict Word alignment
 */
static inline void aes_feed_data(u8 plaintext[16])
{
	u8 *ptr = plaintext + 12;
    for (int i=0; i<4; i++){
    	aes_data_buff_ptr_get()[i] = (ptr[3]) | (ptr[2]<<8) | (ptr[1]<<16) | (ptr[0]<<24);
    	ptr -= 4;
    }

    reg_aes_ptr = (u32)aes_data_buff_ptr_get();

    aes_set_mode((aes_mode_e)FLD_AES_START);      //cipher mode

    while(reg_aes_mode & FLD_AES_START){};
}

/**
 * @brief   	this function is used to get decrypted data by hw aes
 * @param[in]	*key - aes key: 128 bit key for the encryption of the data, big--endian.
 * @return  	none.
 * @Note		Input data requires strict Word alignment
 */
static inline void aes_read_result(u8 result[16])
{
	u8 *ptr = (u8 *)&aes_data_buff_ptr_get()[4];
	for (int i=0; i<16; i++) {
		result[i] = ptr[15 - i];
	}
}




/**
 * @brief   this function is used to calc the aes ccm value
 * @param   micLen - mic lenth (should be 4)
 * @param   iv - initial vector (should be 13 bytes nonce)
 * @param   mStr - plaint text
 * @param   mStrLen - plaint text length
 * @param   aStr -  a string  (should be AAD the data channel PDU header's first octet with NESN, SN and MD bits masked to 0)
 * @param   aStrLen - a atring lenth (should be 1)
 * @param   result - result (result)
 * @return  status l2cap_sts_t
 */
__attribute__((section(".ram_code"))) __attribute__((noinline))
static u8 aes_ccmAuthTran(u8 micLen, u8 *iv, u8 *mStr, u16 mStrLen, u8 *aStr, u16 aStrLen, u8 *result) 
{
	u16 msgLen;
	u8 mStrIndex = 0;
	ccm_flags_t flags;
	u16 i,j;

	if ( aStrLen + 2 > AES_BLOCK_SIZE ) {
		while(1);
	}
	//aes_enc_t *tmpPtr = (aes_enc_t*)ev_buf_allocate(sizeof(aes_enc_t));
	aes_enc_t *tmpPtr, encTmp;
	tmpPtr = &encTmp;
	smemset(tmpPtr, 0, sizeof(aes_enc_t));
	if ( tmpPtr == NULL ) {
		while(1);
	}
	/* get B0 */
	flags.bf.L = 1;  /* L-1 (15-nonceLen-1)*/
	flags.bf.M = (micLen - 2)>>1;
	flags.bf.aData = (aStrLen > 0) ? 1 : 0;
	flags.bf.reserved = 0;

	tmpPtr->bf.B[0] = flags.val;
	/* copy nonce N */
	smemcpy(tmpPtr->bf.B + 1, iv, 13);
	/* last byte is mStrlen */
	tmpPtr->bf.B[14] = mStrLen>>8;
	tmpPtr->bf.B[15] = mStrLen & 0xff;


	tmpPtr->newAstr[0] = aStrLen>>8;
	tmpPtr->newAstr[1] = aStrLen & 0xff;
	smemcpy(tmpPtr->newAstr + 2, aStr, aStrLen);
	aStrLen += 2;

	/* X0 is zero */
	smemset(tmpPtr->tmpResult, 0, AES_BLOCK_SIZE);

	msgLen = aStrLen;
	/* adjust msgLen according to aStrLen and mStrLen, should be 16x */
	if (aStrLen & 0x0f) {
		msgLen &= ~0x0F;
		msgLen += 0x10;
	}

	msgLen += mStrLen;
	if (mStrLen & 0x0f) {
		msgLen &= ~0x0F;
		msgLen += 0x10;
	}


	/* now the msgLen should be the length of AuthData, which is generated by AddAuthData (astring, padded by 0) || PlaintexeData (mString, padded by 0)*/
	for ( i=0; i<msgLen+16; i+=(1<<4) ) {
		for ( j=0; j<AES_BLOCK_SIZE; j++) {
			/* get Xi XOR Bi */
			tmpPtr->tmpResult[j] ^= tmpPtr->bf.B[j];
		}

		/* use aes to get E(key, Xi XOR Bi) */
		aes_feed_data(tmpPtr->tmpResult);
		aes_read_result(tmpPtr->tmpResult);
		/* update B */
		if ( aStrLen >= AES_BLOCK_SIZE ) {
			smemcpy(tmpPtr->bf.B, tmpPtr->newAstr + i, AES_BLOCK_SIZE);
			aStrLen -= AES_BLOCK_SIZE;
		} else if ( (aStrLen>0) && (aStrLen<AES_BLOCK_SIZE) ) {
			smemcpy(tmpPtr->bf.B, tmpPtr->newAstr + i, aStrLen);
			smemset(tmpPtr->bf.B + aStrLen, 0, AES_BLOCK_SIZE - aStrLen);
			aStrLen = 0;
			/* reset the mstring index */
			mStrIndex = 0;
		} else if ( mStrLen >= AES_BLOCK_SIZE ) {
			smemcpy(tmpPtr->bf.B, mStr + (mStrIndex*AES_BLOCK_SIZE), AES_BLOCK_SIZE);
			mStrIndex++;
			mStrLen -= AES_BLOCK_SIZE;
		} else {
			smemcpy(tmpPtr->bf.B, mStr + (mStrIndex*AES_BLOCK_SIZE), mStrLen);
			smemset(tmpPtr->bf.B + mStrLen, 0, AES_BLOCK_SIZE - mStrLen);
		}
	}
	smemcpy(result, tmpPtr->tmpResult, micLen);

	return 0;
}


/**
 * @brief   this function is used to calc the aes ccm value
 * @param   micLen - mic lenth (should be 4)
 * @param   iv - initial vector (should be 13 bytes nonce)
 * @param   mStr - plaint text
 * @param   mStrLen - plaint text length
 * @param   aStr -  a string  (should be AAD the data channel PDU header's first octet with NESN, SN and MD bits masked to 0)
 * @param   aStrLen - a atring lenth (should be 1)
 * @param   result - result (result)
 * @return  status l2cap_sts_t
 */
__attribute__((section(".ram_code"))) __attribute__((noinline))
static u8 aes_ccmBaseTran(u8 micLen, u8 *iv, u8 *mStr, u16 mStrLen, u8 *aStr, u8 aStrLen, u8 *mic)
{
	u16 msgLen;//fix, DLE length greater than 240Bytes, msgLen is 256
	u16 i;
	u8 j;
	u16 counter = 1;

	aes_enc_t *tmpPtr, encTmp;
	tmpPtr = &encTmp;

	/////////////// Block Ai format ////////////////
	tmpPtr->bf.A[0] = 1; //Flags: As per the CCM specification

	/* set the iv */
	smemcpy(tmpPtr->bf.A+1, iv, 13);

	tmpPtr->bf.A[14] = tmpPtr->bf.A[15] = 0;

	aes_feed_data(tmpPtr->bf.A);
	aes_read_result(tmpPtr->tmpResult);

	for ( i=0; i<micLen; i++ ) {
		mic[i] ^= tmpPtr->tmpResult[i];
	}

	tmpPtr->bf.A[14] = counter>>8;
	tmpPtr->bf.A[15] = counter & 0xff;


	msgLen = mStrLen;
	if (msgLen & 0x0f) {
		msgLen &= ~0x0F;
		msgLen += 0x10;
	}


	for ( i=0; i<msgLen; i+=(1<<4) ) {
		/* use aes to the E(key, Ai) */
		aes_feed_data(tmpPtr->bf.A);
		aes_read_result(tmpPtr->tmpResult);
	

		for ( j=0; (j<AES_BLOCK_SIZE) && (i+j < mStrLen); j++) {
			mStr[i+j] ^= tmpPtr->tmpResult[j];
		}

		/* update Ai */
		counter++;
		tmpPtr->bf.A[14] = counter>>8;
		tmpPtr->bf.A[15] = counter & 0xff;
	}

	return 0;
}


/**
 * @brief   	this function is used to initialize the aes_ccm initial value
 * @param[in]   ltk - encryption key, LTK, little--endian.
 * @param[in]   skdm -, little--endian.
 * @param[in]   skds -, little--endian.
 * @param[in]   ivm -, little--endian.
 * @param[in]   ivs -, little--endian.
 * @param[in]   pd - Reference structure ble_crypt_para_t
 * @return  	none
 */
void aes_ll_ccm_encryption_init (u8 *ltk, u8 *skdm, u8 *skds, u8 *ivm, u8 *ivs, ble_crypt_para_t *pd)
{
	u8 sk[16];
	smemcpy (sk, skdm, 8);
	smemcpy (sk+8, skds, 8);

	aes_ll_encryption (ltk, sk, sk);
	swapN(sk, 16); //switch to big--endian.

	aes_init_key(sk);

	smemcpy (pd->sk, sk, 16);
	smemcpy (pd->nonce.iv, ivm, 4);
	smemcpy (pd->nonce.iv + 4, ivs, 4);
	pd->enc_pno = 0;
	pd->dec_pno = 0;

}


/**
 * @brief   	this function is used to encrypt the aes_ccm value
 * @param[in]   pkt - plaint_text
 * @param[in]   master - ll_ccm_enc: Master role must use 1, Slave role must use 0;
                         ll_ccm_dec: Master role must use 0, Slave role must use 1;
 * @param[in]   pd - Reference structure ble_crypt_para_t
 * @return  	none
 */
void aes_ll_ccm_encryption(u8 *pkt, int master, ble_crypt_para_t *pd)
{
	u8 mic[4];
	u8 llid = pkt[0] & 3;
	u8 len = pkt[1];
	pkt[1] += 4;
	pkt += 2;
	pd->nonce.pkt = pd->enc_pno++;
	pd->nonce.dir = master ? 0x80 : 0;

	aes_init_key(pd->sk);
	aes_ccmAuthTran(4, (u8*)&pd->nonce, pkt, len, &llid, 1, mic);
	aes_ccmBaseTran(4, (u8*)&pd->nonce, pkt, len, &llid, 1, mic);
    smemcpy (pkt + len, mic, 4);
}


/**
 * @brief   	this function is used to decrypt the aes_ccm value
 * @param[in]   pkt - plaint_text
 * @param[in]   master - ll_ccm_enc: Master role must use 1, Slave role must use 0;
                         ll_ccm_dec: Master role must use 0, Slave role must use 1;
 * @param[in]   pd - Reference structure ble_crypt_para_t
 * @return  	0: decryption succeeded; 1: decryption failed
 */
__attribute__((section(".ram_code"))) __attribute__((noinline)) 
int aes_ll_ccm_decryption(u8 *pkt, int master, ble_crypt_para_t *pd)
{
	//found by yafei/qinghua.
	//situation: click phone BLUE icon to open/close blue frequently.it is possible that peer not encryption while we encryption.
	if(pkt[1] < 5){
		return 1; ///error, and response mic_fail to peer device
	}

	u32 r = 0;
	u32 chkMIC, rawMIC;
	pkt[1] -= 4;
	u8 llid = pkt[0] & 3;
	u8 len = pkt[1];
	pkt += 2;
	pd->nonce.pkt = pd->dec_pno++;
	pd->nonce.dir = master ? 0x80 : 0;
	aes_init_key(pd->sk);
	aes_ccmBaseTran(4, (u8*)&pd->nonce, pkt, len, &llid, 1, pkt + len);
    aes_ccmAuthTran(4, (u8*)&pd->nonce, pkt, len, &llid, 1, (u8*)&chkMIC);
    smemcpy((u8*)&rawMIC, pkt + len, 4);

    if(rawMIC != chkMIC){
    	r = 1;
    }

    return r;
}


#if (LL_FEATURE_ENABLE_ISO)

void aes_ll_ccm_encryption_v2(leCryptCtrl_t *pd)
{
	u8  enEncFlg = pd->cryptBitsInfo.enEncFlg;
	u8* pPduLen  = &pd->pllPhysChnPdu->llPduHdr.pduHdr.rf_len;
	u8  rawPduLen = *pPduLen;

	if(rawPduLen == 0 || !enEncFlg){ //rf_len must > 0 && encryption enable
		return;
	}

	*pPduLen += 4; //plus 4 byte mic length

	/*
	 * IgnoreBITS_ACL = 0x1C;     // NESN, SN and MD
	 * IgnoreBITS_CIS = 0x5C;     // NESN, SN, NPI and CIE
	 * IgnoreBITS_BIS = 0x3C;     // CSSN and CSTF
	 */
	u8 AAD;
	u8 mic[4];
	u8 nonceType = pd->cryptBitsInfo.noneType;
	u8 pduType = pd->pllPhysChnPdu->llPduHdr.pduHdr.type;
	u8* pPduPkt = pd->pllPhysChnPdu->llPayload;
	ble_cyrpt_nonce_t*	pCcmNonce = &pd->ccmNonce;

	u8 pktcntr[5] = {0}; //39-bit packet counter + 1bit dir
	smemcpy(pktcntr, (u8*)&pd->txPayloadCnt, 5);
	pCcmNonce->pkt = pd->txPayloadCnt & 0xffffffff;
	pCcmNonce->dir = (pktcntr[4] & 0x7F) | (pd->cryptBitsInfo.role ? BIT(7) : 0); //dir:bit7, pktcntr's 32~38bits:bit0~bit6

	if(nonceType == CRYPT_NONCE_TYPE_ACL){
		AAD = pduType & (~0x1c);
		pd->txPayloadCnt++;//For ACL connection, txPayloadCnt's value need ++ here
		pd->txPayloadCnt &= 0x7FFFFFFFFF;
	}
	else if(nonceType == CRYPT_NONCE_TYPE_CIS){
		AAD = pduType & (~0x5c);
		//Notice:Before input parameter, the txPayloadCnt's value use cisPayloadNum
	}
	else{ //BIS
		AAD = pduType & (~0x3c);
		//Notice:Before input parameter, the txPayloadCnt's value use bisPayloadNum
	}

	aes_init_key(pd->sk);
	aes_ccmAuthTran(4, (u8*)pCcmNonce, pPduPkt, rawPduLen, &AAD, 1, mic);
	aes_ccmBaseTran(4, (u8*)pCcmNonce, pPduPkt, rawPduLen, &AAD, 1, mic);
    smemcpy ((pPduPkt + rawPduLen), mic, 4);
}

int aes_ll_ccm_decryption_v2(leCryptCtrl_t *pd)
{
	u8  enEncFlg = pd->cryptBitsInfo.enEncFlg;
	u8* pPduLen  = &pd->pllPhysChnPdu->llPduHdr.pduHdr.rf_len;
	u8 r = 1;//dft:FAIL. 1: FAIL, 0: SUCC

	if(*pPduLen <= 4 || !enEncFlg){ //rf_len must > 4(mic's length 4, not empty pkt) && encryption enable
		return r;
	}

	*pPduLen -= 4; //minus 4 byte mic length
	u8 rawPduLen = *pPduLen;

	u8 AAD;
	u32 chkMIC, rawMIC;
	u8 nonceType = pd->cryptBitsInfo.noneType;
	u8 pduType = pd->pllPhysChnPdu->llPduHdr.pduHdr.type;
	u8* pPduPkt = pd->pllPhysChnPdu->llPayload;
	ble_cyrpt_nonce_t*	pCcmNonce = &pd->ccmNonce;

	u8 pktcntr[5] = {0}; //39-bit packet counter + 1bit dir
	smemcpy(pktcntr, (u8*)&pd->rxPayloadCnt, 5);
	pCcmNonce->pkt = pd->rxPayloadCnt & 0xffffffff;
	pCcmNonce->dir = (pktcntr[4] & 0x7F) | (pd->cryptBitsInfo.role ? BIT(7) : 0); //dir:bit7, pktcntr's 32~38bits:bit0~bit6

	/*
	 * IgnoreBITS_ACL = 0x1C;     // NESN, SN and MD
	 * IgnoreBITS_CIS = 0x5C;     // NESN, SN, NPI and CIE
	 * IgnoreBITS_BIS = 0x3C;     // CSSN and CSTF
	 */
	if(nonceType == CRYPT_NONCE_TYPE_ACL){
		AAD = pduType & (~0x1c);
		pd->rxPayloadCnt++;//For ACL connection, rxPayloadCnt's value need ++ here
		pd->rxPayloadCnt &= 0x7FFFFFFFFF;
	}
	else if(nonceType == CRYPT_NONCE_TYPE_CIS){
		AAD = pduType & (~0x5c);
		//Notice:Before input parameter, the rxPayloadCnt's value use cisPayloadNum
	}
	else{ //BIS
		AAD = pduType & (~0x3c);
		//Notice:Before input parameter, the rxPayloadCnt's value use cisPayloadNum
	}

	aes_init_key(pd->sk);
	aes_ccmBaseTran(4, (u8*)pCcmNonce, pPduPkt, rawPduLen, &AAD, 1, pPduPkt + rawPduLen);
    aes_ccmAuthTran(4, (u8*)pCcmNonce, pPduPkt, rawPduLen, &AAD, 1, (u8*)&chkMIC);
    smemcpy((u8*)&rawMIC, pPduPkt + rawPduLen, 4);

    if(rawMIC == chkMIC){
    	r = 0; //MIC check successfully
    }

    return r;
}

	#if (0)//Test code
		const u8 sk[16] = { 0xaf,0xf7,0x3c,0xb6,0x2f,0xb7,0x06,0xe7,0xd8,0x06,0xfd,0x54,0x2e,0x13,0xb7,0xd6 };
		const u8 iv[8]  = { 0x24,0xab,0xdc,0xba,0xe1,0x77,0xad,0x92 };
																									// mic value: 4byte
		u8 ARaw[] = { 0x02, 0x0B, 0x07, 0x00, 0x04, 0x00, 0x10, 0x0E, 0x00, 0xFF, 0xFF, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00 };
		u8 BRaw[] = { 0x02, 0x0B, 0x07, 0x00, 0x04, 0x00, 0x10, 0x0E, 0x00, 0xFF, 0xFF, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00 };

		ble_crypt_para_t Amy_blc_cyrpt_para;
		leCryptCtrl_t    Bmy_blc_cyrpt_para;

		Amy_blc_cyrpt_para.dec_pno = 65535;
		Amy_blc_cyrpt_para.enc_pno = 65535;
		smemcpy(Amy_blc_cyrpt_para.sk, sk, 16);
		smemcpy(Amy_blc_cyrpt_para.nonce.iv, iv, 8);
		Amy_blc_cyrpt_para.enable = 1;
		/*
		 * ll_ccm_enc: Master role must use 1, Slave role must use 0;
		 * ll_ccm_dec: Master role must use 0, Slave role must use 1;
		 */
		aes_ll_ccm_encryption(ARaw, 0, &Amy_blc_cyrpt_para);
		aes_ll_ccm_decryption(ARaw, 0, &Amy_blc_cyrpt_para);

		Bmy_blc_cyrpt_para.txPayloadCnt = 65535;
		Bmy_blc_cyrpt_para.rxPayloadCnt = 65535;
		smemcpy(Bmy_blc_cyrpt_para.sk, sk, 16);
		smemcpy(Bmy_blc_cyrpt_para.ccmNonce.iv, iv, 8);
		Bmy_blc_cyrpt_para.cryptBitsInfo.enEncFlg = 1;
		Bmy_blc_cyrpt_para.cryptBitsInfo.role = 0;
		Bmy_blc_cyrpt_para.cryptBitsInfo.noneType = CRYPT_NONCE_TYPE_ACL;
		Bmy_blc_cyrpt_para.pllPhysChnPdu = (llPhysChnPdu_t*)BRaw;
		aes_ll_ccm_encryption_v2(&Bmy_blc_cyrpt_para);
		aes_ll_ccm_decryption_v2(&Bmy_blc_cyrpt_para);

		if(memcmp(ARaw, BRaw, sizoeof(BRaw))){
			printf("TEST ERR\n");
		}
	#endif
#endif
