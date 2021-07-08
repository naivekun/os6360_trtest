/*************************************************************************
*
* Copyright (c) Microchip Technology Inc. All rights reserved.
* Licensed under the MIT License. See LICENSE in the project root for license information.
*
*************************************************************************
*
*  File Revision: 1.0
*
*************************************************************************
*
*  Description: contain the functions implementation for comm os functions.
*
*************************************************************************/




/*=========================================================================
/ Define here the Arcitecture
/========================================================================*/


#define _LINUX_PC_ARC_
/*#define _NEW_ARC_*/

#ifdef _LINUX_PC_ARC_

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "../../host_poe_api/inc/host_poe_communication.h"
#include "../inc/host_poe_comm_device.h"
#include "../inc/host_poe_comm_os.h"

static pthread_mutex_t poeMutex = PTHREAD_MUTEX_INITIALIZER;

#endif

/*---------------------------------------------------------------------
 *    description:     Sleep function
 *
 *    input :   sleepTime_mS  - sleep value in mili Seconds
 *                              minimum required range is: 20 mili seconds to 50 mili seconds
 *                              with resulution of 10 mili second
 *
 *    output:   none
 *
 *    return:   ePOE_STATUS_OK                         - operation succeed
 *              e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR   - operation failed due to usleep function operation error
 *---------------------------------------------------------------------*/

PoE_RC_e poeCommOsSleep(UINT16 sleepTime)
{

#ifdef _LINUX_PC_ARC_

	if(usleep(sleepTime * 1000)== 0)
		return PoE_RC_SUCSESS;

	return PoE_RC_ERR_SLEEP_FUNC;
#else
#error poeCommOsSleep function should be Implement.
	return PoE_RC_SUCSESS;

#endif
}

	


/*---------------------------------------------------------------------
 *    description:     initialize the mutex
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   ePOE_STATUS_OK                     - operation succeed
 *              e_POE_STATUS_ERR_MUTEX_INIT_ERROR   - operation failed due to mutex initialize operation error
 *---------------------------------------------------------------------*/
PoE_RC_e poeCommOsMutexInit(void)
{

#ifdef _LINUX_PC_ARC_

	/* initializes the mutex */
	if(pthread_mutex_init(&poeMutex, NULL) == 0)
		return PoE_RC_SUCSESS;
	
	return PoE_RC_ERR_MUTEX_INIT;
#else
#error poeCommOsMutexInit function should be Implement.
		return PoE_RC_SUCSESS;
	
#endif

}





/*---------------------------------------------------------------------
 *    description:     destroy the mutex - Clean up a mutex that is no longer needed
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   ePOE_STATUS_OK                      - operation succeed
 *              ePOE_STATUS_ERR_MUTEX_DESTROY_ERROR - operation failed due to mutex initialize operation error
 *---------------------------------------------------------------------*/

PoE_RC_e poeCommOsMutexDetroy(void)
{
#ifdef _LINUX_PC_ARC_

	if(pthread_mutex_destroy(&poeMutex) == 0 )
		return PoE_RC_SUCSESS;
	
	return PoE_RC_ERR_MUTEX_DESTROY;
#else
#error poeCommOsMutexDetroy function should be Implement.
			return PoE_RC_SUCSESS;
		
#endif

}

/*---------------------------------------------------------------------
 *    description:  locking a mutex
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   ePOE_STATUS_OK                       - operation succeed
 *              e_POE_STATUS_ERR_MUTEX_LOCK_ERROR     - operation failed due to mutex lock operation error
 *---------------------------------------------------------------------*/
PoE_RC_e poeCommOsMutexLock(void)
{

#ifdef _LINUX_PC_ARC_

	if(pthread_mutex_lock(&poeMutex) == 0 )
		return PoE_RC_SUCSESS;
	
	return PoE_RC_ERR_MUTEX_LOCK_UNLOCK;
	
#else
#error poeCommOsMutexLock function should be Implement.
				return PoE_RC_SUCSESS;
			
#endif

}



/*---------------------------------------------------------------------
 *    description:     Unlocking or releasing a mutex
 *
 *    input :   none
 *
 *    output:   none
 *
 *    return:   ePOE_STATUS_OK                       - operation succeed
 *              e_POE_STATUS_ERR_MUTEX_UNLOCK_ERROR   - operation failed due to mutex unlock operation error
 *---------------------------------------------------------------------*/
PoE_RC_e poeCommOsMutexUnLock(void)
{

#ifdef _LINUX_PC_ARC_

	if(pthread_mutex_unlock(&poeMutex) == 0 )
		return PoE_RC_SUCSESS;
	
	return PoE_RC_ERR_MUTEX_LOCK_UNLOCK;
#else
#error poeCommOsMutexUnLock function should be Implement.
					return PoE_RC_SUCSESS;
				
#endif

}




/*=========================================================================
/ End of Arcitecture Definition
/========================================================================*/

