/********************************************************************************************************
 * @file	ext_pm.c
 *
 * @brief	This is the header file for B91
 *
 * @author	BLE Group
 * @date	2020
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
#include "nds_intrinsic.h"
#include "../analog.h"
#include "../dma.h"
#include "../gpio.h"
#include "../pm.h"
#include "../timer.h"
#include "../flash.h"
#include "../mdec.h"
#include "../trng.h"
#include "../sys.h"
#include "../plic.h"
#include "../stimer.h"
#include "../clock.h"
#include "../compatibility_pack/cmpt.h"
#include "ext_pm.h"


#if PM_TIM_RECOVER_MODE
_attribute_data_retention_ 	_attribute_aligned_(4) pm_tim_recover_t	pm_timRecover;
#endif

_attribute_data_retention_  _attribute_aligned_(4) misc_para_t 		blt_miscParam;

_attribute_data_retention_sec_	suspend_handler_t	func_before_suspend = 0;

cpu_pm_handler_t 		  	cpu_sleep_wakeup;  //no need retention,  cause it will be set every wake_up
pm_tim_recover_handler_t    pm_tim_recover;
check_32k_clk_handler_t   	pm_check_32k_clk_stable = 0;
pm_get_32k_clk_handler_t  	pm_get_32k_tick = 0;







void bls_pm_registerFuncBeforeSuspend (suspend_handler_t func )
{
	func_before_suspend = func;
}
/**
 * @brief   	This function serves to kick external crystal.
 * @param[in]   kick_ms - duration of kick.
 * @return  	none.
 */
#if 0
static _attribute_no_inline_ void pwm_kick_32k_pad(unsigned int kick_ms)
{


	write_reg8(0x14031e,0xfe);
	write_reg16(0x140418,0x01);
	write_reg16(0x14041a,0x02);
	write_reg8(0x140400,0x02);
	write_reg8(0x140402,0xb6);						//12M/(0xb6 + 1)/2 = 32k
	//wait for PWM wake up Xtal
	delay_ms(kick_ms*1000);


	write_reg8(0x14031e,0xff);
	write_reg16(0x140418,0x00);
	write_reg16(0x14041a,0x00);
	write_reg8(0x140400,0x00);
	write_reg8(0x140402,0x00);


	//Recover PD0 as Xtal pin
//	write_reg8(0x1401e8,0x02);//default
//	write_reg8(0x1401d8,0x00);
}
#endif
static _attribute_no_inline_ void pwm_kick_32k_pad_times(unsigned int times)
{
	if(times){

		//1. select 32k xtal
		analog_write_reg8(0x4e, 0x95);//32k select:[7]:0 sel 32k rc,1:32k XTAL

		#if 0 //must close, reason is as follows:
			/*
			 * This problem occurs with suspend and deep and deep retention. When the power supply voltage is low, suspend/deep/deep retention cannot
			 * be reset within 12ms, otherwise softstart will work. However, there is not enough delay to wait for softstart to complete. This delay
			 * will be postponed to the code execution area and it will not be able to handle larger code, otherwise it will be dropped by 1.8v, load
			 * error, and finally stuck.(Root: DCDC dly depends on the 32k rc clock, so the 32k rc power supply can't be turned off here.)
			 */
		analog_write_reg8(0x05, 0x01);//Power down 32KHz RC,  Power up [32KHz crystal, 24MHz RC, 24MHz XTAL,DCDC, VBUS_LDO, baseband pll LDO]
		#else
		analog_write_reg8(0x05, 0x00);//Power up 32KHz RC,  Power up 32KHz crystal
		#endif
		//condition: PCLK is 24MHZ,PCLK = HCLK
		write_reg8(0x1401d8,0x01);//PCLK = 12M
		write_reg8(0x1401e8,0x12);//24M crystal,cclk = pclk = 24Mhz
		delay_ms(1);


		int last_32k_tick;
		int curr_32k_tick;
		int i = 0;
		for(i = 0; i< times; i++){

			//After 10ms, the external 32k crystal clk is considered stable(when using PWM to accelerate the oscillation process)
			//A0 : GPIO_PD can't use for pwm kick. A1 :need check with lingyu,they will help to confirm kick function.
#if 0		//A1 need to do
			pwm_kick_32k_pad(10);//PWM kick external 32k pad (duration 10ms)
#else
			delay_ms(10*1000);
#endif
			//Check if 32k pad vibration and basically works stably
			last_32k_tick = pm_get_32k_tick();

			delay_us(305);//for 32k tick accumulator, tick period: 30.5us, dly 10 ticks

			curr_32k_tick = pm_get_32k_tick();

			if(last_32k_tick != curr_32k_tick){ //pwm kick 32k pad success
				break;
			}
		}

		#if (0) //blt_sdk_main_loop: check if 32k pad stable, if not, reboot MCU

			if(i >= times){
				analog_write_reg8(SYS_DEEP_ANA_REG, analog_read_reg8(SYS_DEEP_ANA_REG) & (~SYS_NEED_REINIT_EXT32K)); //clr
				start_reboot(); //reboot the MCU
			}
		#endif
	}
}


/**
 * @brief     this function servers to start reboot.
 * @param[in] none
 * @return    none
 */
_attribute_ram_code_ void start_reboot(void)
{
	if(blt_miscParam.pad32k_en){
		analog_write_reg8(SYS_DEEP_ANA_REG, analog_read_reg8(SYS_DEEP_ANA_REG) & (~SYS_NEED_REINIT_EXT32K)); //clear
	}

	#if (!WDT_REBOOT_RESET_ANA7F_WORK_AROUND)
	if(blt_miscParam.pad32k_en || blt_miscParam.pm_enter_en){
		analog_write_reg8(0x7f, 0x01);	// 0x7f reboot does not reset
	}
	#endif

	core_interrupt_disable ();
	REG_ADDR8(0x1401ef) = 0x20;  //reboot
	while (1);
}


/**
 * @brief   	This function serves to initiate the cpu after power on or deepsleep mode.
 * @param[in]   none.
 * @return  	none.
 */
static _attribute_no_inline_ void cpu_wakeup_no_deepretn_back_init(void)
{
#if (BLC_PM_EN)
	//Set 32k clk src: external 32k crystal, only need init when deep+pad wakeup or 1st power on
	if(blt_miscParam.pad32k_en){
#if 1
		//in this case: ext 32k clk was closed, here need re-init.
		//when deep_pad wakeup or first power on, it needs pwm acc 32k pad vibration time(dly 10ms)
		if(!(analog_read_reg8(SYS_DEEP_ANA_REG) & SYS_NEED_REINIT_EXT32K)){
			analog_write_reg8(SYS_DEEP_ANA_REG,  analog_read_reg8(SYS_DEEP_ANA_REG) | SYS_NEED_REINIT_EXT32K); //if initialized, the FLG is set to "1"

			pwm_kick_32k_pad_times(10);

		}
		else{
			delay_us(6000);
		}
#endif
	}
	else{
		//default 32k clk src: internal 32k rc, here can be optimized
		//analog_write(0x2d, 0x15); //32k select:[7]:0 sel 32k rc,1:32k pad
		//analog_write(0x05, 0x02); //Power down 32k crystal,  Power up [32KHz RC, 24MHz RC, 24MHz XTAL,DCDC, VBUS_LDO, baseband pll LDO]

		clock_cal_32k_rc();  //6.69 ms/6.7 ms
	}
#endif



#if 0
	//////////////////// get Efuse bit32~63 info ////////////////////
	unsigned int  efuse_32to63bit_info = pm_get_info1();

	/////////////  ADC vref calib //////////////////
	if((efuse_32to63bit_info & 0xff)){ //Efuse use new rule(efuse_bit38 and bit39 are all '1')
		if(adc_vref_cfg.adc_calib_en){
			unsigned char adc_ref_idx = efuse_32to63bit_info & 0xff; //ADC Ref: efuse bit32~bit36
			adc_vref_cfg.adc_vref = 1047 + adc_ref_idx ; //ADC ref voltage: g_adc_vref (unit: mV)
		}
	}
#endif

#if 1
	extern unsigned int	ota_program_offset;
	extern unsigned int	ota_program_bootAddr;
	//boot flag storage
	unsigned short boot_flag = read_reg16(0x140104);
	if (boot_flag)
	{
		ota_program_offset = 0;
	}
	else
	{
		ota_program_offset = ota_program_bootAddr;
	}
#endif
}
/**
 * @brief   	This function serves to initialize system.
 * @param[in]	power_mode - power mode(LDO/DCDC/LDO_DCDC)
 * @return  	none
 */
_attribute_ram_code_ void sys_init(power_mode_e power_mode,vbat_type_e vbat_v)
{
	reg_rst 	= 0xffbbffff;   //reset function will be cleared by set "1",which is different from the previous configuration
	reg_clk_en 	= 0xffbbffff;
	analog_write_reg8(0x8c,0x02);//<1>:reg_xo_en_clk_ana_ana=1
	analog_write_reg8(0x7d,0x80);////power on baseband

	write_reg32(0x160000,0x20000000);

	analog_write_reg8(0x0a, power_mode);  	//poweron_dft:	0x90.
											//<0-1>:pd_dcdc_ldo_sw,	default:00, dcdc & bypass ldo status bits.
											//		dcdc_1p4	dcdc_1p8	ldo_1p4		ldo_1p8
											//00:		N			N			Y			Y
											//01:		Y			N			N			Y
											//10:		Y			N			N			N
											//11:		Y			Y			N			N
	analog_write_reg8(0x01, 0x45);			//poweron_dft:	0x44 -> 0x45.
											//<0-2>:bbpll_ldo_trim,			default:100,->101 measured 1.186V.The default value is sometimes crashes.
											//<4-6>:ana_ldo_trim,1.0-1.4V	default:100,->100 analog LDO output voltage trim: 1.2V
	analog_write_reg8(0x0b, 0x38);  		//power on native 1P4 & 1P8
	analog_write_reg8(0x06,analog_read_reg8(0x06) & ~(BIT(0) |vbat_v | BIT(6) | BIT(7)));//poweron_dft:	0xff -> 0x36 or 0x3e.
											//<0>:pd_bbpll_ldo,		default:1,->0 Power on ana LDO.
											//<3>:pd_vbus_sw,		default:1,->0 Power up of bypass switch.
											//<6>:spd_ldo_pd,		default:1,->0 Power up spd ldo.
											//<7>:dig_ret_pd,		default:1,->0 Power up retention  ldo.
	//stimer
	analog_write_reg8(0x05,analog_read_reg8(0x05) & (~BIT(3)));   //<3>:24M_xtl_pd,		default:0,->0 Power up 24MHz XTL oscillator.
//	reg_system_ctrl	|= FLD_SYSTEM_32K_TRACK_EN | FLD_SYSTEM_TIMER_EN ;    //enable 32k cal and stimer
	//add by BLE
	reg_rf_bb_auto_ctrl = 0; //default 0x04

	write_csr(NDS_MILMB,0x01);
	write_csr(NDS_MDLMB,0x80001);

#if WDT_REBOOT_RESET_ANA7F_WORK_AROUND
	u8 analog_7f = analog_read_reg8(0x7f);
	u8 analog_38 = analog_read_reg8(PM_ANA_REG_WD_CLR_BUF0);
	u8 analog_39 = analog_read_reg8(PM_ANA_REG_POWER_ON_CLR_BUF0);
	if((analog_38 & BIT(0)) && ((analog_39 & BIT(0))==0)){
		g_pm_status_info.mcu_status = MCU_STATUS_POWER_ON;
		analog_write_reg8(PM_ANA_REG_WD_CLR_BUF0,analog_38 & 0xfe);
		analog_write_reg8(PM_ANA_REG_POWER_ON_CLR_BUF0,analog_39 | BIT(0));
	}else{
		if(analog_38 & BIT(0)){
			g_pm_status_info.mcu_status = MCU_STATUS_REBOOT_BACK;
			analog_write_reg8(PM_ANA_REG_WD_CLR_BUF0,analog_38 & 0xfe);
		}else{
			if( !(analog_7f & 0x01) ){
				g_pm_status_info.mcu_status = MCU_STATUS_DEEPRET_BACK;
			}else{
				g_pm_status_info.mcu_status = MCU_STATUS_DEEP_BACK;
			}
		}
	}
	analog_write_reg8(0x7f, analog_7f | 0x01);
#else
	if( !(analog_read_reg8(0x7f) & 0x01) ){
		g_pm_status_info.mcu_status = MCU_STATUS_DEEPRET_BACK;
	}
#endif
	g_pm_vbat_v = vbat_v>>3;
	//xo_ready check should be done after Xtal manual on_off, we put it here to save code running time, code running time between
	// Xtal manual on_off and xo_ready check can be used as Xtal be stable timimg
	while( BIT(7) != (analog_read_reg8(0x88) & (BIT(7)))); //0x88<7>: xo_ready_ana
	pm_wait_bbpll_done();

#if (BLC_PM_EN)

	if(g_pm_status_info.mcu_status == MCU_STATUS_DEEPRET_BACK){

		g_pm_status_info.wakeup_src = analog_read_reg8(0x64);
		if ( (analog_read_reg8(0x64) & WAKEUP_STATUS_TIMER_PAD ) == WAKEUP_STATUS_PAD)  //pad, no timer
		{
			g_pm_status_info.is_pad_wakeup = 1;
		}
		BM_CLR(reg_system_irq_mask,BIT(0));//disable 32k cal and stimer
#if SYS_TIMER_AUTO_MODE

		REG_ADDR8(0x140218) = 0x02;//sys tick 16M set upon next 32k posedge
		reg_system_ctrl		|=(FLD_SYSTEM_TIMER_AUTO|FLD_SYSTEM_32K_TRACK_EN) ;

		unsigned int deepRet_32k_tick = clock_get_32k_tick();

		unsigned int deepRet_tick = pm_tim_recover(deepRet_32k_tick + 1); // pm_tim_recover_32k_rc

		reg_system_tick = deepRet_tick + 1;

		//wait cmd set dly done upon next 32k posedge
		//if not using status bit, wait at least 1 32k cycle to set register r_run_upon_next_32k back to 0, or before next normal set
		while((reg_system_st & BIT(7)) == 0);

		REG_ADDR8(0x140218) = 0;//normal sys tick (16/sys) update
#else
		BM_CLR(reg_system_ctrl,FLD_SYSTEM_TIMER_EN | FLD_SYSTEM_TIMER_AUTO | FLD_SYSTEM_32K_TRACK_EN);//disable 32k cal and stimer

		unsigned int deepRet_32k_tick = clock_get_32k_tick();

		unsigned int deepRet_tick = pm_tim_recover(deepRet_32k_tick);
		reg_system_tick = deepRet_tick;
		reg_system_ctrl	|=(FLD_SYSTEM_TIMER_EN|FLD_SYSTEM_32K_TRACK_EN) ;
#endif
	}
	else
#endif
	{
#if SYS_TIMER_AUTO_MODE
		reg_system_ctrl		|=(FLD_SYSTEM_TIMER_AUTO|FLD_SYSTEM_32K_TRACK_EN) ;
		reg_system_tick 	= 0x01;//initial next tick is 1,kick system timer
#else
		reg_system_ctrl		|=(FLD_SYSTEM_TIMER_EN|FLD_SYSTEM_32K_TRACK_EN) ;
#endif
		cpu_wakeup_no_deepretn_back_init(); // to save ramcode

		clock_cal_24m_rc();
	}

	reg_system_irq_mask |= BIT(0);

	g_chip_version = read_reg8(0x1401fd);

	//if clock src is PAD or PLL, and hclk = (1/2)cclk, use reboot may cause problem, need deep to resolve(add by yi.bao, confirm by guangjun 20201016)
	if(g_pm_status_info.mcu_status == MCU_STATUS_REBOOT_BACK)
	{
		cpu_sleep_wakeup(DEEPSLEEP_MODE , PM_WAKEUP_TIMER, (clock_time () + 5*SYSTEM_TIMER_TICK_1MS));//TODO:deep time need to be validated
	}


//need check
//	//**When testing AES_demo, it was found that the timing of baseband was wrong when it was powered on, which caused some of
//	//the registers of ceva to go wrong, which caused the program to run abnormally.(add by weihua zhang, confirmed by junwen 20200819)
//	if(0xff == read_reg8(0x1401fd))
//	{
//		if(0x01 == (analog_read_reg8(0x7d)&0x01))
//		{
//			while(!read_reg32(0x140214));
//			analog_write_reg8(0x7d,0x80);	//power on baseband
//			cpu_sleep_wakeup(DEEPSLEEP_MODE , PM_WAKEUP_TIMER, (clock_time () + 200*SYSTEM_TIMER_TICK_1MS));
//		}
//	}
//	else
//	{
//		analog_write_reg8(0x7d,0x80);
//	}

	//add by BLE,Must be put at the end
	reg_embase_addr = 0xc0000000;//default is 0xc0200000;

}

