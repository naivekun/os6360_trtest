/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapCpssDxChNetif.c
*
* DESCRIPTION:
*       Wrapper functions for DxCh network interface API functions
*
* FILE REVISION NUMBER:
*       $Revision: 45 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <galtisAgent/wrapUtil/cmdCpssNetReceive.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssCommon/private/prvCpssMemLib.h>
#include <galtisAgent/wrapCpss/dxCh/networkIf/cmdCpssDxChNetTransmit.h>

/* table TxNetIf/ExtTxNetIf global variables */
static GT_U32      netTxCurrIndex=0; /*Index of last descriptor got from table*/
extern DXCH_PKT_DESC_STC * cpssDxChTxPacketDescTbl;
/* Traffic generator transmit table descriptors */
extern DXCH_PKT_GEN_DESC_STC * cpssDxChTxGenPacketDescTbl[8][8];
extern GT_U32 wrCpssDxChNetIfTxDxChTblCapacity;
extern GT_U32 wrCpssDxChNetIfTxGenDxChTblCapacity[8][8];
extern GT_UINTPTR  cpssDxChTxGenPacketEvReqHndl;
extern GT_STATUS prvCpssDxChNetIfMultiNetIfNumberGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *numOfNetIfsPtr
);

GT_POOL_ID txBuffersPoolId;

static const char hexcode[] = "0123456789ABCDEF";
static GT_BOOL  poolCreated = GT_FALSE;

static CMD_STATUS cmdCpssDxChNetIfTxGenTblEntrySet
(
    IN  GT_UINTPTR * inArgs,
    IN  GT_UINTPTR * inFields,
    IN  GT_32 numFields,
    OUT DXCH_PKT_GEN_DESC_STC * netTxPacketGenDescPtr
);

/**
* @internal netIfMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs
*         with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*                                       NONE
*/
static void netIfMultiPortGroupsBmpGet
(
    IN   GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    /* default */
    *enablePtr  = GT_FALSE;
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return;
    }

    utilMultiPortGroupsBmpGet(devNum, enablePtr, portGroupsBmpPtr);
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChNetIfCpuCodeRateLimiterTableSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 rateLimiterIndex,
    IN  GT_U32 windowSize,
    IN  GT_U32 pktLimit
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    netIfMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChNetIfCpuCodeRateLimiterTableSet(devNum, rateLimiterIndex,
                                                        windowSize, pktLimit);
    }
    else
    {
        return cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet(devNum, pgBmp,
                                                                rateLimiterIndex,
                                                                windowSize,
                                                                pktLimit);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChNetIfCpuCodeRateLimiterTableGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 rateLimiterIndex,
    OUT  GT_U32 *windowSizePtr,
    OUT  GT_U32 *pktLimitPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    netIfMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChNetIfCpuCodeRateLimiterTableGet(devNum, rateLimiterIndex,
                                                        windowSizePtr,
                                                        pktLimitPtr);
    }
    else
    {
        return cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet(devNum, pgBmp,
                                                                rateLimiterIndex,
                                                                windowSizePtr,
                                                                pktLimitPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 windowResolution
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    netIfMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(devNum,
                                                              windowResolution);
    }
    else
    {
        return cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet(devNum,
                                                              pgBmp,
                                                              windowResolution);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet
(
    IN   GT_U8  devNum,
    OUT  GT_U32 *windowResolutionPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    netIfMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(devNum,
                                                           windowResolutionPtr);
    }
    else
    {
        return cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet(devNum,
                                                           pgBmp,
                                                           windowResolutionPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet
(
    IN   GT_U8               devNum,
    IN   GT_U32              rateLimiterIndex,
    OUT  GT_U32              *packetCntrPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    netIfMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet(devNum,
                                                            rateLimiterIndex,
                                                            packetCntrPtr);
    }
    else
    {
        return cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet(devNum,
                                                               pgBmp,
                                                               rateLimiterIndex,
                                                               packetCntrPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChNetIfCpuCodeRateLimiterDropCntrSet
(
    IN GT_U8                devNum,
    IN GT_U32               dropCntrVal
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    netIfMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChNetIfCpuCodeRateLimiterDropCntrSet(devNum,
                                                          dropCntrVal);
    }
    else
    {
        return cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet(devNum,
                                                                 pgBmp,
                                                                 dropCntrVal);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChNetIfCpuCodeRateLimiterDropCntrGet
(
    IN GT_U8                devNum,
    IN GT_U32               *dropCntrValPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    netIfMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(devNum,
                                                          dropCntrValPtr);
    }
    else
    {
        return cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet(devNum,
                                                                 pgBmp,
                                                                 dropCntrValPtr);
    }
}
/**
* @internal wrCpssDxChNetIfInit function
* @endinternal
*
* @brief   Initialize the network interface structures, Rx descriptors & buffers
*         and Tx descriptors -- SDMA or Ethernet port interface.
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
CMD_STATUS wrCpssDxChNetIfInit
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChNetIfInit(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfRemove function
* @endinternal
*
* @brief   This function is called upon Hot removal of a device, inorder to release
*         all Network Interface related structures.
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
CMD_STATUS wrCpssDxChNetIfRemove
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChNetIfRemove(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfPrePendTwoBytesHeaderSet function
* @endinternal
*
* @brief   Enables/Disable pre-pending a two-byte header to all packets forwarded
*         to the CPU (via the CPU port or the PCI interface).
*         This two-byte header is used to align the IPv4 header to 32 bits.
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*/
CMD_STATUS wrCpssDxChNetIfPrePendTwoBytesHeaderSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfPrePendTwoBytesHeaderSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfPrePendTwoBytesHeaderGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of pre-pending a two-byte header to all
*         packets forwarded to the CPU (via the CPU port or the PCI interface).
*         This two-byte header is used to align the IPv4 header to 32 bits.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNetIfPrePendTwoBytesHeaderGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChNetIfPrePendTwoBytesHeaderGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/****************Table cpssDxChNetIfSdmaRxCounters***************/
static GT_U8 gQueueIdx;

/**
* @internal wrCpssDxChNetIfSdmaRxCountersGetFirst function
* @endinternal
*
* @brief   For each packet processor, get the Rx packet counters from its SDMA
*         packet interface. Return the aggregate counter values for the given
*         traffic class queue. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Counters are reset on every read.
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaRxCountersGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC  rxCounters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    gQueueIdx = 0;

    /* call cpss api function */
    result = cpssDxChNetIfSdmaRxCountersGet(devNum, gQueueIdx, &rxCounters);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = rxCounters.rxInPkts;
    inFields[1] = rxCounters.rxInOctets;

    /* pack and output table fields */
    fieldOutput("%d%d%d", gQueueIdx, inFields[0], inFields[1]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaRxCountersGetNext function
* @endinternal
*
* @brief   For each packet processor, get the Rx packet counters from its SDMA
*         packet interface. Return the aggregate counter values for the given
*         traffic class queue. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Counters are reset on every read.
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaRxCountersGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC  rxCounters;
    GT_U32                              numOfNetIfs;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    gQueueIdx++;
    result = prvCpssDxChNetIfMultiNetIfNumberGet(devNum,&numOfNetIfs);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    if (gQueueIdx >= (CPSS_TC_RANGE_CNS * numOfNetIfs))
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfSdmaRxCountersGet(devNum, gQueueIdx, &rxCounters);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = rxCounters.rxInPkts;
    inFields[1] = rxCounters.rxInOctets;

    /* pack and output table fields */
    fieldOutput("%d%d%d", gQueueIdx, inFields[0], inFields[1]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/****************Table cpssDxChNetIfSdmaRxErrorCount***************/
static GT_U8                                  gQueueNum;
static CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC    gRxErrCount;
static GT_U32                                 gNumOfNetIfs;
/**
* @internal wrCpssDxChNetIfSdmaRxErrorCountGetFirst function
* @endinternal
*
* @brief   Returns the total number of Rx resource errors that occurred on a given
*         Rx queue . -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA DXCH Devices
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaRxErrorCountGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    gQueueNum = 0;

    result = prvCpssDxChNetIfMultiNetIfNumberGet(devNum,&gNumOfNetIfs);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfSdmaRxErrorCountGet(devNum, &gRxErrCount);


    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d", gQueueNum, gRxErrCount.counterArray[gQueueNum]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaRxErrorCountGetNext function
* @endinternal
*
* @brief   Returns the total number of Rx resource errors that occurred on a given
*         Rx queue . -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA DXCH Devices
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaRxErrorCountGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    gQueueNum++;

    if (gQueueNum >= (CPSS_TC_RANGE_CNS * gNumOfNetIfs))
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d", gQueueNum, gRxErrCount.counterArray[gQueueNum]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaRxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for RX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*/
CMD_STATUS wrCpssDxChNetIfSdmaRxQueueEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue = (GT_U8)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChNetIfSdmaRxQueueEnable(devNum, queue, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaRxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for RX
*         packets in CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
CMD_STATUS wrCpssDxChNetIfSdmaRxQueueEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfSdmaRxQueueEnableGet(devNum, queue, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for TX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*/
CMD_STATUS wrCpssDxChNetIfSdmaTxQueueEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue = (GT_U8)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChNetIfSdmaTxQueueEnable(devNum, queue, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
CMD_STATUS wrCpssDxChNetIfSdmaTxQueueEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfSdmaTxQueueEnableGet(devNum, queue, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfDuplicateEnableSet function
* @endinternal
*
* @brief   Enable descriptor duplication (mirror, STC and ingress mirroring
*         to analyzer port when the analyzer port is the CPU), Or Disable any kind
*         of duplication.
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
*/
CMD_STATUS wrCpssDxChNetIfDuplicateEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfDuplicateEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfDuplicateEnableGet function
* @endinternal
*
* @brief   Get descriptor duplication (mirror, STC and ingress mirroring
*         to analyzer port when the analyzer port is the CPU), Or Disable any kind
*         of duplication.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
CMD_STATUS wrCpssDxChNetIfDuplicateEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChNetIfDuplicateEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfPortDuplicateToCpuSet function
* @endinternal
*
* @brief   set per ingress port if Packets received from the port that are
*         duplicated and their duplication target is the CPU, ( mirror, STC and
*         ingress mirroring to analyzer port when the analyzer port is the CPU)
*         are will be duplicated or just are only forwarded to their original
*         target.
*         NOTE : port may be "CPU port" (63)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port number
*/
CMD_STATUS wrCpssDxChNetIfPortDuplicateToCpuSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_PORT_NUM   portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    CONVERT_DEV_PORT_U32_MAC(devNum,portNum);

    /* call cpss api function */
    result = cpssDxChNetIfPortDuplicateToCpuSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfPortDuplicateToCpuGet function
* @endinternal
*
* @brief   get per ingress port if Packets received from the port that are
*         duplicated and their duplication target is the CPU, ( mirror, STC and
*         ingress mirroring to analyzer port when the analyzer port is the CPU)
*         are will be duplicated or just are only forwarded to their original
*         target.
*         NOTE : port may be "CPU port" (63)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
CMD_STATUS wrCpssDxChNetIfPortDuplicateToCpuGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_PORT_NUM   portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    CONVERT_DEV_PORT_U32_MAC(devNum,portNum);

    /* call cpss api function */
    result = cpssDxChNetIfPortDuplicateToCpuGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeTcpSynSet function
* @endinternal
*
* @brief   Enable/Disable Application Specific CPU Code for TCP SYN packets
*         forwarded to the CPU - TCP_SYN_TO_CPU.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeTcpSynSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeTcpSynSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeTcpSynGet function
* @endinternal
*
* @brief   Get whether Application Specific CPU Code for TCP SYN packets forwarded
*         to the CPU - TCP_SYN_TO_CPU.
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeTcpSynGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeTcpSynGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/***************Table cpssDxChNetIfAppSpecificCpuCodeIpProtocol************/
static GT_U8 gIpProtIndex;

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolSet function
* @endinternal
*
* @brief   Set IP Protocol in the IP Protocol CPU Code Table with specific
*         CPU Code. There are 8 IP Protocols may be defined.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index or bad cpuCode
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                      devNum;
    GT_U32                     index;
    GT_U8                      protocol;
    CPSS_NET_RX_CPU_CODE_ENT   cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    index = (GT_U32)inFields[0];
    protocol = (GT_U8)inFields[2];/* inFields[1] for validity field in get*/
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inFields[3];

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet(devNum, index,
                                                          protocol, cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate function
* @endinternal
*
* @brief   Invalidate entry in the IP Protocol CPU Code Table .
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                      devNum;
    GT_U32                     index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate(devNum, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolGetFirst function
* @endinternal
*
* @brief   Get IP Protocol in the IP Protocol CPU Code Table with specific
*         CPU Code.
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                       devNum;
    GT_BOOL                     valid;
    GT_U8                       protocol;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gIpProtIndex = 0;

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(devNum, gIpProtIndex,
                                                          &valid, &protocol,
                                                          &cpuCode);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = gIpProtIndex;
    inFields[1] = valid;
    inFields[2] = protocol;
    inFields[3] = cpuCode;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolGetNext function
* @endinternal
*
* @brief   Get IP Protocol in the IP Protocol CPU Code Table with specific
*         CPU Code.
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                       devNum;
    GT_BOOL                     valid;
    GT_U8                       protocol;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gIpProtIndex++;

    if (gIpProtIndex > 7)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet(devNum, gIpProtIndex,
                                                          &valid, &protocol,
                                                          &cpuCode);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = gIpProtIndex;
    inFields[1] = valid;
    inFields[2] = protocol;
    inFields[3] = cpuCode;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}


/***********Table cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRange********/
static GT_U32   gRangeIndex;

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet function
* @endinternal
*
* @brief   Set range to TCP/UPD Destination Port Range CPU Code Table with specific
*         CPU Code. There are 16 ranges may be defined.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rangeIndex or
*                                       bad packetType or bad protocol or bad cpuCode
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                            devNum;
    GT_U32                           rangeIndex;
    GT_U16                           minDstPort;
    GT_U16                           maxDstPort;
    CPSS_NET_TCP_UDP_PACKET_TYPE_ENT packetType;
    CPSS_NET_PROT_ENT                protocol;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    rangeIndex = (GT_U32)inFields[0];
    /* inFields[1] for validity field in get*/
    minDstPort = (GT_U16)inFields[2];
    maxDstPort = (GT_U16)inFields[3];
    packetType = (CPSS_NET_TCP_UDP_PACKET_TYPE_ENT)inFields[4];
    protocol = (CPSS_NET_PROT_ENT)inFields[5];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inFields[6];

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet(devNum,
                                                                   rangeIndex,
                                                                   minDstPort,
                                                                   maxDstPort,
                                                                   packetType,
                                                                   protocol,
                                                                   cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate function
* @endinternal
*
* @brief   invalidate range to TCP/UPD Destination Port Range CPU Code Table with
*         specific CPU Code. There are 16 ranges may be defined.
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rangeIndex
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                      devNum;
    GT_U32                     index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate(devNum, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGetFirst function
* @endinternal
*
* @brief   Get range for TCP/UPD Destination Port Range CPU Code Table with specific
*         CPU Code. There are 16 ranges may be defined.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rangeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    GT_BOOL                             valid;
    GT_U16                              minDstPort;
    GT_U16                              maxDstPort;
    CPSS_NET_TCP_UDP_PACKET_TYPE_ENT    packetType;
    CPSS_NET_PROT_ENT                   protocol;
    CPSS_NET_RX_CPU_CODE_ENT            cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gRangeIndex = 0;

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(devNum,
                                                                   gRangeIndex,
                                                                   &valid,
                                                                   &minDstPort,
                                                                   &maxDstPort,
                                                                   &packetType,
                                                                   &protocol,
                                                                   &cpuCode);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = gRangeIndex;
    inFields[1] = valid;
    inFields[2] = minDstPort;
    inFields[3] = maxDstPort;
    inFields[4] = packetType;
    inFields[5] = protocol;
    inFields[6] = cpuCode;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                  inFields[3], inFields[4], inFields[5],
                                  inFields[6]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}


/**
* @internal wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGetNext function
* @endinternal
*
* @brief   Get range for TCP/UPD Destination Port Range CPU Code Table with specific
*         CPU Code. There are 16 ranges may be defined.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rangeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    GT_BOOL                             valid;
    GT_U16                              minDstPort;
    GT_U16                              maxDstPort;
    CPSS_NET_TCP_UDP_PACKET_TYPE_ENT    packetType;
    CPSS_NET_PROT_ENT                   protocol;
    CPSS_NET_RX_CPU_CODE_ENT            cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gRangeIndex++;

    if (gRangeIndex > 15)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet(devNum,
                                                                   gRangeIndex,
                                                                   &valid,
                                                                   &minDstPort,
                                                                   &maxDstPort,
                                                                   &packetType,
                                                                   &protocol,
                                                                   &cpuCode);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = gRangeIndex;
    inFields[1] = valid;
    inFields[2] = minDstPort;
    inFields[3] = maxDstPort;
    inFields[4] = packetType;
    inFields[5] = protocol;
    inFields[6] = cpuCode;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                  inFields[3], inFields[4], inFields[5],
                                  inFields[6]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeIpLinkLocalProtSet function
* @endinternal
*
* @brief   Configure CPU code for IPv4 and IPv6 Link Local multicast packets
*         with destination IP in range of 224.0.0.0 ... 224.0.0.255
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad ipVer or bad cpuCode
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeIpLinkLocalProtSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                            devNum;
    CPSS_IP_PROTOCOL_STACK_ENT       ipVer;
    GT_U8                            protocol;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ipVer = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    protocol = (GT_U8)inArgs[2];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeIpLinkLocalProtSet(devNum, ipVer, protocol,
                                                                   cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeIpLinkLocalProtGet function
* @endinternal
*
* @brief   get the Configuration CPU code for IPv4 and IPv6 Link Local multicast
*         packets with destination IP in range of 224.0.0.0 ... 224.0.0.255
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad ipVer
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeIpLinkLocalProtGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                            devNum;
    CPSS_IP_PROTOCOL_STACK_ENT       ipVer;
    GT_U8                            protocol;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ipVer = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    protocol = (GT_U8)inArgs[2];

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(devNum, ipVer, protocol,
                                                                   &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeIeeeReservedMcastProtSet function
* @endinternal
*
* @brief   Configure CPU code for particular IEEE reserved mcast protocol
*         (01-80-C2-00-00-00 ... 01-80-C2-00-00-FF)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeIeeeReservedMcastProtSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                            devNum;
    GT_U8                            protocol;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    protocol = (GT_U8)inArgs[1];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet(devNum, protocol,
                                                                  cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeIeeeReservedMcastProtGet function
* @endinternal
*
* @brief   get the Configuration CPU code for particular IEEE reserved mcast protocol
*         (01-80-C2-00-00-00 ... 01-80-C2-00-00-FF)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeIeeeReservedMcastProtGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                            devNum;
    GT_U8                            protocol;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    protocol = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(devNum, protocol,
                                                                  &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCode);
    return CMD_OK;
}

/****************Table cpssDxChNetIfCpuCode***************/
static GT_U32 gCPUcodeIndex;

/**
* @internal wrCpssDxChNetIfCpuCodeTableSet function
* @endinternal
*
* @brief   Function to set the "CPU code table" entry, for a specific CPU code.
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode or bad one of
*                                       bad one entryInfoPtr parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                   devNum;
    CPSS_NET_RX_CPU_CODE_ENT                cpuCode;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC  entryInfo;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inFields[0];
    entryInfo.tc =  (GT_U8)inFields[1];
    entryInfo.dp =  (CPSS_DP_LEVEL_ENT)inFields[2];
    entryInfo.truncate =  (GT_BOOL)inFields[3];
    entryInfo.cpuRateLimitMode =
                            (CPSS_NET_CPU_CODE_RATE_LIMIT_MODE_ENT)inFields[4];
    entryInfo.cpuCodeRateLimiterIndex =  (GT_U32)inFields[5];
    entryInfo.cpuCodeStatRateLimitIndex =  (GT_U32)inFields[6];
    entryInfo.designatedDevNumIndex =  (GT_U32)inFields[7];

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeTableSet(devNum, cpuCode, &entryInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeTableGetFirst function
* @endinternal
*
* @brief   Function to get the "CPU code table" entry, for a specific CPU code.
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                   devNum;
    CPSS_NET_RX_CPU_CODE_ENT                cpuCode;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC  entryInfo;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    gCPUcodeIndex = 0;

    /*use prv func to go over all ch2 CPU codes only. DSA index = 0-255*/
    result = prvCpssDxChNetIfDsaToCpuCode(gCPUcodeIndex, &cpuCode);

    if(result !=GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeTableGet(devNum, cpuCode, &entryInfo);

    if(result !=GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = cpuCode;
    inFields[1] = entryInfo.tc;
    inFields[2] = entryInfo.dp;
    inFields[3] = entryInfo.truncate;
    inFields[4] = entryInfo.cpuRateLimitMode;
    inFields[5] = entryInfo.cpuCodeRateLimiterIndex;
    inFields[6] = entryInfo.cpuCodeStatRateLimitIndex;
    inFields[7] = entryInfo.designatedDevNumIndex;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                  inFields[3], inFields[4], inFields[5],
                                  inFields[6], inFields[7]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}


/**
* @internal wrCpssDxChNetIfCpuCodeTableGetNext function
* @endinternal
*
* @brief   Function to get the "CPU code table" entry, for a specific CPU code.
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                   devNum;
    CPSS_NET_RX_CPU_CODE_ENT                cpuCode;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC  entryInfo;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    gCPUcodeIndex++;

    if(gCPUcodeIndex > 255)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /*use prv func to go over all ch2 CPU codes only. DSA index = 0-255*/
    result = prvCpssDxChNetIfDsaToCpuCode(gCPUcodeIndex, &cpuCode);

    if(result !=GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeTableGet(devNum, cpuCode, &entryInfo);

    if(result !=GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = cpuCode;
    inFields[1] = entryInfo.tc;
    inFields[2] = entryInfo.dp;
    inFields[3] = entryInfo.truncate;
    inFields[4] = entryInfo.cpuRateLimitMode;
    inFields[5] = entryInfo.cpuCodeRateLimiterIndex;
    inFields[6] = entryInfo.cpuCodeStatRateLimitIndex;
    inFields[7] = entryInfo.designatedDevNumIndex;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                  inFields[3], inFields[4], inFields[5],
                                  inFields[6], inFields[7]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}


/*Table: cpssDxChNetIfCpuCodeStatisticalRateLimits*/
static GT_U32 gStatIndex;
/**
* @internal wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet function
* @endinternal
*
* @brief   Function to set the "Statistical Rate Limits Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   devNum;
    GT_U32  index;
    GT_U32  statisticalRateLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    index = (GT_U32)inFields[0];
    statisticalRateLimit =  (GT_U32)inFields[1];

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(devNum, index,
                                                          statisticalRateLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableGetFirst function
* @endinternal
*
* @brief   Function to get the "Statistical Rate Limits Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    GT_U32                              statisticalRateLimit[1];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gStatIndex = 0;

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(devNum,
                                                               gStatIndex,
                                                        statisticalRateLimit);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d", gStatIndex, statisticalRateLimit[0]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}


/**
* @internal wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableGetNext function
* @endinternal
*
* @brief   Function to get the "Statistical Rate Limits Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    GT_U32                              statisticalRateLimit[1];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gStatIndex++;

    if(gStatIndex > 31)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet(devNum,
                                                               gStatIndex,
                                                        statisticalRateLimit);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d", gStatIndex, statisticalRateLimit[0]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}
/***********Table: cpssDxChNetIfCpuCodeDesignatedDevice*********************/
static GT_U32 gDesIndex;

/**
* @internal wrCpssDxChNetIfCpuCodeDesignatedDeviceTableSet function
* @endinternal
*
* @brief   Function to set the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index or bad
*                                       designatedDevNum
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeDesignatedDeviceTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8           devNum;
    GT_U32          index;
    GT_HW_DEV_NUM   designatedHwDevNum;
    GT_PORT_NUM     __Port; /* Dummy for converting. */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    index = (GT_U32)inFields[0];
    designatedHwDevNum =  (GT_HW_DEV_NUM)inFields[1];

    CONVERT_DEV_PORT_DATA_MAC(designatedHwDevNum,__Port);

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(devNum, index,
                                                          designatedHwDevNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeDesignatedDeviceTableGetFirst function
* @endinternal
*
* @brief   Function to get the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeDesignatedDeviceTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8           devNum;
    GT_HW_DEV_NUM   designatedHwDevNum;
    GT_PORT_NUM     __Port = 0; /* Dummy for converting. */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gDesIndex = 1;

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(devNum,
                                                          gDesIndex,
                                                          &designatedHwDevNum);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    CONVERT_BACK_DEV_PORT_DATA_MAC(designatedHwDevNum,__Port);

    /* pack and output table fields */
    fieldOutput("%d%d", gDesIndex, designatedHwDevNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}


/**
* @internal wrCpssDxChNetIfCpuCodeDesignatedDeviceTableGetNext function
* @endinternal
*
* @brief   Function to get the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeDesignatedDeviceTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8               devNum;
    GT_HW_DEV_NUM       designatedHwDevNum;
    GT_PORT_NUM         __Port = 0; /* Dummy for converting. */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gDesIndex++;

    if(gDesIndex > 7)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChNetIfCpuCodeDesignatedDeviceTableGet(devNum,
                                                          gDesIndex,
                                                          &designatedHwDevNum);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    CONVERT_BACK_DEV_PORT_DATA_MAC(designatedHwDevNum,__Port);

    /* pack and output table fields */
    fieldOutput("%d%d", gDesIndex, designatedHwDevNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/************************Table: cpssDxChNetIfCpuCodeRateLimiter************/
static GT_U32   gRateLimiterIndex;

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterTableSet function
* @endinternal
*
* @brief   Configure rate limiter window size and packets limit forwarded to CPU
*         per rate limiter index.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rateLimiterIndex
* @retval GT_OUT_OF_RANGE          - on pktLimit >= 0x10000 or windowSize >= 0x1000
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 rateLimiterIndex;
    GT_U32 windowSize;
    GT_U32 pktLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    rateLimiterIndex = (GT_U32)inFields[0];
    windowSize = (GT_U32)inFields[1];
    pktLimit = (GT_U32)inFields[2];

    /* call port group wrapper function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterTableSet(devNum,
                                                     rateLimiterIndex,
                                                     windowSize, pktLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterTableGetFirst function
* @endinternal
*
* @brief   get the Configuration rate limiter window size and packets limit
*         forwarded to CPU per rate limiter index.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rateLimiterIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 windowSize;
    GT_U32 pktLimit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    gRateLimiterIndex = 1;

    /* call port group api function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterTableGet(devNum,
                                                             gRateLimiterIndex,
                                                             &windowSize,
                                                             &pktLimit);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d", gRateLimiterIndex, windowSize, pktLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterTableGetNext function
* @endinternal
*
* @brief   get the Configuration rate limiter window size and packets limit
*         forwarded to CPU per rate limiter index.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rateLimiterIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 windowSize;
    GT_U32 pktLimit;
    GT_U32 gRateLimiterIndexMax;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        gRateLimiterIndexMax = 255;
    }
    else
    {
        gRateLimiterIndexMax = 31;
    }

    gRateLimiterIndex++;

    if(gRateLimiterIndex > gRateLimiterIndexMax)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call port group api function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterTableGet(devNum,
                                                             gRateLimiterIndex,
                                                             &windowSize,
                                                             &pktLimit);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d", gRateLimiterIndex, windowSize, pktLimit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet function
* @endinternal
*
* @brief   Set the TO CPU Rate Limiter Window Resolution,
*         which is used as steps in Rate Limiter Window size by
*         cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_OUT_OF_RANGE          - on windowResolution too large(depends on system clock)
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 windowResolution;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    windowResolution = (GT_U32)inArgs[1];

    /* call port group api function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(devNum,
                                                             windowResolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet function
* @endinternal
*
* @brief   Get the TO CPU Rate Limiter Window Resolution,
*         which is used as steps in Rate Limiter Window size by
*         cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
*
* @note   APPLICABLE DEVICES:      ALL DXCH2 Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 windowResolution;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call port group api function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(devNum,
                                                             &windowResolution);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", windowResolution);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterPacketCntrGet function
* @endinternal
*
* @brief   Gets the cpu code rate limiter packet counter for specific
*         rate limiter index.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is reset to 0 when the window ends.
*
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterPacketCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 rateLimiterIndex;
    GT_U32 packetCntr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    rateLimiterIndex = (GT_U32)inArgs[1];

    /* call port group api function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet (devNum,
                                                             rateLimiterIndex,
                                                             &packetCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", packetCntr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterDropCntrSet function
* @endinternal
*
* @brief   set the cpu code rate limiter drop counter to a specific value. This counter
*         counts the number of packets forwarded to the CPU and dropped due to any
*         of the CPU rate limiters.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterDropCntrSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 dropCntrVal;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropCntrVal = (GT_U32)inArgs[1];

    /* call port group api function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterDropCntrSet(devNum,
                                                                dropCntrVal);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeRateLimiterDropCntrGet function
* @endinternal
*
* @brief   get the number of packets forwarded to the CPU and dropped due to any
*         of the CPU rate limiters.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeRateLimiterDropCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_U32 dropCntrVal;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call port group api function */
    result = pg_wrap_cpssDxChNetIfCpuCodeRateLimiterDropCntrGet(devNum,
                                                                &dropCntrVal);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropCntrVal);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet function
* @endinternal
*
* @brief   Set which mode the 'TO_CPU' dsa tag field <Flow-ID/TT Offset> will use
*         'flowid' or 'ttOffset'.
*         related to CPSS_DXCH_NET_DSA_TO_CPU_STC::flowIdTtOffset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT)inArgs[1];

    /* call port group api function */
    result = cpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet(devNum,mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet function
* @endinternal
*
* @brief   Get which mode the 'TO_CPU' dsa tag field <Flow-ID/TT Offset> will use
*         'flowid' or 'ttOffset'.
*         related to CPSS_DXCH_NET_DSA_TO_CPU_STC::flowIdTtOffset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on non synch value between 2 relevant units.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call port group api function */
    result = cpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet(devNum,&mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}


/****************************
    TX
*****************************/

/**
* @internal setBufferArraysOfEqualSize function
* @endinternal
*
* @brief   build GT_BYTE_ARRY from string expanding or trunkating to size
*
* @param[in] sourceDataPtr            - byte array buffer (hexadecimal string)
* @param[in] totalSize                - exact total size of the buffer in bytes
* @param[in] bufferSize               - wanted size of each data buffer
*
* @param[out] pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array data
* @param[out] pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array size
* @param[out] numOfBuffs               - number of buffers the data was splited to.
*                                       None
*/
static GT_STATUS setBufferArraysOfEqualSize
(
    IN GT_U8   *sourceDataPtr,
    IN GT_U32   totalSize,
    IN GT_U32   bufferSize,
    OUT GT_U8   *pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN],
    OUT GT_U32  pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN],
    OUT GT_U32  *numOfBuffs

)
{
    GT_U32 element;
    GT_U32 len = totalSize;
    GT_U32 numOfBuffers = len / bufferSize;
    GT_U32 sizeOfLastBuff = len % bufferSize;
    GT_U32 i;

    numOfBuffers = (sizeOfLastBuff > 0) ? numOfBuffers + 1 : numOfBuffers;

    if (numOfBuffers > MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN)
        return GT_NO_RESOURCE;

    if(cmdOsPoolGetBufFreeCnt(txBuffersPoolId) < numOfBuffers)
        return GT_NO_RESOURCE;

    /* if the length of the data is bigger than the length given by the user,
       the data is truncate otherwise it is expanded */

    for(i = 0; i < numOfBuffers ; i++)
    {
        if(len < bufferSize)
            bufferSize = len;
        else
            len = len - bufferSize;

        pcktData[i] = cmdOsPoolGetBuf(txBuffersPoolId);

        /*for every buffer, we will copy the data*/
        for (element = 0; element < bufferSize; element++)
        {
            pcktData[i][element] =(GT_U8)( (GT_UINTPTR)(cmdOsStrChr(hexcode, cmdOsToUpper(*sourceDataPtr++)) - (GT_UINTPTR)hexcode)<< 4);

            pcktData[i][element]+= (GT_U8)((GT_UINTPTR)(cmdOsStrChr(hexcode, cmdOsToUpper(*sourceDataPtr++)) - (GT_UINTPTR)hexcode) );
        }

        pcktDataLen[i] = bufferSize;

    }

    *numOfBuffs = numOfBuffers;

    return GT_OK;
}

/**
* @internal freeBufferArraysOfEqualSize function
* @endinternal
*
* @brief   Free GT_BYTE_ARRY from string
*
* @param[in] pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array data
* @param[in] pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array size
*                                       None
*
* @note Toolkit:
*
*/
static GT_VOID freeBufferArraysOfEqualSize
(
    IN GT_U8   *pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN],
    IN GT_U32  pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN]
)
{

    GT_U32 i;
    for (i=0; i<MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN; i++)
    {

        /*nothing to free */
        if((pcktData[i] == NULL) || (pcktDataLen[i] == 0))
            break;

        cmdOsPoolFreeBuf(txBuffersPoolId, pcktData[i]);
   }
}

/*============================
          TX
=============================*/


/**
* @internal wrCpssDxChNetIfTxStart function
* @endinternal
*
* @brief   Starts transmition of packets
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxStart
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call tapi api function */
    result = cmdCpssDxChTxStart();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfTxStop function
* @endinternal
*
* @brief   Stop transmition of packets.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxStop
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call tapi api function */
    result = cmdCpssDxChTxStop();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfTxSetMode function
* @endinternal
*
* @brief   This command will set the the transmit parameters.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxSetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32 lCyclesNum;
    GT_U32 lGap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    lCyclesNum = (GT_U32)inArgs[0];
    lGap = (GT_U32)inArgs[1];

    /* call tapi api function */
    result = cmdCpssDxChTxSetMode(lCyclesNum, lGap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfTxGetMode function
* @endinternal
*
* @brief   This command will get the the transmit parameters.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxGetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32 lCyclesNum;
    GT_U32 lGap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call tapi api function */
    result = cmdCpssDxChTxGetMode(&lCyclesNum, &lGap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", lCyclesNum,lGap);
    return CMD_OK;
}


/**
* @internal wrTxSystemReset function
* @endinternal
*
* @brief   Preparation for system reset
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID wrTxSystemReset
(
    GT_VOID
)
{
    if(txBuffersPoolId)
    {
        cmdOsPoolDeletePool(txBuffersPoolId);
        txBuffersPoolId = 0;
    }

    if(cpssDxChTxPacketDescTbl)
    {
        cmdOsFree(cpssDxChTxPacketDescTbl);
        cpssDxChTxPacketDescTbl = 0;
    }

    netTxCurrIndex = 0;

    wrCpssDxChNetIfTxDxChTblCapacity = 0;

    poolCreated = GT_FALSE;

    cpssEventDestroy(cpssDxChTxGenPacketEvReqHndl);
    cpssDxChTxGenPacketEvReqHndl = 0;

}

/*
* Table: TxNetIf
*
* Description:
*     Transmited packet descriptor table.
*
* Comments:
*/
/**
* @internal wrCpssDxChNetIfSetTxNetIfTblEntryFromCpu function
* @endinternal
*
* @brief   Creates new transmit parameters description entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfSetTxNetIfTblEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_32   fieldSizeMax[] = {39, 32}; /* Maximal number of fields in each instance*/
    GT_32   fieldSizeMin[] = {32, 27}; /* Minimal number of fields in each instance*/
    GT_STATUS status;
    DXCH_PKT_DESC_STC * netTxPacketDesc;
    GT_U32   entryIndex=0;
    GT_U32   numOfBuffs;
    GT_PORT_NUM     __Port; /* Dummy for converting. */
    GT_HW_DEV_NUM   __HwDev; /* Dummy for converting. */


    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    if(numFields < fieldSizeMin[inArgs[1]])
        return CMD_FIELD_UNDERFLOW;
    if(numFields > fieldSizeMax[inArgs[1]])
        return CMD_FIELD_OVERFLOW;

    /* creat the pool only once */
    if(poolCreated == GT_FALSE)
    {
        /* register function to reset DB so after system reset we can send traffic again */
        wrCpssRegisterResetCb(wrTxSystemReset);
        /* create pool of buffers from Cache */
        status = cmdOsPoolCreateDmaPool(
                                  TX_BUFFER_SIZE + 64 /* ALIGN_ADDR_CHUNK */,
                                  GT_4_BYTE_ALIGNMENT,
                                  MAX_NUM_OF_BUFFERS_AT_POOL,
                                  GT_TRUE,
                                  &txBuffersPoolId);

        if (status != GT_OK)
        {
                cmdOsPrintf("ERROR-PoolCreateDmaPool\n\n");
                return status;
        }

        poolCreated = GT_TRUE;
    }

    netTxPacketDesc = cmdOsMalloc(sizeof(DXCH_PKT_DESC_STC));
    if (netTxPacketDesc==NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }
    cmdOsMemSet(netTxPacketDesc, 0, sizeof(DXCH_PKT_DESC_STC));


    /* Non zero data len */
    if(((GT_U32)inFields[7] == 0))
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }
    status = setBufferArraysOfEqualSize((GT_U8*)inFields[6],
                               (GT_U32)inFields[7],
                               TX_BUFFER_SIZE,
                               netTxPacketDesc->pcktData,
                               netTxPacketDesc->pcktDataLen,
                               &numOfBuffs);


    if (status != GT_OK)
    {
        cmdOsFree(netTxPacketDesc);
        freeBufferArraysOfEqualSize(netTxPacketDesc->pcktData,
                                    netTxPacketDesc->pcktDataLen);
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }


    netTxPacketDesc->numOfBuffers = numOfBuffs;
    netTxPacketDesc->cmdType = (DXCH_TX_CMD_ENT)inArgs[1];

    netTxPacketDesc->entryId = (GT_U32)inFields[0];
    netTxPacketDesc->txSyncMode = (GT_BOOL)inFields[1];
    netTxPacketDesc->packetIsTagged = (GT_BOOL)inFields[2];

    netTxPacketDesc->pcktsNum = (GT_U32)inFields[3];
    netTxPacketDesc->gap = (GT_U32)inFields[4];
    netTxPacketDesc->waitTime = (GT_U32)inFields[5];
    netTxPacketDesc->numSentPackets = (GT_U32)inFields[8];

    netTxPacketDesc->sdmaInfo.recalcCrc = (GT_BOOL)inFields[9];;
    netTxPacketDesc->sdmaInfo.txQueue = (GT_U8)inFields[10];;
    netTxPacketDesc->sdmaInfo.invokeTxBufferQueueEvent = inFields[11];



    netTxPacketDesc->dsaParam.commonParams.dsaTagType = inFields[12];
    netTxPacketDesc->dsaParam.commonParams.vpt = (GT_U8)inFields[13];
    netTxPacketDesc->dsaParam.commonParams.cfiBit = (GT_8)inFields[14];
    netTxPacketDesc->dsaParam.commonParams.vid = (GT_U16)inFields[15];

    netTxPacketDesc->dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.tc = (GT_U8)inFields[16];
    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dp = (CPSS_DP_LEVEL_ENT)inFields[17];
    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.egrFilterEn = (GT_BOOL)inFields[18];
    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.cascadeControl = (GT_BOOL)inFields[19];
    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.egrFilterRegistered = (GT_BOOL)inFields[20];
    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.srcId = (GT_U32)inFields[21];
    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.srcHwDev = (GT_HW_DEV_NUM)inFields[22];

    __HwDev = netTxPacketDesc->dsaParam.dsaInfo.fromCpu.srcHwDev;
    CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
    netTxPacketDesc->dsaParam.dsaInfo.fromCpu.srcHwDev = __HwDev;

    switch(inArgs[1])
    {
        case DXCH_TX_BY_VIDX:

            /* user can select CPSS_INTERFACE_VIDX_E = 2 or CPSS_INTERFACE_VID_E = 3 */
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.type = inFields[23];
            if(inFields[23] == 2)/*CPSS_INTERFACE_VIDX_E*/
            {
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.vidx = (GT_U16)inFields[24];
            }
            else/*CPSS_INTERFACE_VID_E*/
            {
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = (GT_U16)inFields[24];
            }
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inFields[25];

            __HwDev = netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;
            CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = __HwDev;

            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface = (GT_BOOL)inFields[26];
            /* type can be CPSS_INTERFACE_PORT_E = 0 or CPSS_INTERFACE_TRUNK_E = 1*/
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type = inFields[27];
            if(netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type == CPSS_INTERFACE_PORT_E)
            {
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inFields[28];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.portNum = (GT_PORT_NUM)inFields[29];

                __HwDev = netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.hwDevNum;
                __Port = netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.portNum;

                /* Override Device and Port */
                CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.hwDevNum = __HwDev;
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.portNum = __Port;

            }
            else/*CPSS_INTERFACE_TRUNK_E*/
            {
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId = (GT_TRUNK_ID)inFields[30];
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId);
            }
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.mirrorToAllCPUs = (GT_BOOL)inFields[31];

            if (numFields > 32)
            {
                netTxPacketDesc->dsaParam.commonParams.dropOnSource = (GT_BOOL)inFields[32];
                netTxPacketDesc->dsaParam.commonParams.packetIsLooped = (GT_BOOL)inFields[33];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedIsPhyPort = (GT_BOOL)inFields[34];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged = (CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT)inFields[35];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = (GT_BOOL)inFields[36];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstEport = (GT_PORT_NUM)inFields[37];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.tag0TpidIndex = (GT_U32)inFields[38];
            }

            break;

        case DXCH_TX_BY_PORT:
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inFields[23];
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = (GT_PORT_NUM)inFields[24];

            __HwDev = netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;
            __Port = netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum;

            /* Override Device and Port */
            CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = __HwDev;
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = __Port;

            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = (GT_BOOL)inFields[25];
            netTxPacketDesc->dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = (GT_BOOL)inFields[26];

            if (numFields > 27)
            {
                netTxPacketDesc->dsaParam.commonParams.dropOnSource = (GT_BOOL)inFields[27];
                netTxPacketDesc->dsaParam.commonParams.packetIsLooped = (GT_BOOL)inFields[28];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = (GT_BOOL)inFields[29];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstEport = (GT_PORT_NUM)inFields[30];
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.tag0TpidIndex = (GT_U32)inFields[31];
            }
            else
            {
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstEport = netTxPacketDesc->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum;
                netTxPacketDesc->dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
            }
            break;

        default:
            break;
    }

    /* Insert into table */
    status = cmdCpssDxChTxSetPacketDesc(netTxPacketDesc->entryId,
                                netTxPacketDesc,
                                GT_FALSE,   /* Non extended */
                                &entryIndex);

    /* No need to flush packet buffers because the allocation was made from the cache */
    /*
    if(status == GT_OK)
    {
        for (k=0; k < MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN; k++)
        {
            if( cpssDxChTxPacketDescTbl[entryIndex].pcktDataLen[k] == 0 )
                break;

            extDrvMgmtCacheFlush(GT_MGMT_DATA_CACHE_E,
                                 cpssDxChTxPacketDescTbl[entryIndex].pcktData[k],
                                 cpssDxChTxPacketDescTbl[entryIndex].pcktDataLen[k]);
        }
    }*/

    cmdOsFree(netTxPacketDesc);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/*
* Table: TxNetIf
*
* Description:
*     Transmited packet descriptor table.
*
* Comments:
*/
/**
* @internal wrCpssDxChNetIfSetTxNetIfTblEntryForward function
* @endinternal
*
* @brief   Creates new transmit parameters description entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfSetTxNetIfTblEntryForward
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
     GT_32   fieldSizeMax[] = {35, 34}; /* Maximal number of fields in each instance*/
    GT_32   fieldSizeMin[] = {27, 26}; /* Minimal number of fields in each instance*/

    GT_STATUS status;
    DXCH_PKT_DESC_STC * netTxPacketDesc;
    GT_U32   entryIndex=0;
    GT_U32   numOfBuffs;
    GT_PORT_NUM     __Port; /* Dummy for converting. */
    GT_HW_DEV_NUM   __HwDev; /* Dummy for converting. */


    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

        if(numFields < fieldSizeMin[inArgs[1]])
        return CMD_FIELD_UNDERFLOW;
    if(numFields > fieldSizeMax[inArgs[1]])
        return CMD_FIELD_OVERFLOW;

    /* creat the pool only once */
    if(poolCreated == GT_FALSE)
    {
        /* register function to reset DB so after system reset we can send traffic again */
        wrCpssRegisterResetCb(wrTxSystemReset);
        /* create pool of buffers from Cache */
        status = cmdOsPoolCreateDmaPool(
                                  TX_BUFFER_SIZE + 64 /* ALIGN_ADDR_CHUNK */,
                                  GT_4_BYTE_ALIGNMENT,
                                  MAX_NUM_OF_BUFFERS_AT_POOL,
                                  GT_TRUE,
                                  &txBuffersPoolId);
        if (status != GT_OK)
        {
                cmdOsPrintf("ERROR-PoolCreateDmaPool\n\n");
                return status;
        }

        poolCreated = GT_TRUE;
    }

    netTxPacketDesc = cmdOsMalloc(sizeof(DXCH_PKT_DESC_STC));
    if (netTxPacketDesc==NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }
    cmdOsMemSet(netTxPacketDesc, 0, sizeof(DXCH_PKT_DESC_STC));


    /* Non zero data len */
    if(((GT_U32)inFields[7] == 0))
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }
    status = setBufferArraysOfEqualSize((GT_U8*)inFields[6],
                               (GT_U32)inFields[7],
                               TX_BUFFER_SIZE,
                               netTxPacketDesc->pcktData,
                               netTxPacketDesc->pcktDataLen,
                               &numOfBuffs);

    if (status != GT_OK)
    {
        cmdOsFree(netTxPacketDesc);
        freeBufferArraysOfEqualSize(netTxPacketDesc->pcktData,
                                    netTxPacketDesc->pcktDataLen);
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }


    netTxPacketDesc->numOfBuffers = numOfBuffs;
    netTxPacketDesc->cmdType = (DXCH_TX_CMD_ENT)inArgs[1];

    netTxPacketDesc->entryId = (GT_U32)inFields[0];
    netTxPacketDesc->txSyncMode = (GT_BOOL)inFields[1];
    netTxPacketDesc->packetIsTagged = (GT_BOOL)inFields[2];

    netTxPacketDesc->pcktsNum = (GT_U32)inFields[3];
    netTxPacketDesc->gap = (GT_U32)inFields[4];
    netTxPacketDesc->waitTime = (GT_U32)inFields[5];
    netTxPacketDesc->numSentPackets = (GT_U32)inFields[8];

    netTxPacketDesc->sdmaInfo.recalcCrc = (GT_BOOL)inFields[9];;
    netTxPacketDesc->sdmaInfo.txQueue = (GT_U8)inFields[10];;
    netTxPacketDesc->sdmaInfo.invokeTxBufferQueueEvent = inFields[11];



    netTxPacketDesc->dsaParam.commonParams.dsaTagType = inFields[12];
    netTxPacketDesc->dsaParam.commonParams.vpt = (GT_U8)inFields[13];
    netTxPacketDesc->dsaParam.commonParams.cfiBit = (GT_8)inFields[14];
    netTxPacketDesc->dsaParam.commonParams.vid = (GT_U16)inFields[15];

    netTxPacketDesc->dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;

    netTxPacketDesc->dsaParam.dsaInfo.forward.srcIsTagged = (GT_BOOL)inFields[16];
    netTxPacketDesc->dsaParam.dsaInfo.forward.srcHwDev = (GT_HW_DEV_NUM)inFields[17];
    netTxPacketDesc->dsaParam.dsaInfo.forward.srcIsTrunk = (GT_BOOL)inFields[18];
    if(netTxPacketDesc->dsaParam.dsaInfo.forward.srcIsTrunk == GT_TRUE)
    {
        netTxPacketDesc->dsaParam.dsaInfo.forward.source.trunkId = (GT_TRUNK_ID)inFields[19];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(netTxPacketDesc->dsaParam.dsaInfo.forward.source.trunkId);

        __HwDev = netTxPacketDesc->dsaParam.dsaInfo.forward.srcHwDev;
        CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
        netTxPacketDesc->dsaParam.dsaInfo.forward.srcHwDev = __HwDev;
    }
    else
    {
        netTxPacketDesc->dsaParam.dsaInfo.forward.source.portNum = (GT_PORT_NUM)inFields[19];

        __HwDev = netTxPacketDesc->dsaParam.dsaInfo.forward.srcHwDev;
        __Port = netTxPacketDesc->dsaParam.dsaInfo.forward.source.portNum;

        /* Override Device and Port */
        CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
        netTxPacketDesc->dsaParam.dsaInfo.forward.srcHwDev = __HwDev;
        netTxPacketDesc->dsaParam.dsaInfo.forward.source.portNum = __Port;
    }

    netTxPacketDesc->dsaParam.dsaInfo.forward.srcId = (GT_U32)inFields[20];
    netTxPacketDesc->dsaParam.dsaInfo.forward.egrFilterRegistered = (GT_BOOL)inFields[21];
    netTxPacketDesc->dsaParam.dsaInfo.forward.wasRouted = (GT_BOOL)inFields[22];
    netTxPacketDesc->dsaParam.dsaInfo.forward.qosProfileIndex = (GT_U32)inFields[23];

    switch(inArgs[1])
    {
        case DXCH_TX_BY_VIDX:
            /* user can select CPSS_INTERFACE_VIDX_E = 2 or CPSS_INTERFACE_VID_E = 3 */
            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.type = inFields[24];
            if(inFields[24] == 2)/*CPSS_INTERFACE_VIDX_E*/
            {
                netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.vidx = (GT_U16)inFields[25];
            }
            else/*CPSS_INTERFACE_VID_E*/
            {
                netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.vlanId = (GT_U16)inFields[25];
            }

            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inFields[26];

            __HwDev = netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;

            /* Override Device and Port */
            CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = __HwDev;

            if (numFields > 27)
            {
                netTxPacketDesc->dsaParam.commonParams.dropOnSource = (GT_BOOL)inFields[27];
                netTxPacketDesc->dsaParam.commonParams.packetIsLooped = (GT_BOOL)inFields[28];
                netTxPacketDesc->dsaParam.dsaInfo.forward.isTrgPhyPortValid = (GT_BOOL)inFields[29];
                netTxPacketDesc->dsaParam.dsaInfo.forward.dstEport = (GT_PORT_NUM)inFields[30];
                netTxPacketDesc->dsaParam.dsaInfo.forward.tag0TpidIndex = (GT_U32)inFields[31];
                netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = (GT_BOOL)inFields[32];

                if(netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk == GT_TRUE)
                {
                    netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhy.trunkId = (GT_TRUNK_ID)inFields[33];
                }
                else
                {
                    netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhy.portNum  = (GT_PHYSICAL_PORT_NUM)inFields[33];
                }

                netTxPacketDesc->dsaParam.dsaInfo.forward.phySrcMcFilterEnable = (GT_BOOL)inFields[34];


            }


            break;

        case DXCH_TX_BY_PORT:
            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inFields[24];
            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = (GT_PORT_NUM)inFields[25];

            __HwDev = netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;
            __Port = netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.portNum;

            /* Override Device and Port */
            CONVERT_DEV_PORT_DATA_MAC(__HwDev,__Port);
            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = __HwDev;
            netTxPacketDesc->dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = __Port;

             if (numFields > 26)
            {
                netTxPacketDesc->dsaParam.commonParams.dropOnSource = (GT_BOOL)inFields[26];
                netTxPacketDesc->dsaParam.commonParams.packetIsLooped = (GT_BOOL)inFields[27];
                netTxPacketDesc->dsaParam.dsaInfo.forward.isTrgPhyPortValid = (GT_BOOL)inFields[28];
                netTxPacketDesc->dsaParam.dsaInfo.forward.dstEport = (GT_PORT_NUM)inFields[29];
                netTxPacketDesc->dsaParam.dsaInfo.forward.tag0TpidIndex = (GT_U32)inFields[30];
                netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = (GT_BOOL)inFields[31];

                if(netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhyIsTrunk == GT_TRUE)
                {
                    netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhy.trunkId = (GT_TRUNK_ID)inFields[32];
                }
                else
                {
                    netTxPacketDesc->dsaParam.dsaInfo.forward.origSrcPhy.portNum  = (GT_PHYSICAL_PORT_NUM)inFields[32];
                }

                netTxPacketDesc->dsaParam.dsaInfo.forward.phySrcMcFilterEnable = (GT_BOOL)inFields[33];


            }

            break;

        default:
            break;
    }

    /* Insert into table */
    status = cmdCpssDxChTxSetPacketDesc(netTxPacketDesc->entryId,
                                netTxPacketDesc,
                                GT_FALSE,   /* Non extended */
                                &entryIndex);

    /* No need to flush packet buffers because the allocation was made from the cache */
    /*
    if(status == GT_OK)
    {
        for (k=0; k < MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN; k++)
        {
            if( cpssDxChTxPacketDescTbl[entryIndex].pcktDataLen[k] == 0 )
                break;

            extDrvMgmtCacheFlush(GT_MGMT_DATA_CACHE_E,
                                 cpssDxChTxPacketDescTbl[entryIndex].pcktData[k],
                                 cpssDxChTxPacketDescTbl[entryIndex].pcktDataLen[k]);
        }
    }*/

    cmdOsFree(netTxPacketDesc);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfGetTxNetIfTblEntryFromCpu function
* @endinternal
*
* @brief   Get entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_U32 version
)
{
    GT_BYTE_ARRY lGtBuff;
    GT_HW_DEV_NUM  __HwDev;/* Dummy for converting. */
    GT_PORT_NUM    __Port = 0; /* Dummy for converting. */

    GT_TRUNK_ID __trunkId;/* Dummy for converting. */


    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    {
        GT_STATUS status;
        DXCH_PKT_DESC_STC SpecificPcktDesc;

        /* init to zeros */
        cmdOsMemSet(&SpecificPcktDesc, 0, sizeof(SpecificPcktDesc));

        status = cmdCpssDxChTxGetPacketDesc(&netTxCurrIndex, CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E, &SpecificPcktDesc);
        if (status != GT_OK)
        {
            /* indicates that table is empty or (GT_U32)inArgs[0] is invalid */
            if( (status == GT_EMPTY) ||
                (status == GT_NO_SUCH) ||
                (status == GT_NOT_FOUND) )
                galtisOutput(outArgs, GT_OK, "%d", -1);
            else
                galtisOutput(outArgs, status, "");

            cmdCpssDxChTxEndGetPacketDesc();
            return CMD_OK;
        }

        /* Mapping structure to fields */
         inFields[0] = SpecificPcktDesc.entryId;
         inFields[1] = SpecificPcktDesc.txSyncMode;
         inFields[2] = SpecificPcktDesc.packetIsTagged;
         inFields[3] = SpecificPcktDesc.pcktsNum;
         inFields[4] = SpecificPcktDesc.gap;
         inFields[5] = SpecificPcktDesc.waitTime;

         inFields[8] = SpecificPcktDesc.numSentPackets;
         inFields[9] = SpecificPcktDesc.sdmaInfo.recalcCrc;
         inFields[10] = SpecificPcktDesc.sdmaInfo.txQueue;
         inFields[11] = SpecificPcktDesc.sdmaInfo.invokeTxBufferQueueEvent;
         inFields[12] = SpecificPcktDesc.dsaParam.commonParams.dsaTagType;
         inFields[13] = SpecificPcktDesc.dsaParam.commonParams.vpt;
         inFields[14] = SpecificPcktDesc.dsaParam.commonParams.cfiBit;
         inFields[15] = SpecificPcktDesc.dsaParam.commonParams.vid;
         inFields[16] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.tc;
         inFields[17] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dp;
         inFields[18] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.egrFilterEn;
         inFields[19] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.cascadeControl;
         inFields[20] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.egrFilterRegistered;
         inFields[21] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.srcId;

         __HwDev = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.srcHwDev;
         CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
         inFields[22] = __HwDev;


         inArgs[1] = SpecificPcktDesc.cmdType;

         switch(SpecificPcktDesc.cmdType)
         {
             case DXCH_TX_BY_VIDX:

                 inFields[23] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstInterface.type;
                 if(inFields[23] == 2)/*CPSS_INTERFACE_VIDX_E*/
                 {
                     inFields[24] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstInterface.vidx;
                 }
                 else/*CPSS_INTERFACE_VID_E*/
                 {
                     inFields[24] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId;
                 }

                 __HwDev = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;
                 CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);

                 inFields[25] = __HwDev;
                 inFields[26] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface;
                 inFields[27] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type;
                 if(inFields[27] == 0) /*CPSS_INTERFACE_PORT_E*/
                 {
                     __HwDev  = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.hwDevNum;
                     __Port = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.portNum;

                     CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev, __Port);
                     inFields[28] = __HwDev  ;
                     inFields[29] = __Port ;

                     inFields[30] = 0;
                 }
                 else/*CPSS_INTERFACE_TRUNK_E*/
                 {
                     inFields[28] = 0;
                     inFields[29] = 0;
                     __trunkId = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId;
                     CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(__trunkId);
                     inFields[30] = __trunkId;
                 }

                 inFields[31] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.mirrorToAllCPUs;
                 if (version > 0)
                 {
                     inFields[32] = SpecificPcktDesc.dsaParam.commonParams.dropOnSource;
                     inFields[33] = SpecificPcktDesc.dsaParam.commonParams.packetIsLooped;
                     inFields[34] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedIsPhyPort;
                     inFields[35] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.srcIsTagged;
                     inFields[36] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid;
                     inFields[37] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstEport;
                     inFields[38] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.tag0TpidIndex;
                 }

                 break;

            case DXCH_TX_BY_PORT:

                 __HwDev  = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;
                 __Port = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum;

                 CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev, __Port);
                 inFields[23] = __HwDev  ;
                 inFields[24] = __Port ;

                 inFields[25] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged;
                 inFields[26] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU;
                 if (version > 0)
                 {
                     inFields[27] = SpecificPcktDesc.dsaParam.commonParams.dropOnSource;
                     inFields[28] = SpecificPcktDesc.dsaParam.commonParams.packetIsLooped;
                     inFields[29] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid;
                     inFields[30] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.dstEport;
                     inFields[31] = SpecificPcktDesc.dsaParam.dsaInfo.fromCpu.tag0TpidIndex;
                 }

                 break;

             default:
                 break;
         }


         /* Nathan - need to change this code */
         lGtBuff.data = cmdOsMalloc(SpecificPcktDesc.pcktDataLen[0]);
         lGtBuff.length = SpecificPcktDesc.pcktDataLen[0];
         if (!lGtBuff.data)
         {
             galtisOutput(outArgs, GT_NO_RESOURCE, "");
             return CMD_OK;
         }

         cmdOsMemCpy(lGtBuff.data, SpecificPcktDesc.pcktData[0],
             SpecificPcktDesc.pcktDataLen[0]);

         /* End --- need to be changed */


         switch(SpecificPcktDesc.cmdType)
         {
             case DXCH_TX_BY_VIDX:

                 /* pack and output table fields */
                 fieldOutput("%d%d%d%d%d%d%s%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                     inFields[0], inFields[1], inFields[2], inFields[3],
                     inFields[4], inFields[5], galtisBArrayOut(&lGtBuff),
                     inFields[7], inFields[8], inFields[9], inFields[10],
                     inFields[11], inFields[12], inFields[13], inFields[14],
                     inFields[15], inFields[16], inFields[17],inFields[18],
                     inFields[19], inFields[20], inFields[21],inFields[22],
                     inFields[23], inFields[24], inFields[25],inFields[26],
                     inFields[27], inFields[28], inFields[29],inFields[30], inFields[31]);
                 if (version > 0)
                 {
                     fieldOutputSetAppendMode();
                     fieldOutput("%d%d%d%d%d%d%d",
                         inFields[32], inFields[33], inFields[34], inFields[35],
                         inFields[36], inFields[37], inFields[38]);

                 }
                 break;

             case DXCH_TX_BY_PORT:
                 fieldOutput("%d%d%d%d%d%d%s%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                     inFields[0], inFields[1], inFields[2], inFields[3],
                     inFields[4], inFields[5], galtisBArrayOut(&lGtBuff),
                     inFields[7], inFields[8], inFields[9], inFields[10],
                     inFields[11], inFields[12], inFields[13], inFields[14],
                     inFields[15], inFields[16], inFields[17],inFields[18],
                     inFields[19], inFields[20], inFields[21],inFields[22],
                     inFields[23], inFields[24], inFields[25],inFields[26]);
                 if (version > 0)
                 {
                     fieldOutputSetAppendMode();
                     fieldOutput("%d%d%d%d%d",
                         inFields[27], inFields[28], inFields[29], inFields[30],
                         inFields[31]);

                 }

                 break;

             default:
                 break;
         }

        galtisOutput(outArgs, GT_OK, "%d%f", SpecificPcktDesc.cmdType);

        cmdOsFree(lGtBuff.data);
        cmdOsFree(SpecificPcktDesc.pcktData[0]);
    }
    return CMD_OK;
}


/**
* @internal wrCpssDxChNetIfGetTxNetIfTblFirstEntryFromCpu function
* @endinternal
*
* @brief   Get first entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblFirstEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    netTxCurrIndex = 0;

    status = cmdCpssDxChTxBeginGetPacketDesc();
    if (status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    return wrCpssDxChNetIfGetTxNetIfTblEntryFromCpu(inArgs, inFields, numFields,
                                                    outArgs, 0/* version */);
}

/**
* @internal wrCpssDxChNetIfGetTxNetIfTblFirstEntryFromCpu_1 function
* @endinternal
*
* @brief   Get first entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblFirstEntryFromCpu_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    netTxCurrIndex = 0;

    status = cmdCpssDxChTxBeginGetPacketDesc();
    if (status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    return wrCpssDxChNetIfGetTxNetIfTblEntryFromCpu(inArgs, inFields, numFields,
                                                    outArgs, 1/* version */);
}



/**
* @internal wrCpssDxChNetIfGetTxNetIfTblEntryForward function
* @endinternal
*
* @brief   Get first entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblEntryForward
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_U32 version
)
{
    GT_BYTE_ARRY lGtBuff;
    GT_HW_DEV_NUM  __HwDev;   /* Dummy for converting. */
    GT_PORT_NUM    __Port = 0;  /* Dummy for converting. */

    GT_TRUNK_ID __trunkId;/* Dummy for converting. */


    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    {
        GT_STATUS status;
        DXCH_PKT_DESC_STC SpecificPcktDesc;

        /* init to zeros */
        cmdOsMemSet(&SpecificPcktDesc, 0, sizeof(SpecificPcktDesc));

        status = cmdCpssDxChTxBeginGetPacketDesc();
        if (status != GT_OK)
        {
            galtisOutput(outArgs, status, "");
            return CMD_OK;
        }

        status = cmdCpssDxChTxGetPacketDesc(&netTxCurrIndex, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, &SpecificPcktDesc);
        if (status != GT_OK)
        {
            /* indicates that table is empty or (GT_U32)inArgs[0] is invalid */
            if( (status == GT_EMPTY) ||
                (status == GT_NO_SUCH) ||
                (status == GT_NOT_FOUND) )
                galtisOutput(outArgs, GT_OK, "%d", -1);
            else
                galtisOutput(outArgs, status, "");

            cmdCpssDxChTxEndGetPacketDesc();
            return CMD_OK;
        }

        /* Mapping structure to fields */

        inArgs[1] = SpecificPcktDesc.cmdType;

        inFields[0] = SpecificPcktDesc.entryId;
        inFields[1] = SpecificPcktDesc.txSyncMode;
        inFields[2] = SpecificPcktDesc.packetIsTagged;
        inFields[3] = SpecificPcktDesc.pcktsNum;
        inFields[4] = SpecificPcktDesc.gap;
        inFields[5] = SpecificPcktDesc.waitTime;

        inFields[8] = SpecificPcktDesc.numSentPackets;
        inFields[9] = SpecificPcktDesc.sdmaInfo.recalcCrc;
        inFields[10] = SpecificPcktDesc.sdmaInfo.txQueue;
        inFields[11] = SpecificPcktDesc.sdmaInfo.invokeTxBufferQueueEvent;

        inFields[12] = SpecificPcktDesc.dsaParam.commonParams.dsaTagType;
        inFields[13] = SpecificPcktDesc.dsaParam.commonParams.vpt;
        inFields[14] = SpecificPcktDesc.dsaParam.commonParams.cfiBit;
        inFields[15] = SpecificPcktDesc.dsaParam.commonParams.vid;

        inFields[16] = SpecificPcktDesc.dsaParam.dsaInfo.forward.srcIsTagged;

        __HwDev = SpecificPcktDesc.dsaParam.dsaInfo.forward.srcHwDev;

        CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
        inFields[17] = __HwDev;
        inFields[18] = SpecificPcktDesc.dsaParam.dsaInfo.forward.srcIsTrunk;
        if(SpecificPcktDesc.dsaParam.dsaInfo.forward.srcIsTrunk == GT_TRUE)
        {
            __trunkId = SpecificPcktDesc.dsaParam.dsaInfo.forward.source.trunkId;
            CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(__trunkId);
            inFields[19] = __trunkId;
        }
        else
        {
            __HwDev   = (GT_U32)SpecificPcktDesc.dsaParam.dsaInfo.forward.srcHwDev;
            __Port  = (GT_U32)SpecificPcktDesc.dsaParam.dsaInfo.forward.source.portNum;

            CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev, __Port) ;
            inFields[19] = __Port;
        }

        inFields[20] = SpecificPcktDesc.dsaParam.dsaInfo.forward.srcId;
        inFields[21] = SpecificPcktDesc.dsaParam.dsaInfo.forward.egrFilterRegistered;
        inFields[22] = SpecificPcktDesc.dsaParam.dsaInfo.forward.wasRouted;
        inFields[23] = SpecificPcktDesc.dsaParam.dsaInfo.forward.qosProfileIndex;

        switch(SpecificPcktDesc.cmdType)
        {
            case DXCH_TX_BY_VIDX:
                /* user can select CPSS_INTERFACE_VIDX_E = 2 or CPSS_INTERFACE_VID_E = 3 */
                inFields[24] = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstInterface.type;
                if(inFields[24] == 2)/*CPSS_INTERFACE_VIDX_E*/
                {
                    inFields[25] = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstInterface.vidx;
                }
                else/*CPSS_INTERFACE_VID_E*/
                {
                    inFields[25] = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstInterface.vlanId;
                }

                __HwDev = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;
                CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
                inFields[26] = __HwDev;

                 if (version == 1)
                 {
                     inFields[27] = SpecificPcktDesc.dsaParam.commonParams.dropOnSource;
                     inFields[28] = SpecificPcktDesc.dsaParam.commonParams.packetIsLooped;
                     inFields[29] = SpecificPcktDesc.dsaParam.dsaInfo.forward.isTrgPhyPortValid;
                     inFields[30] = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstEport;
                     inFields[31] = SpecificPcktDesc.dsaParam.dsaInfo.forward.tag0TpidIndex;
                     inFields[32] = SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk;

                     if(SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk == GT_TRUE)
                     {
                        inFields[33] = SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhy.trunkId;
                     }
                     else
                     {
                        inFields[33] = SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhy.portNum ;
                     }

                     inFields[34] = SpecificPcktDesc.dsaParam.dsaInfo.forward.phySrcMcFilterEnable;
                 }

                break;

            case DXCH_TX_BY_PORT:

                __HwDev  = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;
                __Port = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum;

                CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev, __Port);
                inFields[24] = __HwDev  ;
                inFields[25] = __Port ;

                 if (version == 1)
                 {
                     inFields[26] = SpecificPcktDesc.dsaParam.commonParams.dropOnSource;
                     inFields[27] = SpecificPcktDesc.dsaParam.commonParams.packetIsLooped;
                     inFields[28] = SpecificPcktDesc.dsaParam.dsaInfo.forward.isTrgPhyPortValid;
                     inFields[29] = SpecificPcktDesc.dsaParam.dsaInfo.forward.dstEport;
                     inFields[30] = SpecificPcktDesc.dsaParam.dsaInfo.forward.tag0TpidIndex;
                     inFields[31] = SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk;

                     if(SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk == GT_TRUE)
                     {
                        inFields[32] = SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhy.trunkId;
                     }
                     else
                     {
                        inFields[32] = SpecificPcktDesc.dsaParam.dsaInfo.forward.origSrcPhy.portNum ;
                     }

                     inFields[33] = SpecificPcktDesc.dsaParam.dsaInfo.forward.phySrcMcFilterEnable;
                 }

                break;

            default:
                break;
        }

        /* Nathan - need to change this code */
        lGtBuff.data = cmdOsMalloc(SpecificPcktDesc.pcktDataLen[0]);
        lGtBuff.length = SpecificPcktDesc.pcktDataLen[0];
        if (!lGtBuff.data)
        {
            galtisOutput(outArgs, GT_NO_RESOURCE, "");
            return CMD_OK;
        }

        cmdOsMemCpy(lGtBuff.data, SpecificPcktDesc.pcktData[0],
            SpecificPcktDesc.pcktDataLen[0]);

        /* End --- need to be changed */

        switch(inArgs[1])
        {
            case DXCH_TX_BY_VIDX:
            fieldOutput("%d%d%d%d%d%d%s%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3],
                inFields[4], inFields[5], galtisBArrayOut(&lGtBuff),
                inFields[7], inFields[8], inFields[9], inFields[10],
                inFields[11], inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17],inFields[18],
                inFields[19], inFields[20], inFields[21],inFields[22],
                inFields[23], inFields[24], inFields[25], inFields[26]);

             if (version == 1)
             {
                 fieldOutputSetAppendMode();
                 fieldOutput("%d%d%d%d%d%d%d%d",
                     inFields[27], inFields[28], inFields[29], inFields[30],
                     inFields[31], inFields[32], inFields[33],inFields[34]);

             }

            break;

            case DXCH_TX_BY_PORT:
            fieldOutput("%d%d%d%d%d%d%s%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3],
                inFields[4], inFields[5], galtisBArrayOut(&lGtBuff),
                inFields[7], inFields[8], inFields[9], inFields[10],
                inFields[11], inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17],inFields[18],
                inFields[19], inFields[20], inFields[21],inFields[22],
                inFields[23], inFields[24], inFields[25]);

              if (version == 1)
             {
                 fieldOutputSetAppendMode();
                 fieldOutput("%d%d%d%d%d%d%d%d",
                     inFields[26], inFields[27],inFields[28], inFields[29],
                     inFields[30],inFields[31], inFields[32], inFields[33]);

             }

            break;

            default:
            break;
        }

        galtisOutput(outArgs, GT_OK, "%d%f", SpecificPcktDesc.cmdType);

        cmdOsFree(lGtBuff.data);
        cmdOsFree(SpecificPcktDesc.pcktData[0]);
    }
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfGetTxNetIfTblFirstEntryForward_1 function
* @endinternal
*
* @brief   Get first entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/


CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblFirstEntryForward_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    netTxCurrIndex = 0;

    return wrCpssDxChNetIfGetTxNetIfTblEntryForward(inArgs, inFields, numFields,
                                                    outArgs, 1/* version */);
}

/**
* @internal wrCpssDxChNetIfGetTxNetIfTblFirstEntryForward function
* @endinternal
*
* @brief   Get first entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/


CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblFirstEntryForward
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    netTxCurrIndex = 0;

    return wrCpssDxChNetIfGetTxNetIfTblEntryForward(inArgs, inFields, numFields,
                                                    outArgs, 0/* version */);
}



/**
* @internal wrCpssDxChNetIfGetTxNetIfTblNextEntryFromCpu function
* @endinternal
*
* @brief   Get next entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblNextEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    netTxCurrIndex++;
    return wrCpssDxChNetIfGetTxNetIfTblEntryFromCpu(inArgs, inFields, numFields,
                                                    outArgs, 0/* version */);
}


/**
* @internal wrCpssDxChNetIfGetTxNetIfTblNextEntryFromCpu_1 function
* @endinternal
*
* @brief   Get next entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblNextEntryFromCpu_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    netTxCurrIndex++;
    return wrCpssDxChNetIfGetTxNetIfTblEntryFromCpu(inArgs, inFields, numFields,
                                                    outArgs, 1/* version */);
}

/**
* @internal wrCpssDxChNetIfGetTxNetIfTblNextEntryForward function
* @endinternal
*
* @brief   Get next entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblNextEntryForward
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
        /* check for valid arguments */
        if(inArgs == NULL || outArgs == NULL)
            return CMD_AGENT_ERROR;

        netTxCurrIndex ++;

        return wrCpssDxChNetIfGetTxNetIfTblEntryForward(inArgs, inFields, numFields,
                                                        outArgs, 0/* version */);
}

/**
* @internal wrCpssDxChNetIfGetTxNetIfTblNextEntryForward_1 function
* @endinternal
*
* @brief   Get next entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfGetTxNetIfTblNextEntryForward_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
        /* check for valid arguments */
        if(inArgs == NULL || outArgs == NULL)
            return CMD_AGENT_ERROR;

        netTxCurrIndex++;

        return wrCpssDxChNetIfGetTxNetIfTblEntryForward(inArgs, inFields, numFields,
                                                        outArgs, 1/* version */);
}


/**
* @internal wrCpssDxChNetIfDeleteTxNetIfTblEntryFromCpu function
* @endinternal
*
* @brief   Delete packet descriptor entry from the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfDeleteTxNetIfTblEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;
    GT_32   fieldSizeFromCpu[] = {32, 27}; /*Number of fields in each instance*/

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    if(numFields < fieldSizeFromCpu[inArgs[1]])
        return CMD_FIELD_UNDERFLOW;

    status = cmdCpssDxChTxDelPacketDesc((GT_U32)inFields[0], CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfDeleteTxNetIfTblEntryForward function
* @endinternal
*
* @brief   Delete packet descriptor entry from the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfDeleteTxNetIfTblEntryForward
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;
    GT_32   fieldSizeFromCpu[] = {27, 26}; /*Number of fields in each instance*/

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    if(numFields < fieldSizeFromCpu[inArgs[1]])
        return CMD_FIELD_UNDERFLOW;

    status = cmdCpssDxChTxDelPacketDesc((GT_U32)inFields[0], CPSS_DXCH_NET_DSA_CMD_FORWARD_E);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfClearTxNetIfTableFromCpu function
* @endinternal
*
* @brief   Clear all packet descriptors entries from the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfClearTxNetIfTableFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    status = cmdCpssDxChTxClearPacketDesc(CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfClearTxNetIfTableForward function
* @endinternal
*
* @brief   Clear all packet descriptors entries from the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfClearTxNetIfTableForward
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    status = cmdCpssDxChTxClearPacketDesc(CPSS_DXCH_NET_DSA_CMD_FORWARD_E);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}


static GT_U32 netRxExtLastIndex;

/**
* @internal wrCpssDxChNetIfRxPacketTableGet function
* @endinternal
*
* @brief   Get first/next entry from the DXCH rxNetIf table. (always "TO_CPU" dsa tag !)
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval getNext                  - GT_TRUE - "get next"
* @retval GT_FALSE                 - "get first"
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfRxPacketTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL getNext,
    IN  GT_U32  version
)
{
    GT_BYTE_ARRY byteArry;
    GT_STATUS rc;
    GT_U32 packetActualLength;
    GT_U8  packetBuff[GALTIS_RX_BUFFER_MAX_SIZE_CNS];
    GT_U32 buffLen = GALTIS_RX_BUFFER_MAX_SIZE_CNS;
    GT_U8  devNum;
    GT_U8  queue;
    CPSS_DXCH_NET_RX_PARAMS_STC rxParam;
    GT_U8*      packetString;
    GT_U32  ii;
    CPSS_DXCH_NET_DSA_COMMON_STC         *commonParamsPtr;
    CPSS_DXCH_NET_DSA_TO_CPU_STC         *toCpuPtr;
    CPSS_DXCH_NET_DSA_TO_ANALYZER_STC    *toAnalyzerPtr;
    CPSS_DXCH_NET_DSA_FORWARD_STC        *forwardPtr;
    GT_HW_DEV_NUM                        __HwDev; /* Dummy for converting. */
    GT_PORT_NUM                          __Port;/* Dummy for converting. */
    GT_U32   portOrTrunkId;
    GT_TRUNK_ID __trunkId;/* Dummy for converting. */


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (getNext == GT_FALSE)
    {
        rc = cmdCpssRxPkGetFirst(&netRxExtLastIndex,
                                 packetBuff, &buffLen, &packetActualLength,
                                 &devNum,&queue,
                                 &rxParam);
    }
    else
    {
        rc = cmdCpssRxPkGetNext(&netRxExtLastIndex,
                                 packetBuff, &buffLen, &packetActualLength,
                                 &devNum,&queue,
                                 &rxParam);
    }

    if (rc != GT_OK)
    {
        if (rc == GT_NO_MORE)
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
        }
        else
        {
            galtisOutput(outArgs, rc, "");
        }

        return CMD_OK;
    }


    commonParamsPtr = &rxParam.dsaParam.commonParams;
    toCpuPtr        = &rxParam.dsaParam.dsaInfo.toCpu;
    forwardPtr      = &rxParam.dsaParam.dsaInfo.forward;
    toAnalyzerPtr   = &rxParam.dsaParam.dsaInfo.toAnalyzer;

    byteArry.data    = packetBuff;
    byteArry.length  = buffLen;

    /* initialize the index of "inFields" parameters */
    ii = 0;

    inFields[ii++] = devNum;                                      /*0*/
    inFields[ii++] = queue;                                       /*1*/

    ii++;/* one for the packetString */
    packetString = galtisBArrayOut(&byteArry);                    /*2*/

    inFields[ii++] = packetActualLength;                          /*3*/

    if (version > 0)
    {
        inFields[ii++] = commonParamsPtr->dsaTagType;             /*4*/
    }
    else
    {
    /* the Galtis table does not supports 3/4 words DSA tag. */
    inFields[ii++] = ((int)(commonParamsPtr->dsaTagType) > (int)CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E)
                       ? CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E :
                         commonParamsPtr->dsaTagType;                 /*4*/
    }
    inFields[ii++] = commonParamsPtr->vpt;                        /*5*/
    inFields[ii++] = commonParamsPtr->cfiBit;                     /*6*/
    inFields[ii++] = commonParamsPtr->vid;                        /*7*/

    switch (rxParam.dsaParam.dsaType)
    {
    case CPSS_DXCH_NET_DSA_CMD_TO_CPU_E:
            __HwDev  = toCpuPtr->hwDevNum;
            if (toCpuPtr->srcIsTrunk == GT_FALSE)
            {
                __Port = (GT_PORT_NUM)toCpuPtr->interface.portNum;
                CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
                portOrTrunkId = __Port;
            }
            else
            {
                /* TRUNK case, convert device with dummy __Port */
                __Port = 0;
                CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
                __trunkId = toCpuPtr->interface.srcTrunkId;
                CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(__trunkId);
                portOrTrunkId = __trunkId;
            }
            inFields[ii++] = toCpuPtr->isEgressPipe;              /*8*/
            inFields[ii++] = toCpuPtr->isTagged;                  /*9*/
            inFields[ii++] = __HwDev;                               /*10*/
            inFields[ii++] = toCpuPtr->srcIsTrunk;                /*11*/
            inFields[ii++] = portOrTrunkId;                       /*12*/
            inFields[ii++] = toCpuPtr->cpuCode;                   /*13*/
            inFields[ii++] = toCpuPtr->wasTruncated;              /*14*/
            inFields[ii++] = toCpuPtr->originByteCount;           /*15*/
            break;
        case CPSS_DXCH_NET_DSA_CMD_FORWARD_E:
            __HwDev  = forwardPtr->srcHwDev;
            if (forwardPtr->srcIsTrunk == GT_FALSE)
            {
                __Port = forwardPtr->source.portNum;
                CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
                portOrTrunkId = __Port;
            }
            else
            {
                /* TRUNK case, convert device with dummy __Port */
                __Port = 0;
                CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
                __trunkId = forwardPtr->source.trunkId;
                CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(__trunkId);
                portOrTrunkId = __trunkId;
            }
            inFields[ii++] = 0;                                   /*8*/
            inFields[ii++] = forwardPtr->srcIsTagged;             /*9*/
            inFields[ii++] = __HwDev;                               /*10*/
            inFields[ii++] = forwardPtr->srcIsTrunk;              /*11*/
            inFields[ii++] = portOrTrunkId;                       /*12*/
            inFields[ii++] = 0;                                   /*13*/
            inFields[ii++] = 0;                                   /*14*/
            inFields[ii++] = 0;                                   /*15*/
            break;
        case CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E:
            __HwDev  = toAnalyzerPtr->devPort.hwDevNum;
            __Port = toAnalyzerPtr->devPort.portNum;
            CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
            inFields[ii++] = 0;                                   /*8*/
            inFields[ii++] = toAnalyzerPtr->isTagged;             /*9*/
            inFields[ii++] = __HwDev;                               /*10*/
            inFields[ii++] = 0;                                   /*11*/
            inFields[ii++] = __Port;                              /*12*/
            inFields[ii++] = 0;                                   /*13*/
            inFields[ii++] = 0;                                   /*14*/
            inFields[ii++] = 0;                                   /*15*/
            break;
        default:
            inFields[ii++] = 0;                                   /*8*/
            inFields[ii++] = 0;                                   /*9*/
            inFields[ii++] = 0;                                   /*10*/
            inFields[ii++] = 0;                                   /*11*/
            inFields[ii++] = 0;                                   /*12*/
            inFields[ii++] = 0;                                   /*13*/
            inFields[ii++] = 0;                                   /*14*/
            inFields[ii++] = 0;                                   /*15*/
            break;
    }

    if (version > 0)
    {
        /* assign common fields */
        ii = 22;
        inFields[ii++] = commonParamsPtr->dropOnSource;
        inFields[ii]   = commonParamsPtr->packetIsLooped;

        /* assign value for additional fields */
        ii = 16;
        if (rxParam.dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
        {
            inFields[ii++] = toCpuPtr->interface.ePort;
            inFields[ii++] = toCpuPtr->timestamp;
            inFields[ii++] = toCpuPtr->packetIsTT;
            inFields[ii++] = toCpuPtr->flowIdTtOffset.flowId;
            inFields[ii++] = toCpuPtr->flowIdTtOffset.ttOffset;
            inFields[ii++] = toCpuPtr->tag0TpidIndex;
        }
        else
        {
            inFields[ii++] = 0;
            inFields[ii++] = 0;
            inFields[ii++] = 0;
            inFields[ii++] = 0;
            inFields[ii++] = 0;
            inFields[ii++] = 0;
        }
    }

    /* pack and output table fields */
    /*            0 1 2 3 4 5 6 7 8 9 */
    /*                                0 1 2 3 4 5*/
    fieldOutput("%d%d%s%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],
                inFields[1],
                packetString,
                inFields[3],
                inFields[4],
                inFields[5],
                inFields[6],
                inFields[7],
                inFields[8],
                inFields[9],
                inFields[10],
                inFields[11],
                inFields[12],
                inFields[13],
                inFields[14],
                inFields[15]
    );

    if (version > 0)
    {
        fieldOutputSetAppendMode();
        fieldOutput("%d%d%d%d%d%d%d%d",
            inFields[16], inFields[17], inFields[18], inFields[19],
            inFields[20], inFields[21], inFields[22], inFields[23]);

    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCmdDxChRxStartCapture function
* @endinternal
*
* @brief   Start collecting the received packets
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdDxChRxStartCapture
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call tapi api function */
    result = cmdCpssRxStartCapture();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCmdDxChRxStopCapture function
* @endinternal
*
* @brief   Stop collecting the received packets
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdDxChRxStopCapture
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call tapi api function */
    result = cmdCpssRxStopCapture();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCmdDxChRxSetMode function
* @endinternal
*
* @brief   set received packets collection mode and parameters
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdDxChRxSetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GALTIS_RX_MODE_ENT lRxMode;
    GT_U32 lBuferSize;
    GT_U32 lNumOfEnteries;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    lRxMode = (GALTIS_RX_MODE_ENT)inArgs[0];
    lBuferSize = (GT_U32)inArgs[1];
    lNumOfEnteries = (GT_U32)inArgs[2];

    /* call tapi api function */
    result = cmdCpssRxSetMode(lRxMode, lBuferSize, lNumOfEnteries);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCmdDxChRxGetMode function
* @endinternal
*
* @brief   Get received packets collection mode and parameters
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdDxChRxGetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GALTIS_RX_MODE_ENT lMode;
    GT_U32 lBuffSize;
    GT_U32 lNumEntries;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call tapi api function */
    result = cmdCpssRxGetMode(&lMode, &lBuffSize, &lNumEntries);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", lMode, lBuffSize,
        lNumEntries);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfRxPacketTableGetFirst function
* @endinternal
*
* @brief   Get first entry from the DXCH rxNetIf table. (always "TO_CPU" dsa tag !)
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfRxPacketTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChNetIfRxPacketTableGet(inArgs,inFields,numFields,outArgs,
                    GT_FALSE, 0);/* get first */
}


/**
* @internal wrCpssDxChNetIfRxPacketTableGetFirst_2 function
* @endinternal
*
* @brief   Get first entry from the DXCH rxNetIf table. (always "TO_CPU" dsa tag !)
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfRxPacketTableGetFirst_2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChNetIfRxPacketTableGet(inArgs,inFields,numFields,outArgs,
                    GT_FALSE, 1);/* get first */
}

/**
* @internal wrCpssDxChNetIfRxPacketTableGetNext function
* @endinternal
*
* @brief   Get next entry from the DXCH rxNetIf table. (always "TO_CPU" dsa tag !)
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfRxPacketTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChNetIfRxPacketTableGet(inArgs,inFields,numFields,outArgs,
                    GT_TRUE, 0);/* get next */
}


/**
* @internal wrCpssDxChNetIfRxPacketTableGetNext_2 function
* @endinternal
*
* @brief   Get next entry from the DXCH rxNetIf table. (always "TO_CPU" dsa tag !)
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfRxPacketTableGetNext_2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChNetIfRxPacketTableGet(inArgs,inFields,numFields,outArgs,
                    GT_TRUE, 1);/* get next */
}

/**
* @internal wrCpssDxChNetIfRxPacketTableClear function
* @endinternal
*
* @brief   clear all entries from the DXCH rxNetIf table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfRxPacketTableClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdCpssRxPktClearTbl();
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChNetIfFromCpuDpSet function
* @endinternal
*
* @brief   Set DP for from CPU DSA tag packets.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] inArgs[CMD_MAX_ARGS]     - device number
* @param[in] inArgs[CMD_MAX_ARGS]     - drop precedence level
*
* @retval CMD_OK                   - on success
* @retval CMD_AGENT_ERROR          - on failure
*/
static CMD_STATUS wrCpssDxChNetIfFromCpuDpSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    IN GT_U8                devNum;
    IN CPSS_DP_LEVEL_ENT    dpLevel;

    /* Check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dpLevel = (CPSS_DP_LEVEL_ENT)inArgs[1];

    /* call cpss API function */
    result = cpssDxChNetIfFromCpuDpSet(devNum, dpLevel);

    /* pack output arguments to GalTis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfFromCpuDpGet function
* @endinternal
*
* @brief   Get DP for from CPU DSA tag packets.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] inArgs[CMD_MAX_ARGS]     - device number
*
* @param[out] outArgs[CMD_MAX_BUFFER]  - pointer to drop precedence level
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - invalid hardware value for drop precedence level
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChNetIfFromCpuDpGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS          result;

    GT_U8              devNum;
    CPSS_DP_LEVEL_ENT  dpLevelPtr;

    /* Check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];

    /* map input arguments to locals */
    result = cpssDxChNetIfFromCpuDpGet(devNum, &dpLevelPtr);

    /* pack output arguments to GalTis string */
    galtisOutput(outArgs, result, "%d", dpLevelPtr);

    return CMD_OK;
}


/**
* @internal wrCpssDxChNetIfSdmaRxResourceErrorModeSet function
* @endinternal
*
* @brief   Set a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      All DxCh devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
*/
static CMD_STATUS wrCpssDxChNetIfSdmaRxResourceErrorModeSet
(

    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    IN  GT_U8                                   devNum;
    IN  GT_U8                                   queue;
    IN  CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT   mode;

    /* Check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue  = (GT_U8)inArgs[1];
    mode   = (CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT)inArgs[2];

    /* call cpss API function */
    result = cpssDxChNetIfSdmaRxResourceErrorModeSet(devNum, queue, mode);

    /* pack output arguments to GalTis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaRxResourceErrorModeGet function
* @endinternal
*
* @brief   Get a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_BAD_PTR               - on NULL ptr
*/
CMD_STATUS wrCpssDxChNetIfSdmaRxResourceErrorModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                   devNum;
    GT_U8                                   queue;
    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssDxChNetIfSdmaRxResourceErrorModeGet(devNum, queue, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfTxGeneratorSet function
* @endinternal
*
* @brief   Creates new transmit parameters description entry for traffic generator.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxGeneratorSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   status;                 /* Function return status */
    DXCH_PKT_GEN_DESC_STC       netTxGenPacketDesc;     /* TX packet generator structure */
    DXCH_PKT_DESC_STC         * netTxPacketDescPtr;     /* Pointer to common network TX packe structure */
    GT_U32                      entryIndex = 0;         /* Entry index */
    GT_U8                       devNum;                 /* Device number */
    GT_U8                       txQueue;                /* Tx queue */
    GT_U32                      portGroup;              /* Port group ID */
    GT_PORT_GROUPS_BMP          portGroupBmp;           /* Port group bitmap */
    CPSS_DXCH_NET_TX_PARAMS_STC cpssNetTxPacketDesc;    /* TX packet generator structure */
    GT_U32                      packetId;               /* Packet identification number */

    /* Check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;


    cmdOsMemSet(&netTxGenPacketDesc, 0, sizeof(DXCH_PKT_GEN_DESC_STC));

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    portGroupBmp = (1 << portGroup);

    /* Create/update TX packet generator descriptor entry */
    status = cmdCpssDxChNetIfTxGenTblEntrySet(inArgs, inFields,
                                              numFields, &netTxGenPacketDesc);
    if(status != CMD_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    netTxPacketDescPtr = 0;

    for(entryIndex = 0; entryIndex < wrCpssDxChNetIfTxDxChTblCapacity; entryIndex++)
    {
        if(cpssDxChTxPacketDescTbl[entryIndex].entryId ==
           netTxGenPacketDesc.packetGenDescIndex)
        {
            /* Retrieve TX packet descriptor entry by index */
            netTxPacketDescPtr = &cpssDxChTxPacketDescTbl[entryIndex];
            break;
        }
    }

    if(netTxPacketDescPtr == 0)
    {
        /* Entry not found */
        galtisOutput(outArgs, GT_NOT_FOUND, "");
        return CMD_OK;
    }

    /* Assign TX generator queue number */
    netTxPacketDescPtr->sdmaInfo.txQueue = txQueue;

    /* Convert fields to CPSS_DXCH_NET_TX_PARAMS_STC */
    cmdCpssDxChConvertPacketDesc(netTxPacketDescPtr, &cpssNetTxPacketDesc);

    /* Get packet ID */
    packetId = netTxGenPacketDesc.packetId;

    if (netTxGenPacketDesc.actionType == DXCH_TX_GEN_ACTION_ADD_E)
    {
        /* Add TX generator packet */
        status = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum, portGroupBmp,
                                            &cpssNetTxPacketDesc,
                                            netTxPacketDescPtr->pcktData[0],
                                            netTxPacketDescPtr->pcktDataLen[0],
                                            &packetId);
    }
    else
    {
        /* Update TX generator packet */
        status = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(devNum, portGroupBmp,
                                            packetId,
                                            &cpssNetTxPacketDesc,
                                            netTxPacketDescPtr->pcktData[0],
                                            netTxPacketDescPtr->pcktDataLen[0]);

    }
    if(status != GT_OK)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    /* Set packet ID */
    netTxGenPacketDesc.packetId = packetId;

    if(status != GT_OK)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }


    /* Insert into table */
    status = cmdCpssDxChTxGenPacketDescSet(&netTxGenPacketDesc,
                                           &cpssDxChTxGenPacketDescTbl[portGroup][txQueue],
                                           &wrCpssDxChNetIfTxGenDxChTblCapacity[portGroup][txQueue],
                                           &entryIndex);

    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

static GT_U32 txGenPacketGenIndex = 0;
/**
* @internal wrCpssDxChNetIfTxGeneratorGetFirst function
* @endinternal
*
* @brief   Get first entry from the TX packet generator table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxGeneratorGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                 status;                 /* Function return status */
    DXCH_PKT_GEN_DESC_STC     packetGenDesc;          /* Packet generator descriptor */
    GT_U32                    txQueue;                /* Tx queue */
    GT_U32                    portGroup;              /* Port group ID */

    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    txGenPacketGenIndex = 0;

    status = cmdCpssDxChTxGenPacketDescGet(0,
                               cpssDxChTxGenPacketDescTbl[portGroup][txQueue],
                               wrCpssDxChNetIfTxGenDxChTblCapacity[portGroup][txQueue],
                               &packetGenDesc);
    if(status != GT_OK)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    inFields[0] = packetGenDesc.actionType;
    inFields[1] =
        (packetGenDesc.dsaType == CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E) ? 0 : 1;
    inFields[2] = packetGenDesc.packetGenDescIndex;
    inFields[3] = packetGenDesc.packetId;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfTxGeneratorGetNext function
* @endinternal
*
* @brief   Get first entry from the TX packet generator table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxGeneratorGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;                 /* Function return status */
    DXCH_PKT_GEN_DESC_STC   packetGenDesc;          /* Packet generator descriptor */
    GT_U32                  txQueue;                /* Tx queue */
    GT_U32                  portGroup;              /* Port group ID */

    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    txGenPacketGenIndex++;
    if (txGenPacketGenIndex >= wrCpssDxChNetIfTxGenDxChTblCapacity[portGroup][txQueue])
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    result = cmdCpssDxChTxGenPacketDescGet(txGenPacketGenIndex,
                               cpssDxChTxGenPacketDescTbl[portGroup][txQueue],
                               wrCpssDxChNetIfTxGenDxChTblCapacity[portGroup][txQueue],
                               &packetGenDesc);
    if(result != GT_OK)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = packetGenDesc.actionType;
    inFields[1] =
            (packetGenDesc.dsaType == CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E) ? 0 : 1;
    inFields[2] = packetGenDesc.packetGenDescIndex;
    inFields[3] = packetGenDesc.packetId;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfTxGeneratorDelete function
* @endinternal
*
* @brief   Delete transmit parameters description entry for traffic generator.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxGeneratorDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;
    GT_U8                     devNum;                 /* Device number */
    GT_U8                     txQueue;                /* Tx queue */
    GT_U32                    portGroup;              /* Port group ID */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    /* Delete packet descriptor data */
    status = cmdCpssDxChTxGenPacketDescDelete(devNum, txQueue,
                                              portGroup, (GT_U32)inFields[3],
                                              &cpssDxChTxGenPacketDescTbl[portGroup][txQueue],
                                              &wrCpssDxChNetIfTxGenDxChTblCapacity[portGroup][txQueue]);

    galtisOutput(outArgs, status, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfTxGeneratorClear function
* @endinternal
*
* @brief   Delete all transmit parameters description data for traffic generator.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfTxGeneratorClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               status;
    GT_U8                   devNum;                 /* Device number */
    GT_U8                   txQueue;                /* Tx queue */
    GT_U32                  portGroup;              /* Port group ID */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    status = cmdCpssDxChTxGenPacketDescClearAll(devNum, txQueue, portGroup,
                                &cpssDxChTxGenPacketDescTbl[portGroup][txQueue]);

    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaTxGeneratorEnable function
* @endinternal
*
* @brief   Enable selected Tx SDMA Generator.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaTxGeneratorEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;                   /* Device number */
    GT_U8     txQueue;                  /* Tx queue */
    GT_U32    portGroup;                /* Port group ID */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* Port group bitmap */
    GT_BOOL   burstEnable;              /* Packet generation mode */
    GT_U32    burstPacketsNumber;       /* Number of packets in burst */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    portGroupsBmp = 1 << portGroup;
    burstPacketsNumber = (GT_U32)inArgs[3];
    burstEnable        = (GT_BOOL)inArgs[4];

    /* Call CPSS API function */
    result = cpssDxChNetIfSdmaTxGeneratorEnable(devNum, portGroupsBmp, txQueue,
                                                burstEnable, burstPacketsNumber);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaTxGeneratorDisable function
* @endinternal
*
* @brief   Disable selected Tx SDMA Generator.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaTxGeneratorDisable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;                   /* Device number */
    GT_U8     txQueue;                  /* Tx queue */
    GT_U32    portGroup;                /* Port group ID */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* Port group bitmap */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    portGroupsBmp = 1 << portGroup;

    /* Call CPSS API function */
    result = cpssDxChNetIfSdmaTxGeneratorDisable(devNum, portGroupsBmp, txQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaTxGeneratorRateSet function
* @endinternal
*
* @brief   Set Tx SDMA Generator transmission packet rate
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaTxGeneratorRateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;                   /* Device number */
    GT_U8     txQueue;                  /* Tx queue */
    GT_U32    portGroup;                /* Port group ID */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* Port group bitmap */
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode;
    GT_U64                                      rateValue;
    GT_U64                                      actualRateValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];
    rateMode = (CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT)inArgs[3];
    rateValue.l[0] = (GT_U32)inArgs[4];
    rateValue.l[1] = (GT_U32)inArgs[5];

    portGroupsBmp = 1 << portGroup;

    /* Call CPSS API function */
    result = cpssDxChNetIfSdmaTxGeneratorRateSet(devNum, portGroupsBmp,
                                                 txQueue, rateMode, rateValue,
                                                 &actualRateValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",
                 actualRateValue.l[0],actualRateValue.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaTxGeneratorRateGet function
* @endinternal
*
* @brief   Get Tx SDMA Generator transmission packet rate
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaTxGeneratorRateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;                   /* Device number */
    GT_U8     txQueue;                  /* Tx queue */
    GT_U32    portGroup;                /* Port group ID */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* Port group bitmap */
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode;
    GT_U64                                      rateValue;
    GT_U64                                      actualRateValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    portGroupsBmp = 1 << portGroup;

    /* Call CPSS API function */
    result = cpssDxChNetIfSdmaTxGeneratorRateGet(devNum, portGroupsBmp,
                                                 txQueue,
                                                 &rateMode,
                                                 &rateValue,
                                                 &actualRateValue);

    /* Pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d",
                 rateMode,
                 rateValue.l[0],rateValue.l[1],
                 actualRateValue.l[0],actualRateValue.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfSdmaTxGeneratorBurstStatusGet function
* @endinternal
*
* @brief   Get Tx SDMA Generator burst counter status
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssDxChNetIfSdmaTxGeneratorBurstStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;                   /* Device number */
    GT_U8     txQueue;                  /* Tx queue */
    GT_U32    portGroup;                /* Port group ID */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* Port group bitmap */
    CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT burstStatus;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    txQueue = (GT_U8)inArgs[2];

    portGroupsBmp = 1 << portGroup;

    /* Call CPSS API function */
    result = cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(devNum, portGroupsBmp,
                                                        txQueue, &burstStatus);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", burstStatus);

    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeToPhysicalPortSet function
* @endinternal
*
* @brief   CPU code to physical port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpu code or bad physical port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChNetIfCpuCodeToPhysicalPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_NET_RX_CPU_CODE_ENT   cpuCode;
    GT_PHYSICAL_PORT_NUM         portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call api function */
    result = cpssDxChNetIfCpuCodeToPhysicalPortSet(devNum,cpuCode,portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChNetIfCpuCodeToPhysicalPortGet function
* @endinternal
*
* @brief   Get the CPU code to physical port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpu code or bad physical port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on Illegal pointer value
*/
CMD_STATUS wrCpssDxChNetIfCpuCodeToPhysicalPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    CPSS_NET_RX_CPU_CODE_ENT   cpuCode;
    GT_PHYSICAL_PORT_NUM       portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];

    /* call port group api function */
    result = cpssDxChNetIfCpuCodeToPhysicalPortGet(devNum,cpuCode,&portNum);

    /* Override Device and Port */
    CONVERT_BACK_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portNum);
    return CMD_OK;
}

/**
* @internal cpssDxChNetIfSdmaPhysicalPortToQueueGet function
* @endinternal
*
* @brief  Get the 'first' queue of the CPU SDMA port.
*         NOTE: the CPU SDMA hold 8 queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*                                  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - Device number.
* @param[in] portNum               - The physical port number of one of the CPU SDMA ports
* @param[out] firstQueuePtr        - (pointer to) the 'first' queue of the CPU SDMA port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad physical port
* @retval GT_BAD_STATE             - on physical port that is not 'CPU SDMA' port.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNetIfSdmaPhysicalPortToQueueGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U8  firstQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call port group api function */
    result = cpssDxChNetIfSdmaPhysicalPortToQueueGet(devNum,portNum,&firstQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", firstQueue);
    return CMD_OK;
}

/**
* @internal cpssDxChNetIfSdmaQueueToPhysicalPortGet function
* @endinternal
*
* @brief  Get the CPU SDMA physical port , that serves the SDMA queue index
*         NOTE: the CPU SDMA hold 8 queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*                                  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - Device number.
* @param[in] queueIdx              - the global queue index (one of 8) of the CPU SDMA port.
* @param[out] portNumPtr           - (pointer to) The physical port number of the CPU SDMA port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad queue index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChNetIfSdmaQueueToPhysicalPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8  devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U8  queue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue  = (GT_U8)inArgs[1];

    /* call port group api function */
    result = cpssDxChNetIfSdmaQueueToPhysicalPortGet(devNum,queue,&portNum);

    /* Override Device and Port */
    CONVERT_BACK_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portNum);
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChNetIfInit",
        &wrCpssDxChNetIfInit,
        1, 0},

    {"cpssDxChNetIfRemove",
        &wrCpssDxChNetIfRemove,
        1, 0},

    {"cpssDxChNetIfPrePendTwoBytesHeaderSet",
        &wrCpssDxChNetIfPrePendTwoBytesHeaderSet,
        2, 0},

    {"cpssDxChNetIfPrePendTwoBytesHeaderGet",
        &wrCpssDxChNetIfPrePendTwoBytesHeaderGet,
        1, 0},

    {"cpssDxChNetIfSdmaRxCountersGetFirst",
        &wrCpssDxChNetIfSdmaRxCountersGetFirst,
        1, 0},

    {"cpssDxChNetIfSdmaRxCountersGetNext",
        &wrCpssDxChNetIfSdmaRxCountersGetNext,
        1, 0},

    {"cpssDxChNetIfSdmaRxErrorCountGetFirst",
        &wrCpssDxChNetIfSdmaRxErrorCountGetFirst,
        1, 0},

    {"cpssDxChNetIfSdmaRxErrorCountGetNext",
        &wrCpssDxChNetIfSdmaRxErrorCountGetNext,
        1, 0},

    {"cpssDxChNetIfSdmaRxQueueEnable",
        &wrCpssDxChNetIfSdmaRxQueueEnable,
        3, 0},

    {"cpssDxChNetIfSdmaRxQueueEnableGet",
        &wrCpssDxChNetIfSdmaRxQueueEnableGet,
        2, 0},

    {"cpssDxChNetIfSdmaTxQueueEnable",
        &wrCpssDxChNetIfSdmaTxQueueEnable,
        3, 0},

    {"cpssDxChNetIfSdmaTxQueueEnableGet",
        &wrCpssDxChNetIfSdmaTxQueueEnableGet,
        2, 0},

    {"cpssDxChNetIfDuplicateEnableSet",
        &wrCpssDxChNetIfDuplicateEnableSet,
        2, 0},

    {"cpssDxChNetIfDuplicateEnableGet",
        &wrCpssDxChNetIfDuplicateEnableGet,
        1, 0},

    {"cpssDxChNetIfPortDuplicateToCpuSet",
        &wrCpssDxChNetIfPortDuplicateToCpuSet,
        3, 0},

    {"cpssDxChNetIfPortDuplicateToCpuGet",
        &wrCpssDxChNetIfPortDuplicateToCpuGet,
        2, 0},

    {"cpssDxChNetIfAppSpecificCpuCodeTcpSynSet",
        &wrCpssDxChNetIfAppSpecificCpuCodeTcpSynSet,
        2, 0},

    {"cpssDxChNetIfAppSpecificCpuCodeTcpSynGet",
        &wrCpssDxChNetIfAppSpecificCpuCodeTcpSynGet,
        1, 0},

    {"cpssDxChNetIfAppSpcfcCpuCodeIpPrtclSet",
        &wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolSet,
        1, 4},

    {"cpssDxChNetIfAppSpcfcCpuCodeIpPrtclInvalidate",
        &wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate,
        2, 0},

    {"cpssDxChNetIfAppSpcfcCpuCodeIpPrtclGetFirst",
        &wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolGetFirst,
        1, 0},

    {"cpssDxChNetIfAppSpcfcCpuCodeIpPrtclGetNext",
        &wrCpssDxChNetIfAppSpecificCpuCodeIpProtocolGetNext,
        1, 0},

    {"cpssDxChNetIfAppSpcfcCpuCodeDstPrtRngSet",
        &wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet,
        1, 7},

    {"cpssDxChNetIfAppSpcfcCpuCodeDstPrtRngInvlidt",
        &wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate,
        2, 0},

    {"cpssDxChNetIfAppSpcfcCpuCodeDstPrtRngGetFirst",
        &wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGetFirst,
        1, 0},

    {"cpssDxChNetIfAppSpcfcCpuCodeDstPrtRngGetNext",
        &wrCpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGetNext,
        1, 0},

    {"cpssDxChNetIfCpuCodeIpLinkLocalProtSet",
        &wrCpssDxChNetIfCpuCodeIpLinkLocalProtSet,
        4, 0},

    {"cpssDxChNetIfCpuCodeIpLinkLocalProtGet",
        &wrCpssDxChNetIfCpuCodeIpLinkLocalProtGet,
        3, 0},

    {"cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet",
        &wrCpssDxChNetIfCpuCodeIeeeReservedMcastProtSet,
        3, 0},

    {"cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet",
        &wrCpssDxChNetIfCpuCodeIeeeReservedMcastProtGet,
        2, 0},

    {"cpssDxChNetIfCpuCodeSet",
        &wrCpssDxChNetIfCpuCodeTableSet,
        1, 8},

    {"cpssDxChNetIfCpuCodeGetFirst",
        &wrCpssDxChNetIfCpuCodeTableGetFirst,
        1, 0},

    {"cpssDxChNetIfCpuCodeGetNext",
        &wrCpssDxChNetIfCpuCodeTableGetNext,
        1, 0},

    {"cpssDxChNetIfCpuCodeSttstclRateLimitsSet",
        &wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet,
        1, 2},

    {"cpssDxChNetIfCpuCodeSttstclRateLimitsGetFirst",
        &wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableGetFirst,
        1, 0},

    {"cpssDxChNetIfCpuCodeSttstclRateLimitsGetNext",
        &wrCpssDxChNetIfCpuCodeStatisticalRateLimitsTableGetNext,
        1, 0},

    {"cpssDxChNetIfCpuCodeDesignatedDeviceSet",
        &wrCpssDxChNetIfCpuCodeDesignatedDeviceTableSet,
        1, 2},

    {"cpssDxChNetIfCpuCodeDesignatedDeviceGetFirst",
        &wrCpssDxChNetIfCpuCodeDesignatedDeviceTableGetFirst,
        1, 0},

    {"cpssDxChNetIfCpuCodeDesignatedDeviceGetNext",
        &wrCpssDxChNetIfCpuCodeDesignatedDeviceTableGetNext,
        1, 0},

    {"cpssDxChNetIfCpuCodeRateLimiterSet",
        &wrCpssDxChNetIfCpuCodeRateLimiterTableSet,
        1, 3},

    {"cpssDxChNetIfCpuCodeRateLimiterGetFirst",
        &wrCpssDxChNetIfCpuCodeRateLimiterTableGetFirst,
        1, 0},

    {"cpssDxChNetIfCpuCodeRateLimiterGetNext",
        &wrCpssDxChNetIfCpuCodeRateLimiterTableGetNext,
        1, 0},

    {"cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet",
        &wrCpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet,
        2, 0},

    {"cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet",
        &wrCpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet,
        1, 0},

    {"cpssDxChNetIfTxFromCpuSet",
        &wrCpssDxChNetIfSetTxNetIfTblEntryFromCpu,
        2, 32},

    {"cpssDxChNetIfTxFromCpuGetFirst",
        &wrCpssDxChNetIfGetTxNetIfTblFirstEntryFromCpu,
        2, 0},

    {"cpssDxChNetIfTxFromCpuGetNext",
        &wrCpssDxChNetIfGetTxNetIfTblNextEntryFromCpu,
        2, 0},

    {"cpssDxChNetIfTxFromCpu_1Set",
        &wrCpssDxChNetIfSetTxNetIfTblEntryFromCpu,
        2, 39},

    {"cpssDxChNetIfTxFromCpu_1GetFirst",
        &wrCpssDxChNetIfGetTxNetIfTblFirstEntryFromCpu_1,
        2, 0},

    {"cpssDxChNetIfTxFromCpu_1GetNext",
        &wrCpssDxChNetIfGetTxNetIfTblNextEntryFromCpu_1,
        2, 0},

    {"cpssDxChNetIfTxForwardSet",
        &wrCpssDxChNetIfSetTxNetIfTblEntryForward,
        2, 27},

    {"cpssDxChNetIfTxForwardGetFirst",
        &wrCpssDxChNetIfGetTxNetIfTblFirstEntryForward,
        2, 0},

    {"cpssDxChNetIfTxForwardGetNext",
        &wrCpssDxChNetIfGetTxNetIfTblNextEntryForward,
        2, 0},
      {"cpssDxChNetIfTxForward_1Set",
        &wrCpssDxChNetIfSetTxNetIfTblEntryForward,
        2, 35},

    {"cpssDxChNetIfTxForward_1GetFirst",
        &wrCpssDxChNetIfGetTxNetIfTblFirstEntryForward_1,
        2, 0},

    {"cpssDxChNetIfTxForward_1GetNext",
        &wrCpssDxChNetIfGetTxNetIfTblNextEntryForward_1,
        2, 0},

    {"cpssDxChNetIfTxForward_1Delete",
        &wrCpssDxChNetIfDeleteTxNetIfTblEntryForward,
        2, 27},

    {"cpssDxChNetIfTxForward_1Clear",
        &wrCpssDxChNetIfClearTxNetIfTableForward,
        2, 0},

    {"cpssDxChNetIfTxFromCpuDelete",
        &wrCpssDxChNetIfDeleteTxNetIfTblEntryFromCpu,
        2, 32},

    {"cpssDxChNetIfTxFromCpu_1Delete",
        &wrCpssDxChNetIfDeleteTxNetIfTblEntryFromCpu,
        2, 39},

    {"cpssDxChNetIfTxForwardDelete",
        &wrCpssDxChNetIfDeleteTxNetIfTblEntryForward,
        2, 27},

    {"cpssDxChNetIfTxFromCpuClear",
        &wrCpssDxChNetIfClearTxNetIfTableFromCpu,
        2, 0},

    {"cpssDxChNetIfTxFromCpu_1Clear",
        &wrCpssDxChNetIfClearTxNetIfTableFromCpu,
        2, 0},

    {"cpssDxChNetIfTxForwardClear",
        &wrCpssDxChNetIfClearTxNetIfTableForward,
        2, 0},

    {"cpssDxChTxSetMode",
        &wrCpssDxChNetIfTxSetMode,
        2, 0},

    {"cpssDxChTxGetMode",
        &wrCpssDxChNetIfTxGetMode,
        0, 0},

    {"cpssDxChTxStart",
     &wrCpssDxChNetIfTxStart,
     0, 0},

    {"cpssDxChTxStop",
     &wrCpssDxChNetIfTxStop,
     0, 0},

    /* Rx packets table -- start */
    {"cmdDxChRxStartCapture",
        &wrCmdDxChRxStartCapture,
        0, 0},

    {"cmdDxChRxStopCapture",
        &wrCmdDxChRxStopCapture,
        0, 0},

    {"cmdDxChRxSetMode",
        &wrCmdDxChRxSetMode,
        3, 0},

    {"cmdDxChRxGetMode",
        &wrCmdDxChRxGetMode,
        0, 0},

    {"cpssDxChNetIfRxPacketTableGetFirst",/* get first */
        &wrCpssDxChNetIfRxPacketTableGetFirst,
        0, 0},

    {"cpssDxChNetIfRxPacketTableGetNext",
        &wrCpssDxChNetIfRxPacketTableGetNext,/* get next */
        0, 0},

    {"cpssDxChNetIfRxPacketTableClear",
        &wrCpssDxChNetIfRxPacketTableClear,/* clear */
        0, 0},
    /* Rx packets table -- End */

    {"cpssDxChNetIfFromCpuDpSet",
        &wrCpssDxChNetIfFromCpuDpSet,
        2, 0},

    {"cpssDxChNetIfFromCpuDpGet",
        &wrCpssDxChNetIfFromCpuDpGet,
        1, 0},

    {"cpssDxChNetIfRxPacketTable_1GetFirst",/* get first */
        &wrCpssDxChNetIfRxPacketTableGetFirst,
        0, 0},

    {"cpssDxChNetIfRxPacketTable_1GetNext",
        &wrCpssDxChNetIfRxPacketTableGetNext,/* get next */
        0, 0},

    {"cpssDxChNetIfRxPacketTable_1Clear",
        &wrCpssDxChNetIfRxPacketTableClear,/* clear */
        0, 0},
    /* Rx packets table -- End */

    {"cpssDxChNetIfRxPacketTable_2GetFirst",/* get first */
        &wrCpssDxChNetIfRxPacketTableGetFirst_2,
        0, 0},

    {"cpssDxChNetIfRxPacketTable_2GetNext",
        &wrCpssDxChNetIfRxPacketTableGetNext_2,/* get next */
        0, 0},

    {"cpssDxChNetIfRxPacketTable_2Clear",
        &wrCpssDxChNetIfRxPacketTableClear,/* clear like other tables */
        0, 0},
    /* Rx packets table 2 -- End */

    {"cpssDxChNetIfRxPacketTable_3GetFirst",/* get first */
        &wrCpssDxChNetIfRxPacketTableGetFirst_2,
        0, 0},

    {"cpssDxChNetIfRxPacketTable_3GetNext",
        &wrCpssDxChNetIfRxPacketTableGetNext_2,/* get next */
        0, 0},

    {"cpssDxChNetIfRxPacketTable_3Clear",
        &wrCpssDxChNetIfRxPacketTableClear,/* clear like other tables */
        0, 0},
    /* Rx packets table 3 -- End */

    {"cpssDxChNetIfSdmaRxResourceErrorModeSet",
        &wrCpssDxChNetIfSdmaRxResourceErrorModeSet,
        3, 0},

    {"cpssDxChNetIfSdmaRxResourceErrorModeGet",
        &wrCpssDxChNetIfSdmaRxResourceErrorModeGet,
        2, 0},

    {"cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet",
        &wrCpssDxChNetIfCpuCodeRateLimiterPacketCntrGet,
        2, 0},

    {"cpssDxChNetIfCpuCodeRateLimiterDropCntrSet",
        &wrCpssDxChNetIfCpuCodeRateLimiterDropCntrSet,
        2, 0},

    {"cpssDxChNetIfCpuCodeRateLimiterDropCntrGet",
        &wrCpssDxChNetIfCpuCodeRateLimiterDropCntrGet,
        1, 0},

    {"cpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet",
        &wrCpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet,
        2, 0},

    {"cpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet",
        &wrCpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet,
        1, 0},

    {"cpssDxChNetIfTxGeneratorSet",
        &wrCpssDxChNetIfTxGeneratorSet,
        3, 4},

    {"cpssDxChNetIfTxGeneratorGetFirst",
        &wrCpssDxChNetIfTxGeneratorGetFirst,
        3, 0},

    {"cpssDxChNetIfTxGeneratorGetNext",
        &wrCpssDxChNetIfTxGeneratorGetNext,
        3, 0},

    {"cpssDxChNetIfTxGeneratorDelete",
        &wrCpssDxChNetIfTxGeneratorDelete,
        3, 0},

    {"cpssDxChNetIfTxGeneratorClear",
        &wrCpssDxChNetIfTxGeneratorClear,
        3, 0},

    {"cpssDxChNetIfSdmaTxGeneratorEnable",
        &wrCpssDxChNetIfSdmaTxGeneratorEnable,
        5, 0},

    {"cpssDxChNetIfSdmaTxGeneratorDisable",
        &wrCpssDxChNetIfSdmaTxGeneratorDisable,
        3, 0},

    {"cpssDxChNetIfSdmaTxGeneratorRateSet",
        &wrCpssDxChNetIfSdmaTxGeneratorRateSet,
        6, 0},

    {"cpssDxChNetIfSdmaTxGeneratorRateGet",
        &wrCpssDxChNetIfSdmaTxGeneratorRateGet,
        3, 0},

    {"cpssDxChNetIfSdmaTxGeneratorBurstStatusGet",
        &wrCpssDxChNetIfSdmaTxGeneratorBurstStatusGet,
        3, 0},
    {"cpssDxChNetIfCpuCodeToPhysicalPortGet",
        &wrCpssDxChNetIfCpuCodeToPhysicalPortGet,
        2, 0},
    {"cpssDxChNetIfCpuCodeToPhysicalPortSet",
        &wrCpssDxChNetIfCpuCodeToPhysicalPortSet,
        3, 0},

    {"cpssDxChNetIfSdmaPhysicalPortToQueueGet",
        &wrCpssDxChNetIfSdmaPhysicalPortToQueueGet,
        2, 0},

    {"cpssDxChNetIfSdmaQueueToPhysicalPortGet",
        &wrCpssDxChNetIfSdmaQueueToPhysicalPortGet,
        2, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChNetIf function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssDxChNetIf
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

/**
* @internal cmdCpssDxChNetIfTxGenTblEntrySet function
* @endinternal
*
* @brief   Add/update transmit parameters description entry for Tx queue generator.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note Use existing descriptors DB for FORWARD and FROM_CPU TX Packets for traffic generator.
*
*/
static CMD_STATUS cmdCpssDxChNetIfTxGenTblEntrySet
(
    IN  GT_UINTPTR * inArgs,
    IN  GT_UINTPTR * inFields,
    IN  GT_32 numFields,
    OUT DXCH_PKT_GEN_DESC_STC * netTxPacketGenDescPtr
)
{
    netTxPacketGenDescPtr->actionType =
        (DXCH_TX_GEN_ACTION_TYPE_ENT)inFields[0];

    switch(inFields[1])
    {
        case 0:
            netTxPacketGenDescPtr->dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
            break;
        case 1:
            netTxPacketGenDescPtr->dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    netTxPacketGenDescPtr->packetGenDescIndex = (GT_U32)inFields[2];
    netTxPacketGenDescPtr->packetId = (GT_U32)inFields[3];

    return CMD_OK;
}


