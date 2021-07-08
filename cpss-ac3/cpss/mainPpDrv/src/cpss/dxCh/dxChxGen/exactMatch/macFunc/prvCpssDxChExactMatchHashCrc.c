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
* @file prvCpssDxChExactMatchHashCrc.c
*
* @brief Exact Match hash calculate .
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHashCrc.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchLog.h>
#include <cpssCommon/private/prvCpssMath.h>


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
)
{
    GT_U8 byteMsg,bit ;
    GT_U64 remainder ;
    GT_U32 nBytes ,byte, tmp;
    nBytes = nBits / 8 ;


    tmp = 0x1000000;
    remainder.l[0] = seed.l[0];
    remainder.l[1] = seed.l[1];
    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        byteMsg = message[nBytes-byte-1];
        remainder.l[1] ^= byteMsg * tmp ;

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for ( bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder.l[1] & CRC_32_TOPBIT)
            {
                remainder = prvCpssMathXor64(prvCpssMathShl64(remainder),poly);
            }
            else
            {
                remainder = prvCpssMathShl64(remainder);
            }
        }
    }
    switch (mode)
    {
        case 0: * ans= (GT_U16) ( remainder.l[0] & 0xffff );           break;
        case 1: * ans= (GT_U16) ((remainder.l[0] & 0xffff0000)>>16 );  break;
        case 2: * ans= (GT_U16) ( remainder.l[1] & 0xffff);           break;
        case 3: * ans= (GT_U16) ((remainder.l[1] & 0xffff0000)>>16 );  break;
        default:
            break;
    }
}

/**
* @internal prvCpssDxChExactMatchHashCrc_32 function
* @endinternal
*
* @brief   This function calculates CRC with 32 bit poly for Exact match key.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message.
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value.
* @param[out] ans       - the hash result.
*
*/
GT_VOID prvCpssDxChExactMatchHashCrc_32
(
     IN GT_U8                    *message,
     IN GT_U32                    nBits,
     IN GT_U32                    poly,
     IN GT_U32                    seed,
     IN GT_U8                     mode,
     OUT GT_U32                  *ans
)
{
    GT_U8 byteMsg,bit ;
    GT_U32 remainder = seed;
    GT_U32 nBytes ,byte;

    nBytes = nBits / 8 ;

    /*
     * Perform modulo-2 division, a byte at a time - full byte .
     */
    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        byteMsg = message[nBytes-byte-1];
        remainder ^= (byteMsg << (CRC_32_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & CRC_32_TOPBIT)
            {
                remainder = (GT_U32)((remainder << 1) ^ poly);
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    if (mode == 0)
    {
        remainder=(remainder & 0xffff );
    }
    else
    {
        remainder=(remainder & 0xffff0000 )>>16;
    }
    *ans=(GT_U16)remainder;
}

/**
* @internal prvCpssDxChExactMatchHashCrc_16 function
* @endinternal
*
* @brief   This function calculates CRC with 16 bit poly for Exact match key.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value
* @param[out] ans       - the hash result.
*
*/
GT_VOID prvCpssDxChExactMatchHashCrc_16
(
     IN GT_U8                    *message,
     IN GT_U32                    nBits,
     IN GT_U32                    poly,
     IN GT_U32                    seed,
     OUT GT_U32                  *ans
)
{
    GT_U8 byteMsg,bit ;
    GT_U32 remainder = seed;
    GT_U32 nBytes ,byte;

    nBytes = nBits / 8 ;

    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
          byteMsg = message[nBytes-byte-1];
          remainder ^= (byteMsg << (CRC_16_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & CRC_16_TOPBIT)
            {
                remainder = (GT_U16)((remainder << 1) ^ poly);
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    *ans=remainder;

}
