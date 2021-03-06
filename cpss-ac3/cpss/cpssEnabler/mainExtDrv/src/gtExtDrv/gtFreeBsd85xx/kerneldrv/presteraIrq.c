/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

********************************************************************************
*/
/**
********************************************************************************
* @file presteraIrq.c
*
* @brief functions in kernel mode special for prestera IRQ.
*
*/
#include <sys/types.h>
#include <sys/lock.h>
#include <sys/sema.h>
#include <sys/malloc.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>

#ifdef STANDALONE_TEST
#include "include/pssBspApis.h"
#else
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>
#include <gtExtDrv/drivers/pssBspApis.h>
#endif 

#include "include/presteraIrq.h"

/* #define INT_DEBUG */
#ifdef INT_DEBUG
#define DBG_PRINTF(x)   printf x
#else 
#define DBG_PRINTF(x)
#endif 

/* Locals */
MALLOC_DECLARE(M_PRESTINTDATA);
MALLOC_DEFINE(M_PRESTINTDATA, "prestIntData", "prestera interrupt data");

/**
* @internal intConnect function
* @endinternal
*
* @brief   connect and interrupt via register it at the kernel.
*
* @param[in] int                      - the interrupt vector number to connect
* @param[in] routine                  - the bound  for this interrupt vector
*
* @retval 0 on success,            -1 otherwise.
*/
int intConnect
(
    unsigned int        intVec, 
    void                *routine, 
    struct intData      **cookie
)
{
    struct intData *intDataPtr = NULL;	
	GT_STATUS rc;

	DBG_PRINTF(("intConnect start\n"));

    /* it's special malloc for kernel see description in malloc(9) */    
    MALLOC(intDataPtr, struct intData*, sizeof(struct intData), M_PRESTINTDATA, 
            M_ZERO | M_NOWAIT);
    if (NULL == intDataPtr)
    {
        printf("kernel malloc failed\n");
        return ENOMEM;
    }
	*cookie = intDataPtr;

	memset(intDataPtr, 0, sizeof(struct intData));    
    intDataPtr->intVec = intVec;		

	DBG_PRINTF(("intConnect:intVec=%d\n", intVec));

    rc = bspIntConnect(intVec, routine, (GT_U32)intDataPtr);

	DBG_PRINTF(("intConnect:bspIntConnect rc=%d\n", rc));

	return (int)rc;
}



