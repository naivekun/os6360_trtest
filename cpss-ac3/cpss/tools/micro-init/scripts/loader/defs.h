/*******************************************************************************
*                Copyright 2016, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
********************************************************************************
*/
/**
********************************************************************************
* @file defs.h
*
* @brief SatR utility
*
* @version   1
********************************************************************************
*/

#ifndef DEFS_H_INC
#define DEFS_H_INC

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

#define BOOLEAN unsigned long

#define MV_OK			    (0)	/* Operation succeeded */
#define MV_FAIL			    (1)	/* Operation failed    */

/* This macro returns absolute value */
#define MV_ABS(number)  (((int)(number) < 0) ? -(int)(number) : (int)(number))

/* typedefs */
typedef unsigned int MV_U32;
typedef int MV_BOOL;

typedef int8_t              INT_8;
typedef int16_t             INT_16;
typedef int32_t             INT_32;
typedef int64_t             INT_64;

typedef uint8_t             UINT8;
typedef uint8_t             UINT_8;
typedef uint16_t            UINT_16;
typedef uint32_t            UINT32;
typedef uint32_t            UINT_32;
typedef uint64_t            UINT_64;

/* Pointers typedef for arithmetic & logic operations */
typedef intptr_t            INT_ARCH_PTR;
typedef uintptr_t           UINT_ARCH_PTR;

/* sysv */
typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;


typedef int MV_STATUS;
typedef unsigned int MV_U32;
typedef int MV_BOOL;


#endif


