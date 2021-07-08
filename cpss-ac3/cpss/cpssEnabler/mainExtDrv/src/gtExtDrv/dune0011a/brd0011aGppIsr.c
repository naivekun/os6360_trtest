/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file brd0011aGppIsr.c
*
* @brief Includes driver interrupts control functions.
*
* @version   2
********************************************************************************
*/
#include <gtOs/gtGenTypes.h>

typedef GT_U8 (*GT_ISR_FUNCP)
(
    IN void *cookie
);

/**
* @enum GT_GPP_ID
 *
 * @brief Holds the different GPP ids in a given Pp.
*/
typedef enum
{
    GT_GPP_INT_1 = 0,
    GT_GPP_INT_2,
    GT_GPP_INT_3,
    GT_UPLINK_GPP,
    GT_GPP_INTERNAL_1,
    GT_GPP_INTERNAL_2,
    GT_GPP_INTERNAL_3,
    GT_GPP_INTERNAL_4,
    GT_GPP_INTERNAL_5,
    GT_GPP_INTERNAL_6,
    GT_GPP_INTERNAL_7,
    GT_GPP_INTERNAL_8,
    GT_GPP_INTERNAL_9,
    GT_GPP_INTERNAL_10,
    GT_GPP_INTERNAL_11,
    GT_GPP_INTERNAL_12,
    GT_GPP_INTERNAL_13,
    GT_GPP_INTERNAL_14,
    GT_GPP_INTERNAL_15,
    GT_GPP_INTERNAL_16,
    GT_GPP_INTERNAL_17,
    GT_GPP_MAX_NUM
}GT_GPP_ID;

/**
* @internal extDrvConnectGppIsr function
* @endinternal
*
* @brief   This function connects an Isr for a given Gpp interrupt of
*         the specified Pp device number.
* @param[in] devNum                   - The Pp device number at which the Gpp device is conncted.
* @param[in] gppId                    - The Gpp Id to be connected.
* @param[in] isrFuncPtr               - A pointer to the function to be called on Gpp interrupt
*                                      receiption.
* @param[in] cookie                   - A  to be passed to the isrFuncPtr when its called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note 1. To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
GT_STATUS extDrvConnectGppIsr
(
    IN  GT_U8           devNum,
    IN  GT_GPP_ID       gppId,
    IN  GT_ISR_FUNCP    isrFuncPtr,
    IN  void            *cookie
)
{
   return GT_FAIL;
}




