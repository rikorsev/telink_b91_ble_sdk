/********************************************************************************************************
 * @file     main.c 
 *
 * @brief    This is the source file for TLSR8258
 *
 * @author	 junwei.lu@telink-semi.com;
 * @date     May 8, 2018
 *
 * @par      Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd.
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
 * 			 1.initial release(DEC. 26 2018)
 *
 * @version  A001
 *
 *******************************************************************************************************/
#include "app_config.h"
#include "../../drivers.h"



#include "tl_common.h"
#include "../common/blt_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

extern void user_init_normal();
extern void main_loop (void);
extern void usb_init(void) ;


_attribute_ram_code_ void rf_irq_handler(void)
{
	DBG_CHN10_HIGH;

	log_event_irq(BLE_IRQ_DBG_EN, SLEV_irq_rf);

	irq_blt_sdk_handler ();


	plic_interrupt_complete(IRQ15_ZB_RT);

	DBG_CHN10_LOW;
}


_attribute_ram_code_ void stimer_irq_handler(void)
{
	DBG_CHN11_HIGH;

	log_event_irq(BLE_IRQ_DBG_EN, SLEV_irq_sysTimer);

	irq_blt_sdk_handler ();


	plic_interrupt_complete(IRQ1_SYSTIMER);  	//plic_interrupt_complete


	DBG_CHN11_LOW;
}
#if 0//usb setting
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{

	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////

	}

}

_attribute_ram_code_ void usb_endpoint_irq_handler(void)
{

	usb_endpoints_irq_handler ();

	plic_interrupt_complete(IRQ11_USB_ENDPOINT);  	//plic_interrupt_complete


}

#endif

/**
 * @brief		This is main function
 * @param[in]	none
 * @return      none
 */
int main (void)   //must on ramcode
{
	sys_init(LDO_MODE);

#if (CLOCK_SYS_CLOCK_HZ == 16000000)
	clock_init(PLL_CLK_192M, PAD_PLL_DIV, PLL_DIV12_TO_CCLK, CCLK_DIV1_TO_HCLK,  HCLK_DIV1_TO_PCLK, PLL_DIV4_TO_MSPI_CLK);
#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
	clock_init(PLL_CLK_192M, PAD_PLL_DIV, PLL_DIV8_TO_CCLK, CCLK_DIV1_TO_HCLK,  HCLK_DIV1_TO_PCLK, PLL_DIV4_TO_MSPI_CLK);
#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
	clock_init(PLL_CLK_192M, PAD_PLL_DIV, PLL_DIV6_TO_CCLK, CCLK_DIV1_TO_HCLK,  HCLK_DIV2_TO_PCLK, PLL_DIV4_TO_MSPI_CLK);
#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
	clock_init(PLL_CLK_192M, PAD_PLL_DIV, PLL_DIV4_TO_CCLK, CCLK_DIV1_TO_HCLK,  HCLK_DIV2_TO_PCLK, PLL_DIV4_TO_MSPI_CLK);
#elif (CLOCK_SYS_CLOCK_HZ == 64000000)
	clock_init(PLL_CLK_192M, PAD_PLL_DIV, PLL_DIV3_TO_CCLK, CCLK_DIV2_TO_HCLK,  HCLK_DIV2_TO_PCLK, PLL_DIV4_TO_MSPI_CLK);
#endif

	rf_drv_init(RF_MODE_BLE_1M);

	gpio_init(1);

	usb_init();

	user_init_normal();


	irq_enable();

	while (1) {
		main_loop ();
	}

	return 0;
}


