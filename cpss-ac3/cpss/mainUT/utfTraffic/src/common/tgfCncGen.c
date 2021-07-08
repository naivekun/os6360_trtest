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
* @file tgfCncGen.c
*
* @brief Generic API implementation for Cnc
*
* @version   12
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <common/tgfCncGen.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>


/* Functions to convert PRV_TGF_CNC_ to/from CPSS_DXCH_CNC_ */

static void cnvCNC_CLIENT_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_CLIENT_ENT   tgfCncClient,
    OUT CPSS_DXCH_CNC_CLIENT_ENT *dxchCncClientPtr
)
{
    switch (tgfCncClient)
    {
        case PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E;
            break;
        case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
            break;
        case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E;
            break;
        case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E;
            break;
        case PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E;
            break;
        case PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E;
            break;
        case PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E;
            break;
        case PRV_TGF_CNC_CLIENT_EGRESS_PCL_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E;
            break;
        case PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E;
            break;
        case PRV_TGF_CNC_CLIENT_TUNNEL_START_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E;
            break;
        case PRV_TGF_CNC_CLIENT_TTI_E:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_TTI_E;
            break;
            case PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E;
                break;
            case PRV_TGF_CNC_CLIENT_EGRESS_TRG_EPORT_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E;
                break;
            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E;
                break;
            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E;
                break;
            case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E;
                break;
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E;
                break;
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E;
                break;
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E;
                break;
            case PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E;
                break;
            case PRV_TGF_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E;
                break;
            case PRV_TGF_CNC_CLIENT_TM_PASS_DROP_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E;
                break;
            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_2_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E;
                break;
            case PRV_TGF_CNC_CLIENT_TTI_PARALLEL_3_E:
                *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E;
                break;
        default:
            *dxchCncClientPtr = CPSS_DXCH_CNC_CLIENT_LAST_E;
            break;
    }
}

static void cnvCNC_VLAN_INDEX_MODE_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_VLAN_INDEX_MODE_ENT   tgfCncVlanIndexMode,
    OUT CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT *dxchCncVlanIndexModePtr
)
{
    switch (tgfCncVlanIndexMode)
    {
        default:
        case PRV_TGF_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E:
            *dxchCncVlanIndexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E;
            break;
        case PRV_TGF_CNC_VLAN_INDEX_MODE_EVID_E:
            *dxchCncVlanIndexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E;
            break;
        case PRV_TGF_CNC_VLAN_INDEX_MODE_TAG1_VID_E:
            *dxchCncVlanIndexModePtr = CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E;
            break;
    }
}

static void cnvCNC_VLAN_INDEX_MODE_ENT_dxch2tgf
(
    IN  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT dxchCncVlanIndexMode,
    OUT PRV_TGF_CNC_VLAN_INDEX_MODE_ENT   *tgfCncVlanIndexModePtr
)
{
    switch (dxchCncVlanIndexMode)
    {
        default:
        case CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E:
            *tgfCncVlanIndexModePtr = PRV_TGF_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E;
            break;
        case CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E:
            *tgfCncVlanIndexModePtr = PRV_TGF_CNC_VLAN_INDEX_MODE_EVID_E;
            break;
        case CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E:
            *tgfCncVlanIndexModePtr = PRV_TGF_CNC_VLAN_INDEX_MODE_TAG1_VID_E;
            break;
    }
}

static void cnvCNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT   tgfCncPktTypeMode,
    OUT CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT *dxchCncPktTypeModePtr
)
{
    switch (tgfCncPktTypeMode)
    {
        default:
        case PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E:
            *dxchCncPktTypeModePtr = CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E;
            break;
        case PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_CPU_CODE_E:
            *dxchCncPktTypeModePtr = CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_CPU_CODE_E;
            break;
    }
}

static void cnvCNC_BYTE_COUNT_MODE_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_BYTE_COUNT_MODE_ENT   tgfCncByteCountMode,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT *dxchCncByteCountModePtr
)
{
    switch (tgfCncByteCountMode)
    {
        case PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E:
            *dxchCncByteCountModePtr = CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E;
            break;
        case PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E:
            *dxchCncByteCountModePtr = CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E;
            break;
        default:
            /* wrong value */
            *dxchCncByteCountModePtr = CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E + 1;
            break;
    }
}

static void cnvCNC_BYTE_COUNT_MODE_ENT_dxch2tgf
(
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT dxchCncByteCountMode,
    OUT PRV_TGF_CNC_BYTE_COUNT_MODE_ENT   *tgfCncByteCountModePtr
)
{
    switch (dxchCncByteCountMode)
    {
        case CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E:
            *tgfCncByteCountModePtr = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
            break;
        case CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E:
            *tgfCncByteCountModePtr = PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E;
            break;
        default:
            /* wrong value */
            *tgfCncByteCountModePtr = PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E + 1;
            break;
    }
}

static void cnvCNC_EGRESS_DROP_COUNT_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT   tgfCncEgrDropCountMode,
    OUT CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT *dxchCncEgrDropCountModePtr
)
{
    switch (tgfCncEgrDropCountMode)
    {
        case PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E:
            *dxchCncEgrDropCountModePtr =
                CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E;
            break;
        case PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E:
            *dxchCncEgrDropCountModePtr =
                CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E;
            break;
        case PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E:
            *dxchCncEgrDropCountModePtr =
                CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E;
            break;
        default:
            /* wrong value */
            *dxchCncEgrDropCountModePtr =
                CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E + 1;
            break;
    }
}

static void cnvCNC_EGRESS_DROP_COUNT_ENT_dxch2tgf
(
    IN  CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT dxchCncEgrDropCountMode,
    OUT PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT   *tgfCncEgrDropCountModePtr
)
{
    switch (dxchCncEgrDropCountMode)
    {
        case CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E:
            *tgfCncEgrDropCountModePtr =
                PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E;
            break;
        case CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E:
            *tgfCncEgrDropCountModePtr =
                PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E;
            break;
        case CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E:
            *tgfCncEgrDropCountModePtr =
                PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E;
            break;
        default:
            /* wrong value */
            *tgfCncEgrDropCountModePtr =
                PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E + 1;
            break;
    }
}

static void cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT   tgfCncCounterFormat,
    OUT CPSS_DXCH_CNC_COUNTER_FORMAT_ENT *dxchCncCounterFormatPtr
)
{
    switch (tgfCncCounterFormat)
    {
        case PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E:
            *dxchCncCounterFormatPtr =
                CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E;
            break;
        case PRV_TGF_CNC_COUNTER_FORMAT_MODE_1_E:
            *dxchCncCounterFormatPtr =
                CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E;
            break;
        case PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E:
            *dxchCncCounterFormatPtr =
                CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E;
            break;
        default:
            /* wrong value */
            *dxchCncCounterFormatPtr =
                CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E + 1;
            break;
    }
}

static void cnvCNC_COUNTER_FORMAT_ENT_dxch2tgf
(
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT dxchCncCounterFormat,
    OUT PRV_TGF_CNC_COUNTER_FORMAT_ENT   *tgfCncCounterFormatPtr
)
{
    switch (dxchCncCounterFormat)
    {
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E:
            *tgfCncCounterFormatPtr =
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E:
            *tgfCncCounterFormatPtr =
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_1_E;
            break;
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E:
            *tgfCncCounterFormatPtr =
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E;
            break;
        default:
            /* wrong value */
            *tgfCncCounterFormatPtr =
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E + 1;
            break;
    }
}

static void cnvCNC_CNC_COUNTER_STC_tgf2dxch
(
    IN  PRV_TGF_CNC_COUNTER_STC       *tgfCncCounrerPtr,
    OUT CPSS_DXCH_CNC_COUNTER_STC     *dxchCncCounrerPtr
)
{
    dxchCncCounrerPtr->byteCount   = tgfCncCounrerPtr->byteCount;
    dxchCncCounrerPtr->packetCount = tgfCncCounrerPtr->packetCount;
}

static void cnvCNC_CNC_COUNTER_STC_dxch2tgf
(
    IN  CPSS_DXCH_CNC_COUNTER_STC     *dxchCncCounrerPtr,
    OUT PRV_TGF_CNC_COUNTER_STC       *tgfCncCounrerPtr
)
{
    tgfCncCounrerPtr->byteCount   = dxchCncCounrerPtr->byteCount;
    tgfCncCounrerPtr->packetCount = dxchCncCounrerPtr->packetCount;
}

static void cnvCNC_COUNTING_ENABLE_UNIT_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT   tgfCncCounterEnUnit,
    OUT CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT *dxchCncCounterEnUnitPtr
)
{
    switch (tgfCncCounterEnUnit)
    {
        case PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E:
            *dxchCncCounterEnUnitPtr =
                CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E;
            break;
        case PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E:
            *dxchCncCounterEnUnitPtr =
                CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E;
            break;
        default:
            /* wrong value */
            *dxchCncCounterEnUnitPtr =
                CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E + 1;
            break;
    }
}

static void cnvCNC_EGRESS_QUEUE_CLIENT_MODE_ENT_tgf2dxch
(
    IN  PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   tgfCncEgrQueClientMode,
    OUT CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT *dxchCncEgrQueClientModePtr
)
{
    switch (tgfCncEgrQueClientMode)
    {
        case PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E:
            *dxchCncEgrQueClientModePtr =
                CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
            break;
        case PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E:
            *dxchCncEgrQueClientModePtr =
                CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
            break;
        default:
            /* wrong value */
            *dxchCncEgrQueClientModePtr =
                CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E + 1;
            break;
    }
}

static void cnvCNC_EGRESS_QUEUE_CLIENT_MODE_ENT_dxch2tgf
(
    IN  CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT dxchCncEgrQueClientMode,
    OUT PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   *tgfCncEgrQueClientModePtr
)
{
    switch (dxchCncEgrQueClientMode)
    {
        case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E:
            *tgfCncEgrQueClientModePtr =
                PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
            break;
        case CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E:
            *tgfCncEgrQueClientModePtr =
                PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
            break;
        default:
            /* wrong value */
            *tgfCncEgrQueClientModePtr =
                PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E + 1;
            break;
    }
}

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
#include <cpss/exMxPm/exMxPmGen/cnc/cpssExMxPmCnc.h>
#include <cpss/exMxPm/exMxPmGen/config/private/prvCpssExMxPmInfo.h>

/* Functions to convert PRV_TGF_CNC_ to/from CPSS_EXMXPM_CNC_ */

static void cnvCNC_CLIENT_ENT_tgf2exmxpm
(
    IN  PRV_TGF_CNC_CLIENT_ENT          tgfCncClient,
    OUT CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT *exmxpmCncClientPtr
)
{
    switch (tgfCncClient)
    {
        case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E:
            *exmxpmCncClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_INGRESS_PCL_LOOKUP_0_0_E;
            break;
        case PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E:
            *exmxpmCncClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_INGRESS_PCL_LOOKUP_1_0_E;
            break;
        case PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E:
            *exmxpmCncClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_TXQ_NETWORK_E;
            break;
        case PRV_TGF_CNC_CLIENT_EGRESS_PCL_E:
            *exmxpmCncClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_EGRESS_PCL_LOOKUP_0_0_E;
            break;
        case PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E:
            *exmxpmCncClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_HA_E;
            break;
        case PRV_TGF_CNC_CLIENT_TUNNEL_START_E:
            *exmxpmCncClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_HA_E;
            break;
        case PRV_TGF_CNC_CLIENT_TTI_E:
            *exmxpmCncClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_TTI_E;
            break;
        default:
            /* wrong value */
            *exmxpmCncClientPtr = 255;
            break;
    }
}

static GT_BOOL prvCpssExMxPmCnctMuxedBlockEnabled = GT_FALSE;

static GT_STATUS prvCpssExMxPmCncGetMuxedBlock
(
    IN   GT_BOOL forceEnabled,
    IN   GT_U32  block,
    OUT  GT_U32  *muxedBlockPtr
)
{
    *muxedBlockPtr = block;

    if ((forceEnabled == GT_FALSE)
        && (prvCpssExMxPmCnctMuxedBlockEnabled == GT_FALSE))
    {
        return GT_FAIL;
    }

    *muxedBlockPtr = (prvTgfCncFineTuningBlockAmountGet() - 1 - block);

    return GT_OK;

}

static GT_STATUS prvCpssExMxPmCncGetMuxedClient
(
    IN   CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT mainClient,
    OUT  CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT *muxedClientPtr
)
{
    /* default value to prevent compiler warning */
    *muxedClientPtr = mainClient;

    switch (mainClient)
    {
        case CPSS_EXMXPM_CNC_CLIENT_TYPE_TXQ_NETWORK_E:
            *muxedClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_TXQ_FABRIC_E;
            return GT_OK;
        case CPSS_EXMXPM_CNC_CLIENT_TYPE_TXQ_FABRIC_E:
            *muxedClientPtr = CPSS_EXMXPM_CNC_CLIENT_TYPE_TXQ_NETWORK_E;
            return GT_OK;
        default: return GT_FAIL;
    }
}

static void cnvCNC_BYTE_COUNT_MODE_ENT_tgf2exmxpm
(
    IN  PRV_TGF_CNC_BYTE_COUNT_MODE_ENT        tgfCncByteCountMode,
    OUT CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_ENT   *exmxpmCncByteCountModePtr
)
{
    switch (tgfCncByteCountMode)
    {
        case PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E:
            *exmxpmCncByteCountModePtr = CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_L2_E;
            break;
        case PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E:
            *exmxpmCncByteCountModePtr = CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_L3_E;
            break;
        default:
            /* wrong value */
            *exmxpmCncByteCountModePtr = 255;
            break;
    }
}

static void cnvCNC_BYTE_COUNT_MODE_ENT_exmxpm2tgf
(
    IN  CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_ENT   exmxpmCncByteCountMode,
    OUT PRV_TGF_CNC_BYTE_COUNT_MODE_ENT       *tgfCncByteCountModePtr
)
{
    switch (exmxpmCncByteCountMode)
    {
        case CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_L2_E:
            *tgfCncByteCountModePtr = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
            break;
        case CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_L3_E:
            *tgfCncByteCountModePtr = PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E;
            break;
        default:
            /* wrong value */
            *tgfCncByteCountModePtr = 255;
            break;
    }
}

static void cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm
(
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT          tgfCncCounterFormat,
    OUT CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT      *exmxpmCncCounterFormatPtr
)
{
    switch (tgfCncCounterFormat)
    {
        case PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E:
            *exmxpmCncCounterFormatPtr =
                CPSS_EXMXPM_CNC_COUNTER_FORMAT_MODE_0_E;
            break;
        case PRV_TGF_CNC_COUNTER_FORMAT_MODE_1_E:
            *exmxpmCncCounterFormatPtr =
                CPSS_EXMXPM_CNC_COUNTER_FORMAT_MODE_1_E;
            break;
        case PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E:
            *exmxpmCncCounterFormatPtr =
                CPSS_EXMXPM_CNC_COUNTER_FORMAT_MODE_2_E;
            break;
        default:
            /* wrong value */
            *exmxpmCncCounterFormatPtr = 255;
            break;
    }
}

static void cnvCNC_COUNTER_FORMAT_ENT_exmxpm2tgf
(
    IN  CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT exmxpmCncCounterFormat,
    OUT PRV_TGF_CNC_COUNTER_FORMAT_ENT     *tgfCncCounterFormatPtr
)
{
    switch (exmxpmCncCounterFormat)
    {
        case CPSS_EXMXPM_CNC_COUNTER_FORMAT_MODE_0_E:
            *tgfCncCounterFormatPtr =
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
            break;
        case CPSS_EXMXPM_CNC_COUNTER_FORMAT_MODE_1_E:
            *tgfCncCounterFormatPtr =
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_1_E;
            break;
        case CPSS_EXMXPM_CNC_COUNTER_FORMAT_MODE_2_E:
            *tgfCncCounterFormatPtr =
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E;
            break;
        default:
            /* wrong value */
            *tgfCncCounterFormatPtr = 255;
            break;
    }
}

static void cnvCNC_CNC_COUNTER_STC_tgf2exmxpm
(
    IN  PRV_TGF_CNC_COUNTER_STC         *tgfCncCounrerPtr,
    OUT CPSS_EXMXPM_CNC_COUNTER_STC     *exmxpmCncCounrerPtr
)
{
    exmxpmCncCounrerPtr->byteCount   = tgfCncCounrerPtr->byteCount;
    exmxpmCncCounrerPtr->packetCount = tgfCncCounrerPtr->packetCount;
}

static void cnvCNC_CNC_COUNTER_STC_exmxpm2tgf
(
    IN  CPSS_EXMXPM_CNC_COUNTER_STC     *exmxpmCncCounrerPtr,
    OUT PRV_TGF_CNC_COUNTER_STC         *tgfCncCounrerPtr
)
{
    tgfCncCounrerPtr->byteCount   = exmxpmCncCounrerPtr->byteCount;
    tgfCncCounrerPtr->packetCount = exmxpmCncCounrerPtr->packetCount;
}

static void cnvCNC_COUNTING_ENABLE_UNIT_ENT_tgf2exmxpm
(
    IN  PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT      tgfCncCounterEnUnit,
    OUT CPSS_EXMXPM_CNC_COUNTING_ENABLE_UNIT_ENT  *exmxpmCncCounterEnUnitPtr
)
{
    switch (tgfCncCounterEnUnit)
    {
        case PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E:
            *exmxpmCncCounterEnUnitPtr =
                CPSS_EXMXPM_CNC_COUNTING_ENABLE_UNIT_TTI_E;
            break;
        case PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E:
            *exmxpmCncCounterEnUnitPtr =
                CPSS_EXMXPM_CNC_COUNTING_ENABLE_UNIT_INGRESS_PCL_E;
            break;
        case PRV_TGF_CNC_COUNTING_ENABLE_UNIT_HA_E:
            *exmxpmCncCounterEnUnitPtr =
                CPSS_EXMXPM_CNC_COUNTING_ENABLE_UNIT_HA_E;
            break;
        case PRV_TGF_CNC_COUNTING_ENABLE_UNIT_EGRESS_PCL_E:
            *exmxpmCncCounterEnUnitPtr =
                CPSS_EXMXPM_CNC_COUNTING_ENABLE_UNIT_EGRESS_PCL_E;
            break;
        default:
            /* wrong value */
            *exmxpmCncCounterEnUnitPtr = 255;
            break;
    }
}

#endif /* EXMXPM_FAMILY */

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/


/**
* @internal prvTgfCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientEnableSet
(
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   updateEnable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY

    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    if ((updateEnable == GT_FALSE) && (dxchClient > 100))
    {
        /* disable not relevant client - success */
        return GT_OK;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncBlockClientEnableSet(
            devNum, blockNum,
            dxchClient, updateEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG4_MAC("[TGF]: cpssDxChCncBlockClientEnableSet FAILED, rc = [%d], blockNum=[%d], dxchClient=[%d], updateEnable=[%d]\n", rc,
                                blockNum, dxchClient, updateEnable);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT exmxpmClient;
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT muxedExmxpmClient;
    GT_U32                          muxedBlock;
    GT_STATUS                       muxedCientRc;

    cnvCNC_CLIENT_ENT_tgf2exmxpm(client, &exmxpmClient);

    muxedCientRc = prvCpssExMxPmCncGetMuxedClient(
        exmxpmClient, &muxedExmxpmClient);

    if ((updateEnable == GT_FALSE) && (exmxpmClient > 100))
    {
        /* disable not relevant client - success */
        return GT_OK;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncBlockClientEnableSet(
            devNum, blockNum,
            exmxpmClient, updateEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncBlockClientEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        if (muxedCientRc == GT_OK)
        {
            prvCpssExMxPmCnctMuxedBlockEnabled = updateEnable;
            rc = prvCpssExMxPmCncGetMuxedBlock(GT_TRUE, blockNum, &muxedBlock);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvCpssExMxPmCncGetMuxedBlock FAILED, rc = [%d]\n", rc);

                rc1 = rc;
            }
            rc = cpssExMxPmCncBlockClientEnableSet(
                devNum, muxedBlock,
                muxedExmxpmClient, updateEnable);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncBlockClientEnableSet FAILED, rc = [%d]\n", rc);

                rc1 = rc;
            }
        }
    }
    return rc1;

#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientEnableGet
(
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* call device specific API */
    rc = cpssDxChCncBlockClientEnableGet(
        devNum, blockNum, dxchClient, updateEnablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockClientEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT exmxpmClient;

    cnvCNC_CLIENT_ENT_tgf2exmxpm(client, &exmxpmClient);

    /* call device specific API */
    rc = cpssExMxPmCncBlockClientEnableGet(
        devNum, blockNum,
        exmxpmClient, updateEnablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncBlockClientEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] rangesBmpArr[]           - the counter index ranges bitmap array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientRangesSet
(
    IN  GT_U32                      blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT      client,
    IN  GT_U32                      rangesBmpArr[]
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;
    GT_U64                   indexRangesBmp;

    indexRangesBmp.l[0] = rangesBmpArr[0];
    indexRangesBmp.l[1] = rangesBmpArr[1];

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncBlockClientRangesSet(
            devNum, blockNum,
            dxchClient, indexRangesBmp);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG5_MAC("[TGF]: cpssDxChCncBlockClientRangesSet FAILED, rc = [%d], "
                                "blockNum=[%d], dxchClient=[%d], indexRangesBmp=[0x%08x:%08x]\n", rc,
                                    blockNum, dxchClient, indexRangesBmp.l[1], indexRangesBmp.l[0]);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_RANGE_BMP_STC   ranges;
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT exmxpmClient;
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT muxedExmxpmClient;
    GT_U32                          muxedBlock;
    GT_STATUS                       muxedCientRc;

    cnvCNC_CLIENT_ENT_tgf2exmxpm(client, &exmxpmClient);

    muxedCientRc = prvCpssExMxPmCncGetMuxedClient(
        exmxpmClient, &muxedExmxpmClient);

    ranges.rangeBmp[0] = rangesBmpArr[0];
    ranges.rangeBmp[1] = rangesBmpArr[1];
    ranges.rangeBmp[2] = rangesBmpArr[2];
    ranges.rangeBmp[3] = rangesBmpArr[3];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncBlockRangesSet(
            devNum, blockNum, &ranges);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockClientRangesSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
        if (muxedCientRc == GT_OK)
        {
            rc = prvCpssExMxPmCncGetMuxedBlock(GT_TRUE, blockNum, &muxedBlock);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvCpssExMxPmCncGetMuxedBlock FAILED, rc = [%d]\n", rc);

                rc1 = rc;
            }
            rc = cpssExMxPmCncBlockRangesSet(
                devNum, muxedBlock, &ranges);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockClientRangesSet FAILED, rc = [%d]\n", rc);

                rc1 = rc;
            }
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*
* @param[out] rangesBmpArr[]           - the counter index ranges bitmap array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientRangesGet
(
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_U32                    rangesBmpArr[]
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;
    GT_U64                   indexRangesBmp;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* call device specific API */
    rc = cpssDxChCncBlockClientRangesGet(
        devNum, blockNum, dxchClient, &indexRangesBmp);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockClientRangesGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    rangesBmpArr[0] = indexRangesBmp.l[0];
    rangesBmpArr[1] = indexRangesBmp.l[1];

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_RANGE_BMP_STC ranges;

    TGF_PARAM_NOT_USED(client);

    /* call device specific API */
    rc = cpssExMxPmCncBlockRangesGet(
        devNum, blockNum, &ranges);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockClientRangesGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    rangesBmpArr[0] = ranges.rangeBmp[0];
    rangesBmpArr[1] = ranges.rangeBmp[1];
    rangesBmpArr[2] = ranges.rangeBmp[2];
    rangesBmpArr[3] = ranges.rangeBmp[3];

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortClientEnableSet function
* @endinternal
*
* @brief   The function sets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
* @param[in] portIndex                - port index in UTF array
* @param[in] client                   - CNC client
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortClientEnableSet
(
    IN  GT_U8                     portIndex,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS                rc;
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);


    /* call device specific API */
    rc = cpssDxChCncPortClientEnableSet(
        prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
        dxchClient, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortClientEnableSet FAILED, rc = [%d]\n", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portIndex);
    TGF_PARAM_NOT_USED(client);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortClientEnableSetByDevPort function
* @endinternal
*
* @brief   The function sets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan and INGRESS_SRC_EPORT clients supported.
* @param[in] portNum                  - port Number
* @param[in] client                   - CNC client
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortClientEnableSetByDevPort
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    portNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS                rc;
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);


    /* call device specific API */
    rc = cpssDxChCncPortClientEnableSet(
        devNum, portNum, dxchClient, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortClientEnableSet FAILED, rc = [%d]\n", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(client);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCncPortClientEnableGet function
* @endinternal
*
* @brief   The function gets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
* @param[in] portIndex                - port index in UTF array
* @param[in] client                   - CNC client
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortClientEnableGet
(
    IN  GT_U8                     portIndex,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                rc;
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);


    /* call device specific API */
    rc = cpssDxChCncPortClientEnableGet(
        prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
        dxchClient, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortClientEnableGet FAILED, rc = [%d]\n", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portIndex);
    TGF_PARAM_NOT_USED(client);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncIngressVlanPassDropFromCpuCountEnableSet function
* @endinternal
*
* @brief   The function enables or disables counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncIngressVlanPassDropFromCpuCountEnableSet
(
    IN  GT_BOOL enable
)
{
#ifdef CHX_FAMILY

    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncIngressVlanPassDropFromCpuCountEnableGet function
* @endinternal
*
* @brief   The function gets status of counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncIngressVlanPassDropFromCpuCountEnableGet
(
    OUT GT_BOOL *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count counter mode for CNC client.
*
* @param[in] client                   - CNC client
* @param[in] countMode                - count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncClientByteCountModeSet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            client,
    IN  PRV_TGF_CNC_BYTE_COUNT_MODE_ENT   countMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_CLIENT_ENT          dxchClient;
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT dxchCncByteCountMode;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    cnvCNC_BYTE_COUNT_MODE_ENT_tgf2dxch(countMode, &dxchCncByteCountMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncClientByteCountModeSet(
            devNum, dxchClient, dxchCncByteCountMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncClientByteCountModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT     exmxpmClient;
    CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_ENT exmxpmCncByteCountMode;

    cnvCNC_CLIENT_ENT_tgf2exmxpm(client, &exmxpmClient);

    cnvCNC_BYTE_COUNT_MODE_ENT_tgf2exmxpm(countMode, &exmxpmCncByteCountMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncClientByteCountModeSet(
            devNum, exmxpmClient, exmxpmCncByteCountMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncClientByteCountModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count counter mode for CNC client.
*
* @param[in] client                   - CNC client
*
* @param[out] countModePtr             - (pointer to) count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncClientByteCountModeGet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            client,
    OUT PRV_TGF_CNC_BYTE_COUNT_MODE_ENT   *countModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_CLIENT_ENT          dxchClient;
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT dxchCncByteCountMode;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* call device specific API */
    rc = cpssDxChCncClientByteCountModeGet(
        devNum, dxchClient, &dxchCncByteCountMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncClientByteCountModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }
    cnvCNC_BYTE_COUNT_MODE_ENT_dxch2tgf(dxchCncByteCountMode, countModePtr);
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT     exmxpmClient;
    CPSS_EXMXPM_CNC_BYTE_COUNT_MODE_ENT exmxpmCncByteCountMode;

    cnvCNC_CLIENT_ENT_tgf2exmxpm(client, &exmxpmClient);

    /* call device specific API */
    rc = cpssExMxPmCncClientByteCountModeGet(
        devNum, exmxpmClient, &exmxpmCncByteCountMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncClientByteCountModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    cnvCNC_BYTE_COUNT_MODE_ENT_exmxpm2tgf(exmxpmCncByteCountMode, countModePtr);

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncEgressVlanDropCountModeSet function
* @endinternal
*
* @brief   The function sets Egress VLAN Drop counting mode.
*
* @param[in] mode                     - Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressVlanDropCountModeSet
(
    IN  PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT   mode
)
{
#ifdef CHX_FAMILY
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT dxchEgrDropCountMode;

    cnvCNC_EGRESS_DROP_COUNT_ENT_tgf2dxch(mode, &dxchEgrDropCountMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncEgressVlanDropCountModeSet(
            devNum, dxchEgrDropCountMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncEgressVlanDropCountModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mode);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncEgressVlanDropCountModeGet function
* @endinternal
*
* @brief   The function gets Egress VLAN Drop counting mode.
*
* @param[out] modePtr                  - (pointer to) Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on reserved value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressVlanDropCountModeGet
(
    OUT PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT dxchEgrDropCountMode;

    /* call device specific API */
    rc = cpssDxChCncEgressVlanDropCountModeGet(
        devNum, &dxchEgrDropCountMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncEgressVlanDropCountModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    cnvCNC_EGRESS_DROP_COUNT_ENT_dxch2tgf(dxchEgrDropCountMode, modePtr);
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(modePtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncVlanClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Vlan CNC clients.
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncVlanClientIndexModeSet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            vlanClient,
    IN  PRV_TGF_CNC_VLAN_INDEX_MODE_ENT   indexMode
)
{
#ifdef CHX_FAMILY
    GT_U8                               devNum  = 0;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_CLIENT_ENT            dxchVlanClient;
    CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT   dxchVlanIndexMode;

    cnvCNC_CLIENT_ENT_tgf2dxch(vlanClient, &dxchVlanClient);
    cnvCNC_VLAN_INDEX_MODE_ENT_tgf2dxch(indexMode, &dxchVlanIndexMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncVlanClientIndexModeSet(
            devNum, dxchVlanClient, dxchVlanIndexMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncVlanClientIndexModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vlanClient);
    TGF_PARAM_NOT_USED(indexMode);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncVlanClientIndexModeGet function
* @endinternal
*
* @brief   The function Gets the index calculation mode
*         for the Vlan CNC clients.
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
* @param[in] indexModePtr             - (pointer to) index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncVlanClientIndexModeGet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            vlanClient,
    OUT  PRV_TGF_CNC_VLAN_INDEX_MODE_ENT   *indexModePtr
)
{
    #ifdef CHX_FAMILY
    GT_U8                               devNum  = prvTgfDevNum;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_CLIENT_ENT            dxchVlanClient;
    CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT   dxchVlanIndexMode;

    cnvCNC_CLIENT_ENT_tgf2dxch(vlanClient, &dxchVlanClient);

    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
/*reset devNum*/
    devNum=prvTgfDevNum;
    /* call device specific API */
    rc = cpssDxChCncVlanClientIndexModeGet(
         devNum, dxchVlanClient, &dxchVlanIndexMode);
    if (GT_OK != rc)
     {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncVlanClientIndexModeGet FAILED, rc = [%d]\n", rc);
        rc1 = rc;
     }
 /*convert from CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT to PRV_TGF_CNC_VLAN_INDEX_MODE_ENT*/
    cnvCNC_VLAN_INDEX_MODE_ENT_dxch2tgf(dxchVlanIndexMode,indexModePtr);

    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vlanClient);
    TGF_PARAM_NOT_USED(indexModePtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}
/**
* @internal prvTgfCncPacketTypePassDropToCpuModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
* @param[in] toCpuMode                - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPacketTypePassDropToCpuModeSet
(
    IN  PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT   toCpuMode
)
{
#ifdef CHX_FAMILY
    GT_U8                                                devNum  = 0;
    GT_STATUS                                            rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT  dxchCncPktTypeMode;

    cnvCNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_tgf2dxch(toCpuMode, &dxchCncPktTypeMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncPacketTypePassDropToCpuModeSet(
            devNum, dxchCncPktTypeMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPacketTypePassDropToCpuModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(toCpuMode);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadEnableSet
(
    IN  GT_BOOL  enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncCounterClearByReadEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterClearByReadEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncCounterClearByReadEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterClearByReadEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadEnableGet
(
    OUT GT_BOOL  *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY

    /* call device specific API */
    rc = cpssDxChCncCounterClearByReadEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterClearByReadEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* call device specific API */
    rc = cpssExMxPmCncCounterClearByReadEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterClearByReadEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadValueSet
(
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    IN  PRV_TGF_CNC_COUNTER_STC           *counterPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounter;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);
    cnvCNC_CNC_COUNTER_STC_tgf2dxch(counterPtr, &dxchCounter);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncCounterClearByReadValueSet(
            devNum, dxchCounterFormat, &dxchCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterClearByReadValueSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCounter;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm(format, &exmxpmCounterFormat);
    cnvCNC_CNC_COUNTER_STC_tgf2exmxpm(counterPtr, &exmxpmCounter);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncCounterClearByReadValueSet(
            devNum, exmxpmCounterFormat, &exmxpmCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterClearByReadValueSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @param[out] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadValueGet
(
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT PRV_TGF_CNC_COUNTER_STC           *counterPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounter;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);

    /* call device specific API */
    rc = cpssDxChCncCounterClearByReadValueGet(
        devNum, dxchCounterFormat, &dxchCounter);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterClearByReadValueGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    cnvCNC_CNC_COUNTER_STC_dxch2tgf(&dxchCounter, counterPtr);
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCounter;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm(format, &exmxpmCounterFormat);

    /* call device specific API */
    rc = cpssExMxPmCncCounterClearByReadValueGet(
        devNum, exmxpmCounterFormat, &exmxpmCounter);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterClearByReadValueGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    cnvCNC_CNC_COUNTER_STC_exmxpm2tgf(&exmxpmCounter, counterPtr);

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterWraparoundEnableSet
(
    IN  GT_BOOL  enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncCounterWraparoundEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterWraparoundEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncCounterWraparoundEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterWraparoundEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterWraparoundEnableGet
(
    OUT GT_BOOL  *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY

    /* call device specific API */
    rc = cpssDxChCncCounterWraparoundEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterWraparoundEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* call device specific API */
    rc = cpssExMxPmCncCounterWraparoundEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterWraparoundEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters wrapped around
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
GT_STATUS prvTgfCncCounterWraparoundIndexesGet
(
    IN    GT_U32   blockNum,
    INOUT GT_U32   *indexNumPtr,
    OUT   GT_U32   indexesArr[]
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY

    /* call device specific API */
    rc = cpssDxChCncCounterWraparoundIndexesGet(
        devNum, blockNum, indexNumPtr, indexesArr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterWraparoundIndexesGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* call device specific API */
    rc = cpssExMxPmCncCounterWraparoundIndexesGet(
        devNum, blockNum, indexNumPtr, indexesArr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterWraparoundIndexesGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterSet
(
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    IN  PRV_TGF_CNC_COUNTER_STC           *counterPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounter;
    GT_BOOL                             enableClear;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);
    cnvCNC_CNC_COUNTER_STC_tgf2dxch(counterPtr, &dxchCounter);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            /******************************/
            /* the counters are read only */
            /******************************/
            /* do not call cpssDxChCncCounterSet(...) because it generate ERROR
                into the 'error LOG' */
            goto treat_readOnly_lbl;
        }
        /* call device specific API */
        rc = cpssDxChCncCounterSet(
            devNum, blockNum, index, dxchCounterFormat, &dxchCounter);
        if (GT_OK != rc)
        {
            if (rc == GT_NOT_APPLICABLE_DEVICE)
            {
                treat_readOnly_lbl:
                /* Device doesn't support CNC counters set - reset value by read counter */
                rc = cpssDxChCncCounterClearByReadEnableGet(devNum, &enableClear);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterClearByReadEnableGet FAILED, rc = [%d]\n", rc);
                }
                if (enableClear == GT_FALSE)
                {
                    /* Enable clear by read counters */
                    rc = cpssDxChCncCounterClearByReadEnableSet(devNum, GT_TRUE);
                    if (GT_OK != rc)
                    {
                        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterClearByReadEnableSet FAILED, rc = [%d]\n", rc);
                    }
                }
                /* Read counter */
                rc = cpssDxChCncCounterGet(
                    devNum, blockNum, index, dxchCounterFormat, &dxchCounter);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterGet FAILED, rc = [%d]\n", rc);
                }
                if (enableClear == GT_FALSE)
                {
                    /* Disable clear by read counters */
                    rc = cpssDxChCncCounterClearByReadEnableSet(devNum, GT_FALSE);
                    if (GT_OK != rc)
                    {
                        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterClearByReadEnableSet FAILED, rc = [%d]\n", rc);
                    }
                }
            }
            else
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterSet FAILED, rc = [%d]\n", rc);
            }

            rc1 = rc;
        }

        /* set the value to first device only, 0 to all others */
        cpssOsMemSet(&dxchCounter, 0, sizeof(dxchCounter));
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCounter;
    GT_STATUS                             rcMuxedBlock;
    GT_U32                                muxedBlock;

    rcMuxedBlock = prvCpssExMxPmCncGetMuxedBlock(GT_FALSE, blockNum, &muxedBlock);

    cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm(format, &exmxpmCounterFormat);
    cnvCNC_CNC_COUNTER_STC_tgf2exmxpm(counterPtr, &exmxpmCounter);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncCounterSet(
            devNum, blockNum, index, exmxpmCounterFormat, &exmxpmCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        /* set the value to first device only, 0 to all others */
        cpssOsMemSet(&exmxpmCounter, 0, sizeof(exmxpmCounter));

        if (rcMuxedBlock == GT_OK)
        {
            /* call device specific API */
            rc = cpssExMxPmCncCounterSet(
                devNum, muxedBlock, index, exmxpmCounterFormat, &exmxpmCounter);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterSet FAILED, rc = [%d]\n", rc);

                rc1 = rc;
            }
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

#ifdef EXMXPM_FAMILY
/**
* @internal prvExMxPmCncCounterGet function
* @endinternal
*
* @brief   The function adds the Puma3 counter contents to output accomulating counter.
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] exmxpmCounterFormat      - CNC counter HW format
*
* @param[out] exmxpmCounterSumPtr      - (pointer to) accomulating CNC counter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvExMxPmCncCounterGet
(
    IN  GT_U32                                blockNum,
    IN  GT_U32                                index,
    IN  CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat,
    OUT CPSS_EXMXPM_CNC_COUNTER_STC           *exmxpmCounterSumPtr
)
{
    GT_U8                                 devNum;
    GT_STATUS                             rc, rc1 = GT_OK;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCounter;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncCounterGet(
            devNum, blockNum, index, exmxpmCounterFormat, &exmxpmCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterGet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        PRV_UTF_LOG3_MAC(
            "[TGF]: cpssExMxPmCncCounterGet rc %d, blockNum %d, index %d\n",
            rc, blockNum, index);
        PRV_UTF_LOG4_MAC(
            "[TGF]: packets (%d, %d), bytes (%d, %d)\n",
            exmxpmCounter.packetCount.l[0], exmxpmCounter.packetCount.l[1],
            exmxpmCounter.byteCount.l[0], exmxpmCounter.byteCount.l[1]);

        /* sum over all devices */
        exmxpmCounterSumPtr->packetCount =
            prvCpssMathAdd64(
                exmxpmCounterSumPtr->packetCount, exmxpmCounter.packetCount);
        exmxpmCounterSumPtr->byteCount =
            prvCpssMathAdd64(
                exmxpmCounterSumPtr->byteCount, exmxpmCounter.byteCount);
    }

    return rc1;
}

#endif /*EXMXPM_FAMILY*/

/**
* @internal prvTgfCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterGet
(
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT PRV_TGF_CNC_COUNTER_STC           *counterPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounter;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounterSum;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);

    cpssOsMemSet(&dxchCounterSum, 0, sizeof(dxchCounterSum));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncCounterGet(
            devNum, blockNum, index, dxchCounterFormat, &dxchCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterGet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        /* sum over all devices */
        dxchCounterSum.packetCount =
            prvCpssMathAdd64(
                dxchCounterSum.packetCount, dxchCounter.packetCount);
        dxchCounterSum.byteCount =
            prvCpssMathAdd64(
                dxchCounterSum.byteCount, dxchCounter.byteCount);
    }

    cnvCNC_CNC_COUNTER_STC_dxch2tgf(&dxchCounterSum, counterPtr);
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCounterSum;
    GT_STATUS                             rcMuxedBlock;
    GT_U32                                muxedBlock;

    rcMuxedBlock = prvCpssExMxPmCncGetMuxedBlock(GT_FALSE, blockNum, &muxedBlock);

    cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm(format, &exmxpmCounterFormat);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    cpssOsMemSet(&exmxpmCounterSum, 0, sizeof(exmxpmCounterSum));

    rc = prvExMxPmCncCounterGet(
        blockNum, index, exmxpmCounterFormat, &exmxpmCounterSum);
    if (GT_OK != rc)
    {
        rc1 = rc;
    }

    if (rcMuxedBlock == GT_OK)
    {
        /* read and add muxed counter contents */
        rc = prvExMxPmCncCounterGet(
            muxedBlock, index, exmxpmCounterFormat, &exmxpmCounterSum);
        if (GT_OK != rc)
        {
            rc1 = rc;
        }
    }

    cnvCNC_CNC_COUNTER_STC_exmxpm2tgf(&exmxpmCounterSum, counterPtr);

    PRV_UTF_LOG3_MAC(
        "[TGF]: prvTgfCncCounterGet rc %d, blockNum %d, index %d)\n",
        rc, blockNum, index);
    PRV_UTF_LOG4_MAC(
        "[TGF]: packets (%d, %d), bytes (%d, %d)\n",
        counterPtr->packetCount.l[0], counterPtr->packetCount.l[1],
        counterPtr->byteCount.l[0], counterPtr->byteCount.l[1]);

    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         prvTgfCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS                rc = GT_OK;
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChCncBlockUploadTrigger(
        devNum, blockNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockUploadTrigger FAILED, rc = [%d]\n", rc);

    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmCncBlockUploadTrigger(
        devNum, blockNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncBlockUploadTrigger FAILED, rc = [%d]\n", rc);

    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more then one block in such
*         state, but an API matches the original HW representation of the state.
* @param[in] devNum                   - device number
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *inProcessBlocksBmpPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS                rc = GT_OK;
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChCncBlockUploadInProcessGet(
        devNum, inProcessBlocksBmpPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockUploadInProcessGet FAILED, rc = [%d]\n", rc);

    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmCncBlockUploadInProcessGet(
        devNum, inProcessBlocksBmpPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncBlockUploadInProcessGet FAILED, rc = [%d]\n", rc);

    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by prvTgfCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the prvTgfCncUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         prvTgfCncUploadedBlockGet one more time to get rest of the block.
* @param[in] devNum                   - device number
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncUploadedBlockGet
(
    IN  GT_U8                                devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT    PRV_TGF_CNC_COUNTER_STC           *counterValuesPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_STATUS                rc = GT_OK;
    GT_U32                   numOfCounterValues;
    GT_U32                   counterAmount;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCncCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCncCounter;


    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCncCounterFormat);

    for (numOfCounterValues = 0, counterAmount = 1;
          (numOfCounterValues < (*numOfCounterValuesPtr));
          counterValuesPtr ++, numOfCounterValues ++)
    {
        /* call device specific API */
        rc = cpssDxChCncUploadedBlockGet(
            devNum, &counterAmount, dxchCncCounterFormat, &dxchCncCounter);
        if ((GT_OK != rc) && (GT_NO_MORE != rc))
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncBlockUploadInProcessGet FAILED, rc = [%d]\n", rc);
            break;
        }

        if (counterAmount == 0)
        {
            break;
        }

        cnvCNC_CNC_COUNTER_STC_dxch2tgf(&dxchCncCounter, counterValuesPtr);
    }

    *numOfCounterValuesPtr = numOfCounterValues;
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCncCounterFormat;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCncCounter;


    cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm(format, &exmxpmCncCounterFormat);

    for (numOfCounterValues = 0, counterAmount = 1;
          (numOfCounterValues < (*numOfCounterValuesPtr));
          counterValuesPtr ++, numOfCounterValues ++)
    {
        /* call device specific API */
        rc = cpssExMxPmCncUploadedBlockGet(
            devNum, &counterAmount, exmxpmCncCounterFormat, &exmxpmCncCounter);
        if ((GT_OK != rc) && (GT_NO_MORE != rc))
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncUploadedBlockGet FAILED, rc = [%d]\n", rc);
            break;
        }

        if (counterAmount == 0)
        {
            break;
        }

        cnvCNC_CNC_COUNTER_STC_exmxpm2tgf(&exmxpmCncCounter, counterValuesPtr);
    }

    *numOfCounterValuesPtr = numOfCounterValues;
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncCountingEnableSet function
* @endinternal
*
* @brief   The function enables counting on selected cnc unit.
*
* @param[in] cncUnit                  - selected unit for enable\disable counting
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCountingEnableSet
(
    IN  PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT    cncUnit,
    IN  GT_BOOL                                 enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT dxchCncUnit;

    cnvCNC_COUNTING_ENABLE_UNIT_ENT_tgf2dxch(cncUnit, &dxchCncUnit);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncCountingEnableSet(
            devNum, dxchCncUnit, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCountingEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTING_ENABLE_UNIT_ENT exmxpmCncUnit;

    cnvCNC_COUNTING_ENABLE_UNIT_ENT_tgf2exmxpm(cncUnit, &exmxpmCncUnit);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncCountingEnableSet(
            devNum, exmxpmCncUnit, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCountingEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncCountingEnableGet function
* @endinternal
*
* @brief   The function gets enable counting status on selected cnc unit.
*
* @param[in] cncUnit                  - selected unit for enable\disable counting
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCountingEnableGet
(
    IN  PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT    cncUnit,
    OUT GT_BOOL                                 *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT dxchCncUnit;

    cnvCNC_COUNTING_ENABLE_UNIT_ENT_tgf2dxch(cncUnit, &dxchCncUnit);

    /* call device specific API */
    rc = cpssDxChCncCountingEnableGet(
        devNum, dxchCncUnit, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCountingEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTING_ENABLE_UNIT_ENT exmxpmCncUnit;

    cnvCNC_COUNTING_ENABLE_UNIT_ENT_tgf2exmxpm(cncUnit, &exmxpmCncUnit);

    /* call device specific API */
    rc = cpssExMxPmCncCountingEnableGet(
        devNum, exmxpmCncUnit, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCountingEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterFormatSet
(
    IN  GT_U32                            blockNum,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncCounterFormatSet(
            devNum, blockNum, dxchCounterFormat);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterFormatSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm(format, &exmxpmCounterFormat);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncCounterFormatSet(
            devNum, blockNum, exmxpmCounterFormat);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterFormatSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS prvTgfCncCounterFormatGet
(
    IN  GT_U32                            blockNum,
    OUT PRV_TGF_CNC_COUNTER_FORMAT_ENT    *formatPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;

    /* call device specific API */
    rc = cpssDxChCncCounterFormatGet(
        devNum, blockNum, &dxchCounterFormat);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncCounterFormatGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    cnvCNC_COUNTER_FORMAT_ENT_dxch2tgf(dxchCounterFormat, formatPtr);
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat;

    /* call device specific API */
    rc = cpssExMxPmCncCounterFormatGet(
        devNum, blockNum, &exmxpmCounterFormat);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncCounterFormatGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    cnvCNC_COUNTER_FORMAT_ENT_exmxpm2tgf(exmxpmCounterFormat, formatPtr);
    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @param[in] mode                     - Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressQueueClientModeSet
(
    IN  PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   mode
)
{
#ifdef CHX_FAMILY
    GT_U8                                      devNum  = 0;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT dxchCncEgrQueClientMode;

    cnvCNC_EGRESS_QUEUE_CLIENT_MODE_ENT_tgf2dxch(
        mode, &dxchCncEgrQueClientMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncEgressQueueClientModeSet(
            devNum, dxchCncEgrQueClientMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncEgressQueueClientModeSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mode);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressQueueClientModeGet
(
    OUT PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT dxchCncEgrQueClientMode;

    /* call device specific API */
    rc = cpssDxChCncEgressQueueClientModeGet(
        devNum, &dxchCncEgrQueClientMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncEgressQueueClientModeGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }
    cnvCNC_EGRESS_QUEUE_CLIENT_MODE_ENT_dxch2tgf(
        dxchCncEgrQueClientMode, modePtr);

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(modePtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncCpuAccessStrictPriorityEnableSet function
* @endinternal
*
* @brief   The function enables strict priority of CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCpuAccessStrictPriorityEnableSet
(
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY

    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncCpuAccessStrictPriorityEnableSet(
            devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChCncCpuAccessStrictPriorityEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncCpuAccessStrictPriorityEnableGet function
* @endinternal
*
* @brief   The function gets enable status of strict priority of
*         CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCpuAccessStrictPriorityEnableGet
(
    OUT GT_BOOL   *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChCncCpuAccessStrictPriorityEnableGet(
        devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChCncCpuAccessStrictPriorityEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortGroupBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientEnableSet
(
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   updateEnable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncPortGroupBlockClientEnableSet(
            devNum, portGroupsBmp, blockNum,
            dxchClient, updateEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupBlockClientEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT exmxpmClient;
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT muxedExmxpmClient;
    GT_U32                          muxedBlock;
    GT_STATUS                       muxedCientRc;

    cnvCNC_CLIENT_ENT_tgf2exmxpm(client, &exmxpmClient);

    muxedCientRc = prvCpssExMxPmCncGetMuxedClient(
        exmxpmClient, &muxedExmxpmClient);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncPortGroupBlockClientEnableSet(
            devNum, portGroupsBmp, blockNum,
            exmxpmClient, updateEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncPortGroupBlockClientEnableSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        if (muxedCientRc == GT_OK)
        {
            prvCpssExMxPmCnctMuxedBlockEnabled = updateEnable;
            rc = prvCpssExMxPmCncGetMuxedBlock(GT_TRUE, blockNum, &muxedBlock);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvCpssExMxPmCncGetMuxedBlock FAILED, rc = [%d]\n", rc);

                rc1 = rc;
            }
            rc = cpssExMxPmCncPortGroupBlockClientEnableSet(
                devNum, portGroupsBmp, muxedBlock,
                muxedExmxpmClient, updateEnable);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncPortGroupBlockClientEnableSet FAILED, rc = [%d]\n", rc);

                rc1 = rc;
            }
        }
    }

    return rc1;

#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncPortGroupBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientEnableGet
(
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *updateEnablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8                    devNum  = prvTgfDevNum;
    GT_STATUS                rc, rc1 = GT_OK;
#endif

#ifdef CHX_FAMILY
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* call device specific API */
    rc = cpssDxChCncPortGroupBlockClientEnableGet(
        devNum, portGroupsBmp, blockNum, dxchClient, updateEnablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupBlockClientEnableGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_CNC_CLIENT_TYPE_ENT exmxpmClient;

    cnvCNC_CLIENT_ENT_tgf2exmxpm(client, &exmxpmClient);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncPortGroupBlockClientEnableGet(
            devNum, portGroupsBmp, blockNum,
            exmxpmClient, updateEnablePtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncPortGroupBlockClientEnableGet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortGroupBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] indexRangesBmp           - bitmap of counter index ranges
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientRangesSet
(
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT      client,
    IN  GT_U64                      indexRangesBmp
)
{
#ifdef CHX_FAMILY

    GT_U8                    devNum  = 0;
    GT_STATUS                rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncPortGroupBlockClientRangesSet(
            devNum, portGroupsBmp, blockNum,
            dxchClient, indexRangesBmp);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncGroupBlockClientRangesSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(blockNum);
    TGF_PARAM_NOT_USED(client);
    TGF_PARAM_NOT_USED(indexRangesBmp);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncPortGroupBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*
* @param[out] indexRangesBmpPtr        - (pointer to) bitmap counter index ranges
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientRangesGet
(
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_U64                    *indexRangesBmpPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_CLIENT_ENT dxchClient;

    cnvCNC_CLIENT_ENT_tgf2dxch(client, &dxchClient);

    /* call device specific API */
    rc = cpssDxChCncPortGroupBlockClientRangesGet(
        devNum, portGroupsBmp, blockNum, dxchClient, indexRangesBmpPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupBlockClientRangesGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(blockNum);
    TGF_PARAM_NOT_USED(client);
    TGF_PARAM_NOT_USED(indexRangesBmpPtr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortGroupCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] portGroupIdArr[]         (pointer to) array of port group Ids of
*                                      counters wrapped around.
*                                      The NULL pointer supported.
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters
*                                      wrapped around
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
GT_STATUS prvTgfCncPortGroupCounterWraparoundIndexesGet
(
    IN    GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN    GT_U32                  blockNum,
    INOUT GT_U32                  *indexNumPtr,
    OUT   GT_U32                  portGroupIdArr[],
    OUT   GT_U32                  indexesArr[]
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChCncPortGroupCounterWraparoundIndexesGet(
        devNum, portGroupsBmp,
        blockNum, indexNumPtr, portGroupIdArr, indexesArr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupCounterWraparoundIndexesGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(blockNum);
    TGF_PARAM_NOT_USED(indexNumPtr);
    TGF_PARAM_NOT_USED(portGroupIdArr);
    TGF_PARAM_NOT_USED(indexesArr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortGroupCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupCounterSet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    IN  PRV_TGF_CNC_COUNTER_STC           *counterPtr
)
{
#ifdef CHX_FAMILY

    GT_U8                               devNum  = 0;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounter;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);
    cnvCNC_CNC_COUNTER_STC_tgf2dxch(counterPtr, &dxchCounter);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncPortGroupCounterSet(
            devNum, portGroupsBmp,
            blockNum, index, dxchCounterFormat, &dxchCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupCounterSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        /* set the value to first device only, 0 to all others */
        cpssOsMemSet(&dxchCounter, 0, sizeof(dxchCounter));
    }
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(blockNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(format);
    TGF_PARAM_NOT_USED(counterPtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncPortGroupCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupCounterGet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT PRV_TGF_CNC_COUNTER_STC           *counterPtr
)
{
#ifdef CHX_FAMILY

    GT_U8                               devNum  = 0;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounter;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCounterSum;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);

    cpssOsMemSet(&dxchCounterSum, 0, sizeof(dxchCounterSum));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncPortGroupCounterGet(
            devNum, portGroupsBmp,
            blockNum, index, dxchCounterFormat, &dxchCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupCounterGet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }

        /* sum over all devices */
        dxchCounterSum.packetCount =
            prvCpssMathAdd64(
                dxchCounterSum.packetCount, dxchCounter.packetCount);
        dxchCounterSum.byteCount =
            prvCpssMathAdd64(
                dxchCounterSum.byteCount, dxchCounter.byteCount);
    }

    cnvCNC_CNC_COUNTER_STC_dxch2tgf(&dxchCounterSum, counterPtr);
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8                               devNum  = 0;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_EXMXPM_CNC_COUNTER_FORMAT_ENT    exmxpmCounterFormat;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCounter;
    CPSS_EXMXPM_CNC_COUNTER_STC           exmxpmCounterSum;
    GT_STATUS                             rcMuxedBlock;
    GT_U32                                muxedBlock;

    rcMuxedBlock = prvCpssExMxPmCncGetMuxedBlock(GT_FALSE, blockNum, &muxedBlock);

    cnvCNC_COUNTER_FORMAT_ENT_tgf2exmxpm(format, &exmxpmCounterFormat);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    cpssOsMemSet(&exmxpmCounterSum, 0, sizeof(exmxpmCounterSum));

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmCncPortGroupCounterGet(
            devNum, portGroupsBmp, blockNum, index, exmxpmCounterFormat, &exmxpmCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncPortGroupCounterGet FAILED, rc = [%d]\n", rc);
            rc1 = rc;
        }

       /* sum over all devices */
        exmxpmCounterSum.packetCount =
            prvCpssMathAdd64(
                exmxpmCounterSum.packetCount, exmxpmCounter.packetCount);
        exmxpmCounterSum.byteCount =
            prvCpssMathAdd64(
                exmxpmCounterSum.byteCount, exmxpmCounter.byteCount);

        if (rcMuxedBlock == GT_OK)
        {
            /* call device specific API */
            rc = cpssExMxPmCncPortGroupCounterGet(
                devNum, portGroupsBmp, muxedBlock, index, exmxpmCounterFormat, &exmxpmCounter);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmCncPortGroupCounterGet FAILED, rc = [%d]\n", rc);
                rc1 = rc;
            }

           /* sum over all devices */
            exmxpmCounterSum.packetCount =
                prvCpssMathAdd64(
                    exmxpmCounterSum.packetCount, exmxpmCounter.packetCount);
            exmxpmCounterSum.byteCount =
                prvCpssMathAdd64(
                    exmxpmCounterSum.byteCount, exmxpmCounter.byteCount);
        }
    }

    cnvCNC_CNC_COUNTER_STC_exmxpm2tgf(&exmxpmCounterSum, counterPtr);

    return rc1;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncPortGroupBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         prvTgfCncPortGroupBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncPortGroupBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChCncPortGroupBlockUploadTrigger(
        devNum, portGroupsBmp, blockNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupBlockUploadTrigger FAILED, rc = [%d]\n", rc);

    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(blockNum);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortGroupBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more then one block in such
*         state, but an API matches the original HW representation of the state.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Checks all active port groups of the bitmap.
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    OUT GT_U32                      *inProcessBlocksBmpPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChCncPortGroupBlockUploadInProcessGet(
        devNum, portGroupsBmp, inProcessBlocksBmpPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupBlockUploadInProcessGet FAILED, rc = [%d]\n", rc);

    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(inProcessBlocksBmpPtr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortGroupUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by prvTgfCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the prvTgfCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         prvTgfCncPortGroupUploadedBlockGet one more time to get rest of the block.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncPortGroupUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     GT_PORT_GROUPS_BMP                portGroupsBmp,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT    PRV_TGF_CNC_COUNTER_STC           *counterValuesPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                           rc = GT_OK;
    GT_U32                              numOfCounterValues;
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCncCounterFormat;
    CPSS_DXCH_CNC_COUNTER_STC           dxchCncCounter;
    GT_U32                              counterAmount;


    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCncCounterFormat);

    for (numOfCounterValues = 0, counterAmount = 1;
          (numOfCounterValues < (*numOfCounterValuesPtr));
          counterValuesPtr ++, numOfCounterValues ++)
    {
        /* call device specific API */
        rc = cpssDxChCncPortGroupUploadedBlockGet(
            devNum, portGroupsBmp,
            &counterAmount, dxchCncCounterFormat, &dxchCncCounter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupBlockUploadInProcessGet FAILED, rc = [%d]\n", rc);
            break;
        }

        if (counterAmount == 0)
        {
            break;
        }

        cnvCNC_CNC_COUNTER_STC_dxch2tgf(&dxchCncCounter, counterValuesPtr);
    }

    *numOfCounterValuesPtr = numOfCounterValues;
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(numOfCounterValuesPtr);
    TGF_PARAM_NOT_USED(format);
    TGF_PARAM_NOT_USED(counterValuesPtr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncPortGroupCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupCounterFormatSet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format
)
{
#ifdef CHX_FAMILY

    GT_U8                               devNum  = 0;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;

    cnvCNC_COUNTER_FORMAT_ENT_tgf2dxch(format, &dxchCounterFormat);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCncPortGroupCounterFormatSet(
            devNum, portGroupsBmp, blockNum, dxchCounterFormat);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupCounterFormatSet FAILED, rc = [%d]\n", rc);

            rc1 = rc;
        }
    }
    return rc1;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(blockNum);
    TGF_PARAM_NOT_USED(format);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCncPortGroupCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS prvTgfCncPortGroupCounterFormatGet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    OUT PRV_TGF_CNC_COUNTER_FORMAT_ENT    *formatPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                               devNum  = prvTgfDevNum;
    GT_STATUS                           rc, rc1 = GT_OK;
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    dxchCounterFormat;

    /* call device specific API */
    rc = cpssDxChCncPortGroupCounterFormatGet(
        devNum, portGroupsBmp, blockNum, &dxchCounterFormat);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCncPortGroupCounterFormatGet FAILED, rc = [%d]\n", rc);

        rc1 = rc;
    }

    cnvCNC_COUNTER_FORMAT_ENT_dxch2tgf(dxchCounterFormat, formatPtr);
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portGroupsBmp);
    TGF_PARAM_NOT_USED(blockNum);
    TGF_PARAM_NOT_USED(formatPtr);

    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/******************************************************************************\
 *                       API for default settings                             *
\******************************************************************************/

/**
* @internal prvTgfCncFineTuningBlockAmountGet function
* @endinternal
*
* @brief   The function gets Amount of Cnc Blocks per device.
*         if Cnc not supported - 0 returned.
*/
GT_U32 prvTgfCncFineTuningBlockAmountGet
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_U8                               devNum  = prvTgfDevNum;

    return PRV_CPSS_DXCH_PP_MAC(devNum)->
            fineTuning.tableSize.cncBlocks;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* puma3 blocks amount */
    return 64;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return 0;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCncFineTuningBlockSizeGet function
* @endinternal
*
* @brief   The function gets Amount of Cnc Blocks per device.
*         if Cnc not supported - 0 returned.
*/
GT_U32 prvTgfCncFineTuningBlockSizeGet
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_U8                               devNum  = prvTgfDevNum;

    return PRV_CPSS_DXCH_PP_MAC(devNum)->
            fineTuning.tableSize.cncBlockNumEntries;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* puma3 block size */
    return 1024;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return 0;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/*========= Common scenarios ==============*/

/**
* @internal prvTgfCncGenConfigure function
* @endinternal
*
* @brief   This function sets typical Cnc configuration
*
* @param[in] cncConfigurationPtr      - (pointer to)CNC configuration data
* @param[in] stopOnErrors             - to stop after first fail (GT_TRUE, used for configuration)
*                                      or to continue (GT_FALS, use for restore).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS prvTgfCncGenConfigure
(
    IN PRV_TGF_CNC_CONFIGURATION_STC     *cncConfigurationPtr,
    IN GT_BOOL                           stopOnErrors
)
{
    GT_STATUS                   rc, rc1 = GT_OK;

    if (cncConfigurationPtr->configureCountingUnitEnable != GT_FALSE)
    {
        rc = prvTgfCncCountingEnableSet(
            cncConfigurationPtr->countingUnit,
            cncConfigurationPtr->countingUnitEnable);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncCountingEnableSet");
    }

    if (cncConfigurationPtr->configureClientEnable != GT_FALSE)
    {
        rc = prvTgfCncBlockClientEnableSet(
            cncConfigurationPtr->blockNum,
            cncConfigurationPtr->clientType,
            cncConfigurationPtr->clientEnable);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncBlockClientEnableSet");
    }

    if (cncConfigurationPtr->configureIndexRangeBitMap != GT_FALSE)
    {
        rc = prvTgfCncBlockClientRangesSet(
            cncConfigurationPtr->blockNum,
            cncConfigurationPtr->clientType,
            cncConfigurationPtr->indexRangeBitMap);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncBlockClientRangesSet");
    }

    if (cncConfigurationPtr->configureCounterFormat != GT_FALSE)
    {
        rc = prvTgfCncCounterFormatSet(
            cncConfigurationPtr->blockNum,
            cncConfigurationPtr->counterFormat);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncCounterFormatSet");
    }

    if (cncConfigurationPtr->configureCounterValue != GT_FALSE)
    {
        rc = prvTgfCncCounterSet(
            cncConfigurationPtr->blockNum,
            cncConfigurationPtr->counterNum,
            cncConfigurationPtr->counterFormat,
            &(cncConfigurationPtr->counterValue));
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncCounterSet");
    }

    if (cncConfigurationPtr->wraparoundEnable != GT_FALSE)
    {
        rc = prvTgfCncCounterWraparoundEnableSet(
            cncConfigurationPtr->wraparoundEnable);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncCounterWraparoundEnableSet");
    }

    if (cncConfigurationPtr->configureClearByReadEnable != GT_FALSE)
    {
        rc = prvTgfCncCounterClearByReadEnableSet(
            cncConfigurationPtr->clearByReadEnable);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncCounterClearByReadEnableSet");
    }

    if (cncConfigurationPtr->configureClearByReadValue != GT_FALSE)
    {
        rc = prvTgfCncCounterClearByReadValueSet(
            cncConfigurationPtr->counterFormat,
            &(cncConfigurationPtr->clearByReadValue));
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncCounterClearByReadValueSet");
    }

    if (cncConfigurationPtr->configureByteCountMode != GT_FALSE)
    {
        rc = prvTgfCncClientByteCountModeSet(
            cncConfigurationPtr->clientType,
            cncConfigurationPtr->byteCountMode);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncClientByteCountModeSet");
    }

    if (cncConfigurationPtr->configureEgressVlanDropCountMode != GT_FALSE)
    {
        rc = prvTgfCncEgressVlanDropCountModeSet(
            cncConfigurationPtr->egressVlanDropCountMode);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncEgressVlanDropCountModeSet");
    }

    if (cncConfigurationPtr->configureEgressQueueClientMode != GT_FALSE)
    {
        rc = prvTgfCncEgressQueueClientModeSet(
            cncConfigurationPtr->egressQueueClientMode);
        PRV_UTF_VERIFY_COND_STOP_MAC(
            stopOnErrors, rc1, rc,
            "prvTgfCncGenConfigure::prvTgfCncEgressQueueClientModeSet");
    }

    return rc1;
}

/**
* @internal prvTgfCncGenCheck function
* @endinternal
*
* @brief   This function sets typical Cnc check
*
* @param[in] cncCheckPtr              - (pointer to)CNC check data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS prvTgfCncGenCheck
(
    IN PRV_TGF_CNC_CHECK_STC     *cncCheckPtr
)
{
    GT_STATUS                   rc, rc1 = GT_OK;
    PRV_TGF_CNC_COUNTER_STC     counterValue;
    GT_U32                      wraparoundIndexesNum;
    GT_U32                      wraparoundIndexesArr[8];
    GT_U32                      i;
    GT_BOOL                     countersEqual;

    if (cncCheckPtr->compareCounterValue != GT_FALSE)
    {
        rc = prvTgfCncCounterGet(
            cncCheckPtr->blockNum,
            cncCheckPtr->counterNum,
            cncCheckPtr->counterFormat,
            &(counterValue));
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCncCounterGet\n");

        countersEqual = GT_TRUE;

        /* the 64-ones pattern content means - don't compare */
        if ((cncCheckPtr->counterValue.packetCount.l[0] != 0xFFFFFFFF)
            || (cncCheckPtr->counterValue.packetCount.l[1] != 0xFFFFFFFF))
        {
            if ((cncCheckPtr->counterValue.packetCount.l[0] != counterValue.packetCount.l[0])
                || (cncCheckPtr->counterValue.packetCount.l[1] != counterValue.packetCount.l[1]))
            {
                countersEqual = GT_FALSE;
            }
        }
        if ((cncCheckPtr->counterValue.byteCount.l[0] != 0xFFFFFFFF)
            || (cncCheckPtr->counterValue.byteCount.l[1] != 0xFFFFFFFF))
        {
            if ((cncCheckPtr->counterValue.byteCount.l[0] != counterValue.byteCount.l[0])
                || (cncCheckPtr->counterValue.byteCount.l[1] != counterValue.byteCount.l[1]))
            {
                countersEqual = GT_FALSE;
            }
        }
        if (countersEqual == GT_FALSE)
        {
            PRV_UTF_LOG0_MAC("different counter values\n");
            PRV_UTF_LOG4_MAC(
                "expected: packetCount: 0x%X, 0x%X, byteCount: 0x%X, 0x%X\n",
                cncCheckPtr->counterValue.packetCount.l[0],
                cncCheckPtr->counterValue.packetCount.l[1],
                cncCheckPtr->counterValue.byteCount.l[0],
                cncCheckPtr->counterValue.byteCount.l[1]);
            PRV_UTF_LOG4_MAC(
                "received: packetCount: 0x%X, 0x%X, byteCount: 0x%X, 0x%X\n",
                counterValue.packetCount.l[0],
                counterValue.packetCount.l[1],
                counterValue.byteCount.l[0],
                counterValue.byteCount.l[1]);
            rc1 = GT_FAIL;
        }
    }
    if (cncCheckPtr->compareWraparoundIndexes != GT_FALSE)
    {
        wraparoundIndexesNum = 8;
        rc = prvTgfCncCounterWraparoundIndexesGet(
            cncCheckPtr->blockNum,
            &wraparoundIndexesNum,
            wraparoundIndexesArr);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCncCounterWraparoundIndexesGet\n");
        if (cncCheckPtr->wraparoundIndexesNum != wraparoundIndexesNum)
        {
            PRV_UTF_LOG2_MAC(
                "wraparoundIndexesNum: expected: 0x%X, received: 0x%X\n",
                cncCheckPtr->wraparoundIndexesNum, wraparoundIndexesNum);
            rc1 = GT_FAIL;
            wraparoundIndexesNum =
                MIN(cncCheckPtr->wraparoundIndexesNum, wraparoundIndexesNum);
        }
        for (i = 0; (i < wraparoundIndexesNum); i++)
        {
            if (cncCheckPtr->wraparoundIndexesArr[i] != wraparoundIndexesArr[i])
            {
                PRV_UTF_LOG2_MAC(
                    "wraparoundIndexesNum[%d]: expected: 0x%X, received: 0x%X\n",
                    cncCheckPtr->wraparoundIndexesArr[i], wraparoundIndexesArr[i]);
                rc1 = GT_FAIL;
            }
        }
    }

    return rc1;
}

/**
* @internal prvTgfCncTxqClientIndexGet function
* @endinternal
*
* @brief   calculate TXQ client index.
*
* @param[in] devNum                   - device number
* @param[in] cnMode                   - GT_TRUE - CN mode, GT_FALSE - Tail Drop mode
* @param[in] droppedPacket            - 1-dropped, 0-passed
* @param[in] portNum                  - port number
* @param[in] tc                       - traffic class (0..7)
* @param[in] dp                       - drop priority (0..2)
*                                       calculated index or 0xFFFFFFFF on bad parameters
*/
GT_U32 prvTgfCncTxqClientIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL cnMode,
    IN  GT_U32  droppedPacket,
    IN  GT_U32  portNum,
    IN  GT_U32  tc,
    IN  GT_U32  dp
)
{
    GT_U32 txqPortNum;  /* Puma3 and SIP5 use TxQ Port number for index calculation */
    CPSS_DXCH_PORT_MAP_STC  portMap; /* port map for SIP5 devices */
    GT_STATUS               rc; /* return code */
    GT_U32                  cncIndex;

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        /* Puma3 version - CN mode not supported */
        if (cnMode)
        {
            /* cause crush in the caller */
            return 0xFFFFFFFF;
        }
        else
        {
            txqPortNum = (((portNum) & 0xF) + 18 * ((portNum) >> 4));
            return ((droppedPacket << 13) | (txqPortNum  << 5) | (tc << 2) | dp);
        }
    }
    else
    {
        /* DxCh version */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            rc = cpssDxChPortPhysicalPortMapGet(devNum,portNum,1,&portMap);
            if (rc != GT_OK)
            {
                return 0xFFFFFFFF;
            }

            txqPortNum = portMap.txqPortNumber;
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* support high tiles */
                txqPortNum %= _4K;
            }

            if (cnMode != GT_FALSE)
            {
                /* CN Counting Mode */
                if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    /* SIP6 builds index as next, where LSB is CN/non-CN[1bit]:
                       {Queue ID, Pass/Drop, CN/Non-CN}
                       Queue-ID = <port base> + <queue offset>
                       */
                    cncIndex = (    droppedPacket << 1) +
                               ((tc + txqPortNum) << 2);
                    PRV_UTF_LOG4_MAC(
                        "SIP6 CNC in CN mode txqPortNum 0x%X tc %d droppedPacket %d CNC index 0x%X\n",
                        txqPortNum, tc, droppedPacket, cncIndex);
                    return cncIndex;
                }
                else
                if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    /* BC3 builds index as next, where LSB is CN/non-CN[1bit]:
                       TxQ-Port[9:0], TC[2:0], Pass/Drop [1bit], CN/non-CN[1bit] */
                    cncIndex = ((droppedPacket << 1) + (tc << 2) + ((txqPortNum & 0x3FF) << 5));
                    PRV_UTF_LOG4_MAC(
                        "SIP_5_20 CNC in CN mode txqPortNum 0x%X tc %d droppedPacket %d CNC index 0x%X\n",
                        txqPortNum, tc, droppedPacket, cncIndex);
                    return cncIndex;
                }
                else
                {
                    cncIndex = ((tc << 1) + ((txqPortNum & 0xFF) << 4) + (droppedPacket << 12));
                    PRV_UTF_LOG4_MAC(
                        "SIP_5 CNC in CN mode txqPortNum 0x%X tc %d droppedPacket %d CNC index 0x%X\n",
                        txqPortNum, tc, droppedPacket, cncIndex);
                    return cncIndex;
                }
            }
            else
            {
                if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    /* BC3 builds index as next, where LSB is DP[1:0]:
                       {TxQ-Port[9:0], TC[2:0], DP[1:0]}, where DP[1:0] represent:
                       0-2: pass with DP=0-2, respectively; 3: drop with any DP.*/
                    if (droppedPacket)
                    {
                        dp = 3;
                    }

                    /* Tail-Drop Counting Mode */
                    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        /*Index = {Queue ID, Queue DP[1:0]}*/
                        cncIndex = (dp & 3) +
                                   ((tc + txqPortNum) << 2);
                        PRV_UTF_LOG4_MAC(
                            "SIP_6 CNC in TAIL-DROP mode txqPortNum 0x%X tc %d dp %d CNC index 0x%X\n",
                            txqPortNum, tc, dp, cncIndex);
                    }
                    else
                    {
                        cncIndex = ((dp & 3) + (tc << 2) + ((txqPortNum & 0x3FF) << 5));
                        PRV_UTF_LOG4_MAC(
                            "SIP_5_20 CNC in TAIL-DROP mode txqPortNum 0x%X tc %d dp %d CNC index 0x%X\n",
                            txqPortNum, tc, dp, cncIndex);
                    }

                    return cncIndex;
                }
                else
                {
                    cncIndex =
                        ((dp & 3) + (tc << 2) + ((txqPortNum & 0xFF) << 5)
                            + (droppedPacket << 13));
                    PRV_UTF_LOG5_MAC(
                        "SIP_5 CNC in TAIL-DROP mode txqPortNum 0x%X tc %d dp %d droppedPacket %d CNC index 0x%X\n",
                        txqPortNum, tc, dp, droppedPacket, cncIndex);
                    return cncIndex;
                }
            }
        }
        else
        {
            if (cnMode)
            {
                return ((tc << 1) + ((portNum & 0x3F) << 4) + (droppedPacket << 10));
            }
            else
            {
                if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
                {
                    /* xCat, xCat3, xCat2 */
                    cncIndex =
                        ((dp & 3) + (tc << 2) + ((portNum & 0x1F) << 5)
                            + (droppedPacket << 10));
                    PRV_UTF_LOG5_MAC(
                        "XCATs CNC in TAIL-DROP mode portNum 0x%X tc %d dp %d droppedPacket %d CNC index 0x%X\n",
                        portNum, tc, dp, droppedPacket, cncIndex);
                    return cncIndex;
                }
                else
                {
                    /* CH3, Lion, Lion2 */
                    cncIndex =
                        ((dp & 1) + (tc << 1) + ((portNum & 0x3F) << 4)
                            + (droppedPacket << 10) + ((dp >> 1) << 11));
                    PRV_UTF_LOG5_MAC(
                        "CH3,Lion,Lion2 CNC in TAIL-DROP mode portNum 0x%X tc %d dp %d droppedPacket %d CNC index 0x%X\n",
                        portNum, tc, dp, droppedPacket, cncIndex);
                    return cncIndex;
                }
            }
        }
    }
}



