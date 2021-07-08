/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/


#include <gtExtDrv/drivers/gtGppIsrDrv.h>
/* check if this Ok */
/* #include <prestera/core/events/gtCoreGppIntCtrl.h> */



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
    IN  GT_U8                   devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId,
    IN  CPSS_EVENT_ISR_FUNC     isrFuncPtr,
    IN  GT_VOID                 *cookie
)
{
    /* call the cpss to connect the GPP */
    return cpssGenEventGppIsrConnect(devNum,
                                     gppId,
                                     isrFuncPtr,
                                     cookie);
}





