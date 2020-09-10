/********************************************************************************************************
 * @file	analog.c
 *
 * @brief	This is the source file for B91
 *
 * @author	Ble Group
 * @date	2019
 *
 * @par		Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd.
 *			All rights reserved.
 *
 *			The information contained herein is confidential property of Telink
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
#include "analog.h"
#include "compiler.h"
#include "plic.h"
/**********************************************************************************************************************
 *                                			  local constants                                                       *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                           	local macro                                                        *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                             local data type                                                     *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              global variable                                                       *
 *********************************************************************************************************************/



dma_config_st analog_tx_dma_config={
	.dst_req_sel 		= DMA_REQ_ALGM_TX,    /* < tx req  */
	.src_req_sel 		= 0,
	.dst_addr_ctrl		= DMA_ADDR_FIX,
	.src_addr_ctrl	 	= DMA_ADDR_INCREMENT, /* < increment */
	.dstmode		 	= DMA_HANDSHAKE_MODE,	/* < handshake */
	.srcmode			= DMA_NORMAL_MODE,
	.dstwidth 			= DMA_CTR_WORD_WIDTH,
	.srcwidth 			= DMA_CTR_WORD_WIDTH,
	.src_burst_size 	= 0,	/* < must 0 */
	.read_num_en		= 0,
	.priority 			= 0,
	.write_num_en		= 0,
	.auto_en 			= 0,   	/* < must 0 */
};
dma_config_st analog_rx_dma_config={
	.dst_req_sel 		= 0,//tx req
	.src_req_sel 		= DMA_REQ_ALGM_RX,
	.dst_addr_ctrl 		= DMA_ADDR_INCREMENT,
	.src_addr_ctrl 		= DMA_ADDR_FIX,
	.dstmode 			= DMA_NORMAL_MODE,
	.srcmode 			= DMA_HANDSHAKE_MODE,
	.dstwidth 			= DMA_CTR_WORD_WIDTH,	/* < must word */
	.srcwidth 			= DMA_CTR_WORD_WIDTH,	/* < must word */
	.src_burst_size 	= 0,
	.read_num_en 		= 0,
	.priority 			= 0,
	.write_num_en 		= 0,
	.auto_en 			= 0,//must 0
};
/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/
/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/


/**
 * @brief      This function serves to judge whether analog write/read is busy .
 * @return     none.
 */
static inline void analog_wait();
/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/





/**
 * @brief      This function serves to analog register read by byte.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
unsigned char analog_read_reg8(unsigned char addr){
	unsigned int r=core_interrupt_disable();
	reg_ana_addr = addr;
	reg_ana_len=0x1;
    reg_ana_ctrl = FLD_ANA_CYC;
    analog_wait();
	unsigned char data= reg_ana_data(0);
	core_restore_interrupt(r);
	return data;

}

/**
 * @brief      This function serves to analog register write by byte.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
void analog_write_reg8(unsigned char addr, unsigned char data){
	unsigned int r=core_interrupt_disable();
	reg_ana_addr = addr;
    reg_ana_data(0) = data;
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
	analog_wait();
	reg_ana_ctrl =0x00;
	core_restore_interrupt(r);
}

/**
 * @brief      This function serves to analog register write by halfword.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
void analog_write_reg16(unsigned char addr, unsigned short data)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_addr = addr;
	reg_ana_addr_data16	 = data;
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
	analog_wait();
	core_restore_interrupt(r);
}

/**
 * @brief      This function serves to analog register read by halfword.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
unsigned short analog_read_reg16(unsigned char addr)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_len=2;
	reg_ana_addr = addr;
	reg_ana_ctrl = FLD_ANA_CYC;
	analog_wait();
	unsigned short data=reg_ana_addr_data16;
	core_restore_interrupt(r);
	return data;
}



/**
 * @brief      This function serves to analog register read by word.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
unsigned int analog_read_reg32(unsigned char addr)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_len = 4;
	reg_ana_addr = addr;
	reg_ana_ctrl = FLD_ANA_CYC;
	analog_wait();
	unsigned int data=reg_ana_addr_data32;
	core_restore_interrupt(r);
	return data;
}


/**
 * @brief      This function serves to analog register write by word.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
void analog_write_reg32(unsigned char addr, unsigned int data)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_addr = addr;
	reg_ana_addr_data32	= data;
	reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
	analog_wait();
	core_restore_interrupt(r);
}

/**
 * @brief      This function serves to analog register write by word using dma.
 * @param[in]  chn - the dma channel.
 * @param[in]  addr - address need to be write.
 * @param[in]  pdat - the value need to be write.
 * @return     none.
 */
void analog_read_reg32_dma(dma_chn_e chn, unsigned char addr, void *pdat)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_len = 0x04;
	reg_ana_addr = addr;
	reg_ana_ctrl = FLD_ANA_CYC;
	reg_dma_src_addr(chn) = 0x80140184;
	reg_dma_dst_addr(chn) = convert_ram_addr_cpu2bus(pdat);
	dma_set_size(chn, 4, DMA_WORD_WIDTH);
	analog_rx_dma_config.dstwidth = DMA_CTR_WORD_WIDTH;
	analog_rx_dma_config.srcwidth = DMA_CTR_WORD_WIDTH;
	dma_config(chn, &analog_rx_dma_config);
	dma_chn_en(chn);
	analog_wait();
	core_restore_interrupt(r);
}

/**
 * @brief      This function serves to analog register write by word using dma.
 * @param[in]  chn - the dma channel.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
void analog_write_reg32_dma(dma_chn_e chn, unsigned char addr, void *pdat)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_addr = addr;
	reg_dma_src_addr(chn) = convert_ram_addr_cpu2bus(pdat);
	reg_dma_dst_addr(chn) = 0x80140184;
	dma_set_size(chn, 4, DMA_WORD_WIDTH);
	analog_tx_dma_config.dstwidth = DMA_CTR_WORD_WIDTH;
	analog_tx_dma_config.srcwidth = DMA_CTR_WORD_WIDTH;
	dma_config(chn, &analog_tx_dma_config);
	dma_chn_en(chn);
	reg_ana_ctrl = FLD_ANA_CYC | FLD_ANA_RW;
	analog_wait();
	core_restore_interrupt(r);
}





/**
 * @brief      This function write buffer to analog register.
 * @param[in]  addr - address need to be write.
 * @param[in]  *buff - the buffer need to be write.
 * @param[in]  len - the length of buffer.
 * @return     none.
 */
_attribute_ram_code_sec_noinline_ void analog_write_buff(unsigned char addr, unsigned char *buff, int len)
{
	unsigned char wr_idx = 0;
	unsigned char len_t = len;
	unsigned int r =core_interrupt_disable();
	reg_ana_len=len;
	reg_ana_addr = addr;
	reg_ana_ctrl = FLD_ANA_CYC | FLD_ANA_RW;
	if(len_t <= 4)
	{
		while(len_t--)
			reg_ana_data(wr_idx++) = *(buff++);
		write_reg8(ALG_BASE_ADDR + 0x2, 0x60);
		while((read_reg8(ALG_BASE_ADDR + 0x2) & BIT(7)) == BIT(7));
	}
	else
	{
		len_t = 4;
		while(len_t--)
			reg_ana_data(wr_idx++) = *(buff++);
		write_reg8(ALG_BASE_ADDR + 0x2, 0x60);
		len_t = len - 4;
		wr_idx = 0;
		while(len_t--)
		{
			reg_ana_data(wr_idx++) = *(buff++);
			if(wr_idx == 4)
			{
				if(len_t == 0)
					break;
				wr_idx = 0;
				while((read_reg8(ALG_BASE_ADDR + 0xa) & BIT(0)) == 0);
			}
		}
		if(wr_idx != 0)
		while((read_reg8(ALG_BASE_ADDR + 0x2) & BIT(7)) == BIT(7));
	}
	reg_ana_ctrl = 0x00;
	core_restore_interrupt(r);
}

/**
 * @brief      This function read data from analog register to buffer.
 * @param[in]  addr - address need to be read from.
 * @param[in]  *buff - the buffer need to be put data.
 * @param[in]  len - the length of read data.
 * @return     none.
 */
_attribute_ram_code_sec_noinline_ void analog_read_buff(unsigned char addr, unsigned char *buff, int len)
{
	unsigned int r=core_interrupt_disable();
	unsigned char rd_idx = 0;
	unsigned char len_t = len;
	reg_ana_len=len;
	reg_ana_addr = addr;
	reg_ana_ctrl = FLD_ANA_CYC;
	if (len_t <= 4)
	{
		while((read_reg8(ALG_BASE_ADDR + 0x9) & BIT(7)) == 0);
		while(len_t--)
			(*buff++) = reg_ana_data(rd_idx++);
	}
	else
	{
		while((read_reg8(ALG_BASE_ADDR + 0xa) & BIT(1)) == 0);
		while(len_t--)
		{
			(*buff++) = reg_ana_data(rd_idx++);
			if(rd_idx == 4)
			{
				rd_idx = 0;
				if(len_t <= 4)
					break;
				else
					while((read_reg8(ALG_BASE_ADDR + 0xa) & BIT(1)) == 0);
			}
		}
		while((read_reg8(ALG_BASE_ADDR + 0x9) & BIT(7)) == 0);
		while(len_t--)
			(*buff++) = reg_ana_data(rd_idx++);
	}
	reg_ana_ctrl = 0x00;
	core_restore_interrupt(r);
}

/**
 * @brief      This function write buffer to analog register by dma channel.
 * @param[in]  chn - the dma channel.
 * @param[in]  addr - address need to be write.
 * @param[in]  *pdat - the buffer need to be write.
 * @param[in]  len - the length of buffer.
 * @return     none.
 */
void analog_write_buff_dma(dma_chn_e chn, unsigned char addr, unsigned char * pdat, unsigned int len)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_addr = addr;
	reg_dma_src_addr(chn) = convert_ram_addr_cpu2bus(pdat);
	reg_dma_dst_addr(chn) = 0x80140184;
	dma_set_size(chn, len, DMA_WORD_WIDTH);
	analog_tx_dma_config.dstwidth = DMA_CTR_WORD_WIDTH;
	analog_tx_dma_config.srcwidth = DMA_CTR_WORD_WIDTH;
	dma_config(chn, &analog_tx_dma_config);
	dma_chn_en(chn);
	reg_ana_ctrl = 0x60;
	analog_wait();
	core_restore_interrupt(r);
}

/**
 * @brief      This function write buffer to analog register by dma channel.
 * @param[in]  chn - the dma channel.
 * @param[in]  addr - address need to be read from.
 * @param[in]  *pdat - the buffer need to be put data.
 * 			   note: The size of array pdat must be a multiple of 4.
 * 			  	 	For example, if you just need read 5 byte by dma, you should
 * 			  	 	define the size of array pdat to be greater than 8 other than 5.
 * 			  	 	Because the dma would return 4 byte data every time, 5 byte is
 * 			  	 	not enough to store them.
 * @param[in]  len - the length of read data.
 * @return     none.
 */
void analog_read_buff_dma(dma_chn_e chn, unsigned char addr, unsigned char *pdat, unsigned int len)
{
	unsigned int r=core_interrupt_disable();
	reg_ana_len = len;
	reg_ana_addr = addr;

	reg_dma_src_addr(chn) = 0x80140184;
	reg_dma_dst_addr(chn) = convert_ram_addr_cpu2bus(pdat);
	dma_set_size(chn, len, DMA_WORD_WIDTH);
	analog_rx_dma_config.dstwidth = DMA_CTR_WORD_WIDTH;
	analog_rx_dma_config.srcwidth = DMA_CTR_WORD_WIDTH;
	dma_config(chn, &analog_rx_dma_config);
	dma_chn_en(chn);
	reg_ana_ctrl = FLD_ANA_CYC;
	analog_wait();
	core_restore_interrupt(r);
}

/**
 * @brief      This function write buffer to analog register by dma channel.
 * @param[in]  chn - the dma channel.
 * @param[in]  *pdat - the buffer need to be put data.
 * 				The array pdat should look like this:
 * 						------------------
 * 						| 	pdat  	 	 |
 * 						|----------------|
 * 				pdat[0] | address 	0x3a |
 * 				pdat[1] | data     	0x11 |
 * 				pdat[2] | address 	0x3b |
 *				pdat[3] | data    	0x22 |
 *						| ...... 		 |
 * 						------------------
 * 				It means write data 0x11 to address 0x3a,
 * 						 write data 0x22 to address 0x3b,
 * 						 ......
 * @param[in]  len - the length of read data.
 * @return     none.
 */
void analog_write_addr_data_dma(dma_chn_e chn, void *pdat, int len)
{
	unsigned int r=core_interrupt_disable();

	reg_dma_src_addr(chn) = convert_ram_addr_cpu2bus(pdat);
	reg_dma_dst_addr(chn) = 0x80140184;
	dma_set_size(chn, len, DMA_WORD_WIDTH);
	analog_tx_dma_config.dstwidth = DMA_CTR_WORD_WIDTH;
	analog_tx_dma_config.srcwidth = DMA_CTR_WORD_WIDTH;
	dma_config(chn, &analog_tx_dma_config);
	dma_chn_en(chn);

	delay_us(1);
	reg_ana_ctrl = FLD_ANA_RW;
	reg_ana_dma_ctl = FLD_ANA_CYC1|FLD_ANA_DMA_EN;

	delay_us(1);
	while(!(reg_ana_sta & BIT(3)));
    reg_ana_ctrl = 0x00;
    reg_ana_dma_ctl = 0;
    core_restore_interrupt(r);

}

/**********************************************************************************************************************
  *                    						local function implementation                                             *
  *********************************************************************************************************************/
/**
 * @brief      This function serves to judge whether analog write/read is busy .
 * @return     none.
 */
static inline void analog_wait(){
	while(reg_ana_ctrl & FLD_ANA_BUSY){}
}



