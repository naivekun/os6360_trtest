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
* @file mvComPhyEomIf.h
*
* @brief
*
* @version   8
********************************************************************************
*/

#ifndef __mvComPhyEOMIf_H
#define __mvComPhyEOMIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define  EOM_VOLT_RANGE (32)
#define  EOM_WINDOW (64)
#define  EOM_PHASE_RANGE (128)
#define  EOM_SERCH_RANGE (2560)

/************************** Register Definition *************************/

/* internal registers */
#define COM_H_DFE_Resolution_Control_Reg    0x1C
#define COM_H_EOM_DFE_Update_Control_Reg    0x20
#define COM_H_Adapted_DFE_Coefficient0_Reg  0x2C
#define COM_H_EOM_Circuit_Control_Reg       0x9C
#define COM_H_EOM_Set1_Control_Reg0         0xA0
#define COM_H_EOM_Set1_Control_Reg1         0xA4
#define COM_H_EOM_Set1_Population_Counter0  0xA8
#define COM_H_EOM_Set1_Population_Counter1  0xAC
#define COM_H_EOM_Set1_Population_Counter2  0xB0
#define COM_H_EOM_Set1_Population_Counter3  0xB4
#define COM_H_EOM_Set1_Threshold_Counter0   0xB8
#define COM_H_EOM_Set1_Threshold_Counter1   0xBC
#define COM_H_EOM_Set1_Threshold_Counter2   0xC0
#define COM_H_EOM_Set1_Threshold_Counter3   0xC4
#define COM_H_EOM_Set1_Sample_Counter0      0xC8
#define COM_H_EOM_Set1_Sample_Counter1      0xCC
#define COM_H_EOM_Set1_Sample_Counter2      0xD0
#define COM_H_EOM_Set1_Sample_Counter3      0xD4
#define COM_H_EOM_Set1_Error_Counter0       0xD8
#define COM_H_EOM_Set1_Error_Counter1       0xDC
#define COM_H_EOM_Set1_Error_Counter2       0xE0
#define COM_H_EOM_Set1_Error_Counter3       0xE4


/************************************************************************/

typedef enum {SearchLeft, SearchRight, Done} MV_STATE_MODE;

typedef struct
{
  GT_U32 totalError;
}MV_EOM_RESULTS;

typedef struct
{
    GT_U32 eyeWidth;
    GT_U32 leftEyeWidth;
    GT_U32 rightEyeWidth;
} MV_EOM_EYE_DATA;


/**
* @internal mvHwsEomInit function
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
GT_STATUS mvHwsEomInit
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum
);

/**
* @internal mvHwsEomClose function
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
GT_STATUS mvHwsEomClose
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum
);


/**
* @internal mvHwsEomGetUi function
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
GT_STATUS mvHwsEomGetUi
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    GT_U32*      curUi
);

/**
* @internal mvHwsEomGetDfeRes function
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
GT_STATUS mvHwsEomGetDfeRes
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    GT_U32*      dfeRes
);

/**
* @internal mvHwsEomGetMatrix function
* @endinternal
*
* @brief   Calculate and returns the eye mapping matrix.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] timeout                  - wait time in msec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsEomGetMatrix
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
* @internal mvEomMapEye function
* @endinternal
*
* @brief   Calculate and return upper and lower Eye matrix
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvEomMapEye
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum,
    GT_U32  population,
    GT_U32  targetBER,
    GT_U32  leftEdge,
    GT_U32  rightEdge,
    GT_U32  *upperEyeRes,
    GT_U32  *lowerEyeRes
);

/**
* @internal mvHwsComHSerdesEOMGet function
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
GT_STATUS mvHwsComHSerdesEOMGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    GT_U32                    timeout,
    MV_HWS_SERDES_EOM_RES     *results
);

/**
* @internal mvHwsEomFindEyeCenter function
* @endinternal
*
* @brief   Returns the right/left Rx eye margin.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical Serdes number
*                                      timeout   - wait time in mSec
* @param[in] voltage                  - voltage
* @param[in] population               error population
* @param[in] targetBer                - target BER
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsEomFindEyeCenter
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum,
    GT_U32 voltage,
    GT_U32 population,
    GT_U32 targetBer,
    GT_U32 *leftEdge,
    GT_U32 *rightEdge
);

/**
* @internal mvHwsEomEyeMeasure function
* @endinternal
*
* @brief   Returns the Rx eye measurments.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical Serdes number
*                                      timeout   - wait time in mSec
* @param[in] voltage                  - voltage
* @param[in] population               error population
* @param[in] targetBer                - target BER
* @param[in] leftEdge                 - eye left edge
* @param[in] rightEdge                - eye right edge
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsEomEyeMeasure
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum,
    GT_U32 voltage,
    GT_U32 population,
    GT_U32 targetBer,
    GT_U32 leftEdge,
    GT_U32 rightEdge,
    MV_EOM_EYE_DATA  *eomEyeData
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvComPhyEOMIf_H */


