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
* @file mv_hws_avago_if.h
*
* @brief
*
* @version   15
********************************************************************************
*/

#ifndef __mv_hws_avago_if_H
#define __mv_hws_avago_if_H

#ifdef MV_HWS_BIN_HEADER
#include "util.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/************************* definition *****************************************************/
#define  AVAGO_ETH_CONNECTION         1
#define  AVAGO_I2C_CONNECTION         2
#define  AVAGO_SBUS_CONNECTION        3
#define  MAX_AVAGO_SERDES_ADDRESS     255
#define  NA_VALUE                     0
#define  AVAGO_INVALID_SBUS_ADDR      (-1)
#define  AVAGO_SERDES_INIT_BYPASS_VCO (0xFFFF)
#define  AVAGO_CTLE_BITMAP 32

#define AVAGO_AAPL_LGPL
/*#define MARVELL_AVAGO_DEBUG_FLAG*/
/*#define MARVELL_AVAGO_DB_BOARD*/

#ifdef MARVELL_AVAGO_DEBUG_FLAG
#define AVAGO_DBG(s) osPrintf s
#else
#define AVAGO_DBG(s)
#endif /* MARVELL_AVAGO_DEBUG_FLAG */

#define CHECK_AVAGO_RET_CODE_WITH_ACTION(action) \
{ \
    if(aaplSerdesDb[devNum]->return_code < 0) \
    { \
        hwsOsPrintf("%s failed (return code %d)\n", __func__, aaplSerdesDb[devNum]->return_code); \
        aapl_get_return_code(aaplSerdesDb[devNum]); \
        action; \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG); \
    } \
}

#define CHECK_AVAGO_RET_CODE() \
{ \
    if(aaplSerdesDb[devNum]->return_code < 0) \
    { \
        hwsOsPrintf("%s failed (return code %d)\n", __func__, aaplSerdesDb[devNum]->return_code); \
        aapl_get_return_code(aaplSerdesDb[devNum]); \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG); \
    } \
}

#ifdef MV_HWS_BIN_HEADER
#define HWS_MAX_DEVICE_NUM 1
#define osPrintf     mvPrintf

#define genRegisterSet(devNum, portGroup, address, sbus_data, mask) \
     devNum = devNum; \
     portGroup = portGroup; \
     genSwitchRegisterSet(address, sbus_data, mask)

#define genRegisterGet(devNum, portGroup, address, data_ptr, mask) \
     devNum = devNum; \
     portGroup = portGroup; \
     genSwitchRegisterGet(address, data_ptr, mask)
#endif /* MV_HWS_BIN_HEADER */

/************************* Globals *******************************************************/
int mvHwsAvagoInitializationCheck
(
    unsigned char devNum,
    unsigned int  serdesNum
);

int mvHwsAvagoConvertSerdesToSbusAddr
(
    unsigned char devNum,
    unsigned int  serdesNum,
    unsigned int  *sbusAddr
);

#ifndef ASIC_SIMULATION
/**
* @internal mvHwsAvagoSerdesInit function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesInit
(
    unsigned char devNum
);
#endif

/**
* @internal mvHwsAvagoSerdesSpicoInterrupt function
* @endinternal
*
* @brief   Issue the interrupt to the Spico processor.
*         The return value is the interrupt number.
*
* @param[out] result                   - spico interrupt return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesSpicoInterrupt
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    unsigned int    interruptCode,
    unsigned int    interruptData,
    int             *result
);

/**
* @internal mvHwsAvagoSerdesTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) from Avago Serdes
*
* @param[out] temperature              - Serdes  degree value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesTemperatureGet
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    int             *temperature
);

/**
* @internal mvHwsAvagoSerdesPolarityConfigImpl function
* @endinternal
*
* @brief   Per serdes invert the Tx or Rx.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesPolarityConfigImpl
(
    unsigned char     devNum,
    unsigned int      portGroup,
    unsigned int      serdesNum,
    unsigned int     invertTx,
    unsigned int     invertRx

);

/**
* @internal mvHwsAvagoSerdesPolarityConfigGetImpl function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesPolarityConfigGetImpl
(
    unsigned char    devNum,
    unsigned int     portGroup,
    unsigned int     serdesNum,
    unsigned int     *invertTx,
    unsigned int     *invertRx
);

/**
* @internal mvHwsAvagoSerdesResetImpl function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesResetImpl
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     analogReset,
    GT_BOOL     digitalReset,
    GT_BOOL     syncEReset
);

/**
* @internal mvHwsAvagoSerdesArrayPowerCtrlImpl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesArrayPowerCtrlImpl
(
    unsigned char       devNum,
    unsigned int        portGroup,
    GT_UOPT             numOfSer,
    GT_UOPT            *serdesArr,
    unsigned char       powerUp,
    unsigned int        divider,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

/**
* @internal mvHwsAvagoSerdesPowerCtrlImpl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesPowerCtrlImpl
(
    unsigned char               devNum,
    unsigned int                portGroup,
    unsigned int                serdesNum,
    unsigned char               powerUp,
    unsigned int                divider,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
);

/**
* @internal mvHwsAvagoAccessLock function
* @endinternal
*
* @brief   Avago device access lock
*/
void mvHwsAvagoAccessLock
(
    unsigned char devNum
);

/**
* @internal mvHwsAvagoAccessUnlock function
* @endinternal
*
* @brief   Avago device access unlock
*/
void mvHwsAvagoAccessUnlock
(
    unsigned char devNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mv_hws_avago_if_H */


