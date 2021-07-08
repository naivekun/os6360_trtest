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
*  Description: contain the declaration of the functions.
*
*************************************************************************/


#ifndef _HOST_POE_COMM_DEVICE_H_
#define _HOST_POE_COMM_DEVICE_H_

#include "../../host_poe_api/inc/host_poe_api.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>    /* For mode constants */
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
/****************************************************************************
*                         EXPORTED FUNCTIONS PROTOTYPES
*****************************************************************************/
PoE_RC_e poeCommDevOpenUART(void);
PoE_RC_e poeCommDevCloseUART(void);
PoE_RC_e poeCommDevWriteUART(void);
PoE_RC_e poeCommDevReadUART(UINT8 bufLen, UINT8* bufLenRead,BOOL readTimeout);
PoE_RC_e poeCommDevClearUART(void);
PoE_RC_e poeCommDevOpenI2C(void);
PoE_RC_e poeCommDevCloseI2C(void);
PoE_RC_e poeCommDevWriteI2C(UINT16 bufLen);
PoE_RC_e poeCommDevReadI2C (UINT16 bufLen, UINT16 *bufLenRead, BOOL readTimeout);

#define PRODUCER_TX_KEY 4321
#define PRODUCER_RX_KEY 1234

#define CONSUMER_TX_KEY 1234
#define CONSUMER_RX_KEY 4321

PoE_RC_e poeCommDevOpenQueue(void);
PoE_RC_e poeCommDevCloseQueue(void);
PoE_RC_e poeCommDevWriteQueue(void);
PoE_RC_e poeCommDevReadQueue (UINT16 bufLen, UINT16 *bufLenRead, BOOL readTimeout);

PoE_RC_e createMessageQueue(char *_name, key_t key_tx, key_t key_rx);
PoE_RC_e closeMessageQueue( void );
PoE_RC_e receiveMsg(char *p, int size , OUT UINT16 *number_of_bytes_actually_read);
PoE_RC_e sendMsg(char *p, int size);
PoE_RC_e poeCommDevClearQueue();

#endif /* _HOST_POE_COMM_DEVICE_H_ */

