/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file shrMemSharedData.c
*
* @brief This file contains common data for OS/SharedMemory modules
* which should be shared between different processes, thus should
* be linked into libcpss.so (only that object
* has shared BSS/DATA sections).
*
* @version   8
********************************************************************************
*/
#include <gtOs/gtEnvDep.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/gtOsSharedUtil.h>






/*********************************************************************************************
*  The RxTxProcessParams struct used for interprocess communication in multi process appDemo
*  the RxTx parameters transmitted from appDemo process to RxTxProcess used to fill the 
*  the packet capture table
*  
*  function cpssMultiProcComSetRxTxParams used to set the RxTx parameters
*  function cpssMultiProcComGetRxTxParams used to get the RxTx parameters
*  function cpssMultiProcComSetTransmitParams used to set the Packet Transmit  parameters
*  function cpssMultiProcComGetTransmitParams used to get the Packet Transmit  parameters
*
**********************************************************************************************/
#ifdef SHARED_MEMORY

/* keep RxTx parameters struct definition */
static CPSS_RX_TX_MP_MODE_PARAMS_STC RxTxProcessParams;

/**
* @internal cpssMultiProcComSetRxTxParams function
* @endinternal
*
* @brief   Used to set packet RxTx parameters for transmission from appDemo process
*         to RxTx process
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComSetRxTxParams
(
	CPSS_RX_TX_MP_MODE_PARAMS_STC *paramPtr
)
{
	RxTxProcessParams.valid = paramPtr->valid;
	RxTxProcessParams.copyRxInfoFunc = paramPtr->copyRxInfoFunc ;
	RxTxProcessParams.doCapture = paramPtr->doCapture;
	RxTxProcessParams.freeRxInfoFunc = paramPtr->freeRxInfoFunc;
	RxTxProcessParams.indexToInsertPacket = paramPtr->indexToInsertPacket;
	RxTxProcessParams.maxNumOfRxEntries = paramPtr->maxNumOfRxEntries;
	RxTxProcessParams.maxRxBufferSize = paramPtr->maxRxBufferSize;
	RxTxProcessParams.rxEntriesArr = paramPtr->rxEntriesArr;
	RxTxProcessParams.rxTableMode = paramPtr->rxTableMode;
	RxTxProcessParams.specificDeviceFormatPtr = paramPtr->specificDeviceFormatPtr;
	RxTxProcessParams.wasInitDone = paramPtr->wasInitDone;

	return GT_OK;
}

/**
* @internal cpssMultiProcComGetRxTxParams function
* @endinternal
*
* @brief   Used to get packet RxTx parameters for transmission from appDemo process and send it
*         to RxTx process
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComGetRxTxParams
(
	CPSS_RX_TX_MP_MODE_PARAMS_STC *paramPtr
)
{
	paramPtr->valid = RxTxProcessParams.valid;
	paramPtr->copyRxInfoFunc = RxTxProcessParams.copyRxInfoFunc;
	paramPtr->doCapture = RxTxProcessParams.doCapture;
	paramPtr->freeRxInfoFunc = RxTxProcessParams.freeRxInfoFunc;
	paramPtr->indexToInsertPacket = RxTxProcessParams.indexToInsertPacket;
	paramPtr->maxNumOfRxEntries = RxTxProcessParams.maxNumOfRxEntries;
	paramPtr->maxRxBufferSize = RxTxProcessParams.maxRxBufferSize;
	paramPtr->rxEntriesArr = RxTxProcessParams.rxEntriesArr;
	paramPtr->rxTableMode = RxTxProcessParams.rxTableMode;
	paramPtr->specificDeviceFormatPtr = RxTxProcessParams.specificDeviceFormatPtr;
	paramPtr->wasInitDone = RxTxProcessParams.wasInitDone;

	return GT_OK;
}

#ifdef CHX_FAMILY


/*********** TX  ************/

static CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC transmitParams;

/**
* @internal cpssMultiProcComSetTransmitParams function
* @endinternal
*
* @brief   Used to set packet transmit parameters for transmission to RxTx process from appDemo process
*
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComSetTransmitParams
(
	CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC *paramPtr
)
{
	transmitParams.valid = paramPtr->valid;
	transmitParams.cpssDxChTxPacketDescTbl = paramPtr->cpssDxChTxPacketDescTbl ;
	transmitParams.flagStopTransmit = paramPtr->flagStopTransmit;
	transmitParams.modeSettings.cyclesNum = paramPtr->modeSettings.cyclesNum;
	transmitParams.modeSettings.gap = paramPtr->modeSettings.gap;
	transmitParams.txDxChTblCapacity = paramPtr->txDxChTblCapacity;
	transmitParams.txPoolPtr = paramPtr->txPoolPtr;

	return GT_OK;
}


/**
* @internal cpssMultiProcComGetTransmitParams function
* @endinternal
*
* @brief   Used to get packet transmit parameters to RxTx process from appDemo process
*
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS cpssMultiProcComGetTransmitParams
(
	CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC *paramPtr
)
{
	paramPtr->valid = transmitParams.valid;
	paramPtr->cpssDxChTxPacketDescTbl = transmitParams.cpssDxChTxPacketDescTbl;
	paramPtr->flagStopTransmit = transmitParams.flagStopTransmit;
	paramPtr->modeSettings.cyclesNum = transmitParams.modeSettings.cyclesNum;
	paramPtr->modeSettings.gap = transmitParams.modeSettings.gap;
	paramPtr->txDxChTblCapacity = transmitParams.txDxChTblCapacity;
	paramPtr->txPoolPtr = transmitParams.txPoolPtr;
	
	return GT_OK;
}

/************ Command transmit ***********/

static CPSS_RX_TX_PROC_COM_CMD_STC rxTxGaltisCommands = { 0,0 }; 

/**
* @internal cpssMultiProcComGetRxTxCommand function
* @endinternal
*
* @brief   Used to get if the Galtis command cmdCpssRxPktClearTbl was run by appDemo process
*/
GT_STATUS cpssMultiProcComGetRxTxCommand
(
		CPSS_RX_TX_PROC_COM_CMD_STC *cmdParamPtr
)
{

		cmdParamPtr->valid = rxTxGaltisCommands.valid;
		cmdParamPtr->mpCmdCpssRxPktClearTbl = rxTxGaltisCommands.mpCmdCpssRxPktClearTbl;

		return GT_OK;
}

/**
* @internal cpssMultiProcComSetRxTxCommand function
* @endinternal
*
* @brief   Used to set if the Galtis command cmdCpssRxPktClearTbl was run by appDemo process
*/
GT_STATUS cpssMultiProcComSetRxTxCommand
(
	CPSS_RX_TX_PROC_COM_CMD_STC *cmdParamPtr
)
{
	rxTxGaltisCommands.valid = cmdParamPtr->valid;
	rxTxGaltisCommands.mpCmdCpssRxPktClearTbl = cmdParamPtr->mpCmdCpssRxPktClearTbl;

	return GT_OK;
}


/**************** SMI ETH port transmit *************/

static CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC txKeepSmiTransmitParams;


GT_STATUS cpssMultiProcComGetSmiTransmitParams
(
	CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC *paramPtr
)
{
	paramPtr->appDemoSysConfig = txKeepSmiTransmitParams.appDemoSysConfig;
	paramPtr->valid = txKeepSmiTransmitParams.valid;

	return 0;
}

GT_STATUS cpssMultiProcComSetSmiTransmitParams
(
	CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC *paramPtr
)
{
	txKeepSmiTransmitParams.appDemoSysConfig = paramPtr->appDemoSysConfig;
	txKeepSmiTransmitParams.valid = paramPtr->valid;

	return 0;
}



#endif
#endif



