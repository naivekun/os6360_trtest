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
* @file tgfProtectionGen.c
*
* @brief Generic APIs for Protection Switching.
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <common/tgfProtectionGen.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/protection/cpssDxChProtection.h>
#endif /*CHX_FAMILY*/

/**
* @internal prvTgfProtectionEnableSet function
* @endinternal
*
* @brief   Globally enable/disable protection switching
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - protection switching status:
*                                      GT_TRUE: enable protection switching
*                                      GT_FALSE: disable protection switching
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfProtectionEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionEnableSet(devNum, enable);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionEnableGet function
* @endinternal
*
* @brief   Get the global enabling status of protection switching
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) protection switching status:
*                                      GT_TRUE: protection switching is enabled
*                                      GT_FALSE: protection switching is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfProtectionEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionEnableGet(devNum, enablePtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionTxEnableSet function
* @endinternal
*
* @brief   Enable/disable TX protection switching on port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - TX protection switching status on the port:
*                                      GT_TRUE: TX protection switching is enabled on the port
*                                      GT_FALSE: TX protection switching is disabled on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for 1:1 confguration:
*       The device checks if the target ePort is enabled for TX protection
*       switching. If the target ePort is disabled for TX protection switching
*       or the target ePort's associated <LOC Status> is ok, then the target
*       ePort LSB is toggled and the packet is sent over the protection path.
*
*/
GT_STATUS prvTgfProtectionTxEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionTxEnableSet(devNum, portNum, enable);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionTxEnableGet function
* @endinternal
*
* @brief   Get the enabling status of TX protection switching on port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) TX protection switching status on the port:
*                                      GT_TRUE: TX protection switching is enabled on the port
*                                      GT_FALSE: TX protection switching is disabled on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for 1:1 confguration:
*       The device checks if the target ePort is enabled for TX protection
*       switching. If the target ePort is disabled for TX protection switching
*       or the target ePort's associated <LOC Status> is ok, then the target
*       ePort LSB is toggled and the packet is sent over the protection path.
*
*/
GT_STATUS prvTgfProtectionTxEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionTxEnableGet(devNum, portNum, enablePtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionPortToLocMappingSet function
* @endinternal
*
* @brief   Set the mapping between a port to a LOC (Loss of Continuity) status bit
*         index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] locTableIndex            - an index in the Protection LOC table
*                                      (APPLICABLE RANGES: 0..2047)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfProtectionPortToLocMappingSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      locTableIndex
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionPortToLocMappingSet(devNum, portNum, locTableIndex);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(locTableIndex);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionPortToLocMappingGet function
* @endinternal
*
* @brief   Get the mapping between a port to a LOC (Loss of Continuity) status bit
*         index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] locTableIndexPtr         - (pointer to) an index in the Protection LOC table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfProtectionPortToLocMappingGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_U32      *locTableIndexPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionPortToLocMappingGet(devNum, portNum, locTableIndexPtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(locTableIndexPtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}
/**
* @internal prvTgfProtectionLocStatusSet function
* @endinternal
*
* @brief   Set LOC (Loss of Continuity) status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - LOC index
*                                      (APPLICABLE RANGES: 0..2047)
* @param[in] status                   - LOC  for the specified index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The LOC status is set automatically by the device when LOC is detected
*       if <Enable Protection LOC Update> and <Keepalive Aging Enable> are set
*       in the corresponding OAM entry.
*
*/
GT_STATUS prvTgfProtectionLocStatusSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  index,
    IN  PRV_TGF_PROTECTION_LOC_STATUS_ENT       status
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_PROTECTION_LOC_STATUS_ENT   dxChStatus;
    switch (status)
    {
        case PRV_TGF_PROTECTION_LOC_NOT_DETECTED_E:
            dxChStatus = CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E;
            break;
    
        case PRV_TGF_PROTECTION_LOC_DETECTED_E:
            dxChStatus = CPSS_DXCH_PROTECTION_LOC_DETECTED_E;
            break;
    
        default:
            return GT_BAD_PARAM;
    }
    return cpssDxChProtectionLocStatusSet(devNum, index, dxChStatus);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(status);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionLocStatusGet function
* @endinternal
*
* @brief   Get LOC (Loss of Continuity) status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - LOC index
*                                      (APPLICABLE RANGES: 0..2047)
*
* @param[out] statusPtr                - (pointer to) LOC status for the specified index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfProtectionLocStatusGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  index,
    OUT PRV_TGF_PROTECTION_LOC_STATUS_ENT       *statusPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                               rc;
    CPSS_DXCH_PROTECTION_LOC_STATUS_ENT     dxChStatus;

    if (statusPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = cpssDxChProtectionLocStatusGet(devNum, index, &dxChStatus);
    if (rc == GT_OK)
    {
        switch (dxChStatus)
        {
            case CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E:
                *statusPtr = PRV_TGF_PROTECTION_LOC_NOT_DETECTED_E;
                break;

            case CPSS_DXCH_PROTECTION_LOC_DETECTED_E:
                *statusPtr = PRV_TGF_PROTECTION_LOC_DETECTED_E;
                break;

            default:
                return GT_BAD_PARAM;
        }
    }
    return rc;
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(statusPtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionRxExceptionPacketCommandSet function
* @endinternal
*
* @brief   Set the packet command for protection switching RX exception.
*         RX exception occurs if one of the following happens:
*         - traffic receiced from the working path (in TTI action: <RX Enable
*         Protection Switching> = 1 and <RX Is Protection Path> = 0) and the LOC
*         bit associated with the working path is set
*         - traffic receiced from the protection path (in TTI action: <RX Enable
*         Protection Switching> = 1 and <RX Is Protection Path> = 1) and the LOC
*         bit associated with the working path is unset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  - the packet command
*                                      (APPLICABLE VALUES:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfProtectionRxExceptionPacketCommandSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PACKET_CMD_ENT     command
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionRxExceptionPacketCommandSet(devNum, command);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(command);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionRxExceptionPacketCommandGet function
* @endinternal
*
* @brief   Get the packet command for protection switching RX exception.
*         RX exception occurs if one of the following happens:
*         - traffic receiced from the working path (in TTI action: <RX Enable
*         Protection Switching> = 1 and <RX Is Protection Path> = 0) and the LOC
*         bit associated with the working path is set
*         - traffic receiced from the protection path (in TTI action: <RX Enable
*         Protection Switching> = 1 and <RX Is Protection Path> = 1) and the LOC
*         bit associated with the working path is unset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] commandPtr               - (pointer to) the packet command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfProtectionRxExceptionPacketCommandGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionRxExceptionPacketCommandGet(devNum, commandPtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(commandPtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionRxExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set the packet CPU/drop code for protection switching RX exception
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CPU/drop code is not relevant when the protection switching RX
*       exception packet command is configured to CPSS_PACKET_CMD_FORWARD_E.
*
*/
GT_STATUS prvTgfProtectionRxExceptionCpuCodeSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionRxExceptionCpuCodeSet(devNum, cpuCode);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCode);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfProtectionRxExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get the packet CPU/drop code for protection switching RX exception
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodePtr               - (pointer to) the CPU/drop code
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CPU/drop code is not relevant when the protection switching RX
*       exception packet command is configured to CPSS_PACKET_CMD_FORWARD_E.
*
*/
GT_STATUS prvTgfProtectionRxExceptionCpuCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChProtectionRxExceptionCpuCodeGet(devNum, cpuCodePtr);
#endif  /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cpuCodePtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}


