/********************************************************************************************************
 * @file     string.h
 *
 * @brief    This is the source file for TLSR9518
 *
 * @author   BLE Group
 * @date     Jun 3, 2020
 *
 * @par      Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 * @par      History:
 *           1.initial release(Mar. 16, 2020)
 *
 * @version  A001
 *
 *******************************************************************************************************/
#ifndef COMMON_STRING_H_
#define COMMON_STRING_H_

#pragma once

void *  tmemset(void * d, int c, unsigned int  n);
void *  tmemcpy(void * des_ptr, const void * src_ptr, unsigned int);

// do not return void*,  otherwise, we must use a variable to store the dest porinter, that is not performance
void   	tmemcpy4(void * dest, const void * src, unsigned int);

int		tmemcmp(const void *_s1, const void *_s2, unsigned int _n);

#endif /* COMMON_STRING_H_ */
