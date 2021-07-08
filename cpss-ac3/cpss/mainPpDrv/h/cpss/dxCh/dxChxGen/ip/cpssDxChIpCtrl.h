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
* @file cpssDxChIpCtrl.h
*
* @brief the CPSS DXCH Ip HW control registers APIs
*
* @version   55
********************************************************************************
*/

#ifndef __cpssDxChIpCtrlh
#define __cpssDxChIpCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>

/**
* @internal cpssDxChIpSpecialRouterTriggerEnable function
* @endinternal
*
* @brief   Sets the special router trigger enable modes for packets with bridge
*         command other then FORWARD or MIRROR.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] bridgeExceptionCmd       - the bridge exception command the packet arrived to
*                                      the router with.
* @param[in] enableRouterTrigger      - enable /disable router trigger.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or bridgeExceptionCmd
*/
GT_STATUS cpssDxChIpSpecialRouterTriggerEnable
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd,
    IN GT_BOOL                         enableRouterTrigger
);

/**
* @internal cpssDxChIpSpecialRouterTriggerEnableGet function
* @endinternal
*
* @brief   Gets the special router trigger enable modes for packets with bridge
*         command other then FORWARD or MIRROR.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] bridgeExceptionCmd       - the bridge exception command the packet arrived to
*                                      the router with.
*
* @param[out] enableRouterTriggerPtr   - router trigger state (enable/disable).
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or bridgeExceptionCmd
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpSpecialRouterTriggerEnableGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd,
    OUT GT_BOOL                         *enableRouterTriggerPtr
);

/**
* @internal cpssDxChIpExceptionCommandSet function
* @endinternal
*
* @brief   set a specific exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] exceptionType            - the exception type
* @param[in] protocolStack            - whether to set for ipv4/v6 or both
* @param[in] command                  - the command, for availble commands see
*                                      CPSS_DXCH_IP_EXCEPTION_TYPE_ENT.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpExceptionCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN CPSS_PACKET_CMD_ENT              command
);

/**
* @internal cpssDxChIpExceptionCommandGet function
* @endinternal
*
* @brief   get a specific exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] exceptionType            - the exception type
* @param[in] protocolStack            - whether to get for ipv4/v6
*
* @param[out] exceptionCmdPtr          - the command, for availble commands see
*                                      CPSS_DXCH_IP_EXCEPTION_TYPE_ENT
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
*/
GT_STATUS cpssDxChIpExceptionCommandGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT CPSS_PACKET_CMD_ENT             *exceptionCmdPtr
);

/**
* @internal cpssDxChIpHeaderErrorMaskSet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to mask/unmask the error for IPv4 or IPv6
* @param[in] prefixType               - whether to mask/unmask the error for unicast or
*                                      multicast
* @param[in] mask                     - GT_TRUE:  the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS cpssDxChIpHeaderErrorMaskSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_IP_HEADER_ERROR_ENT        ipHeaderErrorType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT           prefixType,
    IN GT_BOOL                              mask
);

/**
* @internal cpssDxChIpHeaderErrorMaskGet function
* @endinternal
*
* @brief   Get the masking status of an IP header error.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to check the masking status for IPv4 or IPv6
* @param[in] prefixType               - whether to check the masking status for unicast or
*                                      multicast
*
* @param[out] maskPtr                  - GT_TRUE: mask the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS cpssDxChIpHeaderErrorMaskGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_IP_HEADER_ERROR_ENT       ipHeaderErrorType,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN  CPSS_UNICAST_MULTICAST_ENT          prefixType,
    OUT GT_BOOL                             *maskPtr
);

/**
* @internal cpssDxChIpUcRouteAgingModeSet function
* @endinternal
*
* @brief   Sets the global route aging modes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] refreshEnable            - Enables the global routing activity refresh mechanism
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpUcRouteAgingModeSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  refreshEnable
);

/**
* @internal cpssDxChIpUcRouteAgingModeGet function
* @endinternal
*
* @brief   Gets the global route aging modes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] refreshEnablePtr         - Enables the global routing activity refresh mechanism
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpUcRouteAgingModeGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *refreshEnablePtr
);

/**
* @internal cpssDxChIpRouterSourceIdSet function
* @endinternal
*
* @brief   set the router source id assignmnet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
* @param[in] sourceId                 - the assigned source id.
* @param[in] sourceIdMask             - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterSourceIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
);

/**
* @internal cpssDxChIpRouterSourceIdGet function
* @endinternal
*
* @brief   get the router source id.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
*
* @param[out] sourceIdPtr              - the assigned source id.
* @param[out] sourceIdMaskPtr          - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterSourceIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
);

/**
* @internal cpssDxChIpPortGroupRouterSourceIdSet function
* @endinternal
*
* @brief   set the router source id assignmnet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
* @param[in] sourceId                 - the assigned source id.
* @param[in] sourceIdMask             - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupRouterSourceIdSet
(
    IN  GT_U8                           devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
);

/**
* @internal cpssDxChIpPortGroupRouterSourceIdGet function
* @endinternal
*
* @brief   get the router source id assignmnet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
*
* @param[out] sourceIdPtr              - the assigned source id.
* @param[out] sourceIdMaskPtr          - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpPortGroupRouterSourceIdGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
);

/**
* @internal cpssDxChIpRouterSourceIdOverrideEnableSet function
* @endinternal
*
* @brief   Enable/Disable overriding of source id by routing engine.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] enable                   - GT_TRUE  -  override
*                                      GT_FALSE - disable override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*
*/
GT_STATUS cpssDxChIpRouterSourceIdOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssDxChIpRouterSourceIdOverrideEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of overriding of source id
*         by routing engine.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
*
* @param[out] enablePtr                - (pointer to)enable:
*                                      GT_TRUE  - enable override
*                                      GT_FALSE - disable override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*
*/
GT_STATUS cpssDxChIpRouterSourceIdOverrideEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet function
* @endinternal
*
* @brief   Sets the multi-target TC queue assigned to multi-target Control
*         packets and to RPF Fail packets where the RPF Fail Command is assigned
*         from the MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] ctrlMultiTargetTCQueue   - The multi-target TC queue for control
*                                      traffic. There are 4 possible queues (APPLICABLE RANGES: 0..3)
* @param[in] failRpfMultiTargetTCQueue - the multi-target TC queue for fail rpf
*                                      traffic. There are 4 possible queues (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as a multi-target control if it is a FROM_CPU DSA
*       Tagged with DSA<Use_Vidx> = 1 or a multi-target packet that is also to
*       be mirrored to the CPU
*
*/
GT_STATUS cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ctrlMultiTargetTCQueue,
    IN  GT_U32  failRpfMultiTargetTCQueue
);

/**
* @internal cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet function
* @endinternal
*
* @brief   Gets the multi-target TC queue assigned to multi-target Control
*         packets and to RPF Fail packets where the RPF Fail Command is assigned
*         from the MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] ctrlMultiTargetTCQueuePtr - The multi-target TC queue for control
*                                      traffic. There are 4 possible queues
* @param[out] failRpfMultiTargetTCQueuePtr - the multi-target TC queue for fail rpf
*                                      traffic. There are 4 possible queues
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*
* @note A packet is considered as a multi-target control if it is a FROM_CPU DSA
*       Tagged with DSA<Use_Vidx> = 1 or a multi-target packet that is also to
*       be mirrored to the CPU
*
*/
GT_STATUS cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet
(
    IN   GT_U8   devNum,
    OUT  GT_U32  *ctrlMultiTargetTCQueuePtr,
    OUT  GT_U32  *failRpfMultiTargetTCQueuePtr
);

/**
* @internal cpssDxChIpQosProfileToMultiTargetTCQueueMapSet function
* @endinternal
*
* @brief   Sets the Qos Profile to multi-target TC queue mapping.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] qosProfile               - QOS Profile index
* @param[in] multiTargetTCQueue       - the multi-target TC queue. There are 4 possible
*                                      queues.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpQosProfileToMultiTargetTCQueueMapSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    IN  GT_U32  multiTargetTCQueue
);

/**
* @internal cpssDxChIpQosProfileToMultiTargetTCQueueMapGet function
* @endinternal
*
* @brief   gets the Qos Profile to multi-target TC queue mapping.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] qosProfile               - QOS Profile index
*
* @param[out] multiTargetTCQueuePtr    - the multi-target TC queue. There are 4 possible
*                                      queues.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpQosProfileToMultiTargetTCQueueMapGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    OUT GT_U32  *multiTargetTCQueuePtr
);



/**
* @internal cpssDxChIpTcDpToMultiTargetTcQueueMapSet function
* @endinternal
*
* @brief   Sets the packet's (TC,DP) to multi-target TC queue mapping.
*         (It is used for packets received on cascade ports configured to work in extended QoS mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] tc                       - traffic class assigned to the packet.
*                                      (received on a cascade port configured for extended Global QoS mode)
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence assigned to the packet for tail drop.
*                                      (received on a cascade port configured for extended Global QoS mode)
* @param[in] multiTargetTCQueue       - the multi-target TC queue. (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpTcDpToMultiTargetTcQueueMapSet
(
    IN  GT_U8              devNum,
    IN  GT_U32             tc,
    IN  CPSS_DP_LEVEL_ENT  dp,
    IN  GT_U32             multiTargetTCQueue
);

/**
* @internal cpssDxChIpTcDpToMultiTargetTcQueueMapGet function
* @endinternal
*
* @brief   Gets the packet's (TC,DP) to multi-target TC queue mapping.
*         (It is used for packets received on cascade ports configured to work in extended QoS mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] tc                       - traffic class assigned to the packet.
*                                      (received on a cascade port configured for extended Global QoS mode)
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence assigned to the packet for tail drop.
*                                      (received on a cascade port configured for extended Global QoS mode)
*
* @param[out] multiTargetTCQueuePtr    - the multi-target TC queue. (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpTcDpToMultiTargetTcQueueMapGet
(
    IN  GT_U8              devNum,
    IN  GT_U32             tc,
    IN  CPSS_DP_LEVEL_ENT  dp,
    OUT  GT_U32            *multiTargetTCQueuePtr
);




/**
* @internal cpssDxChIpMultiTargetQueueFullDropCntGet function
* @endinternal
*
* @brief   Get the multi target queue full drop packet counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] dropPktsPtr              - the number of counted dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMultiTargetQueueFullDropCntGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *dropPktsPtr
);

/**
* @internal cpssDxChIpMultiTargetQueueFullDropCntSet function
* @endinternal
*
* @brief   set the multi target queue full drop packet counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] dropPkts                 - the counter value to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMultiTargetQueueFullDropCntSet
(
    IN GT_U8     devNum,
    IN GT_U32    dropPkts
);

/**
* @internal cpssDxChIpMultiTargetTCQueueSchedModeSet function
* @endinternal
*
* @brief   sets the multi-target TC queue scheduling mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] multiTargetTcQueue       - the multi-target TC queue. There are 4 possible
*                                      queues.
* @param[in] schedulingMode           - the scheduling mode.
* @param[in] queueWeight              - the queue weight for SDWRR scheduler (APPLICABLE RANGES: 0..255)
*                                      (relvant only if schedMode =
*                                      CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMultiTargetTCQueueSchedModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  multiTargetTcQueue,
    IN  CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT schedulingMode,
    IN  GT_U32                                  queueWeight
);

/**
* @internal cpssDxChIpMultiTargetTCQueueSchedModeGet function
* @endinternal
*
* @brief   gets the multi-target TC queue scheduling mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] multiTargetTcQueue       - the multi-target TC queue. There are 4 possible
*                                      queues.
*
* @param[out] schedulingModePtr        - the scheduling mode.
* @param[out] queueWeightPtr           - the queue weight for SDWRR scheduler
*                                      (relvant only if schedMode =
*                                      CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpMultiTargetTCQueueSchedModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  multiTargetTcQueue,
    OUT CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT *schedulingModePtr,
    OUT GT_U32                                  *queueWeightPtr
);

/**
* @internal cpssDxChIpBridgeServiceEnable function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
* @param[in] enableService            - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpBridgeServiceEnable
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode,
    IN  GT_BOOL                                             enableService
);

/**
* @internal cpssDxChIpBridgeServiceEnableGet function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
*
* @param[out] enableServicePtr         - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChIpBridgeServiceEnableGet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode,
    OUT GT_BOOL                                             *enableServicePtr
);

/**
* @internal cpssDxChIpRouterBridgedPacketsExceptionCntGet function
* @endinternal
*
* @brief   Get exception counter for Special Services for Bridged Traffic.
*         it counts the number of Bridged packets failing any of the following checks:
*         - SIP Filter check for bridged IPv4/6 packets
*         - IP Header Check for bridged IPv4/6 packets
*         - Unicast RPF check for bridged IPv4/6 and ARP packets
*         - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] routerBridgedExceptionPktsPtr - the number of counted router bridged
*                                      exception  packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterBridgedPacketsExceptionCntGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *routerBridgedExceptionPktsPtr
);

/**
* @internal cpssDxChIpRouterBridgedPacketsExceptionCntSet function
* @endinternal
*
* @brief   Set exception counter for Special Services for Bridged Traffic.
*         it counts the number of Bridged packets failing any of the following checks:
*         - SIP Filter check for bridged IPv4/6 packets
*         - IP Header Check for bridged IPv4/6 packets
*         - Unicast RPF check for bridged IPv4/6 and ARP packets
*         - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] routerBridgedExceptionPkts - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterBridgedPacketsExceptionCntSet
(
    IN   GT_U8    devNum,
    IN  GT_U32    routerBridgedExceptionPkts
);

/**
* @internal cpssDxChIpMllBridgeEnable function
* @endinternal
*
* @brief   enable/disable MLL based bridging.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllBridgeEnable          - enable /disable MLL based bridging.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllBridgeEnable
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    mllBridgeEnable
);

/**
* @internal cpssDxChIpMllBridgeEnableGet function
* @endinternal
*
* @brief   get state of MLL based bridging.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] mllBridgeEnablePtr       - enable /disable MLL based bridging.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL prt
*/
GT_STATUS cpssDxChIpMllBridgeEnableGet
(
    IN    GT_U8      devNum,
    OUT   GT_BOOL    *mllBridgeEnablePtr
);

/**
* @internal cpssDxChIpMultiTargetRateShaperSet function
* @endinternal
*
* @brief   set the Multi target Rate shaper params.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] multiTargetRateShaperEnable - weather to enable (disable) the Multi
*                                      target Rate shaper
* @param[in] windowSize               - if enabled then this is the shaper window size (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMultiTargetRateShaperSet
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    multiTargetRateShaperEnable,
    IN   GT_U32     windowSize
);

/**
* @internal cpssDxChIpMultiTargetRateShaperGet function
* @endinternal
*
* @brief   set the Multi target Rate shaper params.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
*
* @param[out] multiTargetRateShaperEnablePtr - weather to enable (disable) the Multi
*                                      target Rate shaper
* @param[out] windowSizePtr            - if enabled then this is the shaper window size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMultiTargetRateShaperGet
(
    IN    GT_U8      devNum,
    OUT   GT_BOOL    *multiTargetRateShaperEnablePtr,
    OUT   GT_U32     *windowSizePtr
);

/**
* @internal cpssDxChIpMultiTargetUcSchedModeSet function
* @endinternal
*
* @brief   set the Multi target/unicast sheduler mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] ucSPEnable               weather the Unicast uses SP , if GT_FALSE then it means
*                                      both the Unicast and multi target uses DSWRR scheduling
* @param[in] ucWeight                 - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                                      is the unicast weight (APPLICABLE RANGES: 0..255).
* @param[in] mcWeight                 - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                                      is the multi target weight (APPLICABLE RANGES: 0..255).
* @param[in] schedMtu                 - The MTU used by the scheduler
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMultiTargetUcSchedModeSet
(
    IN   GT_U8                              devNum,
    IN   GT_BOOL                            ucSPEnable,
    IN   GT_U32                             ucWeight,
    IN   GT_U32                             mcWeight,
    IN   CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   schedMtu
);

/**
* @internal cpssDxChIpMultiTargetUcSchedModeGet function
* @endinternal
*
* @brief   get the Multi target/unicast sheduler mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] ucSPEnablePtr            weather the Unicast uses SP , if GT_FALSE then it means
*                                      both the Unicast and multi target uses DSWRR scheduling
* @param[out] ucWeightPtr              - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                                      is the unicast weight (APPLICABLE RANGES: 0..255).
* @param[out] mcWeightPtr              - if DSWRR scheduler is used (ucSPEnable==GT_FALSE) then this
*                                      is the multi target weight (APPLICABLE RANGES: 0..255).
* @param[out] schedMtuPtr              - The MTU used by the scheduler
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_BAD_STATE             - on invalid hardware value read
*/
GT_STATUS cpssDxChIpMultiTargetUcSchedModeGet
(
    IN   GT_U8                              devNum,
    OUT  GT_BOOL                            *ucSPEnablePtr,
    OUT  GT_U32                             *ucWeightPtr,
    OUT  GT_U32                             *mcWeightPtr,
    OUT  CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   *schedMtuPtr
);

/**
* @internal cpssDxChIpArpBcModeSet function
* @endinternal
*
* @brief   set a arp broadcast mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] arpBcMode                - the arp broadcast command. Possible Commands:
*                                      CPSS_PACKET_CMD_NONE_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS cpssDxChIpArpBcModeSet
(
    IN GT_U8               devNum,
    IN CPSS_PACKET_CMD_ENT arpBcMode
);

/**
* @internal cpssDxChIpArpBcModeGet function
* @endinternal
*
* @brief   get a arp broadcast mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] arpBcModePtr             - the arp broadcast command. Possible Commands:
*                                      CPSS_PACKET_CMD_NONE_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note
*       GalTis:
*
*/
GT_STATUS cpssDxChIpArpBcModeGet
(
    IN  GT_U8               devNum,
    OUT CPSS_PACKET_CMD_ENT *arpBcModePtr
);

/**
* @internal cpssDxChIpPortRoutingEnable function
* @endinternal
*
* @brief   Enable multicast/unicast IPv4/v6 routing on a port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port to enable on
* @param[in] ucMcEnable               - routing type to enable Unicast/Multicast
* @param[in] protocolStack            - what type of traffic to enable ipv4 or ipv6 or both.
* @param[in] enableRouting            - enable IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortRoutingEnable
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_IP_UNICAST_MULTICAST_ENT    ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN GT_BOOL                          enableRouting
);

/**
* @internal cpssDxChIpPortRoutingEnableGet function
* @endinternal
*
* @brief   Get status of multicast/unicast IPv4/v6 routing on a port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port to enable on
* @param[in] ucMcEnable               - routing type to enable Unicast/Multicast
* @param[in] protocolStack            - what type of traffic to enable ipv4 or ipv6 or both.
*
* @param[out] enableRoutingPtr         - (pointer to)enable IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note CPSS_IP_PROTOCOL_IPV4V6_E is not supported in this get API.
*       Can not get both values for ipv4 and ipv6 in the same get.
*
*/
GT_STATUS cpssDxChIpPortRoutingEnableGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT    ucMcEnable,
    IN  CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT GT_BOOL                          *enableRoutingPtr
);

/**
* @internal cpssDxChIpPortFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Enable FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port to enable on
* @param[in] enable                   -  FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortFcoeForwardingEnableSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN GT_BOOL                          enable
);

/**
* @internal cpssDxChIpPortFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Get status of FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port to enable on
*
* @param[out] enablePtr                - (pointer to)enable FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChIpPortFcoeForwardingEnableGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    OUT GT_BOOL                          *enablePtr
);

/**
* @internal cpssDxChIpQosProfileToRouteEntryMapSet function
* @endinternal
*
* @brief   Sets the QoS profile to route entry offset mapping table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] qosProfile               - QOS Profile index
* @param[in] routeEntryOffset         - The offset in the array of route entries to be
*                                      selected for this QOS profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In QoS-based routing, the Route table entry is selected according to the
*       following index calculation:
*       1. in xCat3: <Route Entry Index> +
*       (QoSProfile-to-Route-Block-Offset(QoSProfile) %
*       (<Number of Paths> + 1)
*       2. in Lion : <Route Entry Index> +
*       Floor(QoSProfile-to-Route-Block-Offset(QoSProfile)
*       (<Number of Paths> + 1) / 8)
*
*/
GT_STATUS cpssDxChIpQosProfileToRouteEntryMapSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               qosProfile,
    IN  GT_U32               routeEntryOffset
);

/**
* @internal cpssDxChIpQosProfileToRouteEntryMapGet function
* @endinternal
*
* @brief   gets the QoS profile to route entry offset mapping table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] qosProfile               - QOS Profile index
*
* @param[out] routeEntryOffsetPtr      - The offset in the array of route entries to be
*                                      selected for this QOS profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*
* @note In QoS-based routing, the Route table entry is selected according to the
*       following index calculation:
*       1. in xCat3: <Route Entry Index> +
*       (QoSProfile-to-Route-Block-Offset(QoSProfile) %
*       (<Number of Paths> + 1)
*       2. in Lion : <Route Entry Index> +
*       Floor(QoSProfile-to-Route-Block-Offset(QoSProfile)
*       (<Number of Paths> + 1) / 8)
*
*/
GT_STATUS cpssDxChIpQosProfileToRouteEntryMapGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               qosProfile,
    OUT GT_U32               *routeEntryOffsetPtr
);

/**
* @internal cpssDxChIpRoutingEnable function
* @endinternal
*
* @brief   globally enable/disable routing.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] enableRouting            - enable /disable global routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note the ASIC defualt is routing enabled.
*
*/
GT_STATUS cpssDxChIpRoutingEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enableRouting
);

/**
* @internal cpssDxChIpRoutingEnableGet function
* @endinternal
*
* @brief   Get global routing status
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] enableRoutingPtr         -  (pointer to)enable /disable global routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChIpRoutingEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enableRoutingPtr
);

/**
* @internal cpssDxChIpCntGet function
* @endinternal
*
* @brief   Return the IP counter set requested.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] cntSet                   - counters set to retrieve.
*
* @param[out] countersPtr              - (pointer to)struct contains the counter values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function doesn't handle counters overflow.
*
*/
GT_STATUS cpssDxChIpCntGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_CNT_SET_ENT          cntSet,
    OUT CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);

/**
* @internal cpssDxChIpCntSetModeSet function
* @endinternal
*
* @brief   Sets a counter set's bounded inteface and interface mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] cntSet                   - the counter set
* @param[in] cntSetMode               - the counter set bind mode (interface or Next hop)
* @param[in] interfaceModeCfgPtr      - if cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E
*                                      this is the interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpCntSetModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_IP_CNT_SET_ENT                        cntSet,
    IN  CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode,
    IN  CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceModeCfgPtr
);

/**
* @internal cpssDxChIpCntSetModeGet function
* @endinternal
*
* @brief   Gets a counter set's bounded inteface and interface mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] cntSet                   - the counter set
*
* @param[out] cntSetModePtr            - the counter set bind mode (interface or Next hop)
* @param[out] interfaceModeCfgPtr      - if cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E
*                                      this is the interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpCntSetModeGet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_IP_CNT_SET_ENT                        cntSet,
    OUT CPSS_DXCH_IP_CNT_SET_MODE_ENT              *cntSetModePtr,
    OUT CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceModeCfgPtr
);

/**
* @internal cpssDxChIpCntSet function
* @endinternal
*
* @brief   set the requested IP counter set.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] cntSet                   - counters set to clear.
* @param[in] countersPtr              - the counter values to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpCntSet
(
    IN GT_U8                         devNum,
    IN CPSS_IP_CNT_SET_ENT           cntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);


/**
* @internal cpssDxChIpMllSkippedEntriesCountersGet function
* @endinternal
*
* @brief   Get MLL entries skipped counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] skipCounterPtr           - (pointer to) number of MLL entries skipped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllSkippedEntriesCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *skipCounterPtr
);

/**
* @internal cpssDxChIpMllPortGroupSkippedEntriesCountersGet function
* @endinternal
*
* @brief   Get MLL entries skipped counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] skipCounterPtr           - (pointer to) number of MLL entries skipped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllPortGroupSkippedEntriesCountersGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_GROUPS_BMP       portGroupsBmp,
    OUT GT_U32                   *skipCounterPtr
);

/**
* @internal cpssDxChIpSetMllCntInterface function
* @endinternal
*
* @brief   Sets a mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - mll counter set
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] interfaceCfgPtr          - the mll counter interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpSetMllCntInterface
(
    IN GT_U8                                       devNum,
    IN GT_U32                                      mllCntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceCfgPtr
);

/**
* @internal cpssDxChIpMllCntGet function
* @endinternal
*
* @brief   Get the mll counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - the mll counter set out of the 2
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of routed IP Multicast packets Duplicated by the
*                                      MLL Engine and transmitted via this interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
);

/**
* @internal cpssDxChIpMllCntSet function
* @endinternal
*
* @brief   set an mll counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - the mll counter set out of the 2
* @param[in] mllOutMCPkts             - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllCntSet
(
    IN GT_U8    devNum,
    IN GT_U32   mllCntSet,
    IN GT_U32   mllOutMCPkts
);


/**
* @internal cpssDxChIpMllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the MLL priority queues.
*         A silent drop is a drop that is applied to a replica of the packet that
*         was previously replicated in the TTI.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllSilentDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *silentDropPktsPtr
);

/**
* @internal cpssDxChIpMllPortGroupSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the MLL priority queues.
*         A silent drop is a drop that is applied to a replica of the packet that
*         was previously replicated in the TTI.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllPortGroupSilentDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *silentDropPktsPtr
);

/**
* @internal cpssDxChIpDropCntSet function
* @endinternal
*
* @brief   set the drop counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] dropPkts                 - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpDropCntSet
(
    IN GT_U8 devNum,
    IN GT_U32 dropPkts
);

/**
* @internal cpssDxChIpSetDropCntMode function
* @endinternal
*
* @brief   Sets the drop counter count mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] dropCntMode              - the drop counter count mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpSetDropCntMode
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_IP_DROP_CNT_MODE_ENT dropCntMode
);

/**
* @internal cpssDxChIpGetDropCntMode function
* @endinternal
*
* @brief   Gets the drop counter count mode.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] dropCntModePtr           - (pointer to) the drop counter count mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpGetDropCntMode
(
    IN  GT_U8                          devNum,
    OUT CPSS_DXCH_IP_DROP_CNT_MODE_ENT *dropCntModePtr
);

/**
* @internal cpssDxChIpDropCntGet function
* @endinternal
*
* @brief   Get the drop counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] dropPktsPtr              - the number of counted dropped packets according to the
*                                      drop counter mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *dropPktsPtr
);

/**
* @internal cpssDxChIpMtuProfileSet function
* @endinternal
*
* @brief   Sets the next hop interface MTU profile limit value.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mtu                      - the mtu profile index out of the possible 8 (APPLICABLE RANGES: 0..7).
* @param[in] mtu                      - the maximum transmission unit (APPLICABLE RANGES: 0..0x3FFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMtuProfileSet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    IN GT_U32 mtu
);

/**
* @internal cpssDxChIpMtuProfileGet function
* @endinternal
*
* @brief   Gets the next hop interface MTU profile limit value.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] mtuProfileIndex          - the mtu profile index out of the possible 8 (APPLICABLE RANGES: 0..7).
*
* @param[out] mtuPtr                   - the maximum transmission unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChIpMtuProfileGet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    OUT GT_U32 *mtuPtr
);


/**
* @internal cpssDxChIpv6AddrPrefixScopeSet function
* @endinternal
*
* @brief   Defines a prefix of a scope type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] prefixPtr                - a pointer to IPv6 address prefix
* @param[in] prefixLen                - length of the prefix (APPLICABLE RANGES: 0..16)
* @param[in] addressScope             - type of the address scope spanned by the prefix
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configures an entry in the prefix look up table
*
*/
GT_STATUS cpssDxChIpv6AddrPrefixScopeSet
(
    IN  GT_U8                           devNum,
    IN  GT_IPV6ADDR                    *prefixPtr,
    IN  GT_U32                          prefixLen,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope,
    IN  GT_U32                          prefixScopeIndex
);

/**
* @internal cpssDxChIpv6AddrPrefixScopeGet function
* @endinternal
*
* @brief   Get a prefix of a scope type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @param[out] prefixPtr                - an IPv6 address prefix
* @param[out] prefixLenPtr             - length of the prefix
* @param[out] addressScopePtr          - type of the address scope spanned by the prefix
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpv6AddrPrefixScopeGet
(
    IN  GT_U8                           devNum,
    OUT GT_IPV6ADDR                     *prefixPtr,
    OUT GT_U32                          *prefixLenPtr,
    OUT CPSS_IPV6_PREFIX_SCOPE_ENT      *addressScopePtr,
    IN  GT_U32                          prefixScopeIndex
);

/**
* @internal cpssDxChIpv6UcScopeCommandSet function
* @endinternal
*
* @brief   sets the ipv6 Unicast scope commands.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
* @param[in] scopeCommand             - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpv6UcScopeCommandSet
(
    IN GT_U8                      devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest,
    IN GT_BOOL                    borderCrossed,
    IN CPSS_PACKET_CMD_ENT        scopeCommand
);

/**
* @internal cpssDxChIpv6UcScopeCommandGet function
* @endinternal
*
* @brief   gets the ipv6 Unicast scope commands.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
*
* @param[out] scopeCommandPtr          - action to be done on a packet that match the above
*                                      scope configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpv6UcScopeCommandGet
(
    IN  GT_U8                      devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest,
    IN  GT_BOOL                    borderCrossed,
    OUT CPSS_PACKET_CMD_ENT        *scopeCommandPtr
);

/**
* @internal cpssDxChIpv6McScopeCommandSet function
* @endinternal
*
* @brief   sets the ipv6 Multicast scope commands.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
* @param[in] scopeCommand             - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[in] mllSelectionRule         - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpv6McScopeCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN GT_BOOL                          borderCrossed,
    IN CPSS_PACKET_CMD_ENT              scopeCommand,
    IN CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule
);

/**
* @internal cpssDxChIpv6McScopeCommandGet function
* @endinternal
*
* @brief   gets the ipv6 Multicast scope commands.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
*
* @param[out] scopeCommandPtr          - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[out] mllSelectionRulePtr      - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChIpv6McScopeCommandGet
(
    IN  GT_U8                            devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN  GT_BOOL                          borderCrossed,
    OUT CPSS_PACKET_CMD_ENT              *scopeCommandPtr,
    OUT CPSS_IPV6_MLL_SELECTION_RULE_ENT *mllSelectionRulePtr
);

/**
* @internal cpssDxChIpRouterMacSaBaseSet function
* @endinternal
*
* @brief   Sets 40 MSBs of Router MAC SA Base address on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
);

/**
* @internal cpssDxChIpRouterMacSaBaseGet function
* @endinternal
*
* @brief   Get 40 MSBs of Router MAC SA Base address on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
);

/**
* @internal cpssDxChIpPortRouterMacSaLsbModeSet function
* @endinternal
*
* @brief   Sets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] saLsbMode                - The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device 5 most significant bytes are set by cpssDxChIpRouterMacSaBaseSet().
*       The least significant bytes are set by:
*       Port mode is set by cpssDxChIpRouterPortMacSaLsbSet().
*       Vlan mode is set by cpssDxChIpRouterVlanMacSaLsbSet().
*
*/
GT_STATUS cpssDxChIpPortRouterMacSaLsbModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode
);


/**
* @internal cpssDxChIpPortRouterMacSaLsbModeGet function
* @endinternal
*
* @brief   Gets the mode, per port, in which the device sets the packet's MAC SA least
*         significant bytes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] saLsbModePtr             - (pointer to) The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortRouterMacSaLsbModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_MAC_SA_LSB_MODE_ENT    *saLsbModePtr
);

/**
* @internal cpssDxChIpRouterPortMacSaLsbSet function
* @endinternal
*
* @brief   Sets the 8 LSB Router MAC SA for this EGRESS PORT.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - Eggress Port number
* @param[in] saMac                    - The 8 bits SA mac value to be written to the SA bits of
*                                      routed packet if SA alteration mode is configured to
*                                      take LSB according to Eggress Port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterPortMacSaLsbSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    saMac
);

/**
* @internal cpssDxChIpRouterPortMacSaLsbGet function
* @endinternal
*
* @brief   Gets the 8 LSB Router MAC SA for this EGRESS PORT.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - Eggress Port number
*
* @param[out] saMacPtr                 - (pointer to) The 8 bits SA mac value written to the SA
*                                      bits of routed packet if SA alteration mode is configured
*                                      to take LSB according to Eggress Port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterPortMacSaLsbGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8                   *saMacPtr
);

/**
* @internal cpssDxChIpRouterVlanMacSaLsbSet function
* @endinternal
*
* @brief   Sets the LSBs of Router MAC SA for this VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlan                     - VLAN Id
*                                      (APPLICABLE RANGES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2 0..4095).
*                                      (APPLICABLE RANGES: Falcon 0..8191).
* @param[in] saMac                    - The Low Significant bits of SA mac value
*                                      be written to the SA bits of routed packet
*                                      if SA alteration mode is configured to
*                                      take LSB according to VLAN.
*                                      (APPLICABLE RANGES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2 0..255).
*                                      (APPLICABLE RANGES: Falcon 0..4095).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_OUT_OF_RANGE          - wrong saMac
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterVlanMacSaLsbSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlan,
    IN GT_U32   saMac
);

/**
* @internal cpssDxChIpRouterVlanMacSaLsbGet function
* @endinternal
*
* @brief   Gets the LSBs of Router MAC SA for this VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlan                     - VLAN Id
*                                      (APPLICABLE RANGES: xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2 0..4095).
*                                      (APPLICABLE RANGES: Falcon 0..8191).
* @param[out] saMacPtr                 - (pointer to) The Low Significant bits of SA mac value
*                                      be written to the SA bits of routed packet
*                                      if SA alteration mode is configured to
*                                      take LSB according to VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterVlanMacSaLsbGet
(
    IN  GT_U8    devNum,
    IN  GT_U16   vlan,
    OUT GT_U32   *saMacPtr
);

/**
* @internal cpssDxChIpRouterGlobalMacSaSet function
* @endinternal
*
* @brief   Sets full 48-bit Router MAC SA in Global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.(APPLICABLE RANGES: 0..255)
* @param[in] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API is applicable when per-egress-physical-port MAC SA assignment
*       mode is configured to global :CPSS_SA_LSB_FULL_48_BIT_GLOBAL by API
*       cpssDxChIpPortRouterMacSaLsbModeSet. The routerMacSaIndex is configured
*       by cpssDxChIpRouterPortGlobalMacSaIndexSet.
*
*/
GT_STATUS cpssDxChIpRouterGlobalMacSaSet
(
    IN GT_U8        devNum,
    IN GT_U32       routerMacSaIndex,
    IN GT_ETHERADDR *macSaAddrPtr
);

/**
* @internal cpssDxChIpRouterGlobalMacSaGet function
* @endinternal
*
* @brief   Gets full 48-bit Router MAC SA from Global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.(APPLICABLE RANGES: 0..255)
*
* @param[out] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API is applicable when per-egress-physical-port MAC SA assignment
*       mode is configured to global :CPSS_SA_LSB_FULL_48_BIT_GLOBAL by API
*       cpssDxChIpPortRouterMacSaLsbModeSet. The routerMacSaIndex is configured
*       by cpssDxChIpRouterPortGlobalMacSaIndexSet.
*
*/
GT_STATUS cpssDxChIpRouterGlobalMacSaGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       routerMacSaIndex,
    OUT GT_ETHERADDR *macSaAddrPtr
);

/**
* @internal cpssDxChIpRouterPortGlobalMacSaIndexSet function
* @endinternal
*
* @brief   Set router mac sa index refered to global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] routerMacSaIndex         - global MAC SA table index.(APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpRouterPortGlobalMacSaIndexSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               routerMacSaIndex
);

/**
* @internal cpssDxChIpRouterPortGlobalMacSaIndexGet function
* @endinternal
*
* @brief   Get router mac sa index refered to global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] routerMacSaIndexPtr      - (pointer to) global MAC SA table index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChIpRouterPortGlobalMacSaIndexGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *routerMacSaIndexPtr
);

/**
* @internal cpssDxChIpRouterMacSaModifyEnable function
* @endinternal
*
* @brief   Per Egress port bit Enable Routed packets MAC SA Modification
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
* @param[in] enable                   - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterMacSaModifyEnable
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  GT_BOOL                    enable
);

/**
* @internal cpssDxChIpRouterMacSaModifyEnableGet function
* @endinternal
*
* @brief   Per Egress port bit Get Routed packets MAC SA Modification State
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
*
* @param[out] enablePtr                - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterMacSaModifyEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal cpssDxChIpEcmpUcRpfCheckEnableSet function
* @endinternal
*
* @brief   Globally enables/disables ECMP/QoS unicast RPF check.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] enable                   -  / disable ECMP/Qos unicast RPF check
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpEcmpUcRpfCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChIpEcmpUcRpfCheckEnableGet function
* @endinternal
*
* @brief   Gets globally enables/disables ECMP/QoS unicast RPF check state.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @param[out] enablePtr                - (points to) enable / disable ECMP/Qos unicast RPF check
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpEcmpUcRpfCheckEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpPortGroupCntSet function
* @endinternal
*
* @brief   set the requested IP counter set.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] cntSet                   - counters set to clear.
* @param[in] countersPtr              - the counter values to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupCntSet
(
    IN GT_U8                         devNum,
    IN GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN CPSS_IP_CNT_SET_ENT           cntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);

/**
* @internal cpssDxChIpPortGroupCntGet function
* @endinternal
*
* @brief   Return the IP counter set requested.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] cntSet                   - counters set to retrieve.
*
* @param[out] countersPtr              - (pointer to)struct contains the counter values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function doesn't handle counters overflow.
*
*/
GT_STATUS cpssDxChIpPortGroupCntGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN  CPSS_IP_CNT_SET_ENT          cntSet,
    OUT CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
);

/**
* @internal cpssDxChIpPortGroupDropCntSet function
* @endinternal
*
* @brief   set the drop counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] dropPkts                 - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupDropCntSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               dropPkts
);

/**
* @internal cpssDxChIpPortGroupDropCntGet function
* @endinternal
*
* @brief   Get the drop counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] dropPktsPtr              - the number of counted dropped packets according to the
*                                      drop counter mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *dropPktsPtr
);

/**
* @internal cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet function
* @endinternal
*
* @brief   Get the multi target queue full drop packet counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] dropPktsPtr              - the number of counted dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet
(
    IN   GT_U8                  devNum,
    IN   GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT  GT_U32                 *dropPktsPtr
);

/**
* @internal cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet function
* @endinternal
*
* @brief   set the multi target queue full drop packet counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] dropPkts                 - the counter value to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet
(
    IN GT_U8                 devNum,
    IN GT_PORT_GROUPS_BMP    portGroupsBmp,
    IN GT_U32                dropPkts
);

/**
* @internal cpssDxChIpPortGroupMllCntGet function
* @endinternal
*
* @brief   Get the mll counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] mllCntSet                - the mll counter set out of the 2
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of routed IP Multicast packets Duplicated by the
*                                      MLL Engine and transmitted via this interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupMllCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllCntSet,
    OUT GT_U32                  *mllOutMCPktsPtr
);

/**
* @internal cpssDxChIpPortGroupMllCntSet function
* @endinternal
*
* @brief   set an mll counter.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] mllCntSet                - the mll counter set out of the 2
* @param[in] mllOutMCPkts             - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupMllCntSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_GROUPS_BMP       portGroupsBmp,
    IN GT_U32                   mllCntSet,
    IN GT_U32                   mllOutMCPkts
);

/**
* @internal cpssDxChIpUcRpfModeSet function
* @endinternal
*
* @brief   Defines the uRPF check mode for a given VID.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
* @param[in] uRpfMode                 - CPSS_DXCH_IP_URPF_DISABLE_MODE_E = uRPF check is disabled
*                                      for this VID.
*                                      CPSS_DXCH_IP_URPF_VLAN_MODE_E= Vlan: If ECMP uRPF is
*                                      globally enabled, then uRPF check is done
*                                      by comparing the packet VID to the VID
*                                      in the additional route entry, otherwise
*                                      it is done using the SIP-Next Hop Entry VID.
*                                      CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E = uRPF check is done by
*                                      comparing the packet source {device,port}/Trunk to
*                                      the SIP-Next Hop Entry {device,port}/Trunk.
*                                      CPSS_DXCH_IP_URPF_LOOSE_MODE_E = uRPF check is done by checking
*                                      the SIP-Next Hop Entry Route Command.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, vid or uRpfMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS cpssDxChIpUcRpfModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    IN  CPSS_DXCH_IP_URPF_MODE_ENT      uRpfMode
);

/**
* @internal cpssDxChIpUcRpfModeGet function
* @endinternal
*
* @brief   Read uRPF check mode for a given VID.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
*
* @param[out] uRpfModePtr              -  CPSS_DXCH_IP_URPF_DISABLE_MODE_E = uRPF check is disabled
*                                      for this VID.
*                                      CPSS_DXCH_IP_URPF_VLAN_MODE_E= Vlan: If ECMP uRPF is
*                                      globally enabled, then uRPF check is done
*                                      by comparing the packet VID to the VID
*                                      in the additional route entry, otherwise
*                                      it is done using the SIP-Next Hop Entry VID.
*                                      CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E = uRPF check is done by
*                                      comparing the packet source {device,port}/Trunk to
*                                      the SIP-Next Hop Entry {device,port}/Trunk.
*                                      CPSS_DXCH_IP_URPF_LOOSE_MODE_E = uRPF check is done by checking
*                                      the SIP-Next Hop Entry Route Command.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS cpssDxChIpUcRpfModeGet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vid,
    OUT CPSS_DXCH_IP_URPF_MODE_ENT          *uRpfModePtr
);

/**
* @internal cpssDxChIpUrpfLooseModeTypeSet function
* @endinternal
*
* @brief   This function set type of Urpf loose mode
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   - the device number
* @param[in] looseModeType            - value of urpf loose mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChIpUrpfLooseModeTypeSet
(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT looseModeType
);

/**
* @internal cpssDxChIpUrpfLooseModeTypeGet function
* @endinternal
*
* @brief   This function get type of Urpf loose mode
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   - the device number
*
* @param[out] looseModeTypePtr         - pointer to urpf loose mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChIpUrpfLooseModeTypeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT * looseModeTypePtr
);

/**
* @internal cpssDxChIpPortSipSaEnableSet function
* @endinternal
*
* @brief   Enable SIP/SA check for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
* @param[in] enable                   - GT_FALSE: disable SIP/SA check on the port
*                                      GT_TRUE:  enable SIP/SA check on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS cpssDxChIpPortSipSaEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
);

/**
* @internal cpssDxChIpPortSipSaEnableGet function
* @endinternal
*
* @brief   Return the SIP/SA check status for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
*
* @param[out] enablePtr                - GT_FALSE: SIP/SA check on the port is disabled
*                                      GT_TRUE:  SIP/SA check on the port is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS cpssDxChIpPortSipSaEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal cpssDxChIpPortGroupMultiTargetRateShaperSet function
* @endinternal
*
* @brief   set the Multi target Rate shaper params.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] multiTargetRateShaperEnable - weather to enable (disable) the Multi
*                                      target Rate shaper
* @param[in] windowSize               - if enabled then this is the shaper window size (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi port-group device, the multi-target rate shaping
*       shapes a traffic on per-port-group basis according to the
*       source port of the packet.
*
*/
GT_STATUS cpssDxChIpPortGroupMultiTargetRateShaperSet
(
    IN  GT_U8              devNum,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    IN  GT_BOOL            multiTargetRateShaperEnable,
    IN  GT_U32             windowSize
);

/**
* @internal cpssDxChIpPortGroupMultiTargetRateShaperGet function
* @endinternal
*
* @brief   get the Multi target Rate shaper params.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] multiTargetRateShaperEnablePtr - weather to enable (disable) the Multi
*                                      target Rate shaper
* @param[out] windowSizePtr            - if enabled then this is the shaper window size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi port-group device, the multi-target rate shaping
*       shapes a traffic on per-port-group basis according to the
*       source port of the packet.
*
*/
GT_STATUS cpssDxChIpPortGroupMultiTargetRateShaperGet
(
    IN  GT_U8              devNum,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    OUT GT_BOOL            *multiTargetRateShaperEnablePtr,
    OUT GT_U32             *windowSizePtr
);

/**
* @internal cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet function
* @endinternal
*
* @brief   Set exception counter for Special Services for Bridged Traffic.
*         it counts the number of Bridged packets failing any of the following checks:
*         - SIP Filter check for bridged IPv4/6 packets
*         - IP Header Check for bridged IPv4/6 packets
*         - Unicast RPF check for bridged IPv4/6 and ARP packets
*         - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] routerBridgedExceptionPkts - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               routerBridgedExceptionPkts
);

/**
* @internal cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet function
* @endinternal
*
* @brief   Get exception counter for Special Services for Bridged Traffic.
*         it counts the number of Bridged packets failing any of the following checks:
*         - SIP Filter check for bridged IPv4/6 packets
*         - IP Header Check for bridged IPv4/6 packets
*         - Unicast RPF check for bridged IPv4/6 and ARP packets
*         - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] routerBridgedExceptionPktsPtr - the number of counted router bridged
*                                      exception  packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *routerBridgedExceptionPktsPtr
);

/**
* @internal cpssDxChIpUcRoutingVid1AssignEnableSet function
* @endinternal
*
* @brief   Enable/disable VID1 assignment by the unicast routing
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] enable                   - enable/disable VID1 assignment by the unicast routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*
* @note When VID1 assignment is enabled QoS attributes can't be set by the
*       router and the following fields in the unicast nexthop entries are not
*       applicable:
*       qosProfileMarkingEnable, qosProfileIndex, qosPrecedence, modifyUp,
*       modifyDscp.
*       When VID1 assignment is disabled QoS attributes are applicable and
*       nextHopVlanId1 field in the unicast nexthop entries is not applicable.
*       It's recommended to use this API before configuring the nexthop entries.
*       Using this API when nexthops are already configured can cause unexpected
*       results.
*
*/
GT_STATUS cpssDxChIpUcRoutingVid1AssignEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChIpUcRoutingVid1AssignEnableGet function
* @endinternal
*
* @brief   Get the enabling status of VID1 assignment by the unicast routing
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] enablePtr                - (pointer to) VID1 unicast routing assignment enabling status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*
* @note When VID1 assignment is enabled QoS attributes can't be set by the
*       router and the following fields in the unicast nexthop entries are not
*       applicable:
*       qosProfileMarkingEnable, qosProfileIndex, qosPrecedence, modifyUp,
*       modifyDscp.
*       When VID1 assignment is disabled QoS attributes are applicable and
*       nextHopVlanId1 field in the unicast nexthop entries is not applicable.
*
*/
GT_STATUS cpssDxChIpUcRoutingVid1AssignEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpMllMultiTargetShaperBaselineSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline,
*         the respective packet is not served.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes
*                                      (APPLICABLE RANGES: 0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU (the maximum expected packet size in the system).
*       2. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       cpssDxChIpMllMultiTargetShaperMtuSet.
*
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperBaselineSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseline
);

/**
* @internal cpssDxChIpMllMultiTargetShaperBaselineGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective packet
*         is not served.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperBaselineGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *baselinePtr
);

/**
* @internal cpssDxChIpMllMultiTargetShaperMtuSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] mtu                      - MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperMtuSet
(
    IN GT_U8   devNum,
    IN GT_U32  mtu
);

/**
* @internal cpssDxChIpMllMultiTargetShaperMtuGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] mtuPtr                   -  pointer to MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperMtuGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mtuPtr
);

/**
* @internal cpssDxChIpMllMultiTargetShaperTokenBucketModeSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Mode Mll shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] tokenBucketMode          -   Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperTokenBucketModeSet
(
    IN  GT_U8                                        devNum,
    IN  CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            tokenBucketMode
);

/**
* @internal cpssDxChIpMllMultiTargetShaperTokenBucketModeGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Mode Mll shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketModePtr       - (pointer to)Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperTokenBucketModeGet
(
    IN  GT_U8                                        devNum,
    OUT CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            *tokenBucketModePtr
);

/**
* @internal cpssDxChIpMllMultiTargetShaperEnableSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Enable/Disable Token Bucket rate shaping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  cpssDxChIpMllMultiTargetShaperEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChIpMllMultiTargetShaperEnableGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Enable/Disable Token Bucket rate shaping status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - Pointer to Token Bucket rate shaping status.
*                                      - GT_TRUE, enable Shaping
*                                      - GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
);


/**
* @internal cpssDxChIpMllMultiTargetShaperConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] maxBucketSize            - Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 means 4k and 0xFFF means 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to shaper mode.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       see:
*       cpssDxChIpMllMultiTargetShaperMtuSet
*       cpssDxChIpMllMultiTargetShaperBaselineSet.
*
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperConfigurationSet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       maxBucketSize,
    INOUT GT_U32                                     *maxRatePtr
);

/**
* @internal cpssDxChIpMllMultiTargetShaperConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] maxBucketSizePtr         - (pointer to) Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 equal 4k all 12'bFFF equal 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperConfigurationGet
(
    IN  GT_U8                                        devNum,
    OUT GT_U32                                       *maxBucketSizePtr,
    OUT GT_U32                                       *maxRatePtr
);

/**
* @internal cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper Interval.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] tokenBucketIntervalSlowUpdateRatio - Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[in] tokenBucketUpdateInterval -   Defines the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[in] tokenBucketIntervalUpdateRatio - Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          tokenBucketIntervalSlowUpdateRatio,
    IN  GT_U32                                          tokenBucketUpdateInterval,
    IN  CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   tokenBucketIntervalUpdateRatio
);

/**
* @internal cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper Interval.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketIntervalSlowUpdateRatioPtr - (pointer to) Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[out] tokenBucketUpdateIntervalPtr -   (pointer to) the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[out] tokenBucketIntervalUpdateRatioPtr - (pointer to) Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet
(
    IN   GT_U8                                           devNum,
    OUT  GT_U32                                          *tokenBucketIntervalSlowUpdateRatioPtr,
    OUT  GT_U32                                          *tokenBucketUpdateIntervalPtr,
    OUT  CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   *tokenBucketIntervalUpdateRatioPtr
);

/**
* @internal cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet function
* @endinternal
*
* @brief   Enable/disable bypassing the router triggering requirements for policy
*         based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  bypassing the router triggering requirements for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet function
* @endinternal
*
* @brief   Get the enabling status of bypassing the router triggering requirements
*         for policy based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - the router triggering requirements enabling status for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpVlanMrstBitmapSet function
* @endinternal
*
* @brief   Set the next-hop MRST state bitmap per eVLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] mrstBmpPtr               - Pointer to Multicast
*       Routing Shared Tree ID 64 bit bitmap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpVlanMrstBitmapSet
(
    IN  GT_U8     devNum,
    IN  GT_U16    vlanId,
    IN  GT_U64    *mrstBmpPtr
);

/**
* @internal cpssDxChIpVlanMrstBitmapGet function
* @endinternal
*
* @brief   Get the next-hop MRST state bitmap per eVLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] mrstBmpPtr               - pointer to Multicast Routing Shared Tree ID 64 bit bitmap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpVlanMrstBitmapGet
(
    IN  GT_U8     devNum,
    IN  GT_U16    vlanId,
    OUT GT_U64    *mrstBmpPtr
);


/**
* @internal cpssDxChIpFcoeExceptionPacketCommandSet function
* @endinternal
*
* @brief   Set packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChIpFcoeExceptionPacketCommandSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PACKET_CMD_ENT                 command
);

/**
* @internal cpssDxChIpFcoeExceptionPacketCommandGet function
* @endinternal
*
* @brief   Get packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] commandPtr               - points to the command for invalid FCoE packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChIpFcoeExceptionPacketCommandGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/**
* @internal cpssDxChIpFcoeExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set FCoE exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] cpuCode                  - If the FCoE exception packet command is TRAP, MIRROR or Drop
*                                      the CPU Code is determined by this field.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeExceptionCpuCodeSet
(
    IN  GT_U8                          devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT       cpuCode
);

/**
* @internal cpssDxChIpFcoeExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get FCoE exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] cpuCodePtr               - (points to) the CPU Code used if the FCoE exception
*                                      packet command is TRAP, MIRROR or Drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *cpuCodePtr
);

/**
* @internal cpssDxChIpFcoeSoftDropRouterEnableSet function
* @endinternal
*
* @brief   When enabled, FCoE packets with a SOFT_DROP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeSoftDropRouterEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChIpFcoeSoftDropRouterEnableGet function
* @endinternal
*
* @brief   Return if FCoE packets with a SOFT_DROP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeSoftDropRouterEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);


/**
* @internal cpssDxChIpFcoeTrapRouterEnableSet function
* @endinternal
*
* @brief   When enabled, FCoE packets with a TRAP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeTrapRouterEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChIpFcoeTrapRouterEnableGet function
* @endinternal
*
* @brief   Return if FCoE packets with a TRAP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeTrapRouterEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpFcoeBridgedUrpfCheckEnableSet function
* @endinternal
*
* @brief   When enabled, Unicast RPF check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedUrpfCheckEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChIpFcoeBridgedUrpfCheckEnableGet function
* @endinternal
*
* @brief   Return if Unicast RPF check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedUrpfCheckEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpFcoeBridgedUrpfCheckCommandSet function
* @endinternal
*
* @brief   Set packet command assigned to FCoE traffic that fails the UC RPF check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChIpFcoeBridgedUrpfCheckCommandSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PACKET_CMD_ENT                 command
);

/**
* @internal cpssDxChIpFcoeBridgedUrpfCheckCommandGet function
* @endinternal
*
* @brief   Get packet command assigned to FCoE traffic that fails the UC RPF check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] commandPtr               - points to the command for invalid FCoE packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChIpFcoeBridgedUrpfCheckCommandGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/**
* @internal cpssDxChIpFcoeUcRpfAccessLevelSet function
* @endinternal
*
* @brief   Set the SIP Access Level for FCoE Unicast packets where SIP is associated
*         with ECMP block, and Unicast RPF with ECMP is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] accessLevel              - SIP Access Level
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeUcRpfAccessLevelSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              accessLevel
);

/**
* @internal cpssDxChIpFcoeUcRpfAccessLevelGet function
* @endinternal
*
* @brief   Get the SIP Access Level for FCoE Unicast packets where SIP is associated
*         with ECMP block, and Unicast RPF with ECMP is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] accessLevelPtr           - points to the SIP Access Level
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeUcRpfAccessLevelGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *accessLevelPtr
);

/**
* @internal cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet function
* @endinternal
*
* @brief   When enabled, S_ID / SA mismatch check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet function
* @endinternal
*
* @brief   Return if S_ID / SA mismatch check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpFcoeBridgedSidSaMismatchCommandSet function
* @endinternal
*
* @brief   Set packet command for S_ID/SA mismatch exception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChIpFcoeBridgedSidSaMismatchCommandSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PACKET_CMD_ENT                 command
);

/**
* @internal cpssDxChIpFcoeBridgedSidSaMismatchCommandGet function
* @endinternal
*
* @brief   Get packet command for S_ID/SA exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] commandPtr               - points to the command for invalid FCoE packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChIpFcoeBridgedSidSaMismatchCommandGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/**
* @internal cpssDxChIpFcoeBridgedSidFilterEnableSet function
* @endinternal
*
* @brief   When enabled, S_ID filtering is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedSidFilterEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChIpFcoeBridgedSidFilterEnableGet function
* @endinternal
*
* @brief   Return if S_ID filtering is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedSidFilterEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet function
* @endinternal
*
* @brief   When enabled, FCoE header error checking is performed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet function
* @endinternal
*
* @brief   Return if FCoE header error checking is performed,
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChIpFdbRoutePrefixLenSet function
* @endinternal
*
* @brief   set the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
* @param[in] prefixLen                - FDB lookup prefix length.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFdbRoutePrefixLenSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U32                       prefixLen
);

/**
* @internal cpssDxChIpFdbRoutePrefixLenGet function
* @endinternal
*
* @brief   Get the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
*
* @param[out] prefixLenPtr            - (pointer to) FDB lookup prefix length.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_VALUE             - on bad output value
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpFdbRoutePrefixLenGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_U32                       *prefixLenPtr
);

/**
* @internal cpssDxChIpFdbUnicastRouteForPbrEnableSet function
* @endinternal
*
* @brief   Enable/Disable FDB Unicast routing for PBR (Policy 
*          Based Routed) packets
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FDB
*                                       routing for PBR packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS cpssDxChIpFdbUnicastRouteForPbrEnableSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
);

/**
* @internal cpssDxChIpFdbUnicastRouteForPbrEnableGet function
* @endinternal
*
* @brief   Return if FDB Unicast routing for PBR value (Policy Based
*          Routed) packets is enabled
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr               - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS cpssDxChIpFdbUnicastRouteForPbrEnableGet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpCtrlh */


