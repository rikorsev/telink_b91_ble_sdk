/********************************************************************************************************
 * @file     spi.c 
 *
 * @brief    This is the source file for TLSR8258
 *
 * @author	 peng.sun@telink-semi.com;
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
#include "spi.h"

#include "timer.h"
#include "../../common/assert.h"

u8 hspi_tx_dma_chn;
u8 hspi_rx_dma_chn;
u8 pspi_tx_dma_chn;
u8 pspi_rx_dma_chn;

dma_config_st hspi_tx_dma_config={
		.dst_req_sel= DMA_REQ_SPI_AHB_TX,//tx req
		.src_req_sel=0,
		.dst_addr_ctrl=DMA_ADDR_FIX,
		.src_addr_ctrl=DMA_ADDR_INCREMENT,//increment
		.dstmode=DMA_HANDSHAKE_MODE,//handshake
		.srcmode=DMA_NORMAL_MODE,
		.dstwidth=DMA_CTR_WORD_WIDTH,//must word
		.srcwidth=DMA_CTR_WORD_WIDTH,//must word
		.src_burst_size=0,//must 0
		.read_num_en=0,
		.priority=0,
		.write_num_en=0,
		.auto_en=0,//must 0
		};
 dma_config_st hspi_rx_dma_config={
	 .dst_req_sel= 0,//tx req
	.src_req_sel=DMA_REQ_SPI_AHB_RX,
	.dst_addr_ctrl=DMA_ADDR_INCREMENT,
	.src_addr_ctrl=DMA_ADDR_FIX,
	.dstmode=DMA_NORMAL_MODE,
	.srcmode=DMA_HANDSHAKE_MODE,
	.dstwidth=DMA_CTR_WORD_WIDTH,//must word
	.srcwidth=DMA_CTR_WORD_WIDTH,////must word
	.src_burst_size=0,
	.read_num_en=0,
	.priority=0,
	.write_num_en=0,
	.auto_en=0,//must 0
};

dma_config_st pspi_tx_dma_config={
	.dst_req_sel= DMA_REQ_SPI_APB_TX,//tx req
	.src_req_sel=0,
	.dst_addr_ctrl=DMA_ADDR_FIX,
	.src_addr_ctrl=DMA_ADDR_INCREMENT,//increment
	.dstmode=DMA_HANDSHAKE_MODE,//handshake
	.srcmode=DMA_NORMAL_MODE,
	.dstwidth=DMA_CTR_WORD_WIDTH,//must word
	.srcwidth=DMA_CTR_WORD_WIDTH,//must word
	.src_burst_size=0,//must 0
	.read_num_en=0,
	.priority=0,
	.write_num_en=0,
	.auto_en=0,//must 0
};

dma_config_st pspi_rx_dma_config={
	 .dst_req_sel= 0,//tx req
	.src_req_sel=DMA_REQ_SPI_APB_RX,
	.dst_addr_ctrl=DMA_ADDR_INCREMENT,
	.src_addr_ctrl=DMA_ADDR_FIX,
	.dstmode=DMA_NORMAL_MODE,
	.srcmode=DMA_HANDSHAKE_MODE,
	.dstwidth=DMA_CTR_WORD_WIDTH,//must word
	.srcwidth=DMA_CTR_WORD_WIDTH,////must word
	.src_burst_size=0,
	.read_num_en=0,
	.priority=0,
	.write_num_en=0,
	.auto_en=0,//must 0
};

/**
 * @brief     This function selects  pin  for hspi master or slave mode.
 * @param[in]  pin - the selected pin.
 * @return    none
 */
void hspi_set_pin_mux(hspi_pin_def_e pin)
{
	 if(pin!=HSPI_NONE_PIN )
   {
	 u8 val=0;
	 u8 start_bit = (BIT_LOW_BIT(pin & 0xff) %4 )<<1;
     u8 mask =(u8) ~BIT_RNG(start_bit , start_bit+1);

     if((pin==HSPI_SCLK_PB4_PIN)||(pin==HSPI_CS_PB6_PIN)||(pin==HSPI_SDO_PB3_PIN)||(pin==HSPI_SDI_PB2_PIN)||(pin==HSPI_WP_PB1_PIN)||(pin==HSPI_HOLD_PB0_PIN))
     {
    	 val = 0;//function 0
     }
     else if((pin==HSPI_SCLK_PA2_PIN)||(pin==HSPI_CS_PA1_PIN)||(pin==HSPI_SDO_PA4_PIN)||(pin==HSPI_SDI_PA3_PIN))
     {
      	 val = 2<<(start_bit);//function 2
      	reg_gpio_pad_mul_sel|=BIT(1);
     }
     reg_gpio_func_mux(pin)=(reg_gpio_func_mux(pin)& mask)|val;
     gpio_set_gpio_dis(pin);
     gpio_set_input_en(pin);
	}
}

/**
 * @brief     This function configures hspi pin.
 * @param[in] config - the pointer of pin config struct.
 * @return    none
 */
void hspi_set_pin(hspi_pin_config_st * config)
{
	//assert((config->hspi_cs_pin==HSPI_CS_PA1) || (config->hspi_cs_pin==HSPI_CS_PB6));
	hspi_set_pin_mux(config->hspi_cs_pin);
	hspi_set_pin_mux(config->hspi_sclk_pin);
	hspi_set_pin_mux(config->hspi_sdo_pin);
	hspi_set_pin_mux(config->hspi_sdi_pin);
	hspi_set_pin_mux(config->hspi_wp_pin);
	hspi_set_pin_mux(config->hspi_hold_pin);
}

/**
 * @brief     This function selects  pin  for pspi master or slave mode.
 * @param[in]  pin - the selected pin.
 * @return    none
 */
void pspi_set_pin_mux(pspi_pin_def_e pin)
{
if(pin!=PSPI_NONE_PIN )
{
	u8 val=0;
	u8 start_bit = (BIT_LOW_BIT(pin & 0xff) %4 )<<1;
	u8 mask =(u8) ~BIT_RNG(start_bit , start_bit+1);
	if((pin==PSPI_SCLK_PC5_PIN)||(pin==PSPI_CS_PC4_PIN)||(pin==PSPI_SDO_PC7_PIN)||(pin==PSPI_SDI_PC6_PIN))
	{
		val = 0;//function 0
	}

	else if((pin==PSPI_SCLK_PB5_PIN)||(pin==PSPI_SCLK_PD1_PIN)||(pin==PSPI_CS_PC0_PIN)||(pin==PSPI_CS_PD0_PIN)||(pin==PSPI_SDO_PB7_PIN)||(pin==PSPI_SDO_PD3_PIN)||(pin==PSPI_SDI_PB6_PIN)||(pin==PSPI_SDI_PD2_PIN))
	{
		val = 1<<(start_bit);//function 1
	}

	reg_gpio_func_mux(pin)=(reg_gpio_func_mux(pin)& mask)|val;
	gpio_set_gpio_dis(pin);
	gpio_set_input_en(pin);
	}

}
/**
 * @brief     This function configures hspi pin.
 * @param[in] config - the pointer of pin config struct.
 * @return    none
 */
void pspi_set_pin(pspi_pin_config_st * config)
{
	pspi_set_pin_mux(config->pspi_sclk_pin);
	pspi_set_pin_mux(config->pspi_cs_pin);
	pspi_set_pin_mux(config->pspi_sdo_pin);
	pspi_set_pin_mux(config->pspi_sdi_pin);

}

/**
 * @brief     This function set a group  pin for spi slave .
 * sclk=GPIO_PA2,cs=GPIO_PA1,sdo=GPIO_PA4,sdi=GPIO_PA3
 * @param[in]  none.
 * @return    none
 */
void spi_slave_set_pin(void)
{

	reg_gpio_pa_fuc_l= (reg_gpio_pb_fuc_l&0x03);//set PA1 as cs,PA2 as sclk,PA3 as sdi,
	reg_gpio_pa_fuc_h=(reg_gpio_pb_fuc_l&0xfc);//set PA4 slave sdo
	gpio_set_gpio_dis(GPIO_PA1|GPIO_PA2|GPIO_PA3|GPIO_PA4);
	gpio_set_input_en(GPIO_PA1|GPIO_PA2|GPIO_PA3|GPIO_PA4);
}

/**
 * @brief     This function configures the clock and working mode for SPI interface
 * @param[in] div_clock - the division factor for SPI module
 *            spi_clock_out = ahb_clock / ((div_clock+1)*2)
 * @param[in] mode - the selected working mode of SPI module
 *            bit5:CPHA-Clock Polarity  ; bit6:CPOL:CPHA-Clock Phase
 *            MODE0:  CPHA = 0 , CPOL =0;
 *            MODE1:  CPHA = 0 , CPOL =1;
 *            MODE2:  CPHA = 1 , CPOL =0;
 *            MODE3:  CPHA = 1,  CPOL =1;
 * @return    none
 */
void spi_master_init(spi_sel_e spi_sel,u8 div_clock, spi_mode_type_e mode)
{
	//todo reg_clk_en0 |= FLD_CLK0_SPI_EN;//enable hspi clock
	 reg_spi_mode1(spi_sel) = div_clock;
	 reg_spi_mode0(spi_sel)	|= FLD_SPI_MASTER_MODE;//master
	 reg_spi_mode0(spi_sel)	&= (~FLD_SPI_MODE_WORK_MODE); // clear spi working mode
	 reg_spi_mode0(spi_sel) |= (mode<<5);// select SPI mode,support four modes
}


/**
 * @brief     This function configures the clock and working mode for SPI interface
 * @param[in] mode - the selected working mode of SPI module
 *            bit5:CPHA-Clock Polarity  ; bit6:CPOL:CPHA-Clock Phase
 *            MODE0:  CPHA = 0 , CPOL =0;
 *            MODE1:  CPHA = 0 , CPOL =1;
 *            MODE2:  CPHA = 1 , CPOL =0;
 *            MODE3:  CPHA = 1,  CPOL =1;
 * note  spi_clock_in �� (spi_slave_clock frequency)/3
 */

void spi_slave_init(spi_sel_e spi_sel,spi_mode_type_e mode)
{
	 //todo reg_clk_en0 |= FLD_CLK0_SPI_EN;//enable hspi clock
	 reg_spi_mode0(spi_sel)	&= (~FLD_SPI_MASTER_MODE);//slave
	 reg_spi_mode0(spi_sel)	&= (~FLD_SPI_MODE_WORK_MODE); // clear spi working mode
	 reg_spi_mode0(spi_sel) |= (mode<<5);// select SPI mode,support four modes
}

/**
 * @brief     This function servers to set dummy cycle cnt
 * @param[in] dummy_cnt: the cnt of dummy clock.
 * @return    none
 */
void spi_set_dummy_cnt(spi_sel_e spi_sel,u8 dummy_cnt)
{
	reg_spi_trans0(spi_sel) &=(~FLD_SPI_DUMMY_CNT);
	reg_spi_trans0(spi_sel) |=(dummy_cnt-1)&FLD_SPI_DUMMY_CNT;
}

/**
 * @brief     This function servers to set spi transfer mode��
 * @param[in] spi_tans_mode_e:
 * @return    none
 */
void spi_set_transmode(spi_sel_e spi_sel,spi_tans_mode_e mode)
{
	reg_spi_trans0(spi_sel) &=(~FLD_SPI_TRANSMODE);
	reg_spi_trans0(spi_sel) |=(mode&0xf)<<4;
}


/**
 * @brief     This function servers to set normal mode��
 * @param[in] none
 * @return    none
 */
void spi_set_normal_mode(spi_sel_e spi_sel)
{
	spi_dual_mode_dis(spi_sel);
	spi_3line_mode_dis(spi_sel);
	if(HSPI_MODULE == spi_sel)
	{
		hspi_quad_mode_dis(spi_sel);
	}

}


/**
 * @brief     This function servers to set dual mode��
 * @param[in] none
 * @return    none
 */
void spi_set_dual_mode(spi_sel_e spi_sel)
{
	spi_dual_mode_en(spi_sel);//quad  precede over dual
	spi_3line_mode_dis(spi_sel);
	if(HSPI_MODULE == spi_sel)
	{
		hspi_quad_mode_dis(spi_sel);
	}

}


/**
 * @brief     This function servers to set quad mode��
 * @param[in] none
 * @return    none
 */
void hspi_set_quad_mode()
{
	hspi_quad_mode_en();
	spi_dual_mode_dis(HSPI_MODULE);
	spi_3line_mode_dis(HSPI_MODULE);

}

/**
 * @brief     This function servers to set 3line mode��
 * @param[in] none
 * @return    none
 */
void spi_set_3line_mode(spi_sel_e spi_sel)
{
	/*must disable dual and quad*/
	spi_3line_mode_en(spi_sel);
	spi_dual_mode_dis(spi_sel);
	if(HSPI_MODULE == spi_sel)
	{
		hspi_quad_mode_dis(spi_sel);
	}
}

/**
 * @brief     This function servers to set hspi io  mode��
 * @param[in] hspi_io_mode_e: single/dual/quad /3line.
 * @return    nonee
  */
void spi_set_io_mode(spi_sel_e spi_sel,spi_io_mode_e mode)
{
	//assert(!((PSPI==spi_sel) && (HSPI_QUAD_MODE == mode)));//TODO
	switch(mode)
	{
		case SPI_SINGLE_MODE :
			spi_set_normal_mode(spi_sel);
			break;
		case SPI_DUAL_MODE:
			spi_set_dual_mode(spi_sel);
			break;
		case HSPI_QUAD_MODE:
			hspi_set_quad_mode();
			break;
		case SPI_3_LINE_MODE:
			spi_set_3line_mode(spi_sel);
			break;
	}
}



/**
 * @brief     This function servers to config normal mode��
 * @param[in] mode: nomal ,mode 3line
 * @return    none
 */
void spi_master_config(spi_sel_e spi_sel,spi_nomal_3line_mode_e mode )
{
	 spi_cmd_dis(spi_sel);
	 if(HSPI_MODULE==spi_sel)
	 {
		 hspi_addr_dis();
	 }
	 spi_set_io_mode(spi_sel,mode);
}

/**
 * @brief     This function servers to special mode��
 * @param[in] config: the pointer of pin special config struct.
 * @return    none
 */
void hspi_master_config_plus(hspi_config_st *config)
{
	spi_set_io_mode(HSPI_MODULE,config->hspi_io_mode);
	hspi_set_addr_len(config->hspi_addr_len);
	spi_set_dummy_cnt(HSPI_MODULE,config->hspi_dummy_cnt);

	if(true==config->hspi_cmd_en)
	{
		spi_cmd_en(HSPI_MODULE);
	}
	else if(false==config->hspi_cmd_en)
	{
		spi_cmd_dis(HSPI_MODULE);
	}

	if(true==config->hspi_cmd_fmt_en)
	{
		hspi_cmd_fmt_en();
	}
	else if(false==config->hspi_cmd_fmt_en)
	{
	   hspi_cmd_fmt_dis();
	}

	if(true==config->hspi_addr_en)
	{
		hspi_addr_en();
	}
	else if(false==config->hspi_addr_en)
	{
		hspi_addr_dis();
	}

	if(true==config->hspi_addr_fmt_en)
	{
		hspi_addr_fmt_en();
	}
	else if(false==config->hspi_addr_fmt_en)
	{
		hspi_addr_fmt_dis();
	}
}


/**
 * @brief     This function servers to special mode��
 * @param[in] config: the pointer of pin special config struct.
 * @return    none
 */
void pspi_master_config_plus(pspi_config_st *config)
{
	spi_set_io_mode(PSPI_MODULE,config->pspi_io_mode);
	spi_set_dummy_cnt(PSPI_MODULE,config->pspi_dummy_cnt);
	if(true==config->pspi_cmd_en)
	{
		spi_cmd_en(PSPI_MODULE);
	}
	else if(false==config->pspi_cmd_en)
	{
		spi_cmd_dis(PSPI_MODULE);
	}

}


/**
 * @brief     This function servers to set slave address hspi only ��
 * @param[in] addr:address of slave
 * @return    none
 */
void hspi_set_address(u32 addr)
{
	reg_hspi_addr_32=addr;
}
/**
 * @brief     This function servers to write hspi fifo��
 * @param[in] data: the pointer to the data for write.
 * @param[in] len: write length.
 * @return    none
 */
void spi_write(spi_sel_e spi_sel,u8 *data, u32 len)
{
	for(u32 i=0;i<len;i++)
	{
		while (reg_spi_fifo_state(spi_sel)&FLD_SPI_TXF_FULL );
		reg_spi_wr_rd_data(spi_sel,i%4)=data[i];
	}
}


/**
 * @brief     This function servers to read hspi fifo��
 * @param[in] data: the pointer to the data for read.
 * @param[in] len: write length.
 * @return    none
 */
void spi_read(spi_sel_e spi_sel,u8 *data, u32 len)
{
	for(u32 i=0;i<len;i++)
	{
	    while (reg_spi_fifo_state(spi_sel)&FLD_SPI_RXF_EMPTY);
	    data[i]=reg_spi_wr_rd_data(spi_sel,i%4);
	}
}

/**
 * @brief     This function serves to normal write data in normal.
 * @param[in] data:  the pointer to the data for write.
 * @param[in] len: write length.
 * @return    none
 */
void spi_master_write(spi_sel_e spi_sel,u8 *data, u32 len)
{
	  spi_tx_fifo_clr(spi_sel);
	  spi_rx_fifo_clr(spi_sel);
	  spi_tx_cnt(spi_sel,len);
	  spi_set_transmode(spi_sel,HSPI_MODE_WRITE_ONLY);
	  spi_set_cmd(spi_sel,0x00);//when  cmd  disable that  will not sent cmd ,just trigger spi send .
	  spi_write(spi_sel,(u8 *)data, len);
	  while (spi_is_busy(spi_sel));
}


/**
 * @brief     This function serves to normal write and read data in normal mode.
 * @param[in] write: the pointer to the data for write.
 * @param[in] wr_len:write length.
 * @param[in] rd_data:the pointer to the data for read.
 * @param[in] rd_len:read length.
 * @return    none
 */
void spi_master_write_read(spi_sel_e spi_sel,u8 *wr_data, u32 wr_len,u8 *rd_data, u32 rd_len)
{
	    spi_tx_fifo_clr(spi_sel);
	    spi_rx_fifo_clr(spi_sel);
        spi_tx_cnt(spi_sel,wr_len);
        spi_rx_cnt(spi_sel,rd_len);
        spi_set_transmode(spi_sel,HSPI_MODE_WRITE_READ);
        spi_set_cmd(spi_sel,0x00);//when  cmd  disable that  will not sent cmd ,just trigger spi send .
        spi_write(spi_sel,(u8 *)wr_data, wr_len);
        spi_read(spi_sel,(u8 *)rd_data, rd_len);
        while (spi_is_busy(spi_sel));
}



/**
 * @brief      This function serves to single/dual/quad write to the SPI slave
 * @param[in]  cmd:cmd one byte will first write.
 * @param[in]  addr:the address of slave
 * @param[in]  addr_len:the length of address
 * @param[in]  data: pointer to the data need to write
 * @param[in]  data_len:length in byte of the data need to write
 * @param[in]  wr_mode: write mode  dummy or not dummy
 * @return     none
 */
void spi_master_write_plus(spi_sel_e spi_sel,u8 cmd, u32 addr,u8 *data, u32 data_len, spi_wr_tans_mode_e wr_mode)
{
	 spi_tx_fifo_clr(spi_sel);
	 spi_rx_fifo_clr(spi_sel);
	 if(HSPI_MODULE==spi_sel)
	 {
		 hspi_set_address(addr);
	 }
	 spi_set_transmode(spi_sel,wr_mode);
	 spi_tx_cnt(spi_sel,data_len);
	 spi_set_cmd(spi_sel,cmd);
	 spi_write(spi_sel,(u8 *)data, data_len);
	 while (spi_is_busy(spi_sel));
}

/**
 * @brief      This function serves to single/dual/quad  read from the SPI slave
 * @param[in]  cmd:cmd one byte will first write.
 * @param[in]  addr:the address of slave
 * @param[in]  addr_len:the length of address
 * @param[in]  data: pointer to the data need to read
 * @param[in]  rd_mode: read mode.dummy or not dummy
 * @return     none
 */
void spi_master_read_plus(spi_sel_e spi_sel,u8 cmd, u32 addr,u8 *data, u32 data_len,spi_rd_tans_mode_e rd_mode)
{
	 spi_tx_fifo_clr(spi_sel);
	 spi_rx_fifo_clr(spi_sel);
	 if(HSPI_MODULE==spi_sel)
	 {
		   hspi_set_address(addr);
	 }
	 spi_set_transmode(spi_sel, rd_mode);
	 spi_rx_cnt(spi_sel,data_len);
	 spi_set_cmd(spi_sel,cmd);
	 spi_read(spi_sel,(u8 *)data,data_len);
	 while (spi_is_busy(spi_sel));
}

/**
 * @brief     This function serves to set tx_dam channel and config dma tx default.
 * @param[in] dma_chn_e: dma channel.
 * @return    none
 */
void hspi_set_tx_dma_config(dma_chn_e chn)
{
	hspi_tx_dma_chn=chn;
	dma_config(chn,&hspi_tx_dma_config);

}
/**
 * @brief     This function serves to set rx_dam channel and config dma rx default.
 * @param[in] dma_chn_e: dma channel.
 * @return    none
 */
void hspi_set_rx_dma_config(dma_chn_e chn)
{
	 hspi_rx_dma_chn=chn;
	dma_config(chn,&hspi_rx_dma_config);
}
/**
 * @brief     This function serves to set tx_dam channel and config dma tx default.
 * @param[in] dma_chn_e: dma channel.
 * @return    none
 */
void pspi_set_tx_dma_config(dma_chn_e chn)
{
	pspi_tx_dma_chn=chn;
	dma_config(chn,&pspi_tx_dma_config);

}
/**
 * @brief     This function serves to set rx_dam channel and config dma rx default.
 * @param[in] dma_chn_e: dma channel.
 * @return    none
 */
void pspi_set_rx_dma_config(dma_chn_e chn)
{
	 pspi_rx_dma_chn=chn;
	 dma_config(chn,&pspi_rx_dma_config);
}


/**
 * @brief     This function serves to normal write data by dma.
 * @param[in] data:  the pointer to the data for write.
 * @param[in] len: write length.
 * @return    none
 */
void spi_master_write_dma(spi_sel_e spi_sel,u8 *src_addr, u32 len)
{
	u8 tx_dma_chn;
	spi_tx_fifo_clr(spi_sel);
	spi_rx_fifo_clr(spi_sel);
	spi_tx_dma_en(spi_sel);
	spi_tx_cnt(spi_sel,len);
	spi_set_transmode(spi_sel,HSPI_MODE_WRITE_ONLY);
	if(HSPI_MODULE == spi_sel)
	{
		tx_dma_chn = hspi_tx_dma_chn;
	}
	else
	{
		tx_dma_chn = pspi_tx_dma_chn;
	}
    dma_set_address(tx_dma_chn,(u32)reg_dma_addr(src_addr),reg_spi_data_buf_adr(spi_sel));
	dma_set_size(tx_dma_chn,len,DMA_WORD_WIDTH);
	dma_chn_en(tx_dma_chn);
	spi_set_cmd(spi_sel,0x00);
}


/**
 * @brief     This function serves to normal write and read data by dma.
 * @param[in] write: the pointer to the data for write.
 * @param[in] wr_len:write length.
 * @param[in] rd_data:the pointer to the data for read.
 * @param[in] rd_len:read length.
 * @return    none
 */
void spi_master_write_read_dma(spi_sel_e spi_sel,u8 *src_addr, u32 wr_len,u8 * dst_addr, u32 rd_len)
{
	u8 tx_dma_chn,rx_dma_chn;
	spi_tx_fifo_clr(spi_sel);
	spi_rx_fifo_clr(spi_sel);
	spi_tx_dma_en(spi_sel);
	spi_rx_dma_en(spi_sel);
	spi_tx_cnt(spi_sel,wr_len);
	spi_rx_cnt(spi_sel,rd_len);
	spi_set_transmode(spi_sel,HSPI_MODE_WRITE_READ);
	if(HSPI_MODULE == spi_sel)
	{
		tx_dma_chn = hspi_tx_dma_chn;
		rx_dma_chn = hspi_rx_dma_chn;
	}
	else
	{
		tx_dma_chn = pspi_tx_dma_chn;
		rx_dma_chn = pspi_rx_dma_chn;
	}
	dma_set_address(tx_dma_chn,(u32)reg_dma_addr(src_addr),reg_spi_data_buf_adr(spi_sel));
	dma_set_size(tx_dma_chn,wr_len,DMA_WORD_WIDTH);
	dma_chn_en(tx_dma_chn);

	dma_set_address(rx_dma_chn,reg_spi_data_buf_adr(spi_sel),(u32)reg_dma_addr(dst_addr));
	dma_set_size(rx_dma_chn,rd_len,DMA_WORD_WIDTH);
	dma_chn_en(rx_dma_chn);
	spi_set_cmd(spi_sel,0x00);//when  cmd  disable that  will not sent cmd ,just trigger spi send .
}



/**
 * @brief      This function serves to single/dual (quad) write to the SPI slave by dma.
 * @param[in]  cmd:cmd one byte will first write.
 * @param[in]  addr:the address of slave
 * @param[in]  addr_len:the length of address
 * @param[in]  data: pointer to the data need to write
 * @param[in]  data_len:length in byte of the data need to write
 * @param[in]  wr_mode: write mode.dummy or not dummy
 * @return     none
 */
void spi_master_write_dma_plus(spi_sel_e spi_sel,u8 cmd,u32 addr,u8 * src_addr, u32 data_len,spi_wr_tans_mode_e wr_mode)
{
	u8 tx_dma_chn;
	spi_tx_fifo_clr(spi_sel);
	spi_rx_fifo_clr(spi_sel);
	spi_tx_dma_en(spi_sel);
	spi_tx_cnt(spi_sel,data_len);
	spi_set_transmode( spi_sel,wr_mode);
	if(HSPI_MODULE == spi_sel)
	{
		tx_dma_chn = hspi_tx_dma_chn;
		hspi_set_address(addr);
	}
	else
	{
		tx_dma_chn = pspi_tx_dma_chn;
	}
	dma_set_address(tx_dma_chn,(u32)reg_dma_addr(src_addr),reg_spi_data_buf_adr(spi_sel));
	dma_set_size(tx_dma_chn,data_len,DMA_WORD_WIDTH);
	dma_chn_en(tx_dma_chn);
	spi_set_cmd(spi_sel,cmd);
}

/**
 * @brief      This function serves to single/dual (quad) read from the SPI slave by dma
 * @param[in]  cmd:cmd one byte will first write.
 * @param[in]  addr:the address of slave
 * @param[in]  addr_len:the length of address
 * @param[in]  dst_addr: pointer to the buffer that will cache the reading out data
 * @param[in]  data_len:length in byte of the data need to read
 * @param[in]  rd_mode: read mode.dummy or not dummy
 * @return     none
 */
void spi_master_read_dma_plus(spi_sel_e spi_sel,u8 cmd, u32 addr,u8 * dst_addr,u32 data_len,spi_rd_tans_mode_e rd_mode)
{
	u8 rx_dma_chn;
	spi_rx_fifo_clr(spi_sel);
	spi_rx_dma_en(spi_sel);
	spi_set_transmode(spi_sel,rd_mode);
	spi_rx_cnt(spi_sel, data_len);
	if(HSPI_MODULE == spi_sel)
	{
		rx_dma_chn = hspi_rx_dma_chn;
		hspi_set_address(addr);
	}
	else
	{
		rx_dma_chn = pspi_rx_dma_chn;
	}
	dma_set_address(rx_dma_chn,reg_spi_data_buf_adr(spi_sel),(u32)reg_dma_addr(dst_addr));
	dma_set_size(rx_dma_chn,data_len,DMA_WORD_WIDTH);
	dma_chn_en(rx_dma_chn);
	spi_set_cmd(spi_sel,cmd);
}



/**
 * @brief      This function serves to single/dual (quad) write to the SPI slave by xip
 * @param[in]  cmd:cmd one byte will first write.
 * @param[in]  addr_offset:offset of xip base address.
 * @param[in]  data: pointer to the data need to write
 * @param[in]  data_len:length in byte of the data need to write
 * @param[in]  wr_mode: write mode  dummy or not dummy
 * @return     none
 */
void hspi_master_write_xip( u8 cmd,u32 addr_offset, u8 *Data,u32 data_len,spi_wr_tans_mode_e wr_mode)
{
	hspi_xip_write_transmode(wr_mode);
    hspi_xip_addr_offset(addr_offset);
	hspi_xip_wr_cmd_set(cmd);
	for(u32 i = 0; i <data_len; i++)
	{
		write_reg8(reg_hspi_xip_base_adr+i,Data[i]);
	}
}

/**
 * @brief      This function serves to single/dual (quad)  read from the SPI slave by xip
 * @param[in]  cmd:cmd one byte will first write.
 * @param[in]  addr_offset:offset of xip base address.
 * @param[in]  data: pointer to the data need to read
 * @param[in]  addr_len:the length of address
 * @param[in]  rd_mode: read mode.dummy or not dummy
 * @return     none
 * note xip_read will pre_read some clock.
 */
void hspi_master_read_xip(u8 cmd,u32 addr_offset, u8 *Data, u32 data_len,spi_rd_tans_mode_e rd_mode)
{
	hspi_xip_read_transmode(rd_mode);
	hspi_xip_addr_offset(addr_offset);
	hspi_xip_rd_cmd_set(cmd);
	for( u32 i = 0; i <data_len;i++)
		{
			Data[i] = read_reg8(reg_hspi_xip_base_adr+i);
		}
}



/**
 * @brief      This function serves to a cmd and one data write to the SPI slave by xip
 * @param[in]  cmd:cmd one byte will first write.
 * @param[in]  addr_offset:offset of xip base address.
 * @param[in]  data_in: data need to write
 * @param[in]  wr_mode: write mode  dummy or not dummy
 * @return     none
 */
void hspi_master_write_xip_cmd_data( u8 cmd,u32 addr_offset,u8 data_in,spi_wr_tans_mode_e wr_mode)
{
	hspi_xip_write_transmode(wr_mode);
    hspi_xip_addr_offset(addr_offset);
	hspi_xip_wr_cmd_set(cmd);
	write_reg8(reg_hspi_xip_base_adr,data_in);
}