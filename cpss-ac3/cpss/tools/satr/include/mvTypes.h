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

*******************************************************************************/

#ifndef __INCmvTypesh
#define __INCmvTypesh

/* Defines */

/* The following is a list of Marvell status    */
#define MV_ERROR		    (-1)
#define MV_OK			    (0)	/* Operation succeeded                   */
#define MV_FAIL			    (1)	/* Operation failed                      */
#define MV_BAD_VALUE        (2)	/* Illegal value (general)               */
#define MV_OUT_OF_RANGE     (3)	/* The value is out of range             */
#define MV_BAD_PARAM        (4)	/* Illegal parameter in function called  */
#define MV_BAD_PTR          (5)	/* Illegal pointer value                 */
#define MV_BAD_SIZE         (6)	/* Illegal size                          */
#define MV_BAD_STATE        (7)	/* Illegal state of state machine        */
#define MV_SET_ERROR        (8)	/* Set operation failed                  */
#define MV_GET_ERROR        (9)	/* Get operation failed                  */
#define MV_CREATE_ERROR     (10)	/* Fail while creating an item           */
#define MV_NOT_FOUND        (11)	/* Item not found                        */
#define MV_NO_MORE          (12)	/* No more items found                   */
#define MV_NO_SUCH          (13)	/* No such item                          */
#define MV_TIMEOUT          (14)	/* Time Out                              */
#define MV_NO_CHANGE        (15)	/* Parameter(s) is already in this value */
#define MV_NOT_SUPPORTED    (16)	/* This request is not support           */
#define MV_NOT_IMPLEMENTED  (17)	/* Request supported but not implemented */
#define MV_NOT_INITIALIZED  (18)	/* The item is not initialized           */
#define MV_NO_RESOURCE      (19)	/* Resource not available (memory ...)   */
#define MV_FULL             (20)	/* Item is full (Queue or table etc...)  */
#define MV_EMPTY            (21)	/* Item is empty (Queue or table etc...) */
#define MV_INIT_ERROR       (22)	/* Error occured while INIT process      */
#define MV_HW_ERROR         (23)	/* Hardware error                        */
#define MV_TX_ERROR         (24)	/* Transmit operation not succeeded      */
#define MV_RX_ERROR         (25)	/* Recieve operation not succeeded       */
#define MV_NOT_READY	    (26)	/* The other side is not ready yet       */
#define MV_ALREADY_EXIST    (27)	/* Tried to create existing item         */
#define MV_OUT_OF_CPU_MEM   (28)	/* Cpu memory allocation failed.         */
#define MV_NOT_STARTED      (29)	/* Not started yet                       */
#define MV_BUSY             (30)	/* Item is busy.                         */
#define MV_TERMINATE        (31)	/* Item terminates it's work.            */
#define MV_NOT_ALIGNED      (32)	/* Wrong alignment                       */
#define MV_NOT_ALLOWED      (33)	/* Operation NOT allowed                 */
#define MV_WRITE_PROTECT    (34)	/* Write protected                       */
#define MV_DROPPED          (35)	/* Packet dropped                        */
#define MV_STOLEN           (36)	/* Packet stolen */
#define MV_CONTINUE         (37)        /* Continue */
#define MV_RETRY		    (38)	/* Operation failed need retry           */

#define MV_INVALID  (int)(-1)

#define MV_FALSE	0
#define MV_TRUE     (!(MV_FALSE))

#ifndef NULL
#define NULL ((void *)0)
#endif

/* typedefs */

typedef char MV_8;
typedef unsigned char MV_U8;

typedef int MV_32;
typedef unsigned int MV_U32;

typedef short MV_16;
typedef unsigned short MV_U16;

#ifdef MV_PPC64
typedef long MV_64;
typedef unsigned long MV_U64;
#else
typedef long long MV_64;
typedef unsigned long long MV_U64;
#endif

typedef long MV_LONG;		/* 32/64 */
typedef unsigned long MV_ULONG;	/* 32/64 */

typedef int MV_STATUS;
typedef int MV_BOOL;
typedef void MV_VOID;
typedef float MV_FLOAT;

typedef int (*MV_FUNCPTR) (void);	/* ptr to function returning int   */
typedef void (*MV_VOIDFUNCPTR) (void);	/* ptr to function returning void  */
typedef double (*MV_DBLFUNCPTR) (void);	/* ptr to function returning double */
typedef float (*MV_FLTFUNCPTR) (void);	/* ptr to function returning float */


#endif /* __INCmvTypesh */
