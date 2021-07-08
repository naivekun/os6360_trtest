#include <Copyright.h>
/**
********************************************************************************
* @file gtUtils.h
*
* @brief API/Structure definitions for Marvell Common Utils functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtUtils.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell Common Utils functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtUtils_h
#define __prvCpssDrvGtUtils_h

#include <msApiTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported Common Utils Types                                              */
/****************************************************************************/

#define GT_CPSS_INVALID_PHY             0xFF
#define GT_CPSS_INVALID_PORT            0xFF
#define GT_CPSS_INVALID_PORT_VEC        0xFFFFFFFF

#define GT_UNUSED_PARAM(_a)    (_a)=(_a)

#ifdef PRV_CPSS_DEBUG_QD
#define PRV_CPSS_DBG_INFO(x) prvCpssDrvGtDbgPrint x
#else
#define PRV_CPSS_DBG_INFO(x);
/*#define PRV_CPSS_DBG_INFO(x) printf x*/
#endif /* PRV_CPSS_DEBUG_QD */

/*
 * Common util bit operation Macros
 */
#define PRV_CPSS_BIT(n)                  ( 1<<(n) )
#define PRV_CPSS_BIT_SET(y, mask)        ( y |=  (mask) )
#define PRV_CPSS_BIT_CLEAR(y, mask)      ( y &= ~(mask) )
#define PRV_CPSS_BIT_FLIP(y, mask)       ( y ^=  (mask) )

/*! Create a bitmask of length \a len.*/
#define PRV_CPSS_BIT_MASK(len)           ( PRV_CPSS_BIT(len)-1 )
/*! Create a bitfield mask of length \a starting at bit \a start.*/
#define PRV_CPSS_BF_MASK(start, len)     ( PRV_CPSS_BIT_MASK(len)<<(start) )
/*! Prepare a bitmask for insertion or combining.*/
#define PRV_CPSS_BF_PREP(x, start, len)  ( ((x)&PRV_CPSS_BIT_MASK(len)) << (start) )
/*! Extract a bitfield of length \a len starting at bit \a start from \a y.*/
#define PRV_CPSS_BF_GET(y, start, len)   ( ((y)>>(start)) & PRV_CPSS_BIT_MASK(len) )
/*! Insert a new bitfield value \a x into \a y.*/
#define PRV_CPSS_BF_SET(y, x, start, len)    \
    ( y= ((y) &~ PRV_CPSS_BF_MASK(start, len)) | PRV_CPSS_BF_PREP(x, start, len) )

/****************************************************************************/
/* Exported Common Utils Functions                                          */
/****************************************************************************/


/*******************************************************************************
* prvCpssDrvGtMemSet
*
* DESCRIPTION:
*       Set a block of memory
*
* INPUTS:
*       start  - start address of memory block for setting
*       simbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * prvCpssDrvGtMemSet
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
);

/*******************************************************************************
* prvCpssDrvGtMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * prvCpssDrvGtMemCpy
(
    IN void *       destination,
    IN const void * source,
    IN GT_U32       size
);


/**
* @internal prvCpssDrvGtMemCmp function
* @endinternal
*
* @brief   Compares given memories.
*
* @param[in] src1[]                   - source 1
* @param[in] src2[]                   - source 2
* @param[in] size                     -  of memory to copy
*                                       0, if equal.
*                                       negative number, if src1 < src2.
*                                       positive number, if src1 > src2.
*/
int prvCpssDrvGtMemCmp
(
    IN char src1[],
    IN char src2[],
    IN GT_U32 size
);

/**
* @internal prvCpssDrvGtStrlen function
* @endinternal
*
* @brief   Determine the length of a string
*
* @param[in] source                   - string
*
* @retval size                     - number of characters in string, not including EOS.
*/
GT_U32 prvCpssDrvGtStrlen
(
    IN const void * source
);

/**
* @internal prvCpssDrvGtDelay function
* @endinternal
*
* @brief   Wait for the given uSec and return.
*         Current Switch devices with Gigabit Ethernet Support require 250 uSec
*         of delay time for PPU to be disabled.
*         Since this function is System and/or OS dependent, it should be provided
*         by each MSD user.
*/
void prvCpssDrvGtDelay
(
    IN const unsigned int delayTime
);


/**
* @internal prvCpssDrvQdLong2Char function
* @endinternal
*
* @brief   This function converts long value to char value
*
* @param[in] data                     - long value
*                                       logical port vector
*/
GT_U8 prvCpssDrvQdLong2Char
(
    GT_U32 data
);

/**
* @internal prvCpssDrvQdShort2Char function
* @endinternal
*
* @brief   This function converts short value to char value
*
* @param[in] data                     - short value
*                                       logical port vector
*/
GT_U8 prvCpssDrvQdShort2Char
(
    GT_U16 data
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtUtils_h */

