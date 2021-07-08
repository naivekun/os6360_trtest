/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <prestera/os/gtTypes.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#include <gtOs/gtOsIo.h>

/* vxWorks specific includes */
#include <vxWorks.h>
#include <sioLib.h>
#include <sysLib.h>
#include <errno.h>
#include <drv/sio/st16552Sio.h>

/* Default stdout UART channel */
SIO_CHAN * sioChannel = (SIO_CHAN *)NULL; 

/* MACRO - checking for UART initalization fo PSS internal use */
#define EXT_DRV_UART_INIT_DONE()   \
  ( sioChannel != (SIO_CHAN *)NULL )

#define  NUMBER_OF_SENDING_TRYS  0xFFFFFF
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
  
  /* reads the default stdout UART channel */  
  if ((sioChannel = sysSerialChanGet(0)) == (SIO_CHAN *)ERROR )
    return (GT_FAIL);
  
  /* Bind the tx buffer function to the global pointer used by osPrintSync */
  /* The use of binding function enable us to stay support with 
      users that won't like to use this feature. */
  osPrintSyncUartBindFunc = extDrvUartTxBuffer;
  
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
  int i, poolRetVal, dec;
  
  /* Checking if the UART port initilize to work with the PSS */
  if ( ! EXT_DRV_UART_INIT_DONE() )
    return (GT_FAIL);
    
  for (i=0; i<buffSize; i++)
  {
    /* re-setting delay value */
    dec = NUMBER_OF_SENDING_TRYS;
    do {
         /* Call function from the BSP sending charecter in polling mode */
         poolRetVal = (sioChannel->pDrvFuncs->pollOutput)(sioChannel, buffer[i]);
    } while ( (poolRetVal == EAGAIN) && (--dec) );

    /* On Success. */
    if ( poolRetVal == OK )
      continue;

    /* Fail to sent the char over the UART port */
    if ( dec == 0 )
      return (GT_FAIL);
  }
  
  return (GT_OK);
}



