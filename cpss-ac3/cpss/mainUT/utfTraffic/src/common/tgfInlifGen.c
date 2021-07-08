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
* @file tgfInlifGen.c
*
* @brief Generic API implementation for InLIF
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>

#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <common/tgfInlifGen.h>

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/inlif/cpssExMxPmInlif.h>
    #include <cpss/exMxPm/exMxPmGen/inlif/cpssExMxPmInlifTypes.h>
#endif /*EXMXPM_FAMILY*/

/**
* @internal prvTgfInlifPortModeSet function
* @endinternal
*
* @brief   The function configured the InLIf Lookup mode for the Ingress port.
*
* @param[in] portIndex                - port index
* @param[in] inlifMode                - InLIF Lookup Mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifPortModeSet
(
    IN  GT_U8                                portIndex,
    IN  PRV_TGF_INLIF_PORT_MODE_ENT          inlifMode
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portIndex);
    TGF_PARAM_NOT_USED(inlifMode);

    /* not relevant for DxCh, always return GT_OK */
    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_INLIF_PORT_MODE_ENT exMxPmInlifMode;

    switch (inlifMode)
    {
        case PRV_TGF_INLIF_PORT_MODE_PORT_E:
            exMxPmInlifMode = CPSS_EXMXPM_INLIF_PORT_MODE_PORT_E;
            break;
        case PRV_TGF_INLIF_PORT_MODE_VLAN_E:
            exMxPmInlifMode = CPSS_EXMXPM_INLIF_PORT_MODE_VLAN_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmInlifPortModeSet(prvTgfDevsArray[portIndex],
                                      prvTgfPortsArray[portIndex],
                                      exMxPmInlifMode);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfInlifPortModeGet function
* @endinternal
*
* @brief   Get the InLIf Lookup mode for the Ingress port.
*         Each port is independent and can work in every mode.
* @param[in] portIndex                - port index
*
* @param[out] inlifModePtr             - (pointer to)InLIF Lookup Mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifPortModeGet
(
    IN  GT_U8                            portIndex,
    OUT PRV_TGF_INLIF_PORT_MODE_ENT      *inlifModePtr
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portIndex);
    TGF_PARAM_NOT_USED(inlifModePtr);

    /* not relevant for DxCh, always return GT_OK */
    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS                       rc = GT_OK;
    CPSS_EXMXPM_INLIF_PORT_MODE_ENT exMxPmInlifMode;

    /* call device specific API */
    rc = cpssExMxPmInlifPortModeGet(prvTgfDevsArray[portIndex],
                                    prvTgfPortsArray[portIndex],
                                    &exMxPmInlifMode);
    if (rc != GT_OK)
        return rc;

    switch (exMxPmInlifMode)
    {
        case CPSS_EXMXPM_INLIF_PORT_MODE_PORT_E:
            *inlifModePtr = PRV_TGF_INLIF_PORT_MODE_PORT_E;
            break;
        case CPSS_EXMXPM_INLIF_PORT_MODE_VLAN_E:
            *inlifModePtr = PRV_TGF_INLIF_PORT_MODE_VLAN_E;
            break;
        default:
            return GT_BAD_STATE;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfInlifUnkUnregAddrCmdSet function
* @endinternal
*
* @brief   This function set the InLIF's unknown/unregistered addresses command
*
* @param[in] devNum                   - device number
* @param[in] inlifType                - inlif type port/vlan/external
* @param[in] inlifIndex               - inlif index in the table
* @param[in] addrType                 - Supported unkown/unreg addresses types
* @param[in] cmd                      - Flood:Flood the packet according to its VLAN assignment.
*                                      valid options:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_BRIDGE_E
*                                      Mirror:Mirror the packet to the CPU with specific CPU
*                                      code for Unknown Unicast.
*                                      Trap: Trap the packet to the CPU with specific CPU
*                                      code for Unknown Unicast.
*                                      Hard Drop: Hard Drop the packet.
*                                      Soft Drop: Soft Drop the packet.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not ExMxPm family
* @retval GT_FAIL                  - otherwise
*
* @note InLIF supported only for ExMxPm.
*
*/
GT_STATUS prvTgfInlifUnkUnregAddrCmdSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_INLIF_TYPE_ENT              inlifType,
    IN  GT_U32                              inlifIndex,
    IN  PRV_TGF_INLIF_UNK_UNREG_ADDR_ENT    addrType,
    IN  CPSS_PACKET_CMD_ENT                 cmd
)
{
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_INLIF_TYPE_ENT              exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
    CPSS_EXMXPM_INLIF_UNK_UNREG_ADDR_ENT    exMxPmAddrType = CPSS_EXMXPM_INLIF_UNK_UC_SA_E;

    /* convert to device specific format */
    switch (inlifType)
    {
        case PRV_TGF_INLIF_TYPE_PORT_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
            break;

        case PRV_TGF_INLIF_TYPE_VLAN_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_VLAN_E;
            break;

        case PRV_TGF_INLIF_TYPE_EXTERNAL_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_EXTERNAL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert to device specific format */
    switch (addrType)
    {
        case PRV_TGF_INLIF_UNK_UC_SA_E:
            exMxPmAddrType = CPSS_EXMXPM_INLIF_UNK_UC_SA_E;
            break;

        case PRV_TGF_INLIF_UNK_UC_DA_E:
            exMxPmAddrType = CPSS_EXMXPM_INLIF_UNK_UC_DA_E;
            break;

        case PRV_TGF_INLIF_UNREG_NON_IP_MC_E:
            exMxPmAddrType = CPSS_EXMXPM_INLIF_UNREG_NON_IP_MC_E;
            break;

        case PRV_TGF_INLIF_UNREG_NON_IP4_BC_E:
            exMxPmAddrType = CPSS_EXMXPM_INLIF_UNREG_NON_IP4_BC_E;
            break;

        case PRV_TGF_INLIF_UNREG_IP_MC_E:
            exMxPmAddrType = CPSS_EXMXPM_INLIF_UNREG_IP_MC_E;
            break;

        case PRV_TGF_INLIF_UNREG_IP_V4_BC_E:
            exMxPmAddrType = CPSS_EXMXPM_INLIF_UNREG_IP_V4_BC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmInlifUnkUnregAddrCmdSet(devNum, exMxPmInlifType,
                                             inlifIndex, exMxPmAddrType, cmd);
#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(inlifType);
    TGF_PARAM_NOT_USED(inlifIndex);
    TGF_PARAM_NOT_USED(addrType);
    TGF_PARAM_NOT_USED(cmd);

    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}


/**
* @internal prvTgfInlifIntForceVlanModeOnTtSet function
* @endinternal
*
* @brief   Set the Force-Vlan-Mode-on-TT (Enable/Disable).
*         Enabling it means that all packets that have a hit at the TT
*         (Tunnel-Termination) lookup are implicitly assgined
*         a per-VLAN inlif (no matter what is the port mode).
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE = Force: If TT rule hit, IPCL Configuration
*                                      table index - packet VLAN assignment
*                                      GT_FALSE = Don't Force: Doesn't effect the existing
*                                      Policy Configuration table access logic
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifIntForceVlanModeOnTtSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);

    /* not relevant for DxCh, always return GT_OK */
    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* call device specific API */
    return cpssExMxPmInlifIntForceVlanModeOnTtSet(devNum,
                                                  enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}

/**
* @internal prvTgfInlifIntForceVlanModeOnTtGet function
* @endinternal
*
* @brief   Get the Force-Vlan-Mode-on-TT status.
*         Enable means that all packets that have a hit at the TT
*         (Tunnel-Termination) lookup are implicitly assgined
*         a per-VLAN inlif (no matter what is the port mode).
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE = Force: If TT rule hit, IPCL Configuration
*                                      table index - packet VLAN assignment
*                                      GT_FALSE = Don't Force: Doesn't effect the existing
*                                      Policy Configuration table access logic
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifIntForceVlanModeOnTtGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);

    /* not relevant for DxCh, always return GT_OK */
    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* call device specific API */
    return cpssExMxPmInlifIntForceVlanModeOnTtGet(devNum,
                                                  enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}


/**
* @internal prvTgfInlifAutoLearnEnableSet function
* @endinternal
*
* @brief   Set the Automatic Learning (Disable/Enable).
*         When enabled, the device learns the new source MAC Addresses received
*         on this interface automatically and, according to the setting for <NA
*         Message To Cpu Enable>, sends or does not send NA messages to the
*         CPU.
*         When disabled the device does not learn the new source MAC Addresses
*         received on this interface automatically.
* @param[in] devNum                   - device number
* @param[in] inlifType                - inlif type port/vlan/external
* @param[in] inlifIndex               - inlif index in the table
*                                      (APPLICABLE RANGES: 0..63) for inLif type port or
*                                      (APPLICABLE RANGES: 0..4095) for inLif type vlan or
*                                      (APPLICABLE RANGES: Puma2 4096..65535; Puma3 4096..16383)
*                                      for inLif type external
* @param[in] enable                   - GT_TRUE = auto-learning enabled.
*                                      GT_FALSE = auto-learning disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifAutoLearnEnableSet
(
    IN GT_U8                        devNum,
    IN PRV_TGF_INLIF_TYPE_ENT       inlifType,
    IN GT_U32                       inlifIndex,
    IN GT_BOOL                      enable
)
{
#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(inlifType);
    TGF_PARAM_NOT_USED(inlifIndex);
    TGF_PARAM_NOT_USED(enable);

    /* not relevant for DxCh, always return GT_OK */
    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_INLIF_TYPE_ENT              exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_PORT_E;

    /* convert to device specific format */
    switch (inlifType)
    {
        case PRV_TGF_INLIF_TYPE_PORT_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
            break;

        case PRV_TGF_INLIF_TYPE_VLAN_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_VLAN_E;
            break;

        case PRV_TGF_INLIF_TYPE_EXTERNAL_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_EXTERNAL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmInlifAutoLearnEnableSet(devNum, exMxPmInlifType, inlifIndex, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}



