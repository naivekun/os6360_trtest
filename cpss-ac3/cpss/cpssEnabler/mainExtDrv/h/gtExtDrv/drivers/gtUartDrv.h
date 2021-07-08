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
* @file gtUartDrv.h
*
* @brief Includes UART functions wrappers.
*
* @version   1.1.2.1
********************************************************************************
*/
#ifndef __gtUartDrvh
#define __gtUartDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/os/extDrvOs.h>

/**
* @internal extDrvUartInit function
* @endinternal
*
* @brief   Setting the UART environment to work with PSS polling mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
GT_STATUS extDrvUartInit ( void );

/**
* @internal extDrvUartTxBuffer function
* @endinternal
*
* @brief   Sending ASCII buffer to the UART port in pooling mode
*
* @param[in] buffer                   - pointer to  containing the data to be send to the UART port.
* @param[in] buffSize                 - The size of the buffer to be sent to the UART port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
GT_STATUS extDrvUartTxBuffer(
  char * buffer,
  GT_U32  buffSize
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtDmaDrvh */




