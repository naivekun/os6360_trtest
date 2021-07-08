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
* mvGw16If.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvGw16If_H
#define __mvGw16If_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#ifdef __cplusplus
extern "C" {
#endif


/************************* Globals *******************************************************/
#define HWS_D2D_PHY_BASE_ADDR               0x33000000
#define HWS_D2D_OFFSET                      0x01000000
#define HWS_D2D_PCS_BASE_ADDR               0x33000000
#define HWS_D2D_OFFSET                      0x01000000

#define PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (regOffset + HWS_D2D_PCS_BASE_ADDR + d2dIndex * HWS_D2D_OFFSET)

#define PRV_HWS_PHY_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset) (regOffset + d2dIndex * HWS_D2D_OFFSET)
#define PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (HWS_D2D_PHY_BASE_ADDR + PRV_HWS_PHY_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset))

#define HWS_D2D_PMA_BASE_ADDR               0x33000000
#define PRV_HWS_PMA_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset) (regOffset + d2dIndex * HWS_D2D_OFFSET)
#define PRV_HWS_PMA_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (HWS_D2D_PMA_BASE_ADDR + PRV_HWS_PMA_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset))


#define HWS_D2D_KDU_BASE_ADDR               0x33009000

#define HWS_D2D_KDU_FIFO_CTRL               0x0
#define HWS_D2D_KDU_FIFO_FRZ_CTRL           0x4
#define HWS_D2D_KDU_FIFO_XBARCFG_LANE       0x1C
#define HWS_D2D_KDU_FIFO_LANE_CTRL          0x44

#define HWS_D2D_KDU_PAT_RX_CTRL_LANE        0x800
#define HWS_D2D_KDU_PAT_TX_CTRL_LANE        0xC00

#define HWS_D2D_KDU_PAT_RX_PSEL_LANE        0x804
#define HWS_D2D_KDU_PAT_TX_PSEL_LANE        0xC04

#define HWS_D2D_KDU_PAT_RX_STS              0x808
#define HWS_D2D_KDU_PAT_TX_STS              0xC08

#define HWS_D2D_KDU_PAT_RX_PUSER_PAT_LANE   0x810
#define HWS_D2D_KDU_PAT_TX_PUSER_PAT_LANE   0xC10

#define HWS_D2D_KDU_PAT_RX_PERR_CNT_LANE    0x820
#define HWS_D2D_KDU_PAT_TX_PERR_CNT_LANE    0xC20

#define HWS_D2D_KDU_PAT_RX_PUSER_MASK_LANE  0x830
#define HWS_D2D_KDU_PAT_TX_PUSER_MASK_LANE  0xC30


#define PRV_HWS_KDU_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (regOffset + HWS_D2D_KDU_BASE_ADDR + d2dIndex * HWS_D2D_OFFSET)


/**
* @internal mvHwsD2dPmaLaneTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
*/
GT_STATUS mvHwsD2dPmaLaneTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/**
* @internal mvHwsGw16TileTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] mode                     - test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGen
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,  /* D2D index */
    GT_UOPT                     serdesNum,  /* Channel index */
    MV_HWS_SERDES_TX_PATTERN    txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
);

/**
* @internal mvHwsGw16TileTestGenGet function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[out] txPattern               - (pointer to) pattern to transmit
* @param[out] mode                    - (pointer to) test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGenGet
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsGw16TileTestGenStatus function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
*/
GT_STATUS mvHwsGw16TileTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/**
* @internal mvHwsD2dPmaLaneTestGeneneratorConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - PMA lane index (APPLICABLE RANGES: 0..3)
*/
GT_STATUS mvHwsD2dPmaLaneTestGeneneratorConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,  /* D2D index */
    GT_U32                          serdesNum,  /* Lane index 0..3*/
    MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
);


/**
* @internal mvHwsD2dPmaLaneTestCheckerConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - PMA lane index (APPLICABLE RANGES: 0..3)
*/
GT_STATUS mvHwsD2dPmaLaneTestCheckerConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,  /* D2D index */
    GT_U32                          serdesNum,  /* Lane index 0..3*/
    MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
);

/**
* @internal mvHwsD2dPmaLaneTestLoopbackSet function
* @endinternal
*
* @brief   Activates loopback between lane checker and lane generator.
*          To enable loopback XBAR configuration should be done + FIFO reset/unreset
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - PMA lane index (APPLICABLE RANGES: 0..3)
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPmaLaneTestLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,  /* D2D index */
    GT_U32                  serdesNum,  /* Lane index 0..3*/
    MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsD2dPmaLaneTestGenInjectErrorEnable function
* @endinternal
*
* @brief   Activates the D2D PMA error injection.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesDirection          - SERDES direction
*/
GT_STATUS mvHwsD2dPmaLaneTestGenInjectErrorEnable
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,  /* D2D index */
    GT_UOPT                     serdesNum,  /* Lane index 0..3*/
    MV_HWS_SERDES_DIRECTION     serdesDirection
);

/**
* @internal mvHwsD2dPhyIfInit function
* @endinternal
*
* @brief   Init GW16 Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsD2dPhyIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS *funcPtrArray
);

/**
* @internal mvHwsGW16SerdesAutoStartInit function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInit
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U32 d2dNum
);

/**
* @internal mvHwsGW16SerdesAutoStartInitStatusGet function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInitStatusGet
(
    GT_U8 devNum,
    GT_U32 d2dNum
);


/**
* @internal mvHwsGw16IfClose function
* @endinternal
*
* @brief   Release all system resources allocated by Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
void mvHwsGw16IfClose
(
    GT_U8      devNum
);


#ifdef __cplusplus
}
#endif

#endif /* __mvAvagoIf_H */

