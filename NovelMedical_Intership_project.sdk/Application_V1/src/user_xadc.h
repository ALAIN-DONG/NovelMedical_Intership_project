/*
 * user_xadc.h
 *
 *  Created on: 2020年7月19日
 *      Author: Alain
 */

#ifndef SRC_USER_XADC_H_
#define SRC_USER_XADC_H_

#include <stdio.h>
#include "sleep.h"
#include "xil_printf.h"
#include "xadcps.h"       //XADC设备的支持驱动程序

void XADC_Init(XAdcPs* XADCMonInst);
void XADC_Printf(XAdcPs* XADCInstPtr);
float temper_sampling(XAdcPs* XADCInstPtr);

#endif /* SRC_USER_XADC_H_ */
