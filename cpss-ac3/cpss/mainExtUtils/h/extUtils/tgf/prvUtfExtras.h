
/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvUtfExtras.h
*
* @brief Internal header which defines API for helpers functions
* which are specific for cpss unit testing.
*
* @version   1
********************************************************************************
*/
#ifndef __prvUtfExtras
#define __prvUtfExtras

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get common defs */
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>

/* obsolete devices */
#define UTF_TWISTC_E     0
#define UTF_TWISTD_E     0
#define UTF_SAMBA_E      0
#define UTF_TIGER_E      0


/**
* @enum UTF_PP_FAMILY_BIT_ENT
 *
 * @brief The PP family representing bit.
*/
typedef enum
{
    UTF_NONE_FAMILY_E = 0,
    UTF_PIPE_E        = BIT_0,

    UTF_SALSA_E       = BIT_4,
    UTF_PUMA2_E       = BIT_5,
    UTF_PUMA3_E       = BIT_6,
    UTF_CH1_E         = BIT_7,
    UTF_CH1_DIAMOND_E = BIT_8,
    UTF_CH2_E         = BIT_9,
    UTF_CH3_E         = BIT_10,
    UTF_XCAT_E        = BIT_11,
    UTF_LION_E        = BIT_12,
    UTF_XCAT2_E       = BIT_13,
    UTF_LION2_E       = BIT_14,

    UTF_BOBCAT2_E     = BIT_16,
    UTF_XCAT3_E       = BIT_17,
    UTF_BOBCAT3_E     = BIT_18,
    UTF_CAELUM_E      = BIT_19,
    UTF_ALDRIN_E      = BIT_20,
    UTF_AC3X_E        = BIT_21,
    UTF_ALDRIN2_E     = BIT_22,
    UTF_FALCON_E      = BIT_23,


    UTF_PP_FAMILY_BIT__END_OF_DEVICES___E ,/* used for calculation of UTF_ALL_FAMILY_E
                                            must be after the last 'bit' of the last device */

    /* Calc the 'All families' */
    UTF_ALL_FAMILY_E  = ((UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1) * 2) - 1 ,


    UTF_PUMA_E        = (UTF_PUMA2_E | UTF_PUMA3_E),
    UTF_EXMX_E        = (UTF_TWISTC_E | UTF_TWISTD_E | UTF_SAMBA_E | UTF_TIGER_E),
    UTF_DX_SAL_E      =  (UTF_SALSA_E),

    /* Calc the 'dxch' devices bmp based on 'All families' and the other families
       that do NOT change often */
    UTF_DXCH_E        = UTF_ALL_FAMILY_E - UTF_PUMA_E - UTF_EXMX_E - UTF_DX_SAL_E
} UTF_PP_FAMILY_BIT_ENT;


/**
* @internal prvUtfHwDeviceNumberSet function
* @endinternal
*
* @brief   Set HW device number from the SW device number
*
* @param[in] dev                      - SW device number
* @param[in] hwDev                    - new HW device number
*
* @retval GT_OK                    -  Set revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
*/
GT_STATUS prvUtfHwDeviceNumberSet
(
    IN GT_U8               dev,
    IN GT_HW_DEV_NUM       hwDev
);

/**
* @internal prvUtfHwDeviceNumberGet function
* @endinternal
*
* @brief   Get HW device number from the SW device number
*
* @param[in] swDevNum                 - SW device number
*
* @param[out] hwDevPtr                 - (pointer to)HW device number
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfHwDeviceNumberGet
(
    IN  GT_U32              swDevNum,
    OUT GT_HW_DEV_NUM       *hwDevPtr
);

/**
* @internal prvUtfSwFromHwDeviceNumberGet function
* @endinternal
*
* @brief   Get SW device number from the HW device number
*
* @param[in] hwDev                    - HW device number
*
* @param[out] devPtr                   - (pointer to)SW device number
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfSwFromHwDeviceNumberGet
(
    IN GT_HW_DEV_NUM    hwDev,
    OUT GT_U32          *devPtr
);


/**
* @internal utfSetMustUseOrigPorts function
* @endinternal
*
* @brief   Set flag indicating if tests should use only ports from prvTgfPortsArray[]
*         If raised it is implied that prvTgfPortsArray contains at least
*         12 actual port numbers which can be used.
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
void utfSetMustUseOrigPorts
(
    IN GT_BOOL  useOrigPorts
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvUtfExtras */

