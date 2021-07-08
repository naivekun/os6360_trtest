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
* @file gtTwsiDrvCtrl.h
*
* @brief API implementation for TWSI facilities.
*
*/

#ifndef __gtTwsiDrvCtrl
#define __gtTwsiDrvCtrl

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>


/**
* @internal extDrvDirectTwsiInitDriver function
* @endinternal
*
* @brief   Init the TWSI interface
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiInitDriver
(
    GT_VOID
);
  

/**
* @internal extDrvDirectTwsiWaitNotBusy function
* @endinternal
*
* @brief   Wait for TWSI interface not BUSY
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiWaitNotBusy
(
    GT_VOID
);


/**
* @internal extDrvDirectTwsiMasterReadTrans function
* @endinternal
*
* @brief   do TWSI interface Transaction
*
* @param[in] devId                    - I2c slave ID
* @param[in] pData                    - Pointer to array of chars (address / data)
*                                      len   - pData array size (in chars).
*                                      stop  - Indicates if stop bit is needed.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiMasterReadTrans
(
    IN GT_U8           devId,       /* I2c slave ID                              */ 
    IN GT_U8           *pData,      /* Pointer to array of chars (address / data)*/
    IN GT_U8           len,         /* pData array size (in chars).              */
    IN GT_BOOL         stop         /* Indicates if stop bit is needed in the end  */
);


/**
* @internal extDrvDirectTwsiMasterWriteTrans function
* @endinternal
*
* @brief   do TWSI interface Transaction
*
* @param[in] devId                    - I2c slave ID
* @param[in] pData                    - Pointer to array of chars (address / data)
*                                      len   - pData array size (in chars).
*                                      stop  - Indicates if stop bit is needed.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectTwsiMasterWriteTrans
(
    IN GT_U8           devId,       /* I2c slave ID                              */ 
    IN GT_U8           *pData,      /* Pointer to array of chars (address / data)*/
    IN GT_U8           len,         /* pData array size (in chars).              */
    IN GT_BOOL         stop         /* Indicates if stop bit is needed in the end  */
);
  

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtTwsiDrvCtrl */



