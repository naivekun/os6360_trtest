#ifndef _SCHED_ALIAS_H_
#define _SCHED_ALIAS_H_

/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief declaration for TM registers alias structure.
 *
* @file tm_regs.h
*
* $Revision: 2.0 $
 */



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>




extern struct prvCpssDxChTxqSchedAddressSpace {
    struct {
        GT_U64 EccErrStatus_Addr;
        GT_U64 ScrubDis_Addr;
        GT_U64 Plast_Addr;
        GT_U64 PPerRateShapInt_Addr;
        GT_U64 PMemsEccErrStatus_Addr;
        GT_U64 PBnkEccErrStatus_Addr;
        GT_U64 PPerStatus_Addr;
        GT_U64 CPerRateShapInt_Addr;
        GT_U64 CMemsEccErrStatus_Addr;
        GT_U64 CMyQEccErrStatus_Addr;
        GT_U64 CBnkEccErrStatus_Addr;
        GT_U64 CPerStatus_Addr;
        GT_U64 BPerRateShapInt_Addr;
        GT_U64 BMemsEccErrStatus_Addr;
        GT_U64 BMyQEccErrStatus_Addr;
        GT_U64 BBnkEccErrStatus_Addr;
        GT_U64 BPerStatus_Addr;
        GT_U64 APerRateShapInt_Addr;
        GT_U64 AMemsEccErrStatus_Addr;
        GT_U64 AMyQEccErrStatus_Addr;
        GT_U64 ABnkEccErrStatus_Addr;
        GT_U64 APerStatus_Addr;
        GT_U64 QMemsEccErrStatus_Addr;
        GT_U64 QPerRateShapInt_Addr;
        GT_U64 QBnkEccErrStatus_StartAddr;
        GT_U64 QueueBank1EccErrStatus;
        GT_U64 QueueBank2EccErrStatus;
        GT_U64 QueueBank3EccErrStatus;
        GT_U64 ErrorStatus_Addr;
        GT_U64 FirstExcp_Addr;
        GT_U64 ErrCnt_Addr;
        GT_U64 ExcpCnt_Addr;
        GT_U64 ExcpMask_Addr;
        GT_U64 Identity_Addr;
        GT_U64 ForceErr_Addr;
        GT_U64 ScrubSlots_Addr;
        GT_U64 BPMsgFIFO_Addr;
        GT_U64 TreeDeqEn_Addr;
        GT_U64 PDWRREnReg;
        GT_U64 PPerCtlConf_Addr;
        GT_U64 PPerRateShap_Addr;
        GT_U64 PortExtBPEn_Addr;
        GT_U64 PBytePerBurstLimit_Addr;
        GT_U64 CPerCtlConf_Addr;
        GT_U64 CPerRateShap_Addr;
        GT_U64 BPerCtlConf_Addr;
        GT_U64 BPerRateShap_Addr;
        GT_U64 APerCtlConf_Addr;
        GT_U64 APerRateShap_Addr;
        GT_U64 QPerCtlConf_Addr;
        GT_U64 QPerRateShap_Addr;
        GT_U64 EccConfig_Addr;
        GT_U64 ECC_Memory_Parameters0;
        GT_U64 ECC_Memory_Parameters1;
        GT_U64 ECC_Memory_Parameters2;
        GT_U64 ECC_Memory_Parameters3;
        GT_U64 ECC_Memory_Parameters4;
        GT_U64 ECC_Memory_Parameters5;
        GT_U64 ECC_Memory_Parameters6;
        GT_U64 ECC_Memory_Parameters7;
        GT_U64 ECC_Memory_Parameters8;
        GT_U64 ECC_Memory_Parameters9;
        GT_U64 ECC_Memory_Parameters10;
        GT_U64 ECC_Memory_Parameters11;
        GT_U64 ECC_Memory_Parameters12;
        GT_U64 ECC_Memory_Parameters13;
        GT_U64 ECC_Memory_Parameters14;
        GT_U64 ECC_Memory_Parameters15;
        GT_U64 ECC_Memory_Parameters16;
        GT_U64 ECC_Memory_Parameters17;
        GT_U64 ECC_Memory_Parameters18;
        GT_U64 ECC_Memory_Parameters19;
        GT_U64 ECC_Memory_Parameters20;
        GT_U64 ECC_Memory_Parameters21;
        GT_U64 ECC_Memory_Parameters22;
        GT_U64 ECC_Memory_Parameters23;
        GT_U64 ECC_Memory_Parameters24;
        GT_U64 ECC_Memory_Parameters25;
        GT_U64 ECC_Memory_Parameters26;
        GT_U64 ECC_Memory_Parameters27;
        GT_U64 ECC_Memory_Parameters28;
        GT_U64 ECC_Memory_Parameters29;
        GT_U64 ECC_Memory_Parameters30;
        GT_U64 ECC_Memory_Parameters31;
        GT_U64 ECC_Memory_Parameters32;
        GT_U64 ECC_Memory_Parameters33;
        GT_U64 ECC_Memory_Parameters34;
        GT_U64 ECC_Memory_Parameters35;
        GT_U64 ECC_Memory_Parameters36;
        GT_U64 ECC_Memory_Parameters37;
        GT_U64 PortNodeState;
        GT_U64 PortMyQ;
        GT_U64 PortRRDWRRStatus01;
        GT_U64 PortRRDWRRStatus23;
        GT_U64 PortWFS;
        GT_U64 PortBPFromSTF;
        GT_U64 PortBPFromQMgr;
        GT_U64 ClvlNodeState;
        GT_U64 ClvlMyQ;
        GT_U64 ClvlRRDWRRStatus01;
        GT_U64 ClvlRRDWRRStatus23;
        GT_U64 ClvlWFS;
        GT_U64 ClvlL0ClusterStateLo;
        GT_U64 ClvlBPFromSTF;
        GT_U64 TMtoTMClvlBPState;
        GT_U64 BlvlNodeState;
        GT_U64 BlvlMyQ;
        GT_U64 BlvlRRDWRRStatus01;
        GT_U64 BlvlRRDWRRStatus23;
        GT_U64 BlvlWFS;
        GT_U64 BlvlL0ClusterStateLo;
        GT_U64 BlvlL1ClusterStateLo;
        GT_U64 BlvlL1ClusterStateHi;
        GT_U64 AlvlNodeState;
        GT_U64 AlvlMyQ;
        GT_U64 AlvlRRDWRRStatus01;
        GT_U64 AlvlWFS;
        GT_U64 AlvlL0ClusterStateLo;
        GT_U64 AlvlL1ClusterStateLo;
        GT_U64 AlvlL1ClusterStateHi;
        GT_U64 AlvlL2ClusterStateLo;
        GT_U64 AlvlL2ClusterStateHi;
        GT_U64 QueueNodeState;
        GT_U64 QueueWFS;
        GT_U64 QueueL0ClusterStateLo;
        GT_U64 QueueL1ClusterStateLo;
        GT_U64 QueueL1ClusterStateHi;
        GT_U64 QueueL2ClusterStateLo;
        GT_U64 QueueL2ClusterStateHi;
        GT_U64 TMtoTMQueueBPState;
        GT_U64 QueuePerStatus;
        GT_U64 PortEligPrioFunc_Entry;
        GT_U64 PortEligPrioFuncPtr;
        GT_U64 PortTokenBucketTokenEnDiv;
        GT_U64 PortTokenBucketBurstSize;
        GT_U64 PortDWRRPrioEn;
        GT_U64 PortQuantumsPriosLo;
        GT_U64 PortQuantumsPriosHi;
        GT_U64 PortRangeMap;
        GT_U64 ClvlEligPrioFunc_Entry;
        GT_U64 ClvlEligPrioFuncPtr;
        GT_U64 ClvlTokenBucketTokenEnDiv;
        GT_U64 ClvlTokenBucketBurstSize;
        GT_U64 ClvlDWRRPrioEn;
        GT_U64 ClvlQuantum;
        GT_U64 ClvltoPortAndBlvlRangeMap;
        GT_U64 BlvlEligPrioFunc_Entry;
        GT_U64 BlvlEligPrioFuncPtr;
        GT_U64 BlvlTokenBucketTokenEnDiv;
        GT_U64 BlvlTokenBucketBurstSize;
        GT_U64 BlvlDWRRPrioEn;
        GT_U64 BlvlQuantum;
        GT_U64 BLvltoClvlAndAlvlRangeMap;
        GT_U64 AlvlEligPrioFunc_Entry;
        GT_U64 AlvlEligPrioFuncPtr;
        GT_U64 AlvlTokenBucketTokenEnDiv;
        GT_U64 AlvlTokenBucketBurstSize;
        GT_U64 AlvlDWRRPrioEn;
        GT_U64 AlvlQuantum;
        GT_U64 ALvltoBlvlAndQueueRangeMap;
        GT_U64 QueueEligPrioFunc;
        GT_U64 QueueEligPrioFuncPtr;
        GT_U64 QueueTokenBucketTokenEnDiv;
        GT_U64 QueueTokenBucketBurstSize;
        GT_U64 QueueQuantum;
        GT_U64 QueueAMap;
        GT_U64 PortShpBucketLvls;
        GT_U64 ClvlShpBucketLvls;
        GT_U64 CLvlDef;
        GT_U64 BlvlShpBucketLvls;
        GT_U64 BlvlDef;
        GT_U64 AlvlShpBucketLvls;
        GT_U64 AlvlDef;
        GT_U64 QueueShpBucketLvls;
        GT_U64 QueueDef;
        GT_U64 PortShaperBucketNeg;
        GT_U64 CLevelShaperBucketNeg;
        GT_U64 BLevelShaperBucketNeg;
        GT_U64 ALevelShaperBucketNeg;
        GT_U64 QueueShaperBucketNeg;
        GT_U64 PortRRDWRRStatus67;
        GT_U64 PortRRDWRRStatus45;
        GT_U64 ClvlRRDWRRStatus67;
        GT_U64 ClvlRRDWRRStatus45;
        GT_U64 ClvlL0ClusterStateHi;
        GT_U64 BlvlRRDWRRStatus67;
        GT_U64 BlvlRRDWRRStatus45;
        GT_U64 BlvlL0ClusterStateHi;
        GT_U64 AlvlRRDWRRStatus67;
        GT_U64 AlvlRRDWRRStatus45;
        GT_U64 AlvlL0ClusterStateHi;
        GT_U64 QueueL0ClusterStateHi;
        GT_U64 TMtoTMPortBPState;
        GT_U64 TMtoTMBlvlBPState;
        GT_U64 AlvlRRDWRRStatus23;
        GT_U64 TMtoTMAlvlBPState;
	    GT_U64 PortDefPrio0;
        GT_U64 PortDefPrio1;
        GT_U64 PortDefPrio2;
        GT_U64 PortDefPrio3;
        GT_U64 PortDefPrio4;
        GT_U64 PortDefPrio5;
        GT_U64 PortDefPrio6;
        GT_U64 PortDefPrio7;
    } Sched;
} addressSpace;



void prvSchedInitAdressStruct(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SCHED_ALIAS_H_*/
