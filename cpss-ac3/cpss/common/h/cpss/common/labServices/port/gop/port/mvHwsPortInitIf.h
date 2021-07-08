/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortInitIf.h
*
* @brief
*
* @version   18
********************************************************************************
*/

#ifndef __mvHwServicesPortIf_H
#define __mvHwServicesPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/common/os/hwsEnv.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>

/**
* @enum MV_HWS_PORT_STANDARD
 *
 * @brief Defines the different port standard metrics.
*/
typedef enum
{
/* 0  */ _100Base_FX,
/* 1  */ SGMII,
/* 2  */ _1000Base_X,
/* 3  */ SGMII2_5,
/* 4  */ QSGMII,
/* 5  */ _10GBase_KX4,
/* 6  */ _10GBase_KX2,   /* 10GBase-DHX */
/* 7  */ _10GBase_KR,
/* 8  */ _20GBase_KR,
/* 9  */ _40GBase_KR,
/* 10 */ _100GBase_KR10,
/* 11 */ HGL,
/* 12 */ RHGL,
/* 13 */ CHGL,   /* CHGL_LR10 */
/* 14 */ RXAUI,
/* 15 */ _20GBase_KX4,   /* HS-XAUI */
/* 16 */ _10GBase_SR_LR,
/* 17 */ _20GBase_SR_LR,
/* 18 */ _40GBase_SR_LR,
/* 19 */ _12_1GBase_KR,          /* Serdes speed: 12.5G;    Data speed: 12.1G */
/* 20 */ XLHGL_KR4,              /* 48G */
/* 21 */ HGL16G,
/* 22 */ HGS,
/* 23 */ HGS4,
/* 24 */ _100GBase_SR10,
/* 25 */ CHGL_LR12,
/* 26 */ TCAM,
/* 27 */ INTLKN_12Lanes_6_25G,
/* 28 */ INTLKN_16Lanes_6_25G,
/* 29 */ INTLKN_24Lanes_6_25G,
/* 30 */ INTLKN_12Lanes_10_3125G,
/* 31 */ INTLKN_16Lanes_10_3125G,
/* 32 */ INTLKN_12Lanes_12_5G,
/* 33 */ INTLKN_16Lanes_12_5G,
/* 34 */ INTLKN_16Lanes_3_125G,
/* 35 */ INTLKN_24Lanes_3_125G,
/* 36 */ CHGL11_LR12,

/* 37 */ INTLKN_4Lanes_3_125G,
/* 38 */ INTLKN_8Lanes_3_125G,
/* 39 */ INTLKN_4Lanes_6_25G,
/* 40 */ INTLKN_8Lanes_6_25G,

/* 41 */ _2_5GBase_QX,
/* 42 */ _5GBase_DQX,
/* 43 */ _5GBase_HX,
/* 44 */ _12GBaseR,              /* Serdes speed: 12.1875G;    Data speed: 1*11.8G */
/* 45 */ _5_625GBaseR,           /* Serdes speed:   5.625G;    Data speed: 1*5.45G*/
/* 46 */ _48GBaseR,              /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
/* 47 */ _12GBase_SR,            /* Serdes speed: 12.1875G;    Data speed: 1*11.8G */
/* 48 */ _48GBase_SR,            /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
/* 49 */ _5GBaseR,               /* Serdes speed: 5.15625G;    Data speed: 1*5G */
/* 50 */ _22GBase_SR,            /* Serdes speed: 11.5625G;    Data speed: 2*11.2G */
/* 51 */ _24GBase_KR2,           /* Serdes speed: 12.1875G;    Data speed: 2*11.8G */
/* 52 */ _12_5GBase_KR,          /* Serdes speed: 12.8906G;    Data speed: 1*12.5G */
/* 53 */ _25GBase_KR2,           /* Serdes speed: 12.8906G;    Data speed: 2*12.5G */
/* 54 */ _50GBase_KR4,           /* Serdes speed: 12.8906G;    Data speed: 4*12.5G */

/* 55 */ _25GBase_KR,            /* Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 56 */ _50GBase_KR2,           /* Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 57 */ _100GBase_KR4,          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 58 */ _25GBase_SR,            /* Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 59 */ _50GBase_SR2,           /* Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 60 */ _100GBase_SR4,          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 61 */ _100GBase_MLG,          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 62 */ _107GBase_KR4,          /* Serdes speed: 27.5G;       Data speed: 4*26.66G (106.6G)*/
/* 63 */ _25GBase_KR_C,          /* Consortium - Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 64 */ _50GBase_KR2_C,         /* Consortium - Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 65 */ _40GBase_KR2,           /* Serdes speed: 20.625G;     Data speed: 2*20G */
/* 66 */ _29_09GBase_SR,         /* Serdes speed: 7.5G;        Data speed: 4*7.272G */

/* 67 */ _40GBase_CR4,
/* 68 */ _25GBase_CR,
/* 69 */ _50GBase_CR2,
/* 70 */ _100GBase_CR4,
/* 71 */ _25GBase_KR_S,
/* 72 */ _25GBase_CR_S,
/* 73 */ _25GBase_CR_C,
/* 74 */ _50GBase_CR2_C,

/* 75 */ _50GBase_KR,              /* Serdes speed: _26_5625G;   Data speed: 1*51.51G */
/* 76 */ _100GBase_KR2,            /* Serdes speed: _26_5625G;   Data speed: 2*51.51G */
/* 77 */ _200GBase_KR4,            /* Serdes speed: _26_5625G;   Data speed: 4*51.51G */
/* 78 */ _200GBase_KR8,            /* Serdes speed:  25.78125G;  Data speed: 8*25G */
/* 79 */ _400GBase_KR8,            /* Serdes speed: _26_5625G;   Data speed: 8*51.51G */

/* 80 */ _102GBase_KR4,            /* Serdes speed: _26.25G;     Data speed: 4*25G (102G)*/
/* 81 */ _52_5GBase_KR2,           /* Serdes speed: _27.1G;      Data speed: 2*26.28G (52.5G) */
/* 82 */ _40GBase_KR4,
/* 83 */ _26_7GBase_KR,            /* Serdes speed: 27.5G;       Data speed: 1*26.66G (26.66G) */

    NON_SUP_MODE,
    LAST_PORT_MODE = NON_SUP_MODE

}MV_HWS_PORT_STANDARD;

/* Macro for two lanes speeds/port modes - to ease the use. */
#define HWS_TWO_LANES_MODE_CHECK(_mode) ((_mode == _50GBase_KR2) || (_mode == _50GBase_KR2_C) || (_mode == _40GBase_KR2) || \
                                         (_mode == _50GBase_CR2) || (_mode == _50GBase_CR2_C) || (_mode == _50GBase_SR2) || \
                                         (_mode == _52_5GBase_KR2))

#define HWS_25G_MODE_CHECK(_mode) ((_mode == _25GBase_KR_S) || (_mode == _25GBase_KR) || (_mode == _25GBase_KR_C) || \
                                         (_mode == _25GBase_CR_C) || (_mode == _25GBase_CR_S) || (_mode == _25GBase_CR) || \
                                         (_mode == _25GBase_SR))

/**
* @enum MV_HWS_REF_CLOCK_SOURCE
 *
 * @brief Defines the supported reference clock source.
*/
typedef enum{

    PRIMARY_LINE_SRC,

    SECONDARY_LINE_SRC

} MV_HWS_REF_CLOCK_SOURCE;

/**
* @enum MV_HWS_REF_CLOCK_SUP_VAL
 *
 * @brief Defines the supported reference clock.
*/
typedef enum
{
  MHz_156,
  MHz_78,
  MHz_25,
  MHz_125,
  MHz_312,
  MHz_164
}MV_HWS_REF_CLOCK_SUP_VAL; /* need to be synchronized with MV_HWS_REF_CLOCK enum */

/**
* @enum MV_HWS_PORT_ACTION
 *
 * @brief Defines different actions during port delete.
*/
typedef enum{

    PORT_POWER_DOWN,

    PORT_RESET

} MV_HWS_PORT_ACTION;

/**
* @enum MV_HWS_PORT_INIT_FLAVOR
 *
 * @brief Defines different suites of port init process:
 * PORT_AP_INIT -   run AP on current port
 * PORT_REGULAR_INIT - regular port create flow (no SERDES training)
 * PORT_RX_TRAIN_INIT - create port and run RX training on all SERDESes
 * PORT_TRX_TRAIN_INIT - create port and run TRX training on all SERDESes
*/
typedef enum{

    PORT_AP_INIT,

    PORT_REGULAR_INIT,

    PORT_RX_TRAIN_INIT,

    PORT_TRX_TRAIN_INIT

} MV_HWS_PORT_INIT_FLAVOR;

/**
* @enum MV_HWS_RESET
 *
 * @brief Defines reset types to set valid sequence
*/
typedef enum{

    RESET,

    UNRESET,

    FULL_RESET

} MV_HWS_RESET;

typedef enum
{
    CPLL0,
    CPLL1,
    CPLL2,
    MV_HWS_MAX_CPLL_NUMBER

}MV_HWS_CPLL_NUMBER;

typedef enum
{
    MV_HWS_25MHzIN,
    MV_HWS_156MHz_IN,
    MV_HWS_MAX_INPUT_FREQUENCY

}MV_HWS_CPLL_INPUT_FREQUENCY;

typedef enum
{
    MV_HWS_156MHz_OUT,          /* 156.25   Mhz */
    MV_HWS_78MHz_OUT,           /* 78.125   Mhz */
    MV_HWS_200MHz_OUT,          /* 200      Mhz */
    MV_HWS_312MHz_OUT,          /* 312.5    Mhz */
    MV_HWS_161MHz_OUT,          /* 160.9697 Mhz */
    MV_HWS_164MHz_OUT,          /* 164.2424 Mhz */
    MV_HWS_MAX_OUTPUT_FREQUENCY

}MV_HWS_CPLL_OUTPUT_FREQUENCY;

typedef enum
{
    MV_HWS_ANA_GRP0,
    MV_HWS_ANA_GRP1,
    MV_HWS_CPLL,
    MV_HWS_RCVR_CLK_IN,
    MV_HWS_MAX_INPUT_SRC_CLOCK

}MV_HWS_INPUT_SRC_CLOCK;

typedef struct
{
    GT_BOOL enableSerdesMuxing;
    GT_U8   serdesLanes[4];
}MV_HWS_PORT_SERDES_TO_MAC_MUX;

typedef struct
{
    GT_BOOL                  lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
    MV_HWS_PORT_FEC_MODE     portFecMode;
}MV_HWS_PORT_INIT_INPUT_PARAMS;

#define HWS_IS_PORT_MULTI_SEGMENT(portMode)     ((portMode == _200GBase_KR4) || (portMode == _200GBase_KR8) || (portMode == _400GBase_KR8))

/**
* @internal mvHwsPortInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

/**
* @internal mvHwsApPortInit function
* @endinternal
*
* @brief   Init physical port for 802.3AP protocol. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

/**
* @internal mvHwsPortFlavorInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly to specified flavor.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlavorInit
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

/**
* @internal mvHwsPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION      action
);

/**
* @internal mvHwsPortValidate function
* @endinternal
*
* @brief   Validate port API's input parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortValidate
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPortLoopbackValidate function
* @endinternal
*
* @brief   Validate loopback port input parameters.
*         In MMPCS mode: the ref_clk comes from ExtPLL, thus the Serdes can be in power-down.
*         In all other PCS modes: there is no ExtPLL, thus the ref_clk comes is taken
*         from Serdes, so the Serdes should be in power-up.
* @param[in] portPcsType              - port pcs type
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] serdesInit               - if true, init port serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackValidate
(
    GT_U32  portPcsType,
    GT_BOOL lbPort,
    GT_BOOL *serdesInit
);

/**
* @internal mvHwsPortSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSerdesPowerUp
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE refClockSource,
    GT_U32 *curLanesList
);

/**
* @internal mvHwsPortApSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes for 802.3AP protocol.
*         assumes: parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesPowerUp
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD      portMode,
    MV_HWS_REF_CLOCK_SUP_VAL  refClock,
    MV_HWS_REF_CLOCK_SOURCE   refClockSource,
    GT_U32                    *curLanesList
);

/**
* @internal mvHwsPortModeSquelchCfg function
* @endinternal
*
* @brief   reconfigure default squelch threshold value only for KR (CR) modes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] curLanesList             - active Serdes lanes list according to configuration
* @param[in] txAmp                    - Reference clock frequency
* @param[in] emph0                    - Reference clock source line
* @param[in] emph1                    - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeSquelchCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32 *curLanesList,
    GT_U32 txAmp,
    GT_U32 emph0,
    GT_U32 emph1
);

/**
* @internal mvHwsPortModeCfg function
* @endinternal
*
* @brief   configures MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPortStartCfg function
* @endinternal
*
* @brief   Unreset MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStartCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPortStopCfg function
* @endinternal
*
* @brief   Reset MAC and PCS components
*         Port power down on each related serdes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - port power down or reset
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*                                      reset pcs        - reset pcs option
*                                      reset mac        - reset mac option
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStopCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_ACTION action,
    GT_U32 *curLanesList,
    MV_HWS_RESET reset_pcs,
    MV_HWS_RESET reset_mac
);

#ifdef __cplusplus
}
#endif

#endif /* mvHwServicesPortIf_H */



