/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssMath.c
*
* DESCRIPTION:
*       Math operations for CPSS.
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
******************************************************************************/

#include <cpssCommon/private/prvCpssMath.h>
#include <cpssCommon/cpssPresteraDefs.h>

/**
* @internal prvCpssMathAdd64 function
* @endinternal
*
* @brief   Summarize two 64 bits values.
*
* @param[in] x                        - first value for sum.
* @param[in] y                        - second value for sum
*                                       Calculated sum.
*
* @note This function does not care 65 bit overflow.
*
*/
GT_U64  prvCpssMathAdd64
(
    IN  GT_U64 x,
    IN  GT_U64 y
)
{
    GT_U64  z;
    GT_U32  maxVal;

    maxVal = y.l[0];
    if (x.l[0] > y.l[0])
        maxVal = x.l[0];

    z.l[0] = x.l[0] + y.l[0];           /* low order word sum  */
    z.l[1] = x.l[1] + y.l[1];           /* high order word sum */
    z.l[1] += (z.l[0] < maxVal) ? 1:0;  /* low-word overflow */

    return z;
}

/**
* @internal prvCpssMathSub64 function
* @endinternal
*
* @brief   Subtract two 64 bits values.
*
* @param[in] x                        - first value for difference.
* @param[in] y                        - second value for difference
*
* @retval Calculated difference x  - y.
*/
GT_U64  prvCpssMathSub64
(
    IN  GT_U64 x,
    IN  GT_U64 y
)
{
    GT_U64  z;

    z.l[0] = x.l[0] - y.l[0];            /* low order word difference  */
    z.l[1] = x.l[1] - y.l[1];            /* high order word difference */
    z.l[1] -= (x.l[0] < y.l[0]) ? 1 : 0; /* low-word borrow            */

    return z;
}

/**
* @internal prvCpssMathCompare64 function
* @endinternal
*
* @brief   Compare two 64 bits values.
*
* @param[in] x                        - first value.
* @param[in] y                        - second value.
*                                       x > y ==> 1
*                                       x == y ==> 0
*
* @retval x <  y ==>               -1
*/
int  prvCpssMathCompare64
(
    IN  GT_U64 x,
    IN  GT_U64 y
)
{
    if (x.l[1] > y.l[1]) return 1;
    if (x.l[1] < y.l[1]) return -1;
    if (x.l[0] > y.l[0]) return 1;
    if (x.l[0] < y.l[0]) return -1;
    return 0;
}

/**
* @internal prvCpssMathDiv64By16 function
* @endinternal
*
* @brief   Divides 64 bits value by 16 bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*                                       Calculated quotient.
*/
GT_U64  prvCpssMathDiv64By16
(
    IN  GT_U64 x,
    IN  GT_U16 y
)
{
    GT_U64  z; /* result   */
    GT_U32  v; /* value    */
    GT_U32  r; /* reminder */
    GT_U32  q; /* quotient */

    /* represent the 64-bit value as 4 16-bit figures number ABCD */
    /* divide it as manually by one 16-bit figure nymber Y        */

    /* begin with figures AB */
    v = x.l[1];
    q = v / y;
    r = v % y;

    /* save result */
    z.l[1] = q;

    /* including the figure C */
    v = ((r << 16) | ((x.l[0] >> 16) & 0xFFFF));
    q = v / y;
    r = v % y;

    /* save result */
    z.l[0] = (q << 16);

    /* including the last figure D, reminder not needed */
    v = ((r << 16) | (x.l[0] & 0xFFFF));
    q = v / y;

    /* save result */
    z.l[0] |= q;

    return z;
}

/**
* @internal prvCpssMathMod64By16 function
* @endinternal
*
* @brief   Calculate reminder from division 64-bits value by 16-bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*                                       Calculated reminder.
*/
GT_U16  prvCpssMathMod64By16
(
    IN  GT_U64 x,
    IN  GT_U16 y
)
{
    GT_U32  v; /* value    */
    GT_U32  r; /* reminder */

    /* represent the 64-bit value as 4 16-bit figures number ABCD */
    /* devide it as manually by one 16-bit figure nymber Y        */

    /* begin with figures AB */
    v = x.l[1];
    r = v % y;

    /* including the figure C */
    v = ((r << 16) | ((x.l[0] >> 16) & 0xFFFF));
    r = v % y;

    /* including the last figure D */
    v = ((r << 16) | (x.l[0] & 0xFFFF));
    r = v % y;

    return (GT_U16)r;
}

/**
* @internal prvCpssMathMul64 function
* @endinternal
*
* @brief   Calculate multiplication of 32-bits value by 32-bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*                                       Calculated xy.
*/
GT_U64 prvCpssMathMul64
(
    IN GT_U32 x,
    IN GT_U32 y
)
{
    GT_U32 x0, x1, y0, y1, k, t;
    GT_U32 w1, w2, w3;
    GT_U64 z;

    x0 = x >> 16; x1 = x & 0xFFFF;
    y0 = y >> 16; y1 = y & 0xFFFF;

    t = x1*y1;
    w3 = t & 0xFFFF;
    k = t >> 16;

    t = x0*y1 + k;
    w2 = t & 0xFFFF;
    w1 = t >> 16;

    t = x1*y0 + w2;
    k = t >> 16;

    z.l[0] = (t << 16) + w3;
    z.l[1] = x0*y0 + w1 + k;;
    return z;
}

/**
* @internal prvCpssMathDiv64By32 function
* @endinternal
*
* @brief   Calculate quotient and reminder of division of 64-bits value by 32-bits value.
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*
* @param[out] modPtr                   - (pointer to) 32-bit reminder of division.
*                                      Can be NULL.
*
* @retval Calculated 64            -bit x/y.
*/
GT_U64 prvCpssMathDiv64By32
(
    IN  GT_U64  x,
    IN  GT_U32  y,
    OUT GT_U32  *modPtr
)
{
    GT_U64 q64; /* quotient       */
    GT_U64 d64; /* work dividend  */
    GT_U64 w64; /* work           */
    GT_U32 i;   /* index          */
    GT_U32 s;   /* shift value    */
    int    cmp; /* compare result */

    if (y == 0) y = 1; /* to avoid exception */
    if (y == 1)
    {
        if (modPtr) *modPtr = 0;
        return x;
    }

    /* initialization before loop of shifts */
    q64.l[0] = 0;
    q64.l[1] = x.l[1] / y;
    d64.l[0] = x.l[0];
    d64.l[1] = x.l[1] % y;

    /* shift loop                                            */
    /* unsigned values shifted to both sides padded by zeros */
    for (i = 1; (i < 32); i++)
    {
        if ((d64.l[0] == 0) && (d64.l[1] == 0)) break;
        s = 32 - i;
        w64.l[1] = y >> i;
        w64.l[0] = y << s;
        cmp = prvCpssMathCompare64(d64, w64);
        if (cmp < 0) continue;
        d64 = prvCpssMathSub64(d64, w64);
        q64.l[0] |= (1 << s);
    }
    /* loop last step */
    w64.l[1] = 0;
    w64.l[0] = y;
    cmp = prvCpssMathCompare64(d64, w64);
    if (cmp >= 0)
    {
        d64 = prvCpssMathSub64(d64, w64);
        q64.l[0] |= 1;
    }

    if (modPtr) *modPtr = d64.l[0];
    return q64;
}

/**
* @internal prvCpssMathIsPowerOf2 function
* @endinternal
*
* @brief   This routine checks whether number is a power of 2.
*
* @param[in] number                   - the  to verify.
*
* @retval GT_TRUE                  - for true.
* @retval GT_FALSE                 - for false.
*/
GT_BOOL prvCpssMathIsPowerOf2
(
    IN GT_U32 number
)
{
    if (number == 0)
        return GT_FALSE;

    if(((number | (number - 1)) - (number - 1)) == number)
        return GT_TRUE;
    else
        return GT_FALSE;
}

/**
* @internal prvCpssMathLog2 function
* @endinternal
*
* @brief   This function calculates log(x) by base 2.
*
* @param[in] num                      - The number to perform the calculation on. This number must be a
*                                      power of 2.
*
* @param[out] resultPtr                - pointer to log(num) result.
*                                       GT_OK on success,
*                                       GT_FAIL otherwise (if the number is not a power of 2).
*/
GT_STATUS prvCpssMathLog2
(
    IN  GT_U32  num,
    OUT GT_U32  *resultPtr
)
{
    /* Holds the most signifacant part of the remaining number. */
    GT_U32  msPart;
    /* Holds the least signifacant part of the remaining number. */
    GT_U32  lsPart;
    /* Mask to be used to get the lsPart from the current number. */
    GT_U32  lsMask;
    /* Number of bits to be shifted inorder to get the msPart from the current
       number. */
    GT_U32  msShift;
    /* Calculated log2(num). */
    GT_U32  res;

    if(prvCpssMathIsPowerOf2(num) == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    if(prvCpssMathIsPowerOf2(num) == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    res     = 0;
    lsMask  = 0xFFFF;
    msShift = 16;

    lsPart  = num & lsMask;
    msPart  = num >> msShift;

    while((msPart & 0x1) != 1)
    {
        if(lsPart != 0)
        {
            msShift = msShift / 2;
            lsMask  = lsMask >> msShift;
            msPart  = lsPart >> msShift;
            lsPart  = lsPart & lsMask;
        }
        else    /* msPart != 0 */
        {
            res     += msShift;
            msShift = msShift / 2;
            lsMask  = lsMask >> msShift;
            lsPart  = msPart & lsMask;
            msPart  = msPart >> msShift;
        }
    }

    res += msShift;
    *resultPtr = res;
    return GT_OK;
}

/**
* @internal prvCpssMathPowerOf2 function
* @endinternal
*
* @brief   Calculate power of 2
*
* @param[in] exponent                 - the exponent, a value between 0 to 7
*
* @retval GT_BAD_PARAM             - if exponent is not between 0 to 7
* @retval GT_OK                    - otherwise
*/
GT_STATUS prvCpssMathPowerOf2
(
    IN  GT_U8   exponent,
    OUT GT_U8   *result
)
{
    GT_U8 i;

    if (exponent > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *result = 1;

    for (i = 1; i <= exponent; i++)
    {
        *result <<= 1;
    }
    return GT_OK;
}

/**
* @internal prvCpssRoundDiv function
* @endinternal
*
* @brief   Divide and round a value with accuracy of 3 digits.
*
* @param[in] value                    - Value to divide.
* @param[in] divisor                  - the divisor.
*
* @param[out] resultPtr                - pointer to result after round
*                                       GT_OK
*/
GT_STATUS prvCpssRoundDiv
(
    IN  GT_U32  value,
    IN  GT_U32  divisor,
    OUT GT_U32 *resultPtr
)
{
    GT_U32 big_val;
    GT_U32 big_rem; /* remainder */
    GT_U32 ret_result;

    ret_result = value / divisor; /* Not rounded result */
    big_val = value * 1000;
    big_rem = (big_val / divisor) % 1000;
    if ( 500 <= big_rem )
    {
        ++ret_result;
    }
    *resultPtr = ret_result;
    return GT_OK;
}

/**
* @internal prvCpssMathShr64 function
* @endinternal
*
* @brief   Shift 64-bit value right by 1.
*
* @param[in] x    - value to shift right.
*
* @retval Shifted 64-bit value x>>1
*/
GT_U64 prvCpssMathShr64
(
    IN  GT_U64      x
)
{
    GT_U64 rv;
    rv.l[0] = (x.l[0] >> 1) | (x.l[1] << 31);
    rv.l[1] = x.l[1] >> 1;
    return rv;
}

/**
* @internal prvCpssMathShl64 function
* @endinternal
*
* @brief   Shift 64-bit value left by 1.
*
* @param[in] x    - value to shift left.
*
* @retval Shifted 64-bit value x << 1
*/
GT_U64 prvCpssMathShl64
(
    IN  GT_U64      x
)
{
    GT_U64 rv;
    rv.l[1] = (x.l[1] << 1) | (x.l[0] >> 31);
    rv.l[0] =  x.l[0] << 1;
    return rv;
}

/**
* @internal prvCpssMathDiv64 function
* @endinternal
*
* @brief   Calculate quotient and reminder of division of 64-bits values
*
* @param[in] x                        - dividend.
* @param[in] y                        - divisor
*
* @param[out] resPtr                  - (pointer to) x/y.
* @param[out] modPtr                  - (pointer to) reminder of division.
*                                       Can be NULL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on bad pointer to result
* @retval GT_BAD_PARAM             - on y = 0
*
*/
GT_STATUS prvCpssMathDiv64
(
    IN  GT_U64      x,
    IN  GT_U64      y,
    OUT GT_U64      *resPtr,
    OUT GT_U64      *modPtr
)
{
    GT_U64 d1, p2;

    CPSS_NULL_PTR_CHECK_MAC(resPtr);

    if ((y.l[0] == 0) && (y.l[1] == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((y.l[0] == 1) && (y.l[1] == 0))
    {
        resPtr->l[0] = x.l[0];
        resPtr->l[1] = x.l[1];
        if(modPtr)
        {
            modPtr->l[0] = 0;
            modPtr->l[1] = 0;
        }
        return GT_OK;
    }

    if(prvCpssMathCompare64(x, y) == -1)
    {
        resPtr->l[0] = 0;
        resPtr->l[1] = 0;
    }
    else
    {
        resPtr->l[0] = 0;
        resPtr->l[1] = 0;

        /* calculate biggest power of 2 times d that's <= x */
        p2.l[0] = 1;
        p2.l[1] = 0;
        d1.l[0] = y.l[0];
        d1.l[1] = y.l[1];
        x = prvCpssMathSub64(x, d1);
        while(prvCpssMathCompare64(x, d1) != -1)
        {
            x = prvCpssMathSub64(x, d1);
            d1 = prvCpssMathAdd64(d1, d1);
            p2 = prvCpssMathAdd64(p2, p2);
        }
        x = prvCpssMathAdd64(x, d1);

        while((p2.l[0] != 0) || (p2.l[1] != 0))
        {
            if (prvCpssMathCompare64(x, d1) != -1)
            {
                x = prvCpssMathSub64(x, d1);
                *resPtr = prvCpssMathAdd64(*resPtr, p2);
            }
            p2 = prvCpssMathShr64(p2);
            d1 = prvCpssMathShr64(d1);
        }
    }

    /* return remainder if they asked for it */
    if (NULL != modPtr) {
        modPtr->l[0] = x.l[0];
        modPtr->l[1] = x.l[1];
    }

    return GT_OK;
}

/**
* @internal prvCpssMathXor64 function
* @endinternal
*
* @brief   Calculate XOR of 2  64-bits numbers.
*
* @param[in] x  - first value.
* @param[in] y  - second value.
*
* @retval Xored 64-bit value
*/
GT_U64 prvCpssMathXor64
(
    IN  GT_U64      x ,
    IN  GT_U64      y
)
{
    GT_U64 rv;
    rv.l[0] = x.l[0] ^ y.l[0] ;
    rv.l[1] = x.l[1] ^ y.l[1] ;

    return rv;
}
