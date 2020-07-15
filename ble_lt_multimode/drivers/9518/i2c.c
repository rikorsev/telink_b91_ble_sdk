/********************************************************************************************************
 * @file     i2c.c
 *
 * @brief    This is the source file for TLSR8258
 *
 * @author	 Driver Group
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

#include "i2c.h"

static unsigned char i2c_dma_tx_chn;
static unsigned char i2c_dma_rx_chn;

dma_config_st i2c_tx_dma_config={
	.dst_req_sel= DMA_REQ_I2C_TX,//tx req
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
dma_config_st i2c_rx_dma_config={
	.dst_req_sel= DMA_REQ_AUDIO0_TX,
	.src_req_sel=DMA_REQ_I2C_RX,
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
 * @brief      This function selects a pin port for I2C interface.
 * @param[in]  sda_pin - the pin port selected as I2C sda pin port.
 * @param[in]  scl_pin - the pin port selected as I2C scl pin port.
 * @return     none
 */
void i2c_set_pin(i2c_sda_pin_e sda_pin,i2c_scl_pin_e scl_pin)
{

	unsigned char val = 0;
	unsigned char mask = 0xff;

	//disable sda_pin and scl_pin gpio function.
	gpio_set_gpio_dis(scl_pin);
	gpio_set_gpio_dis(sda_pin);

	//enable gpio as i2c sda function.
	if(sda_pin == I2C_GPIO_SDA_B3)
	{
		mask= (unsigned char)~(BIT(7)|BIT(6));
		val = BIT(6);
	}
	else if(sda_pin == I2C_GPIO_SDA_C2)
	{
		mask = (unsigned char)~(BIT(5)|BIT(4));
		val = 0;
	}
	else if(sda_pin == I2C_GPIO_SDA_E2)
	{
		mask = (unsigned char)~(BIT(5)|BIT(4));
		val = 0;
	}
	else if(sda_pin == I2C_GPIO_SDA_E3)
	{
		mask = (unsigned char)~(BIT(7)|BIT(6));
		val = 0;
	}

	reg_gpio_func_mux(sda_pin)=(reg_gpio_func_mux(sda_pin)& mask)|val;


	//enable gpio as i2c scl function.
	if(scl_pin == I2C_GPIO_SCL_B2)
	{
		mask= (unsigned char)~(BIT(5)|BIT(4));
		val = BIT(4);
	}
	else if(scl_pin == I2C_GPIO_SCL_C1)
	{
		mask = (unsigned char)~(BIT(3)|BIT(2));
		val = 0;
	}
	else if(scl_pin == I2C_GPIO_SCL_E0)
	{
		mask = (unsigned char)~(BIT(1)|BIT(0));
		val = 0;
	}
	else if(scl_pin == I2C_GPIO_SCL_E1)
	{
		mask = (unsigned char)~(BIT(3)|BIT(2));
		val = 0;
	}

	reg_gpio_func_mux(scl_pin)=(reg_gpio_func_mux(scl_pin)& mask)|val;
    
	gpio_setup_up_down_resistor(sda_pin, PM_PIN_PULLUP_10K);
	gpio_setup_up_down_resistor(scl_pin, PM_PIN_PULLUP_10K);
	gpio_set_input_en(sda_pin);//enable sda input
	gpio_set_input_en(scl_pin);//enable scl input
}

/**
 * @brief      This function serves to enable i2c master function.
 * @param[in]  none.
 * @return     none.
 */
void i2c_master_init(void)
{
	reg_i2c_sct0  |=  FLD_I2C_MASTER;       //i2c master enable.
}



/**
 * @brief      This function serves to set the i2c clock frequency.The i2c clock is consistent with the system clock.
 *             Currently, the default system clock is 48M, and the i2c clock is also 48M.
 * @param[in]  clock - the division factor of I2C clock,
 *             I2C frequency = System_clock / (4*DivClock).
 * @return     none
 */
void i2c_set_master_clk(unsigned char clock)
{

	//i2c frequency = system_clock/(4*clock)
	reg_i2c_sp = clock;

	//set enable flag.
    reg_clk_en0 |= FLD_CLK0_I2C_EN;
}


/**
 * @brief      This function serves to enable slave mode.
 * @param[in]  id - the id of slave device.it contains write or read bit,the laster bit is write or read bit.
 *                       ID|0x01 indicate read. ID&0xfe indicate write.
 * @return     none
 */
void i2c_slave_init(unsigned char id)
{
	reg_i2c_sct0 &= (~FLD_I2C_MASTER); //enable slave mode.

	reg_i2c_id	  = id;                   //defaul eagle slave ID is 0x5a
}


/**
 *  @brief      The function of this API is to ensure that the data can be successfully sent out.
 *  @param[in]  id - to set the slave ID.for kite slave ID=0x5c,for eagle slave ID=0x5a.
 *  @param[in]  data - The data to be sent, The first three bytes can be set as the RAM address of the slave.
 *  @param[in]  len - This length is the total length, including both the length of the slave RAM address and the length of the data to be sent.
 *  @return     none
 */
void i2c_master_write(unsigned char id, unsigned char *data, unsigned char len)
{
	BM_SET(reg_i2c_status,FLD_I2C_TX_CLR);//clear index

	//set i2c master write.
	reg_i2c_id = id & (~FLD_I2C_WRITE_READ_BIT); //BIT(0):R:High  W:Low
	reg_i2c_len   =  len;   //length why configure this length?? is must?
	//ls_start ls_id
	reg_i2c_sct1 = (FLD_I2C_LS_ID | FLD_I2C_LS_START);

	while(i2c_master_busy());

	//write data
	unsigned int cnt = 0;
	while(cnt<len){
		if(i2c_get_tx_buf_cnt()<7){
			reg_i2c_data_buf(cnt % 4) = data[cnt];	//write data
			cnt++;
			if(cnt==1){
			    reg_i2c_sct1 = ( FLD_I2C_LS_DATAW|FLD_I2C_LS_STOP ); //launch stop cycle
			 }
		}
	}

	while(i2c_master_busy());
}


/**
 * @brief      This function serves to read a packet of data from the specified address of slave device
 * @param[in]  id - to set the slave ID.for kite slave ID=0x5c,for eagle slave ID=0x5a.
 * @param[in]  data - Store the read data
 * @param[in]  len - The total length of the data read back.
 * @return     none.
 */
void i2c_master_read(unsigned char id, unsigned char *data, unsigned char len)
{

	reg_i2c_id = (id | FLD_I2C_WRITE_READ_BIT); //BIT(0):R:High  W:Low

	//set i2c master read.
	BM_SET(reg_i2c_status,FLD_I2C_RX_CLR);//clear index

	reg_i2c_sct0  |=  FLD_I2C_RNCK_EN;       //i2c rnck enable.
	reg_i2c_sct1 = ( FLD_I2C_LS_START | FLD_I2C_LS_ID  | FLD_I2C_LS_DATAR | FLD_I2C_LS_ID_R | FLD_I2C_LS_STOP);

	reg_i2c_len   =  len;   //length why configure this length?? is must?

	unsigned int cnt = 0;
	while(cnt<len){
		if(i2c_get_rx_buf_cnt()>0){
			data[cnt] = reg_i2c_data_buf(cnt % 4);
			cnt++;
		}
	}
	while(i2c_master_busy());

}

/**
 * @brief      The function of this API is just to write data to the i2c tx_fifo by DMA.
 * @param[in]  id - to set the slave ID.for kite slave ID=0x5c,for eagle slave ID=0x5a.
 * @param[in]  data - The data to be sent, The first three bytes represent the RAM address of the slave.
 * @param[in]  len - This length is the total length, including both the length of the slave RAM address and the length of the data to be sent.
 * @return     none.
 */
void i2c_master_write_dma(unsigned char id, unsigned char *data, unsigned char len)
{

	//set id.
	reg_i2c_id = (id & (~FLD_I2C_WRITE_READ_BIT)); //BIT(0):R:High  W:Low

	dma_set_size(i2c_dma_tx_chn,len,DMA_WORD_WIDTH);
	dma_set_address(i2c_dma_tx_chn,(u32)reg_dma_addr(data),reg_i2c_data_buf0_addr);
	dma_chn_en(i2c_dma_tx_chn);

	reg_i2c_len   =  len;
	reg_i2c_sct1 = (FLD_I2C_LS_ID|FLD_I2C_LS_START|FLD_I2C_LS_DATAW |FLD_I2C_LS_STOP);

}

/**
 * @brief      This function serves to read a packet of data from the specified address of slave device.
 * @param[in]  id - to set the slave ID.for kite slave ID=0x5c,for eagle slave ID=0x5a.
 * @param[in]  rx_data - Store the read data
 * @param[in]  len - The total length of the data read back.
 * @return     none.
 */
void i2c_master_read_dma(unsigned char id, unsigned char *rx_data, unsigned char len)
{

	reg_i2c_sct0  |=  FLD_I2C_RNCK_EN;       //i2c rnck enable

	//set i2c master read.
	reg_i2c_id |= FLD_I2C_WRITE_READ_BIT;  //BIT(0):R:High  W:Low

	dma_set_size(i2c_dma_rx_chn,len,DMA_WORD_WIDTH);
	dma_set_address(i2c_dma_rx_chn,reg_i2c_data_buf0_addr,(u32)reg_dma_addr(rx_data));
	dma_chn_en(i2c_dma_rx_chn);

	reg_i2c_len   =  len;
	reg_i2c_sct1 = ( FLD_I2C_LS_ID | FLD_I2C_LS_DATAR | FLD_I2C_LS_START | FLD_I2C_LS_ID_R | FLD_I2C_LS_STOP);

}


/**
 * @brief     This function serves to set i2c tx_dam channel and config dma tx default.
 * @param[in] chn: dma channel.
 * @return    none
 */
void i2c_set_tx_dma_config(dma_chn_e chn)
{
	i2c_dma_tx_chn = chn;
	dma_config(chn, &i2c_tx_dma_config);
}

/**
 * @brief     This function serves to set i2c rx_dam channel and config dma rx default.
 * @param[in] chn: dma channel.
 * @return    none
 */
void i2c_set_rx_dma_config(dma_chn_e chn)
{
	i2c_dma_rx_chn = chn;
	dma_config(chn, &i2c_rx_dma_config);
}







