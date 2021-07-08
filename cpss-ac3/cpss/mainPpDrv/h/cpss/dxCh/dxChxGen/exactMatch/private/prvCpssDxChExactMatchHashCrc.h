/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChExactMatchHashCrc.h
*
* @brief CRC Hash functions
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChExactMatchHashCrc_h
#define __prvCpssDxChExactMatchHashCrc_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/common/cpssTypes.h>

#define CRC_16_WIDTH  (8 * sizeof(GT_U16))
#define CRC_16_TOPBIT 0x8000

#define CRC_32_WIDTH  (8 * sizeof(GT_U32))
#define CRC_32_TOPBIT 0x80000000



/**
* @internal prvCpssDxChExactMatchHashCrc_64 function
* @endinternal
*
* @brief   This function calculates CRC with 64 bit poly for Exact match key.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message .
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value .
* @param[in]  mode      - which 16 bit return to ans .
* @param[out] ans       - the hash result.
*/
GT_VOID prvCpssDxChExactMatchHashCrc_64
(
    IN GT_U8                    *message,
    IN GT_U32                    nBits,
    IN GT_U64                    poly,
    IN GT_U64                    seed,
    IN GT_U8                     mode,
    OUT GT_U32                  *ans
);

/**
* @internal prvCpssDxChExactMatchHashCrc_32 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message.
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value.
* @param[out] ans       - the hash result.
*/
GT_VOID prvCpssDxChExactMatchHashCrc_32
(
    IN GT_U8                    *message,
    IN GT_U32                    nBits,
    IN GT_U32                    poly,
    IN GT_U32                    seed,
    IN GT_U8                     mode,
    OUT GT_U32                  *ans
);

/**
* @internal prvCpssDxChExactMatchHashCrc_16 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value
* @param[out] ans       - the hash result.
*/

GT_VOID prvCpssDxChExactMatchHashCrc_16
(
    IN GT_U8                    *message,
    IN GT_U32                    nBits,
    IN GT_U32                    poly,
    IN GT_U32                    seed,
    OUT GT_U32                  *ans
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __prvCpssDxChExactMatchHashCrc_h */


