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
* @file tgfPortGen.c
*
* @brief Generic API for Port configuration.
*
* @version   42
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortEcn.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxDba.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/port/cpssExMxPmPortCtrl.h>
    #include <cpss/exMxPm/exMxPmGen/port/cpssExMxPmPortStat.h>
    #include <cpss/exMxPm/exMxPmGen/port/cpssExMxPmPortTx.h>
    #include <cpss/exMxPm/exMxPmGen/port/cpssExMxPmPortBufMg.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgCount.h>
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertTgfToCpssTailDropProfileParams function
* @endinternal
*
* @brief   Convert generic into device specific Tail Drop Profile parameters.
*
* @param[in] tgfTailDropProfileParamsPtr - (pointer to) TGF Tail Drop Profile parameters.
*
* @param[out] cpssTailDropProfileParamsPtr - (pointer to) Cpss Tail Drop Profile
*                                      parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertTgfToCpssTailDropProfileParams
(
    IN  PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  *tgfTailDropProfileParamsPtr,
    OUT CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS     *cpssTailDropProfileParamsPtr
)
{
    cpssOsMemSet(cpssTailDropProfileParamsPtr, 0,
                 sizeof(CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS));

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxDescrNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxDescrNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxDescrNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxDescrNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxMCBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxMCBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxMCBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, sharedUcAndMcCountersDisable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0QueueAlpha);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1QueueAlpha);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2QueueAlpha);

    return GT_OK;
}

/**
* @internal prvTgfConvertCpssToTgfTailDropProfileParams function
* @endinternal
*
* @brief   Convert device specific Tail Drop Profile parameters into generic.
*
* @param[in] cpssTailDropProfileParamsPtr - (pointer to) Cpss Tail Drop Profile
*                                      parameters.
*
* @param[out] tgfTailDropProfileParamsPtr - (pointer to) TGF Tail Drop Profile parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertCpssToTgfTailDropProfileParams
(
    IN  CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS     *cpssTailDropProfileParamsPtr,
    OUT PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  *tgfTailDropProfileParamsPtr
)
{
    cpssOsMemSet(tgfTailDropProfileParamsPtr, 0,
                 sizeof(tgfTailDropProfileParamsPtr));

    /* convert entry info into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxMCBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxMCBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxMCBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, sharedUcAndMcCountersDisable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0QueueAlpha);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1QueueAlpha);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2QueueAlpha);

    return GT_OK;
}
#endif

#ifdef EXMXPM_FAMILY

/**
* @internal prvTgfConvertTgfToCpssExMxPmTailDropProfileParams function
* @endinternal
*
* @brief   Convert generic into device specific Tail Drop Profile parameters.
*
* @param[in] tgfTailDropProfileParamsPtr - (pointer to) TGF Tail Drop Profile parameters.
*
* @param[out] cpssTailDropProfileParamsPtr - (pointer to) Cpss Tail Drop Profile
*                                      parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertTgfToCpssExMxPmTailDropProfileParams
(
    IN  PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  *tgfTailDropProfileParamsPtr,
    OUT CPSS_EXMXPM_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS     *cpssTailDropProfileParamsPtr
)
{
    cpssOsMemSet(cpssTailDropProfileParamsPtr, 0,
                 sizeof(CPSS_EXMXPM_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS));

    /* convert entry info into device specific format */

    /* prevent exmxpm GT_U32->GT_U16 casting warnings (possible loss of data) */
    cpssTailDropProfileParamsPtr->dp0MaxBuffNum  = (GT_U16)tgfTailDropProfileParamsPtr->dp0MaxBuffNum;
    cpssTailDropProfileParamsPtr->dp1MaxBuffNum  = (GT_U16)tgfTailDropProfileParamsPtr->dp1MaxBuffNum;
    cpssTailDropProfileParamsPtr->dp2MaxBuffNum  = (GT_U16)tgfTailDropProfileParamsPtr->dp2MaxBuffNum;
    cpssTailDropProfileParamsPtr->dp0MaxDescrNum = (GT_U16)tgfTailDropProfileParamsPtr->dp0MaxDescrNum;
    cpssTailDropProfileParamsPtr->dp1MaxDescrNum = (GT_U16)tgfTailDropProfileParamsPtr->dp1MaxDescrNum;
    cpssTailDropProfileParamsPtr->dp2MaxDescrNum = (GT_U16)tgfTailDropProfileParamsPtr->dp2MaxDescrNum;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxDescrNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxMCBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxMCBuffNum);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxMCBuffNum);

    cpssTailDropProfileParamsPtr->sharedCountersEn = (tgfTailDropProfileParamsPtr->sharedUcAndMcCountersDisable == GT_TRUE) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal prvTgfConvertCpssExMxPmToTgfTailDropProfileParams function
* @endinternal
*
* @brief   Convert device specific Tail Drop Profile parameters into generic.
*
* @param[in] cpssTailDropProfileParamsPtr - (pointer to) Cpss Tail Drop Profile
*                                      parameters.
*
* @param[out] tgfTailDropProfileParamsPtr - (pointer to) TGF Tail Drop Profile parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertCpssExMxPmToTgfTailDropProfileParams
(
    IN  CPSS_EXMXPM_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS     *cpssTailDropProfileParamsPtr,
    OUT PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  *tgfTailDropProfileParamsPtr
)
{
    cpssOsMemSet(tgfTailDropProfileParamsPtr, 0,
                 sizeof(tgfTailDropProfileParamsPtr));

    /* convert entry info into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, tcMaxDescrNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp0MaxMCBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp1MaxMCBuffNum);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssTailDropProfileParamsPtr,
                                   tgfTailDropProfileParamsPtr, dp2MaxMCBuffNum);

    tgfTailDropProfileParamsPtr->sharedUcAndMcCountersDisable = (cpssTailDropProfileParamsPtr->sharedCountersEn == GT_TRUE) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}
#endif

/**
* @internal prvTgfPortDeviceNumByPortGet function
* @endinternal
*
* @brief   Gets device number for specified port number.
*
* @param[in] portNum                  - port number
*
* @param[out] devNumPtr                - (pointer to)device number
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on port number not found in port list
*/
GT_STATUS prvTgfPortDeviceNumByPortGet
(
    IN  GT_U32                          portNum,
    OUT GT_U8                          *devNumPtr
)
{
    GT_U32  portIter = 0;

    /* find port index */
    for (portIter = 0; (portIter < prvTgfPortsNum); portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    if (portIter >= prvTgfPortsNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);
        return GT_BAD_PARAM;
    }

    *devNumPtr = prvTgfDevsArray[portIter];
    return GT_OK;
}

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfPortForceLinkDownEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8  devNum = 0;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChPortForceLinkDownEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmPortForceLinkDownSet(devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified bridge egress
*         counters mode on specified device.
* @param[in] cntrSetNum               - counter set number.
* @param[in] setModeBmp               - counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] vlanId                   - VLAN Id, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue, if corresponding bit in setModeBmp is 1.
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
GT_STATUS prvTgfPortEgressCntrModeSet
(
    IN  GT_U8                           cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_U32                           portNum,
    IN  GT_U16                          vlanId,
    IN  GT_U8                           tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
)
{
#ifdef CHX_FAMILY
    GT_U8  devNum = prvTgfDevNum;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {

        /* support device with less TC */
        tc %=
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.txQueuesNum;

        /* call device specific API */
        return cpssDxChPortEgressCntrModeSet(
            devNum, cntrSetNum, setModeBmp, portNum, vlanId, tc, dpLevel);
    }
    else
    {
        /* support device with less TC */
        tc %=
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.txQueuesNum;

        /* call device specific API */
        return cpssDxChPortEgressCntrModeSet(
            devNum, cntrSetNum, setModeBmp, portNum, vlanId, tc, dpLevel);
    }

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8  devNum;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }
    /* call device specific API */
    return cpssExMxPmBrgCntBridgeEgressCntrModeSet(
        devNum, (CPSS_EXMXPM_BRG_CNTR_SET_ID_ENT)cntrSetNum,
        setModeBmp, portNum, vlanId, tc, dpLevel);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of a bridge egress counters set.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] vlanIdPtr                - Pointer to the vlan id of the packets counted by the
*                                      set of counters.
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
GT_STATUS prvTgfPortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U8                           cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_U8                           *portNumPtr,
    OUT  GT_U16                          *vlanIdPtr,
    OUT  GT_U8                           *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChPortEgressCntrModeGet(devNum, cntrSetNum, setModeBmpPtr, (GT_PHYSICAL_PORT_NUM *)portNumPtr, vlanIdPtr, tcPtr, dpLevelPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgCntBridgeEgressCntrModeGet(
        devNum, (CPSS_EXMXPM_BRG_CNTR_SET_ID_ENT)cntrSetNum,
        setModeBmpPtr, portNumPtr, vlanIdPtr, tcPtr, dpLevelPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfPortFCPacketsCntrsGet function
* @endinternal
*
* @brief   Get the number of received and dropped Flow Control packets
*
* @param[out] receivedCntPtr           - the number of received packets
* @param[out] droppedCntPtr            - the number of dropped packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortFCPacketsCntrsGet
(
    OUT GT_U32      *receivedCntPtr,
    OUT GT_U32      *droppedCntPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                   rc, rc1 = GT_OK;
    GT_U32                      receivedCnt = 0;
    GT_U32                      droppedCnt  = 0;

    /* clear output counters */
    *receivedCntPtr = 0;
    *droppedCntPtr  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPortFlowControlPacketsCntGet (devNum, &receivedCnt, &droppedCnt);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPortFlowControlPacketsCntGet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /* add result to output counters */
        *receivedCntPtr += receivedCnt;
        *droppedCntPtr  += droppedCnt;
    }

    return rc1;




#else /* not CHX_FAMILY */

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;

#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfPortPFCAllPortsReceptionEnableSet function
* @endinternal
*
* @brief   Enable/disable PFC packets reception on all ports
*
* @param[in] enable                   -  or disable reception.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortPFCAllPortsReceptionEnableSet
(
   IN GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_U8                           devNum  = 0;
    GT_STATUS                       rc, rc1 = GT_OK;
    GT_U32                          portIndex;
    CPSS_DXCH_PORT_PFC_ENABLE_ENT   pfcEnable;
    GT_PHYSICAL_PORT_NUM            portNum;

    pfcEnable = (enable == GT_FALSE) ? CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_ONLY_E : CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPortPfcEnableSet (devNum, pfcEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPortPfcEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /* configure all ports in test  */
        for(portIndex = 0; portIndex < prvTgfPortsNum; portIndex++)
        {
            portNum =  prvTgfPortsArray[portIndex];
            rc = cpssDxChPortPfcForwardEnableSet(devNum,portNum,enable);
            if (GT_OK != rc)
            {
                if(prvCpssDxChPortRemotePortCheck(devNum, portNum) == GT_FALSE)
                {
                    PRV_UTF_LOG1_MAC(
                        "[TGF]: cpssDxChPortPfcForwardEnableSet FAILED, rc = [%d]", rc);

                    rc1 = rc;
                    continue;
                }
            }
        }
    }

    return rc1;

#else /* not CHX_FAMILY */

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;

#endif /* CHX_FAMILY */

}

GT_STATUS appDemoCaelumEgressCntrReset
(
    IN GT_U8        devNum,
    IN GT_U32       cntrSetNum
);

/**
* @internal prvTgfPortEgressCntIsCaelumErratumExist function
* @endinternal
*
* @brief   Returns GT_TRUE if device has Caelum's egress counters erratum.
*
* @param[in] devNum                   - device number
*
* @retval GT_TRUE                  - device has Caelum's egress counters erratum.
* @retval GT_FALSE                 - device doesn't has Caelum's egress counters erratum.
*/
GT_BOOL prvTgfPortEgressCntIsCaelumErratumExist
(
    IN GT_U8        devNum
)
{
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT2_E ||
       PRV_CPSS_PP_MAC(devNum)->devSubFamily != CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        return GT_FALSE;

    /* revision 0 of Caelum family has erratum */
    if (PRV_CPSS_PP_MAC(devNum)->revision)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal prvTgfPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @param[in] cntrSetNum               - counter set number : 0, 1
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortEgressCntrsGet
(
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
)
{
#if (defined EXMXPM_FAMILY)
    CPSS_EXMXPM_BRG_EGRESS_CNTR_STC pmEgrCntr;

    static CPSS_EXMXPM_BRG_EGRESS_CNTR_STC    pmLastTimeRead[2];/*WA for Puma3 that not clear on read*/
    static GT_BOOL  initDone = GT_FALSE;
    CPSS_EXMXPM_BRG_EGRESS_CNTR_STC pmEgrCntr_2;
    GT_U32  puma3_gm_clearOnRead_bug = 0;

#endif

#if (defined CHX_FAMILY)
    CPSS_PORT_EGRESS_CNTR_STC   egrCntr;
#endif

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8                       devNum  = 0;
   GT_STATUS                   rc, rc1 = GT_OK;

   /* clear output counters */
   cpssOsMemSet(egrCntrPtr, 0, sizeof(CPSS_PORT_EGRESS_CNTR_STC));

#endif /* (defined CHX_FAMILY) !! (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
       rc = cpssDxChPortEgressCntrsGet(
           devNum, cntrSetNum, &egrCntr);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChPortEgressCntrsGet FAILED, rc = [%d]", rc);

           rc1 = rc;
           continue;
       }

        /* add result to output counters */
        egrCntrPtr->brgEgrFilterDisc += egrCntr.brgEgrFilterDisc;
        egrCntrPtr->egrFrwDropFrames += egrCntr.egrFrwDropFrames;
        egrCntrPtr->outBcFrames      += egrCntr.outBcFrames;
        egrCntrPtr->outCtrlFrames    += egrCntr.outCtrlFrames;
        egrCntrPtr->outMcFrames      += egrCntr.outMcFrames;
        egrCntrPtr->outUcFrames      += egrCntr.outUcFrames;
        egrCntrPtr->txqFilterDisc    += egrCntr.txqFilterDisc;
        egrCntrPtr->mcFilterDropPkts += egrCntr.mcFilterDropPkts;
        egrCntrPtr->mcFifoDropPkts   += egrCntr.mcFifoDropPkts;
#ifndef ASIC_SIMULATION
        if (prvTgfPortEgressCntIsCaelumErratumExist(devNum))
        {
            appDemoCaelumEgressCntrReset(devNum,cntrSetNum);
        }
#endif
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
       rc = cpssExMxPmBrgCntBridgeEgressCntrsGet(
        devNum, (CPSS_EXMXPM_BRG_CNTR_SET_ID_ENT)cntrSetNum, &pmEgrCntr);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmBrgCntBridgeEgressCntrsGet FAILED, rc = [%d]", rc);

           rc1 = rc;
           continue;
       }

        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E &&
           puma3_gm_clearOnRead_bug == 1 )
        {
            if(initDone == GT_FALSE)
            {
                initDone = GT_TRUE;
                cpssOsMemSet(&pmLastTimeRead,0,sizeof(pmLastTimeRead));
            }

            pmEgrCntr_2 = pmEgrCntr;

            /* clear on read is not working . */
            pmEgrCntr.brgEgrFilterPkts  -=  pmLastTimeRead[cntrSetNum].brgEgrFilterPkts  ;
            pmEgrCntr.tailDropPkts      -=  pmLastTimeRead[cntrSetNum].tailDropPkts      ;
            pmEgrCntr.mcTailDropPkts    -=  pmLastTimeRead[cntrSetNum].mcTailDropPkts    ;
            pmEgrCntr.outBcPkts         -=  pmLastTimeRead[cntrSetNum].outBcPkts         ;
            pmEgrCntr.ctrlPkts          -=  pmLastTimeRead[cntrSetNum].ctrlPkts          ;
            pmEgrCntr.outMcPkts         -=  pmLastTimeRead[cntrSetNum].outMcPkts         ;
            pmEgrCntr.outUcPkts         -=  pmLastTimeRead[cntrSetNum].outUcPkts         ;
            pmEgrCntr.frwdRestrictPkts  -=  pmLastTimeRead[cntrSetNum].frwdRestrictPkts  ;
            /*save for next time*/
            pmLastTimeRead[cntrSetNum] = pmEgrCntr_2;
        }

       /* add result to output counters */
       egrCntrPtr->brgEgrFilterDisc += pmEgrCntr.brgEgrFilterPkts;
       egrCntrPtr->egrFrwDropFrames +=
           pmEgrCntr.tailDropPkts + pmEgrCntr.mcTailDropPkts;
       egrCntrPtr->outBcFrames      += pmEgrCntr.outBcPkts;
       egrCntrPtr->outCtrlFrames    += pmEgrCntr.ctrlPkts;
       egrCntrPtr->outMcFrames      += pmEgrCntr.outMcPkts;
       egrCntrPtr->outUcFrames      += pmEgrCntr.outUcPkts;
       egrCntrPtr->txqFilterDisc    += pmEgrCntr.frwdRestrictPkts;
   }

   return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Overiding of previous capture by another port in a group is occurs,
*       when the capture triggered port
*       (see cpssDxChPortMacCountersCaptureTriggerSet) and previous
*       capture triggered port are in the same group.
*       (This comment dosn't connected to HyperG.Stack ports.)
*       Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvTgfPortMacCountersCaptureOnPortGet
(
    IN  GT_U32                           portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8                            devNum  = 0;
   GT_STATUS                        rc = GT_OK;
   GT_BOOL                          captureIsDonePtr;
   GT_U32                           i = 0;

   if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
   {
       return GT_BAD_PARAM;
   }
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#if (defined CHX_FAMILY)
   /* Set trigger for given port */
   rc = cpssDxChPortMacCountersCaptureTriggerSet(devNum, portNum);
   if(rc != GT_OK)
   {
        GT_U32 isMtiMac;
        GT_U32 portMacNum;      /* MAC number */
        
        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
        isMtiMac = (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_64_E) || (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_400_E);
        if(rc != GT_OK && isMtiMac)
        {
            return prvTgfPortMacCountersOnPortGet(portNum, portMacCounterSetArrayPtr);
        }
   }
   if ((GT_NOT_SUPPORTED == rc) &&
       prvCpssDxChPortRemotePortCheck(devNum, portNum))
   {
       /* Remote port may not support capture - use straight read instead */
       return prvTgfPortMacCountersOnPortGet(portNum, portMacCounterSetArrayPtr);
   }
   if (GT_OK != rc)
   {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChPortMacCountersCaptureTriggerSet FAILED, rc = [%d]", rc);
   }

   captureIsDonePtr = GT_FALSE;
   i = 0;

   /* do busy wait on capture status */
   while (!captureIsDonePtr)
   {
       /* check that the capture was done */
       rc = cpssDxChPortMacCountersCaptureTriggerGet(devNum, portNum, &captureIsDonePtr);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChPortMacCountersCaptureTriggerGet FAILED, rc = [%d]", rc);
       }

       i++;

       if (i == 1000)
       {
           break;
       }

       if (i > 500)
       {
           cpssOsTimerWkAfter(0);
       }
   }

   if (captureIsDonePtr == GT_FALSE)
   {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChPortMacCountersCaptureTriggerGet FAILED, captureIsDonePtr = [%d]", captureIsDonePtr);
   }

   /* Get Port Captured MAC MIB counters */
   rc = cpssDxChPortMacCountersCaptureOnPortGet(devNum, portNum,
                                                portMacCounterSetArrayPtr);
   if (GT_OK != rc)
   {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
   }

   return rc;
#endif /* CHX_FAMILY */


#if (defined EXMXPM_FAMILY)
   /* Set trigger for given port */
   rc = cpssExMxPmPortMacCountersCaptureTriggerSet(devNum, portNum, GT_TRUE);
   if (GT_OK != rc)
   {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssExMxPmPortMacCountersCaptureTriggerSet FAILED, rc = [%d]", rc);
   }

   captureIsDonePtr = GT_TRUE;
   i = 0;

   /* do busy wait on capture status */
   while (captureIsDonePtr)
   {
       /* check that the capture was done */
       rc = cpssExMxPmPortMacCountersCaptureTriggerGet(devNum, portNum, &captureIsDonePtr);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmPortMacCountersCaptureTriggerGet FAILED, rc = [%d]", rc);
       }

       i++;

       if (i == 1000)
       {
           break;
       }
   }

   if (captureIsDonePtr == GT_TRUE)
   {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssExMxPmPortMacCountersCaptureTriggerGet FAILED, captureIsDonePtr = [%d]", captureIsDonePtr);
   }

   /* Get Port Captured MAC MIB counters */
   rc = cpssExMxPmPortCapturedMacCountersGet(devNum, portNum,
                                                portMacCounterSetArrayPtr);
   if (GT_OK != rc)
   {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssExMxPmPortCapturedMacCountersGet FAILED, rc = [%d]", rc);
   }

   return rc;
#endif /* EXMXPM_FAMILY */


#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(portMacCounterSetArrayPtr);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvTgfPortMacCountersOnPortGet
(
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
)
{
#ifdef CHX_FAMILY
{
    GT_U8                            devNum  = 0;
    GT_STATUS                        rc = GT_OK;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E)
    {
        if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
        {
           return GT_BAD_PARAM;
        }

        /* Get Port MAC MIB counters */
        rc = cpssDxChPortMacCountersOnPortGet(devNum, portNum,
                                              portMacCounterSetArrayPtr);
        if (GT_OK != rc)
        {
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChPortMacCountersOnPortGet FAILED, rc = [%d]", rc);
        }


        return rc;
    }
}
#endif /* !(defined CHX_FAMILY) */

    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(portMacCounterSetArrayPtr);


    return GT_BAD_STATE;
}


/**
* @internal prvTgfPortMacCountersRxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for received packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_FALSE - Counters are updated.
*                                      GT_TRUE - Counters are not updated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMacCountersRxHistogramEnable
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)

    GT_U8  devNum;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */
#if (defined CHX_FAMILY)
    /* call device specific API */
    return cpssDxChPortMacCountersRxHistogramEnable(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#if (defined EXMXPM_FAMILY)
    /* call device specific API */
    return cpssExMxPmPortMacCountersRxHistogramEnableSet(devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortMacCountersTxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for transmitted packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_FALSE - Counters are updated.
*                                      GT_TRUE - Counters are not updated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMacCountersTxHistogramEnable
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)

    GT_U8  devNum;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#if (defined CHX_FAMILY)
    /* call device specific API */
    return cpssDxChPortMacCountersTxHistogramEnable(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#if (defined EXMXPM_FAMILY)
    /* call device specific API */
    return cpssExMxPmPortMacCountersTxHistogramEnableSet(devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMacCountersClearOnReadSet
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable
)
{

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8  devNum;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }
    return prvTgfDevPortMacCountersClearOnReadSet(devNum, portNum, enable);
#endif /* CHX_FAMILY || EXMXPM_FAMILY */


#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get status (Enable or disable) of MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to enable clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMacCountersClearOnReadGet
(
    IN GT_U8    portNum,
    OUT GT_BOOL  *enablePtr
)
{

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8  devNum;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }

    return prvTgfDevPortMacCountersClearOnReadGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY || EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] mru                      - max receive packet size in bytes. (0..16382)
*                                      value must be even
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruSet
(
    IN GT_U32    portNum,
    IN GT_U32   mru
)
{

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8  devNum;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }
#endif /* (defined CHX_FAMILY) || (defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChPortMruSet(devNum, portNum, mru);

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmPortMruSet(devNum, portNum, mru);

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(mru);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMruGet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] mruPtr                   - max receive packet size in bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPortMruGet
(
    IN  GT_U32    portNum,
    OUT GT_U32   *mruPtr
)
{

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8  devNum;

    if (GT_OK != prvTgfPortDeviceNumByPortGet(portNum, &devNum))
    {
        return GT_BAD_PARAM;
    }
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChPortMruGet(devNum, portNum, mruPtr);

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmPortMruGet(devNum, portNum, mruPtr);

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(mruPtr);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

#ifdef CHX_FAMILY
/* static DB to store BM related configurations for SIP5 device */
static GT_BOOL eArchBMConfigSaved = GT_FALSE;
static GT_U32 eArchBMConfigRxLimitSavedArr[CPSS_PORT_RX_FC_PROFILE_8_E + 1];
static GT_U32 eArchBMConfigxOffLimitSavedArr[CPSS_PORT_RX_FC_PROFILE_8_E + 1];

/* maximal values for shared mode */
GT_U32 eArchBMConfigRxLimitMax = (BIT_16 / 4) - 1;
GT_U32 eArchBMConfigxOffLimitMax  = (BIT_16 / 2) - 1;
#endif /* !(defined CHX_FAMILY) */
/**
* @internal prvTgfPortFcHolSysModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode on the specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] modeFcHol                - Flow Control or HOL system mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortFcHolSysModeSet
(
    IN  GT_U8                   devNum,
    IN  PRV_TGF_PORT_HOL_FC_ENT modeFcHol
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_PORT_HOL_FC_ENT   dxChModeFcHol;
    CPSS_PORT_RX_FC_PROFILE_SET_ENT   profile;

    switch(modeFcHol)
    {
        case PRV_TGF_PORT_PORT_FC_E: dxChModeFcHol = CPSS_DXCH_PORT_FC_E;
                                     break;

        case PRV_TGF_PORT_PORT_HOL_E: dxChModeFcHol = CPSS_DXCH_PORT_HOL_E;
                                      break;

        default: return GT_BAD_PARAM;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (eArchBMConfigSaved == GT_FALSE)
        {
            for (profile = CPSS_PORT_RX_FC_PROFILE_1_E; profile <= CPSS_PORT_RX_FC_PROFILE_8_E; profile++)
            {
                rc = cpssDxChPortRxBufLimitGet(devNum, profile, &eArchBMConfigRxLimitSavedArr[profile]);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortRxBufLimitGet FAILED, rc = [%d]", rc);
                    return rc;
                }

                rc = cpssDxChPortXoffLimitGet(devNum, profile, &eArchBMConfigxOffLimitSavedArr[profile]);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortXoffLimitGet FAILED, rc = [%d]", rc);
                    return rc;
                }
            }
            eArchBMConfigSaved = GT_TRUE;
        }
    }


    /* call device specific API */
    rc = cpssDxChPortFcHolSysModeSet(devNum, dxChModeFcHol);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortFcHolSysModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* emulate shared or divided mode of BM for eArch devices */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (modeFcHol == PRV_TGF_PORT_PORT_FC_E)
        {
            /* restore default settings those represent divided mode */
            for (profile = CPSS_PORT_RX_FC_PROFILE_1_E; profile <= CPSS_PORT_RX_FC_PROFILE_8_E; profile++)
            {
                rc = cpssDxChPortRxBufLimitSet(devNum, profile, eArchBMConfigRxLimitSavedArr[profile]);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortRxBufLimitSet FAILED, rc = [%d]", rc);
                    return rc;
                }

                rc = cpssDxChPortXoffLimitSet(devNum, profile, eArchBMConfigxOffLimitSavedArr[profile]);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortXoffLimitSet FAILED, rc = [%d]", rc);
                    return rc;
                }
            }
        }
        else if (modeFcHol == PRV_TGF_PORT_PORT_HOL_E)
        {
            /* emulate shared mode - set Rx and xOff limits to maximal values.
              only global limits are working in this mode. */
            for (profile = CPSS_PORT_RX_FC_PROFILE_1_E; profile <= CPSS_PORT_RX_FC_PROFILE_8_E; profile++)
            {
                rc = cpssDxChPortRxBufLimitSet(devNum, profile, eArchBMConfigRxLimitMax);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortRxBufLimitSet FAILED, rc = [%d]", rc);
                    return rc;
                }

                rc = cpssDxChPortXoffLimitSet(devNum, profile, eArchBMConfigxOffLimitMax);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortXoffLimitSet FAILED, rc = [%d]", rc);
                    return rc;
                }
            }
        }
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    CPSS_EXMXPM_PORT_HOL_FC_ENT   exmxpmModeFcHol;

    switch(modeFcHol)
    {
        case PRV_TGF_PORT_PORT_FC_E: exmxpmModeFcHol = CPSS_EXMXPM_PORT_FC_E;
                                     break;

        case PRV_TGF_PORT_PORT_HOL_E: exmxpmModeFcHol = CPSS_EXMXPM_PORT_HOL_E;
                                      break;

        default: return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssExMxPmPortFlowControlHolSysModeSet(devNum, exmxpmModeFcHol);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortFlowControlHolSysModeSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(modeFcHol);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropProfileSet function
* @endinternal
*
* @brief Configures dynamic limit alpha and sets maximal port's limits of buffers
*        and descriptors.
*
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                       thresholds.
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
* @param[in] portMaxDescrLimit        - maximal number of descriptors for a port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropProfileSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    IN  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT portAlpha,
    IN  GT_U32                               portMaxBuffLimit,
    IN  GT_U32                               portMaxDescrLimit
)
{

#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxTailDropProfileSet(devNum, profileSet, portAlpha,
                                          portMaxBuffLimit, portMaxDescrLimit);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTailDropProfileSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxProfileTailDropLimitsSet(devNum, profileSet,
                                          portMaxBuffLimit, portMaxDescrLimit,
                                          CPSS_EXMXPM_PORT_TX_SHARED_POOL_USERS_GREEN_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxProfileTailDropLimitsSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(portAlpha);
    TGF_PARAM_NOT_USED(portMaxBuffLimit);
    TGF_PARAM_NOT_USED(portMaxDescrLimit);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropProfileWithoutAlphaSet function
* @endinternal
*
* @brief Set maximal port's limits of buffers and descriptors. Does not configure
*        dynamic limit alpha.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
* @param[in] portMaxDescrLimit        - maximal number of descriptors for a port.
*/
GT_STATUS prvTgfPortTxTailDropProfileWithoutAlphaSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    IN  GT_U32                               portMaxBuffLimit,
    IN  GT_U32                               portMaxDescrLimit
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alpha;
    GT_U32      portMaxBuffLimitGet;
    GT_U32      portMaxDescrLimitGet;

    /* call device specific API */

    /* Save alpha */
    rc = cpssDxChPortTxTailDropProfileGet(devNum, profileSet, &alpha,
                                          &portMaxBuffLimitGet, &portMaxDescrLimitGet);

    rc = cpssDxChPortTxTailDropProfileSet(devNum, profileSet, alpha,
                                          portMaxBuffLimit, portMaxDescrLimit);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTailDropProfileSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(portMaxBuffLimit);
    TGF_PARAM_NOT_USED(portMaxDescrLimit);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropProfileGet function
* @endinternal
*
* @brief Get dynamic limit alpha of an egress port and get maximal port's limits
*        of buffers and descriptors.
*
*
* @param[in] devNum                    - physical device number
* @param[in] profileSet                - the Profile Set in which the Traffic
*                                        Class Drop Parameters is associated
*
* @param[out] portAlphaPtr             - Pointer to ratio of the free buffers used for the
*                                        port thresholds.
* @param[out] portMaxBuffLimitPtr      - Pointer to maximal number of buffers for a port
* @param[out] portMaxDescrLimitPtr     - Pointer to maximal number of descriptors for a port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropProfileGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT           profileSet,
    OUT CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT        *portAlphaPtr,
    OUT GT_U32                                      *portMaxBuffLimitPtr,
    OUT GT_U32                                      *portMaxDescrLimitPtr
)
{

#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxTailDropProfileGet(devNum, profileSet, portAlphaPtr,
                                          portMaxBuffLimitPtr, portMaxDescrLimitPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTailDropProfileGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    CPSS_EXMXPM_PORT_TX_SHARED_POOL_USERS_ENT sharedPoolUsers;
    GT_U32   portMaxBuffLimit;
    GT_U32   portMaxDescrLimit;

    /* call device specific API */
    rc = cpssExMxPmPortTxProfileTailDropLimitsGet(devNum, profileSet,
                                          &portMaxBuffLimit, &portMaxDescrLimit,
                                          &sharedPoolUsers);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxProfileTailDropLimitsGet FAILED, rc = [%d]", rc);
    }
    *portMaxBuffLimitPtr = portMaxBuffLimit;
    *portMaxDescrLimitPtr = portMaxDescrLimit;

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(portAlphaPtr);
    TGF_PARAM_NOT_USED(portMaxBuffLimitPtr);
    TGF_PARAM_NOT_USED(portMaxDescrLimitPtr);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropProfileWithoutAlphaGet function
* @endinternal
*
* @brief Get maximal port's limits of buffers and descriptors.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum                    - physical device number
* @param[in] profileSet                - the Profile Set in which the Traffic
*                                        Class Drop Parameters is associated
* @param[out] portMaxBuffLimitPtr      - Pointer to maximal number of buffers for a port
* @param[out] portMaxDescrLimitPtr     - Pointer to maximal number of descriptors for a port
*/
GT_STATUS prvTgfPortTxTailDropProfileWithoutAlphaGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT           profileSet,
    OUT GT_U32                                      *portMaxBuffLimitPtr,
    OUT GT_U32                                      *portMaxDescrLimitPtr
)
{

#ifdef CHX_FAMILY
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alpha;
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxTailDropProfileGet(devNum, profileSet, &alpha,
                                          portMaxBuffLimitPtr, portMaxDescrLimitPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTailDropProfileGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(alphaPtr);
    TGF_PARAM_NOT_USED(portMaxBuffLimitPtr);
    TGF_PARAM_NOT_USED(portMaxDescrLimitPtr);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTx4TcTailDropProfileSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters.
* @param[in] tailDropProfileParamsPtr -
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTx4TcTailDropProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *tailDropProfileParamsPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS   rc = GT_OK;
#endif /*(defined CHX_FAMILY) !! (defined EXMXPM_FAMILY)*/

#ifdef CHX_FAMILY
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS   cpssTailDropProfileParams;

    rc = prvTgfConvertTgfToCpssTailDropProfileParams(tailDropProfileParamsPtr,
                                                         &cpssTailDropProfileParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertTgfToCpssTailDropProfileParams FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChPortTx4TcTailDropProfileSet(devNum, profileSet, trafficClass,
                                             &cpssTailDropProfileParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTx4TcTailDropProfileSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS   cpssTailDropProfileParams;

    rc = prvTgfConvertTgfToCpssExMxPmTailDropProfileParams(tailDropProfileParamsPtr,
                                                         &cpssTailDropProfileParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertTgfToCpssTailDropProfileParams FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmPortTxTailDropProfileSet(devNum, profileSet, trafficClass,
                                             &cpssTailDropProfileParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTailDropProfileSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(trafficClass);
    TGF_PARAM_NOT_USED(tailDropProfileParamsPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTx4TcTailDropProfileGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters.
*
* @param[out] tailDropProfileParamsPtr - Pointer to
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTx4TcTailDropProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *tailDropProfileParamsPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS   rc = GT_OK;
#endif /*(defined CHX_FAMILY) !! (defined EXMXPM_FAMILY)*/

#ifdef CHX_FAMILY
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS   cpssTailDropProfileParams;
    cpssOsMemSet(&cpssTailDropProfileParams, 0, sizeof(cpssTailDropProfileParams));

    /* call device specific API */
    rc = cpssDxChPortTx4TcTailDropProfileGet(devNum, profileSet, trafficClass,
                                             &cpssTailDropProfileParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxBindPortToDpSet FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = prvTgfConvertCpssToTgfTailDropProfileParams(&cpssTailDropProfileParams,
                                                         tailDropProfileParamsPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertCpssToTgfTailDropProfileParams FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS   cpssTailDropProfileParams;

    cpssOsMemSet(&cpssTailDropProfileParams, 0, sizeof(cpssTailDropProfileParams));

    /* call device specific API */
    rc = cpssExMxPmPortTxTailDropProfileGet(devNum, profileSet, trafficClass,
                                             &cpssTailDropProfileParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTailDropProfileGet FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = prvTgfConvertCpssExMxPmToTgfTailDropProfileParams(&cpssTailDropProfileParams,
                                                         tailDropProfileParamsPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertCpssToTgfTailDropProfileParams FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(trafficClass);
    TGF_PARAM_NOT_USED(tailDropProfileParamsPtr);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxBindPortToDpSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfPortTxBindPortToDpSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                               portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet
)
{

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS   rc = GT_OK;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChPortTxBindPortToDpSet(devNum, portNum, profileSet);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxBindPortToDpSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmPortTxTailDropProfileIndexSet(devNum, portNum, profileSet);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTailDropProfileIndexSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(profileSet);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxBindPortToDpGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - Pointer to the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxBindPortToDpGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                               portNum,
    OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
)
{

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS   rc = GT_OK;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChPortTxBindPortToDpGet(devNum, portNum, profileSetPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxBindPortToDpGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U32 index;

    /* call device specific API */
    rc = cpssExMxPmPortTxTailDropProfileIndexGet(devNum, portNum, &index);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTailDropProfileIndexGet FAILED, rc = [%d]", rc);

        return rc;
    }

    *profileSetPtr = (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)index;
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(profileSetPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission from a Traffic Class queue
*         on the specified port of specified device.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port
* @param[in] enable                   - GT_TRUE,  transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueTxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U32     portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
)
{

#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS   rc = GT_OK;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChPortTxQueueTxEnableSet(devNum, portNum, tcQueue, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxQueueTxEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmPortTxQueueTxEnableSet(devNum, portNum, tcQueue, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxQueueTxEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission from a Traffic Class queue
*         on the specified port of specified device (Enable/Disable).
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port (0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueTxEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U32     portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxQueueTxEnableGet(devNum, portNum, tcQueue,enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxQueueTxEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxQueueTxEnableGet(devNum, portNum, tcQueue,enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxQueueTxEnableGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxResourceHistogramThresholdSet function
* @endinternal
*
* @brief   Sets threshold for Histogram counter increment.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrNum                  - Histogram Counter number. Range 0..3.
* @param[in] threshold                - If the Global Descriptors Counter exceeds this Threshold,
*                                      the Histogram Counter is incremented by 1.
*                                      Range 0..0x3FFF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxResourceHistogramThresholdSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  cntrNum,
    IN  GT_U32  threshold
)
{

#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxResourceHistogramThresholdSet(devNum, cntrNum, threshold);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxResourceHistogramThresholdSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cntrNum);
    TGF_PARAM_NOT_USED(threshold);


    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxResourceHistogramCounterGet function
* @endinternal
*
* @brief   Gets Histogram Counter.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrNum                  - Histogram Counter number. Range 0..3.
*
* @param[out] cntrPtr                  - (pointer to) Histogram Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Histogram Counter is cleared on read.
*
*/
GT_STATUS prvTgfPortTxResourceHistogramCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  cntrNum,
    OUT GT_U32  *cntrPtr
)
{

#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxResourceHistogramCounterGet(devNum, cntrNum, cntrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxResourceHistogramCounterGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cntrNum);
    TGF_PARAM_NOT_USED(cntrPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxGlobalBufNumberGet function
* @endinternal
*
* @brief   Gets total number of virtual buffers enqueued.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*
*/
GT_STATUS prvTgfPortTxGlobalBufNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxGlobalBufNumberGet(devNum, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxGlobalBufNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    GT_U32      descNum;

    /* call device specific API */
    rc = cpssExMxPmPortTxGlobalBuffDescNumberGet(devNum, numberPtr, &descNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxGlobalBuffDescNumberGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(numberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxGlobalDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxGlobalDescNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxGlobalDescNumberGet(devNum, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxGlobalDescNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    GT_U32      buffNum;

    /* call device specific API */
    rc = cpssExMxPmPortTxGlobalBuffDescNumberGet(devNum, &buffNum, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxGlobalBuffDescNumberGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(numberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSharedResourceDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated from the shared descriptors
*         pool.
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - shared pool number, range 0..7
*
* @param[out] numberPtr                - (pointer to) number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharedResourceDescNumberGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    OUT GT_U32 *numberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSharedResourceDescNumberGet(devNum, poolNum, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSharedResourceDescNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;
    GT_U32      tmp;

    /* call device specific API */
    rc = cpssExMxPmPortTxSharedResourceBuffDescNumberGet(devNum, poolNum, &tmp, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxSharedResourceBuffDescNumberGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(poolNum);
    TGF_PARAM_NOT_USED(numberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSharedResourceBufNumberGet function
* @endinternal
*
* @brief   Gets the number of virtual buffers enqueued from the shared buffer pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - shared pool number, range 0..7
*
* @param[out] numberPtr                - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharedResourceBufNumberGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    OUT GT_U32 *numberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSharedResourceBufNumberGet(devNum, poolNum, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSharedResourceBufNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;
    GT_U32      tmp;

    /* call device specific API */
    rc = cpssExMxPmPortTxSharedResourceBuffDescNumberGet(devNum, poolNum, numberPtr, &tmp);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxSharedResourceBuffDescNumberGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(poolNum);
    TGF_PARAM_NOT_USED(numberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSniffedBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for mirrored packets.
*
* @param[in] devNum                   - physical device number
* @param[in] rxSniffMaxBufNum         - The number of buffers allocated for packets
*                                      forwarded to the ingress analyzer port due to
*                                      mirroring. Range 0..0xFFFF
* @param[in] txSniffMaxBufNum         - The number of buffers allocated for packets
*                                      forwarded to the egress analyzer port due to
*                                      mirroring. Range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS prvTgfPortTxSniffedBuffersLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  rxSniffMaxBufNum,
    IN  GT_U32  txSniffMaxBufNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSniffedBuffersLimitSet(devNum, rxSniffMaxBufNum, txSniffMaxBufNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedBuffersLimitSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(rxSniffMaxBufNum);
    TGF_PARAM_NOT_USED(txSniffMaxBufNum);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSniffedPcktDescrLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for mirrored packets.
*
* @param[in] devNum                   - physical device number
* @param[in] rxSniffMaxDescNum        - The number of descriptors allocated for packets
*                                      forwarded to the ingress analyzer port due to
*                                      mirroring.
*                                      For DxCh1,DxCh2: range 0..0xFFF
*                                      For DxCh3,xCat,xCat2: range 0..0x3FFF
*                                      For Lion: range 0..0xFFFF
* @param[in] txSniffMaxDescNum        - The number of descriptors allocated for packets
*                                      forwarded to the egress analyzer port due to
*                                      mirroring.
*                                      For DxCh,DxCh2: range 0..0xFFF
*                                      For DxCh3,xCat,xCat2: range 0..0x3FFF
*                                      For Lion: range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS prvTgfPortTxSniffedPcktDescrLimitSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 rxSniffMaxDescNum,
    IN    GT_U32                 txSniffMaxDescNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSniffedPcktDescrLimitSet(devNum, rxSniffMaxDescNum, txSniffMaxDescNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedPcktDescrLimitSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(rxSniffMaxDescNum);
    TGF_PARAM_NOT_USED(txSniffMaxDescNum);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSniffedPcktDescrLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for mirrored packets.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxSniffMaxDescNumPtr     - (pointer to) The number of descriptors allocated
*                                      for packets forwarded to the ingress analyzer
*                                      port due to mirroring.
* @param[out] txSniffMaxDescNumPtr     - (pointer to) The number of descriptors allocated
*                                      for packets forwarded to the egress analyzer
*                                      port due to mirroring.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSniffedPcktDescrLimitGet
(
    IN    GT_U8                  devNum,
    OUT   GT_U32                *rxSniffMaxDescNumPtr,
    OUT   GT_U32                *txSniffMaxDescNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSniffedPcktDescrLimitGet(devNum, rxSniffMaxDescNumPtr, txSniffMaxDescNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortTxSniffedPcktDescrLimitGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(rxSniffMaxDescNumPtr);
    TGF_PARAM_NOT_USED(txSniffMaxDescNumPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSniffedDescNumberGet function
* @endinternal
*
* @brief   Gets total number of mirrored descriptors allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxNumberPtr              - (pointer to) number of ingress mirrored descriptors.
* @param[out] txNumberPtr              - (pointer to) number of egress mirrored descriptors.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSniffedDescNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *rxNumberPtr,
    OUT GT_U32 *txNumberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSniffedDescNumberGet(devNum, rxNumberPtr, txNumberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedDescNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxMcastSniffedDescNumberGet(devNum, rxNumberPtr, txNumberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxMcastSniffedDescNumberGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(rxNumberPtr);
    TGF_PARAM_NOT_USED(txNumberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSniffedBufNumberGet function
* @endinternal
*
* @brief   Gets total number of mirrored buffers allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxNumberPtr              - (pointer to) number of ingress mirrored buffers.
* @param[out] txNumberPtr              - (pointer to) number of egress mirrored buffers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSniffedBufNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *rxNumberPtr,
    OUT GT_U32 *txNumberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSniffedBufNumberGet(devNum, rxNumberPtr, txNumberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSniffedBufNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxSniffedBufNumberGet(devNum, rxNumberPtr, txNumberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxSniffedBufNumberGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(rxNumberPtr);
    TGF_PARAM_NOT_USED(txNumberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxMcastDescNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target descriptors allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS prvTgfPortTxMcastDescNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxMcastDescNumberGet(devNum, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxMcastDescNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;
    GT_U32      tmp;

    /* call device specific API */
    rc = cpssExMxPmPortTxMcastDescBuffNumberGet(devNum, &tmp, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxMcastDescBuffNumberGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(numberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxMcastBufNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target buffers allocated (virtual buffers).
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*       Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS prvTgfPortTxMcastBufNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxMcastBufNumberGet(devNum, numberPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxMcastBufNumberGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;
    GT_U32      tmp;

    /* call device specific API */
    rc = cpssExMxPmPortTxMcastDescBuffNumberGet(devNum, numberPtr, &tmp);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxMcastDescBuffNumberGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(numberPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTcProfileSharedPoolSet function
* @endinternal
*
* @brief   Sets the shared pool associated for traffic class and Profile.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
* @param[in] poolNum                  - shared pool associated, range 0..7.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcProfileSharedPoolSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT pfSet,
    IN  GT_U8                             tc,
    IN  GT_U32                            poolNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxTcProfileSharedPoolSet(devNum, pfSet, tc, poolNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTcProfileSharedPoolSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxTcProfileSharedPoolSet(devNum, pfSet, tc, poolNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTcProfileSharedPoolSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(pfSet);
    TGF_PARAM_NOT_USED(tc);
    TGF_PARAM_NOT_USED(poolNum);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTcProfileSharedPoolGet function
* @endinternal
*
* @brief   Gets the shared pool associated for traffic class and Profile.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
*
* @param[out] poolNumPtr               - (pointer to) shared pool associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcProfileSharedPoolGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT pfSet,
    IN  GT_U8                             tc,
    OUT GT_U32                            *poolNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxTcProfileSharedPoolGet(devNum, pfSet, tc, poolNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTcProfileSharedPoolGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxTcProfileSharedPoolGet(devNum, pfSet, tc, poolNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTcProfileSharedPoolGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(pfSet);
    TGF_PARAM_NOT_USED(tc);
    TGF_PARAM_NOT_USED(poolNumPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number. Range 0..7
* @param[in] maxBufNum                - The number of buffers allocated for a shared pool.
*                                      Range 0..0x3FFF.
* @param[in] maxDescNum               - The number of descriptors allocated for a shared pool.
*                                      Range 0..0x3FFF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS prvTgfPortTxSharedPoolLimitsSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    IN  GT_U32 maxBufNum,
    IN  GT_U32 maxDescNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSharedPoolLimitsSet(devNum, poolNum, maxBufNum, maxDescNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSharedPoolLimitsSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxSharedPoolLimitsSet(devNum, poolNum, maxBufNum, maxDescNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxSharedPoolLimitsSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(poolNum);
    TGF_PARAM_NOT_USED(maxBufNum);
    TGF_PARAM_NOT_USED(maxDescNum);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSharedPoolLimitsGet function
* @endinternal
*
* @brief   Get maximal descriptors and buffers limits for shared pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number. Range 0..7
*
* @param[out] maxBufNumPtr             - (pointer to) The number of buffers allocated for a
*                                      shared pool.
* @param[out] maxDescNumPtr            - (pointer to) The number of descriptors allocated
*                                      for a shared pool.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharedPoolLimitsGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    OUT GT_U32 *maxBufNumPtr,
    OUT GT_U32 *maxDescNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSharedPoolLimitsGet(devNum, poolNum, maxBufNumPtr, maxDescNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSharedPoolLimitsGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxSharedPoolLimitsGet(devNum, poolNum, maxBufNumPtr, maxDescNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxSharedPoolLimitsGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(poolNum);
    TGF_PARAM_NOT_USED(maxBufNumPtr);
    TGF_PARAM_NOT_USED(maxDescNumPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTcSharedProfileEnableSet function
* @endinternal
*
* @brief   Enable/Disable usage of the shared descriptors / buffer pool for
*         packets with the traffic class (tc) that are transmited via a
*         port that is associated with the Profile (pfSet).
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
* @param[in] enableMode               - Drop Precedence (DPs) enabled mode for sharing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcSharedProfileEnableSet
(
    IN  GT_U8                              devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT  pfSet,
    IN  GT_U8                              tc,
    IN  PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT enableMode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_PORT_TX_SHARED_DP_MODE_ENT dxChEnableMode;

    /* convert into device specific format */
    switch(enableMode)
    {
        case PRV_TGF_PORT_TX_SHARED_DP_MODE_DISABLE_E:
            dxChEnableMode = CPSS_PORT_TX_SHARED_DP_MODE_DISABLE_E;
            break;

        case PRV_TGF_PORT_TX_SHARED_DP_MODE_ALL_E:
            dxChEnableMode = CPSS_PORT_TX_SHARED_DP_MODE_ALL_E;
            break;

        case PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E:
            dxChEnableMode = CPSS_PORT_TX_SHARED_DP_MODE_DP0_E;
            break;

        case PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_DP1_E:
            dxChEnableMode = CPSS_PORT_TX_SHARED_DP_MODE_DP0_DP1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChPortTxTcSharedProfileEnableSet(devNum, pfSet, tc, dxChEnableMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTcSharedProfileEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;
    CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_ENT exmxpmEnableMode;

    /* convert into device specific format */
    switch(enableMode)
    {
        case PRV_TGF_PORT_TX_SHARED_DP_MODE_DISABLE_E:
            exmxpmEnableMode = CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_DISABLE_E;
            break;

        case PRV_TGF_PORT_TX_SHARED_DP_MODE_ALL_E:
            exmxpmEnableMode = CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_ALL_E;
            break;

        case PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E:
            exmxpmEnableMode = CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_DP0_E;
            break;

        case PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_DP1_E:
            exmxpmEnableMode = CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_DP0_DP1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssExMxPmPortTxTcSharedProfileEnableSet(devNum, pfSet, tc, exmxpmEnableMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTcSharedProfileEnableSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(pfSet);
    TGF_PARAM_NOT_USED(tc);
    TGF_PARAM_NOT_USED(enableMode);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTcSharedProfileEnableGet function
* @endinternal
*
* @brief   Gets usage of the shared descriptors / buffer pool status for
*         packets with the traffic class (tc) that are transmited via a
*         port that is associated with the Profile (pfSet).
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
*
* @param[out] enableModePtr            - (pointer to) Drop Precedence (DPs) enabled mode for
*                                      sharing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcSharedProfileEnableGet
(
    IN  GT_U8                              devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT  pfSet,
    IN  GT_U8                              tc,
    OUT PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT *enableModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_PORT_TX_SHARED_DP_MODE_ENT dxChEnableMode;

    /* call device specific API */
    rc = cpssDxChPortTxTcSharedProfileEnableGet(devNum, pfSet, tc, &dxChEnableMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTcSharedProfileEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert from device specific format */
    switch(dxChEnableMode)
    {
        case CPSS_PORT_TX_SHARED_DP_MODE_DISABLE_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_DISABLE_E;
            break;

        case CPSS_PORT_TX_SHARED_DP_MODE_ALL_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_ALL_E;
            break;

        case CPSS_PORT_TX_SHARED_DP_MODE_DP0_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E;
            break;

        case CPSS_PORT_TX_SHARED_DP_MODE_DP0_DP1_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_DP1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_ENT exmxpmEnableMode;

    /* call device specific API */
    rc = cpssExMxPmPortTxTcSharedProfileEnableGet(devNum, pfSet, tc, &exmxpmEnableMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTcSharedProfileEnableGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* convert from device specific format */
    switch(exmxpmEnableMode)
    {
        case CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_DISABLE_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_DISABLE_E;
            break;

        case CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_ALL_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_ALL_E;
            break;

        case CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_DP0_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E;
            break;

        case CPSS_EXMXPM_PORT_TX_SHARED_DP_MODE_DP0_DP1_E:
            *enableModePtr = PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_DP1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(pfSet);
    TGF_PARAM_NOT_USED(tc);
    TGF_PARAM_NOT_USED(enableModePtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxProfileWeightedRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Weighted Random Tail Drop Threshold to overcome
*         synchronization.
* @param[in] devNum                   - device number.
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] dp                       - Drop Precedence
*                                      (APPLICABLE RANGES: Lion 0..2; xCat2 0..1)
* @param[in] tc                       - the Traffic Class, range 0..7.
*                                      (APPLICABLE DEVICES xCat2)
* @param[in] enablersPtr              - (pointer to) Tail Drop limits enabling
*                                      for Weigthed Random Tail Drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxProfileWeightedRandomTailDropEnableSet
(
    IN GT_U8                                       devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT           pfSet,
    IN GT_U32                                      dp,
    IN GT_U8                                       tc,
    IN PRV_TGF_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                rc = GT_OK;
    CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC dxChEnablers = {0};

    /* convert into device specific format */
    dxChEnablers.tcDpLimit       = enablersPtr->tcDpLimit;
    dxChEnablers.portLimit       = enablersPtr->portLimit;
    dxChEnablers.tcLimit         = enablersPtr->tcLimit;
    dxChEnablers.sharedPoolLimit = enablersPtr->sharedPoolLimit;

    /* call device specific API */
    rc = cpssDxChPortTxProfileWeightedRandomTailDropEnableSet(devNum, pfSet, dp, tc, &dxChEnablers);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxProfileWeightedRandomTailDropEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS                                rc;
    CPSS_EXMXPM_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC exmxpmEnablers;

    cpssOsMemSet(&exmxpmEnablers, 0, sizeof(exmxpmEnablers));

    /* convert into device specific format */
    exmxpmEnablers.tcDpLimit       = enablersPtr->tcDpLimit;
    exmxpmEnablers.portLimit       = enablersPtr->portLimit;
    exmxpmEnablers.tcLimit         = enablersPtr->tcLimit;
    exmxpmEnablers.sharedPoolLimit = enablersPtr->sharedPoolLimit;

    /* call device specific API */
    rc = cpssExMxPmPortTxTailDropWrtdProfileEnableSet(devNum, pfSet, dp, tc, &exmxpmEnablers);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTailDropWrtdProfileEnableSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(pfSet);
    TGF_PARAM_NOT_USED(dp);
    TGF_PARAM_NOT_USED(tc);
    TGF_PARAM_NOT_USED(enablersPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropWrtdMasksSet function
* @endinternal
*
* @brief   Sets Weighted Random Tail Drop (WRTD) masks.
*
* @param[in] devNum                   - physical device number
* @param[in] maskLsbPtr               - WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropWrtdMasksSet
(
    IN GT_U8                         devNum,
    IN PRV_TGF_PORT_TX_WRTD_MASK_LSB *maskLsbPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                  rc = GT_OK;
    CPSS_PORT_TX_WRTD_MASK_LSB_STC dxChMaskLsb = {0};

    /* convert into device specific format */
    dxChMaskLsb.tcDp = maskLsbPtr->tcDp;
    dxChMaskLsb.port = maskLsbPtr->port;
    dxChMaskLsb.tc   = maskLsbPtr->tc;
    dxChMaskLsb.pool = maskLsbPtr->pool;

    /* call device specific API */
    rc = cpssDxChPortTxTailDropWrtdMasksSet(devNum, &dxChMaskLsb);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTailDropWrtdMasksSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS                                rc;
    CPSS_EXMXPM_PORT_TX_WRTD_MASK_STC   exmxpmMaskLsb;

    cpssOsMemSet(&exmxpmMaskLsb, 0, sizeof(exmxpmMaskLsb));

    /* convert into device specific format */
    exmxpmMaskLsb.tcDp = maskLsbPtr->tcDp;
    exmxpmMaskLsb.port = maskLsbPtr->port;
    exmxpmMaskLsb.tc   = maskLsbPtr->tc;
    exmxpmMaskLsb.pool = maskLsbPtr->pool;

    /* call device specific API */
    rc = cpssExMxPmPortTxTailDropWrtdMasksSet(devNum, &exmxpmMaskLsb);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxTailDropWrtdMasksSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maskLsbPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxMcastBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for multicast packets.
*
* @param[in] devNum                   - physical device number
* @param[in] mcastMaxBufNum           - The number of buffers allocated for multicast
*                                      packets. Range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range mcastMaxBufNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastBuffersLimitSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 mcastMaxBufNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxMcastBuffersLimitSet(devNum, mcastMaxBufNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxMcastBuffersLimitSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxMcPacketBuffersLimitSet(devNum, mcastMaxBufNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxMcPacketBuffersLimitSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mcastMaxBufNum);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxMcastBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for multicast packets.
*
* @param[in] devNum                   - physical device number
*
* @param[out] mcastMaxBufNumPtr        - (pointer to) the number of buffers allocated
*                                      for multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastBuffersLimitGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mcastMaxBufNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxMcastBuffersLimitGet(devNum, mcastMaxBufNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxMcastBuffersLimitGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxMcPacketBuffersLimitGet(devNum, mcastMaxBufNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxMcPacketBuffersLimitGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mcastMaxBufNumPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxMcastPcktDescrLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for multicast packets.
*
* @param[in] devNum                   - physical device number
* @param[in] mcastMaxDescNum          - The number of descriptors allocated for multicast
*                                      packets.
*                                      For all devices except Lion and above: in units of 128
*                                      descriptors, the actual number descriptors
*                                      allocated will be 128  mcastMaxDescNum.
*                                      For Lion and above: actual descriptors number (granularity
*                                      of 1).
*                                      For DxCh,DxCh2,DxCh3,DxChXcat: range 0..0xF80
*                                      For Lion and above: range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastPcktDescrLimitSet
(
    IN    GT_U8  devNum,
    IN    GT_U32 mcastMaxDescNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxMcastPcktDescrLimitSet(devNum, mcastMaxDescNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxMcastPcktDescrLimitSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxMcPacketDescriptorsLimitSet(devNum, mcastMaxDescNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxMcPacketDescriptorsLimitSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mcastMaxDescNum);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxMcastPcktDescrLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for multicast packets.
*
* @param[in] devNum                   - physical device number
*
* @param[out] mcastMaxDescNumPtr       - (pointer to) the number of descriptors allocated
*                                      for multicast packets.
*                                      For all devices except Lion and above: in units of 128
*                                      descriptors, the actual number descriptors
*                                      allocated will be 128  mcastMaxDescNum.
*                                      For Lion and above: actual descriptors number (granularity
*                                      of 1).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastPcktDescrLimitGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *mcastMaxDescNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxMcastPcktDescrLimitGet(devNum, mcastMaxDescNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxMcastPcktDescrLimitGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxMcPacketDescriptorsLimitGet(devNum, mcastMaxDescNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxMcPacketDescriptorsLimitGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mcastMaxDescNumPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue
*         on the specified port of specified device.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this device (0..7)
* @param[in] enable                   - GT_TRUE,  enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U32     portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxQueueingEnableSet(devNum, portNum, tcQueue,enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxQueueingEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxQueueingEnableSet(devNum, portNum, tcQueue,enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxQueueingEnableSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxQueueingEnableGet function
* @endinternal
*
* @brief   Get the status of enqueuing to a Traffic Class queue
*         on the specified port of specified device (Enable/Disable).
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this device (0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueingEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxQueueingEnableGet(devNum, portNum, tcQueue,enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxQueueingEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmPortTxQueueingEnableGet(devNum, portNum, tcQueue,enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortTxQueueingEnableGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvTgfPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortSpeedGet(devNum, portNum, speedPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortSpeedGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
#ifdef EXMXPM_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssExMxPmPortSpeedGet(devNum, portNum, speedPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortSpeedGet FAILED, rc = [%d]", rc);
    }

    return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(speedPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed and FE ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortFlowControlEnableSet(devNum, portNum, state);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortFlowControlEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(state);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSharingGlobalResourceEnableSet function
* @endinternal
*
* @brief   Enable/Disable sharing of resources for enqueuing of packets.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE   -  TX queue resourses sharing
*                                      GT_FALSE - disable TX queue resources sharing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharingGlobalResourceEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortTxSharingGlobalResourceEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxSharingGlobalResourceEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSchedulerArbGroupSet function
* @endinternal
*
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue
* @param[in] arbGroup                 - scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device or arbGroup
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerArbGroupSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileSet,
    IN  GT_U32                                  tcQueue,
    IN  CPSS_PORT_TX_Q_ARB_GROUP_ENT            arbGroup
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profile;
    GT_U8     tc;

    profile = (CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT)profileSet;
    tc = (GT_U8)tcQueue;
    /* call device specific API */
    rc = cpssDxChPortTxQArbGroupSet(devNum, tc, arbGroup, profile);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxQArbGroupSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(arbGroup);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfPortTxSchedulerArbGroupGet function
* @endinternal
*
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerArbGroupGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileSet,
    IN  GT_U32                                  tcQueue,
    OUT CPSS_PORT_TX_Q_ARB_GROUP_ENT            *arbGroupPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profile;
    GT_U8     tc;

    profile = (CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT)profileSet;
    tc = (GT_U8)tcQueue;

    /* call device specific API */
    rc = cpssDxChPortTxQArbGroupGet(devNum, tc, profile, arbGroupPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxQArbGroupGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(arbGroupPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSchedulerProfileIndexSet function
* @endinternal
*
* @brief   Bind a port to scheduler profile set.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number,
*                                      or the Queue Group number in case of Aggregate Mode
*                                      in ExMxPm XG device.
* @param[in] index                    - The scheduler profile index. range 0..7.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerProfileIndexSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_U32  index
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profile;

    profile = (CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT)index;

    /* call device specific API */
    rc = cpssDxChPortTxBindPortToSchedulerProfileSet(devNum, portNum, profile);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxBindPortToSchedulerProfileSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(index);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxSchedulerProfileIndexGet function
* @endinternal
*
* @brief   Get scheduler profile set that is binded to the port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number,
*                                      or the Queue Group number in case of Aggregate Mode
*                                      in ExMxPm XG device.
*
* @param[out] indexPtr                 - (pointer to) The scheduler profile index. range 0..7.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U32   portNum,
    OUT GT_U32  *indexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profile;

    /* call device specific API */
    rc = cpssDxChPortTxBindPortToSchedulerProfileGet(devNum, portNum, &profile);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxBindPortToSchedulerProfileGet FAILED, rc = [%d]", rc);
        return rc;
    }

    *indexPtr = (GT_U32) profile;

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(indexPtr);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/* some WS tests of xCat3 fail when succeed to set XG ports as SR_LR
  (they fail as CPSS_PORT_INTERFACE_MODE_XGMII_E !!!)
    test like prvTgfStgTableWriteWorkaround.

  but test : prvTgfFdbTableReadUnderWireSpeed is ok with succeed to set XG ports as SR_LR
*/
static GT_U32   xcat3AllowXGPortsSR_LR = 1;
GT_U32 prvTgfPortMaxSpeedForce_xcat3AllowXGPortsSR_LR(IN GT_U32   allow)
{
    GT_U32  temp = xcat3AllowXGPortsSR_LR;
    xcat3AllowXGPortsSR_LR = allow;

    return temp;
}

/**
* @internal prvTgfPortMaxSpeedForce function
* @endinternal
*
* @brief   Force or restore port speed, duplex and other configurations for Full
*         Wire Speed testing.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] force                    - GT_TRUE -  maximal settings and  link UP
*                                      GT_FALSE - disable force link UP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMaxSpeedForce
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     force
)
{
#ifdef CHX_FAMILY

    GT_STATUS   rc;
    CPSS_PORT_SPEED_ENT  forceSpeed = CPSS_PORT_SPEED_NA_E;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_INTERFACE_MODE_ENT forceIfMode=CPSS_PORT_INTERFACE_MODE_NA_E;
    GT_BOOL  configSerdes = GT_FALSE;
    GT_BOOL  configAutoneg = GT_FALSE;
    GT_BOOL  use_cpssDxChPortModeSpeedSet = (PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
                                            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ? GT_TRUE : GT_FALSE;

    if(force == GT_TRUE)
    {
        /* get interface mode */
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_XCAT_E:
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                if (portNum < 24)
                {
                    /* network ports */
                    if (ifMode == CPSS_PORT_INTERFACE_MODE_MII_E)
                    {
                        /* force speed to be common acceptable 100M,
                          do not configure SERDES */
                        forceSpeed = CPSS_PORT_SPEED_100_E;
                    }
                    else
                    {
                        /* force speed to be common acceptable 1G
                           do not configure SERDES */
                        forceSpeed = CPSS_PORT_SPEED_1000_E;
                    }

                    /* disable autoneg */
                    configAutoneg = GT_TRUE;
                }
                else
                {
                    /* stack/flex link ports */
                    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
                    {
                        /* configure XAUI mode 10G */
                        forceSpeed = CPSS_PORT_SPEED_10000_E;
                        forceIfMode = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) || (xcat3AllowXGPortsSR_LR == 0) ?
                            CPSS_PORT_INTERFACE_MODE_XGMII_E :
                            CPSS_PORT_INTERFACE_MODE_SR_LR_E;  /*xCat3*/
                        configSerdes = GT_TRUE;
                    }
                    else
                    {
                        /* force speed to be common acceptable 2.5G
                           configure SERDES */
                        forceSpeed = CPSS_PORT_SPEED_2500_E;
                        forceIfMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
                        configSerdes = GT_TRUE;
                        /* disable autoneg */
                        configAutoneg = GT_TRUE;
                    }
                }
                break;

            case CPSS_PP_FAMILY_DXCH_LION_E:
            case CPSS_PP_FAMILY_DXCH_LION2_E:
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    /* configure Local XAUI mode 10G */
                    forceSpeed = CPSS_PORT_SPEED_10000_E;
                    configSerdes = GT_TRUE;
                    forceIfMode = CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E;
                }

                break;

            case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            case CPSS_PP_FAMILY_DXCH_FALCON_E:
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    /* configure KR mode 10G */
                    forceSpeed = CPSS_PORT_SPEED_10000_E;
                    configSerdes = GT_TRUE;
                    forceIfMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                }
                break;
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    /* configure KR mode 10G */
                    forceSpeed = CPSS_PORT_SPEED_10000_E;
                    configSerdes = GT_TRUE;
                    forceIfMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                }
                else
                {
                    if (portNum < 48)
                    {
                        /* QSGMII ports */
                        /* force speed to be common acceptable 1G
                           do not configure SERDES */
                        forceSpeed = CPSS_PORT_SPEED_1000_E;
                        forceIfMode = CPSS_PORT_INTERFACE_MODE_QSGMII_E;

                        /* disable autoneg */
                        configAutoneg = GT_TRUE;
                    }
                }


                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            case CPSS_PP_FAMILY_DXCH_AC3X_E:
                if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
                {
                    forceSpeed = CPSS_PORT_SPEED_1000_E;
                    configAutoneg = GT_TRUE;
                }
                else if ((ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E) ||
                         (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E))
                {
                    forceSpeed = CPSS_PORT_SPEED_1000_E;
                    configAutoneg = GT_TRUE;
                }
                break;
            default: break;
        }

        if (configAutoneg == GT_TRUE)
        {
            /* set port speed */
            rc = cpssDxChPortSpeedSet(devNum, portNum, forceSpeed);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG4_MAC("FAIL : cpssDxChPortSpeedSet(%d,%d,%d) - rc[%d]\n",
                                 devNum, portNum, forceSpeed, rc);
                return rc;
            }

            /* set the port to not be 'auto negotiation' */
            rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortSpeedAutoNegEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }

            /* set the port to not be 'auto negotiation' */
            rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortDuplexAutoNegEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }

            /* set port to be full duplex */
            rc = cpssDxChPortDuplexModeSet(devNum, portNum, CPSS_PORT_FULL_DUPLEX_E);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortDuplexModeSet - port[%d]\n",
                                 portNum);
                return rc;
            }
        }
    }
    else
    {
        switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_XCAT_E:
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                if (portNum < 24)
                {
                    /* enable autoneg */
                    configAutoneg = GT_TRUE;
                }
                else
                {
                    /* stack/flex link ports */
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
                    {

                        /* force speed to be common acceptable 1G
                           configure SERDES */
                        forceSpeed = CPSS_PORT_SPEED_1000_E;
                        forceIfMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
                        configSerdes = GT_TRUE;

                        /* enable autoneg */
                        configAutoneg = GT_TRUE;
                    }
                }
                break;

            case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            case CPSS_PP_FAMILY_DXCH_FALCON_E:
                break;
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                if (portNum < 48)
                {
                    /* QSGMII ports */
                    /* enable autoneg */
                    configAutoneg = GT_TRUE;
                }

                break;
            default: break;
        }

        if (configAutoneg == GT_TRUE)
        {
            /* set the port to be 'auto negotiation' */
            rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortSpeedAutoNegEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }

            /* set the port to be 'auto negotiation' */
            rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortDuplexAutoNegEnableSet - port[%d]\n",
                                 portNum);
                return rc;
            }
        }
    }

    if (configSerdes == GT_TRUE)
    {
        if (use_cpssDxChPortModeSpeedSet == GT_FALSE)
        {
            /* set interface mode */
            rc = cpssDxChPortInterfaceModeSet(devNum, portNum, forceIfMode);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfPortMaxSpeedForce FAIL : cpssDxChPortInterfaceModeSet port[%d],forceIfMode[%d]\n",
                                 portNum, forceIfMode);
                return rc;
            }

            /* set speed */
            rc = cpssDxChPortSpeedSet(devNum, portNum, forceSpeed);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortSpeedSet port[%d]\n",
                                 portNum);
                return rc;
            }

            /* SERDES, XPCS, MAC configuration */
            rc = cpssDxChPortSerdesPowerStatusSet(devNum, portNum,
                                                  CPSS_PORT_DIRECTION_BOTH_E,
                                                  0xF, GT_TRUE);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL : cpssDxChPortSerdesPowerStatusSet port[%d]\n",
                                 portNum);
                return rc;
            }
        }
        else
        {
            CPSS_PORTS_BMP_STC portsBmp;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
            rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, forceIfMode,
                                          forceSpeed);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG3_MAC("prvTgfPortMaxSpeedForce FAIL : cpssDxChPortModeSpeedSet port[%d],forceIfMode[%d],forceSpeed[%d]\n",
                                 portNum, forceIfMode, forceSpeed);
                return rc;
            }
        }
    }

    if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        /* need to manage EGF filter for remote ports */
        rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,portNum,
            force ?
                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
        if(rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("FAIL : cpssDxChBrgEgrFltPortLinkEnableSet - port[%d]\n",
                             portNum);
            return rc;
        }
    }

    rc = tgfTrafficGeneratorPortForceLinkWa(devNum, portNum, force);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("FAIL : tgfTrafficGeneratorPortForceLinkWa - port[%d]\n",
                         portNum);
        return rc;
    }

    /* enable/disable force link pass */
    return cpssDxChPortForceLinkPassEnableSet(devNum,
                                              portNum,
                                              force);
#endif /* CHX_FAMILY */
#if !(defined CHX_FAMILY)
#ifdef EXMXPM_FAMILY
{   GT_STATUS   rc;

    /* For Puma3 devices use port speed/mode setting of appDemo.
       And just force link UP/Down */
    rc = cpssExMxPmPortForceLinkUpSet(devNum, portNum, force);
    return rc;
}
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(force);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortSpeedForce function
* @endinternal
*
* @brief   Force port speed, configure interface mode if need.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable for XCAT and XCAT2 only.
*
*/
GT_STATUS prvTgfPortSpeedForce
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    IN  CPSS_PORT_SPEED_ENT         speed
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    CPSS_PORT_INTERFACE_MODE_ENT forceIfMode=CPSS_PORT_INTERFACE_MODE_NA_E;

    switch (speed)
    {
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_XCAT_E:
                case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                    if (portNum < 24)
                    {
                        /* do not change interface mode */
                    }
                    else
                    {
                        /* change interface mode to be SGMII */
                        forceIfMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
                    }
                    break;
                default:
                    PRV_UTF_LOG1_MAC("prvTgfPortSpeedForce: device not supported port[%d]\n",
                                     portNum);
                   return GT_NOT_SUPPORTED;
            }
            break;
        default:
            PRV_UTF_LOG1_MAC("prvTgfPortSpeedForce: speed not supported port[%d]\n",
                             portNum);
           return GT_NOT_SUPPORTED;
    }

    if (forceIfMode != CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            /* set interface mode */
            rc = cpssDxChPortInterfaceModeSet(devNum, portNum, forceIfMode);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfPortSpeedForce FAIL : cpssDxChPortInterfaceModeSet port[%d],forceIfMode[%d]\n",
                                 portNum, forceIfMode);
                return rc;
            }

            /* set speed */
            rc = cpssDxChPortSpeedSet(devNum, portNum, speed);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL: cpssDxChPortSpeedSet port[%d]\n",
                                 portNum);
                return rc;
            }

            /* SERDES, XPCS, MAC configuration */
            rc = cpssDxChPortSerdesPowerStatusSet(devNum, portNum,
                                                  CPSS_PORT_DIRECTION_BOTH_E,
                                                  0xF, GT_TRUE);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL: cpssDxChPortSerdesPowerStatusSet port[%d]\n",
                                 portNum);
                return rc;
            }
        }
        else
        {
            CPSS_PORTS_BMP_STC portsBmp;

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
            rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, forceIfMode, speed);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG3_MAC("prvTgfPortSpeedForce FAIL : cpssDxChPortModeSpeedSet port[%d],forceIfMode[%d],forceSpeed[%d]\n",
                                 portNum, forceIfMode, speed);
                return rc;
            }

            rc = cpssDxChPortForceLinkPassEnableSet(devNum, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("FAIL: cpssDxChPortForceLinkPassEnableSet port[%d]\n", portNum);
                return rc;
            }
        }
    }

    /* set the port to not be 'auto negotiation' */
    rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, portNum, GT_FALSE);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("FAIL: cpssDxChPortSpeedAutoNegEnableSet - port[%d]\n",
                         portNum);
        return rc;
    }

    /* set port speed */
    rc = cpssDxChPortSpeedSet(devNum, portNum, speed);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("FAIL: cpssDxChPortSpeedSet - port[%d]\n",
                         portNum);
        return rc;
    }

    /* set the port to not be 'auto negotiation' */
    rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, portNum, GT_FALSE);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("FAIL: cpssDxChPortDuplexAutoNegEnableSet - port[%d]\n",
                         portNum);
        return rc;
    }

    /* set port to be full duplex */
    rc = cpssDxChPortDuplexModeSet(devNum, portNum, CPSS_PORT_FULL_DUPLEX_E);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("FAIL: cpssDxChPortDuplexModeSet - port[%d]\n",
                         portNum);
        return rc;
    }

    return GT_OK;

#endif /* CHX_FAMILY */
#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(speed);

    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortProfileMruSizeSet function
* @endinternal
*
* @brief   Set MRU size for MRU profile.
*         cpssDxChPortProfileMruSizeSet set MRU size for a specific MRU profile.
* @param[in] mruIndex                 - MRU profile index [0..7]
* @param[in] mruValue                 - MRU size in bytes [0..0x3FFF]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortProfileMruSizeSet
(
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortProfileMruSizeSet(devNum, mruIndex, mruValue);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortProfileMruSizeSet FAILED, rc = [%d]", rc);

        rc1 = rc;
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(mruIndex);
    TGF_PARAM_NOT_USED(mruValue);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortProfileMruSizeGet function
* @endinternal
*
* @brief   Get MRU size for MRU profile.
*         cpssDxChPortProfileMruSizeSet set MRU size for a specific MRU profile.
* @param[in] mruIndex                 - MRU profile index [0..7]
*                                      mruValue - MRU size in bytes [0..0x3FFF]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortProfileMruSizeGet
(
    IN GT_U32    mruIndex,
    OUT GT_U32  *mruValuePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortProfileMruSizeGet(devNum, mruIndex, mruValuePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortProfileMruSizeGet FAILED, rc = [%d]", rc);

        rc1 = rc;
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(mruIndex);
    TGF_PARAM_NOT_USED(mruValuePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMruExceptionCommandSet function
* @endinternal
*
* @brief   Set the command assigned to frames that exceed the default ePort MRU size.
*
* @param[in] command                  - the  valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCommandSet
(
    IN CPSS_PACKET_CMD_ENT      command
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPortMruExceptionCommandSet(devNum, command);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortMruExceptionCommandSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;
#else
    TGF_PARAM_NOT_USED(command);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMruExceptionCommandGet function
* @endinternal
*
* @brief   Get the command assigned to frames that exceed the default ePort MRU size.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCommandGet
(
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortMruExceptionCommandGet(devNum, commandPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortMruExceptionCommandGet FAILED, rc = [%d]", rc);

        rc1 = rc;
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(commandPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}

/**
* @internal prvTgfPortMruExceptionCPUCodeSet function
* @endinternal
*
* @brief   Set the CPU/drop code assigned to a frame which fails the MRU check.
*
* @param[in] cpuCode                  - the CPU/drop code
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCPUCodeSet
(
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChPortMruExceptionCpuCodeSet(devNum, cpuCode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortMruExceptionCpuCodeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;

#else
    TGF_PARAM_NOT_USED(cpuCode);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}

/**
* @internal prvTgfPortMruExceptionCPUCodeGet function
* @endinternal
*
* @brief   Get the CPU/drop code assigned to a frame which fails the MRU check.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCPUCodeGet
(
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortMruExceptionCpuCodeGet(devNum, cpuCodePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortMruExceptionCpuCodeGet FAILED, rc = [%d]", rc);

        rc1 = rc;
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(cpuCodePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}

/**
* @internal prvTgfPortMruProfileSet function
* @endinternal
*
* @brief   Set an MRU profile for port
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number, CPU port
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruProfileSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U32           profileId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortMruProfileSet(devNum, portNum, profileId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortMruProfileSet FAILED, rc = [%d]", rc);
    }
    return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(profileId);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortMruProfileGet function
* @endinternal
*
* @brief   Get an MRU profile for port
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number, CPU port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruProfileGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32         *profileIdPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortMruProfileGet(devNum, portNum, profileIdPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortMruProfileGet FAILED, rc = [%d]", rc);
    }
    return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(profileIdPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable Explicit Congestion Notification (ECN) extension in IPv4/IPv6 header
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortEcnMarkingEnableSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT        cmd,
    IN GT_BOOL                           enable
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                      rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChPortEcnMarkingEnableSet(devNum, cmd, enable);

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChPortEcnMarkingEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cmd);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfPortEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get status of Explicit Congestion Notification (ECN) extension from IPv4/IPv6 packet header
*
* @param[out] enablePtr                - GT_TRUE: ECN marking enable
*                                      GT_FALSE: ECN marking disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfPortEcnMarkingEnableGet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT        cmd,
    OUT GT_BOOL                          *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc = GT_OK;

    rc = cpssDxChPortEcnMarkingEnableGet(devNum, cmd, enablePtr);

     if (GT_OK != rc)
     {
         PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChPortEcnMarkingEnableGet FAILED, rc = [%d]", rc);
     }
     return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cmd);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
* @param[in] enable                   -  GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropUcEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    rc = cpssDxChPortTxTailDropUcEnableSet(devNum, enable);

     if (GT_OK != rc)
     {
         PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChPortTxTailDropUcEnableSet FAILED, rc = [%d]", rc);
     }
     return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropUcEnableGet function
* @endinternal
*
* @brief   Get enable/disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
*
* @param[out] enablePtr                -  pointer to tail drop status:
*                                      GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropUcEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    rc = cpssDxChPortTxTailDropUcEnableGet(devNum, enablePtr);

     if (GT_OK != rc)
     {
         PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChPortTxTailDropUcEnableGet FAILED, rc = [%d]", rc);
     }
     return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvTgfPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChPortEnableSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortEnableSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssExMxPmPortEnableSet(devNum, (GT_U8)portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmPortEnableSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortPhysicalMapAdd function
* @endinternal
*
* @brief   Add physical port map entry
*
* @param[in] devNum                   - physical device number
* @param[in] newInfoPtr               - pointer to new MAP entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortPhysicalMapAdd
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PORT_MAP_STC *newInfoPtr
)
{
    GT_STATUS       result;
    GT_BOOL         isValid;
    CPSS_DXCH_PORT_MAP_STC  *portMapArray;
    GT_U32          index;
    GT_U32          phyPortsMaxNum;
    GT_PHYSICAL_PORT_NUM currPortMap;

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* the GM does not supports port MAPs */
        return GT_OK;
    }

    /* get maximal physical port number */
    result = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, &phyPortsMaxNum);
    if (result != GT_OK)
    {
        return result;
    }

    portMapArray = (CPSS_DXCH_PORT_MAP_STC  *)cpssOsMalloc(/*CPSS_MAX_PORTS_NUM_CNS*/phyPortsMaxNum * sizeof(CPSS_DXCH_PORT_MAP_STC));
    if (!portMapArray)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    currPortMap = 0;
    index = 0;

    while((currPortMap < phyPortsMaxNum) && (index < phyPortsMaxNum))
    {
        /* check that port is valid */
        result = cpssDxChPortPhysicalPortMapIsValidGet(devNum, currPortMap, &isValid);
        if((result != GT_OK) || (isValid != GT_TRUE))
        {
            currPortMap++;
            continue;
        }

        /* store map */
        result = cpssDxChPortPhysicalPortMapGet(devNum, currPortMap, 1, &portMapArray[index]);
        if(result != GT_OK)
        {
            cpssOsFree(portMapArray);
            return result;
        }

        currPortMap++;
        index++;
    }

    if(index >= phyPortsMaxNum)
    {
        /* there is no place for new entry */
        cpssOsFree(portMapArray);
        return GT_FULL;
    }

    /* add new port as the 'last entry' */
    portMapArray[index] = *newInfoPtr;
    index ++;

    result = cpssDxChPortPhysicalPortMapSet(devNum, index, portMapArray);
    cpssOsFree(portMapArray);
    return result;
}

/**
* @internal prvTgfPortPhysicalMapDelete function
* @endinternal
*
* @brief   Delete physical port map entry
*
* @param[in] devNum                   - physical device number
* @param[in] portNumToDelete          - physical port number to delete from map
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortPhysicalMapDelete
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNumToDelete
)
{
    GT_STATUS       result;
    GT_BOOL         isValid;
    CPSS_DXCH_PORT_MAP_STC  *portMapArray;
    GT_U32          index;
    GT_U32          phyPortsMaxNum;
    GT_PHYSICAL_PORT_NUM currPortMap;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* test pass without it */
        return GT_OK;
    }

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* the GM does not supports port MAPs */
        return GT_OK;
    }

    /* get maximal physical port number */
    result = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, &phyPortsMaxNum);
    if (result != GT_OK)
    {
        return result;
    }

    portMapArray = (CPSS_DXCH_PORT_MAP_STC  *)cpssOsMalloc(/*CPSS_MAX_PORTS_NUM_CNS*/phyPortsMaxNum * sizeof(CPSS_DXCH_PORT_MAP_STC));
    if (!portMapArray)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    currPortMap = 0;
    index = 0;

    while((currPortMap < phyPortsMaxNum) && (index < phyPortsMaxNum))
    {
        if (portNumToDelete == currPortMap)
        {
            /* skip port so it will not in portMapArray */
            currPortMap++;
            continue;
        }

        /* check that port is valid */
        result = cpssDxChPortPhysicalPortMapIsValidGet(devNum, currPortMap, &isValid);
        if((result != GT_OK) || (isValid != GT_TRUE))
        {
            currPortMap++;
            continue;
        }

        /* store map */
        result = cpssDxChPortPhysicalPortMapGet(devNum, currPortMap, 1, &portMapArray[index]);
        if(result != GT_OK)
        {
            cpssOsFree(portMapArray);
            return result;
        }

        currPortMap++;
        index++;
    }

    if(index >= phyPortsMaxNum)
    {
        /* there is bug in algorithm */
        cpssOsFree(portMapArray);
        return GT_FAIL;
    }

    result = cpssDxChPortPhysicalPortMapSet(devNum, index, portMapArray);
    if( PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) )
    {
        /* the test code is obsolete, it passes without mapping as well in case of ALDRIN2 */
        /* TBD: to update the test scenario */
        result = GT_OK;
    }
    cpssOsFree(portMapArray);
    return result;
}

/**
* @internal prvTgfPortRemotePhysicalMapAdd function
* @endinternal
*
* @brief   Add physical port map for remote port
*
* @param[in] devNum                   - physical device number
* @param[in] localPortNumForRemote    - local physical port number for remote one
* @param[in] cscdPortNum              - cascade physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_ALREADY_EXIST         - on localPortNumForRemote already in map
*/
GT_STATUS prvTgfPortRemotePhysicalMapAdd
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     localPortNumForRemote,
    IN  GT_PHYSICAL_PORT_NUM     cscdPortNum
)
{
    GT_STATUS               result;
    CPSS_DXCH_PORT_MAP_STC  newInfo;
    GT_BOOL                 oldIsValid;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* test pass without it */
        return GT_OK;
    }

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* the GM does not supports port MAPs */
        return GT_OK;
    }

    /* get information for cascade port */
    result = cpssDxChPortPhysicalPortMapGet(devNum, cscdPortNum, 1, &newInfo);
    if (result != GT_OK)
    {
        return result;
    }

    /* set new physical port number for remote port */
    if (newInfo.tmEnable)
    {
        /* don't use TM for remote ports because
           TM was not configured to support remote ports */
        newInfo.tmEnable = GT_FALSE;
        newInfo.txqPortNumber = cscdPortNum;
    }
    newInfo.physicalPortNumber = localPortNumForRemote;
    newInfo.mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E;

    /* check that port is valid */
    result = cpssDxChPortPhysicalPortMapIsValidGet(devNum, localPortNumForRemote, &oldIsValid);
    if((result == GT_OK) && (oldIsValid == GT_TRUE))
    {
       PRV_UTF_LOG1_MAC("[TGF]: Physical port [%d] already exist in the map", localPortNumForRemote);
       return GT_ALREADY_EXIST;
    }

    result = prvTgfPortPhysicalMapAdd(devNum, &newInfo);


    if( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) )
    {
        /* the test code is obsolete, it passes without mapping as well in case of BC3 */
        /* TBD: to update the test scenario */
        result = GT_OK;
    }

    return result;
}

/**
* @internal prvTgfPortTxDbaEnableSet function
* @endinternal
*
* @brief Enabling DBA feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum - physical device number.
* @param[in] enable - GT_TRUE  - enable DBA feature
*                     GT_FALSE - no DBA feature support
*/
GT_STATUS prvTgfPortTxDbaEnableSet
(
    IN    GT_U8         devNum,
    IN    GT_BOOL       enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxDbaEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxDbaEnableGet function
* @endinternal
*
* @brief Indicates if DBA feature is enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum - physical device number.
*
* @param[out] enable - (pointer to) GT_TRUE  - enable DBA feature
*                                   GT_FALSE - no DBA feature support
*/
GT_STATUS prvTgfPortTxDbaEnableGet
(
    IN    GT_U8         devNum,
    IN    GT_BOOL       *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxDbaEnableGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxDbaAvailBuffSet function
* @endinternal
*
* @brief Set maximal available buffers for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range number of buffers
*
* @param[in] devNum - physical device number.
* @param[in] maxBuff - Maximal available buffers for allocation.
*/
GT_STATUS prvTgfPortTxDbaAvailBuffSet
(
    IN    GT_U8         devNum,
    IN    GT_BOOL       maxBuff
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxDbaAvailBuffSet(devNum, maxBuff);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxBuff);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortTxDbaAvailBuffGet function
* @endinternal
*
* @brief Get maximal available buffers for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message @retval
*
* @param[in] devNum - physical device number.
*
* @param[out] maxBuffPtr - (pointer to) Maximal available buffers for allocation.
*/
GT_STATUS prvTgfPortTxDbaAvailBuffGet
(
    IN    GT_U8         devNum,
    IN    GT_U32        *maxBuffPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxDbaAvailBuffGet(devNum, maxBuffPtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxBuffPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
 * @internal prvTgfPortTxBufferStatisticsEnableSet function
 * @endinternal
 *
 * @brief  Enable/disable port buffer statistics.
 *
 * @param[in] devNum                   - device number
 * @param[in] enPortStat               - enable port statistics
 *                                       GT_FALSE - disable port buffer statistics.
 *                                       GT_TRUE  - enable port buffer statistics.
 * @param[in] enQueueStat              - enable port queue statistics
 *                                       GT_FALSE - disable port queue buffer statistics.
 *                                       GT_TRUE  - enable port queue buffer statistics.
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxBufferStatisticsEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enPortStat,
    IN  GT_BOOL              enQueueStat
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxBufferStatisticsEnableSet(devNum, enPortStat, enQueueStat);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enPortStat);
    TGF_PARAM_NOT_USED(enQueueStat);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
 * @internal prvTgfPortTxBufferStatisticsEnableGet function
 * @endinternal
 *
 * @brief  Get enable status of port buffer statistics.
 *
 * @param[in]  devNum                   - device number
 *
 * @param[out] enPortStatPtr            - (pointer to) enabled status of
 *                                        port buffer statistics
 * @param[out] enQueueStatPtr           - (pointer to) enabled status of
 *                                        queue buffer statistics
 *
 * @retval GT_OK                        - on success
 * @retval GT_BAD_PTR                   - on NULL pointer
 * @retval GT_BAD_PARAM                 - wrong value in any of the parameters
 * @retval GT_HW_ERROR                  - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
 */
GT_STATUS prvTgfPortTxBufferStatisticsEnableGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *enPortStatPtr,
    OUT GT_BOOL              *enQueueStatPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxBufferStatisticsEnableGet(devNum, enPortStatPtr, enQueueStatPtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enPortStatPtr);
    TGF_PARAM_NOT_USED(enQueueStatPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
 * @internal prvTgfPortTxMaxBufferFillLevelSet function
 * @endinternal
 *
 * @brief  Set max port buffer fill level per port.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] maxBuffFillLvl           - max buffer fill level (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxMaxBufferFillLevelSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               maxBuffFillLvl
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxMaxBufferFillLevelSet(devNum, portNum, maxBuffFillLvl);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(maxBuffFillLvl);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
 * @internal prvTgfPortTxMaxBufferFillLevelGet function
 * @endinternal
 *
 * @brief  Get max port buffer fill level per port.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 *
 * @param[out] maxBuffFillLvlPtr       - (pointer to)max buffer fill level
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxMaxBufferFillLevelGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *maxBuffFillLvlPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxMaxBufferFillLevelGet(devNum, portNum, maxBuffFillLvlPtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(maxBuffFillLvlPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
 * @internal prvTgfPortTxQueueMaxBufferFillLevelSet function
 * @endinternal
 *
 * @brief  Set max port queue buffer fill level per port.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] tc                       - traffic class queue (0..7)
 * @param[in] maxBuffFillLvl           - max buffer fill level (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxQueueMaxBufferFillLevelSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN  GT_U32               maxBuffFillLvl
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxQueueMaxBufferFillLevelSet(devNum, portNum, tc, maxBuffFillLvl);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tc);
    TGF_PARAM_NOT_USED(maxBuffFillLvl);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
 * @internal prvTgfPortTxQueueMaxBufferFillLevelGet function
 * @endinternal
 *
 * @brief  Get max port queue buffer fill level per port.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] tc                       - traffic class queue (0..7)
 *
 * @param[out] maxBuffFillLvlPtr       - (pointer to)max buffer fill level
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxQueueMaxBufferFillLevelGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    OUT GT_U32               *maxBuffFillLvlPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChPortTxQueueMaxBufferFillLevelGet(devNum, portNum, tc, maxBuffFillLvlPtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tc);
    TGF_PARAM_NOT_USED(maxBuffFillLvlPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*          on port's serdeses power down and power up sequence.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC portsBmp;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

    if ((ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
        || (speed >= CPSS_PORT_SPEED_NA_E))
    {
        /* power off only */
        ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        speed  = CPSS_PORT_SPEED_NA_E;
    }
    rc = cpssDxChPortModeSpeedSet(
        devNum, &portsBmp, GT_FALSE/*powerUp*/, ifMode, speed);
    if (GT_OK != rc) return rc;

    if ((ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
        || (speed >= CPSS_PORT_SPEED_NA_E))
    {
        return GT_OK; /* power off */
    }
    return cpssDxChPortModeSpeedSet(
        devNum, &portsBmp, GT_TRUE/*powerUp*/, ifMode, speed);

#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ifMode);
    TGF_PARAM_NOT_USED(speed);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*          on port's serdeses power down and power up sequence.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[out] ifModePtr               - (pointer to) interface mode
* @param[out] speedPtr                - (pointer to) port data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR                  - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfPortModeSpeedGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT    *ifModePtr,
    OUT CPSS_PORT_SPEED_ENT             *speedPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChPortInterfaceModeGet(
        devNum, portNum, ifModePtr);
    if (GT_OK != rc) return rc;

    return cpssDxChPortSpeedGet(
        devNum, portNum, speedPtr);

#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ifModePtr);
    TGF_PARAM_NOT_USED(speedPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

