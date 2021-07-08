/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
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
* @file mvHwsDdr3Bap.h
*
* @brief
*
* @version   5
********************************************************************************
*/




#ifndef _MV_HWS_DDR3_BAP_H_
#define _MV_HWS_DDR3_BAP_H_

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <mvDdr3TrainingIp.h>

#ifdef __cplusplus
   extern "C"
   {
#endif

/********************************* definitions ********************************/

#define MV_BAP_MAX_CHANNELS 2

/******************************************************************************/
/* Definitions of PHY registers                                               */
/******************************************************************************/



/********************************* macros *************************************/

/********************************* structures *********************************/

typedef struct
{
    MV_HWS_ALGO_TYPE    algoType;
    GT_U32              algoFlowMask;
} MV_HWS_BAP_ALGO_PARAMS;

/******************************************************************************/

/**
* @internal mvHwsDdr3BapRegRead function
* @endinternal
*
* @brief   Read BAP register.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS    mvHwsDdr3BapRegRead
(
    GT_U8                 devNum,
    GT_U32                bapId,
    GT_U32                regAddr,
    GT_U32                *dataLow,
    GT_U32                *dataHigh,
    GT_U32                maskLow,
    GT_U32                maskHigh
);

/**
* @internal mvHwsDdr3BapRegWrite function
* @endinternal
*
* @brief   Write BAP register.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS    mvHwsDdr3BapRegWrite
(
    GT_U8                 devNum,
    GT_U32                bapId,
    GT_U32                regAddr,
    GT_U32                dataLow,
    GT_U32                dataHigh,
    GT_U32                maskLow,
    GT_U32                maskHigh
);

/**
* @internal mvHwsDdr3BapMemRead function
* @endinternal
*
* @brief   Read 128 bits (16 bytes) from the external memory.
*
* @param[in] devNum                   - system device number
* @param[in] bapId                    - BAP ID
* @param[in] baseAddress              - offset physical port number
*
* @param[out] data[4]                  - array for read data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS    mvHwsDdr3BapMemRead
(
    GT_U8       devNum,
    GT_U32      bapId,
    GT_U32      baseAddress,
    GT_U32      data[4]
);

/**
* @internal mvHwsDdr3BapMemWrite function
* @endinternal
*
* @brief   Write 128 bits (16 bytes) from the external memory.
*
* @param[in] devNum                   - system device number
* @param[in] bapId                    - BAP ID
* @param[in] baseAddress              - offset physical port number
* @param[in] data[4]                  - array of  to be written
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS    mvHwsDdr3BapMemWrite
(
    GT_U8       devNum,
    GT_U32      bapId,
    GT_U32      baseAddress,
    GT_U32      data[4]
);

/**
* @internal mvHwsDdr3BapRunAlg function
* @endinternal
*
* @brief   Run DDR algorithm to configure the TM.
*
* @param[in] devNum                   - system device number
*                                      eAlgoType    - algorithm type (static/dynamic)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS    mvHwsDdr3BapRunAlg
(
    GT_U8                     devNum,
    MV_HWS_BAP_ALGO_PARAMS    *bapAlgoParams
);


#ifdef __cplusplus
   }
#endif


/*****************************************************************************/

#endif /* _MV_HWS_DDR3_BAP_H_ */



