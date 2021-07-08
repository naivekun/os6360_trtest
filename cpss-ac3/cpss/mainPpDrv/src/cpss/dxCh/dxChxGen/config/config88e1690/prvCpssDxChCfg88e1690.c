/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChCfg88e1690.c
*
* @brief file to manage the 88e1690 PHY-MAC.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>
#include <gtOs/cpssOsTimer.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <msApi.h>
#include <msApiTypes.h>
#include <gtDrvSwRegs.h>

#define NOT_VALID_CNS   0xFFFFFFFF
#define cpuPort     9


/* TxQ to TX DMA mapping */
GT_STATUS prvCpssDxChPortMappingTxQPort2TxDMAMapSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
);


/**
* @internal prvCpssDxChCfg88e1690RemoteFcModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] modeFcHol                - CPSS_DXCH_PORT_FC_E  : set Flow Control mode
*                                      CPSS_DXCH_PORT_HOL_E : set HOL system mode
* @param[in] profileSet               - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[in] tcBitmap                 - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                      bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfg88e1690RemoteFcModeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT           modeFcHol,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                              tcBitmap
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;    /* register address */
    GT_U8       tc;
    GT_U8       entryIndex;
    GT_U32      index;
    PRV_CPSS_DXCH_PORT_INFO_STC             *portPtr;
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *connectedPhyMacInfoPtr;
    GT_U32      portMacNum; /* MAC number */
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tailDropProfileParams;
    CPSS_PORT_CNM_GENERATION_CONFIG_STC     cnmGenerationCfg;
    CPSS_PORT_CN_PROFILE_CONFIG_STC         cnProfileCfg;
    CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC  cnSamplentry;
    CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC  fbCalcCfg;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, cascadePortNum, portMacNum);
    CPSS_PARAM_CHECK_MAX_MAC(tcBitmap-1,BIT_8-1);

    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    connectedPhyMacInfoPtr = &portPtr->remotePhyMacInfoArr[cascadePortNum]->connectedPhyMacInfo;

    /* Set general system mode */
    rc = cpssDxChPortFcHolSysModeSet(devNum, modeFcHol);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (modeFcHol)
    {
    case CPSS_DXCH_PORT_FC_E:
/*
 Set TailDrop profile:

     Assign TailDrop profile to remote ports, different than current CPSS default:
     cpssDxChPortTxBindPortToDpSet(tdProfile)

     Configure thresholds:
     cpssDxChPortTxTailDropProfileSet(tdProfile, MaxDesc:800, MaxBuff:800)

     Configure TailDrop profile FC per TC configuration(for all TCs in tcBitMap):
     cpssDxChPortTx4TcTailDropProfileSet(tdProfile, tc, MaxDesc:50, MaxBuff:50)


     Configure Enable Shared Pool Usage (for all TCs in tcBitMap):
     cpssDxChPortTxTcSharedProfileEnableSet(tdProfile, tc, MODE_ALL)

     Configure Shared Pool to be used per TC:
     cpssDxChPortTxTcProfileSharedPoolSet(tdProfile, tc, shared pool per TC (same number as tc))

     Configure Shared Pool limits per shared pool (one shared pool per TC):
     cpssDxChPortTxSharedPoolLimitsSet(shared pool per TC, 512, 512)

 CN Profile per Queue Configuration:

     Configure thresholds per TC (for all TCs in tcBitMap)
     cpssDxChPortCnProfileQueueConfigSet(tdProfile, cnAware:true, threshold:25)
 */

        rc = cpssDxChPortTxTailDropProfileSet(devNum,
                                        profileSet,
                                        CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
                                        800,
                                        800);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsMemSet(&tailDropProfileParams, 0, sizeof(tailDropProfileParams));
        tailDropProfileParams.dp0MaxBuffNum = 50;
        tailDropProfileParams.dp1MaxBuffNum = 50;
        tailDropProfileParams.dp2MaxBuffNum = 50;
        tailDropProfileParams.dp0MaxDescrNum = 50;
        tailDropProfileParams.dp1MaxDescrNum = 50;
        tailDropProfileParams.dp2MaxDescrNum = 50;
        tailDropProfileParams.tcMaxBuffNum = 50;
        tailDropProfileParams.tcMaxDescrNum = 50;
        tailDropProfileParams.dp0MaxMCBuffNum = 50;
        tailDropProfileParams.dp1MaxMCBuffNum = 50;
        tailDropProfileParams.dp2MaxMCBuffNum = 50;
        tailDropProfileParams.sharedUcAndMcCountersDisable = GT_FALSE;

        cpssOsMemSet(&cnProfileCfg, 0, sizeof(cnProfileCfg));
        cnProfileCfg.cnAware = GT_TRUE;
        cnProfileCfg.threshold = 25;

        for (tc = 0; tc < 8; tc++)
        {
            if (((tcBitmap >> tc) & 0x01) == 0)
            {
                continue;
            }

            rc = cpssDxChPortTx4TcTailDropProfileSet(devNum,
                                            profileSet,
                                            tc,
                                            &tailDropProfileParams);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChPortTxTcSharedProfileEnableSet(devNum,
                                            profileSet,
                                            tc,
                                            CPSS_PORT_TX_SHARED_DP_MODE_ALL_E);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPortTxTcProfileSharedPoolSet(devNum,
                                            profileSet,
                                            tc,
                                            tc);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPortTxSharedPoolLimitsSet(devNum,
                                            tc,
                                            512,
                                            512);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPortCnProfileQueueConfigSet(devNum,
                                            profileSet,
                                            tc,
                                            &cnProfileCfg);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

/*
 BM Configuration:
     Set BM Global Xoff/Xon limits,
     cpssDxChGlobalXoffLimitSet(10000),
     cpssDxChGlobalXonLimitSet(7500)
 */

        /* xon/xoff limit API gets theshold in 2 buffer units */

        rc = cpssDxChGlobalXoffLimitSet(devNum, (10000 / 2));
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChGlobalXonLimitSet(devNum, (7500 / 2));
        if (rc != GT_OK)
        {
            return rc;
        }

/*
 QCN Configuration:
 Global configurations:

     CN Mode:
     cpssDxChPortCnModeEnableSet(Enable)

     Set CN Type to CCFC:
     cpssDxChPortCnMessageTypeSet(CCFC)

     Enable CNM triggering regardless of if the tagged state of sampled frame is CN-Tagged:
     cpssDxChPortCnMessageGenerationConfigSet(cnUntaggedEnable:true)
 */

        rc = cpssDxChPortCnModeEnableSet(devNum, CPSS_DXCH_PORT_CN_MODE_ENABLE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortCnMessageTypeSet(devNum, CPSS_DXCH_PORT_CN_MESSAGE_TYPE_CCFC_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortCnMessageGenerationConfigGet(devNum, &cnmGenerationCfg);
        if (rc != GT_OK)
        {
            return rc;
        }

        cnmGenerationCfg.scaleFactor = 512;
        cnmGenerationCfg.cnUntaggedEnable = GT_TRUE;
        rc = cpssDxChPortCnMessageGenerationConfigSet(devNum, &cnmGenerationCfg);
        if (rc != GT_OK)
        {
            return rc;
        }

/*
 Global CNM Triggering configurations:

     Sampling Interval for indices 0..7:
     cpssDxChPortCnSampleEntrySet(index, interval:0x690, randBitmap: 0)

     Feedback Calculations, Feedback Min/Max:
     cpssDxChPortCnFbCalcConfigSet(
   fbCalcCfg={
     deltaEnable=false,
     fbMin=8,
     fbMax=0x060,
     fbLsb=1,
     wExp=0
   }
 )
 */

        cpssOsMemSet(&cnSamplentry, 0, sizeof(cnSamplentry));
        cnSamplentry.interval = 0x690;
        cnSamplentry.randBitmap = 0;
        for (entryIndex = 0; entryIndex < 8; entryIndex++)
        {
            rc = cpssDxChPortCnSampleEntrySet(devNum, entryIndex, &cnSamplentry);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        cpssOsMemSet(&fbCalcCfg, 0, sizeof(fbCalcCfg));
        fbCalcCfg.wExp = 0;
        fbCalcCfg.fbLsb = 1;
        fbCalcCfg.deltaEnable = GT_FALSE;
        fbCalcCfg.fbMin = 8;
        fbCalcCfg.fbMax = 0x060;
        rc = cpssDxChPortCnFbCalcConfigSet(devNum, &fbCalcCfg);
        if (rc != GT_OK)
        {
            return rc;
        }

/*
 RXAUI Cascade Ports between AC3X and E1690:

     For all remoting cascade ports Z:
     Configure FCA "PFCPriorityMask" to allow pausing TCs 0-6
     cpssDrvPpHwRegBitMaskWrite (regAddr: 0x10180704+Z*0x1000, mask:0xFF, value:0x80)

     Configure FCA "DSATag" to look like a PFC packet
     cpssDrvPpHwRegBitMaskWrite (regAddr: 0x10180674+Z*0x1000, mask:0 , value:0x0101)
     cpssDrvPpHwRegBitMaskWrite (regAddr: 0x10180678+Z*0x1000, mask:0 , value:0x8808)

     Configure FCA Periodic PFC:
     cpssDxChPortPeriodicFlowControlCounterSet(Z, 1700)

     Configure FCA mode to PFC:
     cpssDxChPortFlowControlModeSet(Z, PFC)

     Enable PFC:
     cpssDxChPortFlowControlEnableSet(Z, TX)
 */

        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).PFCPriorityMask;
        rc = cpssDrvPpHwRegBitMaskWrite(devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        regAddr,
                                        0xFF,
                                        0x80);
        if (rc != GT_OK)
        {
            return rc;
        }
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).DSATag0To15;
        rc = cpssDrvPpHwRegBitMaskWrite(devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        regAddr,
                                        0xffff,
                                        0x0101);
        if (rc != GT_OK)
        {
            return rc;
        }
        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).DSATag16To31;
        rc = cpssDrvPpHwRegBitMaskWrite(devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        regAddr,
                                        0xffff,
                                        0x8808);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortPeriodicFlowControlCounterSet(devNum,
                                        cascadePortNum,
                                        1700);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortPeriodicFcEnableSet(devNum,
                                        cascadePortNum,
                                        CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortFlowControlModeSet(devNum,
                                        cascadePortNum,
                                        CPSS_DXCH_PORT_FC_MODE_PFC_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPortPfcCascadeEnableSet(devNum,
                                        cascadePortNum,
                                        GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortFlowControlEnableSet(devNum,
                                        cascadePortNum,
                                        CPSS_PORT_FLOW_CONTROL_TX_ONLY_E);
        if (rc != GT_OK)
        {
            return rc;
        }

/*
    Assign newly created TD profile to all remote ports served by this cascade port.
 */
        for (index = 0; index < connectedPhyMacInfoPtr->numOfRemotePorts; index++)
        {
            rc = cpssDxChPortTxBindPortToDpSet(devNum,
                                    connectedPhyMacInfoPtr->remotePortInfoArr[index].remotePhysicalPortNum,
                                    profileSet);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        break;

    case CPSS_DXCH_PORT_HOL_E:

/*
    Revert TD profile to default on all remote ports served by this cascade port.
 */
        for (index = 0; index < connectedPhyMacInfoPtr->numOfRemotePorts; index++)
        {
            rc = cpssDxChPortTxBindPortToDpSet(devNum,
                                    connectedPhyMacInfoPtr->remotePortInfoArr[index].remotePhysicalPortNum,
                                    CPSS_PORT_TX_DROP_PROFILE_3_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

/*
    Revert FC enable to default (disabled - no periodic) on cascade RXAUI port.
 */
        rc = cpssDxChPortFlowControlEnableSet(devNum,
                                        cascadePortNum,
                                        CPSS_PORT_FLOW_CONTROL_DISABLE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortPeriodicFcEnableSet(devNum,
                                        cascadePortNum,
                                        CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).PFCPriorityMask;
        rc = cpssDrvPpHwRegBitMaskWrite(devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        regAddr,
                                        0xFF,
                                        0xFF);
        if (rc != GT_OK)
        {
            return rc;
        }

/*
    Revert QCN mode to default (disabled).
 */
        rc = cpssDxChPortCnModeEnableSet(devNum,
                                        CPSS_DXCH_PORT_CN_MODE_DISABLE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

/*
     Revert BM Global Xoff/Xon limits to 32K-1 pairs of buffers.
 */

        rc = cpssDxChGlobalXoffLimitSet(devNum, _32K-1);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChGlobalXonLimitSet(devNum, _32K-1);
        if (rc != GT_OK)
        {
            return rc;
        }

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    }

    return rc;
}

/**
* @internal prvCpssDxChCfgPort88e1690CascadePfcParametersSet function
* @endinternal
*
* @brief   Set PFC parameters per remoting cascade port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] timer                    - PFC  (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] tcBitmap                 - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note PFC is sent in uplink direction from remote port.
*
*/
GT_STATUS prvCpssDxChCfgPort88e1690CascadePfcParametersSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    cascadePortNum,
    IN  GT_U32                  xOffThreshold,
    IN  GT_U32                  xOnThreshold,
    IN  GT_U32                  timer,
    IN  GT_U32                  tcBitmap
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;
    GT_CPSS_QD_DEV       *driverInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(xOffThreshold,BIT_8);
    CPSS_PARAM_CHECK_MAX_MAC(xOnThreshold,xOffThreshold);
    CPSS_PARAM_CHECK_MAX_MAC(timer,BIT_16);
    CPSS_PARAM_CHECK_MAX_MAC(tcBitmap-1,BIT_8-1);

    /* . */
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[cascadePortNum];
    if(remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Write command to firmware in ASCII string */
    /* Write op-code */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, 'X');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '*');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write Xoff threshold */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((xOffThreshold >> 4) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((xOffThreshold >> 0) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write xOn threshold */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((xOnThreshold >> 4) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((xOnThreshold >> 0) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write timer value */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >> 12) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >>  8) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >>  4) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >>  0) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write PEV value */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((tcBitmap >>  4) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((tcBitmap >>  0) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCfgPort88e1690CascadePfcParametersGet function
* @endinternal
*
* @brief   Get PFC parameters per remoting cascade port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
*
* @param[out] xOffThresholdPtr         - xOff threshold in buffers
* @param[out] xOnThresholdPtr          - xOn threshold in buffers
* @param[out] timerPtr                 - PFC timer
* @param[out] tcBitmapPtr              - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfgPort88e1690CascadePfcParametersGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    cascadePortNum,
    OUT GT_U32                  *xOffThresholdPtr,
    OUT GT_U32                  *xOnThresholdPtr,
    OUT GT_U32                  *timerPtr,
    OUT GT_U32                  *tcBitmapPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;
#ifdef ASIC_SIMULATION
#else   /* ASIC_SIMULATION */
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_U8                   data;
#endif  /* ASIC_SIMULATION */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Check cascade port */
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[cascadePortNum];
    if(remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

#ifdef ASIC_SIMULATION

    /* Firmware is not implemented at simulation, so that prvCpssDrvImpReadComm() times out */
    *xOffThresholdPtr = 0;
    *xOnThresholdPtr = 0;
    *timerPtr = 0xFFFF;
    *tcBitmapPtr = 0xFF;

#else   /* ASIC_SIMULATION */

    /* Get the Phy driver information */
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Write command to firmware in ASCII string */
    /* Write op-code */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, 'X');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '?');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Read command response from firmware in ASCII string */
    /* Read Xoff threshold */
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *xOffThresholdPtr  = (data <<  4) & 0x00f0;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *xOffThresholdPtr |= (data <<  0) & 0x000f;
    /* Read Xon threshold */
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *xOnThresholdPtr  = (data <<  4) & 0x00f0;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *xOnThresholdPtr |= (data <<  0) & 0x000f;
    /* Read timer */
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *timerPtr  = (data << 12) & 0xf000;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *timerPtr |= (data <<  8) & 0x0f00;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *timerPtr |= (data <<  4) & 0x00f0;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *timerPtr |= (data <<  0) & 0x000f;
    /* Read PEV value */
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *tcBitmapPtr  = (data <<  4) & 0x00f0;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    *tcBitmapPtr |= (data <<  0) & 0x000f;

#endif   /* ASIC_SIMULATION */

    return rc;
}

/**
* @internal prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketInit function
* @endinternal
*
* @brief   Init port statistics MAC counter set retrieval by CPU packet.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] cpuCode                  - CPU code used to identify packet as MAC counter packet.
*                                      Recommended value in User Defined range, however
*                                      any code which can be translated to HW CPU Code is allowed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function must not be called before remote ports are bound.
*       The CPU code is saved for later use.
*
*/
static GT_STATUS prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketInit
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        cascadePortNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT      dsaCpuCode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(cascadePortNum,PRV_CPSS_MAX_MAC_PORTS_NUM_CNS);

    /* Obtain remote Phy information */
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[cascadePortNum];
    if(remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Check CPU code */
    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &dsaCpuCode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Write command to firmware in ASCII string */
    /* Write op-code */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, 'I');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write cascade port number (used as unique PHY device id) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((cascadePortNum >> 4) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((cascadePortNum >> 0) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write HW CPU code */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((dsaCpuCode >> 4) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((dsaCpuCode >> 0) & 0x0F));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }

    /* Save the CPU code */
    remotePhyMacInfoPtr->connectedPhyMacInfo.mibCounterReadByPacketCpuCode = cpuCode;

    return rc;
}

/**
* @internal prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketParse function
* @endinternal
*
* @brief   Parses packet containing Ethernet MAC counters.
*         The result is stored at shadow buffers, and it can be retrieved
*         by function cpssDxChPortMacCountersCaptureOnPortGet()
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfBuff                - Num of used buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The input parameters are obtained by function cpssDxChNetIfSdmaRxPacketGet()
*       The output parameter are the parsed port number and MAC counter values.
*
*/
GT_STATUS prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                          *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  CPSS_DXCH_NET_RX_PARAMS_STC    *rxParamsPtr
)
{
    GT_STATUS   rc = GT_OK;

    CPSS_NET_RX_CPU_CODE_ENT                    cpuCode;
    GT_U16                                      etherType;
    GT_PHYSICAL_PORT_NUM                        cascadePortNum;
    GT_U32                                      localMacPortNum;
    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC       *prvRemoteMacInfoArr;
    CPSS_PORT_MAC_COUNTER_SET_STC               *captureMibShadowPtr;
    CPSS_PORT_MAC_COUNTER_SET_STC               *mibShadowPtr;
    GT_U32                                      i;
    GT_U32                                      buffIndex;      /* buffer index */
    GT_U32                                      buffStartIndex; /* byte index of start of current buffer */
    GT_U32                                      buffEndIndex;   /* byte index of end of current buffer */
    GT_U32                                      byteIndex;      /* byte index into current buffer */
    GT_U32                                      dummy;

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return /* silently */ GT_BAD_PARAM;
    }

    /* Obtain the CPU code */
    cpuCode = rxParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode;

    /* Start packet parsing: look for ET */
#define PRV_GET_NEXT_BYTE()   ((byteIndex < buffLenArr[buffIndex]) ? \
                                    (packetBuffsArrPtr[buffIndex][byteIndex++]) : \
                                    (buffIndex < (numOfBuff-1)) ? \
                                        (packetBuffsArrPtr[++buffIndex][byteIndex = 0]) : \
                                        0)
    byteIndex = 0;
    buffStartIndex = 0;
    for (buffIndex = 0; buffIndex < numOfBuff; buffIndex++)
    {
        buffEndIndex = buffStartIndex + buffLenArr[buffIndex];
        if (buffEndIndex > 12)
        {
            byteIndex = 12 - buffStartIndex;
            break;
        }
        buffStartIndex = buffEndIndex;
    }
    if(buffIndex == numOfBuff)
    {
        /* Extremely short buffer */
        return /* silently */ GT_BAD_STATE;
    }

    /* Check the Ethertype */
    etherType  = ((PRV_GET_NEXT_BYTE() <<  8));
    etherType |= ((PRV_GET_NEXT_BYTE() <<  0));
    if(etherType != 0xCCCC)
    {
        /* Not our packet */
        return /* silently */ GT_NO_SUCH;
    }

    /* Obtain the cascade port number (i.e. unique device id.) */
    cascadePortNum = PRV_GET_NEXT_BYTE();

    /* Obtain remote Phy information */
    if(cascadePortNum >= PRV_CPSS_MAX_MAC_PORTS_NUM_CNS)
    {
        return /* silently */ GT_BAD_PARAM;
    }
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[cascadePortNum];
    if(remotePhyMacInfoPtr == NULL)
    {
        return /* silently */ GT_BAD_STATE;
    }
    if (remotePhyMacInfoPtr->connectedPhyMacInfo.useMibCounterReadByPacket == GT_FALSE)
    {
        /* Device is not configured for read MIB counters by packet */
        return /* silently */ GT_NO_SUCH;
    }
    if (remotePhyMacInfoPtr->connectedPhyMacInfo.mibCounterReadByPacketCpuCode != cpuCode)
    {
        /* Not our CPU code */
        return /* silently */ GT_NO_SUCH;
    }

    /* Obtain local MAC port number */
    localMacPortNum = PRV_GET_NEXT_BYTE();
    for (i = 0; i < remotePhyMacInfoPtr->connectedPhyMacInfo.numOfRemotePorts; i++)
    {
        CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC *remotePortInfoArr;

        remotePortInfoArr = &remotePhyMacInfoPtr->connectedPhyMacInfo.remotePortInfoArr[i];
        if (remotePortInfoArr->remoteMacPortNum == localMacPortNum)
        {
            break;
        }
    }
    if(i == remotePhyMacInfoPtr->connectedPhyMacInfo.numOfRemotePorts)
    {
        return /* silently */ GT_NO_SUCH;
    }

    /* Check the read counters operation shadow information */
    prvRemoteMacInfoArr = &remotePhyMacInfoPtr->prvRemoteMacInfoArr[localMacPortNum];
    if(prvRemoteMacInfoArr == NULL)
    {
        return /* silently */ GT_NO_SUCH;
    }
    captureMibShadowPtr = &prvRemoteMacInfoArr->captureMibShadow;
    mibShadowPtr = &prvRemoteMacInfoArr->mibShadow;

    /* Skip reserved bytes */
    dummy = PRV_GET_NEXT_BYTE();
    dummy = PRV_GET_NEXT_BYTE();

    /* Packet parsing: go over all retrieved counters */
#define PRV_GET_NEXT_COUNTER(_counter)  \
                _counter  = ((PRV_GET_NEXT_BYTE() <<  0));    \
                _counter |= ((PRV_GET_NEXT_BYTE() <<  8));    \
                _counter |= ((PRV_GET_NEXT_BYTE() << 16));    \
                _counter |= ((PRV_GET_NEXT_BYTE() << 24));

    cpssOsMemSet (captureMibShadowPtr, 0, sizeof(*captureMibShadowPtr));
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->goodOctetsRcv.l[0]) /* statsCounterSet.InGoodOctetsLo; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->goodOctetsRcv.l[1]) /* statsCounterSet.InGoodOctetsHi; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->badOctetsRcv.l[0]) /* statsCounterSet.InBadOctets; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->macTransmitErr.l[0]) /* statsCounterSet.OutFCSErr; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->ucPktsRcv.l[0]) /* statsCounterSet.InUnicasts; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->deferredPktsSent.l[0]) /* statsCounterSet.Deferred; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->brdcPktsRcv.l[0]) /* statsCounterSet.InBroadcasts; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->mcPktsRcv.l[0]) /* statsCounterSet.InMulticasts; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->pkts64Octets.l[0]) /* statsCounterSet.Octets64; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->pkts65to127Octets.l[0]) /* statsCounterSet.Octets127; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->pkts128to255Octets.l[0]) /* statsCounterSet.Octets255; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->pkts256to511Octets.l[0]) /* statsCounterSet.Octets511; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->pkts512to1023Octets.l[0]) /* statsCounterSet.Octets1023; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->pkts1024tomaxOoctets.l[0]) /* statsCounterSet.OctetsMax; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->goodOctetsSent.l[0]) /* statsCounterSet.OutOctetsLo; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->goodOctetsSent.l[1]) /* statsCounterSet.OutOctetsHi; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->ucPktsSent.l[0]) /* statsCounterSet.OutUnicasts; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->excessiveCollisions.l[0]) /* statsCounterSet.Excessive; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->mcPktsSent.l[0]) /* statsCounterSet.OutMulticasts; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->brdcPktsSent.l[0]) /* statsCounterSet.OutBroadcasts; */
    PRV_GET_NEXT_COUNTER(dummy) /* statsCounterSet.Single; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->fcSent.l[0]) /* statsCounterSet.OutPause; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->goodFcRcv.l[0]) /* statsCounterSet.InPause; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->multiplePktsSent.l[0]) /* statsCounterSet.Multiple; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->undersizePkts.l[0]) /* statsCounterSet.InUndersize; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->fragmentsPkts.l[0]) /* statsCounterSet.InFragments; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->oversizePkts.l[0]) /* statsCounterSet.InOversize; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->jabberPkts.l[0]) /* statsCounterSet.InJabber; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->macRcvError.l[0]) /* statsCounterSet.InRxErr; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->badCrc.l[0]) /* statsCounterSet.InFCSErr; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->collisions.l[0]) /* statsCounterSet.Collisions; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->lateCollisions.l[0]) /* statsCounterSet.Late; */
    PRV_GET_NEXT_COUNTER(captureMibShadowPtr->dropEvents.l[0]) /* statsCounterSet.InDiscards; */
    /* Packet parsing ended */

    /* Adjust counter value according to Clear on Read emulation */
    captureMibShadowPtr->goodOctetsRcv = prvCpssMathSub64(
                        captureMibShadowPtr->goodOctetsRcv,
                        mibShadowPtr->goodOctetsRcv);
    captureMibShadowPtr->badOctetsRcv.l[0] -= mibShadowPtr->badOctetsRcv.l[0];
    captureMibShadowPtr->macTransmitErr.l[0] -= mibShadowPtr->macTransmitErr.l[0];
    captureMibShadowPtr->brdcPktsRcv.l[0] -= mibShadowPtr->brdcPktsRcv.l[0];
    captureMibShadowPtr->mcPktsRcv.l[0] -= mibShadowPtr->mcPktsRcv.l[0];
    captureMibShadowPtr->pkts64Octets.l[0] -= mibShadowPtr->pkts64Octets.l[0];
    captureMibShadowPtr->pkts65to127Octets.l[0] -= mibShadowPtr->pkts65to127Octets.l[0];
    captureMibShadowPtr->pkts128to255Octets.l[0] -= mibShadowPtr->pkts128to255Octets.l[0];
    captureMibShadowPtr->pkts256to511Octets.l[0] -= mibShadowPtr->pkts256to511Octets.l[0];
    captureMibShadowPtr->pkts512to1023Octets.l[0] -= mibShadowPtr->pkts512to1023Octets.l[0];
    captureMibShadowPtr->pkts1024tomaxOoctets.l[0] -= mibShadowPtr->pkts1024tomaxOoctets.l[0];
    captureMibShadowPtr->goodOctetsSent = prvCpssMathSub64(
                        captureMibShadowPtr->goodOctetsSent,
                        mibShadowPtr->goodOctetsSent);
    captureMibShadowPtr->excessiveCollisions.l[0] -= mibShadowPtr->excessiveCollisions.l[0];
    captureMibShadowPtr->mcPktsSent.l[0] -= mibShadowPtr->mcPktsSent.l[0];
    captureMibShadowPtr->brdcPktsSent.l[0] -= mibShadowPtr->brdcPktsSent.l[0];
    captureMibShadowPtr->fcSent.l[0] -= mibShadowPtr->fcSent.l[0];
    captureMibShadowPtr->goodFcRcv.l[0] -= mibShadowPtr->goodFcRcv.l[0];
    captureMibShadowPtr->dropEvents.l[0] -= mibShadowPtr->dropEvents.l[0];
    captureMibShadowPtr->undersizePkts.l[0] -= mibShadowPtr->undersizePkts.l[0];
    captureMibShadowPtr->fragmentsPkts.l[0] -= mibShadowPtr->fragmentsPkts.l[0];
    captureMibShadowPtr->oversizePkts.l[0] -= mibShadowPtr->oversizePkts.l[0];
    captureMibShadowPtr->jabberPkts.l[0] -= mibShadowPtr->jabberPkts.l[0];
    captureMibShadowPtr->macRcvError.l[0] -= mibShadowPtr->macRcvError.l[0];
    captureMibShadowPtr->badCrc.l[0] -= mibShadowPtr->badCrc.l[0];
    captureMibShadowPtr->collisions.l[0] -= mibShadowPtr->collisions.l[0];
    captureMibShadowPtr->lateCollisions.l[0] -= mibShadowPtr->lateCollisions.l[0];
    captureMibShadowPtr->ucPktsRcv.l[0] -= mibShadowPtr->ucPktsRcv.l[0];
    captureMibShadowPtr->ucPktsSent.l[0] -= mibShadowPtr->ucPktsSent.l[0];
    captureMibShadowPtr->multiplePktsSent.l[0] -= mibShadowPtr->multiplePktsSent.l[0];
    captureMibShadowPtr->deferredPktsSent.l[0] -= mibShadowPtr->deferredPktsSent.l[0];

    /* Save shadow counter value according to Clear on Read emulation*/
    if (prvRemoteMacInfoArr->clearOnReadEnable)
    {
        mibShadowPtr->goodOctetsRcv = prvCpssMathAdd64(
                            mibShadowPtr->goodOctetsRcv,
                            captureMibShadowPtr->goodOctetsRcv);
        mibShadowPtr->badOctetsRcv.l[0] += captureMibShadowPtr->badOctetsRcv.l[0];
        mibShadowPtr->macTransmitErr.l[0] += captureMibShadowPtr->macTransmitErr.l[0];
        mibShadowPtr->brdcPktsRcv.l[0] += captureMibShadowPtr->brdcPktsRcv.l[0];
        mibShadowPtr->mcPktsRcv.l[0] += captureMibShadowPtr->mcPktsRcv.l[0];
        mibShadowPtr->pkts64Octets.l[0] += captureMibShadowPtr->pkts64Octets.l[0];
        mibShadowPtr->pkts65to127Octets.l[0] += captureMibShadowPtr->pkts65to127Octets.l[0];
        mibShadowPtr->pkts128to255Octets.l[0] += captureMibShadowPtr->pkts128to255Octets.l[0];
        mibShadowPtr->pkts256to511Octets.l[0] += captureMibShadowPtr->pkts256to511Octets.l[0];
        mibShadowPtr->pkts512to1023Octets.l[0] += captureMibShadowPtr->pkts512to1023Octets.l[0];
        mibShadowPtr->pkts1024tomaxOoctets.l[0] += captureMibShadowPtr->pkts1024tomaxOoctets.l[0];
        mibShadowPtr->goodOctetsSent = prvCpssMathAdd64(
                            mibShadowPtr->goodOctetsSent,
                            captureMibShadowPtr->goodOctetsSent);
        mibShadowPtr->excessiveCollisions.l[0] += captureMibShadowPtr->excessiveCollisions.l[0];
        mibShadowPtr->mcPktsSent.l[0] += captureMibShadowPtr->mcPktsSent.l[0];
        mibShadowPtr->brdcPktsSent.l[0] += captureMibShadowPtr->brdcPktsSent.l[0];
        mibShadowPtr->fcSent.l[0] += captureMibShadowPtr->fcSent.l[0];
        mibShadowPtr->goodFcRcv.l[0] += captureMibShadowPtr->goodFcRcv.l[0];
        mibShadowPtr->dropEvents.l[0] += captureMibShadowPtr->dropEvents.l[0];
        mibShadowPtr->undersizePkts.l[0] += captureMibShadowPtr->undersizePkts.l[0];
        mibShadowPtr->fragmentsPkts.l[0] += captureMibShadowPtr->fragmentsPkts.l[0];
        mibShadowPtr->oversizePkts.l[0] += captureMibShadowPtr->oversizePkts.l[0];
        mibShadowPtr->jabberPkts.l[0] += captureMibShadowPtr->jabberPkts.l[0];
        mibShadowPtr->macRcvError.l[0] += captureMibShadowPtr->macRcvError.l[0];
        mibShadowPtr->badCrc.l[0] += captureMibShadowPtr->badCrc.l[0];
        mibShadowPtr->collisions.l[0] += captureMibShadowPtr->collisions.l[0];
        mibShadowPtr->lateCollisions.l[0] += captureMibShadowPtr->lateCollisions.l[0];
        mibShadowPtr->ucPktsRcv.l[0] += captureMibShadowPtr->ucPktsRcv.l[0];
        mibShadowPtr->ucPktsSent.l[0] += captureMibShadowPtr->ucPktsSent.l[0];
        mibShadowPtr->multiplePktsSent.l[0] += captureMibShadowPtr->multiplePktsSent.l[0];
        mibShadowPtr->deferredPktsSent.l[0] += captureMibShadowPtr->deferredPktsSent.l[0];
     }

    /* Mark capture Mib shadow as valid */
    prvRemoteMacInfoArr->captureMibShadowValid = GT_TRUE;

    return rc;
}

/**
* @internal prvCpssDxChCfg88e1690ReDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on error
*
* @note This function is called under ISR lock.
*
*/
GT_STATUS prvCpssDxChCfg88e1690ReDevNum
(
    IN  GT_U8 oldDevNum,
    IN  GT_U8 newDevNum
)
{
    GT_U32  port;
    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr = &PRV_CPSS_DXCH_PP_MAC(newDevNum)->port;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;

    /* Walk over all remote cascading ports to change their device number */
    for(port = 0 ; port < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; port++)
    {
        remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[port];
        if(remotePhyMacInfoPtr == NULL)
        {
            continue;
        }

        if (remotePhyMacInfoPtr->dxDevNum != oldDevNum)
        {
            /* Should be the old Dev number there */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Change it to the new Dev number */
        remotePhyMacInfoPtr->dxDevNum = newDevNum;
    }

    return GT_OK;
}

/*******************************************************************************
* prvCpssDxChCfgPort88e1690CascadePortToInfoGet
*
* DESCRIPTION:
*       get the 88e1690 info that is on the DX device on the cascade port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       physicalPort  - the physical port number (of the DX cascade port)
*
* OUTPUTS:
*       None.
* RETURNS:
*       pointer to the '88e1690' info.
*       NULL - if not found
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC*  prvCpssDxChCfgPort88e1690CascadePortToInfoGet(
    IN GT_U8    devNum,
    IN GT_U32 physicalPort
)
{
    return PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[physicalPort];
}

/*******************************************************************************
* prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet
*
* DESCRIPTION:
*       get info about 88e1690 that hold the DX remote physical port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       remotePhysicalPortNum  - the remote physical port number
*
* OUTPUTS:
*       portIndexPtr - (pointer to) the port index in the array of remotePortInfoArr[]
* RETURNS:
*       pointer to the '88e1690' info.
*       NULL - if 'remotePhysicalPortNum' not found
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC*  prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet(
    IN GT_U8    devNum,
    IN GT_U32   remotePhysicalPortNum,
    OUT GT_U32  *portIndexPtr
)
{
    GT_U32  port,ii;
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr;

    for(port = 0 ; port < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; port++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[port] == NULL)
        {
            continue;
        }

        infoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[port]->connectedPhyMacInfo;

        for(ii = 0 ; ii < infoPtr->numOfRemotePorts;  ii++)
        {
            if(infoPtr->remotePortInfoArr[ii].remotePhysicalPortNum == remotePhysicalPortNum)
            {
                *portIndexPtr = ii;
                return PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[port];
            }
        }
    }

    return NULL;
}




/* control register of SMI on the 88E1690 */
#define SMI_CTRL_ON_88E1690_REG_ADDR        0x0
/* data register of SMI on the 88E1690 */
#define SMI_VALUE_ON_88E1690_REG_ADDR       0x1


/**
* @internal waitFor88E1690SmiReady function
* @endinternal
*
* @brief   wait for 88E1690 to be ready for SMI operations.
*
* @param[in] devNum                   - The device number.
* @param[in] internal_infoPtr         - the physical info.
* @param[in] onlyCheck                - do we want only to check the current status , or to wait
*                                      until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the value until value reached
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the value was not reached
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS    waitFor88E1690SmiReady(
    IN GT_U8    devNum ,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC   *internal_infoPtr,
    IN GT_BOOL  onlyCheck
)
{
    GT_STATUS rc;
    volatile GT_U32      busyIterator; /* busy wait iterator */
    GT_U16      reg88E1690Val;
    GT_U16      regMask = 0x8000;
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr = &internal_infoPtr->connectedPhyMacInfo;

    busyIterator = 0;
    /* use "busy" wait */

    do{
        /* read the control register from the 88E1690 */
        rc = cpssSmiRegisterReadShort(devNum,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_PHY_SMI_INTERFACE_0_E + infoPtr->mngBusNumber,
            (GT_U32)(infoPtr->busBaseAddr),
            SMI_CTRL_ON_88E1690_REG_ADDR, /* control register */
            &reg88E1690Val);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* check that number of iteration not over the limit */
        PRV_CPSS_SMI_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,(busyIterator++),GT_TIMEOUT);

    }while((reg88E1690Val & regMask) && (onlyCheck == GT_FALSE));

    if(reg88E1690Val & regMask)
    {
        /* the case of (onlyCheck == GT_FALSE) would have already returned GT_TIMEOUT
            from the macro of PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS */
        return /* do not register as error (up to the caller to register error if needed) */ GT_BAD_STATE;
    }

    return GT_OK;

}


/**
* @internal writeTo88E1690OnSmi function
* @endinternal
*
* @brief   write to 88E1690 specific register.
*
* @param[in] devNum                   - The device number.
* @param[in] internal_infoPtr         - the physical info.
* @param[in] smiDevAddr               - 0..10 – ports , 0x1b – global 1 , 0x1c – global2 ...
*                                      (0..31)
* @param[in] regOffset                - register offset (0..31)
* @param[in] regData                  - register data
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - previous SMI DX transaction not finished
* @retval GT_HW_ERROR              - HW error
* @retval GT_NOT_INITIALIZED       - The driver was not initialized or
*                                       SMI ctrl register callback not registered
* @retval GT_TIMEOUT               - previous SMI 88E1690 transaction not finished
*/
static GT_STATUS   writeTo88E1690OnSmi
(
    IN GT_U8    devNum ,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC   *internal_infoPtr,
    IN GT_U32   smiDevAddr ,
    IN GT_U32   regOffset,
    IN GT_U16   regData
)
{
    GT_STATUS rc;
    GT_U16      reg88E1690Val;
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr = &internal_infoPtr->connectedPhyMacInfo;

    if(smiDevAddr > 31 || regOffset > 31)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = waitFor88E1690SmiReady(devNum,internal_infoPtr,GT_FALSE/* wait for 88E1690 to be ready */);
    if(rc != GT_OK)
    {
        return rc;
    }

    reg88E1690Val = regData;
    /* 1. we write to the data register of the 88E1690 the 'data to write' */
    rc = cpssSmiRegisterWriteShort(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        CPSS_PHY_SMI_INTERFACE_0_E + infoPtr->mngBusNumber,
        (GT_U32)(infoPtr->busBaseAddr),
        SMI_VALUE_ON_88E1690_REG_ADDR, /* data register */
        reg88E1690Val);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* 2. we write to the control register of the 88E1690 the 'write operation'
        and smiDevAddr and register addr info */
    reg88E1690Val = (1 << 12) |/*1 = Generate IEEE 802.3 Clause 22 SMI frames*/
                    (1 << 10) | /*Write Data Register*/
                    ((smiDevAddr & 0x1F) << 5)    |
                    ((regOffset & 0x1F) << 0) |
                    (1 << 15) ;/* trigger operation*/


    rc = cpssSmiRegisterWriteShort(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        CPSS_PHY_SMI_INTERFACE_0_E + infoPtr->mngBusNumber,
        (GT_U32)(infoPtr->busBaseAddr),
        SMI_CTRL_ON_88E1690_REG_ADDR,/* control register */
        reg88E1690Val);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal writeTo88E1690OnPort function
* @endinternal
*
* @brief   write to 88E1690 specific register.
*
* @param[in] devNum                   - The device number.
* @param[in] physicalPort             - the physical number.
* @param[in] smiDevAddr               - 0..10 – ports , 0x1b – global 1 , 0x1c – global2 ...
*                                      (0..31)
* @param[in] regOffset                - register offset (0..31)
* @param[in] regData                  - register data
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - previous SMI DX transaction not finished
* @retval GT_HW_ERROR              - HW error
* @retval GT_NOT_INITIALIZED       - The driver was not initialized or
*                                       SMI ctrl register callback not registered
* @retval GT_TIMEOUT               - previous SMI 88E1690 transaction not finished
*/
GT_STATUS   writeTo88E1690OnPort
(
    IN GT_U8    devNum ,
    IN GT_U32   physicalPort ,
    IN GT_U32   smiDevAddr ,
    IN GT_U32   regOffset,
    IN GT_U16   regData
)
{
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,physicalPort);
    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return writeTo88E1690OnSmi(devNum,portInfoPtr,
        smiDevAddr,regOffset,regData);
}


/**
* @internal readFrom88E1690OnSmi function
* @endinternal
*
* @brief   Read from the 88E1690 specific register.
*
* @param[in] devNum                   - The device number.
* @param[in] internal_infoPtr         - the physical info.
* @param[in] smiDevAddr               - 0..10 – ports , 0x1b – global 1 , 0x1c – global2 ...
*                                      (0..31)
* @param[in] regOffset                - register offset (0..31)
*
* @param[out] regDataPtr               - (pointer to)register data
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - previous SMI DX transaction not finished
* @retval GT_HW_ERROR              - HW error
* @retval GT_NOT_INITIALIZED       - The driver was not initialized or
*                                       SMI ctrl register callback not registered
* @retval GT_TIMEOUT               - previous SMI 88E1690 transaction not finished
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS   readFrom88E1690OnSmi
(
    IN GT_U8    devNum ,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC   *internal_infoPtr,
    IN GT_U32   smiDevAddr ,
    IN GT_U32   regOffset,
    IN GT_U16   *regDataPtr
)
{
    GT_STATUS rc;
    GT_U16      reg88E1690Val;
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr = &internal_infoPtr->connectedPhyMacInfo;

    CPSS_NULL_PTR_CHECK_MAC(regDataPtr);
    if(smiDevAddr > 31 || regOffset > 31)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    waitFor88E1690SmiReady(devNum,internal_infoPtr,GT_FALSE/* wait for 88E1690 to be ready */);

    /* 1. we write to the control register of the 88E1690 the 'read operation'
        and smiDevAddr and register addr info */
    reg88E1690Val = (1 << 12) |/*1 = Generate IEEE 802.3 Clause 22 SMI frames*/
                    (2 << 10) | /*Read Data Register*/
                    ((smiDevAddr & 0x1F) << 5)    |
                    ((regOffset & 0x1F) << 0) |
                    (1 << 15) ;/* trigger operation*/

    rc = cpssSmiRegisterWriteShort(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        CPSS_PHY_SMI_INTERFACE_0_E + infoPtr->mngBusNumber,
        (GT_U32)(infoPtr->busBaseAddr),
        SMI_CTRL_ON_88E1690_REG_ADDR,/* control register */
        reg88E1690Val);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* 2. we read the data register from the 88E1690 the 'data to read' */
    rc = cpssSmiRegisterReadShort(devNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        CPSS_PHY_SMI_INTERFACE_0_E + infoPtr->mngBusNumber,
        (GT_U32)(infoPtr->busBaseAddr),
        SMI_VALUE_ON_88E1690_REG_ADDR, /* data register */
        &reg88E1690Val);
    if(rc != GT_OK)
    {
        return rc;
    }


    *regDataPtr = reg88E1690Val;

    return GT_OK;
}

/**
* @internal readFrom88E1690OnPort function
* @endinternal
*
* @brief   Read from the 88E1690 specific register.
*
* @param[in] devNum                   - The device number.
* @param[in] physicalPort             - the physical port number.
* @param[in] smiDevAddr               - 0..10 – ports , 0x1b – global 1 , 0x1c – global2 ...
*                                      (0..31)
* @param[in] regOffset                - register offset (0..31)
*
* @param[out] regDataPtr               - (pointer to)register data
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - previous SMI DX transaction not finished
* @retval GT_HW_ERROR              - HW error
* @retval GT_NOT_INITIALIZED       - The driver was not initialized or
*                                       SMI ctrl register callback not registered
* @retval GT_TIMEOUT               - previous SMI 88E1690 transaction not finished
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS   readFrom88E1690OnPort
(
    IN GT_U8    devNum ,
    IN GT_U32   physicalPort ,
    IN GT_U32   smiDevAddr ,
    IN GT_U32   regOffset,
    IN GT_U16   *regDataPtr
)
{
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,physicalPort);
    if(portInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return readFrom88E1690OnSmi(devNum,portInfoPtr,
        smiDevAddr,regOffset,regDataPtr);
}

/* get the HW devNum for the 88e1690 device */
static GT_U16   get88e1690HwDevNum(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum
)
{
    /*current implementation can use value 1 for all devices*/
    devNum = devNum;
    cascadePortNum = cascadePortNum;

    return 1;
}

/**
* @internal internal_initConnectionTo88e1690_ccfc function
* @endinternal
*
* @brief   Configure per device CCFC.
*
* @param[in] devNum                   - The device number.
* @param[in] cascadePortNum           - Extended cascade port number connected to remote device.
* @param[in] internalFcTxqPort        - TxQ port number assigned to CCFC traffic.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - previous SMI DX transaction not finished
* @retval GT_HW_ERROR              - HW error
* @retval GT_NOT_INITIALIZED       - The driver was not initialized or
*                                       SMI ctrl register callback not registered
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_initConnectionTo88e1690_ccfc(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum,
    IN GT_U32   internalFcTxqPort
)
{
    GT_STATUS rc = GT_OK;
    GT_PHYSICAL_PORT_NUM portNum;           /* loop iterator */
    CPSS_DXCH_TABLE_ENT tableType; /* table type */
    GT_U32      txDmaNum;
    GT_U32      regAddr;
    GT_U32      value;
    GT_U32  isSip5_16 = PRV_CPSS_SIP_5_16_CHECK_MAC(devNum);

    if(isSip5_16)
    {
        /*
                Configure SOHO CCFC Speed%s PauseTimer to different Timer
                    Pause frame: 180,000 bit time
         */
        /* SPD 0 - 10 Mbps */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sohoCcfc.sohoCcfcSpeedToTimer[0];
        value = (180*1000) / 512;   /* 802.3x pause value is in units of 512 bits */

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* SPD 1 - 100 Mbps */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sohoCcfc.sohoCcfcSpeedToTimer[1];
        value = (180*1000) / 512;   /* 802.3x pause value is in units of 512 bits */

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* SPD 2 - 1000 Mbps */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sohoCcfc.sohoCcfcSpeedToTimer[2];
        value = (180*1000) / 512;   /* 802.3x pause value is in units of 512 bits */

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (internalFcTxqPort != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
        {
            /*
                    CCFC supposed to be sending to E1690 mapped as follows:
                    (Configure <FORWARD and FROM_CPU To Loopback Mapper> Table and
                        Set QagMetalFixEn[0] = Enable in the QAG)
             */
            regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).global.QAGMetalFix;
            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &value);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (value == 0)
            {
                /* First time:
                 *  - Set the register
                 *  - Configure Table for all non-remote ports
                 */
                value = 1;

                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, value);
                if (rc != GT_OK)
                {
                    return rc;
                }

                tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E;
                /* EGF Forward and From_CPU to Loopback Mapper Table Configuration */
                for(portNum = 0; portNum < 128; portNum++)
                {
                    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid == GT_FALSE)
                    {
                        /* we not want the call to prvCpssDxChPortPhysicalPortMapCheckAndConvert
                           to leave CPSS_LOG indication of fail. */
                        continue;
                    }

                    /* Mapping for all TXQ ports - except for remote ports which will be modified later */
                    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.mappingType ==
                                CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
                    {
                        /* Remote port - ignore */
                        continue;
                    }

                    /* Port to TxQ mapping */
                    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum,
                                            portNum,
                                            PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
                                            &txDmaNum);
                    if(rc != GT_OK)
                    {
                        /* Not valid or no TxQ mapping - ignore */
                        continue;
                    }

                    value = (txDmaNum << 8);
                    rc = prvCpssDxChWriteTableEntryField(
                        devNum,
                        tableType,
                        portNum,
                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                        0,
                        16,
                        value);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }
            }
        }

        /*
                The following configuration should be applied in the TTI:
                    TTI Global Configuration<CNM Priority/Speed Bits Location> should be set to 0 (CPID), this is for correct system behavior.
                    Configure SOHO CCFC Global<SohoCCFC_Enable> to Enable
                    Configure SOHO CCFC Global<SohoCCFC_PrioAware> to 802.3x
                    Configure SOHO CCFC Speed%s PauseTimer to different Timer as described above.
         */
        rc = cpssDxChPortCnPrioritySpeedLocationSet(devNum, CPSS_DXCH_PORT_CN_PRIORITY_SPEED_CPID_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).sohoCcfc.sohoCcfcGlobal;
        value = (1 << 0) | (0 << 1);   /* Enable: 1, PrioAware: 0 */

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 2, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /*
                on top of that the following should be configure in the HA:
                    Header Alteration Global Configuration<CNM Priority/Speed Bits Location> should be set to 0 (CPID).
                    Configure Congestion Notification Configuration<SOHO CCFC Enable> to Enable
                    Configure Congestion Notification Configuration<SOHO CCFC Priority Aware> to 802.3x
                    Configure Congestion Notification Configuration<SOHO CCFC Priority> to 7 (different from the Priority of a regular packet)
         */
        /* set HA Congestion Configuration Register */
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).congestionNotificationConfig;
        value = (1 << 1) | (0 << 0) | (7 << 2);   /* Enable: 1, PrioAware: 0, Prio: 7 */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 5, value);
        if (rc != GT_OK)
        {
            return rc;
        }

    }

    /* TxQ to TX DMA mapping */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum,
                            cascadePortNum,
                            PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E,
                            &txDmaNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortMappingTxQPort2TxDMAMapSet(devNum,
                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                            internalFcTxqPort,
                            txDmaNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}


static GT_STATUS internal_initConnectionTo88e1690_dxch_remote_port(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum,
    IN GT_U32   internalFcTxqPort,
    IN CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC *remotePortInfoPtr
)
{
    GT_STATUS rc;
    GT_U32  portNum = remotePortInfoPtr->remotePhysicalPortNum;
    GT_U32  dsa_HwDevNum , dsa_PortNum = remotePortInfoPtr->remoteMacPortNum;
    GT_U32  value = 1;

    dsa_HwDevNum = get88e1690HwDevNum(devNum,cascadePortNum);

    rc = cpssDxChCscdPortTypeSet(devNum, portNum ,
        CPSS_PORT_DIRECTION_TX_E , CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
        No API (yet) <Transmit FROM_CPU packets with forced 4B FROM_CPU DSA Tag>
        No API (yet) <Transmit TO_ANALYZER packets with forced 4B FROM_CPU DSA Tag>
        No API (yet) <Transmit FORWARD packets with forced 4B FROM_CPU DSA Tag>
    */

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (internalFcTxqPort != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
    {
        /*
                CCFC supposed to be sending to E1690 mapped as follows:
                (Configure <FORWARD and FROM_CPU To Loopback Mapper> Table and
                    Set QagMetalFixEn[0] = Enable in the QAG)
         */
        value = ((internalFcTxqPort << 8) & 0xff00) | (((cascadePortNum) << 0) & 0x00ff);
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0,
            16,
            value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet (devNum, portNum,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* set info what the egress DSA should hold for target port , target device */
    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet (devNum, portNum,
        dsa_HwDevNum , dsa_PortNum);
    if(rc != GT_OK)
    {
        return rc;
    }
    /*for the 'unaware application' about 'link down filter': force link down */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet (devNum, portNum,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

static GT_STATUS internal_initConnectionTo88e1690_dxch(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum
)
{
    GT_STATUS rc;
    GT_U32  srcDevLsbAmount = 0 , srcPortLsbAmount = 4;
    GT_U32    physicalPortBase;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * internal_infoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr = &internal_infoPtr->connectedPhyMacInfo;
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC *currentRemotePortInfoPtr;
    GT_U32  ii;

    if(internal_infoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /***********************/
    /* global - per device */
    /***********************/
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(
        devNum, srcDevLsbAmount,srcPortLsbAmount);
    if(rc != GT_OK)
    {
        return rc;
    }

    /***********************/
    /* per cascade port    */
    /***********************/

    /* to limit errors by application that added it to vlan */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, cascadePortNum  ,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdPortTypeSet(devNum, cascadePortNum ,
        CPSS_PORT_DIRECTION_BOTH_E , CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, cascadePortNum ,
         CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    physicalPortBase = infoPtr->remotePortInfoArr[0].remotePhysicalPortNum -
                       infoPtr->remotePortInfoArr[0].remoteMacPortNum;

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, cascadePortNum ,
        physicalPortBase);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum,
        cascadePortNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the 88e1690 supports Tx for max 10K , we add 16 ... to not be bottleneck in any case */
    rc = cpssDxChPortMruSet(devNum, cascadePortNum , _10K + 16);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*******************************/
    /* per remote physical port    */
    /*******************************/

    currentRemotePortInfoPtr = &infoPtr->remotePortInfoArr[0];
    for(ii = 0 ; ii < 8 ; ii++ , currentRemotePortInfoPtr++)
    {
        rc = internal_initConnectionTo88e1690_dxch_remote_port(devNum,cascadePortNum,
                infoPtr->internalFcTxqPort,
                currentRemotePortInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /***********************/
    /* global - CCFC per device */
    /***********************/
    rc = internal_initConnectionTo88e1690_ccfc(devNum,
                cascadePortNum,
                infoPtr->internalFcTxqPort);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

typedef struct {
    GT_U16   length;
    GT_U16   address;
    GT_CHAR  *data;
} PRV_CPSS_DXCH_88E1690_FIRMWARE_TABLE_STC;

static PRV_CPSS_DXCH_88E1690_FIRMWARE_TABLE_STC prvCpssDxCh_88e1690FirmwareTable[] = {
    /* The file included below is generated from Z80 firmware compilation,
     *  using the relevant IMP development environment */
    #include <CCFC_QueueMonitor.h>
};

/**
* @internal internal_loadFirmwareTo88e1690 function
* @endinternal
*
* @brief   Configure per device CCFC.
*
* @param[in] internalInfoPtr          - Information needed to access the device
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - previous SMI DX transaction not finished
* @retval GT_HW_ERROR              - HW error
* @retval GT_NOT_INITIALIZED       - The driver was not initialized or
*                                       SMI ctrl register callback not registered
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_loadFirmwareTo88e1690(
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr
)
{
    GT_STATUS   rc;
    GT_CPSS_QD_DEV   *dev = (GT_CPSS_QD_DEV *) internalInfoPtr->drvInfoPtr;
    GT_U32      i;          /* Index into firmware table */

    /* Issue a Stop IMPOp - this sets the IMP Debug logic into the Stop state */
    rc = prvCpssDrvImpStop(dev);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Issue a Reset IMPOp - this resets the IMP CPU, re-starts its clocks, but
     *  since the IMP Debug logic is in the Stop state, the IMP CPU will not try
     *  to re-boot from the EEPROM */
    rc = prvCpssDrvImpReset(dev);
    if(rc != GT_OK)
    {
        return rc;
    }

    i = 0;
    while (prvCpssDxCh_88e1690FirmwareTable[i].length != 0)
    {
        /* Load the desired code to the IMP RAM using the Examine, Deposit &
         *  Deposit Next IMPOps */
        /* Use Examine to point to the address in RAM to modify */
        /* Use Deposit to write to the RAM address that is currently being pointed to */
        /* Use Deposit Next to write to the RAM address that is currently being
         *  pointed to and then point to the next higher RAM address */

        rc = prvCpssDrvImpLoadToRAM(dev,
                    prvCpssDxCh_88e1690FirmwareTable[i].address,
                    prvCpssDxCh_88e1690FirmwareTable[i].length,
                    (GT_U8 *) prvCpssDxCh_88e1690FirmwareTable[i].data);
        if(rc != GT_OK)
        {
            return rc;
        }
        i++;
    }

    /* Execute the just entered code using the Examine & Run IMPOps */
    /* Use Examine to point to the address in RAM where the CPU is to start executing */
    /* Use Run to get the CPU to start executing at that address */

    rc = prvCpssDrvImpRun(dev, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_initConnectionTo88e1690_tcam function
* @endinternal
*
* @brief   Configure 88e1690 TCAM.
*
* @param[in] internalInfoPtr          - Information needed to access the device
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - previous SMI DX transaction not finished
* @retval GT_HW_ERROR              - HW error
* @retval GT_NOT_INITIALIZED       - The driver was not initialized or
*                                       SMI ctrl register callback not registered
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_initConnectionTo88e1690_tcam(
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr
)
{
    GT_STATUS               rc;
    GT_CPSS_QD_DEV              *dev = (GT_CPSS_QD_DEV *) internalInfoPtr->drvInfoPtr;
    GT_CPSS_LPORT                port;
    GT_U8                   tcamPointer;
    GT_CPSS_TCAM_DATA            tcamData;
    GT_CPSS_TCAM_EGR_DATA        tcamEgrData;

    /* Only work on device which has actually TCAM */
    switch (dev->deviceId)
    {
    case GT_CPSS_88E6190:
    case GT_CPSS_88E6190X:
        /* No TCAM in these devices - ignore */
        return GT_OK;
    default:
        break;
    }

    /* Flush All the TCAM */
    rc = prvCpssDrvGtcamFlushAll(dev);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* 1. Set rule to divert CCFC packet to the CPU */
    for (tcamPointer = 1; tcamPointer <= 8;tcamPointer++)
    {
        /* Flush Egress Entry All ports */
        rc = prvCpssDrvGtcamEgrFlushEntryAllPorts(dev, tcamPointer);
        if(rc != GT_OK)
        {
            return rc;
        }

        cpssOsMemSet(&tcamData, 0, sizeof(tcamData));
        /* Pattern & mask */
        tcamData.frameType = GT_CPSS_FRAME_MODE_DSA;
        tcamData.frameTypeMask = 0x03;
        tcamData.frameOctet[12] = 0x80;         /* CCFC DSA tag */
        tcamData.frameOctet[13] = 0x02;
        tcamData.frameOctet[15] = tcamPointer;  /* port */
        tcamData.frameOctetMask[12] = 0xDF;
        tcamData.frameOctetMask[13] = 0x03;
        tcamData.frameOctetMask[15] = 0x1F;
        /* Ingress Action */
        tcamData.IncTcamCtr = GT_TRUE;
        tcamData.factionOverride = GT_TRUE;
        tcamData.factionData = (1 << 0);    /* Map2IntCpu */
        tcamData.qpriOverride = GT_TRUE;
        tcamData.qpriData = tcamPointer;    /* queue per port */
        tcamData.dpvSF = GT_TRUE;
        tcamData.dpvData = 0;       /* Don't forward */
        /*tcamData.dpvData = (1 << 2);        Forward to port 2 */
        tcamData.dpvMode = 3;       /* Replace DPV with the above */

        rc = prvCpssDrvGtcamLoadEntry (dev, tcamPointer, &tcamData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* 2. Set rule to overide Provider mode port behavior on egress */
    tcamPointer = 12;

    /* Flush Egress Entry All ports */
    rc = prvCpssDrvGtcamEgrFlushEntryAllPorts(dev, tcamPointer);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Egress Action */
    cpssOsMemSet(&tcamEgrData, 0, sizeof(tcamEgrData));
    tcamEgrData.frameModeOverride = GT_TRUE;
    tcamEgrData.frameMode = GT_CPSS_FRAME_MODE_NORMAL;      /* Network Mode */
    for (port = 1; port <= 8; port++)
    {
        rc = prvCpssDrvGtcamEgrLoadEntry (dev, port, tcamPointer, &tcamEgrData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    cpssOsMemSet(&tcamData, 0, sizeof(tcamData));
    /* Pattern & mask */
    tcamData.frameType = GT_CPSS_FRAME_MODE_DSA;
    tcamData.frameTypeMask = 0x03;
    tcamData.frameOctet[12] = 0x40;         /* FROM_CPU DSA tag */
    tcamData.frameOctetMask[12] = 0xC0;
    /* Ingress Action */
    tcamData.egActPoint = tcamPointer;      /* Point to Egress Action */

    rc = prvCpssDrvGtcamLoadEntry (dev, tcamPointer, &tcamData);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @struct SW_EEPROM_STC
 *
 * @brief A struct containing data for each 88e1690 configuration
 * database (eeprom) entry.
*/
typedef struct{

    /** Smi device address (0..31) */
    GT_U32 smiDevAddr;

    /** Smi register address offset (0..31) */
    GT_U32 registerOffset;

    /** Data */
    GT_U16 data;

    /** @brief (16 bits) allow to set part of the register
     *  value 0 is the same as '0xFFFF'
     */
    GT_U16 mask;

    /** @brief To allow 'n' consecutive ports with the same value on register.
     *  value 0 is the same as 1
     */
    GT_U32 numsmiDevAddrRepetitions;

    /** @brief To allow 'm' consecutive registers with the same value on register.
     *  value 0 is the same as 1
     */
    GT_U32 numRegisterOffsetRepetition;

    /** @brief When set, wait for bit 15 to be 0 after writing
     *  Comments:
     *  None.
     */
    GT_BOOL waitBusy;

} SW_EEPROM_STC;

/**
* @struct SW_EEPROM_DB_STC
 *
 * @brief A struct containing data for every 88e1690 configuration
 * database (eeprom), with the relevant device family and
 * delay for every configuration database.
*/
typedef struct{

    SW_EEPROM_STC *swEepromPtr;

    /** delay needed prior executing databse configurations */
    GT_U32 delay;

    /** @brief device family applicable for this configuration database
     *  Comments:
     *  Use CPSS_MAX_FAMILY as a devFamily parameter to apply configuration
     *  database on every given device.
     */
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

} SW_EEPROM_DB_STC;

static SW_EEPROM_STC  swEeprom88e1690Arr[] =
{
/* 88e1690 Configuration script */
/* V0.1                                                                */
/* NO_CPU should be pulled low to  */
/* Put the switch in CPU Attached        */
/*        Mode                                                        */

/* Set flow control path and receive only */
/* Index 18-1F                                                         */
/* Enable all PFC receive and transmit priorities. Don't enable PFC yet */
/* rw u1 p0-8 r2 h908D */
    /* device 0 will be incremented by repetition - for all 9 ports (devAddr) switch port registers */
    {0 , 2 , 0x1800 , 0 , 11, 1, GT_FALSE},
    {0 , 2 , 0x9888 , 0xFF88 , 11, 1, GT_FALSE},
    {0 , 2 , 0x1900 , 0 , 11, 1, GT_FALSE},
    {0 , 2 , 0x9988 , 0xFF88 , 11, 1, GT_FALSE},
    {0 , 2 , 0x1A00 , 0 , 11, 1, GT_FALSE},
    {0 , 2 , 0x9A88 , 0xFF88 , 11, 1, GT_FALSE},
    {0 , 2 , 0x1B00 , 0 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x9B88 , 0xFF88 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x1C00 , 0 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x9C88 , 0xFF88 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x1D00 , 0 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x9D88 , 0xFF88 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x1E00 , 0 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x9E88 , 0xFF88 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x1F00 , 0 , 9 , 1, GT_FALSE},
    {0 , 2 , 0x9F88 , 0xFF88 , 9 , 1, GT_FALSE},

/* Index 10                                                         */
/* If  QC is congested CCFC messages will be sent to to cross chip link*/
/* rw u1 p0-8 r2 h908D */
    /* device 0 will be incremented by repetition - for all 9 ports (devAddr) switch port registers */
    {0 , 2 , 0x908D , 0 , 9 , 1, GT_FALSE}

/* Enable Flow control in both directions on cross chip link */
/*rw u1 p9-10 r2 h908F*/
    /* device 9 will be incremented by repetition - ports 9 and 10 (devAddr) switch port registers */
    ,{9 , 2 , 0x908F , 0 , 2 , 1, GT_FALSE}

/* Put port 9 in DSA mode                                                        */
/*rw u1 p9 r4 h017f*/
    /* device 9 - port 9 (devAddr) switch port registers */
    ,{9 , 4 , 0x017f , 0 , 1 , 1, GT_FALSE}

/* Be sure port 9 is in RXAUI mode                           */
/*rw u1 p9 r0 h004D*/
    /* device 9 - port 9 (devAddr) switch port registers */
    ,{9 , 0 , 0x000D , 0x000F , 1 , 1, GT_FALSE}

/* Put ports 0-8 in network mode */
/* also set InitialPri = 0x1 = Use Tag Priority (bits 4:5)*/
/*rw u1 p0-8 r4 h000f*/
    ,{0 , 4 , 0x001c , 0 , 9 , 1, GT_FALSE}

/* Implement WA for RN 3.4 */
    ,{5 , 0x1A , 0x01c0 , 0 , 0 , 1, GT_FALSE}
    ,{4 , 0x1A , 0xfc00 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfc20 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfc40 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfc60 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfc80 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfca0 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfcc0 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfce0 , 0 , 0 , 1, GT_TRUE}
    ,{4 , 0x1A , 0xfd00 , 0 , 0 , 1, GT_TRUE}
    ,{PRV_CPSS_GLOBAL1_DEV_ADDR , PRV_CPSS_QD_REG_GLOBAL_CONTROL,     0x8000,0x8000,0,0, GT_TRUE}
/* End of WA */

/* Put ports 0-8 in forwarding mode */
/*rw u1 p0-8 r4 h000f*/
    ,{0 , 4 , 0x0003, 0x0003, 9 , 1, GT_FALSE}

/* disable MapDA */
/*rw u1 p1-8 r8 h2000*/
    /* device 1 will be incremented by repetition - for all 8 ports (devAddr) switch port registers */
    ,{1 , 8 , 0x2000 , 0 , 9 , 1, GT_FALSE}
/* Set port based VLAN map to forward only to port 9 */
/*rw u1 p1-8  r6 h0200*/
    /* device 1 will be incremented by repetition - for all 8 ports (devAddr) switch port registers */
    ,{1 , 6 , 0x0200 , 0 , 8 , 1, GT_FALSE}
#if 0 /* need to get values as Aldrin remote ports */
/* Each port in its own priority queue set default VID to match the port*/
/*
rw u1 p1 r7 0000 0000 0000 0001
rw u1 p2 r7 0010 0000 0000 0001
rw u1 p3 r7 0100 0000 0000 0001
rw u1 p4 r7 0110 0000 0000 0001
rw u1 p5 r7 1000 0000 0000 0001
rw u1 p6 r7 1010 0000 0000 0001
rw u1 p7 r7 1100 0000 0000 0001
rw u1 p8 r7 1110 0000 0000 0001
*/
    ,{1 , 7 , 0x0001,0,0,0, GT_FALSE}
    ,{2 , 7 , 0x2001,0,0,0, GT_FALSE}
    ,{3 , 7 , 0x4001,0,0,0, GT_FALSE}
    ,{4 , 7 , 0x6001,0,0,0, GT_FALSE}
    ,{5 , 7 , 0x8001,0,0,0, GT_FALSE}
    ,{6 , 7 , 0xa001,0,0,0, GT_FALSE}
    ,{7 , 7 , 0xc001,0,0,0, GT_FALSE}
    ,{8 , 7 , 0xe001,0,0,0, GT_FALSE}
#endif /*0*/

/* loopback filter to prevent flooded frames from egressing the same port they ingressed on */
/*rw u1 p28 r5 1xxx xxxx xxxx xxxx*/
    /* device 28 - global registers, register 5- management */
    ,{28 , 5 , 0x8000 , 0x8000,0,0, GT_FALSE}
/* Enable CCFC messages
rw u1 p28 r5 xx1x xxxx xxxx xxxx */


/* Change threshold for 802.3 pause frames */
/* Port register register 0x28 Index 0x1c */
/* Currently writing default value to register the last 8 bits ar ethe value to tune */

/* rw u1 p9-10 r28 HD026 */

/********************/
/* manually added : */
/********************/

    /* Enable TCAM on port 9 */
    /* device 9 - port 9 (devAddr) switch port registers */
    ,{9, PRV_CPSS_QD_REG_PRI_OVERRIDE, 0x01, 0x003, 0, 0, GT_FALSE}

    /* Disable EEE on Serdes and RGMII ports (RN section 3.10) */
    /* device 0 - port 0 (devAddr) switch port registers,
       register QD_REG_PHY_CONTROL*/
    ,{ 0,PRV_CPSS_QD_REG_PHY_CONTROL,(BIT_8),(BIT_9|BIT_8) ,0,0, GT_FALSE}
    /* device 9 - port 9 (devAddr) switch port registers,
       register QD_REG_PHY_CONTROL*/
    ,{ 9,PRV_CPSS_QD_REG_PHY_CONTROL,(BIT_8),(BIT_9|BIT_8) ,0,0, GT_FALSE}
    /* device 10 - port 10 (devAddr) switch port registers,
       register QD_REG_PHY_CONTROL*/
    ,{10,PRV_CPSS_QD_REG_PHY_CONTROL,(BIT_8),(BIT_9|BIT_8) ,0,0, GT_FALSE}

    /* disable auto learning on the ports : set <PAV> = 0 */
    /* device 1 will be incremented by repetition - for all 8 ports (devAddr) switch port registers,
       register QD_REG_PAV */
    ,{1,PRV_CPSS_QD_REG_PAV,0,0x7ff,9,0, GT_FALSE}/* ports 1..9 */

    /*<802.1QMode> = 0x1 = Fallback.
        Enable 802.1Q for this Ingress port. Do not discard
        Ingress Membership violations2 and use the VLANTable bits (i.e., port
        based VLANs – Port offset 0x06) if the frame’s VID is not contained in the
        VTU (both errors are logged – Global 1 offset 0x05).
    */
    /* device 1 will be incremented by repetition - for all 8 ports (devAddr) switch port registers,
       register QD_REG_PORT_CONTROL2 */
    ,{1,PRV_CPSS_QD_REG_PORT_CONTROL2, (GT_CPSS_FALLBACK<<10), (BIT_10|BIT_11),8,0, GT_FALSE}/*ports 1..8*/

    /* Set default VID to zero */
    ,{1,PRV_CPSS_QD_REG_PVID, 0, 0x0FFF,8,0, GT_FALSE}/*ports 1..8*/

    /* Use CFI Yellow = 0x1 So that the CFI bit from Network port
        ingressed packets is propagated up to DSA port egress */
    /* device 1 will be incremented by repetition - for all 8 ports (devAddr) switch port registers,
       register QD_REG_PORT_CONTROL3 */
    ,{1, PRV_CPSS_QD_REG_PORT_CONTROL3, (BIT_1|BIT_0), (BIT_1|BIT_0), 8, 0, GT_FALSE}   /*ports 1..8*/

    /* Map all normal traffic to Queue priority 0 */
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8000,     0,9,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8201,     0,9,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8402,     0,9,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8603,     0,9,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8804,     0,9,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8a05,     0,9,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8c06,     0,9,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_IEEE_PRI_MAPPING_TBL, 0x8e07,     0,9,0, GT_FALSE}

    /* Map DSA From_CPU traffic to Queue priority 0 */
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_PRIORITY_OVERRIDE, 0x8808,     0,0,0, GT_FALSE}

    /* Configure Queue #7 as Strict (and highest) priority */
    /* device 1 will be incremented by repetition - for all 8 ports (devAddr) switch port registers,
       register QD_REG_Q_CONTROL */
    ,{1, PRV_CPSS_QD_REG_Q_CONTROL, 0x8001,     0,9,0, GT_FALSE}

    /* Configure FC Xoff/Xon thresholds to 2*0x23/2*0x0f */
    ,{1, PRV_CPSS_QD_REG_Q_CONTROL, 0xd023,     0,8,0, GT_FALSE}
    ,{1, PRV_CPSS_QD_REG_Q_CONTROL, 0xd10f,     0,8,0, GT_FALSE}

    /* Configure Round Robin weights as equal for all other priorities */
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_QOS_WEIGHT, 0x8010,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_QOS_WEIGHT, 0x8132,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_QOS_WEIGHT, 0x8254,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_QOS_WEIGHT, 0x8376,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_QOS_WEIGHT, 0xc008,     0,0,0, GT_TRUE}

    /* CPU port settings */
    /* Set CPU port to DSA mode w/Header */
    /* device PRV_CPSS_CPU_PORT_ADDR - CPU (devAddr) port switch port registers,
       register QD_REG_PORT_CONTROL */
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_PORT_CONTROL, 0x097f,     0,0,0, GT_FALSE}
    /* Set CPU port to Force FC */
    /* device PRV_CPSS_CPU_PORT_ADDR - CPU (devAddr) port switch port registers,
       register QD_REG_LIMIT_PAUSE_CONTROL */
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_LIMIT_PAUSE_CONTROL, 0x908c,     0,0,0, GT_FALSE}
    /* Set CPU port no Jumbo frames, no MapDA, Def Q Prio = 7 */
    /* device PRV_CPSS_CPU_PORT_ADDR - CPU (devAddr) port switch port registers,
       register QD_REG_PORT_CONTROL2 */
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_PORT_CONTROL2, 0x0007,     0,0,0, GT_FALSE}
    /* Limit CPU port Queues size */
    /* device PRV_CPSS_CPU_PORT_ADDR - CPU (devAddr) port switch port registers,
       register QD_REG_Q_CONTROL */
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9081,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9181,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9281,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9381,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9481,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9581,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9681,     0,0,0, GT_TRUE}
    ,{PRV_CPSS_CPU_PORT_ADDR, PRV_CPSS_QD_REG_Q_CONTROL, 0x9781,     0,0,0, GT_TRUE}
    /* Enable FC messages, use Tag FC Data */
    /* device PRV_CPSS_GLOBAL2_DEV_ADDR - global2 registers,
       register QD_REG_MANAGEMENT */
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_MANAGEMENT, (BIT_13|BIT_9),     (BIT_13|BIT_9),0,0, GT_FALSE}
    /* End CPU port settings */

    /* set <deviceNumber> ('ownDevNum') of the 88e1690 */
    /* device PRV_CPSS_GLOBAL1_DEV_ADDR - global1 registers,
       register QD_REG_MANAGEMENT */
    ,{PRV_CPSS_GLOBAL1_DEV_ADDR , PRV_CPSS_QD_REG_GLOBAL_CONTROL2,     0,0x1f,0,0, GT_FALSE}

    /* Power up Serdes of the 88e1690 port 9 */
    /* Write Address */
    /* device PRV_CPSS_GLOBAL2_DEV_ADDR - global2 registers,
       register QD_REG_SMI_PHY_DATA and QD_REG_SMI_PHY_CMD */
    /* First writing the specific serdes register address as the data, and then performing operation
       in the command register to "write-address" registers*/
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR , PRV_CPSS_QD_REG_SMI_PHY_DATA,     0x1000,0,0,0, GT_FALSE}
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR , PRV_CPSS_QD_REG_SMI_PHY_CMD,     (BIT_15 | (0<<12) | (0<<10) | (9<<5) | (4<<0)) ,0,0,0, GT_TRUE}
    /* Read */
    /* device PRV_CPSS_GLOBAL2_DEV_ADDR - global2 registers,
       register QD_REG_SMI_PHY_CMD */
    /* now we issue a command to read the serdes register address */
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR , PRV_CPSS_QD_REG_SMI_PHY_CMD,     (BIT_15 | (0<<12) | (3<<10) | (9<<5) | (4<<0)) ,0,0,0, GT_TRUE}
    /* Modify */
    /* device PRV_CPSS_GLOBAL2_DEV_ADDR - global2 registers,
       register QD_REG_SMI_PHY_DATA */
    /* modiying the data register after it was fetched from internal serdes registers */
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR , PRV_CPSS_QD_REG_SMI_PHY_DATA,     0, BIT_11, 0,0, GT_FALSE}
    /* Write */
    /* device PRV_CPSS_GLOBAL2_DEV_ADDR - global2 registers,
       register QD_REG_SMI_PHY_CMD */
    /* writing the data back (to the "write-address" that was written in the first place) */
    ,{PRV_CPSS_GLOBAL2_DEV_ADDR , PRV_CPSS_QD_REG_SMI_PHY_CMD,     (BIT_15 | (0<<12) | (1<<10) | (9<<5) | (4<<0)) ,0,0,0, GT_TRUE}
    /* End Power up Serdes of the 88e1690 port 9 */

    ,{NOT_VALID_CNS,0,0,0,0,0, GT_FALSE}
};

/* sequences needed to be performed on 1690 Phy , found on AC3X device, as W/A for link flapping issue  */
static SW_EEPROM_STC  swWA1_AC3X_Eeprom88e1690Arr[] =
{

/*
     Performing PHASE fifo reset on the RXAUI according to the next registers operations:

        RW U1 P28 R25 H9000
        RW U1 P28 R24 H8124
        RW U1 P28 R25 H100
        RW U1 P28 R24 H8524
        RW U1 P28 R25 H9000
        RW U1 P28 R24 H8124
        RW U1 P28 R25 H0
        RW U1 P28 R24 H8524
*/
    /* device 28 - global2 registers, register 25 - SMI Phy data register */
    /* device 28 - global2 registers, register 24 - SMI Phy command register, to access internal phy\serdes registers. */
    /* in this case, its serdes registers */
    {28 , 25 , 0x9000,0,0,0, GT_FALSE}
    /*write address (0x9000 [specific internal serdes address] from last data write)op, dev9 reg4 means serdes registers of port 9*/
    ,{28 , 24 , 0x8124,0,0,0, GT_TRUE}
    ,{28 , 25 , 0x100,0,0,0, GT_FALSE}
    /*write data (data 0x100 from last data write) op, dev9 reg4 means serdes registers of port 9, and to the specific register 0x9000 */
    ,{28 , 24 , 0x8524,0,0,0, GT_TRUE}
    ,{28 , 25 , 0x9000,0,0,0, GT_FALSE}
    /*write address (0x9000 [specific internal serdes address] from last data write)op, dev9 reg4 means serdes registers of port 9*/
    ,{28 , 24 , 0x8124,0,0,0, GT_TRUE}
    ,{28 , 25 , 0x0,0,0,0, GT_FALSE}
    /*write data (data 0 from last data write) op, dev9 reg4 means serdes registers of port 9, and to the specific register 0x9000 */
    ,{28 , 24 , 0x8524,0,0,0, GT_TRUE}

/*
     Performing XAUI->RXAUI W/A
*/
    /* device 9 - port 9 (devAddr) switch port registers */
    ,{9 , 0 , 0x000C , 0x000F , 1 , 1, GT_FALSE}
    ,{9 , 0 , 0x004D , 0x004F , 1 , 1, GT_FALSE}

/*
     Serdes reset W/A
*/

    ,{28 , 25 , 0x1000,0,0,0, GT_FALSE}
    ,{28 , 24 , 0x8124,0,0,0, GT_TRUE} /*write address (0x1000 [specific internal serdes address] from last data write)op, dev9 reg4 means serdes registers of port 9*/
    ,{28 , 25 , 0x0A040,0,0,0, GT_FALSE}
    ,{28 , 24 , 0x8524,0,0,0, GT_TRUE}  /*write data (data 0x100 from last data write) op, dev9 reg4 means serdes registers of port 9, and to the specific register 0x1000 */

    ,{NOT_VALID_CNS,0,0,0,0,0, GT_FALSE}
};

/* sequences needed to be performed on 1690 Phy , found on AC3X device, as W/A for link flapping issue  */
static SW_EEPROM_STC  swWA2_AC3X_Eeprom88e1690Arr[] =
{
/*
    Description of the next WA sequence:
        elf calibration  feature in 1690 Phy embedded PHYs is not working correctly
        resulting port to port and chip to chip VOD measurmenet variation.
        This will result in some boards failing some of the IEEE conformance tests.

    the sequence:
        PHY 0_0.11 = 0  => Clear power down bit in PHY => achieved by previous steps
        Wait for 300us  => achieved by previous steps
        Phy register 0x16 = 248
        Phy register 0x8=0x36
        PHY register 0x16=0
        PHY register 0x0=0x9140
*/

    /* device 28 - global2 registers, register 25 - SMI Phy data register */
    /* device 28 - global2 registers, register 24 - SMI Phy command register, to access internal phy\serdes registers. */
    /* write 248 as date to be inserted to page register */
    {28 , 25 , 248, 0,0,0, GT_FALSE}
    /* dev1 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9436,0,0,0, GT_TRUE}
    /* dev2 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9456,0,0,0, GT_TRUE}
    /* dev3 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9476,0,0,0, GT_TRUE}
    /* dev4 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9496,0,0,0, GT_TRUE}
    /* dev5 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94B6,0,0,0, GT_TRUE}
    /* dev6 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94D6,0,0,0, GT_TRUE}
    /* dev7 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94F6,0,0,0, GT_TRUE}
    /* dev8 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9516,0,0,0, GT_TRUE}

    /* write data 0x36 to all phys */
    ,{28 , 25 , 0x36, 0,0,0, GT_FALSE}
    /* dev1 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9428,0,0,0, GT_TRUE}
    /* dev2 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9448,0,0,0, GT_TRUE}
    /* dev3 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9468,0,0,0, GT_TRUE}
    /* dev4 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9488,0,0,0, GT_TRUE}
    /* dev5 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94a8,0,0,0, GT_TRUE}
    /* dev6 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94c8,0,0,0, GT_TRUE}
    /* dev7 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94e8,0,0,0, GT_TRUE}
    /* dev8 , register 8 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9508,0,0,0, GT_TRUE}

    /* restore page to 0 */
    ,{28 , 25 , 0, 0,0,0, GT_FALSE}
    /* dev1 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9436,0,0,0, GT_TRUE}
    /* dev2 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9456,0,0,0, GT_TRUE}
    /* dev3 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9476,0,0,0, GT_TRUE}
    /* dev4 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9496,0,0,0, GT_TRUE}
    /* dev5 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94B6,0,0,0, GT_TRUE}
    /* dev6 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94D6,0,0,0, GT_TRUE}
    /* dev7 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x94F6,0,0,0, GT_TRUE}
    /* dev8 , register 22 (page reg) bit10=1 => write data*/
    ,{28 , 24 , 0x9516,0,0,0, GT_TRUE}

     /* set all ports to normal operations */
    /*rw u1 p28 r25 h9140 */
    ,{28 , 25 , 0x9140,0,0,0, GT_FALSE}
    /*rw u1 p28 r24 h9420 */
    ,{28 , 24 , 0x9420,0,0,0, GT_TRUE}
    /*rw u1 p28 r24 h9440 */
    ,{28 , 24 , 0x9440,0,0,0, GT_TRUE}
    /*rw u1 p28 r24 h9460 */
    ,{28 , 24 , 0x9460,0,0,0, GT_TRUE}
    /*rw u1 p28 r24 h9480 */
    ,{28 , 24 , 0x9480,0,0,0, GT_TRUE}
    /*rw u1 p28 r24 h94A0 */
    ,{28 , 24 , 0x94a0,0,0,0, GT_TRUE}
    /*rw u1 p28 r24 h94c0 */
    ,{28 , 24 , 0x94c0,0,0,0, GT_TRUE}
    /*rw u1 p28 r24 h94e0 */
    ,{28 , 24 , 0x94e0,0,0,0, GT_TRUE}
    /*rw u1 p28 r24 h9500 */
    ,{28 , 24 , 0x9500,0,0,0, GT_TRUE}

    ,{NOT_VALID_CNS,0,0,0,0,0, GT_FALSE}
};

SW_EEPROM_DB_STC  swEepromsDb[]={
    /*  Eeprom                    Delay(MSec)         DevFamily       */
    {&swEeprom88e1690Arr[0],            0,      CPSS_MAX_FAMILY           }
    ,{&swWA1_AC3X_Eeprom88e1690Arr[0],  0,      CPSS_PP_FAMILY_DXCH_AC3X_E}
    ,{&swWA2_AC3X_Eeprom88e1690Arr[0],  1,      CPSS_PP_FAMILY_DXCH_AC3X_E}
};

static GT_STATUS internal_initConnectionTo88e1690(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  mask;/* (16 bits) allow to set part of the register .
                    value 0 is the same as '0xFFFF' */
    GT_U32  smiDevAddr;/* (0..31)*/
    GT_U32  registerOffset;/* (0..31) */
    GT_BOOL waitBusy; /* When set, wait for bit 15 to be 0 after writing */
    GT_U16  data,newData,oldData;
    GT_U32  jj,jjMax,kk,kkMax;
    GT_U8   epromsIdx;
    SW_EEPROM_STC *swEeprom;

    for (epromsIdx=0; epromsIdx<sizeof(swEepromsDb)/sizeof(SW_EEPROM_DB_STC); epromsIdx++)
    {
        if (swEepromsDb[epromsIdx].devFamily != CPSS_MAX_FAMILY)
        {
            if (swEepromsDb[epromsIdx].devFamily != PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                continue;
            }
        }

        if (swEepromsDb[epromsIdx].delay > 0)
        {
            cpssOsTimerWkAfter(swEepromsDb[epromsIdx].delay);
        }

        swEeprom = swEepromsDb[epromsIdx].swEepromPtr;

        for (ii = 0; swEeprom[ii].smiDevAddr != NOT_VALID_CNS; ii++)
        {
            jjMax = swEeprom[ii].numsmiDevAddrRepetitions < 2 ?
                    1 :
                    swEeprom[ii].numsmiDevAddrRepetitions;
            kkMax = swEeprom[ii].numRegisterOffsetRepetition < 2 ?
                    1 :
                    swEeprom[ii].numRegisterOffsetRepetition;

            mask = swEeprom[ii].mask == 0 ?
                    0xFFFF :
                    swEeprom[ii].mask;
            waitBusy = swEeprom[ii].waitBusy;

            smiDevAddr = swEeprom[ii].smiDevAddr;
            registerOffset = swEeprom[ii].registerOffset;
            data = swEeprom[ii].data;

            if(smiDevAddr == PRV_CPSS_GLOBAL1_DEV_ADDR &&
               registerOffset == PRV_CPSS_QD_REG_GLOBAL_CONTROL2)
            {
                data = get88e1690HwDevNum(devNum,cascadePortNum);
            }

            for(jj = 0 ; jj < jjMax ; jj++)
            {
                for(kk = 0 ; kk < kkMax ; kk++)
                {
                    if(mask != 0xFFFF)
                    {
                        /* first read the register */
                        rc = readFrom88E1690OnPort(devNum,cascadePortNum,
                            smiDevAddr + jj    , registerOffset + kk  ,   &oldData);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }

                        newData = (data & mask) | (oldData & (~mask));
                    }
                    else
                    {
                        newData = data;
                    }

                    /* write back the register */
                    rc = writeTo88E1690OnPort(devNum,cascadePortNum,
                        smiDevAddr + jj    , registerOffset + kk   ,   newData);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    if(waitBusy)
                    {
                        GT_U32  timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP;
                        do
                        {
                            /* Re-read the register */
                            rc = readFrom88E1690OnPort(devNum,cascadePortNum,
                                smiDevAddr + jj    , registerOffset + kk  ,   &newData);
                            if(rc != GT_OK)
                            {
                                return rc;
                            }

                            if(timeOut-- < 1 )
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                            }
                        }
                        while((newData & BIT_15) != 0);
                    }
                }/*kk*/
            }/*jj*/
        }/*ii*/
    }/*epromsIdx*/




    return GT_OK;
}

static GT_STATUS initConnectionTo88e1690(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum
)
{
    GT_STATUS rc;
    CPSS_MACDRV_OBJ_STC* phyMacObjPtr;

    rc = internal_initConnectionTo88e1690(devNum,cascadePortNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = internal_initConnectionTo88e1690_dxch(devNum,cascadePortNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    phyMacObjPtr = prvCpssDxChCfgPort88e1690ObjInit(/*void*/);
    if(phyMacObjPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* bind ports to phy object  */
    rc = cpssDxChCfgBindPortPhymacObject(devNum,cascadePortNum,phyMacObjPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/*****************************************************************************
*
* bool etherReadMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int* value)
*
* Description
* This function will access the MII registers and will read the value of
* the MII register , and will retrieve the value in the pointer.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* Outputs
* value - pointer to unsigned int which will receive the value.
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/
static GT_STATUS gtBspReadMii (
    GT_CPSS_QD_DEV* dev,
    GT_U8                phyAddr,
    GT_U8                miiReg,
    GT_U16              * value,
    GT_CPSS_CONTEXT_ENT      context
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC   *internal_infoPtr;
    GT_U8 dxDevNum;

    (void)context; /* unused because there is no "interrupt mode" now */
    internal_infoPtr = dev->appData;
    dxDevNum = internal_infoPtr->dxDevNum;


    /* call to internal function that don't do 'CPSS LOCK/UNLOCK' */
    /* for ISR it is not allowed */
    /* for regular thread we are already under 'CPSS LOCK' !!! */
    rc = internal_cpssSmiRegisterReadShort(dxDevNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        CPSS_PHY_SMI_INTERFACE_0_E + internal_infoPtr->connectedPhyMacInfo.mngBusNumber,
        phyAddr/*(GT_U32)(infoPtr->busBaseAddr)*/,
        miiReg, /* control/data register */
        value);

    return rc;
}

/*****************************************************************************
*
* bool etherWriteMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int value)
*
* Description
* This function will access the MII registers and will write the value
* to the MII register.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* value -the value that will be written.
* Outputs
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/

static GT_STATUS gtBspWriteMii (
    GT_CPSS_QD_DEV*   dev,
    GT_U8                 phyAddr,
    GT_U8                 miiReg,
    GT_U16                 value,
    GT_CPSS_CONTEXT_ENT          context
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC   *internal_infoPtr;
    GT_U8 dxDevNum;

    (void)context; /* unused because there is no "interrupt mode" now */
    internal_infoPtr = dev->appData;
    dxDevNum = internal_infoPtr->dxDevNum;

    /* call to internal function that don't do 'CPSS LOCK/UNLOCK' */
    /* for ISR it is not allowed */
    /* for regular thread we are already under 'CPSS LOCK' !!! */
    rc = internal_cpssSmiRegisterWriteShort(dxDevNum,
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        CPSS_PHY_SMI_INTERFACE_0_E + internal_infoPtr->connectedPhyMacInfo.mngBusNumber,
        phyAddr/*(GT_U32)(infoPtr->busBaseAddr)*/,
        miiReg, /* control/data register */
        value);

    return rc;
}


/**
* @internal drv88e1690Protection function
* @endinternal
*
* @brief   drv88e1690 protection : sych between threads and the ISR operations.
*
* @param[in] devDrvInstancePtr        - (pointer to) the device driver info.
* @param[in] startProtection          - indication to start protection.
*                                      GT_TRUE  - start the protection
*                                      GT_FALSE - end   the protection
* @param[in] cookiePtr                - (pointer to) cookie that is internally set and used.
*                                      the cookie must not be changed by the caller between calls
*                                      for 'start' and 'stop'
*                                       None
*/
static void drv88e1690Protection
(
    IN GT_CPSS_QD_DEV *devDrvInstancePtr,
    IN GT_BOOL  startProtection,
    IN GT_32    *cookiePtr GT_UNUSED
)
{

    devDrvInstancePtr = devDrvInstancePtr;


    if(startProtection == GT_TRUE)
    {
        PRV_CPSS_INT_SCAN_LOCK();
    }
    else
    {
        PRV_CPSS_INT_SCAN_UNLOCK();
    }

}

/**
* @internal init88e1690Driver function
* @endinternal
*
* @brief   init the driver of the 88e1690.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      infoPtr  - (pointer to) The needed info about the remote device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS init88e1690Driver
(
    IN  GT_U8                               devNum,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC   *internal_infoPtr
)
{
    GT_STATUS   status;
    GT_CPSS_SYS_CONFIG   cfg;
    GT_CPSS_QD_DEV   *devDrvInstancePtr;

    cpssOsMemSet((char*)&cfg,0,sizeof(GT_CPSS_SYS_CONFIG));

    internal_infoPtr->drvInfoPtr = cpssOsMalloc(sizeof(GT_CPSS_QD_DEV));
    if(internal_infoPtr->drvInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(internal_infoPtr->drvInfoPtr,0,sizeof(GT_CPSS_QD_DEV));

    devDrvInstancePtr = internal_infoPtr->drvInfoPtr;

    /* bind the driver with quick access to the needed info */
    devDrvInstancePtr->appData = internal_infoPtr;
    /* this info will be needed by the 'callback' of SMI read/write */
    internal_infoPtr->dxDevNum = devNum;


    {
        cfg.BSPFunctions.readMii   = gtBspReadMii;
        cfg.BSPFunctions.writeMii  = gtBspWriteMii;
        cfg.BSPFunctions.semCreate = NULL;
        cfg.BSPFunctions.semDelete = NULL;
        cfg.BSPFunctions.semTake   = NULL;
        cfg.BSPFunctions.semGive   = NULL;
    }


    cfg.cpuPortNum = cpuPort;
    cfg.baseAddr = (GT_U8)internal_infoPtr->connectedPhyMacInfo.busBaseAddr;        /* this is the phyAddr used by QD family device.
                                Valid value are 1 ~ 31.*/
#ifdef ASIC_SIMULATION
    /* give the INI file deviceId (1..6) */
    cfg.devNum = cfg.baseAddr;
#endif/**/

    /* we only need the 'Registration in the DB' of the driver */
    /* we not need it to access the registers */
    cfg.skipInitSetup = 1/*GT_SKIP_INIT_SETUP*/;/* not used by prvCpssDrvQdLoadDriver(..)! */
    cfg.initPorts = GT_FALSE;/* not used by prvCpssDrvQdLoadDriver(..)! */

    if((status=prvCpssDrvQdLoadDriver(&cfg, devDrvInstancePtr)) != GT_OK)
    {
        return status;
    }

    /* we need to generate single protection for synchronization of
        'regular threads' and the ISR

       ALL 'regular threads' that call regular functions are protected by the
       'CPSS lock/unlock'

       but the accessing to the 'SMI control' register should be protected also
       for ISR accessing.

       since the ISR can't use semaphores , it is protected by :

        #define INT_LOCK(key)                                              \
            key = 0;                                                       \
            PRV_CPSS_LOCK_BY_MUTEX;                                        \
            cpssExtDrvSetIntLockUnlock(CPSS_OS_INTR_MODE_LOCK_E, &(key))


        NOTE: all the 88e1690 devices that are on current 'DX device' must use
              the same 'lock'.
    */
    devDrvInstancePtr->globalProtection = drv88e1690Protection;


    return GT_OK;
}

/**
* @internal prvCpssDxChCfgPort88e1690Bind function
* @endinternal
*
* @brief   Creation and Bind PHY-MAC for PHY '88E1690' to the DX device
*         on specific portNum.
*         NOTE:
*         The portNum should be the 'physical port num' of the DX port connection
*         to the PHY-MAC device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] infoPtr                  - (pointer to) The needed info about the remote device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCfgPort88e1690Bind
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32      portMacNum; /* MAC number */
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    GT_U32      numOfMacPorts88e1690 = 9;/*0..8*/

    /* make sure that the physical port hold MAC number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[portNum] == NULL &&
        infoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[portNum] && infoPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    if(infoPtr == NULL)
    {
        internalInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[portNum];
        /* indication to detach from the PHY */
        FREE_PTR_MAC(internalInfoPtr->interrupts.intScanRoot);
        FREE_PTR_MAC(internalInfoPtr->interrupts.intNodesPool);
        FREE_PTR_MAC(internalInfoPtr->interrupts.intMaskShadow);
        FREE_PTR_MAC(internalInfoPtr->connectedPhyMacInfo.remotePortInfoArr);
        FREE_PTR_MAC(internalInfoPtr->drvInfoPtr);
        FREE_PTR_MAC(internalInfoPtr->prvRemoteMacInfoArr);
        FREE_PTR_MAC(internalInfoPtr);

        return GT_OK;
    }

    if(infoPtr->mngInterfaceType != CPSS_CHANNEL_SMI_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.smi_support.numberOfSmiIf <=
        infoPtr->mngBusNumber)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(((GT_U32)(infoPtr->busBaseAddr)) >= 32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(infoPtr->numOfRemotePorts > 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(ii = 0 ; ii < infoPtr->numOfRemotePorts; ii++)
    {
        if(GT_FALSE == prvCpssDxChPortRemotePortCheck(devNum,
            infoPtr->remotePortInfoArr[ii].remotePhysicalPortNum))
        {
            /* the port was not defined as 'remote physical port' */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(infoPtr->remotePortInfoArr[ii].remoteMacPortNum >= numOfMacPorts88e1690)
        {
            /* the mac not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    internalInfoPtr =
        cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC));
    if(internalInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(internalInfoPtr,0,sizeof(*internalInfoPtr));


    /* copy info from 'internalInfoPtr' */
    internalInfoPtr->connectedPhyMacInfo = *infoPtr;

    /* malloc the array */
    internalInfoPtr->connectedPhyMacInfo.remotePortInfoArr =
        cpssOsMalloc(infoPtr->numOfRemotePorts*sizeof(CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC));
    if(internalInfoPtr->connectedPhyMacInfo.remotePortInfoArr == NULL)
    {
        FREE_PTR_MAC(internalInfoPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* copy the array from 'internalInfoPtr' */
    for(ii = 0 ; ii < infoPtr->numOfRemotePorts; ii++)
    {
        internalInfoPtr->connectedPhyMacInfo.remotePortInfoArr[ii] =
            infoPtr->remotePortInfoArr[ii];
    }

    /* malloc the array */
    internalInfoPtr->prvRemoteMacInfoArr =
        cpssOsMalloc(numOfMacPorts88e1690*sizeof(internalInfoPtr->prvRemoteMacInfoArr[0]));
    if(internalInfoPtr->prvRemoteMacInfoArr == NULL)
    {
        FREE_PTR_MAC(internalInfoPtr->connectedPhyMacInfo.remotePortInfoArr);
        FREE_PTR_MAC(internalInfoPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(internalInfoPtr->prvRemoteMacInfoArr,
                0,
                numOfMacPorts88e1690*sizeof(internalInfoPtr->prvRemoteMacInfoArr[0]));
    /* by default need to emulate 'clear on read' on the remote ports */
    for(ii = 0 ; ii < numOfMacPorts88e1690; ii++)
    {
        internalInfoPtr->prvRemoteMacInfoArr[ii].clearOnReadEnable = GT_TRUE;
    }

    /* bind the pointer to */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[portNum] =
        internalInfoPtr;

    rc = initConnectionTo88e1690(devNum,portNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* bind to 88e1690 driver */
    rc = init88e1690Driver(devNum,internalInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Init interrupts/events support */
    rc = prvCpssDxChCfgPort88e1690InitEvents(devNum,portNum,internalInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Load firmware */
    rc = internal_loadFirmwareTo88e1690(internalInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Load TCAM */
    rc = internal_initConnectionTo88e1690_tcam(internalInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* If MIB counters are read by packet, configure CPU code.
        In other case, configure some (dummy) CPU code in order to check FW has correctly loaded. */
    if (internalInfoPtr->connectedPhyMacInfo.useMibCounterReadByPacket)
    {
        rc = prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketInit(
                devNum,
                portNum,
                internalInfoPtr->connectedPhyMacInfo.mibCounterReadByPacketCpuCode);
    }
    else
    {
        rc = prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketInit(
                devNum,
                portNum,
                CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E);
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCfgAreRemotePortsBound function
* @endinternal
*
* @brief   Check if remote ports were bound to device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] remotePortBoundPtr       - GT_TRUE - was bound, GT_FALSE - wasn't bound
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChCfgAreRemotePortsBound
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                            *remotePortBoundPtr
)
{
    GT_PHYSICAL_PORT_NUM portNum;

    for(portNum = 0; portNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; portNum++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[portNum])
        {
            *remotePortBoundPtr = GT_TRUE;
            return GT_OK;
        }
    }

    *remotePortBoundPtr = GT_FALSE;
    return GT_OK;
}

/**
* @internal prvCpssDxChAc3xRxauiPortWA function
* @endinternal
*
* @brief   Perform WA to Rxaui port in AC3X device, to overcome link flapping issue
*         found during power cycle test.
*
* @note   APPLICABLE DEVICES:      AC3X;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChAc3xRxauiPortWA
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum
)
{
    GT_STATUS rc;
    GT_U16 data;

    /* Device applicability test */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC3X_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /*
       Performing XAUI -> RXAUI W/A. Setting C_MODE of port to XAUI, than back to RXAUI
    */

    /* reading "Switch Port Registers" register Port Status */
    rc = readFrom88E1690OnPort(devNum,portNum, 9, 0, &data);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* overriding C_MODE to XAUI mode */
    rc = writeTo88E1690OnPort(devNum, portNum, 9, 0, (data & (~0xF)) | 0xC);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* overriding C_MODE to RXAUI mode */
    rc = writeTo88E1690OnPort(devNum, portNum, 9, 0, (data & (~0x4F)) | 0x4D);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*
       Performing SerDes reset W/A
    */

    /* write address 0x1000 to data reg */
    rc = writeTo88E1690OnPort(devNum, portNum, 28, 25, 0x1000);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* performing "Write Address" command in order to write the data from previous command as an address to access next */
    rc = writeTo88E1690OnPort(devNum, portNum, 28, 24, 0x8124);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* write data to data 0x0A040 to data register */
    rc = writeTo88E1690OnPort(devNum, portNum, 28, 25, 0x0A040);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* performing "Write Data" command, in order to write data to the register from previous command */
    rc = writeTo88E1690OnPort(devNum, portNum, 28, 24, 0x8524);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChAc3xIsPortConnectedTo1690Phy function
* @endinternal
*
* @brief   Check whether given port is connected to 1690 Phy device.
*
* @note   APPLICABLE DEVICES:      AC3X;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portConnectedTo1690Phy   - (pointer to) whether or not port is connected to
*                                      a 1690 Phy device
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChAc3xIsPortConnectedTo1690Phy
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *portConnectedTo1690Phy
)
{
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC * portInfoPtr;

    /* Device applicability test */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC3X_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* get info structure of cascade port (ports connected to 1690 Phy are considered as "cascade ports") */
    portInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum, portNum);

    /* if no information is availble for a port, this port is not connected to a 1690 Phy */
    if(portInfoPtr == NULL)
    {
        *portConnectedTo1690Phy = GT_FALSE;
        return GT_OK;
    }

    /* checking that the connected device is a 1690 Phy */
    if (portInfoPtr->connectedPhyMacInfo.phyMacType == CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E)
    {
        *portConnectedTo1690Phy = GT_TRUE;
    }
    else
    {
        *portConnectedTo1690Phy = GT_FALSE;
    }

    return GT_OK;
}


