#ifndef __gtDbDxAC3_Quagga_Utils_H
#define __gtDbDxAC3_Quagga_Utils_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagPacketGenerator.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>




#define ERR_HW_ERROR 23


typedef enum{
	QUAGGA_24_PORT,
	QUAGGA_48_PORT	
} WNC_DEV_TYPE;


GT_STATUS WNCCmds();


GT_STATUS WNC_AikidoSpiCtrl
(	
	int status
);


GT_STATUS WNC_WhiteLedCtrl
(	
	int status
);


GT_STATUS WNC_OrangeLedCtrl
(	
	int status
);

GT_STATUS WNC_FrontPort10GLedCtrl
(
	int ledtype,
	int status
);

GT_STATUS WNC_FrontPort1GLedCtrl
(
	int ledtype,
	int status
);

GT_STATUS WNC_Set10GRxAutoTune();
GT_STATUS WNC_Check1GPortLinkUp();
GT_STATUS WNC_Check10GPortLinkUp();
GT_STATUS WNC_Show1GUCCnt();
GT_STATUS WNC_Show10GUCCnt();
GT_STATUS WNC_CleanUCCnt
(
	int printR
);


GT_STATUS WNCPktGen
(
    IN int devNum,
    IN GT_PHYSICAL_PORT_NUM pgPort,
	IN GT_U32 pktLength,
	IN GT_U32 pktCount,
	IN GT_U8 pktCountMultiplier,
	IN GT_U8 pktIfaceSize,
	IN GT_U8 checkDone
);


GT_STATUS WNCPktGenStop
(
   IN int devNum,	
   IN GT_PHYSICAL_PORT_NUM pgPort
);

GT_STATUS WNCLedSetting
(
   IN WNC_DEV_TYPE dev_type
);

GT_STATUS WNC_1GPktGenTest
(
	IN GT_U32 pktCount	
);

GT_STATUS WNC_10GPktGenTest
(
	IN GT_U32 pktCount	
);

GT_STATUS WNCFCSetting
(
   IN WNC_DEV_TYPE dev_type
);

GT_STATUS WNCSFPTXSetting
(
   IN WNC_DEV_TYPE dev_type
);

GT_STATUS WNCQSGMIISetting
(
   IN WNC_DEV_TYPE dev_type
);

GT_STATUS WNCVODSetting
(
   IN WNC_DEV_TYPE dev_type
);

GT_STATUS WNC_1GLedCtrl
(
	int port,
	int status
);

#endif
