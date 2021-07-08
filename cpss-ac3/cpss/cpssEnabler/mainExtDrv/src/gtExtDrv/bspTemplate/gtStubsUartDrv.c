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
* @file gtStubsUartDrv.c
*
* @brief Enable the use of specific UART operations.
*
*/

#include <prestera/os/gtTypes.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#include <gtOs/gtOsIo.h>

/**
* @internal extDrvUartInit function
* @endinternal
*
* @brief   Setting the UART environment to work with PSS polling mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
GT_STATUS extDrvUartInit ( void )
{

  return (GT_OK);
}

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
GT_STATUS extDrvUartTxBuffer
(
  char * buffer,
  GT_U32  buffSize
)
{
  return (GT_OK);
}



