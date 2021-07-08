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
* @file wrapCpssDxChPha.c
*
* @brief Wrapper functions for Cpss Dx PHA APIs
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>


/* get a field from inFields[] , and increment the index for next field */
#define GET_NEXT_FIELD_FROM_GALTIS(fieldName,cast)   \
    {                                                \
        fieldName = (cast)inFields[index++];         \
        if((GT_U32)numFields < index)                \
        {                                            \
            cpssOsPrintf("ERROR : [%s] not hold value in inFields[] \n", \
                #fieldName);                         \
            return CMD_FIELD_UNDERFLOW;              \
        }                                            \
    }

/* get a GT_U32 field from inFields[] , and increment the index for next field */
#define GET_GT_U32_NEXT_FIELD_FROM_GALTIS(fieldName)   \
    GET_NEXT_FIELD_FROM_GALTIS(fieldName,GT_U32)


/* set field of STC according to field by the same name */
#define SET_FIELD_IN_STC(stcPtr,fieldName)   \
    (stcPtr)->fieldName = fieldName


/* set field into 'output' string of galtis */
#define SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(fieldName)     \
    {                                           \
        fieldOutput("%d", fieldName);           \
        fieldOutputSetAppendMode();             \
    }


/**
* @internal cpssDxChPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                      change between heavily processed flows and lightly processed flows
*                                      GT_TRUE  - packet order is not maintained
*                                      GT_FALSE  - packet order is maintained

*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPhaInit

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_BOOL  packetOrderChangeEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    packetOrderChangeEnable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPhaInit(devNum,packetOrderChangeEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal cpssDxChPhaThreadIdEntrySet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[in] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[in] extType                  - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPhaThreadIdEntry_ioam_Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC  *ioamIngPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC  *ioamTransPtr;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;
    GT_U32               tabIndex;/* tabulator index in the Galtis table */

    GT_U32               index = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    /*refresh_curr_phaThreadId     = inArgs[1];*/
    /*refresh_end_phaThreadId      = inArgs[2];*/
    tabIndex  = inArgs[3];

    ioamIngPtr = NULL;
    ioamTransPtr = NULL;
    switch(tabIndex)
    {
        case 0:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E;
            ioamIngPtr = &extInfo.ioamIngressSwitchIpv4;
            break;
        case 1:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E;
            ioamIngPtr = &extInfo.ioamIngressSwitchIpv6;
            break;
        case 2:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E;
            ioamTransPtr =&extInfo.ioamTransitSwitchIpv4;
            break;
        case 3:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E;
            ioamTransPtr =&extInfo.ioamTransitSwitchIpv6;
            break;
        default:
            extType = CPSS_DXCH_PHA_THREAD_TYPE___LAST___E;/* cause error at CPSS */
            break;
    }

    /* start : get the fields from inFields[] */
    GET_GT_U32_NEXT_FIELD_FROM_GALTIS(phaThreadId);

    /* end   : get the fields from inFields[] */

    /* fill the STC info */
    GET_GT_U32_NEXT_FIELD_FROM_GALTIS(commonInfo.statisticalProcessingFactor);
    GET_GT_U32_NEXT_FIELD_FROM_GALTIS(commonInfo.busyStallMode);
    GET_GT_U32_NEXT_FIELD_FROM_GALTIS(commonInfo.stallDropCode);

    /* fill the UNION info */
    if(ioamIngPtr)
    {
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->IOAM_Trace_Type);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Maximum_Length);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Flags);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Hop_Lim);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->node_id);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Type1);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->IOAM_HDR_len1);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Reserved1);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Next_Protocol1);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Type2);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->IOAM_HDR_len2);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Reserved2);
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamIngPtr->Next_Protocol2);
    }
    else
    if(ioamTransPtr)
    {
        GET_GT_U32_NEXT_FIELD_FROM_GALTIS(ioamTransPtr->node_id);
    }
    else
    {
        extInfo.notNeeded = 0;
    }

    /* call cpss api function */
    result = cpssDxChPhaThreadIdEntrySet(devNum,phaThreadId,&commonInfo,extType,&extInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/* current phaThreadId index for the 'refresh' of the table */
static GT_U32   refresh_curr_phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
/* 'last'  phaThreadId index for the 'refresh' of the table */
static GT_U32   refresh_end_phaThreadId  = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;


/**
* @internal cpssDxChPhaThreadIdEntryGet function
* @endinternal
*
* @brief   Get the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[out] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[out] extTypePtr               - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPhaThreadIdEntry_ioam_GetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC  *ioamIngPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC  *ioamTransPtr;

    GT_U8                devNum;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;

    GT_U32               tabIndex;/* tabulator index in the Galtis table */

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];

newEntryStart_lbl:
    if (refresh_curr_phaThreadId > refresh_end_phaThreadId)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    phaThreadId = refresh_curr_phaThreadId++;
    result = cpssDxChPhaThreadIdEntryGet(devNum,phaThreadId,&commonInfo,&extType,&extInfo);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    ioamIngPtr = NULL;
    ioamTransPtr = NULL;

    switch(extType)
    {
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E:
            tabIndex = 0;
            ioamIngPtr = &extInfo.ioamIngressSwitchIpv4;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E:
            tabIndex = 1;
            ioamIngPtr = &extInfo.ioamIngressSwitchIpv6;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E:
            tabIndex = 2;
            ioamTransPtr =&extInfo.ioamTransitSwitchIpv4;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E:
            tabIndex = 3;
            ioamTransPtr =&extInfo.ioamTransitSwitchIpv6;
            break;
        default:
            /* jump to next entry ... check if it is one of 'IOAM' entry info */
            goto    newEntryStart_lbl;
    }

    /************************************************************/
    /* start building the info that is sent to the galtis agent */
    /************************************************************/
    SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(phaThreadId);

    SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(commonInfo.statisticalProcessingFactor);
    SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(commonInfo.busyStallMode);
    SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(commonInfo.stallDropCode);

    if(ioamIngPtr)
    {
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->IOAM_Trace_Type);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Maximum_Length);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Flags);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Hop_Lim);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->node_id);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Type1);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->IOAM_HDR_len1);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Reserved1);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Next_Protocol1);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Type2);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->IOAM_HDR_len2);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Reserved2);
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamIngPtr->Next_Protocol2);
    }
    else
    if(ioamTransPtr)
    {
        SET_NEXT_FIELD_IN_REFRESH_TO_GALTIS(ioamTransPtr->node_id);
    }

    /************************************************************/
    /* ended building the info that is sent to the galtis agent */
    /************************************************************/

    /**************************************/
    /* put last info : the tabulator type */
    /**************************************/
    galtisOutput(outArgs, GT_OK, "%d%f", tabIndex);
    return CMD_OK;
}

/* wrapper for cpssDxChPhaThreadIdEntryGet */
static CMD_STATUS wr_cpssDxChPhaThreadIdEntry_ioam_GetFirst
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

    /* map input arguments to locals */
    /*devNum    = (GT_U8)inArgs[0];*/
    refresh_curr_phaThreadId     = inArgs[1];
    refresh_end_phaThreadId      = inArgs[2];
    /*tabIndex  = inArgs[3]; */

    if(refresh_curr_phaThreadId == 0)
    {
        /* support 'unaware' of '0' */
        refresh_curr_phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
    }

    if(refresh_end_phaThreadId > PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC ||
       refresh_end_phaThreadId == 0)
    {
        /* support 'out of range'   */
        /* support 'unaware' of '0' */
        refresh_end_phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
    }

    if(refresh_curr_phaThreadId > PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC)
    {
        /* support 'out of range'  ... get last index only */
        refresh_curr_phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
    }

    if(refresh_curr_phaThreadId > refresh_end_phaThreadId)
    {
        /* support 'start > end'  ... get single index only */
        refresh_end_phaThreadId = refresh_curr_phaThreadId;
    }

    return wr_cpssDxChPhaThreadIdEntry_ioam_GetNext(inArgs, inFields, numFields, outArgs);
}

/**
* @internal cpssDxChPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
* @param[in] phaThreadId              - the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wr_cpssDxChPhaPortThreadIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32       phaThreadId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PORT_NUM)inArgs[1];
    enable      = (GT_BOOL)inArgs[2];
    phaThreadId = inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPhaPortThreadIdSet(devNum,portNum,enable,phaThreadId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChPhaPortThreadIdGet function
* @endinternal
*
* @brief   Per target port ,get the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enablePtr                - (pointer to)enable/disable the use of threadId for the target port.
* @param[in] phaThreadIdPtr           - (pointer to)the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wr_cpssDxChPhaPortThreadIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32       phaThreadId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PORT_NUM)inArgs[1];
    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPhaPortThreadIdGet(devNum, portNum,&enable,&phaThreadId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable,phaThreadId);

    return CMD_OK;
}

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoSet function
* @endinternal
*
* @brief   Set the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] dropCode        - Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[in] packetCommand   - Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or dropCode or packetCommand
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8        devNum;
    CPSS_NET_RX_CPU_CODE_ENT    dropCode;
    CPSS_PACKET_CMD_ENT         packetCommand;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    dropCode        = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];
    packetCommand   = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPhaHeaderModificationViolationInfoSet(devNum,dropCode,packetCommand);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal cpssDxChPhaHeaderModificationViolationInfoGet function
* @endinternal
*
* @brief   Get the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] dropCodePtr    - (pointer to) Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[out] packetCommandPtr - (pointer to) Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    CPSS_NET_RX_CPU_CODE_ENT    dropCode;
    CPSS_PACKET_CMD_ENT         packetCommand;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPhaHeaderModificationViolationInfoGet(devNum, &dropCode,&packetCommand);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", dropCode,packetCommand);

    return CMD_OK;
}

/**
* @internal cpssDxChPhaHeaderModificationViolationCapturedGet function
* @endinternal
*
* @brief  Get the thread ID of the first header size violating packet , and the violation type.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] capturedThreadIdPtr   - (pointer to) The thread Id of the first header size violating packet
* @param[out] violationTypePtr      - (pointer to) Type describes which exact header size violation occurred
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - when no more modification violations to get info about
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       capturedThreadId;
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  violationType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPhaHeaderModificationViolationCapturedGet(devNum, &capturedThreadId,&violationType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", capturedThreadId,violationType);

    return CMD_OK;
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdSet function
* @endinternal
*
* @brief   set the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] phaThreadId              - the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wr_cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8        devNum;
    GT_U32       phaThreadId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    phaThreadId     = inArgs[1];

    /* call cpss api function */
    result = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(devNum,phaThreadId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdGet function
* @endinternal
*
* @brief   get the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] phaThreadIdPtr           - (pointer to) the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wr_cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       phaThreadId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(devNum, &phaThreadId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", phaThreadId);

    return CMD_OK;
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterGet function
* @endinternal
*
* @brief   Get the PHA counter value of statistical processing cancellations.
*           Note: the counter is cleared after read (by HW).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] counterPtr              - (pointer)the counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wr_cpssDxChPhaStatisticalProcessingCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U64       counter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPhaStatisticalProcessingCounterGet(devNum, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", counter.l[0],counter.l[1]);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName, funcReference, funcArgs, funcFields */
    { "cpssDxChPhaInit",
       &wr_cpssDxChPhaInit,
       2,  0 }

   ,{ "cpssDxChPhaThreadIdEntry_ioamSet",
       &wr_cpssDxChPhaThreadIdEntry_ioam_Set,
       4, 17}
   ,{ "cpssDxChPhaThreadIdEntry_ioamGetFirst",
       &wr_cpssDxChPhaThreadIdEntry_ioam_GetFirst,
       4,  0 }
   ,{ "cpssDxChPhaThreadIdEntry_ioamGetNext",
       &wr_cpssDxChPhaThreadIdEntry_ioam_GetNext,
       4,  0 }

   ,{ "cpssDxChPhaPortThreadIdSet",
       &wr_cpssDxChPhaPortThreadIdSet,
       4,  0 }
   ,{ "cpssDxChPhaPortThreadIdGet",
       &wr_cpssDxChPhaPortThreadIdGet,
       2,  0 }

   ,{ "cpssDxChPhaHeaderModificationViolationInfoSet",
       &wr_cpssDxChPhaHeaderModificationViolationInfoSet,
       3,  0 }
   ,{ "cpssDxChPhaHeaderModificationViolationInfoGet",
       &wr_cpssDxChPhaHeaderModificationViolationInfoGet,
       1,  0 }

   ,{ "cpssDxChPhaHeaderModificationViolationCapturedGet",
       &wr_cpssDxChPhaHeaderModificationViolationCapturedGet,
       1,  0 }

   ,{ "cpssDxChPhaStatisticalProcessingCounterThreadIdSet",
       &wr_cpssDxChPhaStatisticalProcessingCounterThreadIdSet,
       2,  0 }
   ,{ "cpssDxChPhaStatisticalProcessingCounterThreadIdGet",
       &wr_cpssDxChPhaStatisticalProcessingCounterThreadIdGet,
       1,  0 }

   ,{ "cpssDxChPhaStatisticalProcessingCounterGet",
       &wr_cpssDxChPhaStatisticalProcessingCounterGet,
       1,  0 }

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))
/**
* @internal cmdLibInitCpssDxChPha function
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
GT_STATUS cmdLibInitCpssDxChPha
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

