/********************************************************************************************************
 * @file     uart.c 
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

#include "uart.h"

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
dma_config_st uart_tx_dma_config[2]={
	{	.dst_req_sel 		= DMA_REQ_UART0_TX,//tx req
		.src_req_sel 		= 0,
		.dst_addr_ctrl		= DMA_ADDR_FIX,
		.src_addr_ctrl	 	= DMA_ADDR_INCREMENT,//increment
		.dstmode		 	= DMA_HANDSHAKE_MODE,//handshake
		.srcmode			= DMA_NORMAL_MODE,
		.dstwidth 			= DMA_CTR_WORD_WIDTH,//must word
		.srcwidth 			= DMA_CTR_WORD_WIDTH,//must word
		.src_burst_size 	= 0,//must 0
		.read_num_en		= 0,
		.priority 			= 0,
		.write_num_en		= 0,
		.auto_en 			= 0,//must 0
	},
	{	.dst_req_sel 		= DMA_REQ_UART1_TX,//tx req
		.src_req_sel 		= 0,
		.dst_addr_ctrl		= DMA_ADDR_FIX,
		.src_addr_ctrl	 	= DMA_ADDR_INCREMENT,//increment
		.dstmode		 	= DMA_HANDSHAKE_MODE,//handshake
		.srcmode			= DMA_NORMAL_MODE,
		.dstwidth 			= DMA_CTR_WORD_WIDTH,//must word
		.srcwidth 			= DMA_CTR_WORD_WIDTH,//must word
		.src_burst_size 	= 0,//must 0
		.read_num_en		= 0,
		.priority 			= 0,
		.write_num_en		= 0,
		.auto_en 			= 0,//must 0
	}
};
dma_config_st uart_rx_dma_config[2]={
	{ 	.dst_req_sel 		= 0,//tx req
		.src_req_sel 		= DMA_REQ_UART0_RX,
		.dst_addr_ctrl 		= DMA_ADDR_INCREMENT,
		.src_addr_ctrl 		= DMA_ADDR_FIX,
		.dstmode 			= DMA_NORMAL_MODE,
		.srcmode 			= DMA_HANDSHAKE_MODE,
		.dstwidth 			= DMA_CTR_WORD_WIDTH,//must word
		.srcwidth 			= DMA_CTR_WORD_WIDTH,////must word
		.src_burst_size 	= 0,
		.read_num_en 		= 0,
		.priority 			= 0,
		.write_num_en 		= 1,
		.auto_en 			= 0,//must 0
	},
	{ 	.dst_req_sel 		= 0,//tx req
		.src_req_sel 		= DMA_REQ_UART1_RX,
		.dst_addr_ctrl 		= DMA_ADDR_INCREMENT,
		.src_addr_ctrl 		= DMA_ADDR_FIX,
		.dstmode 			= DMA_NORMAL_MODE,
		.srcmode 			= DMA_HANDSHAKE_MODE,
		.dstwidth 			= DMA_CTR_WORD_WIDTH,//must word
		.srcwidth 			= DMA_CTR_WORD_WIDTH,////must word
		.src_burst_size 	= 0,
		.read_num_en 		= 0,
		.priority 			= 0,
		.write_num_en 		= 1,
		.auto_en 			= 0,//must 0
	}
};
/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/
 static unsigned char uart_dma_tx_chn[2];
 static unsigned char uart_dma_rx_chn[2];
/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/
 /**
  * @brief     This function is used to look for the prime.if the prime is finded,it will
  * 			  return 1, or return 0.
  * @param[in] none
  * @return    none
  */
 static unsigned char uart_IsPrime(unsigned int n);

 /**
  *	@brief	This function serves to set pin for UART0 fuction .
  *	@param  tx_pin -To set TX pin
  *	@param  rx_pin -To set RX pin
  *	@return	none
  */
static void uart_set_fuc_pin(uart_tx_pin_e tx_pin,uart_rx_pin_e rx_pin);

/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/

/**
 * @brief      This function initializes the UART module.
 * @param[in]  g_uart_div  -  uart clock divider
 * @param[in]  g_bwpc      -  bitwidth, should be set to larger than 2
 * @param[in]  parity      - selected parity type for UART interface
 * @param[in]  stopBit     - selected length of stop bit for UART interface
 * @return     none
 */
/*
*  			sys_clk      baud rate   g_uart_div         g_bwpc
 *
 *	  	  	  	16Mhz        9600          118   			 13
 *           	 	 	 	 19200         118     			 6
 *          	 	 	 	 115200         9       		 13
 *
 *	  	  	  	24Mhz        9600          249       		 9
 *           	 	 	 	 19200		   124               9
 *           	 	 	 	 115200         12    			 15
 *
 *	  	  	  	32Mhz        9600          235       		 13
 *           	 	 	 	 19200		   235               6
 *           	 	 	 	 115200         17    			 13
 *
 *	  	  	  	48Mhz        9600          499       		 9
 *           	 	 	 	 19200		   249               9
 *           	 	 	 	 115200         25    			 15
*/
void uart_init(uart_num_e uart_num,unsigned short div, unsigned char bwpc, uart_parity_e parity, uart_stop_bit_e stop_bit)
{
    reg_uart_ctrl0(uart_num) = bwpc; //set bwpc
    reg_uart_clk_div(uart_num) = (div | FLD_UART_CLK_DIV_EN); //set div_clock

    //parity config
    if (parity) {
    	reg_uart_ctrl1(uart_num)  |= FLD_UART_PARITY_ENABLE; //enable parity function
        if (UART_PARITY_EVEN == parity) {
        	reg_uart_ctrl1(uart_num)  &= (~FLD_UART_PARITY_POLARITY); //enable even parity
        }
        else if (UART_PARITY_ODD == parity) {
        	reg_uart_ctrl1(uart_num)  |= FLD_UART_PARITY_POLARITY; //enable odd parity
        }
    }
    else {
    	reg_uart_ctrl1(uart_num)  &= (~FLD_UART_PARITY_ENABLE); //disable parity function
    }

    //stop bit config
    reg_uart_ctrl1(uart_num)  &= (~FLD_UART_STOP_SEL);
    reg_uart_ctrl1(uart_num)  |= stop_bit;
}

/***********************************************************
 * @brief  		calculate the best bwpc(bit width) .i.e reg0x96
 * @algorithm: 	BaudRate*(div+1)*(bwpc+1) = system clock
 *    			simplify the expression: div*bwpc =  constant(z)
 *    			bwpc range from 3 to 15.so loop and
 *    			get the minimum one decimal point
 * @param[in]	baudrate: baut rate of UART
 * 				tmp_sysclk: system clock
 * @param[out]	div  --  uart clock divider
 * @param[out]	bwpc --  bitwidth, should be set to larger than 2
 * @return 		the position of getting the minimum value
 */
void uart_cal_div_and_bwpc(unsigned int baudrate, unsigned int sysclk, unsigned short* div, unsigned char *bwpc)//uart0_GetBetterBwpc(unsigned int baut_rate,unsigned int  tmp_sysclk )
{
	unsigned char i = 0, j= 0;
	unsigned int primeInt = 0;
	unsigned char primeDec = 0;
	unsigned int D_intdec[13],D_int[13];
	unsigned char D_dec[13];

	primeInt = sysclk/baudrate;
	primeDec = 10*sysclk/baudrate - 10*primeInt;

	if(uart_IsPrime(primeInt)){ // primeInt is prime
		primeInt += 1;  //+1 must be not prime. and primeInt must be larger than 2.
	}
	else{
		if(primeDec > 5){ // >5
			primeInt += 1;
			if(uart_IsPrime(primeInt)){
				primeInt -= 1;
			}
		}
	}

	for(i=3;i<=15;i++){
		D_intdec[i-3] = (10*primeInt)/(i+1);////get the LSB
		D_dec[i-3] = D_intdec[i-3] - 10*(D_intdec[i-3]/10);///get the decimal section
		D_int[i-3] = D_intdec[i-3]/10;///get the integer section
	}

	//find the max and min one decimation point
	unsigned char position_min = 0,position_max = 0;
	unsigned int min = 0xffffffff,max = 0x00;
	for(j=0;j<13;j++){
		if((D_dec[j] <= min)&&(D_int[j] != 0x01)){
			min = D_dec[j];
			position_min = j;
		}
		if(D_dec[j]>=max){
			max = D_dec[j];
			position_max = j;
		}
	}

	if((D_dec[position_min]<5) && (D_dec[position_max]>=5)){
		if(D_dec[position_min]<(10-D_dec[position_max])){
			*bwpc = position_min + 3;
			*div = D_int[position_min]-1;
		}
		else{
			*bwpc = position_max + 3;
			*div = D_int[position_max];
		}
	}
	else if((D_dec[position_min]<5) && (D_dec[position_max]<5)){
		*bwpc = position_min + 3;
		*div = D_int[position_min] - 1;
	}
	else{
		*bwpc = position_max + 3;
		*div = D_int[position_max];
	}
}

/**
 * @brief  		set r_rxtimeout. this setting is transfer one bytes need cycles base on uart_clk.
 * 				For example, if  transfer one bytes (1start bit+8bits data+1 priority bit+2stop bits) total 12 bits,
 * 				this register setting should be (bpwc+1)*12.
 * @param[in]	uart_num	-- 	UART0 or UART1
 * @param[in]	bwpc 		--  bitwidth, should be set to larger than 2
 * @param[in]	bit_cnt 	--  bit number
 * @param[in]	mul		 	--  mul
 * @return 		none
 */
void uart_set_dma_rx_timeout(uart_num_e uart_num,unsigned char bwpc, unsigned char bit_cnt, uart_timeout_mul_e mul)
{
    reg_uart_rx_timeout0(uart_num) = (bwpc+1) * bit_cnt; //one byte includes 12 bits at most
	reg_uart_rx_timeout1(uart_num)  = mul; //if over 2*(tmp_bwpc+1),one transaction end.
}

/**
 * @brief     uart send data function by byte with not DMA method.
 *            variable uart_TxIndex,it must cycle the four registers 0x14080 0x14081 0x14082 0x14083 for the design of SOC.
 *            so we need variable to remember the index.
 * @param[in] uart_num - UART0 or UART1.
 * @param[in] tx_data - the data to be send.
 * @return    none
 */
void uart_send_byte(uart_num_e uart_num, unsigned char tx_data)
{
	static unsigned char uart_tx_byte_index[2] = {0};

	while(uart_get_txfifo_num(uart_num)>7);

	reg_uart_data_buf(uart_num, uart_tx_byte_index[uart_num]) = tx_data;
	uart_tx_byte_index[uart_num] ++;
	(uart_tx_byte_index[uart_num]) &= 0x03;
}


/**
 * @brief     This function serves to receive uart data by byte with not DMA method.
 * @param[in] uart_num - UART0 or UART1.
 * @return    none
 */
unsigned char uart_read_byte(uart_num_e uart_num)
{

	static unsigned char uart_rx_byte_index[2]={0};

	//while(uart0_get_rxfifo_num()<1);

	unsigned char rx_data = reg_uart_data_buf(uart_num, uart_rx_byte_index[uart_num]) ;
	uart_rx_byte_index[uart_num]++;
	uart_rx_byte_index[uart_num] &= 0x03 ;
	return rx_data;
}



/**
 * @brief     This function serves to send uart0 data by halfword with not DMA method.
 * @param[in] uart_num - UART0 or UART1.
 * @param[in] tx_data - the data to be send.
 * @return    none
 */
void uart_send_hword(uart_num_e uart_num, unsigned short tx_data)
{
	static unsigned char uart_tx_hword_index[2]={0};

	while(uart_get_txfifo_num(uart_num)>6);

	reg_uart_data_hword_buf(uart_num, uart_tx_hword_index[uart_num]) = tx_data;
	uart_tx_hword_index[uart_num]++ ;
	uart_tx_hword_index[uart_num] &= 0x01 ;
}




/**
 * @brief     This function serves to send uart0 data by word with not DMA method.
 * @param[in] uart_num - UART0 or UART1.
 * @param[in] uartData - the data to be send.
 * @return    none
 */
void uart_send_word(uart_num_e uart_num, unsigned int uartData)
{
	while (uart_get_txfifo_num(uart_num)>4);
	reg_uart_data_word_buf(uart_num) = uartData;

}

/**
 * @brief     This function sets the RTS pin's level manually
 * @param[in] uart_num - UART0 or UART1
 * @param[in] Polarity - set the output of RTS pin(only for manual mode)
 * @return    none
 */
void uart_set_rts_level(uart_num_e uart_num, unsigned char Polarity)
{
    if (Polarity) {
    	reg_uart_ctrl2(uart_num) |= FLD_UART_RTS_MANUAL_V;
    }
    else {
    	reg_uart_ctrl2(uart_num) &= (~FLD_UART_RTS_MANUAL_V);
    }
}


/**
 *	@brief		This function serves to set pin for UART0 cts function .
 *	@param[in]  cts_pin -To set cts pin
 *	@return		none
 */
void uart_set_cts_pin(uart_cts_pin_e cts_pin)
{
	u8 val = 0;
	u8 mask = 0xff;
	if(cts_pin == UART0_CTS_PA1)
	{
		mask= (unsigned char)~(BIT(2)|BIT(3));
		val = BIT(2);
	}
	else if(cts_pin == UART0_CTS_PB6)
	{
		mask = (unsigned char)~(BIT(4)|BIT(5));
		val = BIT(5);
		reg_gpio_pad_mul_sel|=BIT(0);
	}
	else if(cts_pin == UART0_CTS_PD0)
	{
		mask = (unsigned char)~(BIT(0)|BIT(1));
		val = 0;
	}
	else if(cts_pin == UART1_CTS_PC4)
	{
		mask= (unsigned char)~(BIT(0)|BIT(1));
		val = BIT(1);
		reg_gpio_pad_mul_sel|=BIT(0);
	}
	else if(cts_pin == UART1_CTS_PD4)
	{
		mask = (unsigned char)~(BIT(0)|BIT(1));
		val = 0;
	}
	else if(cts_pin == UART1_CTS_PE1)
	{
		mask = (unsigned char)~(BIT(2)|BIT(3));
		val = BIT(2);
	}
	reg_gpio_func_mux(cts_pin)=(reg_gpio_func_mux(cts_pin)& mask)|val;
	gpio_set_gpio_dis(cts_pin);
}


/**
 *	@brief		This function serves to set pin for UART0 rts function .
 *	@param[in]  rts_pin -To set rts pin
 *	@return		none
 */
void uart_set_rts_pin(uart_rts_pin_e rts_pin)
{
	u8 val = 0;
	u8 mask = 0xff;
	if(rts_pin == UART0_RTS_PA2)
	{
		mask= (unsigned char)~(BIT(4)|BIT(5));
		val = BIT(4);
	}
	else if(rts_pin == UART0_RTS_PB4)
	{
		mask = (unsigned char)~(BIT(0)|BIT(1));
		val = BIT(1);
		reg_gpio_pad_mul_sel|=BIT(0);
	}
	else if(rts_pin == UART0_RTS_PD1)
	{
		mask = (unsigned char)~(BIT(2)|BIT(3));
		val = 0;
	}
	else if(rts_pin == UART1_RTS_PC5)
	{
		mask= (unsigned char)~(BIT(2)|BIT(3));
		val = BIT(3);
		reg_gpio_pad_mul_sel|=BIT(0);
	}
	else if(rts_pin == UART1_RTS_PD5)
	{
		mask = (unsigned char)~(BIT(2)|BIT(3));
		val = 0;
	}
	else if(rts_pin == UART1_RTS_PE3)
	{
		mask = (unsigned char)~(BIT(6)|BIT(7));
		val = BIT(6);
	}
	reg_gpio_func_mux(rts_pin)=(reg_gpio_func_mux(rts_pin)& mask)|val;
	gpio_set_gpio_dis(rts_pin);
}



/**
* @brief      This function serves to select pin for UART module.
* @param[in]  tx_pin   - the pin to send data.
* @param[in]  rx_pin   - the pin to receive data.
* @return     none
*/
void uart_set_pin(uart_tx_pin_e tx_pin,uart_rx_pin_e rx_pin)
{
	gpio_set_up_down_res(tx_pin, PM_PIN_PULLUP_10K);
	gpio_set_up_down_res(rx_pin, PM_PIN_PULLUP_10K);
	uart_set_fuc_pin(tx_pin,rx_pin);//set tx and rx pin
	gpio_set_input_en(tx_pin);
	gpio_set_input_en(rx_pin);
}



/**
 * @brief     	uart0 send data function, this  function tell the DMA to get data from the RAM and start
 * @param[in]  	uart_num - UART0 or UART1
 *            	the DMA transmission
 * @param[in] 	Addr - pointer to the buffer containing data need to send
 * @param[in] 	len - DMA transmission length
 * @return    	none
 */
volatile unsigned char uart_send_dma(uart_num_e uart_num, unsigned char * Addr, unsigned char len )
{
	if (reg_uart_status2(uart_num) & FLD_UART_TX_DONE )
	{
		dma_set_address(uart_dma_tx_chn[uart_num],(u32)reg_dma_addr(Addr),reg_uart_data_buf_adr(uart_num));
		dma_set_size(uart_dma_tx_chn[uart_num],len,DMA_WORD_WIDTH);
		dma_chn_en(uart_dma_tx_chn[uart_num]);

		return 1;
	}
	return 0;
}


/**
 * @brief     	uart0 receive data function, this  function tell the DMA to get data from the uart0 data fifo and start
 *            	the DMA transmission
 * @param[in]  	uart_num - UART0 or UART1
 * @param[in] 	Addr - pointer to the buffer  receive data��
 * @return    	none
 */
 void uart_receive_dma(uart_num_e uart_num, unsigned char * Addr)
{
	dma_set_address(uart_dma_rx_chn[uart_num],reg_uart_data_buf_adr(uart_num),(u32)reg_dma_addr(Addr+4));
	reg_dma_size(uart_dma_rx_chn[uart_num]) = 0xffffffff;
//	dma_set_size(uart_dma_rx_chn[uart_num], 16, DMA_WORD_WIDTH);
	dma_chn_en(uart_dma_rx_chn[uart_num]);
}

 /**
  * @brief     This function serves to set uart tx_dam channel and config dma tx default.
  * @param[in] uart_num: UART0 or UART1.
  * @param[in] chn: dma channel.
  * @return    none
  */
 void uart_set_tx_dma_config(uart_num_e uart_num, dma_chn_e chn)
 {
	uart_dma_tx_chn[uart_num]=chn;
 	dma_config(chn, &uart_tx_dma_config[uart_num]);
 }

 /**
  * @brief     This function serves to set uart rx_dam channel and config dma rx default.
  * @param[in] uart_num: UART0 or UART1.
  * @param[in] chn: dma channel.
  * @return    none
  */
 void uart_set_rx_dma_config(uart_num_e uart_num, dma_chn_e chn)
 {
	uart_dma_rx_chn[uart_num]=chn;
 	dma_config(chn, &uart_rx_dma_config[uart_num]);
 }

 /**
  * @brief     UART hardware flow control configuration. Configure CTS.
  * @param[in] uart_num - UART0 or UART1.
  * @param[in] cts_pin   - RTS pin select.
  * @param[in] cts_parity - when CTS's input equals to select, tx will be stopped
  * @return    none
  */
 void uart_cts_config(uart_num_e uart_num,uart_cts_pin_e cts_pin,u8 cts_parity)
 {
	uart_set_cts_pin(cts_pin);

	gpio_set_input_en(cts_pin);//enable input

	if (cts_parity)
	{
		reg_uart_ctrl1(uart_num) |= FLD_UART_TX_CTS_POLARITY;
	}
	else
	{
		reg_uart_ctrl1(uart_num)  &= (~FLD_UART_TX_CTS_POLARITY);
	}
 }

 /**
  * @brief     UART hardware flow control configuration. Configure RTS.
  * @param[in] uart_num - UART0 or UART1.
  * @param[in] rts_pin - RTS pin select.
  * @param[in] rts_parity - whether invert the output of RTS pin(only for auto mode)
  * @param[in] auto_mode_en - set the mode of RTS(auto or manual).
  * @return    none
  */
 void uart_rts_config(uart_num_e uart_num,uart_rts_pin_e rts_pin,u8 rts_parity,u8 auto_mode_en)
 {
	uart_set_rts_pin(rts_pin);

	if (auto_mode_en)
	{
		reg_uart_ctrl2(uart_num) |= FLD_UART_RTS_MANUAL_M;
	}
	else {
		reg_uart_ctrl2(uart_num) &= (~FLD_UART_RTS_MANUAL_M);
	}

	if (rts_parity)
	{
		reg_uart_ctrl2(uart_num) |= FLD_UART_RTS_POLARITY;
	}
	else
	{
		reg_uart_ctrl2(uart_num) &= (~FLD_UART_RTS_POLARITY);
	}
 }

 /**********************************************************************************************************************
  *                    						local function implementation                                             *
  *********************************************************************************************************************/
 /**
  * @brief     This function is used to look for the prime.if the prime is finded,it will
  * 			  return 1, or return 0.
  * @param[in] none
  * @return    none
  */
 static unsigned char uart_IsPrime(unsigned int n)
 {
 	unsigned int i = 5;
 	if(n <= 3){
 		return 1; //althought n is prime, but the bwpc must be larger than 2.
 	}
 	else if((n %2 == 0) || (n % 3 == 0)){
 		return 0;
 	}
 	else{
 		for(i=5;i*i<n;i+=6){
 			if((n % i == 0)||(n %(i+2))==0){
 				return 0;
 			}
 		}
 		return 1;
 	}
 }

 /**
  *	@brief	This function serves to set pin for UART0 fuction .
  *	@param  tx_pin -To set TX pin
  *	@param  rx_pin -To set RX pin
  *	@return	none
  */
static void uart_set_fuc_pin(uart_tx_pin_e tx_pin,uart_rx_pin_e rx_pin)
 {
 	u8 val = 0;
 	u8 mask = 0xff;

 	if(tx_pin == UART0_TX_PA3)
 	{
 		mask= (unsigned char)~(BIT(7)|BIT(6));
 		val = BIT(6);
 	}
 	else if(tx_pin == UART0_TX_PB2)
 	{
 		mask = (unsigned char)~(BIT(5)|BIT(4));
 		val = BIT(5);
 		reg_gpio_pad_mul_sel|=BIT(0);
 	}
 	else if(tx_pin == UART0_TX_PD2)
 	{
 		mask = (unsigned char)~(BIT(5)|BIT(4));
 		val = 0;
 	}
 	else if(tx_pin == UART1_TX_PC6)
 	{
 		mask = (unsigned char)~(BIT(5)|BIT(4));
 		val = BIT(5);
 		reg_gpio_pad_mul_sel|=BIT(0);
 	}
 	else if(tx_pin == UART1_TX_PD6)
 	{
 		mask = (unsigned char)~(BIT(5)|BIT(4));
 		val = 0;
 	}
 	else if(tx_pin == UART1_TX_PE0)
 	{
 		mask = (unsigned char)~(BIT(1)|BIT(0));;
 		val = BIT(0);
 	}
 	reg_gpio_func_mux(tx_pin)=(reg_gpio_func_mux(tx_pin)& mask)|val;


 	if(rx_pin == UART0_RX_PA4)
 	{
 		mask= (unsigned char)~(BIT(1)|BIT(0));
 		val = BIT(0);

 	}
 	else if(rx_pin == UART0_RX_PB3)
 	{
 		mask = (unsigned char)~(BIT(7)|BIT(6));
 		val = BIT(7);
 		reg_gpio_pad_mul_sel|=BIT(0);
 	}
 	else if(rx_pin ==UART0_RX_PD3)
 	{
 		mask = (unsigned char)~(BIT(7)|BIT(6));
 		val = 0;
 	}
 	else if(rx_pin == UART1_RX_PC7)
 	{
 		mask = (unsigned char)~(BIT(7)|BIT(6));
 		val = BIT(7);
 		reg_gpio_pad_mul_sel|=BIT(0);
 	}
 	else if(rx_pin ==  UART1_RX_PD7)
 	{
 		mask = (unsigned char)~(BIT(7)|BIT(6));
 		val = 0;
 	}
 	else if(rx_pin ==  UART1_RX_PE2)
 	{
 		mask = (unsigned char)~(BIT(5)|BIT(4));
 		val = BIT(4);
 	}
 	//note:  setting pad the function  must before  setting no_gpio function, cause it will lead to uart transmit extra one byte data at begin.(confirmed by minghai&sunpeng)
 	reg_gpio_func_mux(rx_pin)=(reg_gpio_func_mux(rx_pin)& mask)|val;

 	gpio_set_gpio_dis(tx_pin);
 	gpio_set_gpio_dis(rx_pin);
 }


unsigned char uart_dma_send(unsigned char* Addr)//todo : biao
{
	return 0;
}
unsigned char uart_tx_is_busy(void)
{
    return 0;
}
