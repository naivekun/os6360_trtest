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
 * @brief set TM register addresses - generated file
 *
* @file tm_regs.c
*
* $Revision: 2.0 $
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>



struct prvCpssDxChTxqSchedAddressSpace addressSpace;



/** Synced to Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.10} \TXQ_PSI \TXQ_PDQ 
 */

#define SET_REG_ADDRESS(alias, hiaddr, loaddr)	  alias.l[0]=loaddr;  alias.l[1]=hiaddr;

static GT_BOOL isInitialized = GT_FALSE;



void prvSchedInitAdressStruct()
{

        if (isInitialized == GT_TRUE)
            return;
        SET_REG_ADDRESS(addressSpace.Sched.EccErrStatus_Addr, 0x00000000, 0x00064200)    /* QM Scheduler ECC Error Log - General */
        SET_REG_ADDRESS(addressSpace.Sched.ScrubDis_Addr, 0x00000000, 0x00064208)    /* Scrubbing Disable */
        SET_REG_ADDRESS(addressSpace.Sched.Plast_Addr, 0x00000000, 0x00064210)    /* Tree Status */
        SET_REG_ADDRESS(addressSpace.Sched.PPerRateShapInt_Addr, 0x00000000, 0x00067620)    /* Port Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.PMemsEccErrStatus_Addr, 0x00000000, 0x00067628)    /* Port ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.PBnkEccErrStatus_Addr, 0x00000000, 0x00067630)    /* Port Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.PPerStatus_Addr, 0x00000000, 0x00067920)    /* Port Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.CPerRateShapInt_Addr, 0x00000000, 0x0006ac80)    /* C-level Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.CMemsEccErrStatus_Addr, 0x00000000, 0x0006ac88)    /* C Level ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.CMyQEccErrStatus_Addr, 0x00000000, 0x0006ac90)    /* C Level MyQ ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.CBnkEccErrStatus_Addr, 0x00000000, 0x0006aca0)    /* C Level Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.CPerStatus_Addr, 0x00000000, 0x0006b600)    /* C Level Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.BPerRateShapInt_Addr, 0x00000000, 0x0006ecc0)    /* B-level Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.BMemsEccErrStatus_Addr, 0x00000000, 0x0006ecc8)    /* B Level ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.BMyQEccErrStatus_Addr, 0x00000000, 0x0006ecd0)    /* B Level MyQ ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.BBnkEccErrStatus_Addr, 0x00000000, 0x0006ece0)    /* B Level Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.BPerStatus_Addr, 0x00000000, 0x0006ecf0)    /* B Level Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.APerRateShapInt_Addr, 0x00000000, 0x0007ce80)    /* A-level Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.AMemsEccErrStatus_Addr, 0x00000000, 0x0007ce88)    /* A Level ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.AMyQEccErrStatus_Addr, 0x00000000, 0x0007ce90)    /* A Level MyQ ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.ABnkEccErrStatus_Addr, 0x00000000, 0x0007cea0)    /* A Level Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.APerStatus_Addr, 0x00000000, 0x0007ceb0)    /* A Level Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.QMemsEccErrStatus_Addr, 0x00000000, 0x0008af00)    /* Queue ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QPerRateShapInt_Addr, 0x00000000, 0x0008af08)    /* Queue Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.QBnkEccErrStatus_StartAddr, 0x00000000, 0x0008af20)    /* Queue ECC Bank 0 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QueueBank1EccErrStatus, 0x00000000, 0x0008af28)    /* Queue ECC Bank 1 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QueueBank2EccErrStatus, 0x00000000, 0x0008af30)    /* Queue ECC Bank 2 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QueueBank3EccErrStatus, 0x00000000, 0x0008af38)    /* Queue ECC Bank 3 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.ErrorStatus_Addr, 0x00000000, 0x00000200)    /* QM Scheduler Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.FirstExcp_Addr, 0x00000000, 0x00000208)    /* QM Scheduler First Exception */
        SET_REG_ADDRESS(addressSpace.Sched.ErrCnt_Addr, 0x00000000, 0x00000210)    /* QM Scheduler Error Counter */
        SET_REG_ADDRESS(addressSpace.Sched.ExcpCnt_Addr, 0x00000000, 0x00000218)    /* QM Scheduler Exception Counter */
        SET_REG_ADDRESS(addressSpace.Sched.ExcpMask_Addr, 0x00000000, 0x00000220)    /* QM Scheduler Exception Mask */
        SET_REG_ADDRESS(addressSpace.Sched.Identity_Addr, 0x00000000, 0x00000228)    /* Identity */
        SET_REG_ADDRESS(addressSpace.Sched.ForceErr_Addr, 0x00000000, 0x00000230)    /* QM Scheduler Force Error */
        SET_REG_ADDRESS(addressSpace.Sched.ScrubSlots_Addr, 0x00000000, 0x00000238)    /* Scrubbing Slots Allocation */
        SET_REG_ADDRESS(addressSpace.Sched.BPMsgFIFO_Addr, 0x00000000, 0x00000240)    /* TM to TM Backpressure FIFO */
        SET_REG_ADDRESS(addressSpace.Sched.TreeDeqEn_Addr, 0x00000000, 0x00000248)    /* Tree Dequeue Enable */
        SET_REG_ADDRESS(addressSpace.Sched.PDWRREnReg, 0x00000000, 0x00000250)    /* Tree DWRR Priority Enable for Port Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.PPerCtlConf_Addr, 0x00000000, 0x00000a00)    /* Port Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.PPerRateShap_Addr, 0x00000000, 0x00000a08)    /* Port Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.PortExtBPEn_Addr, 0x00000000, 0x00000a10)    /* Port External backpressure Enable */
        SET_REG_ADDRESS(addressSpace.Sched.PBytePerBurstLimit_Addr, 0x00000000, 0x00000a18)    /* Port DWRR Byte Per Burst Limit */
        SET_REG_ADDRESS(addressSpace.Sched.CPerCtlConf_Addr, 0x00000000, 0x00005600)    /* C-level Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.CPerRateShap_Addr, 0x00000000, 0x00005608)    /* C Level Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.BPerCtlConf_Addr, 0x00000000, 0x00009a00)    /* B-Level Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.BPerRateShap_Addr, 0x00000000, 0x00009a08)    /* B level Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.APerCtlConf_Addr, 0x00000000, 0x0000de00)    /* A-Level Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.APerRateShap_Addr, 0x00000000, 0x0000de08)    /* A Level Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.QPerCtlConf_Addr, 0x00000000, 0x0001b200)    /* Queue Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.QPerRateShap_Addr, 0x00000000, 0x0001b208)    /* Queue Level Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.EccConfig_Addr, 0x00000000, 0x00094240)    /* ECC Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters0, 0x00000000, 0x00094248)    /* ECC Memory Parameters0 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters1, 0x00000000, 0x00094250)    /* ECC Memory Parameters1 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters2, 0x00000000, 0x00094258)    /* ECC Memory Parameters2 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters3, 0x00000000, 0x00094260)    /* ECC Memory Parameters3 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters4, 0x00000000, 0x00094268)    /* ECC Memory Parameters4 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters5, 0x00000000, 0x00094270)    /* ECC Memory Parameters5 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters6, 0x00000000, 0x00094278)    /* ECC Memory Parameters6 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters7, 0x00000000, 0x00094280)    /* ECC Memory Parameters7 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters8, 0x00000000, 0x00094288)    /* ECC Memory Parameters8 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters9, 0x00000000, 0x00094290)    /* ECC Memory Parameters9 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters10, 0x00000000, 0x00094298)    /* ECC Memory Parameters10 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters11, 0x00000000, 0x000942a0)    /* ECC Memory Parameters11 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters12, 0x00000000, 0x000942a8)    /* ECC Memory Parameters12 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters13, 0x00000000, 0x000942b0)    /* ECC Memory Parameters13 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters14, 0x00000000, 0x000942b8)    /* ECC Memory Parameters14 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters15, 0x00000000, 0x000942c0)    /* ECC Memory Parameters15 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters16, 0x00000000, 0x000942c8)    /* ECC Memory Parameters16 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters17, 0x00000000, 0x000942d0)    /* ECC Memory Parameters17 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters18, 0x00000000, 0x000942d8)    /* ECC Memory Parameters18 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters19, 0x00000000, 0x000942e0)    /* ECC Memory Parameters19 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters20, 0x00000000, 0x000942e8)    /* ECC Memory Parameters20 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters21, 0x00000000, 0x000942f0)    /* ECC Memory Parameters21 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters22, 0x00000000, 0x000942f8)    /* ECC Memory Parameters22 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters23, 0x00000000, 0x00094300)    /* ECC Memory Parameters23 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters24, 0x00000000, 0x00094308)    /* ECC Memory Parameters24 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters25, 0x00000000, 0x00094310)    /* ECC Memory Parameters25 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters26, 0x00000000, 0x00094318)    /* ECC Memory Parameters26 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters27, 0x00000000, 0x00094320)    /* ECC Memory Parameters27 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters28, 0x00000000, 0x00094328)    /* ECC Memory Parameters28 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters29, 0x00000000, 0x00094330)    /* ECC Memory Parameters29 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters30, 0x00000000, 0x00094338)    /* ECC Memory Parameters30 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters31, 0x00000000, 0x00094340)    /* ECC Memory Parameters31 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters32, 0x00000000, 0x00094348)    /* ECC Memory Parameters32 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters33, 0x00000000, 0x00094350)    /* ECC Memory Parameters33 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters34, 0x00000000, 0x00094358)    /* ECC Memory Parameters34 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters35, 0x00000000, 0x00094360)    /* ECC Memory Parameters35 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters36, 0x00000000, 0x00094368)    /* ECC Memory Parameters36 */
        SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters37, 0x00000000, 0x00094370)    /* ECC Memory Parameters37 */
        SET_REG_ADDRESS(addressSpace.Sched.PortNodeState, 0x00000000, 0x00064600)    /* Port Node State */
        SET_REG_ADDRESS(addressSpace.Sched.PortMyQ, 0x00000000, 0x00064e00)    /* Port  MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus01, 0x00000000, 0x00065600)    /* Port RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus23, 0x00000000, 0x00065e00)    /* Port RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortWFS, 0x00000000, 0x00067600)    /* Port Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.PortBPFromSTF, 0x00000000, 0x00067700)    /* Port Back-Pressure From FCU */
        SET_REG_ADDRESS(addressSpace.Sched.PortBPFromQMgr, 0x00000000, 0x00067800)    /* Port Back-Pressure From Queue Manager */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlNodeState, 0x00000000, 0x00067a00)    /* C Level Node State */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlMyQ, 0x00000000, 0x00068200)    /* C Level MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus01, 0x00000000, 0x00068a00)    /* C Level RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus23, 0x00000000, 0x00069200)    /* C Level RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlWFS, 0x00000000, 0x0006aa00)    /* C Level Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateLo, 0x00000000, 0x0006aa80)    /* C Level L0 Cluster State Lo */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlBPFromSTF, 0x00000000, 0x0006acb0)    /* C Level Back-Pressure From Status Formatter */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMClvlBPState, 0x00000000, 0x0006ae00)    /* C Level TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlNodeState, 0x00000000, 0x0006ba00)    /* B Level Node State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlMyQ, 0x00000000, 0x0006c200)    /* B Level MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus01, 0x00000000, 0x0006ca00)    /* B Level RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus23, 0x00000000, 0x0006d200)    /* B Level RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlWFS, 0x00000000, 0x0006ea00)    /* B Level Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateLo, 0x00000000, 0x0006ea80)    /* B Level L0 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateLo, 0x00000000, 0x0006ec80)    /* B Level L1 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateHi, 0x00000000, 0x0006eca0)    /* B Level L1 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlNodeState, 0x00000000, 0x00070200)    /* A Level Node State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlMyQ, 0x00000000, 0x00072200)    /* A Level MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus01, 0x00000000, 0x00074200)    /* A Level RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlWFS, 0x00000000, 0x0007c200)    /* A Level Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateLo, 0x00000000, 0x0007c400)    /* A Level L0 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateLo, 0x00000000, 0x0007cc00)    /* A Level L1 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateHi, 0x00000000, 0x0007cd00)    /* A Level L1 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateLo, 0x00000000, 0x0007ce00)    /* A Level L2 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateHi, 0x00000000, 0x0007ce40)    /* A Level L2 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueNodeState, 0x00000000, 0x00080200)    /* Queue Node State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueWFS, 0x00000000, 0x00088200)    /* Queue Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateLo, 0x00000000, 0x00088a00)    /* Queue L0 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateLo, 0x00000000, 0x0008aa00)    /* Queue L1 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateHi, 0x00000000, 0x0008ac00)    /* Queue L1 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateLo, 0x00000000, 0x0008ae00)    /* Queue L2 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateHi, 0x00000000, 0x0008ae80)    /* Queue L2 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMQueueBPState, 0x00000000, 0x0008c200)    /* Queue TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.QueuePerStatus, 0x00000000, 0x00094200)    /* Queue Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFunc_Entry, 0x00000000, 0x00000c00)    /* Port Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFuncPtr, 0x00000000, 0x00001e00)    /* Port Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketTokenEnDiv, 0x00000000, 0x00002600)    /* Port Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketBurstSize, 0x00000000, 0x00002e00)    /* Port Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.PortDWRRPrioEn, 0x00000000, 0x00003600)    /* Port DWRR Priority Enable for C-Level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.PortQuantumsPriosLo, 0x00000000, 0x00003e00)    /* Port DWRR Quantum - Low Priorities (0-3) */
        SET_REG_ADDRESS(addressSpace.Sched.PortQuantumsPriosHi, 0x00000000, 0x00004600)    /* Port DWRR Quantum - High Priorities (4-7) */
        SET_REG_ADDRESS(addressSpace.Sched.PortRangeMap, 0x00000000, 0x00004e00)    /* Port Mapping to C-Level Range */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFunc_Entry, 0x00000000, 0x00005800)    /* C-Level Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFuncPtr, 0x00000000, 0x00006a00)    /* C-level Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketTokenEnDiv, 0x00000000, 0x00007200)    /* C-level Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketBurstSize, 0x00000000, 0x00007a00)    /* C-level Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlDWRRPrioEn, 0x00000000, 0x00008200)    /* C-level DWRR Priority Enable for C-Level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlQuantum, 0x00000000, 0x00008a00)    /* C-Level DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.ClvltoPortAndBlvlRangeMap, 0x00000000, 0x00009200)    /* C-Level Mapping: C-Level to Port and C-Level to B-Level Range */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFunc_Entry, 0x00000000, 0x00009c00)    /* B-Level Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFuncPtr, 0x00000000, 0x0000ae00)    /* B-Level Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketTokenEnDiv, 0x00000000, 0x0000b600)    /* B-Level Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketBurstSize, 0x00000000, 0x0000be00)    /* B-Level Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlDWRRPrioEn, 0x00000000, 0x0000c600)    /* B-Level DWRR Priority Enable for A-Level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlQuantum, 0x00000000, 0x0000ce00)    /* B-Level DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap, 0x00000000, 0x0000d600)    /* B-Level Mapping: B-Level to C-Level Node and B-Level to A-Level Range */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFunc_Entry, 0x00000000, 0x0000e000)    /* A-Level Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFuncPtr, 0x00000000, 0x0000f200)    /* A-Level Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketTokenEnDiv, 0x00000000, 0x00011200)    /* A-Level Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketBurstSize, 0x00000000, 0x00013200)    /* A-Level Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlDWRRPrioEn, 0x00000000, 0x00015200)    /* A-Level DWRR Priority Enable for Queue level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlQuantum, 0x00000000, 0x00017200)    /* A-Level DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap, 0x00000000, 0x00019200)    /* A-Level Mapping: A-Level to B-Level Node And A-Level to Queue Range */
        SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFunc, 0x00000000, 0x0001b400)    /* Queue Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFuncPtr, 0x00000000, 0x0001c200)    /* Queue Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketTokenEnDiv, 0x00000000, 0x00024200)    /* Queue Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketBurstSize, 0x00000000, 0x0002c200)    /* Queue Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.QueueQuantum, 0x00000000, 0x00034200)    /* Queue DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.QueueAMap, 0x00000000, 0x0003c200)    /* Queue Mapping to A-Level Node */
        SET_REG_ADDRESS(addressSpace.Sched.PortShpBucketLvls, 0x00000000, 0x00044200)    /* Port Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlShpBucketLvls, 0x00000000, 0x00048e00)    /* C-Level Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.CLvlDef, 0x00000000, 0x00049a00)    /* C-Level DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlShpBucketLvls, 0x00000000, 0x0004a200)    /* B-Level Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlDef, 0x00000000, 0x0004ae00)    /* B-Level DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlShpBucketLvls, 0x00000000, 0x0004c200)    /* A-Level Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlDef, 0x00000000, 0x0004f200)    /* A-Level DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.QueueShpBucketLvls, 0x00000000, 0x00054200)    /* Queue Shaper Bucket Level */
        SET_REG_ADDRESS(addressSpace.Sched.QueueDef, 0x00000000, 0x00060200)    /* Queue DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.PortShaperBucketNeg, 0x00000000, 0x00044a00)    /* Port Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.CLevelShaperBucketNeg, 0x00000000, 0x00049600)    /* C-Level Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.BLevelShaperBucketNeg, 0x00000000, 0x0004aa00)    /* B-Level Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.ALevelShaperBucketNeg, 0x00000000, 0x0004e200)    /* A-Level Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.QueueShaperBucketNeg, 0x00000000, 0x0005c200)    /* Queue Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus67, 0x00000000, 0x00066e00)    /* Port RR/DWRR 67 Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus45, 0x00000000, 0x00066600)    /* Port RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus67, 0x00000000, 0x0006a200)    /* C Level RR/DWRR 67 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus45, 0x00000000, 0x00069a00)    /* C Level RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateHi, 0x00000000, 0x0006ab80)    /* C Level L0 Cluster State Hi */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus67, 0x00000000, 0x0006e200)    /* B Level RR/DWRR 67 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus45, 0x00000000, 0x0006da00)    /* B Level RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateHi, 0x00000000, 0x0006eb80)    /* B Level L0 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus67, 0x00000000, 0x0007a200)    /* A Level RR/DWRR 67 Status */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus45, 0x00000000, 0x00078200)    /* A Level RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateHi, 0x00000000, 0x0007c800)    /* A Level L0 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateHi, 0x00000000, 0x00089a00)    /* Queue L0 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMPortBPState, 0x00000000, 0x00067900)    /* Port Back-Pressure From TM-to-TM */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMBlvlBPState, 0x00000000, 0x0006ee00)    /* B Level TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus23, 0x00000000, 0x00076200)    /* A Level RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMAlvlBPState, 0x00000000, 0x0007d200)    /* A Level TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio1, 0x00000000, 0x00045600)    /* Port DWRR Deficit - Priority 1 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio7, 0x00000000, 0x00048600)    /* Port DWRR Deficit - Priority 7 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio2, 0x00000000, 0x00045e00)    /* Port DWRR Deficit - Priority 2 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio6, 0x00000000, 0x00047e00)    /* Port DWRR Deficit - Priority 6 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio4, 0x00000000, 0x00046e00)    /* Port DWRR Deficit - Priority 4 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio0, 0x00000000, 0x00044e00)    /* Port DWRR Deficit - Priority 0 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio3, 0x00000000, 0x00046600)    /* Port DWRR Deficit - Priority 3 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio5, 0x00000000, 0x00047600)    /* Port DWRR Deficit - Priority 5 */
}


