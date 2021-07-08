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
*  Description: contain the functions declaration for comm os functions.
*
*************************************************************************/



#ifndef _HOST_POE_COMM_OS_H_
#define _HOST_POE_COMM_OS_H_



/****************************************************************************
*                         EXPORTED FUNCTIONS PROTOTYPES
*****************************************************************************/
PoE_RC_e poeCommOsSleep(UINT16 sleepTime);
PoE_RC_e poeCommOsMutexInit(void);
PoE_RC_e poeCommOsMutexDetroy(void);
PoE_RC_e poeCommOsMutexLock(void);
PoE_RC_e poeCommOsMutexUnLock(void);



/****************************************************************************
*                         EXPORTED MACROS PROTOTYPES
*****************************************************************************/


#define OS_SLEEP(_time) \
	PoE_RC_e poe_rc =  poeCommOsSleep(_time); \
	if (poe_rc != PoE_RC_SUCSESS)  \
		return poe_rc;



#define MUTEX_LOCK \
	PoE_RC_e poe_rc =  poeCommOsMutexLock();\
	if(poe_rc != PoE_RC_SUCSESS)  \
		return poe_rc;


#define MUTEX_UNLOCK \
	poe_rc =  poeCommOsMutexUnLock();\
	if(poe_rc != PoE_RC_SUCSESS)  \
		return poe_rc;

#endif /* _HOST_POE_COMM_DEVICE_H_ */

