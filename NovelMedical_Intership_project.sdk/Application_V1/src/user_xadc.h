/*
 * user_xadc.h
 *
 *  Created on: 2020��7��19��
 *      Author: Alain
 */

#ifndef SRC_USER_XADC_H_
#define SRC_USER_XADC_H_

#include <stdio.h>
#include "sleep.h"
#include "xil_printf.h"
#include "xadcps.h"       //XADC�豸��֧����������

void XADC_Init(XAdcPs* XADCMonInst);
void XADC_Printf(XAdcPs* XADCInstPtr);
float temper_sampling(XAdcPs* XADCInstPtr);

#endif /* SRC_USER_XADC_H_ */
