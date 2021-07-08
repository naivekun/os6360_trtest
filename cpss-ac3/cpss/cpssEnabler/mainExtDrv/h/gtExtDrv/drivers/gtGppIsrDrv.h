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
* @file gtGppIsrDrv.h
*
* @brief Includes driver interrupts control functions.
*
* @version   1.1.2.2
********************************************************************************
*/
#ifndef __gtGppIsrDrvh
#define __gtGppIsrDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>
#include <cpss/generic/events/cpssGenEventCtrl.h>


/**
* @internal extDrvConnectGppIsr function
* @endinternal
*
* @brief   This function connects an Isr for a given Gpp interrupt.of specified
*         the specified Pp device.
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
    IN  GT_U8                   devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId,
    IN  CPSS_EVENT_ISR_FUNC     isrFuncPtr,
    IN  GT_VOID                 *cookie
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



