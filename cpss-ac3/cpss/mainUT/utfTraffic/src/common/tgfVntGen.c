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
* @file tgfVntGen.c
*
* @brief Generic API for Virtual Network Tester (VNT) Technology facility API.
* VNT features:
* - Operation, Administration, and Maintenance (OAM).
* - Connectivity Fault Management (CFM).
*
* @version   9
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
#include <common/tgfVntGen.h>
#include <common/tgfPclGen.h>

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfVntOamPortLoopBackModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable 802.3ah Loopback mode on the specified port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
*/
GT_STATUS prvTgfVntOamPortLoopBackModeEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntOamPortLoopBackModeEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmVntOamPortLoopbackModeEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntOamPortLoopBackModeEnableGet function
* @endinternal
*
* @brief   Get 802.3ah Loopback mode on the specified port
*
* @param[out] enablePtr                - (pointer to) Loopback mode status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntOamPortLoopBackModeEnableGet
(
    IN  GT_U8                         portNum,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntOamPortLoopBackModeEnableGet(prvTgfDevsArray[portIter], portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmVntOamPortLoopbackModeEnableGet(prvTgfDevsArray[portIter], portNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntOamPortUnidirectionalEnableSet function
* @endinternal
*
* @brief   Enable/Disable the port for unidirectional transmit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
*/
GT_STATUS prvTgfVntOamPortUnidirectionalEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (portIter >= prvTgfPortsNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntOamPortUnidirectionalEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmVntOamPortUnidirectionalEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntOamPortUnidirectionalEnableGet function
* @endinternal
*
* @brief   Gets the current status of unidirectional transmit for the port
*
* @param[out] enablePtr                - (pointer to) current status of unidirectional transmit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntOamPortUnidirectionalEnableGet
(
    IN  GT_U8                         portNum,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntOamPortUnidirectionalEnableGet(prvTgfDevsArray[portIter], portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmVntOamPortUnidirectionalEnableGet(prvTgfDevsArray[portIter], portNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntCfmEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CFM (Connectivity Fault Management) PDUs
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong etherType values
*/
GT_STATUS prvTgfVntCfmEtherTypeSet
(
    IN GT_U32                         etherType
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
        rc = cpssDxChVntCfmEtherTypeSet(devNum, etherType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChVntCfmEtherTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    if(rc1 != GT_OK)
    {
        return rc1;
    }

    /* set also UDE0 for the PCL */
    return prvTgfPclUdeEtherTypeSet(0/*UDE0*/,etherType);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* set also UDE0 for the PCL */
    return prvTgfPclUdeEtherTypeSet(0/*UDE0*/,etherType);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntCfmEtherTypeGet function
* @endinternal
*
* @brief   Gets the EtherType to identify CFM (Connectivity Fault Management) PDUs
*
* @param[in] devNum                   - device number
*
* @param[out] etherTypePtr             - (pointer to) CFM (Connectivity Fault Management) EtherType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntCfmEtherTypeGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                       *etherTypePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntCfmEtherTypeGet(devNum, etherTypePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
     /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(etherTypePtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntCfmLbrOpcodeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management) LBR (Loopback Response) opcode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, opcode.
* @retval GT_OUT_OF_RANGE          - on wrong opcode values.
*/
GT_STATUS prvTgfVntCfmLbrOpcodeSet
(
    IN GT_U32                         opcode
)
{

    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChVntCfmLbrOpcodeSet(devNum, opcode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChVntCfmLbrOpcodeSet FAILED, rc = [%d]", rc);

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
        rc = cpssExMxPmVntCfmLbrOpcodeSet(devNum, opcode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmVntCfmLbrOpcodeSet FAILED, rc = [%d]", rc);

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
* @internal prvTgfVntCfmLbrOpcodeGet function
* @endinternal
*
* @brief   Gets the current CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode
* @param[in] devNum                   - device number
*
* @param[out] opcodePtr                - (pointer to) CFM LBR Opcode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntCfmLbrOpcodeGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                       *opcodePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntCfmLbrOpcodeGet(devNum, opcodePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
     /* call device specific API */
    return cpssExMxPmVntCfmLbrOpcodeGet(devNum, opcodePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntCfmReplyModeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
* @param[in] mode                     - CFM Opcode change mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfVntCfmReplyModeSet
(
    IN PRV_TGF_VNT_CFM_REPLY_MODE_ENT   mode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT dxChMode;

    switch(mode)
    {
        case PRV_TGF_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E:
            dxChMode = CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E;
            break;
        case PRV_TGF_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E:
            dxChMode = CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(
        &devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChVntCfmReplyModeSet(devNum, dxChMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]:cpssDxChVntCfmReplyModeSet  FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mode);
    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntCfmReplyModeGet function
* @endinternal
*
* @brief   Gets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) CFM Opcode change mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - output parameter is NULL pointer.
*/
GT_STATUS prvTgfVntCfmReplyModeGet
(
    IN GT_U8                            devNum,
    IN PRV_TGF_VNT_CFM_REPLY_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT dxChMode;

    /* call device specific API */
    rc = cpssDxChVntCfmReplyModeGet(devNum, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChVntCfmReplyModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(dxChMode)
    {
        case CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E:
            *modePtr = PRV_TGF_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E;
            break;
        case CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E:
            *modePtr = PRV_TGF_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E;
            break;
        default:
            return GT_BAD_STATE;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(modePtr);
    return GT_BAD_STATE;
#endif /* EXMXPM_FAMILY) */
}

/**
* @internal prvTgfVntLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers
* @param[in] devNum                   - device number
*
* @param[out] timeStampValuePtr        - (pointer to) timestamp of the last read operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntLastReadTimeStampGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                       *timeStampValuePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntLastReadTimeStampGet(devNum, timeStampValuePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
     /* call device specific API */
    return cpssExMxPmVntLastReadTimeStampGet(devNum, timeStampValuePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfVntOamPortPduTrapEnableSet function
* @endinternal
*
* @brief   Enable/Disable trap to CPU of 802.3ah Link Layer Control protocol on
*         the specified port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
GT_STATUS prvTgfVntOamPortPduTrapEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
    GT_U32  portIter = 0;

    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntOamPortPduTrapEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* call device specific API */
    return cpssExMxPmVntOamPortPduTrapEnableSet(prvTgfDevsArray[portIter], portNum, enable);
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfVntOamPortPduTrapEnableGet function
* @endinternal
*
* @brief   Get trap to CPU status(Enable/Disable) of 802.3ah Link Layer Control protocol on
*         the specified port.
*
* @param[out] enablePtr                - pointer to Loopback mode status:
*                                      GT_TRUE:  Enable trap to CPU of 802.3ah Link Layer Control protocol.
*                                      GT_FALSE: Disable trap to CPU of 802.3ah Link Layer Control protocol.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - on bad pointer of enablePtr
*/
GT_STATUS prvTgfVntOamPortPduTrapEnableGet
(
    IN  GT_U8                         portNum,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChVntOamPortPduTrapEnableGet(prvTgfDevsArray[portIter], portNum, enablePtr);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* call device specific API */
    return cpssExMxPmVntOamPortPduTrapEnableGet(prvTgfDevsArray[portIter], portNum, enablePtr);
#endif /* !(defined CHX_FAMILY) */
}


