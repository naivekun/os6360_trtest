#include <Copyright.h>
/*
 * Copyright 2016, Marvell Semiconductor Ltd.
 *
 * THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 * NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 * OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 * DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 * THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED
 * OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
 */

/*
 *
 * DESCRIPTION:
 *     This file defines common data types used on Host sides.
 */
/* do not allow to collide with cpss types */
#define CPSS____MV_TYPES_H

#ifndef CPSS_MV_TYPES_H
#define CPSS_MV_TYPES_H

/* use CPSS types */
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define    GT_ETHERNET_HEADER_SIZE        (6)

#ifndef CPSS____MV_TYPES_H

#error must use CPSS definitions!
/* general */

#undef IN
#define IN
#undef OUT
#define OUT
#undef INOUT
#define INOUT


#ifndef NULL
#define NULL ((void*)0)
#endif

typedef void  GT_VOID;
typedef char  GT_8;
typedef short GT_16;
typedef long  GT_32;

typedef unsigned char  GT_U8;
typedef unsigned short GT_U16;
typedef unsigned long  GT_U32;
typedef unsigned int   GT_UINT;

typedef enum {
    GT_FALSE = 0,
    GT_TRUE  = 1
} GT_BOOL;

typedef struct
{
    GT_U8       arEther[GT_ETHERNET_HEADER_SIZE];
}GT_ETHERADDR;


/* status / error codes */
typedef int GT_STATUS;

#define GT_OK              (0x00)   /* Operation succeeded                   */
#define GT_FAIL            (0x01)   /* Operation failed                      */
#define GT_BAD_PARAM       (0x04)   /* Illegal parameter in function called  */
#define GT_NO_SUCH         (0x0D)   /* No such item                          */
#define GT_ALREADY_EXIST   (0x1B)   /* Tried to create existing item         */
#define GT_BAD_CPU_PORT    (0x20)   /* Input CPU Port is not valid physical port number */

#endif /*! CPSS____MV_TYPES_H*/

typedef GT_U32 GT_CPSS_LPORT;
typedef GT_U32 GT_CPSS_ETYPE;
typedef GT_U32 GT_CPSS_SEM;

typedef double GT_DOUBLE;

#ifdef __cplusplus
}
#endif

#endif /* CPSS_MV_TYPES_H */
