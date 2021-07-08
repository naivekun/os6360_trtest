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
* wrapConfigCpssDxSal.c
*
* DESCRIPTION:
*       Wrapper functions for Config cpss.dxSal functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 23 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>



/**
* @internal wrCpssDxChCfgDevRemove function
* @endinternal
*
* @brief   Remove the device from the CPSS.
*         This function we release the DB of CPSS that associated with the PP ,
*         and will "know" that the device is not longer valid.
*         This function will not do any HW actions , and therefore can be called
*         before or after the HW was actually removed.
*         After this function is called the devNum is no longer valid for any API
*         until this devNum is used in the initialization sequence for a "new" PP.
*         NOTE: 1. Application may want to disable the Traffic to the CPU , and
*         messages , prior to this operation (if the device still exists).
*         2. Application may want to a reset the HW PP , and there for need
*         to call the "hw reset" function (before calling this function)
*         and after calling this function to do the initialization
*         sequence all over again for the device.
*
* @note   APPLICABLE DEVICES:      All 98Dx250/260/270/246/243/166/163
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - wrong dev
*/
static CMD_STATUS wrCpssDxChCfgDevRemove

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgDevRemove(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChCfgDevEnable function
* @endinternal
*
* @brief   This routine sets the device state.
*
* @note   APPLICABLE DEVICES:      All 98Dx250/260/270/246/243/166/163
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*
*/
static CMD_STATUS wrCpssDxChCfgDevEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_BOOL  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgDevEnable(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgDevEnableGet function
* @endinternal
*
* @brief   This routine gets the device state.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChCfgDevEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_BOOL  enable = GT_FALSE;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgDevEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet function
* @endinternal
*
* @brief   Sets device ID modification for Routed packets.
*         Enables/Disables FORWARD DSA tag modification of the <source device>
*         and <source port> fields of packets routed by the local device.
*         The <source device> is set to the local device ID and the <source port>
*         is set to 61 (the virtual router port).
*
* @note   APPLICABLE DEVICES:      All DxCh devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
*/
static CMD_STATUS wrCpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_PORT_NUM    portNum;
    GT_BOOL  modifyEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    modifyEnable = (GT_BOOL)inArgs[2];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(devNum, portNum, modifyEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet function
* @endinternal
*
* @brief   Gets device ID modification for Routed packets.
*         See Set Api comments.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_PORT_NUM    portNum;
    GT_BOOL  modifyEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(devNum, portNum, &modifyEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "&d", modifyEnable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgDsaTagSrcDevPortModifySet function
* @endinternal
*
* @brief   Enables/Disables Device ID Modification upon packet sending to another
*         stack unit.
*         When Connecting DxCh Devices to SOHO in a Value Blade model to enable
*         DxCh grade Capabilities for FE Ports, in a staking system, we must be
*         able to overcome the 32 devices limitation.
*         To do that, SOHO Device Numbers are not unique and packets received
*         from the SOHO by the DxCh and are relayed to other stack units
*         have their Device ID changed to the DxCh Device ID.
*         On Upstream (packet from SOHO to DxCh):
*         The SOHO Sends a packet to the DxCh and the packet is sent back
*         to the SOHO. In this case we don't want to change the Device ID in
*         the DSA Tag so that the SOHO is able to filter the Src Port and is
*         able to send back the packet to its source when doing cross chip
*         flow control.
*         On Downsteam (packet from SOHO to SOHO):
*         The SOHO receives a packet from another SOHO in this case we
*         want to change the Device ID so that the packet is not filtered.
*         On top of the above, for forwarding packets between the DxCh devices
*         and for Auto Learning, the Port Number must also be changed.
*         In addition Changing the Device ID is needed only for FORWARD DSA Tag.
*
* @note   APPLICABLE DEVICES:      All DxCh devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
static CMD_STATUS wrCpssDxChCfgDsaTagSrcDevPortModifySet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_BOOL  modifedDsaSrcDev;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    modifedDsaSrcDev = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgDsaTagSrcDevPortModifySet(devNum, modifedDsaSrcDev);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgDsaTagSrcDevPortModifyGet function
* @endinternal
*
* @brief   Get status of Device ID Modification upon packet sending to another
*         stack unit. See Set Api comments.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
static CMD_STATUS wrCpssDxChCfgDsaTagSrcDevPortModifyGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_BOOL  modifedDsaSrcDev = GT_FALSE;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgDsaTagSrcDevPortModifyGet(devNum, &modifedDsaSrcDev);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", modifedDsaSrcDev);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgTableNumEntriesGet function
* @endinternal
*
* @brief   the function return the number of entries of each individual table in
*         the HW
*         when several type of entries like ARP and tunnelStart resize in the same
*         table (memory) , the function returns the number of entries for the least
*         denominator type of entry --> in this case number of ARP entries.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgTableNumEntriesGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_CFG_TABLES_ENT table;
    GT_U32 numEntries;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    table = (CPSS_DXCH_CFG_TABLES_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgTableNumEntriesGet(devNum, table, &numEntries);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", numEntries);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgReNumberDevNum function
* @endinternal
*
* @brief   function allow the caller to modify the DB of the cpss ,
*         so all info that was 'attached' to 'oldDevNum' will be moved to 'newDevNum'.
*         NOTE:
*         1. it is the responsibility of application to update the 'devNum' of
*         HW entries from 'oldDevNum' to 'newDevNum' using appropriate API's
*         such as 'Trunk members','Fdb entries','NH entries','PCE entries'...
*         2. it's application responsibility to update the HW device number !
*         see API cpssDxChCfgHwDevNumSet
*         3. no HW operations involved in this API
*         NOTE:
*         this function MUST be called under 'Interrupts are locked' and under
*         'Tasks lock'
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the device oldDevNum not exist
* @retval GT_OUT_OF_RANGE          - the device newDevNum value > 0x1f (0..31)
* @retval GT_ALREADY_EXIST         - the new device number is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChCfgReNumberDevNum
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    oldDevNum;
    GT_U8    newDevNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    oldDevNum = (GT_U8)inArgs[0];
    newDevNum = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgReNumberDevNum(oldDevNum, newDevNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChCfgIngressDropCntrSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgIngressDropCntrSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      counter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    counter = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgIngressDropCntrSet(devNum, counter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgIngressDropCntrGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgIngressDropCntrGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      counter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgIngressDropCntrGet(devNum, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counter);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgIngressDropCntrModeSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_OUT_OF_RANGE          - on wrong portNum or vlan.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgIngressDropCntrModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT mode;
    GT_PORT_NUM                                      portNum;
    GT_U16                                      vlan;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mode = (GT_U32)inArgs[1];
    portNum = (GT_PORT_NUM)inArgs[2];
    vlan = (GT_U16)inArgs[3];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCfgIngressDropCntrModeSet(devNum, mode, (GT_PORT_NUM)portNum, vlan);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgIngressDropCntrModeGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - illegal state of configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgIngressDropCntrModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_HW_DEV_NUM       hwDevNum;
    CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT mode;
    GT_PORT_NUM                                 portNum;
    GT_U16                                      vlan;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    hwDevNum = (GT_HW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgIngressDropCntrModeGet((GT_U8)hwDevNum, &mode, &portNum, &vlan);

    /* Override Back Device and Port */
    CONVERT_BACK_DEV_PORT_DATA_MAC(hwDevNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", mode, portNum, vlan);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgGlobalEportSet function
* @endinternal
*
* @brief   Set Global ePorts ranges configuration.
*         Global ePorts are ePorts that are global in the entire system, and are
*         not unique per device as regular ePorts are.
*         Global ePorts are used for Representing:
*         1. a Trunk.(map target ePort to trunkId by the E2PHY)
*         2. a ePort ECMP Group.(map 'primary' target ePort to 'secondary' target ePort)
*         3. an ePort Replication Group (eVIDX).(map target ePort to eVIDX group)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgGlobalEportSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  ecmp;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  global;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    global.enable = (GT_BOOL)inArgs[1];
    global.pattern = (GT_U32)inArgs[2];
    global.mask = (GT_U32)inArgs[3];

    ecmp.enable = (GT_BOOL)inArgs[4];
    ecmp.pattern = (GT_U32)inArgs[5];
    ecmp.mask = (GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssDxChCfgGlobalEportSet(devNum, &global , &ecmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgGlobalEportGet function
* @endinternal
*
* @brief   Get Global ePorts ranges configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgGlobalEportGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  ecmp = {0,0,0};
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  global;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgGlobalEportGet(devNum, &global,&ecmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d",
                 global.enable,global.pattern,global.mask,
                 ecmp.enable,ecmp.pattern,ecmp.mask);
    return CMD_OK;
}

/*******************************************************************************
* cpssDxChCfgDevInfoGet
* 
* Return device family and subfamily info
* 
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - Device number
*
* OUTPUTS:
*       devInfoPtr   - (pointer to) device information
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong device number
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
* COMMENTS:
*
*******************************************************************************/
static CMD_STATUS wrCpssDxChCfgDevInfoGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        devNum;
    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;

    GT_UTIL_PP_FAMILY_TYPE_ENT      wrFamily;
    GT_UTIL_PP_SUB_FAMILY_TYPE_ENT  wrSubFamily;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgDevInfoGet(devNum, &devInfo);

    if( GT_OK != utilCpssToWrFamilyConvert(devInfo.genDevInfo.devFamily,&wrFamily) )
    {
        cmdOsPrintf("utilCpssToWrFamilyConvert failed\n");
        return GT_FAIL;
    }

    if( GT_OK != utilCpssToWrSubFamilyConvert(devInfo.genDevInfo.devSubFamily, &wrSubFamily) )
    {
        cmdOsPrintf("utilCpssToWrSubFamilyConvert failed\n");
        return GT_FAIL;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", devInfo.genDevInfo.devType,
                                                                    devInfo.genDevInfo.revision,
                                                                    wrFamily,
                                                                    wrSubFamily,
                                                                    devInfo.genDevInfo.maxPortNum,
                                                                    devInfo.genDevInfo.numOfVirtPorts,
                                                                    devInfo.genDevInfo.existingPorts.ports[0],
                                                                    devInfo.genDevInfo.existingPorts.ports[1],
                                                                    devInfo.genDevInfo.existingPorts.ports[2],
                                                                    devInfo.genDevInfo.existingPorts.ports[3],
                                                                    devInfo.genDevInfo.existingPorts.ports[4],
                                                                    devInfo.genDevInfo.existingPorts.ports[5],
                                                                    devInfo.genDevInfo.existingPorts.ports[6],
                                                                    devInfo.genDevInfo.existingPorts.ports[7],
                                                                    devInfo.genDevInfo.hwDevNumMode,
                                                                    devInfo.genDevInfo.cpuPortMode);

    return CMD_OK;

}

/**
* @internal wrCpssDxChCfgHitlessWriteMethodEnableSet function
* @endinternal
*
* @brief   Enable/disable hitless write methood.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChCfgHitlessWriteMethodEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8    devNum;
    GT_BOOL  enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgHitlessWriteMethodEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChCfgPortDefaultSourceEportNumberSet function
* @endinternal
*
* @brief   Set Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number. In Multi-core
*       architecture it is the global physical port number (i.e. Local Core
*       Port number bitwise ORed with LocalCoreID<<4)
*       2. Can be overriden by TTI Action/RBridge
*
*/
static CMD_STATUS wrCpssDxChCfgPortDefaultSourceEportNumberSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_PHYSICAL_PORT_NUM               portNum;
    GT_PORT_NUM                        ePort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ePort = (GT_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC (devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgPortDefaultSourceEportNumberGet function
* @endinternal
*
* @brief   Get Port default Source ePort number.
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
*
* @note 1. Default value is physical port number. In Multi-core
*       architecture it is the global physical port number (i.e. Local Core
*       Port number bitwise ORed with LocalCoreID<<4)
*       2. Can be overriden by TTI Action/RBridge
*
*/
static CMD_STATUS wrCpssDxChCfgPortDefaultSourceEportNumberGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_PHYSICAL_PORT_NUM               portNum;
    GT_PORT_NUM                        ePort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, &ePort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",ePort);
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgRemoteFcModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static CMD_STATUS wrCpssDxChCfgRemoteFcModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                               devNum;
    CPSS_DXCH_PORT_HOL_FC_ENT           modeFcHol;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet;
    GT_U32                              tcBitmap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    modeFcHol = (CPSS_DXCH_PORT_HOL_FC_ENT)inArgs[1];
    profileSet = (CPSS_PORT_TX_DROP_PROFILE_SET_ENT)inArgs[2];
    tcBitmap = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChCfgRemoteFcModeSet(devNum, modeFcHol, profileSet, tcBitmap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgReplicationCountersGet function
* @endinternal
*
* @brief   Get Outgoing Replication Counters.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
                                   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
static CMD_STATUS wrCpssDxChCfgReplicationCountersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                            result;
    GT_U8                                                devNum;
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT          replicationType;
    GT_U32                                               counter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    replicationType = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgReplicationCountersGet(
        devNum, replicationType, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d",
        counter);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgProbePacketConfigSet function
* @endinternal
*
* @brief Set the Probe packet configuration
*
* @note APPLICABLE DEVICES:      Falcon.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;
*                                DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                - device Number.
* @param[in] probeCfgPtr           - (Pointer to)Probe Packet configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - parameter value out of range.
*
*/
static CMD_STATUS wrCpssDxChCfgProbePacketConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                    result;
    GT_U8                                        devNum;
    CPSS_DXCH_CFG_PROBE_PACKET_STC               probeCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum               = (GT_U8)inArgs[0];
    probeCfg.bitLocation = (GT_U32)inArgs[1];
    probeCfg.enable      = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChCfgProbePacketConfigSet(
        devNum, &probeCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgProbePacketConfigGet function
* @endinternal
*
* @brief Get the Probe packet configuration
*
* @note APPLICABLE DEVICES:      Falcon.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;
*                                DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                - device Number.
* @param[in] probeCfgPtr           - (Pointer to)Probe Packet configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChCfgProbePacketConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                            result;
    GT_U8                                                devNum;
    CPSS_DXCH_CFG_PROBE_PACKET_STC                       probeCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum               = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChCfgProbePacketConfigGet(
        devNum, &probeCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", probeCfg.bitLocation,
                                          probeCfg.enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChCfgProbePacketDropCodeGet function
* @endinternal
*
* @brief Get the last Probe packet drop code.
*
* @note APPLICABLE DEVICES:      Falcon.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;
*                                DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                - device Number.
* @param[in] direction             - ingress/egress direction.
* @param[in] probeCfgPtr           - (Pointer to)Probe Packet configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static CMD_STATUS wrCpssDxChCfgProbePacketDropCodeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                            result;
    GT_U8                                                devNum;
    CPSS_NET_RX_CPU_CODE_ENT                             dropCode;
    CPSS_DIRECTION_ENT                                   direction;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum               = (GT_U8)inArgs[0];
    direction            = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChCfgProbePacketDropCodeGet(
        devNum, direction, &dropCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropCode);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChCfgDevRemove",
        &wrCpssDxChCfgDevRemove,
        1, 0},

    {"cpssDxChCfgDevEnable",
        &wrCpssDxChCfgDevEnable,
        2, 0},

    {"cpssDxChCfgDevEnableGet",
        &wrCpssDxChCfgDevEnableGet,
        1, 0},

    {"cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet",
        &wrCpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet,
        3, 0},

    {"cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet",
        &wrCpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet,
        2, 0},

    {"cpssDxChCfgDsaTagSrcDevPortModifySet",
        &wrCpssDxChCfgDsaTagSrcDevPortModifySet,
        2, 0},

    {"cpssDxChCfgDsaTagSrcDevPortModifyGet",
        &wrCpssDxChCfgDsaTagSrcDevPortModifyGet,
        1, 0},

    {"cpssDxChCfgTableNumEntriesGet",
        &wrCpssDxChCfgTableNumEntriesGet,
        2, 0},

    {"cpssDxChCfgReNumberDevNum",
        &wrCpssDxChCfgReNumberDevNum,
        2, 0},

    {"cpssDxChCfgIngressDropCntrSet",
        &wrCpssDxChCfgIngressDropCntrSet,
        2, 0},

    {"cpssDxChCfgIngressDropCntrGet",
        &wrCpssDxChCfgIngressDropCntrGet,
        1, 0},

    {"cpssDxChCfgIngressDropCntrModeSet",
        &wrCpssDxChCfgIngressDropCntrModeSet,
        4, 0},

    {"cpssDxChCfgIngressDropCntrModeGet",
        &wrCpssDxChCfgIngressDropCntrModeGet,
        1, 0},

    {"cpssDxChCfgGlobalEportSet",
        &wrCpssDxChCfgGlobalEportSet,
        7, 0},

    {"cpssDxChCfgGlobalEportGet",
        &wrCpssDxChCfgGlobalEportGet,
        1, 0},

    {"cpssDxChCfgDevInfoGet",
        &wrCpssDxChCfgDevInfoGet,
        1, 0},

    {"cpssDxChCfgHitlessWriteMethodEnableSet",
        &wrCpssDxChCfgHitlessWriteMethodEnableSet,
        2, 0},

    {"cpssDxChCfgPortDefaultSourceEportNumberSet",
        &wrCpssDxChCfgPortDefaultSourceEportNumberSet,
        3, 0},

    {"cpssDxChCfgPortDefaultSourceEportNumberGet",
        &wrCpssDxChCfgPortDefaultSourceEportNumberGet,
        2, 0},

    {"cpssDxChCfgRemoteFcModeSet",
        &wrCpssDxChCfgRemoteFcModeSet,
        4, 0},

    {"cpssDxChCfgReplicationCountersGet",
        &wrCpssDxChCfgReplicationCountersGet,
        2, 0},

    {"cpssDxChCfgProbePacketConfigSet",
        &wrCpssDxChCfgProbePacketConfigSet,
        3, 0},

    {"cpssDxChCfgProbePacketConfigGet",
        &wrCpssDxChCfgProbePacketConfigGet,
        1, 0},

    {"cpssDxChCfgProbePacketDropCodeGet",
        &wrCpssDxChCfgProbePacketDropCodeGet,
        2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChConfig function
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
GT_STATUS cmdLibInitCpssDxChConfig
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


