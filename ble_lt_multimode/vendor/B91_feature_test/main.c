/********************************************************************************************************
 * @file     main.c 
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
  * @date     May. 10, 2018
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
#include "app.h"
#include <tl_common.h>
#include "drivers.h"
#include "app_config.h"
#include "stack/ble/ble.h"
#include "vendor/common/blt_common.h"
#include "vendor/common/user_config.h"


extern my_fifo_t hci_rx_fifo;

extern void user_init_normal();
extern void user_init_deepRetn();

extern void main_loop (void);


_attribute_ram_code_ void rf_irq_handler(void)
{

	irq_blt_sdk_handler ();

	plic_interrupt_complete(IRQ15_ZB_RT);
}


_attribute_ram_code_ void stimer_irq_handler(void)
{

	irq_blt_sdk_handler ();

	plic_interrupt_complete(IRQ1_SYSTIMER);

}


_attribute_ram_code_ void uart0_irq_handler(void)
{
#if(FEATURE_TEST_MODE == TEST_BLE_PHY)
	app_phytest_irq_proc();
#endif
	plic_interrupt_complete(IRQ19_UART0);

}

_attribute_ram_code_ int main(void)
{
	sys_init(LDO_MODE);

	clock_init(PLL_CLK_192M, PAD_PLL_DIV, PLL_DIV8_TO_CCLK,CCLK_DIV1_TO_HCLK, HCLK_DIV1_TO_PCLK,CCLK_TO_MSPI_CLK);

	rf_drv_init(RF_MODE_BLE_1M);

	gpio_init(1);


	user_init_normal ();
	irq_enable();


	while(1)
	{
	#if(MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
	#endif
		main_loop ();
	}
	return 0;
}

