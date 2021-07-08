/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
*/
/**
********************************************************************************
* @file wrapCpssDxChCscd.c
*
* @brief Wrapper functions for Cscd cpss.dxCh functions
*
* @version   34
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>

/****************************************************************/
/****** trunk WA 'C' for phy1690 ********************************/
/****************************************************************/
extern GT_STATUS cpssDxChCscdPortTypeSet_phy1690_WA_C
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
);
extern GT_U32   appDemo_trunk_phy1690_WA_C;
/****************************************************************/

static GT_STATUS wr_cpssDxChCscdPortTypeSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    if(appDemo_trunk_phy1690_WA_C)
    {
        return cpssDxChCscdPortTypeSet_phy1690_WA_C(devNum,portNum,portDirection,portType);
    }
    else
    {
        return cpssDxChCscdPortTypeSet(devNum,portNum,portDirection,portType);
    }
}

/**
* @enum WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ENT
 *
 * @brief type
*/
typedef enum{

    /** @brief Get all entries in cascade device map table.
     *  WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ALL_E -
     *  Get entries for specific device in cascade device map table.
     *  WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ALL_E -
     *  Get entries for specific device and specific port
     *  in cascade device map table.
     */
    WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ALL_E,

    WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_SPEC_DEV_E,

    WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_SPEC_DEV_PORT_E

} WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ENT;

/**
* @internal wrCpssDxChCscdPortTypeSet function
* @endinternal
*
* @brief   Configure a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdPortTypeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                      devNum;
    GT_PHYSICAL_PORT_NUM                      portNum;
    CPSS_CSCD_PORT_TYPE_ENT    portType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portType = (CPSS_CSCD_PORT_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = wr_cpssDxChCscdPortTypeSet(devNum, portNum,CPSS_PORT_DIRECTION_BOTH_E, portType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortTypeGet function
* @endinternal
*
* @brief   Retrieve a PP port cascade port configuration. Application is
*         responsible for setting the default values of the port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortTypeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                      devNum;
    GT_PHYSICAL_PORT_NUM                      portNum;
    CPSS_CSCD_PORT_TYPE_ENT    portType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortTypeGet(devNum, portNum, CPSS_PORT_DIRECTION_BOTH_E,&portType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portType);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortTypeSet1 function
* @endinternal
*
* @brief   Configure a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdPortTypeSet1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                      devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    CPSS_CSCD_PORT_TYPE_ENT    portType;
    CPSS_PORT_DIRECTION_ENT      portDirection;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_PORT_DIRECTION_ENT)inArgs[2];
    portType = (CPSS_CSCD_PORT_TYPE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = wr_cpssDxChCscdPortTypeSet(devNum, portNum,portDirection, portType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortTypeGet1 function
* @endinternal
*
* @brief   Retrieve a PP port cascade port configuration. Application is
*         responsible for setting the default values of the port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortTypeGet1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                      devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    CPSS_CSCD_PORT_TYPE_ENT    portType;
    CPSS_PORT_DIRECTION_ENT      portDirection;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_PORT_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortTypeGet(devNum, portNum, portDirection,&portType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portType);
    return CMD_OK;
}

/************table cpssDxChCscdDevMap*****************/
#define NUM_OF_TRGT_DEV 32
static GT_HW_DEV_NUM    gTargetDevNum;
static GT_HW_DEV_NUM    endTargetDevNum;
static GT_PORT_NUM      gTargetPortNum;
static GT_PORT_NUM      endTargetPortNum;
static CPSS_PORTS_BMP_STC portsMembers[NUM_OF_TRGT_DEV];

/**
* @internal wrCpssDxChCscdDevMapTableSet1 function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number or bad trunk hash mode
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk (applicable devices : Lion2)
*                                       2. hold members from no local device (applicable devices : Lion2)
*                                       3. hold members from more then single hemisphere (applicable devices : Lion2)
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableSet1

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                          devNum;
    GT_HW_DEV_NUM                  hwDevNum_;
    GT_HW_DEV_NUM                  targetHwDevNum;
    GT_HW_DEV_NUM                  sourceHwDevNum;
    GT_PORT_NUM                    portNum;
    CPSS_CSCD_LINK_TYPE_STC        cascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;
    GT_BOOL                        egressAttributesLocallyEn=GT_FALSE;
    GT_TRUNK_ID                    trunkId;
    GT_PORT_NUM                    portNum_;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hwDevNum_ = (GT_HW_DEV_NUM)inArgs[0];

    targetHwDevNum = (GT_HW_DEV_NUM)inFields[0];
    sourceHwDevNum = (GT_HW_DEV_NUM)inFields[1];
    portNum = (GT_PORT_NUM)inFields[2];
    cascadeLink.linkNum = (GT_U32)inFields[3];
    cascadeLink.linkType = (CPSS_CSCD_LINK_TYPE_ENT)inFields[4];
    srcPortTrunkHashEn = (CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT)inFields[5];

    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(targetHwDevNum, portNum);
    CONVERT_DEV_PORT_DATA_MAC(sourceHwDevNum, portNum);

    if(cascadeLink.linkType == CPSS_CSCD_LINK_TYPE_PORT_E)
    {
        portNum_ = cascadeLink.linkNum;
        CONVERT_DEV_PORT_DATA_MAC(hwDevNum_, portNum_);
        cascadeLink.linkNum = portNum_;
    }
    else
    {
        trunkId = (GT_TRUNK_ID)cascadeLink.linkNum;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        cascadeLink.linkNum = (GT_U32)trunkId;
    }

    /* call cpss api function */
    result = cpssDxChCscdDevMapTableSet(devNum, targetHwDevNum, sourceHwDevNum,
                                        portNum,
                                        &cascadeLink,
                                        srcPortTrunkHashEn,
                                        egressAttributesLocallyEn);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdDevMapTableGetNext1 function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableGetNext1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdDevMapTableGetFirst1 function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableGetFirst1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_HW_DEV_NUM                       targetHwDevNum;
    GT_HW_DEV_NUM                       sourceHwDevNum;
    GT_PORT_NUM                         portNum;
    GT_U8                               devNum;
    GT_HW_DEV_NUM                       hwDevNum_;
    CPSS_CSCD_LINK_TYPE_STC             cascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT  srcPortTrunkHashEn;
    GT_BOOL                             egressAttributesLocallyEn;
    GT_TRUNK_ID                         trunkId;
    GT_PORT_NUM                         portNum_;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hwDevNum_ = (GT_HW_DEV_NUM)inArgs[0];
    targetHwDevNum = (GT_HW_DEV_NUM)inArgs[1];
    sourceHwDevNum = (GT_HW_DEV_NUM)inArgs[2];
    portNum = (GT_PORT_NUM)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(targetHwDevNum, portNum);
    CONVERT_DEV_PORT_DATA_MAC(sourceHwDevNum, portNum);


    /* call cpss api function */
    result = cpssDxChCscdDevMapTableGet(devNum, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink,
                                        &srcPortTrunkHashEn, &egressAttributesLocallyEn);


    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    if(cascadeLink.linkType == CPSS_CSCD_LINK_TYPE_PORT_E)
    {
        portNum_ = (GT_U8)cascadeLink.linkNum;
        CONVERT_BACK_DEV_PORT_DATA_MAC(hwDevNum_, portNum_);
        cascadeLink.linkNum = (GT_U32)portNum_;
    }
    else
    {
        trunkId = (GT_TRUNK_ID)cascadeLink.linkNum;
        CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(trunkId);
        cascadeLink.linkNum = (GT_U32)trunkId;
    }

    inFields[0] = targetHwDevNum;
    inFields[1] = sourceHwDevNum;
    inFields[2] = portNum;
    inFields[3] = cascadeLink.linkNum;
    inFields[4] = cascadeLink.linkType;
    inFields[5] = srcPortTrunkHashEn;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                              inFields[3], inFields[4], inFields[5]);

    galtisOutput(outArgs, result, "%f");
    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdDevMapTableSetExt function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or target port
*                                       or bad trunk hash mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableSetExt

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                          devNum;
    GT_HW_DEV_NUM                  hwDevNum_;
    GT_HW_DEV_NUM                  targetDevNum;
    GT_PORT_NUM                    targetPortNum;
    CPSS_CSCD_LINK_TYPE_STC        cascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;
    GT_BOOL                            egressAttributesLocallyEn = GT_FALSE;
    GT_TRUNK_ID                         trunkId;
    GT_U32                              portNum;

    GT_HW_DEV_NUM                   tempHwTrgDev;
    GT_PORT_NUM                     tempTrgPort;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hwDevNum_ = (GT_HW_DEV_NUM)inArgs[0];

    targetDevNum = (GT_HW_DEV_NUM)inFields[0];
    targetPortNum = (GT_PORT_NUM)inFields[1];
    cascadeLink.linkNum = (GT_U32)inFields[2];
    cascadeLink.linkType = (CPSS_CSCD_LINK_TYPE_ENT)inFields[3];
    srcPortTrunkHashEn = (CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT)inFields[4];

    tempHwTrgDev = targetDevNum;
    tempTrgPort =  targetPortNum;

    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(tempHwTrgDev, tempTrgPort);

    targetDevNum = tempHwTrgDev;
    targetPortNum = tempTrgPort;

    if(cascadeLink.linkType == CPSS_CSCD_LINK_TYPE_PORT_E)
    {
        portNum = cascadeLink.linkNum;
        CONVERT_DEV_PORT_DATA_MAC(hwDevNum_, portNum);
        cascadeLink.linkNum = portNum;
    }
    else
    {
        trunkId = (GT_TRUNK_ID)cascadeLink.linkNum;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        cascadeLink.linkNum = (GT_U32)trunkId;
    }

    /* call cpss api function */
    result = cpssDxChCscdDevMapTableSet(devNum, targetDevNum, 0, targetPortNum,
                                        &cascadeLink,
                                        srcPortTrunkHashEn,
                                        egressAttributesLocallyEn);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdDevMapTableSet function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device (which is not
*         the local device)should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    static GT_UINTPTR inFieldsNew[CMD_MAX_FIELDS];
    /* map input arguments to locals */
    /*inArgs[0];->devNum*/
    inFieldsNew[0] = inFields[0];/*targetDevNum*/
    inFieldsNew[1] = 0; /*targetPortNum*/
    inFieldsNew[2] = inFields[1];/*cascadeLink.linkNum*/
    inFieldsNew[3] = inFields[2];/*cascadeLink.linkType*/
    inFieldsNew[4] = inFields[3];/*srcPortTrunkHashEn*/

    return wrCpssDxChCscdDevMapTableSetExt(inArgs,inFieldsNew,numFields+1,outArgs);
}

/**
* @internal wrCpssDxChCscdDevMapTableGetNextExt function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device (which is not
*         the local device)should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or bad trunk hash
*                                       mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableGetNextExt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    GT_HW_DEV_NUM                       hwDevNum_;
    GT_HW_DEV_NUM                       targetHwDevNum;
    GT_PORT_NUM                         targetPortNum;
    GT_HW_DEV_NUM                       backCnvTrgHwDevNum;
    GT_PORT_NUM                         backCnvTrgPortNum;
    CPSS_CSCD_LINK_TYPE_STC             cascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT  srcPortTrunkHashEn;
    GT_BOOL                             egressAttributesLocallyEn;
    GT_TRUNK_ID                         trunkId;
    GT_PORT_NUM                         portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(gTargetDevNum >= endTargetDevNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hwDevNum_ = (GT_HW_DEV_NUM)inArgs[0];

    targetHwDevNum = (GT_HW_DEV_NUM)gTargetDevNum;
    targetPortNum = (GT_PORT_NUM)gTargetPortNum;

    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(targetHwDevNum, targetPortNum);

    /* call cpss api function */
    result = cpssDxChCscdDevMapTableGet(devNum, targetHwDevNum, 0, targetPortNum,
                                        &cascadeLink,
                                        &srcPortTrunkHashEn,
                                        &egressAttributesLocallyEn);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = gTargetDevNum;
    inFields[1] = gTargetPortNum;

    if(cascadeLink.linkType == CPSS_CSCD_LINK_TYPE_PORT_E)
    {
        portNum = cascadeLink.linkNum;
        CONVERT_BACK_DEV_PORT_DATA_MAC(hwDevNum_, portNum);
        cascadeLink.linkNum = portNum;
    }
    else
    {
        trunkId = (GT_TRUNK_ID)cascadeLink.linkNum;
        CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(trunkId);
        cascadeLink.linkNum = (GT_U32)trunkId;
    }

    inFields[2] = cascadeLink.linkNum;
    inFields[3] = cascadeLink.linkType;
    inFields[4] = srcPortTrunkHashEn;

    /* clear fields if this pair <devId, portNum> already treated */
    /* due to Galtis DEV_PORT conversion                          */
    /* assumed that if <dev0, port0> converted to <dev1, port1>   */
    /* <dev1, port1> also back_converted to <dev0, port0>         */
    backCnvTrgHwDevNum = targetHwDevNum;
    backCnvTrgPortNum  = targetPortNum;
    CONVERT_BACK_DEV_PORT_DATA_MAC(backCnvTrgHwDevNum, backCnvTrgPortNum);
    if ((backCnvTrgHwDevNum < gTargetDevNum) ||
        ((backCnvTrgHwDevNum == gTargetDevNum)
         && (backCnvTrgPortNum < gTargetPortNum)))
    {
        inFields[2] = 0;
        inFields[3] = 0;
        inFields[4] = 0;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d", inFields[0], inFields[1], inFields[2] ,
                            inFields[3], inFields[4]);

    galtisOutput(outArgs, result, "%f");

    if(gTargetPortNum == endTargetPortNum)
    {
        gTargetDevNum++;
        gTargetPortNum = 0;
    }
    else
    {
        gTargetPortNum++;
    }

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdDevMapTableGetFirstExt function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device (which is not
*         the local device)should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or bad trunk hash
*                                       mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableGetFirstExt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ENT type;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode;
    GT_U8                               devNum;
    GT_BOOL                             dualMode;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];

    result = cpssDxChCscdDevMapLookupModeGet(devNum, &mode);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    result = cpssSystemDualDeviceIdModeEnableGet(&dualMode);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    if(mode == CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E)
    {
        endTargetPortNum = (dualMode == GT_TRUE)? 127 : 63;
    }
    else
    {
        endTargetPortNum = 0;
    }

    gTargetPortNum = 0;

    type = (WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ENT)inArgs[1];
    if(type != WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_ALL_E)
    {
        gTargetDevNum = (GT_HW_DEV_NUM)inArgs[2];
        endTargetDevNum = gTargetDevNum + 1;

        if(type == WR_CPSS_DXCH_CSCD_DEV_MAP_TABLE_GET_SPEC_DEV_PORT_E)
        {
            if(mode == CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E)
            {
                gTargetPortNum = (GT_PORT_NUM)inArgs[3];
            }
            endTargetPortNum = gTargetPortNum;
        }
    }
    else
    {
        gTargetDevNum = 0;
        endTargetDevNum = NUM_OF_TRGT_DEV;
    }

    return wrCpssDxChCscdDevMapTableGetNextExt(
        inArgs,inFields,numFields,outArgs);

}

/**
* @internal wrCpssDxChCscdDevMapTableGetNext function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device (which is not
*         the local device)should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or bad trunk hash
*                                       mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    GT_HW_DEV_NUM                       hwDevNum_;
    GT_HW_DEV_NUM                       targetHwDevNum, targetPortNum;
    CPSS_CSCD_LINK_TYPE_STC             cascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT  srcPortTrunkHashEn;
    GT_BOOL                             egressAttributesLocallyEn;
    GT_TRUNK_ID                         trunkId;
    GT_PORT_NUM                         portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    hwDevNum_ = (GT_HW_DEV_NUM)inArgs[0];
    targetHwDevNum = (GT_HW_DEV_NUM)gTargetDevNum;
    if (targetHwDevNum >= NUM_OF_TRGT_DEV)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    targetPortNum = 0;

    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(targetHwDevNum, targetPortNum);

    targetPortNum = 0;/* restore to 0 in any case */

    /* call cpss api function */
    result = cpssDxChCscdDevMapTableGet(devNum, targetHwDevNum, 0, targetPortNum,
                                        &cascadeLink,
                                        &srcPortTrunkHashEn,
                                        &egressAttributesLocallyEn);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    if( targetHwDevNum >= NUM_OF_TRGT_DEV )
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    inFields[0] = gTargetDevNum;

    if(!CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsMembers[targetHwDevNum], targetPortNum))
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers[targetHwDevNum],targetPortNum);
        if(cascadeLink.linkType == CPSS_CSCD_LINK_TYPE_PORT_E)
        {
            portNum = cascadeLink.linkNum;
            CONVERT_BACK_DEV_PORT_DATA_MAC(hwDevNum_, portNum);
            cascadeLink.linkNum = portNum;
        }
        else
        {
            trunkId = (GT_TRUNK_ID)cascadeLink.linkNum;
            CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(trunkId);
            cascadeLink.linkNum = (GT_U32)trunkId;
        }

        inFields[1] = cascadeLink.linkNum;
        inFields[2] = cascadeLink.linkType;
        inFields[3] = srcPortTrunkHashEn;
    }
    else
    {
        inFields[1] = 0;
        inFields[2] = 0;
        inFields[3] = 0;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2] , inFields[3]);

    galtisOutput(outArgs, result, "%f");

    gTargetDevNum++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdDevMapTableGetFirst function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device (which is not
*         the local device)should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or bad trunk hash
*                                       mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdDevMapTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    gTargetDevNum = 0;

    cpssOsMemSet(portsMembers, 0, sizeof(portsMembers));

    return wrCpssDxChCscdDevMapTableGetNext(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChCscdRemapQosModeSet function
* @endinternal
*
* @brief   Enables/disables remapping of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdRemapQosModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT    remapType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    remapType = (CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdRemapQosModeSet(devNum, portNum, remapType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdRemapQosModeGet function
* @endinternal
*
* @brief   Get remapping status of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChCscdRemapQosModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    GT_PHYSICAL_PORT_NUM                 portNum;
    CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT    remapType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdRemapQosModeGet(devNum, portNum, &remapType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", remapType);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCtrlQosSet function
* @endinternal
*
* @brief   Set control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdCtrlQosSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_U8                                ctrlTc;
    CPSS_DP_LEVEL_ENT                    ctrlDp;
    CPSS_DP_LEVEL_ENT                    cpuToCpuDp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ctrlTc = (GT_U8)inArgs[1];
    ctrlDp = (CPSS_DP_LEVEL_ENT)inArgs[2];
    cpuToCpuDp = (CPSS_DP_LEVEL_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChCscdCtrlQosSet(devNum, ctrlTc, ctrlDp, cpuToCpuDp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCtrlQosGet function
* @endinternal
*
* @brief   Get control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChCscdCtrlQosGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_U8                                ctrlTc;
    CPSS_DP_LEVEL_ENT                    ctrlDp;
    CPSS_DP_LEVEL_ENT                    cpuToCpuDp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdCtrlQosGet(devNum, &ctrlTc, &ctrlDp, &cpuToCpuDp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", ctrlTc, ctrlDp, cpuToCpuDp);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdRemapDataQosTblSet function
* @endinternal
*
* @brief   Set table to remap Data packets QoS parameters
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdRemapDataQosTblSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_U8                                tc;
    CPSS_DP_LEVEL_ENT                    dp;
    GT_U8                                remapTc;
    CPSS_DP_LEVEL_ENT                    remapDp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tc = (GT_U8)inArgs[1];
    dp = (CPSS_DP_LEVEL_ENT)inArgs[2];
    remapTc = (GT_U8)inArgs[3];
    remapDp = (CPSS_DP_LEVEL_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChCscdRemapDataQosTblSet(devNum, tc, dp, remapTc, remapDp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdRemapDataQosTblGet function
* @endinternal
*
* @brief   Get QoS parameters from table to remap Data packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChCscdRemapDataQosTblGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_U8                                tc;
    CPSS_DP_LEVEL_ENT                    dp;
    GT_U8                                remapTc;
    CPSS_DP_LEVEL_ENT                    remapDp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tc = (GT_U8)inArgs[1];
    dp = (CPSS_DP_LEVEL_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChCscdRemapDataQosTblGet(devNum, tc, dp, &remapTc, &remapDp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", remapTc, remapDp);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdDsaSrcDevFilterSet function
* @endinternal
*
* @brief   Enable/Disable filtering the ingress DSA tagged packets in which
*         source id equals to local device number.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdDsaSrcDevFilterSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_BOOL                              enableOwnDevFltr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enableOwnDevFltr = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCscdDsaSrcDevFilterSet(devNum, enableOwnDevFltr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChCscdHyperGInternalPortModeSet function
* @endinternal
*
* @brief   Change HyperG.stack internal port speed.
*
* @note   APPLICABLE DEVICES:      All DxCh devices with XG ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdHyperGInternalPortModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode = (CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdHyperGInternalPortModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdHyperGInternalPortModeGet function
* @endinternal
*
* @brief   Get status of HyperG port mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdHyperGInternalPortModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;

    GT_U8                                devNum;
    GT_PHYSICAL_PORT_NUM                                portNum;
    CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdHyperGInternalPortModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverFastStackRecoveryEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast stack recovery.
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverFastStackRecoveryEnableSet
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
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCscdFastFailoverFastStackRecoveryEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverFastStackRecoveryEnableGet function
* @endinternal
*
* @brief   Get the status of fast stack recovery (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverFastStackRecoveryEnableGet
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
    result = cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverSecondaryTargetPortMapSet function
* @endinternal
*
* @brief   Set secondary target port map.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when secondaryTargetPort is out of range
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverSecondaryTargetPortMapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_PHYSICAL_PORT_NUM secondaryTargetPort;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    secondaryTargetPort = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, secondaryTargetPort);

    /* call cpss api function */
    result = cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(devNum, portNum, secondaryTargetPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverSecondaryTargetPortMapGet function
* @endinternal
*
* @brief   Get Secondary Target Port Map for given device.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverSecondaryTargetPortMapGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_PHYSICAL_PORT_NUM secondaryTargetPort;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* remove casting after changing wrappers to fit new earch code*/
    CPSS_TBD_BOOKMARK_EARCH

    /* call cpss api function */
    result = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(devNum, (GT_U32)portNum, (GT_U32 *)&secondaryTargetPort);

    /* Override Device and Port */
    CONVERT_BACK_DEV_PHYSICAL_PORT_MAC(devNum, secondaryTargetPort);


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", secondaryTargetPort);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast failover loopback termination
*         for single-destination packets.
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet
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
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the status of fast failover loopback termination
*         for single-destination packets (Enabled/Disabled).
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet
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
    result = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverPortIsLoopedSet function
* @endinternal
*
* @brief   Enable/disable Fast Failover on a failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverPortIsLoopedSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL isLooped;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    isLooped = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdFastFailoverPortIsLoopedSet(devNum, portNum, isLooped);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdFastFailoverPortIsLoopedGet function
* @endinternal
*
* @brief   Get status (Enable/Disable) of Fast Failover on the failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      Only DxCh3 devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChCscdFastFailoverPortIsLoopedGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL isLooped;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdFastFailoverPortIsLoopedGet(devNum, portNum, &isLooped);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", isLooped);

    return CMD_OK;
}


/**
* @internal wrCpssDxChCscdPortBridgeBypassEnableSet function
* @endinternal
*
* @brief   The function enables/disables bypass of the bridge engine per port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the DSA tag is not extended Forward, the bridging decision
*       is performed regardless of the setting.
*
*/
static CMD_STATUS wrCpssDxChCscdPortBridgeBypassEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortBridgeBypassEnableGet function
* @endinternal
*
* @brief   The function gets bypass of the bridge engine per port
*         configuration status.
*
* @note   APPLICABLE DEVICES:      All DxCh devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortBridgeBypassEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM portNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortBridgeBypassEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdOrigSrcPortFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable filtering the multi-destination packet that was received
*         by the local device, sent to another device, and sent back to this
*         device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdOrigSrcPortFilterEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];


    /* call cpss api function */
    result = cpssDxChCscdOrigSrcPortFilterEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdOrigSrcPortFilterEnableGet function
* @endinternal
*
* @brief   Get the status of filtering the multi-destination packet that was
*         received by the local device, sent to another device, and sent back to
*         this device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      All DXCH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdOrigSrcPortFilterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    /* call cpss api function */
    result = cpssDxChCscdOrigSrcPortFilterEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdHyperGPortCrcModeSet function
* @endinternal
*
* @brief   Set CRC mode to be standard 4 bytes or proprietary one byte CRC mode.
*
* @note   APPLICABLE DEVICES:      DxCh2 , DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdHyperGPortCrcModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_DIRECTION_ENT portDirection;
    CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT crcMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_PORT_DIRECTION_ENT)inArgs[2];
    crcMode = (CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdHyperGPortCrcModeSet(devNum, portNum, portDirection, crcMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdHyperGPortCrcModeGet function
* @endinternal
*
* @brief   Get CRC mode (standard 4 bytes or proprietary one byte).
*
* @note   APPLICABLE DEVICES:      DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh3.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChCscdHyperGPortCrcModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_DIRECTION_ENT portDirection;
    CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT crcMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_PORT_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdHyperGPortCrcModeGet(devNum, portNum, portDirection, &crcMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", crcMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdQosPortTcRemapEnableSet function
* @endinternal
*
* @brief   Enable/Disable Traffic Class Remapping on cascading port.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosPortTcRemapEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdQosPortTcRemapEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdQosPortTcRemapEnableGet function
* @endinternal
*
* @brief   Get the status of Traffic Class Remapping on cascading port
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosPortTcRemapEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdQosPortTcRemapEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdQosTcRemapTableSet function
* @endinternal
*
* @brief   Remap Traffic Class on cascading port to new Traffic Classes,
*         for each DSA tag type and for source port type (local or cascading).
*         If the source port is enabled for Traffic Class remapping, then traffic
*         will egress with remapped Traffic Class.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosTcRemapTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 tc;
    CPSS_DXCH_CSCD_QOS_TC_REMAP_STC tcMappings;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tc = (GT_U32)inFields[0];
    tcMappings.forwardLocalTc = (GT_U32)inFields[1];
    tcMappings.forwardStackTc = (GT_U32)inFields[2];
    tcMappings.toAnalyzerLocalTc = (GT_U32)inFields[3];
    tcMappings.toAnalyzerStackTc = (GT_U32)inFields[4];
    tcMappings.toCpuLocalTc = (GT_U32)inFields[5];
    tcMappings.toCpuStackTc = (GT_U32)inFields[6];
    tcMappings.fromCpuLocalTc = (GT_U32)inFields[7];
    tcMappings.fromCpuStackTc = (GT_U32)inFields[8];

    /* call cpss api function */
    result = cpssDxChCscdQosTcRemapTableSet(devNum, tc, &tcMappings);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* cpssDxChCscdQosTcRemapTableGet
*
* DESCRIPTION:
*       Get the remapped Traffic Classes for given Traffic Class.
*
* APPLICABLE DEVICES:  DxCh3 and above
*
* INPUTS:
*       devNum          - device number
*       tc              - Traffic Class of the packet on the source port (0..7)
*
* OUTPUTS:
*       tcMappingsPtr   - (pointer to )remapped Traffic Classes
*                         for ingress Traffic Class
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device or port
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_U32 prv_tc_index;

static CMD_STATUS wrCpssDxChCscdQosTcRemapTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_CSCD_QOS_TC_REMAP_STC tcMappings;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    prv_tc_index = 0; /*reset on first*/
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdQosTcRemapTableGet(devNum, prv_tc_index, &tcMappings);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    inFields[0] = prv_tc_index;
    inFields[1] = tcMappings.forwardLocalTc;
    inFields[2] = tcMappings.forwardStackTc;
    inFields[3] = tcMappings.toAnalyzerLocalTc;
    inFields[4] = tcMappings.toAnalyzerStackTc;
    inFields[5] = tcMappings.toCpuLocalTc;
    inFields[6] = tcMappings.toCpuStackTc;
    inFields[7] = tcMappings.fromCpuLocalTc;
    inFields[8] = tcMappings.fromCpuStackTc;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                                      inFields[3], inFields[4], inFields[5],
                                      inFields[6], inFields[7], inFields[8]);

    galtisOutput(outArgs, result, "%f");

    prv_tc_index++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdQosTcRemapTableGetNext function
* @endinternal
*
* @brief   Get the remapped Traffic Classes for given Traffic Class.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosTcRemapTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_CSCD_QOS_TC_REMAP_STC tcMappings;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(prv_tc_index >= CPSS_TC_RANGE_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdQosTcRemapTableGet(devNum, prv_tc_index, &tcMappings);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    inFields[0] = prv_tc_index;
    inFields[1] = tcMappings.forwardLocalTc;
    inFields[2] = tcMappings.forwardStackTc;
    inFields[3] = tcMappings.toAnalyzerLocalTc;
    inFields[4] = tcMappings.toAnalyzerStackTc;
    inFields[5] = tcMappings.toCpuLocalTc;
    inFields[6] = tcMappings.toCpuStackTc;
    inFields[7] = tcMappings.fromCpuLocalTc;
    inFields[8] = tcMappings.fromCpuStackTc;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3],
                inFields[4], inFields[5], inFields[6], inFields[7],
                inFields[8]);

    galtisOutput(outArgs, result, "%f");

    prv_tc_index++;

    return CMD_OK;
}


/**
* @internal wrCpssDxChCscdQosTcDpRemapTableSet function
* @endinternal
*
* @brief   Set remaping of (packet TC,packet ingress Port Type, packet DSA cmd,packet drop precedence)
*         on cascading port to new priority queue for enqueuing the packet and new drop
*         precedence assigned to this packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosTcDpRemapTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 newTc;
    CPSS_DP_LEVEL_ENT newDp;
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&tcDpRemapping, 0, sizeof(tcDpRemapping));
    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tcDpRemapping.tc = (GT_U32)inArgs[1];
    tcDpRemapping.dp = (CPSS_DP_LEVEL_ENT)inArgs[2];
    tcDpRemapping.isStack = (GT_BOOL)inArgs[3];
    tcDpRemapping.dsaTagCmd = (CPSS_DXCH_NET_DSA_CMD_ENT)inArgs[4];
    newTc = (GT_U32)inArgs[5];
    newDp = (CPSS_DP_LEVEL_ENT)inArgs[6];
    /* call cpss api function */
    result = cpssDxChCscdQosTcDpRemapTableSet(devNum, &tcDpRemapping, newTc, newDp, newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdQosTcDpRemapTableGet function
* @endinternal
*
* @brief   Get the remapped value of priority queue and drop precedence assigned to the packet for given
*         (packet TC,packet ingress Port Type, packet DSA cmd,packet drop precedence)
*         on cascading port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosTcDpRemapTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 mappedTc;
    CPSS_DP_LEVEL_ENT mappedDp;
    GT_U32 mappedPfcTc;
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT     remappedMcastPriority;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&tcDpRemapping, 0, sizeof(tcDpRemapping));
    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tcDpRemapping.tc = (GT_U32)inArgs[1];
    tcDpRemapping.dp = (CPSS_DP_LEVEL_ENT)inArgs[2];
    tcDpRemapping.isStack = (GT_BOOL)inArgs[3];
    tcDpRemapping.dsaTagCmd = (CPSS_DXCH_NET_DSA_CMD_ENT)inArgs[4];
    /* call cpss api function */
    result = cpssDxChCscdQosTcDpRemapTableGet(devNum, &tcDpRemapping, &mappedTc, &mappedDp, &mappedPfcTc,&remappedMcastPriority);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", mappedTc, mappedDp);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdQosTcDpRemapTableSet1 function
* @endinternal
*
* @brief   Set remaping of (packet TC, packet ingress Port Type, packet DSA cmd,
*         packet drop precedence and packet multi ot single-destination) on cascading port
*         to new priority queue for enqueuing the packet, new drop precedence
*         and new priority queue for PFC assigned to this packet.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosTcDpRemapTableSet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 newTc;
    CPSS_DP_LEVEL_ENT newDp;
    GT_U32 newPfcTc;
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT  mcastPriority;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&tcDpRemapping, 0, sizeof(tcDpRemapping));
    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tcDpRemapping.tc = (GT_U32)inArgs[1];
    tcDpRemapping.dp = (CPSS_DP_LEVEL_ENT)inArgs[2];
    tcDpRemapping.isStack = (CPSS_DXCH_PORT_PROFILE_ENT)inArgs[3];
    tcDpRemapping.dsaTagCmd = (CPSS_DXCH_NET_DSA_CMD_ENT)inArgs[4];
    tcDpRemapping.targetPortTcProfile = (CPSS_DXCH_PORT_PROFILE_ENT)inArgs[5];
    tcDpRemapping.packetIsMultiDestination = (GT_BOOL)inArgs[6];

    newTc = (GT_U32)inArgs[7];
    newDp = (CPSS_DP_LEVEL_ENT)inArgs[8];
    newPfcTc = (GT_U32)inArgs[9];
    mcastPriority = (CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT)inArgs[10];

    /* call cpss api function */
    result = cpssDxChCscdQosTcDpRemapTableSet(devNum, &tcDpRemapping, newTc, newDp, newPfcTc,mcastPriority);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdQosTcDpRemapTableGet1 function
* @endinternal
*
* @brief   Get the remapped value of priority queue, drop precedence and priority queue for PFC
*         assigned to the packet for given (packet TC, packet ingress Port Type,
*         packet DSA cmd, packet drop precedence and packet multi or single-destination) on cascading port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdQosTcDpRemapTableGet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 mappedTc;
    CPSS_DP_LEVEL_ENT mappedDp;
    GT_U32 mappedPfcTc;
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT    remappedMcastPriority;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&tcDpRemapping, 0, sizeof(tcDpRemapping));
    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tcDpRemapping.tc = (GT_U32)inArgs[1];
    tcDpRemapping.dp = (CPSS_DP_LEVEL_ENT)inArgs[2];
    tcDpRemapping.isStack = (CPSS_DXCH_PORT_PROFILE_ENT)inArgs[3];
    tcDpRemapping.dsaTagCmd = (CPSS_DXCH_NET_DSA_CMD_ENT)inArgs[4];
    tcDpRemapping.targetPortTcProfile = (CPSS_DXCH_PORT_PROFILE_ENT)inArgs[5];
    tcDpRemapping.packetIsMultiDestination = (GT_BOOL)inArgs[6];

    /* call cpss api function */
    result = cpssDxChCscdQosTcDpRemapTableGet(devNum, &tcDpRemapping, &mappedTc, &mappedDp, &mappedPfcTc,&remappedMcastPriority);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", mappedTc, mappedDp, mappedPfcTc,remappedMcastPriority);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdDevMapLookupModeSet function
* @endinternal
*
* @brief   Set lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdDevMapLookupModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCscdDevMapLookupModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdDevMapLookupModeGet function
* @endinternal
*
* @brief   Get lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdDevMapLookupModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdDevMapLookupModeGet(devNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdPortTrgLocalDevMapLookupEnableSet function
* @endinternal
*
* @brief   Enable / Disable the local target port for device map lookup
*         for local device.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable access to the Device Map Table for the local target devices
*       - Enable the local source port for device map lookup
*       - Enable the local target port for device map lookup
*
*/
static CMD_STATUS wrCpssDxChCscdPortTrgLocalDevMapLookupEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DIRECTION_ENT portDirection;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_DIRECTION_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortLocalDevMapLookupEnableSet(devNum, portNum,
                                                         portDirection, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}


/**
* @internal wrCpssDxChCscdPortTrgLocalDevMapLookupEnableGet function
* @endinternal
*
* @brief   Get status of enabling / disabling the local target port
*         for device map lookup for local device.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortTrgLocalDevMapLookupEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_DIRECTION_ENT portDirection;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portDirection = (CPSS_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortLocalDevMapLookupEnableGet(devNum, portNum,
                                                         portDirection, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;

}


/**
* @internal wrCpssDxChCscdDbRemoteHwDevNumModeSet function
* @endinternal
*
* @brief   Set single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - hwDevNum is odd number and hwDevMode is
*                                       CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
*
* @note 1. Only even device numbers allowed to be marked as "dual HW device"
*       2. "Dual HW device" mode must be configured before any other
*       configuration that uses hwDevNum.
*       3. There are no restrictions on SW devNum for dual mode devices.
*
*/
static CMD_STATUS wrCpssDxChCscdDbRemoteHwDevNumModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_HW_DEV_NUM                       hwDevNum;
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT    hwDevMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    hwDevNum = (GT_HW_DEV_NUM)inArgs[0];
    hwDevMode = (CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdDbRemoteHwDevNumModeGet function
* @endinternal
*
* @brief   Get single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChCscdDbRemoteHwDevNumModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_HW_DEV_NUM                       hwDevNum;
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT    hwDevMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    hwDevNum = (GT_HW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", hwDevMode);

    return CMD_OK;

}


/**
* @internal wrCpssDxChCscdPortStackAggregationEnableSet function
* @endinternal
*
* @brief   Enable/disable stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortStackAggregationEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdPortStackAggregationEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortStackAggregationEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;

}


/**
* @internal wrCpssDxChCscdPortStackAggregationConfigSet function
* @endinternal
*
* @brief   Set stack aggregation configuration per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_OUT_OF_RANGE          - on wrong aggDevNum, aggPortNum, aggSrcId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortStackAggregationConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  aggDevNum;
    GT_U32                  aggPortNum;
    GT_U32                  aggSrcId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    aggDevNum = (GT_HW_DEV_NUM)inArgs[2];
    aggPortNum = (GT_PORT_NUM)inArgs[3];
    aggSrcId = (GT_U32)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_DATA_MAC(aggDevNum, aggPortNum);

    /* call cpss api function */
    result = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum, aggDevNum, aggPortNum, aggSrcId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}


/**
* @internal wrCpssDxChCscdPortStackAggregationConfigGet function
* @endinternal
*
* @brief   Get stack aggregation configuration per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortStackAggregationConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  aggDevNum;
    GT_U32                  aggPortNum;
    GT_U32                  aggSrcId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum, &aggDevNum, &aggPortNum, &aggSrcId);

    /* Override Device and Port */
    CONVERT_BACK_DEV_PHYSICAL_PORT_DATA_MAC(aggDevNum, aggPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", aggDevNum, aggPortNum, aggSrcId);

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  srcDevLsbAmount;
    GT_U32                  srcPortLsbAmount;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    srcDevLsbAmount = (GT_U32)inArgs[1];
    srcPortLsbAmount = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(devNum, srcDevLsbAmount, srcPortLsbAmount);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  srcDevLsbAmount;
    GT_U32                  srcPortLsbAmount;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(devNum, &srcDevLsbAmount, &srcPortLsbAmount);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", srcDevLsbAmount, srcPortLsbAmount);

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet function
* @endinternal
*
* @brief   Set the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT     assignmentMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    assignmentMode = (CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet function
* @endinternal
*
* @brief   Get the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT     assignmentMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", assignmentMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortQosDsaModeSet function
* @endinternal
*
* @brief   Configures DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port,portQosDsaTrustMode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChCscdPortQosDsaModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portQosDsaMode = (CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChCscdPortQosDsaModeGet function
* @endinternal
*
* @brief   Get DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port.
* @retval GT_BAD_PTR               - portQosDsaTrustModePtr is NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChCscdPortQosDsaModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portQosDsaMode);

    return CMD_OK;
}


/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet function
* @endinternal
*
* @brief   Set the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  physicalPortBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    physicalPortBase = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, physicalPortBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet function
* @endinternal
*
* @brief   Get the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  physicalPortBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, &physicalPortBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", physicalPortBase);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet function
* @endinternal
*
* @brief   Enable the assignment of the packet source device to be the local device
*         number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the assignment of the packet source device to
*         be the local device number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet function
* @endinternal
*
* @brief   Enable mapping of target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the mapping of target physical port to a
*         remote physical port that resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet function
* @endinternal
*
* @brief   Map a target physical port to a remote physical port that resides over
*         a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_HW_DEV_NUM           remotePhysicalHwDevNum;
    GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    remotePhysicalHwDevNum = (GT_HW_DEV_NUM)inArgs[2];
    remotePhysicalPortNum = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_DATA_MAC(remotePhysicalHwDevNum, remotePhysicalPortNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum,
                                                                    remotePhysicalHwDevNum,
                                                                    remotePhysicalPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet function
* @endinternal
*
* @brief   Get the mapping of a target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_HW_DEV_NUM           remotePhysicalHwDevNum;
    GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum,
                                                                    &remotePhysicalHwDevNum,
                                                                    &remotePhysicalPortNum);
    /* Override Device and Port */
    CONVERT_BACK_DEV_PHYSICAL_PORT_DATA_MAC(remotePhysicalHwDevNum, remotePhysicalPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", (GT_U32)remotePhysicalHwDevNum, (GT_U32)remotePhysicalPortNum);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum,
                                                         enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisModeEnableGet function
* @endinternal
*
* @brief   Get initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                         &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisReservedDevNumSet function
* @endinternal
*
* @brief   Configured what is the device number that must not be used by any of
*         the devices behind this port
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisReservedDevNumSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_HW_DEV_NUM        reservedHwDevNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    reservedHwDevNum = (GT_HW_DEV_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                             reservedHwDevNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisReservedDevNumGet function
* @endinternal
*
* @brief   Get CC reserved device number
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisReservedDevNumGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_HW_DEV_NUM        reservedHwDevNumPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                             &reservedHwDevNumPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", reservedHwDevNumPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisSrcIdSet function
* @endinternal
*
* @brief   Configured what is the source ID used by the line card directly
*         attached to the Centralized chassis port
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisSrcIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               srcId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    srcId = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisSrcIdGet function
* @endinternal
*
* @brief   Get CC Src ID
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisSrcIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               srcIdPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, &srcIdPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", srcIdPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet function
* @endinternal
*
* @brief   Set initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_PORT_NUM          portBaseEport;
    GT_PORT_NUM          trunkBaseEport;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portBaseEport = (GT_PORT_NUM)inArgs[2];
    trunkBaseEport = (GT_PORT_NUM)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum, portNum,
                                                                       portBaseEport,
                                                                       trunkBaseEport);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet function
* @endinternal
*
* @brief   Get initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_PORT_NUM          portBaseEportPtr;
    GT_PORT_NUM          trunkBaseEportPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum, portNum,
                                                                       &portBaseEportPtr,
                                                                       &trunkBaseEportPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", portBaseEportPtr, trunkBaseEportPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    srcTrunkLsbAmount;
    GT_U32    srcPortLsbAmount;
    GT_U32    srcDevLsbAmount;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    srcTrunkLsbAmount = (GT_U32)inArgs[1];
    srcPortLsbAmount = (GT_U32)inArgs[2];
    srcDevLsbAmount = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(devNum,
                                                                     srcTrunkLsbAmount,
                                                                     srcPortLsbAmount,
                                                                     srcDevLsbAmount);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    srcTrunkLsbAmountPtr;
    GT_U32    srcPortLsbAmountPtr;
    GT_U32    srcDevLsbAmountPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(devNum,
                                                                     &srcTrunkLsbAmountPtr,
                                                                     &srcPortLsbAmountPtr,
                                                                     &srcDevLsbAmountPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", srcTrunkLsbAmountPtr,
                 srcPortLsbAmountPtr, srcDevLsbAmountPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdDsaSrcDevFilterGet function
* @endinternal
*
* @brief   get value of Enable/Disable filtering the ingress DSA tagged packets in which
*         source id equals to local device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdDsaSrcDevFilterGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enableOwnDevFltrPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdDsaSrcDevFilterGet(devNum, &enableOwnDevFltrPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enableOwnDevFltrPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortMruCheckOnCascadeEnableGet function
* @endinternal
*
* @brief   Get (Enable/Disable) MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortMruCheckOnCascadeEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              enablePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum,
                                                        portNum, &enablePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enablePtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortMruCheckOnCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortMruCheckOnCascadeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum,
                                                        portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdHashInDsaEnableSet function
* @endinternal
*
* @brief   Enable a packet's hash to be extracted from(rx) and/or inserted into(tx)
*         DSA tag. Relevant for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdHashInDsaEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    CPSS_PORT_DIRECTION_ENT  direction;
    GT_BOOL                  enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[2];
    enable    = (CPSS_CSCD_PORT_TYPE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdHashInDsaEnableSet(devNum, portNum,direction, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChCscdHashInDsaEnableGet function
* @endinternal
*
* @brief   Get value of flag determining whether a packet's hash should be
*         extracted from(rx)/inserted info(tx) DSA tag.
*         Relevand for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdHashInDsaEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    CPSS_PORT_DIRECTION_ENT  direction;
    GT_BOOL                  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdHashInDsaEnableGet(devNum, portNum, direction, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortTcProfiletSet function
* @endinternal
*
* @brief   Sets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortTcProfiletSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    CPSS_PORT_DIRECTION_ENT  direction;
    CPSS_DXCH_PORT_PROFILE_ENT profile;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[2];
    profile   = (CPSS_DXCH_PORT_PROFILE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortTcProfiletSet(devNum, portNum,direction, profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortTcProfiletGet function
* @endinternal
*
* @brief   Gets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortTcProfiletGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    CPSS_PORT_DIRECTION_ENT  direction;
    CPSS_DXCH_PORT_PROFILE_ENT profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortTcProfiletGet(devNum, portNum, direction, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet function
* @endinternal
*
* @brief   Configure port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL                  acceptSkipSaLookup;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    portNum   = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    acceptSkipSaLookup = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(devNum,
                                portNum, acceptSkipSaLookup);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet function
* @endinternal
*
* @brief   Gets port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL                  acceptSkipSaLookupPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(devNum,
                                portNum, &acceptSkipSaLookupPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", acceptSkipSaLookupPtr);
    return CMD_OK;
}

/**
* @internal
*           wrcpssDxChCscdRemotePhysicalPortVlanTagTpidSet
*           function
* @endinternal
*
* @brief   Set VLAN-Tag TPID for remote ports.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChCscdRemotePhysicalPortVlanTagTpidSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                    devNum;
    GT_U32                  vlanTpid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    vlanTpid = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(devNum,vlanTpid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal 
*           wrcpssDxChCscdRemotePhysicalPortVlanTagTpidGet
*           function
* @endinternal
*
* @brief   Gets VLAN-Tag TPID for remote ports.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChCscdRemotePhysicalPortVlanTagTpidGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                    devNum;
    GT_U32                  vlanTpidPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(devNum,&vlanTpidPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vlanTpidPtr);
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChCscdPortTypeSet",
        &wrCpssDxChCscdPortTypeSet,
        3, 0},

    {"cpssDxChCscdPortTypeGet",
        &wrCpssDxChCscdPortTypeGet,
        2, 0},

    {"cpssDxChCscdPortTypeSet1",
        &wrCpssDxChCscdPortTypeSet1,
        4, 0},

    {"cpssDxChCscdPortTypeGet1",
        &wrCpssDxChCscdPortTypeGet1,
        3, 0},

    {"cpssDxChCscdDevMapSet",
        &wrCpssDxChCscdDevMapTableSet,
        1, 4},

    {"cpssDxChCscdDevMapGetFirst",
        &wrCpssDxChCscdDevMapTableGetFirst,
        1, 0},

    {"cpssDxChCscdDevMapGetNext",
        &wrCpssDxChCscdDevMapTableGetNext,
        1, 0},

    {"cpssDxChCscdDevMapExtSet",
        &wrCpssDxChCscdDevMapTableSetExt,
        1, 5},

    {"cpssDxChCscdDevMap1Set",
        &wrCpssDxChCscdDevMapTableSet1,
        1, 6},

    {"cpssDxChCscdDevMap1GetFirst",
        &wrCpssDxChCscdDevMapTableGetFirst1,
        4, 0},

    {"cpssDxChCscdDevMap1GetNext",
        &wrCpssDxChCscdDevMapTableGetNext1,
        4, 0},

    {"cpssDxChCscdDevMapExtGetFirst",
        &wrCpssDxChCscdDevMapTableGetFirstExt,
        4, 0},

    {"cpssDxChCscdDevMapExtGetNext",
        &wrCpssDxChCscdDevMapTableGetNextExt,
        4, 0},

    {"cpssDxChCscdRemapQosModeSet",
        &wrCpssDxChCscdRemapQosModeSet,
        3, 0},

    {"cpssDxChCscdRemapQosModeGet",
        &wrCpssDxChCscdRemapQosModeGet,
        2, 0},

    {"cpssDxChCscdCtrlQosSet",
        &wrCpssDxChCscdCtrlQosSet,
        4, 0},

    {"cpssDxChCscdCtrlQosGet",
        &wrCpssDxChCscdCtrlQosGet,
        1, 0},

    {"cpssDxChCscdRemapDataQosTblSet",
        &wrCpssDxChCscdRemapDataQosTblSet,
        5, 0},

    {"cpssDxChCscdRemapDataQosTblGet",
        &wrCpssDxChCscdRemapDataQosTblGet,
        3, 0},

    {"cpssDxChCscdDsaSrcDevFilterSet",
        &wrCpssDxChCscdDsaSrcDevFilterSet,
        2, 0},

    {"cpssDxChCscdHyperGInternalPortModeSet",
        &wrCpssDxChCscdHyperGInternalPortModeSet,
        3, 0},

    {"cpssDxChCscdHyperGInternalPortModeGet",
        &wrCpssDxChCscdHyperGInternalPortModeGet,
        2, 0},

    {"cpssDxChCscdFastFailoverFastStackRecoveryEnableSet",
         &wrCpssDxChCscdFastFailoverFastStackRecoveryEnableSet,
         2, 0},

    {"cpssDxChCscdFastFailoverFastStackRecoveryEnableGet",
         &wrCpssDxChCscdFastFailoverFastStackRecoveryEnableGet,
         1, 0},

    {"cpssDxChCscdFastFailoverSecondaryTargetPortMapSet",
         &wrCpssDxChCscdFastFailoverSecondaryTargetPortMapSet,
         3, 0},

    {"cpssDxChCscdFastFailoverSecondaryTargetPortMapGet",
         &wrCpssDxChCscdFastFailoverSecondaryTargetPortMapGet,
         2, 0},

    {"cpssDxChCscdFastFailoverTerminLocLoopbackEnableSet",
         &wrCpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet,
         2, 0},

    {"cpssDxChCscdFastFailoverTerminLocLoopbackEnableGet",
         &wrCpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet,
         1, 0},

    {"cpssDxChCscdFastFailoverPortIsLoopedSet",
         &wrCpssDxChCscdFastFailoverPortIsLoopedSet,
         3, 0},

    {"cpssDxChCscdFastFailoverPortIsLoopedGet",
         &wrCpssDxChCscdFastFailoverPortIsLoopedGet,
         2, 0},

    {"cpssDxChCscdPortBridgeBypassEnableSet",
         &wrCpssDxChCscdPortBridgeBypassEnableSet,
         3, 0},

    {"cpssDxChCscdPortBridgeBypassEnableGet",
         &wrCpssDxChCscdPortBridgeBypassEnableGet,
         2, 0},

    {"cpssDxChCscdOrigSrcPortFilterEnableSet",
         &wrCpssDxChCscdOrigSrcPortFilterEnableSet,
         2, 0},

    {"cpssDxChCscdOrigSrcPortFilterEnableGet",
         &wrCpssDxChCscdOrigSrcPortFilterEnableGet,
         1, 0},

    {"cpssDxChCscdHyperGPortCrcModeSet",
         &wrCpssDxChCscdHyperGPortCrcModeSet,
         4, 0},

   {"cpssDxChCscdHyperGPortCrcModeGet",
        &wrCpssDxChCscdHyperGPortCrcModeGet,
        3, 0},

    {"cpssDxChCscdQosPortTcRemapEnableSet",
         &wrCpssDxChCscdQosPortTcRemapEnableSet,
         3, 0},

    {"cpssDxChCscdQosPortTcRemapEnableGet",
         &wrCpssDxChCscdQosPortTcRemapEnableGet,
         2, 0},

    {"cpssDxChCscdQosTcRemapTableSet",
         &wrCpssDxChCscdQosTcRemapTableSet,
         1, 9},

    {"cpssDxChCscdQosTcRemapTableGetFirst",
         &wrCpssDxChCscdQosTcRemapTableGetFirst,
         1, 0},

    {"cpssDxChCscdQosTcRemapTableGetNext",
         &wrCpssDxChCscdQosTcRemapTableGetNext,
         1, 0},

    {"cpssDxChCscdQosTcDpRemapTableSet",
         &wrCpssDxChCscdQosTcDpRemapTableSet,
         7, 0},

    {"cpssDxChCscdQosTcDpRemapTableGet",
         &wrCpssDxChCscdQosTcDpRemapTableGet,
         5, 0},

    {"cpssDxChCscdDevMapLookupModeSet",
         &wrCpssDxChCscdDevMapLookupModeSet,
         2, 0},

    {"cpssDxChCscdDevMapLookupModeGet",
         &wrCpssDxChCscdDevMapLookupModeGet,
         1, 0},

    {"cpssDxChCscdPortLocalDevMapLookupEnableSet",
         &wrCpssDxChCscdPortTrgLocalDevMapLookupEnableSet,
         4, 0},

    {"cpssDxChCscdPortLocalDevMapLookupEnableGet",
         &wrCpssDxChCscdPortTrgLocalDevMapLookupEnableGet,
         3, 0},

    {"cpssDxChCscdDbRemoteHwDevNumModeSet",
        &wrCpssDxChCscdDbRemoteHwDevNumModeSet,
        2, 0},

    {"cpssDxChCscdDbRemoteHwDevNumModeGet",
        &wrCpssDxChCscdDbRemoteHwDevNumModeGet,
        1, 0},

    {"cpssDxChCscdPortStackAggregationEnableSet",
        &wrCpssDxChCscdPortStackAggregationEnableSet,
        3, 0},

    {"cpssDxChCscdPortStackAggregationEnableGet",
        &wrCpssDxChCscdPortStackAggregationEnableGet,
        2, 0},

    {"cpssDxChCscdPortStackAggregationConfigSet",
        &wrCpssDxChCscdPortStackAggregationConfigSet,
        5, 0},

    {"cpssDxChCscdPortStackAggregationConfigGet",
        &wrCpssDxChCscdPortStackAggregationConfigGet,
        2, 0},

    {"cpssDxChCscdCentrChasPhyPortMapDsaSrcLsbAmountSet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet,
        3, 0},

    {"cpssDxChCscdCentrChasPhyPortMapDsaSrcLsbAmountGet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet,
        1, 0},

    {"cpssDxChCscdCentrChassisMyPhyPortAssignModeSet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet,
        3, 0},

    {"cpssDxChCscdCentrChassisMyPhyPortAssignModeGet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet,
        2, 0},

    {"cpssDxChCscdPortQosDsaModeSet",
        &wrCpssDxChCscdPortQosDsaModeSet,
        3, 0},

    {"cpssDxChCscdPortQosDsaModeGet",
        &wrCpssDxChCscdPortQosDsaModeGet,
        2, 0},

    {"cpssDxChCscdCentralizedChassisMyPhysPortBaseSet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet,
        3, 0},

    {"cpssDxChCscdCentralizedChassisMyPhysPortBaseGet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet,
        2, 0},

    {"cpssDxChCscdCentrChasMyPhyPortAssignSrcDevEnSet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet,
        3, 0},

    {"cpssDxChCscdCentrChasMyPhyPortAssignSrcDevEnGet",
        &wrCpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet,
        2, 0},

    {"cpssDxChCscdCentrChasRemotePhyPortMapEnableSet",
        &wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet,
        3, 0},

    {"cpssDxChCscdCentrChasRemotePhyPortMapEnableGet",
        &wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet,
        2, 0},

    {"cpssDxChCscdCentrChasRemotePhyPortMapSet",
        &wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet,
        4, 0},

    {"cpssDxChCscdCentrChasRemotePhyPortMapGet",
        &wrCpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet,
        2, 0},
    {"cpssDxChCscdCentralizedChassisModeEnableSet",
        &wrCpssDxChCscdCentralizedChassisModeEnableSet,
        3, 0},

    {"cpssDxChCscdCentralizedChassisModeEnableGet",
        &wrCpssDxChCscdCentralizedChassisModeEnableGet,
        2, 0},

    {"cpssDxChCscdCentralizedChassisReservedDevNumSet",
        &wrCpssDxChCscdCentralizedChassisReservedDevNumSet,
        3, 0},

    {"cpssDxChCscdCentralizedChassisReservedDevNumGet",
        &wrCpssDxChCscdCentralizedChassisReservedDevNumGet,
        2, 0},

    {"cpssDxChCscdCentralizedChassisSrcIdSet",
        &wrCpssDxChCscdCentralizedChassisSrcIdSet,
        3, 0},

    {"cpssDxChCscdCentralizedChassisSrcIdGet",
        &wrCpssDxChCscdCentralizedChassisSrcIdGet,
        2, 0},

    {"cpssDxChCscdCentrChasLineCardDefaultEportBaseSet",
        &wrCpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet,
        4, 0},

    {"cpssDxChCscdCentrChasLineCardDefaultEportBaseGet",
        &wrCpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet,
        2, 0},

    {"cpssDxChCscdCentrChasMappingDsaSrcLsbAmountSet",
        &wrCpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet,
        4, 0},

    {"cpssDxChCscdCentrChasMappingDsaSrcLsbAmountGet",
        &wrCpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet,
        1, 0},

    {"cpssDxChCscdDsaSrcDevFilterGet",
        &wrCpssDxChCscdDsaSrcDevFilterGet,
        1, 0},

    {"cpssDxChCscdPortMruCheckOnCascadeEnableGet",
        &wrCpssDxChCscdPortMruCheckOnCascadeEnableGet,
        2, 0},

    {"cpssDxChCscdPortMruCheckOnCascadeEnableSet",
        &wrCpssDxChCscdPortMruCheckOnCascadeEnableSet,
        3, 0},

    {"cpssDxChCscdHashInDsaEnableSet",
        &wrCpssDxChCscdHashInDsaEnableSet,
        4, 0},

    {"cpssDxChCscdHashInDsaEnableGet",
        &wrCpssDxChCscdHashInDsaEnableGet,
        3, 0},

    {"cpssDxChCscdQosTcDpRemapTableSet1",
        &wrCpssDxChCscdQosTcDpRemapTableSet1,
        11, 0},

    {"cpssDxChCscdQosTcDpRemapTableGet1",
        &wrCpssDxChCscdQosTcDpRemapTableGet1,
        7, 0},

    {"cpssDxChCscdPortTcProfiletSet",
        &wrCpssDxChCscdPortTcProfiletSet,
        4, 0},

    {"cpssDxChCscdPortTcProfiletGet",
        &wrCpssDxChCscdPortTcProfiletGet,
        3, 0},

    {"cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet",
        &wrCpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet,
        3, 0},

    {"cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet",
        &wrCpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet,
        2, 0},

    {"cpssDxChCscdRemotePhysicalPortVlanTagTpidSet",
        &wrcpssDxChCscdRemotePhysicalPortVlanTagTpidSet,
        2, 0},

    {"cpssDxChCscdRemotePhysicalPortVlanTagTpidGet",
        &wrcpssDxChCscdRemotePhysicalPortVlanTagTpidGet,
        1, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChCscd function
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
GT_STATUS cmdLibInitCpssDxChCscd
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


