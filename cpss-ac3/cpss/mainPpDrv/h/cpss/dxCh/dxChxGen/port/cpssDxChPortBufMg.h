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
* @file cpssDxChPortBufMg.h
*
* @brief CPSS DXCH Port Buffers management API.
*
* @version   28
********************************************************************************
*/

#ifndef __cpssDxChPortBufMgh
#define __cpssDxChPortBufMgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

/**
* @enum CPSS_DXCH_PORT_HOL_FC_ENT
 *
 * @brief Enumeration of system modes HOL and Flow Control
*/
typedef enum{

    /** Flow Control system mode enable (default mode) */
    CPSS_DXCH_PORT_FC_E,

    /** HOL system mode enable */
    CPSS_DXCH_PORT_HOL_E

} CPSS_DXCH_PORT_HOL_FC_ENT;

/**
* @enum CPSS_DXCH_PORT_BUFFERS_MODE_ENT
 *
 * @brief Enumeration of buffer modes shared and divided
*/
typedef enum{

    /** @brief Buffers are
     *  shared among all ports. The maximal number of buffers
     *  allocated per port is limited by maximal number of
     *  buffers in the device.
     */
    CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E,

    /** @brief Buffers are
     *  divided among the ports. The maximal number of buffers
     *  allocated per port is limited by the port buffers limit
     *  configured by cpssDxChPortRxBufLimitSet.
     */
    CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E

} CPSS_DXCH_PORT_BUFFERS_MODE_ENT;

/**
* @enum CPSS_DXCH_PORT_GROUP_ENT
 *
 * @brief Enumeration of port groups
*/
typedef enum{

    /** @brief Gigabit ports group
     *  including CPU port
     */
    CPSS_DXCH_PORT_GROUP_GIGA_E,

    /** @brief HyperG.Stack ports group
     *  including HX/QX ports
     */
    CPSS_DXCH_PORT_GROUP_HGS_E

} CPSS_DXCH_PORT_GROUP_ENT;

/**
* @enum CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT
 *
 * @brief Enumeration of buffer memory FIFOs
*/
typedef enum{

    /** Gigabit ports FIFO */
    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E = 0,

    /** 10G/20G/40G ports FIFO */
    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XG_E,

    /** HGL ports FIFO */
    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_HGL_E,

    /** XLG ports FIFO */
    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XLG_E,

    /** CPU port FIFO */
    CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_CPU_E

} CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT;
/**
* @internal cpssDxChGlobalXonLimitSet function
* @endinternal
*
* @brief   Sets the Global Xon limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xon limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xonLimit), all ports that support 802.3x Flow Control send an Xon
*         frame.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or xonLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The Global Xon limit must be not be greater than Global Xoff Limit.
*       2. To disable Global Xon, the 2xonLimit should be set greater-or-equal
*       of 8<Max Buffer Limit> (maximal number of buffers allocated for
*       all ports).
*       3. Since in CH3 and above the HW resolution is 8,
*       in case the xonLimit input parameter is not a multiple of 8 we
*       round it UP to the nearest one.
*
*/
GT_STATUS cpssDxChGlobalXonLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xonLimit
);

/**
* @internal cpssDxChGlobalXonLimitGet function
* @endinternal
*
* @brief   Gets the Global Xon limit value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChGlobalXonLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xonLimitPtr
);

/**
* @internal cpssDxChGlobalXoffLimitSet function
* @endinternal
*
* @brief   Sets the Global Xoff limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xoff limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xoffLimit), all ports that support 802.3x Flow Control send a pause
*         frame.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or xoffLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To disable Global Xoff, the 2xoffLimit should be set
*       greater-or-equal of 8<Max Buffer Limit> (maximal number of buffers
*       allocated for all ports).
*       2. Since in CH3 and above the HW resolution is 8,
*       in case the xoffLimit input parameter is not a multiple of 8 we
*       round it UP to the nearest one.
*
*/
GT_STATUS cpssDxChGlobalXoffLimitSet
(
    IN GT_U8    devNum,
    IN GT_U32   xoffLimit
);

/**
* @internal cpssDxChGlobalXoffLimitGet function
* @endinternal
*
* @brief   Gets the Global Xoff limit value.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChGlobalXoffLimitGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *xoffLimitPtr
);

/**
* @internal cpssDxChPortRxFcProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxFcProfileSet
(
    IN GT_U8                            devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet
);

/**
* @internal cpssDxChPortRxFcProfileGet function
* @endinternal
*
* @brief   Get the port's flow control profile.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - (pointer to) the Profile Set in which the Flow Control
*                                      Parameters are associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxFcProfileGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_PORT_RX_FC_PROFILE_SET_ENT  *profileSetPtr
);

/**
* @internal cpssDxChPortXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xonLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXonLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xonLimit
);

/**
* @internal cpssDxChPortXonLimitGet function
* @endinternal
*
* @brief   Gets the X-ON port limit. When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters of
*                                      are associated.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXonLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xonLimitPtr
);

/**
* @internal cpssDxChPortXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xoffLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXoffLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xoffLimit
);

/**
* @internal cpssDxChPortXoffLimitGet function
* @endinternal
*
* @brief   Gets the X-OFF port limit. When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortXoffLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xoffLimitPtr
);

/**
* @internal cpssDxChPortRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 and above, the HW resolution is 16, in case the rxBufLimit
*       input parameter is not a multiple of 16 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortRxBufLimitSet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           rxBufLimit
);

/**
* @internal cpssDxChPortRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] rxBufLimitPtr            - (pointer to) the rx buffer limit value in resolution
*                                      of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2 buffer resolution in HW is 1.
*
*/
GT_STATUS cpssDxChPortRxBufLimitGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *rxBufLimitPtr
);

/**
* @internal cpssDxChPortCpuRxBufCountGet function
* @endinternal
*
* @brief   Gets receive buffer count for the CPU on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - The device to get the Cpu Rx buffer count for.
*
* @param[out] cpuRxBufCntPtr           - (Pointer to) number of buffers allocated for the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCpuRxBufCountGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *cpuRxBufCntPtr
);

/**
* @internal cpssDxChPortFcHolSysModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode on the specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] modeFcHol                - GT_FC_E  : set Flow Control mode
*                                      GT_HOL_E : set HOL system mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFcHolSysModeSet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT modeFcHol
);

/**
* @internal cpssDxChPortBuffersModeSet function
* @endinternal
*
* @brief   The function sets buffers mode either shared or divided.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] bufferMode               - buffers mode, divided or shared.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on invalid input parameters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortBuffersModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_PORT_BUFFERS_MODE_ENT  bufferMode
);

/**
* @internal cpssDxChPortBuffersModeGet function
* @endinternal
*
* @brief   The function gets buffers mode either shared or divided.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bufferModePtr            - pointer to the buffer mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on invalid input parameters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortBuffersModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_PORT_BUFFERS_MODE_ENT *bufferModePtr
);

/**
* @internal cpssDxChPortGroupXonLimitSet function
* @endinternal
*
* @brief   The function sets the port-group X-ON threshold.
*         When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] xonLimit                 -  X-ON limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of xonLimit field in hardware:
*       - DXCH and DXCH2 devices: 2 buffers.
*       - DXCH3 and above devices: 8 buffers.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortGroupXonLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xonLimit
);

/**
* @internal cpssDxChPortGroupXonLimitGet function
* @endinternal
*
* @brief   The function gets the port-group X-ON threshold.
*         When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] xonLimitPtr              - Pointer to the  X-ON limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupXonLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xonLimitPtr
);

/**
* @internal cpssDxChPortGroupXoffLimitSet function
* @endinternal
*
* @brief   The function sets the port-group X-OFF threshold.
*         When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] xoffLimit                - X-OFF limit in buffers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of xoffLimit field in hardware:
*       - DXCH and DXCH2 devices: 2 buffers.
*       - DXCH3 and above devices: 8 buffers.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortGroupXoffLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    xoffLimit
);

/**
* @internal cpssDxChPortGroupXoffLimitGet function
* @endinternal
*
* @brief   The function gets the port-group X-OFF threshold.
*         When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] xoffLimitPtr             - Pointer to the X-OFF limit in buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupXoffLimitGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT portGroup,
    OUT GT_U32                   *xoffLimitPtr
);

/**
* @internal cpssDxChPortGroupRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port-group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
* @param[in] rxBufLimit               - Maximal number of buffers that may be allocated for the
*                                      ports belong to a group
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of rxBufLimit field in hardware:
*       - DXCH and DXCH2 devices: 4 buffers.
*       - DXCH3 and above devices: 16 buffers.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortGroupRxBufLimitSet
(
    IN GT_U8                     devNum,
    IN CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    IN GT_U32                    rxBufLimit
);

/**
* @internal cpssDxChPortGroupRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified port-group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] rxBufLimitPtr            - Pointer to value of maximal number of buffers that
*                                      may be allocated for the ports belong to a group.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupRxBufLimitGet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32                    *rxBufLimitPtr
);

/**
* @internal cpssDxChPortCrossChipFcPacketRelayEnableSet function
* @endinternal
*
* @brief   Enable/Disable SOHO Cross Chip Flow control packet relay.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable SOHO Cross Chip Flow control packet relay.
*                                      GT_FALSE: Disable SOHO Cross Chip Flow control packet relay.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCrossChipFcPacketRelayEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortCrossChipFcPacketRelayEnableGet function
* @endinternal
*
* @brief   Gets the current status of SOHO Cross Chip Flow control packet relay.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to SOHO Cross Chip Flow control packet relay staus:
*                                      GT_TRUE  -  Enable SOHO Cross Chip Flow control packet relay.
*                                      GT_FALSE - Disable SOHO Cross Chip Flow control packet relay.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortCrossChipFcPacketRelayEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortClearByBufferEnableSet function
* @endinternal
*
* @brief   Set port to work in Clear By Buffer mode.
*
* @note   APPLICABLE DEVICES:      DxCh1.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: Clear By Buffer mode.
*                                      GT_FALSE: Regular Clear mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortClearByBufferEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortClearByBufferEnableGet function
* @endinternal
*
* @brief   Get port Buffer Memory Clear type.
*
* @note   APPLICABLE DEVICES:      DxCh1.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: Clear By Buffer mode.
*                                      GT_FALSE: Regular Clear mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortClearByBufferEnableGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChPortGlobalRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGlobalRxBufNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfBuffersPtr
);

/**
* @internal cpssDxChPortGroupRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified
*         port group.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - port group
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or port group
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGroupRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_GROUP_ENT  portGroup,
    OUT GT_U32  *numOfBuffersPtr
);

/**
* @internal cpssDxChPortRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxBufNumberGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *numOfBuffersPtr
);

/**
* @internal cpssDxChPortGlobalPacketNumberGet function
* @endinternal
*
* @brief   Gets total number of unique packets currently in the system.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numOfPacketsPtr          - (pointer to) number of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortGlobalPacketNumberGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *numOfPacketsPtr
);

/**
* @internal cpssDxChPortRxNumOfAgedBuffersGet function
* @endinternal
*
* @brief   Gets number of buffers cleared by aging.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] agedBuffersPtr           - (pointer to) Number of aged buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxNumOfAgedBuffersGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *agedBuffersPtr
);

/**
* @internal cpssDxChPortRxMcCntrGet function
* @endinternal
*
* @brief   Gets multicast counter of a certain buffer.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrIdx                  - counter index
*                                      (APPLICABLE RANGES for Lion, Lion2: 0..16383)
*                                      (APPLICABLE RANGES for xCat2: 0..4095)
*                                      (APPLICABLE RANGES for Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X: 0..8191)
*
* @param[out] mcCntrPtr                - (pointer to) multicast counter of a certain buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or bufIdx
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRxMcCntrGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   cntrIdx,
    OUT GT_U32   *mcCntrPtr
);


/**
* @internal cpssDxChPortBufMemFifosThresholdSet function
* @endinternal
*
* @brief   Sets buffer memory FIFOs Threshold.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] fifoType                 - FIFO type
* @param[in] descFifoThreshold        - number of descriptors per port threshold mode.
* @param[in] txFifoThreshold          - number of lines per port threshold mode in the Tx FIFO.
* @param[in] minXFifoReadThreshold    - minimal number of descriptors to read per port threshold mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of values may affect entire system behavior.
*
*/
GT_STATUS cpssDxChPortBufMemFifosThresholdSet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT fifoType,
    IN  GT_U32   descFifoThreshold,
    IN  GT_U32   txFifoThreshold,
    IN  GT_U32   minXFifoReadThreshold
);

/**
* @internal cpssDxChPortBufMemFifosThresholdGet function
* @endinternal
*
* @brief   Gets buffer memory FIFOs Thresholds.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] fifoType                 - FIFO type
*
* @param[out] descFifoThresholdPtr     - pointer to number of descriptors per port per port threshold mode.
* @param[out] txFifoThresholdPtr       - pointer to number of lines per port per port threshold mode in the Tx FIFO.
* @param[out] minXFifoReadThresholdPtr - pointer to minimal number of descriptors to read per port per port threshold mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortBufMemFifosThresholdGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT fifoType,
    OUT GT_U32   *descFifoThresholdPtr,
    OUT GT_U32   *txFifoThresholdPtr,
    OUT GT_U32   *minXFifoReadThresholdPtr
);

/**
* @internal cpssDxChPortTxdmaBurstLimitEnableSet function
* @endinternal
*
* @brief   For a given port Enable/Disable TXDMA burst limit thresholds use.
*         When enabled: instruct the TxDMA to throttle the Transmit Queue Scheduler
*         as a function of the TxDMA FIFOs fill level measured in bytes and descriptor.
*         When disabled: the TxDMA throttles the Transmit Queue Scheduler
*         based on FIFOs fill level measured in descriptors only.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortTxdmaBurstLimitEnableGet function
* @endinternal
*
* @brief   Gets the current status for a given port of TXDMA burst limit thresholds use.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortTxdmaBurstLimitThresholdsSet function
* @endinternal
*
* @brief   Sets the TXDMA burst limit thresholds for a given port.
*         The TxDMA supports three throttling levels: Normal, Slow and OFF.
*         The levels are controlled using two thresholds (almostFullThreshold and
*         fullThreshold) measuring the FIFOs fill level.
*         Normal - the Transmit Queue scheduler is not throttled.
*         Slow - the Transmit Queue scheduler is throttled.
*         OFF - the Transmit Queue scheduler is paused.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] almostFullThreshold      - almost full threshold in Bytes (0..0x3FFC0).
* @param[in] fullThreshold            - full threshold in Bytes (0..0x3FFC0).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of almostFullThreshold and fullThreshold fields in hardware:
*       - Lion and Lion2 devices: 64 Bytes.
*       - Bobcat2, Caelum, Bobcat3, Aldrin, AC3X devices: 128 Bytes.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitThresholdsSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 almostFullThreshold,
    IN GT_U32                 fullThreshold
);

/**
* @internal cpssDxChPortTxdmaBurstLimitThresholdsGet function
* @endinternal
*
* @brief   Gets the TXDMA burst limit thresholds for a given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] almostFullThresholdPtr   - (pointer to) almost full threshold in Bytes.
* @param[out] fullThresholdPtr         - (pointer to) full threshold in Bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxdmaBurstLimitThresholdsGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *almostFullThresholdPtr,
    OUT GT_U32                 *fullThresholdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortBufMgh */

