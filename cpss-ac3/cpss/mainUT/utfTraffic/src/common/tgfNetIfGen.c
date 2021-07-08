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
* @file tgfNetIfGen.c
*
* @brief Generic API of Network Interface.
*
* @version   4
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
#include <common/tgfNetIfGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
    #include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIfDbg.h>
#endif /* CHX_FAMILY */


/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChNetTxParams function
* @endinternal
*
* @brief   Convert generic into device specific network Tx packet parameters
*         (including DSF info).
* @param[in] netTxPacketPtr           - (pointer to) generic network Tx packet parameters
*
* @param[out] dxChNetTxPacketPtr       - (pointer to) DxCh network Tx packet parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChNetTxParams
(
    IN  PRV_TGF_NET_TX_PARAMS_STC   *netTxPacketPtr,
    OUT CPSS_DXCH_NET_TX_PARAMS_STC *dxChNetTxPacketPtr
)
{
    TGF_DSA_DSA_FROM_CPU_STC        *dsaFromCpuInfoPtr;
    CPSS_DXCH_NET_DSA_FROM_CPU_STC  *dxChDsaFromCpuInfoPtr;

    TGF_DSA_DSA_FORWARD_STC         *dsaForwardInfoPtr;
    CPSS_DXCH_NET_DSA_FORWARD_STC   *dxChDsaForwardInfoPtr;

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr, packetIsTagged);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr, cookie);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr, sdmaInfo.recalcCrc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr, sdmaInfo.txQueue);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr, sdmaInfo.evReqHndl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr, sdmaInfo.invokeTxBufferQueueEvent);

    /* convert DSA Tag type into device specific format */
    switch (netTxPacketPtr->dsaParam.dsaType)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.commonParams.dsaTagType,
                                TGF_DSA_1_WORD_TYPE_E,
                                CPSS_DXCH_NET_DSA_1_WORD_TYPE_ENT);

        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.commonParams.dsaTagType,
                                TGF_DSA_2_WORD_TYPE_E,
                                CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT);

        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.commonParams.dsaTagType,
                                TGF_DSA_3_WORD_TYPE_E,
                                CPSS_DXCH_NET_DSA_3_WORD_TYPE_ENT);

        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.commonParams.dsaTagType,
                                TGF_DSA_4_WORD_TYPE_E,
                                CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT);

        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr,
                                   dsaParam.commonParams.vpt);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr,
                                   dsaParam.commonParams.cfiBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr,
                                   dsaParam.commonParams.vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr,
                                   dsaParam.commonParams.dropOnSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChNetTxPacketPtr, netTxPacketPtr,
                                   dsaParam.commonParams.packetIsLooped);

    /* convert DSA Tag command into device specific format */
    switch(netTxPacketPtr->dsaParam.dsaCommand)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.dsaType,
                                TGF_DSA_CMD_TO_CPU_E,
                                CPSS_DXCH_NET_DSA_CMD_TO_CPU_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.dsaType,
                                TGF_DSA_CMD_FROM_CPU_E,
                                CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.dsaType,
                                TGF_DSA_CMD_TO_ANALYZER_E,
                                CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E);

        PRV_TGF_SWITCH_CASE_MAC(dxChNetTxPacketPtr->dsaParam.dsaType,
                                TGF_DSA_CMD_FORWARD_E,
                                CPSS_DXCH_NET_DSA_CMD_FORWARD_E);

        default:
            return GT_BAD_PARAM;
    }

    switch(netTxPacketPtr->dsaParam.dsaCommand)
    {
        case TGF_DSA_CMD_FROM_CPU_E:
            dsaFromCpuInfoPtr = &netTxPacketPtr->dsaParam.dsaInfo.fromCpu;
            dxChDsaFromCpuInfoPtr = &dxChNetTxPacketPtr->dsaParam.dsaInfo.fromCpu;

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           dstInterface);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           tc);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           dp);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           egrFilterEn);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           cascadeControl);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           egrFilterRegistered);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           srcId);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           srcHwDev);

            if( CPSS_INTERFACE_PORT_E == dsaFromCpuInfoPtr->dstInterface.type )
            {
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                               dsaFromCpuInfoPtr,
                                               extDestInfo.devPort.dstIsTagged);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                               dsaFromCpuInfoPtr,
                                               extDestInfo.devPort.mailBoxToNeighborCPU);
            }
            else if( CPSS_INTERFACE_VIDX_E == dsaFromCpuInfoPtr->dstInterface.type )
            {
                /* VIDX dependand copying */
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                               dsaFromCpuInfoPtr,
                                               extDestInfo.multiDest.excludeInterface);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                               dsaFromCpuInfoPtr,
                                               extDestInfo.multiDest.excludedInterface);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                               dsaFromCpuInfoPtr,
                                               extDestInfo.multiDest.mirrorToAllCPUs);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                               dsaFromCpuInfoPtr,
                                               extDestInfo.multiDest.excludedIsPhyPort);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                               dsaFromCpuInfoPtr,
                                               extDestInfo.multiDest.srcIsTagged);
            }
            else
            {
                return GT_NOT_IMPLEMENTED;
            }

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           isTrgPhyPortValid);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           dstEport);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaFromCpuInfoPtr,
                                           dsaFromCpuInfoPtr,
                                           tag0TpidIndex);
            break;

        case TGF_DSA_CMD_FORWARD_E:
            dsaForwardInfoPtr = &netTxPacketPtr->dsaParam.dsaInfo.forward;
            dxChDsaForwardInfoPtr = &dxChNetTxPacketPtr->dsaParam.dsaInfo.forward;

            /* convert Tag command into device specific format */
            switch(dsaForwardInfoPtr->srcIsTagged)
            {
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E);
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E);
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E);
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E);
                PRV_TGF_SWITCH_CASE_MAC(dxChDsaForwardInfoPtr->srcIsTagged,
                                        PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E,
                                        CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
                default:
                    return GT_BAD_PARAM;
            }

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           srcHwDev);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           srcIsTrunk);

            if( GT_TRUE == dsaForwardInfoPtr->srcIsTrunk )
            {
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                               dsaForwardInfoPtr,
                                               source.trunkId);
            }
            else /* GT_FALSE == dsaForwardInfoPtr->srcIsTrunk */
            {
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                               dsaForwardInfoPtr,
                                               source.portNum);
            }

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           srcId);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           egrFilterRegistered);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           wasRouted);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           qosProfileIndex);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           dstInterface);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           isTrgPhyPortValid);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           dstEport);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           tag0TpidIndex);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           origSrcPhyIsTrunk);

            if( GT_TRUE == dsaForwardInfoPtr->origSrcPhyIsTrunk )
            {
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                               dsaForwardInfoPtr,
                                               origSrcPhy.trunkId);
            }
            else /* GT_FALSE == dsaForwardInfoPtr->origSrcPhyIsTrunk */
            {
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                               dsaForwardInfoPtr,
                                               origSrcPhy.portNum);
            }

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           phySrcMcFilterEnable);

            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           hash);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDsaForwardInfoPtr,
                                           dsaForwardInfoPtr,
                                           skipFdbSaLookup);

            break;

        case TGF_DSA_CMD_TO_ANALYZER_E:
        case TGF_DSA_CMD_TO_CPU_E:
            return GT_NOT_IMPLEMENTED;

        default:
            return GT_BAD_PARAM;

    }

    return GT_OK;
}
#endif /* CHX_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfNetIfSdmaTxGeneratorEnable function
* @endinternal
*
* @brief   This function enables selected Tx SDMA Generator.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
* @param[in] burstEnable              - GT_TRUE for packets burst generation,
*                                      - GT_FALSE for continuous packets generation.
* @param[in] burstPacketsNumber       - Number of packets in burst.
*                                      Relevant only if burstEnable == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..512M-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or burst
*                                       size.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If queue is empty, configuration is done beside the enabling which will
*       be implicitly performed after the first packet will be added to the
*       queue.
*       On each HW queue enabling the Tx SDMA Current Descriptor Pointer
*       register will set to point to the first descriptor in chain list.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorEnable
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    IN  GT_BOOL burstEnable,
    IN  GT_U32  burstPacketsNumber
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChNetIfSdmaTxGeneratorEnable(devNum,
                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                txQueue,
                                                burstEnable,
                                                burstPacketsNumber);
    }
    else
    {
        rc = cpssDxChNetIfSdmaTxGeneratorEnable(devNum,
                                                currPortGroupsBmp,
                                                txQueue,
                                                burstEnable,
                                                burstPacketsNumber);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorEnable FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(burstEnable);
    TGF_PARAM_NOT_USED(burstPacketsNumber);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}


/**
* @internal prvTgfNetIfSdmaTxGeneratorDisable function
* @endinternal
*
* @brief   This function disables selected Tx SDMA Generator.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorDisable
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChNetIfSdmaTxGeneratorDisable(devNum,
                                                 CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                 txQueue);
    }
    else
    {
        rc = cpssDxChNetIfSdmaTxGeneratorDisable(devNum,
                                                 currPortGroupsBmp,
                                                 txQueue);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorDisable FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}


/**
* @internal prvTgfNetIfSdmaTxGeneratorRateSet function
* @endinternal
*
* @brief   This function sets Tx SDMA Generator transmission packet rate.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue.
* @param[in] rateMode                 - transmission rate mode.
* @param[in] rateValue                - packets per second for rateMode ==
*                                      PRV_TGF_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValue.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad rate
*                                       mode or value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_BAD_VALUE             - rate cannot be configured for Tx SDMA.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Rate can be set while Tx SDMA is enabled.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorRateSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   txQueue,
    IN  PRV_TGF_NET_TX_GENERATOR_RATE_MODE_ENT  rateMode,
    IN  GT_U64                                  rateValue,
    OUT GT_U64                                  *actualRateValuePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    dxChRateMode;

    /* convert rateMode into device specific format */
    switch (rateMode)
    {
        case PRV_TGF_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E:
             dxChRateMode =
                    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E;
            break;

        case PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E:
             dxChRateMode =
                    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E;
            break;

        case PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E:
             dxChRateMode = CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChNetIfSdmaTxGeneratorRateSet(devNum,
                                                 CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                 txQueue,
                                                 dxChRateMode,
                                                 rateValue,
                                                 actualRateValuePtr);
    }
    else
    {
        rc = cpssDxChNetIfSdmaTxGeneratorRateSet(devNum,
                                                 currPortGroupsBmp,
                                                 txQueue,
                                                 dxChRateMode,
                                                 rateValue,
                                                 actualRateValuePtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorRateSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(rateMode);
    TGF_PARAM_NOT_USED(rateValue);
    TGF_PARAM_NOT_USED(actualRateValuePtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxGeneratorBurstStatusGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator burst counter status.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] burstStatusPtr           - (pointer to) Burst counter status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorBurstStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U8                                       txQueue,
    OUT PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_ENT   *burstStatusPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT dxChBurstStatus;

    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(devNum,
                                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                        txQueue,
                                                        &dxChBurstStatus);
    }
    else
    {
        rc = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(devNum,
                                                        currPortGroupsBmp,
                                                        txQueue,
                                                        &dxChBurstStatus);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorBurstStatusGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* convert burst status into generic format */
    switch (dxChBurstStatus)
    {
        case CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_IDLE_E:
             *burstStatusPtr = PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_IDLE_E;
            break;

        case CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_RUN_E:
             *burstStatusPtr = PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_RUN_E;
            break;

        case CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_LIMIT_E:
             *burstStatusPtr = PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_LIMIT_E;
            break;

        case CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_TERMINATE_E:
             *burstStatusPtr = PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_TERMINATE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(burstStatusPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxGeneratorPacketRemove function
* @endinternal
*
* @brief   This function removes packet from Tx SDMA working as a Packet Generator.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to remove the packet from.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or packet
*                                       is not transmitted on this Tx SDMA.
* @retval GT_BAD_VALUE             - removal of packet will violate the required
*                                       rate configuration.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Packet can be removed while Tx SDMA is enabled.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorPacketRemove
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    IN  GT_U32  packetId
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum,
                                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                      txQueue,
                                                      packetId);
    }
    else
    {
        rc = cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum,
                                                      currPortGroupsBmp,
                                                      txQueue,
                                                      packetId);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorPacketRemove FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(packetId);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxGeneratorPacketAdd function
* @endinternal
*
* @brief   This function adds a new packet to Tx SDMA working as Packet Generator.
*         This packet will be transmitted by the selected Tx SDMA with previous
*         packets already transmitted by this Packet generator.
* @param[in] devNum                   - Device number.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @param[out] packetIdPtr              - (pointer to) packet identification number, used by
*                                      other Packet Generator functions which require access
*                                      to this packet.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_RESOURCE           - no available buffer or descriptor.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_VALUE             - addition of packet will violate the required
*                                       rate configuration.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be added while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorPacketAdd
(
    IN  GT_U8                       devNum,
    IN  PRV_TGF_NET_TX_PARAMS_STC   *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength,
    OUT GT_U32                      *packetIdPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_NET_TX_PARAMS_STC dxChPacketParams;

    rc = prvTgfConvertGenericToDxChNetTxParams(packetParamsPtr,
                                               &dxChPacketParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChNetTxParams FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum,
                                                   CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                   &dxChPacketParams,
                                                   packetDataPtr,
                                                   packetDataLength,
                                                   packetIdPtr);
    }
    else
    {
        rc = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum,
                                                   currPortGroupsBmp,
                                                   &dxChPacketParams,
                                                   packetDataPtr,
                                                   packetDataLength,
                                                   packetIdPtr);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorPacketAdd FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(packetParamsPtr);
    TGF_PARAM_NOT_USED(packetDataPtr);
    TGF_PARAM_NOT_USED(packetDataLength);
    TGF_PARAM_NOT_USED(packetIdPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxGeneratorPacketUpdate function
* @endinternal
*
* @brief   This function updates already transmitted packet content and parameters.
*
* @param[in] devNum                   - Device number.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, packet is
*                                       not transmitted on this Tx SDMA, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be updated while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorPacketUpdate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      packetId,
    IN  PRV_TGF_NET_TX_PARAMS_STC   *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_NET_TX_PARAMS_STC dxChPacketParams;

    rc = prvTgfConvertGenericToDxChNetTxParams(packetParamsPtr,
                                               &dxChPacketParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChNetTxParams FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API */
    if(usePortGroupsBmp == GT_FALSE)
    {
        rc = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(devNum,
                                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                      packetId,
                                                      &dxChPacketParams,
                                                      packetDataPtr,
                                                      packetDataLength);
    }
    else
    {
        rc = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(devNum,
                                                      currPortGroupsBmp,
                                                      packetId,
                                                      &dxChPacketParams,
                                                      packetDataPtr,
                                                      packetDataLength);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorPacketUpdate FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(packetId);
    TGF_PARAM_NOT_USED(packetParamsPtr);
    TGF_PARAM_NOT_USED(packetDataPtr);
    TGF_PARAM_NOT_USED(packetDataLength);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
* @param[in] devNum                   - device number
*                                      queue     - traffic class queue
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfNetIfSdmaTxQueueEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    OUT GT_BOOL *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    rc = cpssDxChNetIfSdmaTxQueueEnableGet(devNum,
                                           txQueue,
                                           enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxQueueEnableGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Set status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA related
* @param[in] devNum                   - device number
*                                      queue     - traffic class queue
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
*/
GT_STATUS prvTgfNetIfSdmaTxQueueEnable
(
    IN  GT_U8   devNum,
    IN  GT_U8   txQueue,
    OUT GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    rc = cpssDxChNetIfSdmaTxQueueEnable(devNum, txQueue, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxQueueEnable FAILED, "
            "rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxGeneratorMemCheck function
* @endinternal
*
* @brief   This function verify that descriptors and buffers memory is arranged
*         to support optimal performance.
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] checkResultPtr           - (pointer to) memory check result status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note memory check can be performed while queue is enabled.
*
*/
GT_STATUS prvTgfNetIfSdmaTxGeneratorMemCheck
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   txQueue,
    OUT PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_ENT *checkResultPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_TX_SDMA_GENERATOR_MEM_CHECK_ENT dxChcheckResult;

    rc = cpssDxChNetIfSdmaTxGeneratorMemCheck(devNum,
                                              txQueue,
                                              &dxChcheckResult);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfSdmaTxGeneratorMemCheck FAILED, rc = [%d]", rc);
        return rc;
    }

    /* convert memory check result status into generic format */
    switch (dxChcheckResult)
    {
        case CPSS_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E:
             *checkResultPtr = PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E;
            break;

        case CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E:
             *checkResultPtr = PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E;
            break;

        case CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_NEXT_DESC_E:
             *checkResultPtr = PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_BAD_NEXT_DESC_E;
            break;

        case CPSS_TX_SDMA_GENERATOR_MEM_CHECK_EMPTY_QUEUE_E:
             *checkResultPtr = PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_EMPTY_QUEUE_E;
            break;

        case CPSS_TX_SDMA_GENERATOR_MEM_CHECK_DESC_EXCEED_E:
             *checkResultPtr = PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_DESC_EXCEED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(checkResultPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertTgfToCpssNetCpuCodeTableParams function
* @endinternal
*
* @brief   Convert generic into device specific network interface CPU code table parameters.
*
* @param[in] tgfNetCpuCodeTableParamsPtr - (pointer to) TGF CPU code table parameters.
*
* @param[out] cpssNetCpuCodeTableParamsPtr - (pointer to) Cpss CPU code table parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertTgfToCpssNetCpuCodeTableParams
(
    IN  PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC  *tgfNetCpuCodeTableParamsPtr,
    OUT CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *cpssNetCpuCodeTableParamsPtr
)
{
    cpssOsMemSet(cpssNetCpuCodeTableParamsPtr, 0,
                 sizeof(CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC));

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, tc);
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, dp);
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, truncate);
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, cpuRateLimitMode);
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, cpuCodeRateLimiterIndex);
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, cpuCodeStatRateLimitIndex);
    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, designatedDevNumIndex);

    return GT_OK;
}

/**
* @internal prvTgfConvertCpssToTgfNetCpuCodeTableParams function
* @endinternal
*
* @brief   Convert device specific network interface CPU code table parameters into generic.
*
* @param[in] cpssNetCpuCodeTableParamsPtr - (pointer to) Cpss CPU code table parameters.
*
* @param[out] tgfNetCpuCodeTableParamsPtr - (pointer to) TGF CPU code table parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertCpssToTgfNetCpuCodeTableParams
(
    IN CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *cpssNetCpuCodeTableParamsPtr,
    OUT PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC  *tgfNetCpuCodeTableParamsPtr
)
{
    cpssOsMemSet(tgfNetCpuCodeTableParamsPtr, 0,
                 sizeof(PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC));

    /* convert entry info into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, tc);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, dp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, truncate);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, cpuRateLimitMode);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, cpuCodeRateLimiterIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, cpuCodeStatRateLimitIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(cpssNetCpuCodeTableParamsPtr,
                                   tgfNetCpuCodeTableParamsPtr, designatedDevNumIndex);

    return GT_OK;
}
#endif

/**
* @internal prvTgfNetIfCpuCodeTableSet function
* @endinternal
*
* @brief   Function to set the "CPU code table" entry, for a specific CPU code.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU code (key parameter to access the "CPU code table").
*                                      use CPSS_NET_ALL_CPU_OPCODES_E as a wildcard to be used when
*                                      need to set all the table entries with the same info.
* @param[in] entryInfoPtr             - (pointer to) The entry information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode or bad one of
*                                       bad one entryInfoPtr parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeTableSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
)
{

#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;

    rc = prvTgfConvertTgfToCpssNetCpuCodeTableParams(entryInfoPtr, &entryInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertTgfToCpssNetCpuCodeTableParams FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChNetIfCpuCodeTableSet(devNum, cpuCode, &entryInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfCpuCodeTableSet FAILED, rc = [%d]", rc);
    }

    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCode);
    TGF_PARAM_NOT_USED(entryInfoPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfNetIfCpuCodeTableGet function
* @endinternal
*
* @brief   Function to get the "CPU code table" entry, for a specific CPU code.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU code (key parameter to access the "CPU code table").
*                                      use CPSS_NET_ALL_CPU_OPCODES_E as a wildcard to be used when
*                                      need to set all the table entries with the same info.
*
* @param[out] entryInfoPtr             - (pointer to) The entry information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeTableGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;

    rc = cpssDxChNetIfCpuCodeTableGet(devNum, cpuCode, &entryInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfCpuCodeTableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = prvTgfConvertCpssToTgfNetCpuCodeTableParams(&entryInfo, entryInfoPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertCpssToTgfNetCpuCodeTableParams FAILED, rc = [%d]", rc);
    }

    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCode);
    TGF_PARAM_NOT_USED(entryInfoPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfCpuCodeDesignatedDeviceTableSet function
* @endinternal
*
* @brief   Function to set the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
* @param[in] devNum                   - device number
* @param[in] index                    -  into the designated device table (APPLICABLE RANGES: 1..7)
* @param[in] designatedHwDevNum       - The designated HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index or bad
*                                       designatedHwDevNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeDesignatedDeviceTableSet
(
    IN GT_U8            devNum,
    IN GT_U32           index,
    IN GT_HW_DEV_NUM    designatedHwDevNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(devNum, index,
                                                      designatedHwDevNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfCpuCodeDesignatedDeviceTableSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(designatedHwDevNum);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfNetIfCpuCodeDesignatedDeviceTableGet function
* @endinternal
*
* @brief   Function to get the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
* @param[in] devNum                   - device number
* @param[in] index                    -  into the designated device table (APPLICABLE RANGES: 1..7)
*
* @param[out] designatedHwDevNumPtr    - (pointer to)The designated HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNetIfCpuCodeDesignatedDeviceTableGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_HW_DEV_NUM   *designatedHwDevNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(devNum, index,
                                                      designatedHwDevNumPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChNetIfCpuCodeDesignatedDeviceTableGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(designatedHwDevNumPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfNetIfSdmaTxPacketSend function
* @endinternal
*
* @brief   This function verify that descriptors and buffers memory is arranged
*         to support optimal performance.
* @param[in] devNum                   - Device number
* @param[in] pcktParamsPtr            - The internal packet params to be set into packet
*                                      descriptors
* @param[in] buffList[]               - A packet data buffers list
* @param[in] buffLenList[]            - A list of buffers len in buffList
* @param[in] numOfBufs                - Length of bufflist
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_BAD_PARAM             - on bad DSA params or the data buffer is longer
*                                       than allowed. Buffer data can occupied up to the
*                                       maximum number of descriptors defined.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
* @retval GT_BAD_STATE             - function is not implemented for selected family
*/
GT_STATUS prvTgfNetIfSdmaTxPacketSend
(
    IN GT_U8                        devNum,
    IN PRV_TGF_NET_TX_PARAMS_STC    *pcktParamsPtr,
    IN GT_U8                        *buffList[],
    IN GT_U32                       buffLenList[],
    IN GT_U32                       numOfBufs
)
{
#ifdef CHX_FAMILY
    GT_STATUS                   rc;
    CPSS_DXCH_NET_TX_PARAMS_STC dxChNetTxPacket;

    cpssOsMemSet(&dxChNetTxPacket, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));

    rc = prvTgfConvertGenericToDxChNetTxParams(pcktParamsPtr,
        &dxChNetTxPacket);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChNetTxParams FAILED, "
            "rc = [%d]", rc);
        return rc;
    }

    rc = cpssDxChNetIfSdmaTxPacketSend(devNum, &dxChNetTxPacket, buffList,
        buffLenList, numOfBufs);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfNetIfSdmaTxPacketSend FAILED, "
            "rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(txQueue);
    TGF_PARAM_NOT_USED(checkResultPtr);

    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY */
}

