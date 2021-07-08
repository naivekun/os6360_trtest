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
* @file gtOsSharedUtil.h
*
* @brief
*
* @version   5
********************************************************************************
*/

#ifndef __gtOsSharedUtilh
#define __gtOsSharedUtilh

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IMPL_GALTIS

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>
#include <galtisAgent/wrapUtil/cmdCpssNetReceive.h>

#ifdef CHX_FAMILY
#include <galtisAgent/wrapCpss/dxCh/networkIf/cmdCpssDxChNetTransmit.h>
#endif
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/*
*	This struct used to transmit the Galtis command indication from appDemo process to RxTxProcess
*
*	Fields:
*
*		valid			-	if 1 - the struct was updated by appDemo process
*								if 0 - the data were get by the RxTxProcess
* 
*		mpCmdCpssRxPktClearTbl			-	if 1 cmdCpssRxPktClearTbl Galtis command was run in 
*																		appDemo process
*
*/
typedef struct{

	GT_BOOL valid;
	GT_U8		mpCmdCpssRxPktClearTbl;

}CPSS_RX_TX_PROC_COM_CMD_STC;

/**
* @internal cpssMultiProcComGetRxTxCommand function
* @endinternal
*
* @brief   Used to get if the Galtis command cmdCpssRxPktClearTbl was run by appDemo process
*
* @param[in] paramPtr                 - pointer to struct CPSS_RX_TX_PROC_COM_CMD_STC
*/
GT_STATUS cpssMultiProcComGetRxTxCommand
(
		CPSS_RX_TX_PROC_COM_CMD_STC *paramPtr
);

/**
* @internal cpssMultiProcComSetRxTxCommand function
* @endinternal
*
* @brief   Used to set if the Galtis command cmdCpssRxPktClearTbl was run by appDemo process
*
* @param[in] paramPtr                 - pointer to struct CPSS_RX_TX_PROC_COM_CMD_STC
*/
GT_STATUS cpssMultiProcComSetRxTxCommand
(
	CPSS_RX_TX_PROC_COM_CMD_STC *paramPtr
);


/*
*	This struct used to transmit the Rx parameters and Rx table pointer from appDemo process to
* 	RxTxProcess
*	Fields:
*
*		valid					-	data in the structure is valid (true/false) and can be red by RxTxProcess
*		rxEntriesArr			-	pointer to received packet table first entry
*		rxTableMode			-	cyclic or one time mode
*		maxRxBufferSize			-	rx buffer size
*		maxNumOfRxEntries		-	max numbr of entries
*		indexToInsertPacket		-	index of current packet insertion
*		doCapture			-	specifies if the capture should be done (true/false)
*		wasInitDone			-	indicates if the packet table initialized
*		specificDeviceFormatPtr		-	pointer to the specific device Rx info format (field of  packet entry)
*		freeRxInfoFunc			-	callback function to free the specific rx info format (field of  in packet entry)
*		copyRxInfoFunc			-	callback function to copy the specific rx info format (field of  in packet entry)
*
*/
typedef struct{

	GT_BOOL valid;
	CMD_RX_ENTRY *rxEntriesArr;
	GALTIS_RX_MODE_ENT rxTableMode;
	GT_U32  maxRxBufferSize;
	GT_U32  maxNumOfRxEntries;
	GT_U32  indexToInsertPacket;
	GT_U32  doCapture;
	GT_BOOL wasInitDone;
	void*           specificDeviceFormatPtr;
	FREE_RX_INFO_FUNC freeRxInfoFunc;
	COPY_RX_INFO_FUNC copyRxInfoFunc;

}CPSS_RX_TX_MP_MODE_PARAMS_STC;



/**
* @internal cpssMultiProcComSetRxTxParams function
* @endinternal
*
* @brief   Used to set packet RxTx parameters for transmission from appDemo process
*         to RxTx process
* @param[in] paramPtr                 - pointer to struct CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC
*/
GT_STATUS  cpssMultiProcComSetRxTxParams
(
		CPSS_RX_TX_MP_MODE_PARAMS_STC *paramPtr
 );

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
);


/**********************************************************************/
#ifdef CHX_FAMILY
/*********************************************************************/

/*
*	This struct used to transmit the Transmit parameters from appDemo process to
* 	RxTxProcess
*	Fields:
*
*	valid					-	data in the structure is valid (true/false)
*	cpssDxChTxPacketDescTbl			-	pointer to packet descriptor table
*	modeSettings includes two fields:   
*			cyclesNum		-	The maximum number of loop cycles (-1 = forever)
*			gap	   		-	The time to wit between two cycles.
*	flagStopTransmit 			-	stop flag
*	txDxChTblCapacity 			-	number of descriptors in the table
*	txPoolPtr							-	tx pool pointer
*/

typedef struct{
	GT_BOOL valid;
	DXCH_PKT_DESC_STC* cpssDxChTxPacketDescTbl;
	CPSS_TX_MODE_SETITNGS_STC modeSettings;
	GT_BOOL flagStopTransmit;
	GT_U32 txDxChTblCapacity;
	void * txPoolPtr;

}CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC;



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
);

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
);

/******************** SMI ETH port params ******************/
typedef struct{
	GT_BOOL valid;
	APP_DEMO_SYS_CONFIG_STC appDemoSysConfig;


}CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC;

GT_STATUS cpssMultiProcComGetSmiTransmitParams
(
	CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC *paramPtr
);

GT_STATUS cpssMultiProcComSetSmiTransmitParams
(
	CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC *paramPtr
);

#endif /* IMPL_GALTIS */

#ifdef __cplusplus
}
#endif

#endif

#endif  /* __gtOsSharedUtilh */
/* Do Not Add Anything Below This Line */


