#include "user_xadc.h"

void XADC_Init(XAdcPs* XADCMonInst)
{
	int Status;

	/* 初始化XADC */
	XAdcPs_Config *ConfigPtr;
	ConfigPtr = XAdcPs_LookupConfig(XPAR_XADC_WIZ_0_DEVICE_ID);
	if (ConfigPtr == NULL) {
		xil_printf("Can't find XADC device.\r\n");
		return;
	}
	Status = XAdcPs_CfgInitialize(XADCMonInst,ConfigPtr,ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("XADC Init FAILED!\r\n");
		return;
	}

	//XADC自检
	Status = XAdcPs_SelfTest(XADCMonInst);
	if (Status != XST_SUCCESS) {
		xil_printf("XADC selftest FAILED!\r\n");
		return;
	}

	XAdcPs_SetSequencerMode(XADCMonInst,XADCPS_SEQ_MODE_SINGCHAN); //设置排序器模式
	XAdcPs_SetAlarmEnables(XADCMonInst, 0x0);   //设置是否启用报警
	XAdcPs_SetSeqInputMode(XADCMonInst, XADCPS_SEQ_MODE_SAFE);  //设置模拟输入模式
	/*  使能特定通道   */
	XAdcPs_SetSeqChEnables(XADCMonInst,XADCPS_CH_TEMP);
	//XADCPS_CH_TEMP|XADCPS_CH_VCCINT|XADCPS_CH_VCCAUX|XADCPS_CH_VBRAM|XADCPS_CH_VCCPINT|XADCPS_CH_VCCPAUX|XADCPS_CH_VCCPDRO

	xil_printf("XADC Init SUCCESS!\r\n");
}

float temper_sampling(XAdcPs* XADCInstPtr){
	u32 TempRawData;
	TempRawData = XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_TEMP);
	return XAdcPs_RawToTemperature(TempRawData);
}

void XADC_Printf(XAdcPs* XADCInstPtr)
{
	u32 TempRawData,VccIntRawData,VccAuxRawData,VBramRawData,VccPIntRawData,VccPAuxRawData,VDDRRawData;
	float TempData,VccIntData,VccAuxData,VBramData,VccPIntData,VccPAuxData,VDDRData;

	TempRawData = XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_TEMP);
	TempData = XAdcPs_RawToTemperature(TempRawData);
	printf("Raw Temp %lu Real Temp %f \n\r", TempRawData, TempData);
//	VccIntRawData= XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_VCCINT);
//	VccIntData = XAdcPs_RawToVoltage(VccIntRawData);
//	printf("Raw VccInt %lu Real VccInt %f \n\r", VccIntRawData,VccIntData);
//
//	VccAuxRawData = XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_VCCAUX);
//	VccAuxData = XAdcPs_RawToVoltage(VccAuxRawData);
//	printf("Raw VccAux %lu Real VccAux %f \n\r", VccAuxRawData,VccAuxData);
//
//	VBramRawData = XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_VBRAM);
//	VBramData = XAdcPs_RawToVoltage(VBramRawData);
//	printf("Raw VccBram %lu Real VccBram %f \n\r", VBramRawData, VBramData);
//
//	VccPIntRawData = XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_VCCPINT);
//	VccPIntData = XAdcPs_RawToVoltage(VccPIntRawData);
//	printf("Raw VccPInt %lu Real VccPInt %f \n\r", VccPIntRawData, VccPIntData);
//
//	VccPAuxRawData = XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_VCCPAUX);
//	VccPAuxData = XAdcPs_RawToVoltage(VccPAuxRawData);
//	printf("Raw VccPAux %lu Real VccPAux %f \n\r", VccPAuxRawData, VccPAuxData);
//
//	VDDRRawData = XAdcPs_GetAdcData(XADCInstPtr, XADCPS_CH_VCCPDRO);
//	VDDRData = XAdcPs_RawToVoltage(VDDRRawData);
//	printf("Raw VccDDR %lu Real VccDDR %f \n\r", VDDRRawData, VDDRData);
}
