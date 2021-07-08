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
* @file mvComPhyH28nmEomIf.h
*
* @brief
*
* @version   3
********************************************************************************
*/

#ifndef __mvComPhy28nmEOMIf_H
#define __mvComPhy28nmEOMIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define  EOM_VOLT_RANGE (32)
#define  EOM_PHASE_RANGE (128)

/**
* @internal mvHwsComH28nmEomInit function
* @endinternal
*
* @brief   Init EOM serdes mechanism.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmEomInit
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum
);

/**
* @internal mvHwsComH28nmEomClose function
* @endinternal
*
* @brief   Disable EOM serdes mechanism.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmEomClose
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum
);


/**
* @internal mvHwsComH28nmEomGetUi function
* @endinternal
*
* @brief   Returns the current system baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmEomGetUi
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    GT_U32*    curUi
);

/**
* @internal mvHwsComH28nmEomGetDfeRes function
* @endinternal
*
* @brief   Returns the current DFE parameters.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmEomGetDfeRes
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    GT_U32*    dfeRes
);

/**
* @internal mvHwsComH28nmEomGetMatrix function
* @endinternal
*
* @brief   Returns the eye mapping matrix.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] timeout                  - wait time in msec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmEomGetMatrix
(
    GT_U8     devNum,
    GT_U32    portGroup,
    GT_U32    serdesNum,
    GT_U32    timeout,
    GT_U32    *rowSize,
    GT_U32    *upMatrix,
    GT_U32    *loMatrix
);

/**
* @internal mvHwsComH28nmSerdesEOMGet function
* @endinternal
*
* @brief   Returns the horizontal/vertical Rx eye margin.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] timeout                  - wait time in msec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesEOMGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    GT_U32                    timeout,
    MV_HWS_SERDES_EOM_RES     *results
);

/**
* @internal mvHwsComH28nmSerdesMatrixMap function
* @endinternal
*
* @brief   Returns the horizontal/vertical Rx eye margin.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] timeout                  - wait time in msec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComH28nmSerdesMatrixMap
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    GT_U32                  timeout,
    GT_U32                  *rowSize,
    GT_U32                  *upperMatrix,
    GT_U32                  *lowerMatrix
);


void mvHwsComH28nmEomPrintMap
(
    GT_U32  leftEdge,
    GT_U32  rightEdge,
    GT_U32  *upperEyeRes,
    GT_U32  *lowerEyeRes
);

/* test function */
GT_STATUS eomTest2
(
    GT_U8                     devNum,
    GT_U32                    serdesNum,
    GT_U32                    timeout
);

GT_STATUS eomTest3
(
    GT_U8                     devNum,
    GT_U32                    serdesNum,
    GT_U32                    timeout
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvComPhy28nmEOMIf_H */


