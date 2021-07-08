/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file cpssDxChProtection.h
*
* @brief CPSS DxCh Protection Switching APIs
*
* @version   4
********************************************************************************
*/

#ifndef __cpssDxChProtection
#define __cpssDxChProtection

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/**
* @enum CPSS_DXCH_PROTECTION_LOC_STATUS_ENT
 *
 * @brief LOC status on port
*/
typedef enum{

    /** LOC was not detected on the port */
    CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E = 0,

    /** LOC was detected on the port */
    CPSS_DXCH_PROTECTION_LOC_DETECTED_E

} CPSS_DXCH_PROTECTION_LOC_STATUS_ENT;

/**
* @internal cpssDxChProtectionEnableSet function
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
GT_STATUS cpssDxChProtectionEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChProtectionEnableGet function
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
GT_STATUS cpssDxChProtectionEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChProtectionTxEnableSet function
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
*       If the target ePort is enabled for TX protection switching and target
*       ePort's associated <LOC Status> is 1 (Loss of Continuity), then the target
*       ePort LSB is toggled and the packet is sent over the protection path.
*       It's meaningless to call this function with an odd port number. However
*       the function will set the entry if the port is odd and will return GT_OK
*
*/
GT_STATUS cpssDxChProtectionTxEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChProtectionTxEnableGet function
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
*       If the target ePort is enabled for TX protection switching and target
*       ePort's associated <LOC Status> is 1 (Loss of Continuity), then the target
*       ePort LSB is toggled and the packet is sent over the protection path.
*       It's meaningless to call this function with an odd port number. However
*       the function will get the entry if the port is odd and will return GT_OK
*
*/
GT_STATUS cpssDxChProtectionTxEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChProtectionPortToLocMappingSet function
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
GT_STATUS cpssDxChProtectionPortToLocMappingSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      locTableIndex
);

/**
* @internal cpssDxChProtectionPortToLocMappingGet function
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
GT_STATUS cpssDxChProtectionPortToLocMappingGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_U32      *locTableIndexPtr
);

/**
* @internal cpssDxChProtectionLocStatusSet function
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
GT_STATUS cpssDxChProtectionLocStatusSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  index,
    IN  CPSS_DXCH_PROTECTION_LOC_STATUS_ENT     status
);

/**
* @internal cpssDxChProtectionLocStatusGet function
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
GT_STATUS cpssDxChProtectionLocStatusGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  index,
    OUT CPSS_DXCH_PROTECTION_LOC_STATUS_ENT     *statusPtr
);

/**
* @internal cpssDxChProtectionRxExceptionPacketCommandSet function
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
GT_STATUS cpssDxChProtectionRxExceptionPacketCommandSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PACKET_CMD_ENT     command
);

/**
* @internal cpssDxChProtectionRxExceptionPacketCommandGet function
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
GT_STATUS cpssDxChProtectionRxExceptionPacketCommandGet
(
    IN  GT_U8                   devNum,
    OUT CPSS_PACKET_CMD_ENT     *commandPtr
);

/**
* @internal cpssDxChProtectionRxExceptionCpuCodeSet function
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
GT_STATUS cpssDxChProtectionRxExceptionCpuCodeSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode
);

/**
* @internal cpssDxChProtectionRxExceptionCpuCodeGet function
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
GT_STATUS cpssDxChProtectionRxExceptionCpuCodeGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *cpuCodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChProtection */


