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
* @file prvCpssDxChBrgFdbHashCrc.c
*
* @brief CRC hash calculate .
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHashCrc.h>

/* macro to get bit from the array of words */
#define _D(bit) (D[(bit)>>5] >> ((bit)&0x1f))
#define _C(bit) (c[(bit)>>5] >> ((bit)&0x1f))

#define _H(bit,_value)   U32_SET_FIELD_MASKED_MAC(h[(bit)>>5] , ((bit)&0x1f), 1 , _value)

#define _PARITY(x) x^=x>>16; x^=x>>8; x^=x>>4; x^=x>>2; x^=x>>1; x&=1

/**
* @internal prvCpssDxChBrgFdbHashCrc_16a_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 2 15 16)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16a_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xcfffbfff) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x0000fcff);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x9fff7ffe) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x0000f9ff);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xf0014003) ^ (c[0] & 0x00000f00);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe0028006) ^ (D[1] & 0x00000001) ^ (c[0] & 0x00001e00);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc005000c) ^ (D[1] & 0x00000003) ^ (c[0] & 0x00003c00);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x800a0018) ^ (D[1] & 0x00000007) ^ (c[0] & 0x00007800);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x00140030) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x0000f001);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x00280060) ^ (D[1] & 0x0000000e) ^ (c[0] & 0x0000e002);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x005000c0) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x0000c005);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x00a00180) ^ (D[1] & 0x00000008) ^ (c[0] & 0x0000800a);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x01400300) ^ (c[0] & 0x00000014);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x02800600) ^ (c[0] & 0x00000028);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x05000c00) ^ (c[0] & 0x00000050);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x0a001800) ^ (c[0] & 0x000000a0);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x14003000) ^ (c[0] & 0x00000140);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xe7ffdfff) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x0000fe7f);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16a_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 2 15 16)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16a_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xcfffbfff) ^ (D[1] & 0x00000bff) ^ (c[0] & 0x0000bffc);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x9fff7ffe) ^ (D[1] & 0x000007ff) ^ (c[0] & 0x00007ff9);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xf0014003) ^ (D[1] & 0x00000400) ^ (c[0] & 0x0000400f);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe0028006) ^ (D[1] & 0x00000801) ^ (c[0] & 0x0000801e);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc005000c) ^ (D[1] & 0x00000003) ^ (c[0] & 0x0000003c);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x800a0018) ^ (D[1] & 0x00000007) ^ (c[0] & 0x00000078);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x00140030) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x000000f0);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x00280060) ^ (D[1] & 0x0000001e) ^ (c[0] & 0x000001e0);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x005000c0) ^ (D[1] & 0x0000003c) ^ (c[0] & 0x000003c0);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x00a00180) ^ (D[1] & 0x00000078) ^ (c[0] & 0x00000780);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x01400300) ^ (D[1] & 0x000000f0) ^ (c[0] & 0x00000f00);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x02800600) ^ (D[1] & 0x000001e0) ^ (c[0] & 0x00001e00);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x05000c00) ^ (D[1] & 0x000003c0) ^ (c[0] & 0x00003c00);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x0a001800) ^ (D[1] & 0x00000780) ^ (c[0] & 0x00007800);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x14003000) ^ (D[1] & 0x00000f00) ^ (c[0] & 0x0000f001);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xe7ffdfff) ^ (D[1] & 0x000005ff) ^ (c[0] & 0x00005ffe);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16a_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 2 15 16)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16a_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xcfffbfff) ^ (D[1] & 0x00ffebff) ^ (c[0] & 0x00000ffe);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x9fff7ffe) ^ (D[1] & 0x01ffd7ff) ^ (c[0] & 0x00001ffd);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xf0014003) ^ (D[1] & 0x03004400) ^ (c[0] & 0x00003004);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe0028006) ^ (D[1] & 0x06008801) ^ (c[0] & 0x00006008);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc005000c) ^ (D[1] & 0x0c011003) ^ (c[0] & 0x0000c011);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x800a0018) ^ (D[1] & 0x08022007) ^ (c[0] & 0x00008022);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x00140030) ^ (D[1] & 0x0004400f) ^ (c[0] & 0x00000044);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x00280060) ^ (D[1] & 0x0008801e) ^ (c[0] & 0x00000088);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x005000c0) ^ (D[1] & 0x0011003c) ^ (c[0] & 0x00000110);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x00a00180) ^ (D[1] & 0x00220078) ^ (c[0] & 0x00000220);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x01400300) ^ (D[1] & 0x004400f0) ^ (c[0] & 0x00000440);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x02800600) ^ (D[1] & 0x008801e0) ^ (c[0] & 0x00000880);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x05000c00) ^ (D[1] & 0x011003c0) ^ (c[0] & 0x00001100);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x0a001800) ^ (D[1] & 0x02200780) ^ (c[0] & 0x00002200);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x14003000) ^ (D[1] & 0x04400f00) ^ (c[0] & 0x00004400);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xe7ffdfff) ^ (D[1] & 0x087ff5ff) ^ (c[0] & 0x000087ff);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16b_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 5 12 16)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16b_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x1c581911) ^ (D[1] & 0x0000000b) ^ (c[0] & 0x0000b1c5);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x38b03222) ^ (D[1] & 0x00000006) ^ (c[0] & 0x0000638b);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x71606444) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x0000c716);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe2c0c888) ^ (D[1] & 0x00000008) ^ (c[0] & 0x00008e2c);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc5819110) ^ (D[1] & 0x00000001) ^ (c[0] & 0x00001c58);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x975b3b31) ^ (D[1] & 0x00000008) ^ (c[0] & 0x00008975);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x2eb67662) ^ (D[1] & 0x00000001) ^ (c[0] & 0x000012eb);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x5d6cecc4) ^ (D[1] & 0x00000002) ^ (c[0] & 0x000025d6);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xbad9d988) ^ (D[1] & 0x00000004) ^ (c[0] & 0x00004bad);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x75b3b310) ^ (D[1] & 0x00000009) ^ (c[0] & 0x0000975b);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xeb676620) ^ (D[1] & 0x00000002) ^ (c[0] & 0x00002eb6);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0xd6cecc40) ^ (D[1] & 0x00000005) ^ (c[0] & 0x00005d6c);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xb1c58191) ^ (c[0] & 0x00000b1c);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x638b0322) ^ (D[1] & 0x00000001) ^ (c[0] & 0x00001638);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xc7160644) ^ (D[1] & 0x00000002) ^ (c[0] & 0x00002c71);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x8e2c0c88) ^ (D[1] & 0x00000005) ^ (c[0] & 0x000058e2);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16b_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 5 12 16)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16b_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x1c581911) ^ (D[1] & 0x0000040b) ^ (c[0] & 0x000040b1);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x38b03222) ^ (D[1] & 0x00000816) ^ (c[0] & 0x00008163);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x71606444) ^ (D[1] & 0x0000002c) ^ (c[0] & 0x000002c7);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe2c0c888) ^ (D[1] & 0x00000058) ^ (c[0] & 0x0000058e);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc5819110) ^ (D[1] & 0x000000b1) ^ (c[0] & 0x00000b1c);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x975b3b31) ^ (D[1] & 0x00000568) ^ (c[0] & 0x00005689);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x2eb67662) ^ (D[1] & 0x00000ad1) ^ (c[0] & 0x0000ad12);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x5d6cecc4) ^ (D[1] & 0x000005a2) ^ (c[0] & 0x00005a25);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xbad9d988) ^ (D[1] & 0x00000b44) ^ (c[0] & 0x0000b44b);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x75b3b310) ^ (D[1] & 0x00000689) ^ (c[0] & 0x00006897);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xeb676620) ^ (D[1] & 0x00000d12) ^ (c[0] & 0x0000d12e);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0xd6cecc40) ^ (D[1] & 0x00000a25) ^ (c[0] & 0x0000a25d);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xb1c58191) ^ (D[1] & 0x00000040) ^ (c[0] & 0x0000040b);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x638b0322) ^ (D[1] & 0x00000081) ^ (c[0] & 0x00000816);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xc7160644) ^ (D[1] & 0x00000102) ^ (c[0] & 0x0000102c);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x8e2c0c88) ^ (D[1] & 0x00000205) ^ (c[0] & 0x00002058);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16b_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 5 12 16)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16b_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x1c581911) ^ (D[1] & 0x059b040b) ^ (c[0] & 0x000059b0);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x38b03222) ^ (D[1] & 0x0b360816) ^ (c[0] & 0x0000b360);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x71606444) ^ (D[1] & 0x066c102c) ^ (c[0] & 0x000066c1);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe2c0c888) ^ (D[1] & 0x0cd82058) ^ (c[0] & 0x0000cd82);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc5819110) ^ (D[1] & 0x09b040b1) ^ (c[0] & 0x00009b04);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x975b3b31) ^ (D[1] & 0x06fb8568) ^ (c[0] & 0x00006fb8);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x2eb67662) ^ (D[1] & 0x0df70ad1) ^ (c[0] & 0x0000df70);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x5d6cecc4) ^ (D[1] & 0x0bee15a2) ^ (c[0] & 0x0000bee1);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xbad9d988) ^ (D[1] & 0x07dc2b44) ^ (c[0] & 0x00007dc2);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x75b3b310) ^ (D[1] & 0x0fb85689) ^ (c[0] & 0x0000fb85);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xeb676620) ^ (D[1] & 0x0f70ad12) ^ (c[0] & 0x0000f70a);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0xd6cecc40) ^ (D[1] & 0x0ee15a25) ^ (c[0] & 0x0000ee15);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xb1c58191) ^ (D[1] & 0x0859b040) ^ (c[0] & 0x0000859b);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x638b0322) ^ (D[1] & 0x00b36081) ^ (c[0] & 0x00000b36);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xc7160644) ^ (D[1] & 0x0166c102) ^ (c[0] & 0x0000166c);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x8e2c0c88) ^ (D[1] & 0x02cd8205) ^ (c[0] & 0x00002cd8);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16c_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 5 7 8 9 11 15 16)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16c_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x3fd4b4df) ^ (c[0] & 0x000003fd);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x407ddd61) ^ (c[0] & 0x00000407);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xbf2f0e1d) ^ (c[0] & 0x00000bf2);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x7e5e1c3a) ^ (D[1] & 0x00000001) ^ (c[0] & 0x000017e5);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc3688cab) ^ (D[1] & 0x00000002) ^ (c[0] & 0x00002c36);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xb905ad89) ^ (D[1] & 0x00000005) ^ (c[0] & 0x00005b90);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x720b5b12) ^ (D[1] & 0x0000000b) ^ (c[0] & 0x0000b720);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xdbc202fb) ^ (D[1] & 0x00000006) ^ (c[0] & 0x00006dbc);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x8850b129) ^ (D[1] & 0x0000000d) ^ (c[0] & 0x0000d885);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x2f75d68d) ^ (D[1] & 0x0000000b) ^ (c[0] & 0x0000b2f7);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x5eebad1a) ^ (D[1] & 0x00000006) ^ (c[0] & 0x000065ee);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x8203eeeb) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x0000c820);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x0407ddd6) ^ (D[1] & 0x00000009) ^ (c[0] & 0x00009040);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x080fbbac) ^ (D[1] & 0x00000002) ^ (c[0] & 0x00002080);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x101f7758) ^ (D[1] & 0x00000004) ^ (c[0] & 0x00004101);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x1fea5a6f) ^ (D[1] & 0x00000008) ^ (c[0] & 0x000081fe);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16c_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 5 7 8 9 11 15 16)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16c_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x3fd4b4df) ^ (D[1] & 0x00000970) ^ (c[0] & 0x00009703);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x407ddd61) ^ (D[1] & 0x00000b90) ^ (c[0] & 0x0000b904);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xbf2f0e1d) ^ (D[1] & 0x00000e50) ^ (c[0] & 0x0000e50b);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x7e5e1c3a) ^ (D[1] & 0x00000ca1) ^ (c[0] & 0x0000ca17);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc3688cab) ^ (D[1] & 0x00000032) ^ (c[0] & 0x0000032c);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xb905ad89) ^ (D[1] & 0x00000915) ^ (c[0] & 0x0000915b);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x720b5b12) ^ (D[1] & 0x0000022b) ^ (c[0] & 0x000022b7);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xdbc202fb) ^ (D[1] & 0x00000d26) ^ (c[0] & 0x0000d26d);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x8850b129) ^ (D[1] & 0x0000033d) ^ (c[0] & 0x000033d8);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x2f75d68d) ^ (D[1] & 0x00000f0b) ^ (c[0] & 0x0000f0b2);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x5eebad1a) ^ (D[1] & 0x00000e16) ^ (c[0] & 0x0000e165);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x8203eeeb) ^ (D[1] & 0x0000055c) ^ (c[0] & 0x000055c8);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x0407ddd6) ^ (D[1] & 0x00000ab9) ^ (c[0] & 0x0000ab90);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x080fbbac) ^ (D[1] & 0x00000572) ^ (c[0] & 0x00005720);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x101f7758) ^ (D[1] & 0x00000ae4) ^ (c[0] & 0x0000ae41);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x1fea5a6f) ^ (D[1] & 0x00000cb8) ^ (c[0] & 0x0000cb81);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16c_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 5 7 8 9 11 15 16)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16c_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x3fd4b4df) ^ (D[1] & 0x024b9970) ^ (c[0] & 0x000024b9);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x407ddd61) ^ (D[1] & 0x06dcab90) ^ (c[0] & 0x00006dca);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xbf2f0e1d) ^ (D[1] & 0x0ff2ce50) ^ (c[0] & 0x0000ff2c);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x7e5e1c3a) ^ (D[1] & 0x0fe59ca1) ^ (c[0] & 0x0000fe59);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xc3688cab) ^ (D[1] & 0x0d80a032) ^ (c[0] & 0x0000d80a);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xb905ad89) ^ (D[1] & 0x094ad915) ^ (c[0] & 0x000094ad);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x720b5b12) ^ (D[1] & 0x0295b22b) ^ (c[0] & 0x0000295b);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xdbc202fb) ^ (D[1] & 0x0760fd26) ^ (c[0] & 0x0000760f);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x8850b129) ^ (D[1] & 0x0c8a633d) ^ (c[0] & 0x0000c8a6);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x2f75d68d) ^ (D[1] & 0x0b5f5f0b) ^ (c[0] & 0x0000b5f5);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x5eebad1a) ^ (D[1] & 0x06bebe16) ^ (c[0] & 0x00006beb);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x8203eeeb) ^ (D[1] & 0x0f36e55c) ^ (c[0] & 0x0000f36e);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x0407ddd6) ^ (D[1] & 0x0e6dcab9) ^ (c[0] & 0x0000e6dc);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x080fbbac) ^ (D[1] & 0x0cdb9572) ^ (c[0] & 0x0000cdb9);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x101f7758) ^ (D[1] & 0x09b72ae4) ^ (c[0] & 0x00009b72);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x1fea5a6f) ^ (D[1] & 0x0125ccb8) ^ (c[0] & 0x0000125c);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16d_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 2 5 6 8 10 11 12 13 16)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16d_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf81e8e39) ^ (D[1] & 0x00000008) ^ (c[0] & 0x00008f81);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0xf03d1c72) ^ (D[1] & 0x00000001) ^ (c[0] & 0x00001f03);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x1864b6dd) ^ (D[1] & 0x0000000b) ^ (c[0] & 0x0000b186);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x30c96dba) ^ (D[1] & 0x00000006) ^ (c[0] & 0x0000630c);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0x6192db74) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x0000c619);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x3b3b38d1) ^ (c[0] & 0x000003b3);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x8e68ff9b) ^ (D[1] & 0x00000008) ^ (c[0] & 0x000088e6);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x1cd1ff36) ^ (D[1] & 0x00000001) ^ (c[0] & 0x000011cd);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xc1bd7055) ^ (D[1] & 0x0000000a) ^ (c[0] & 0x0000ac1b);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x837ae0aa) ^ (D[1] & 0x00000005) ^ (c[0] & 0x00005837);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xfeeb4f6d) ^ (D[1] & 0x00000003) ^ (c[0] & 0x00003fee);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x05c810e3) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x0000f05c);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xf38eafff) ^ (D[1] & 0x00000006) ^ (c[0] & 0x00006f38);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x1f03d1c7) ^ (D[1] & 0x00000005) ^ (c[0] & 0x000051f0);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x3e07a38e) ^ (D[1] & 0x0000000a) ^ (c[0] & 0x0000a3e0);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x7c0f471c) ^ (D[1] & 0x00000004) ^ (c[0] & 0x000047c0);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_16d_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 2 5 6 8 10 11 12 13 16)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16d_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf81e8e39) ^ (D[1] & 0x00000628) ^ (c[0] & 0x0000628f);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0xf03d1c72) ^ (D[1] & 0x00000c51) ^ (c[0] & 0x0000c51f);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x1864b6dd) ^ (D[1] & 0x00000e8b) ^ (c[0] & 0x0000e8b1);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x30c96dba) ^ (D[1] & 0x00000d16) ^ (c[0] & 0x0000d163);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0x6192db74) ^ (D[1] & 0x00000a2c) ^ (c[0] & 0x0000a2c6);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x3b3b38d1) ^ (D[1] & 0x00000270) ^ (c[0] & 0x00002703);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x8e68ff9b) ^ (D[1] & 0x000002c8) ^ (c[0] & 0x00002c88);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x1cd1ff36) ^ (D[1] & 0x00000591) ^ (c[0] & 0x00005911);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xc1bd7055) ^ (D[1] & 0x00000d0a) ^ (c[0] & 0x0000d0ac);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x837ae0aa) ^ (D[1] & 0x00000a15) ^ (c[0] & 0x0000a158);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xfeeb4f6d) ^ (D[1] & 0x00000203) ^ (c[0] & 0x0000203f);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x05c810e3) ^ (D[1] & 0x0000022f) ^ (c[0] & 0x000022f0);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xf38eafff) ^ (D[1] & 0x00000276) ^ (c[0] & 0x0000276f);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x1f03d1c7) ^ (D[1] & 0x000002c5) ^ (c[0] & 0x00002c51);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x3e07a38e) ^ (D[1] & 0x0000058a) ^ (c[0] & 0x000058a3);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x7c0f471c) ^ (D[1] & 0x00000b14) ^ (c[0] & 0x0000b147);
    _PARITY(value);
    _H(15,value);

}


/**
* @internal prvCpssDxChBrgFdbHashCrc_16d_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 2 5 6 8 10 11 12 13 16)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_16d_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf81e8e39) ^ (D[1] & 0x024f9628) ^ (c[0] & 0x000024f9);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0xf03d1c72) ^ (D[1] & 0x049f2c51) ^ (c[0] & 0x000049f2);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x1864b6dd) ^ (D[1] & 0x0b71ce8b) ^ (c[0] & 0x0000b71c);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x30c96dba) ^ (D[1] & 0x06e39d16) ^ (c[0] & 0x00006e39);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0x6192db74) ^ (D[1] & 0x0dc73a2c) ^ (c[0] & 0x0000dc73);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x3b3b38d1) ^ (D[1] & 0x09c1e270) ^ (c[0] & 0x00009c1e);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x8e68ff9b) ^ (D[1] & 0x01cc52c8) ^ (c[0] & 0x00001cc5);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x1cd1ff36) ^ (D[1] & 0x0398a591) ^ (c[0] & 0x0000398a);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xc1bd7055) ^ (D[1] & 0x057edd0a) ^ (c[0] & 0x000057ed);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x837ae0aa) ^ (D[1] & 0x0afdba15) ^ (c[0] & 0x0000afdb);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xfeeb4f6d) ^ (D[1] & 0x07b4e203) ^ (c[0] & 0x00007b4e);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x05c810e3) ^ (D[1] & 0x0d26522f) ^ (c[0] & 0x0000d265);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xf38eafff) ^ (D[1] & 0x08033276) ^ (c[0] & 0x00008033);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x1f03d1c7) ^ (D[1] & 0x0249f2c5) ^ (c[0] & 0x0000249f);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x3e07a38e) ^ (D[1] & 0x0493e58a) ^ (c[0] & 0x0000493e);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x7c0f471c) ^ (D[1] & 0x0927cb14) ^ (c[0] & 0x0000927c);
    _PARITY(value);
    _H(15,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32a_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 5 7 8 10 11 12 16 22 23 26 32)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf7011641) ^ (D[1] & 0x00000005) ^ (c[0] & 0x5f701164);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x19033ac3) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xe19033ac);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xc50763c7) ^ (D[1] & 0x00000009) ^ (c[0] & 0x9c50763c);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x8a0ec78e) ^ (D[1] & 0x00000003) ^ (c[0] & 0x38a0ec78);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xe31c995d) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2e31c995);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x313824fb) ^ (c[0] & 0x0313824f);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x627049f6) ^ (c[0] & 0x0627049f);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x33e185ad) ^ (D[1] & 0x00000005) ^ (c[0] & 0x533e185a);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x90c21d1b) ^ (D[1] & 0x0000000f) ^ (c[0] & 0xf90c21d1);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x21843a36) ^ (D[1] & 0x0000000f) ^ (c[0] & 0xf21843a3);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xb409622d) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xbb409622);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x9f13d21b) ^ (D[1] & 0x00000002) ^ (c[0] & 0x29f13d21);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xc926b277) ^ (c[0] & 0x0c926b27);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x924d64ee) ^ (D[1] & 0x00000001) ^ (c[0] & 0x1924d64e);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x249ac9dc) ^ (D[1] & 0x00000003) ^ (c[0] & 0x3249ac9d);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x493593b8) ^ (D[1] & 0x00000006) ^ (c[0] & 0x6493593b);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x656a3131) ^ (D[1] & 0x00000009) ^ (c[0] & 0x9656a313);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xcad46262) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2cad4626);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x95a8c4c4) ^ (D[1] & 0x00000005) ^ (c[0] & 0x595a8c4c);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2b518988) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xb2b51898);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x56a31310) ^ (D[1] & 0x00000006) ^ (c[0] & 0x656a3131);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xad462620) ^ (D[1] & 0x0000000c) ^ (c[0] & 0xcad46262);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xad8d5a01) ^ (D[1] & 0x0000000c) ^ (c[0] & 0xcad8d5a0);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xac1ba243) ^ (D[1] & 0x0000000c) ^ (c[0] & 0xcac1ba24);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x58374486) ^ (D[1] & 0x00000009) ^ (c[0] & 0x95837448);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb06e890c) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2b06e890);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x97dc0459) ^ (c[0] & 0x097dc045);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x2fb808b2) ^ (D[1] & 0x00000001) ^ (c[0] & 0x12fb808b);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x5f701164) ^ (D[1] & 0x00000002) ^ (c[0] & 0x25f70116);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xbee022c8) ^ (D[1] & 0x00000004) ^ (c[0] & 0x4bee022c);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x7dc04590) ^ (D[1] & 0x00000009) ^ (c[0] & 0x97dc0459);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0xfb808b20) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2fb808b2);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32a_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 5 7 8 10 11 12 16 22 23 26 32)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf7011641) ^ (D[1] & 0x00000025) ^ (c[0] & 0x025f7011);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x19033ac3) ^ (D[1] & 0x0000006e) ^ (c[0] & 0x06e19033);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xc50763c7) ^ (D[1] & 0x000000f9) ^ (c[0] & 0x0f9c5076);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x8a0ec78e) ^ (D[1] & 0x000001f3) ^ (c[0] & 0x1f38a0ec);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xe31c995d) ^ (D[1] & 0x000003c2) ^ (c[0] & 0x3c2e31c9);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x313824fb) ^ (D[1] & 0x000007a0) ^ (c[0] & 0x7a031382);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x627049f6) ^ (D[1] & 0x00000f40) ^ (c[0] & 0xf4062704);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x33e185ad) ^ (D[1] & 0x00000ea5) ^ (c[0] & 0xea533e18);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x90c21d1b) ^ (D[1] & 0x00000d6f) ^ (c[0] & 0xd6f90c21);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x21843a36) ^ (D[1] & 0x00000adf) ^ (c[0] & 0xadf21843);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xb409622d) ^ (D[1] & 0x0000059b) ^ (c[0] & 0x59bb4096);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x9f13d21b) ^ (D[1] & 0x00000b12) ^ (c[0] & 0xb129f13d);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xc926b277) ^ (D[1] & 0x00000600) ^ (c[0] & 0x600c926b);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x924d64ee) ^ (D[1] & 0x00000c01) ^ (c[0] & 0xc01924d6);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x249ac9dc) ^ (D[1] & 0x00000803) ^ (c[0] & 0x803249ac);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x493593b8) ^ (D[1] & 0x00000006) ^ (c[0] & 0x00649359);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x656a3131) ^ (D[1] & 0x00000029) ^ (c[0] & 0x029656a3);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xcad46262) ^ (D[1] & 0x00000052) ^ (c[0] & 0x052cad46);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x95a8c4c4) ^ (D[1] & 0x000000a5) ^ (c[0] & 0x0a595a8c);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2b518988) ^ (D[1] & 0x0000014b) ^ (c[0] & 0x14b2b518);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x56a31310) ^ (D[1] & 0x00000296) ^ (c[0] & 0x29656a31);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xad462620) ^ (D[1] & 0x0000052c) ^ (c[0] & 0x52cad462);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xad8d5a01) ^ (D[1] & 0x00000a7c) ^ (c[0] & 0xa7cad8d5);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xac1ba243) ^ (D[1] & 0x000004dc) ^ (c[0] & 0x4dcac1ba);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x58374486) ^ (D[1] & 0x000009b9) ^ (c[0] & 0x9b958374);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb06e890c) ^ (D[1] & 0x00000372) ^ (c[0] & 0x372b06e8);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x97dc0459) ^ (D[1] & 0x000006c0) ^ (c[0] & 0x6c097dc0);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x2fb808b2) ^ (D[1] & 0x00000d81) ^ (c[0] & 0xd812fb80);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x5f701164) ^ (D[1] & 0x00000b02) ^ (c[0] & 0xb025f701);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xbee022c8) ^ (D[1] & 0x00000604) ^ (c[0] & 0x604bee02);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x7dc04590) ^ (D[1] & 0x00000c09) ^ (c[0] & 0xc097dc04);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0xfb808b20) ^ (D[1] & 0x00000812) ^ (c[0] & 0x812fb808);
    _PARITY(value);
    _H(31,value);

}


/**
* @internal prvCpssDxChBrgFdbHashCrc_32a_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 5 7 8 10 11 12 16 22 23 26 32)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf7011641) ^ (D[1] & 0x04e5b025) ^ (c[0] & 0x4e5b025f);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x19033ac3) ^ (D[1] & 0x0d2ed06e) ^ (c[0] & 0xd2ed06e1);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xc50763c7) ^ (D[1] & 0x0eb810f9) ^ (c[0] & 0xeb810f9c);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x8a0ec78e) ^ (D[1] & 0x0d7021f3) ^ (c[0] & 0xd7021f38);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xe31c995d) ^ (D[1] & 0x0e05f3c2) ^ (c[0] & 0xe05f3c2e);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x313824fb) ^ (D[1] & 0x08ee57a0) ^ (c[0] & 0x8ee57a03);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x627049f6) ^ (D[1] & 0x01dcaf40) ^ (c[0] & 0x1dcaf406);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x33e185ad) ^ (D[1] & 0x075ceea5) ^ (c[0] & 0x75ceea53);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x90c21d1b) ^ (D[1] & 0x0a5c6d6f) ^ (c[0] & 0xa5c6d6f9);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x21843a36) ^ (D[1] & 0x04b8dadf) ^ (c[0] & 0x4b8dadf2);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xb409622d) ^ (D[1] & 0x0d94059b) ^ (c[0] & 0xd94059bb);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x9f13d21b) ^ (D[1] & 0x0fcdbb12) ^ (c[0] & 0xfcdbb129);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xc926b277) ^ (D[1] & 0x0b7ec600) ^ (c[0] & 0xb7ec600c);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x924d64ee) ^ (D[1] & 0x06fd8c01) ^ (c[0] & 0x6fd8c019);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x249ac9dc) ^ (D[1] & 0x0dfb1803) ^ (c[0] & 0xdfb18032);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x493593b8) ^ (D[1] & 0x0bf63006) ^ (c[0] & 0xbf630064);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x656a3131) ^ (D[1] & 0x0309d029) ^ (c[0] & 0x309d0296);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xcad46262) ^ (D[1] & 0x0613a052) ^ (c[0] & 0x613a052c);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x95a8c4c4) ^ (D[1] & 0x0c2740a5) ^ (c[0] & 0xc2740a59);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2b518988) ^ (D[1] & 0x084e814b) ^ (c[0] & 0x84e814b2);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x56a31310) ^ (D[1] & 0x009d0296) ^ (c[0] & 0x09d02965);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xad462620) ^ (D[1] & 0x013a052c) ^ (c[0] & 0x13a052ca);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xad8d5a01) ^ (D[1] & 0x0691ba7c) ^ (c[0] & 0x691ba7ca);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xac1ba243) ^ (D[1] & 0x09c6c4dc) ^ (c[0] & 0x9c6c4dca);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x58374486) ^ (D[1] & 0x038d89b9) ^ (c[0] & 0x38d89b95);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb06e890c) ^ (D[1] & 0x071b1372) ^ (c[0] & 0x71b1372b);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x97dc0459) ^ (D[1] & 0x0ad396c0) ^ (c[0] & 0xad396c09);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x2fb808b2) ^ (D[1] & 0x05a72d81) ^ (c[0] & 0x5a72d812);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x5f701164) ^ (D[1] & 0x0b4e5b02) ^ (c[0] & 0xb4e5b025);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xbee022c8) ^ (D[1] & 0x069cb604) ^ (c[0] & 0x69cb604b);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x7dc04590) ^ (D[1] & 0x0d396c09) ^ (c[0] & 0xd396c097);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0xfb808b20) ^ (D[1] & 0x0a72d812) ^ (c[0] & 0xa72d812f);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32b_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 6 8 9 10 11 13 14 18 19 20 22 23 25 26 27 28 32)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32b_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xdea713f1) ^ (D[1] & 0x00000008) ^ (c[0] & 0x8dea713f);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0xbd4e27e2) ^ (D[1] & 0x00000001) ^ (c[0] & 0x1bd4e27e);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x7a9c4fc4) ^ (D[1] & 0x00000003) ^ (c[0] & 0x37a9c4fc);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xf5389f88) ^ (D[1] & 0x00000006) ^ (c[0] & 0x6f5389f8);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xea713f10) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xdea713f1);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xd4e27e20) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xbd4e27e2);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x7763efb1) ^ (D[1] & 0x0000000f) ^ (c[0] & 0xf7763efb);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xeec7df62) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xeeec7df6);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x0328ad35) ^ (D[1] & 0x00000005) ^ (c[0] & 0x50328ad3);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0xd8f6499b) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2d8f6499);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x6f4b80c7) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xd6f4b80c);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x0030127f) ^ (D[1] & 0x00000002) ^ (c[0] & 0x20030127);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x006024fe) ^ (D[1] & 0x00000004) ^ (c[0] & 0x4006024f);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xde675a0d) ^ (c[0] & 0x0de675a0);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x6269a7eb) ^ (D[1] & 0x00000009) ^ (c[0] & 0x96269a7e);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xc4d34fd6) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2c4d34fd);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x89a69fac) ^ (D[1] & 0x00000005) ^ (c[0] & 0x589a69fa);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0x134d3f58) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xb134d3f5);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0xf83d6d41) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xef83d6d4);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2eddc973) ^ (D[1] & 0x00000005) ^ (c[0] & 0x52eddc97);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x831c8117) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2831c811);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0x0639022e) ^ (D[1] & 0x00000005) ^ (c[0] & 0x50639022);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xd2d517ad) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2d2d517a);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x7b0d3cab) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xd7b0d3ca);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0xf61a7956) ^ (D[1] & 0x0000000a) ^ (c[0] & 0xaf61a795);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x3293e15d) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xd3293e15);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0xbb80d14b) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2bb80d14);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xa9a6b167) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xda9a6b16);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x8dea713f) ^ (D[1] & 0x00000003) ^ (c[0] & 0x38dea713);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x1bd4e27e) ^ (D[1] & 0x00000007) ^ (c[0] & 0x71bd4e27);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x37a9c4fc) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xe37a9c4f);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x6f5389f8) ^ (D[1] & 0x0000000c) ^ (c[0] & 0xc6f5389f);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32b_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 6 8 9 10 11 13 14 18 19 20 22 23 25 26 27 28 32)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32b_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xdea713f1) ^ (D[1] & 0x00000c38) ^ (c[0] & 0xc38dea71);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0xbd4e27e2) ^ (D[1] & 0x00000871) ^ (c[0] & 0x871bd4e2);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x7a9c4fc4) ^ (D[1] & 0x000000e3) ^ (c[0] & 0x0e37a9c4);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xf5389f88) ^ (D[1] & 0x000001c6) ^ (c[0] & 0x1c6f5389);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xea713f10) ^ (D[1] & 0x0000038d) ^ (c[0] & 0x38dea713);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xd4e27e20) ^ (D[1] & 0x0000071b) ^ (c[0] & 0x71bd4e27);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x7763efb1) ^ (D[1] & 0x0000020f) ^ (c[0] & 0x20f7763e);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xeec7df62) ^ (D[1] & 0x0000041e) ^ (c[0] & 0x41eeec7d);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x0328ad35) ^ (D[1] & 0x00000405) ^ (c[0] & 0x4050328a);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0xd8f6499b) ^ (D[1] & 0x00000432) ^ (c[0] & 0x432d8f64);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x6f4b80c7) ^ (D[1] & 0x0000045d) ^ (c[0] & 0x45d6f4b8);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x0030127f) ^ (D[1] & 0x00000482) ^ (c[0] & 0x48200301);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x006024fe) ^ (D[1] & 0x00000904) ^ (c[0] & 0x90400602);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xde675a0d) ^ (D[1] & 0x00000e30) ^ (c[0] & 0xe30de675);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x6269a7eb) ^ (D[1] & 0x00000059) ^ (c[0] & 0x0596269a);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xc4d34fd6) ^ (D[1] & 0x000000b2) ^ (c[0] & 0x0b2c4d34);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x89a69fac) ^ (D[1] & 0x00000165) ^ (c[0] & 0x16589a69);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0x134d3f58) ^ (D[1] & 0x000002cb) ^ (c[0] & 0x2cb134d3);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0xf83d6d41) ^ (D[1] & 0x000009ae) ^ (c[0] & 0x9aef83d6);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2eddc973) ^ (D[1] & 0x00000f65) ^ (c[0] & 0xf652eddc);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x831c8117) ^ (D[1] & 0x000002f2) ^ (c[0] & 0x2f2831c8);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0x0639022e) ^ (D[1] & 0x000005e5) ^ (c[0] & 0x5e506390);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xd2d517ad) ^ (D[1] & 0x000007f2) ^ (c[0] & 0x7f2d2d51);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x7b0d3cab) ^ (D[1] & 0x000003dd) ^ (c[0] & 0x3dd7b0d3);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0xf61a7956) ^ (D[1] & 0x000007ba) ^ (c[0] & 0x7baf61a7);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x3293e15d) ^ (D[1] & 0x0000034d) ^ (c[0] & 0x34d3293e);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0xbb80d14b) ^ (D[1] & 0x00000aa2) ^ (c[0] & 0xaa2bb80d);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xa9a6b167) ^ (D[1] & 0x0000097d) ^ (c[0] & 0x97da9a6b);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x8dea713f) ^ (D[1] & 0x00000ec3) ^ (c[0] & 0xec38dea7);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x1bd4e27e) ^ (D[1] & 0x00000d87) ^ (c[0] & 0xd871bd4e);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x37a9c4fc) ^ (D[1] & 0x00000b0e) ^ (c[0] & 0xb0e37a9c);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x6f5389f8) ^ (D[1] & 0x0000061c) ^ (c[0] & 0x61c6f538);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32b_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 6 8 9 10 11 13 14 18 19 20 22 23 25 26 27 28 32)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32b_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xdea713f1) ^ (D[1] & 0x0869ec38) ^ (c[0] & 0x869ec38d);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0xbd4e27e2) ^ (D[1] & 0x00d3d871) ^ (c[0] & 0x0d3d871b);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x7a9c4fc4) ^ (D[1] & 0x01a7b0e3) ^ (c[0] & 0x1a7b0e37);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xf5389f88) ^ (D[1] & 0x034f61c6) ^ (c[0] & 0x34f61c6f);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xea713f10) ^ (D[1] & 0x069ec38d) ^ (c[0] & 0x69ec38de);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xd4e27e20) ^ (D[1] & 0x0d3d871b) ^ (c[0] & 0xd3d871bd);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x7763efb1) ^ (D[1] & 0x0212e20f) ^ (c[0] & 0x212e20f7);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xeec7df62) ^ (D[1] & 0x0425c41e) ^ (c[0] & 0x425c41ee);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x0328ad35) ^ (D[1] & 0x00226405) ^ (c[0] & 0x02264050);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0xd8f6499b) ^ (D[1] & 0x082d2432) ^ (c[0] & 0x82d2432d);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x6f4b80c7) ^ (D[1] & 0x0833a45d) ^ (c[0] & 0x833a45d6);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x0030127f) ^ (D[1] & 0x080ea482) ^ (c[0] & 0x80ea4820);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x006024fe) ^ (D[1] & 0x001d4904) ^ (c[0] & 0x01d49040);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xde675a0d) ^ (D[1] & 0x08537e30) ^ (c[0] & 0x8537e30d);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x6269a7eb) ^ (D[1] & 0x08cf1059) ^ (c[0] & 0x8cf10596);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xc4d34fd6) ^ (D[1] & 0x019e20b2) ^ (c[0] & 0x19e20b2c);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x89a69fac) ^ (D[1] & 0x033c4165) ^ (c[0] & 0x33c41658);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0x134d3f58) ^ (D[1] & 0x067882cb) ^ (c[0] & 0x67882cb1);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0xf83d6d41) ^ (D[1] & 0x0498e9ae) ^ (c[0] & 0x498e9aef);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2eddc973) ^ (D[1] & 0x01583f65) ^ (c[0] & 0x1583f652);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x831c8117) ^ (D[1] & 0x0ad992f2) ^ (c[0] & 0xad992f28);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0x0639022e) ^ (D[1] & 0x05b325e5) ^ (c[0] & 0x5b325e50);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xd2d517ad) ^ (D[1] & 0x030fa7f2) ^ (c[0] & 0x30fa7f2d);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x7b0d3cab) ^ (D[1] & 0x0e76a3dd) ^ (c[0] & 0xe76a3dd7);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0xf61a7956) ^ (D[1] & 0x0ced47ba) ^ (c[0] & 0xced47baf);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x3293e15d) ^ (D[1] & 0x01b3634d) ^ (c[0] & 0x1b3634d3);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0xbb80d14b) ^ (D[1] & 0x0b0f2aa2) ^ (c[0] & 0xb0f2aa2b);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xa9a6b167) ^ (D[1] & 0x0e77b97d) ^ (c[0] & 0xe77b97da);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x8dea713f) ^ (D[1] & 0x04869ec3) ^ (c[0] & 0x4869ec38);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x1bd4e27e) ^ (D[1] & 0x090d3d87) ^ (c[0] & 0x90d3d871);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x37a9c4fc) ^ (D[1] & 0x021a7b0e) ^ (c[0] & 0x21a7b0e3);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x6f5389f8) ^ (D[1] & 0x0434f61c) ^ (c[0] & 0x434f61c6);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32k_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 6 7 10 11 15 16 17 19 20 26 28 29 30 32)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32k_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x17d232cd) ^ (D[1] & 0x0000000c) ^ (c[0] & 0xc17d232c);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x38765757) ^ (D[1] & 0x00000004) ^ (c[0] & 0x43876575);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x673e9c63) ^ (D[1] & 0x00000004) ^ (c[0] & 0x4673e9c6);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xce7d38c6) ^ (D[1] & 0x00000008) ^ (c[0] & 0x8ce7d38c);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0x8b284341) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xd8b28434);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x16508682) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xb1650868);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x3b733fc9) ^ (D[1] & 0x0000000a) ^ (c[0] & 0xa3b733fc);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x61344d5f) ^ (D[1] & 0x00000008) ^ (c[0] & 0x861344d5);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xc2689abe) ^ (c[0] & 0x0c2689ab);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x84d1357c) ^ (D[1] & 0x00000001) ^ (c[0] & 0x184d1357);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x1e705835) ^ (D[1] & 0x0000000f) ^ (c[0] & 0xf1e70583);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x2b3282a7) ^ (D[1] & 0x00000002) ^ (c[0] & 0x22b3282a);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x5665054e) ^ (D[1] & 0x00000004) ^ (c[0] & 0x45665054);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xacca0a9c) ^ (D[1] & 0x00000008) ^ (c[0] & 0x8acca0a9);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x59941538) ^ (D[1] & 0x00000001) ^ (c[0] & 0x15994153);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xa4fa18bd) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xea4fa18b);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x5e2603b7) ^ (D[1] & 0x00000001) ^ (c[0] & 0x15e2603b);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xab9e35a3) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xeab9e35a);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x573c6b46) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xd573c6b4);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0xb9aae441) ^ (D[1] & 0x00000006) ^ (c[0] & 0x6b9aae44);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x6487fa4f) ^ (D[1] & 0x00000001) ^ (c[0] & 0x16487fa4);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xc90ff49e) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2c90ff49);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0x921fe93c) ^ (D[1] & 0x00000005) ^ (c[0] & 0x5921fe93);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x243fd278) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xb243fd27);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x487fa4f0) ^ (D[1] & 0x00000006) ^ (c[0] & 0x6487fa4f);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x90ff49e0) ^ (D[1] & 0x0000000c) ^ (c[0] & 0xc90ff49e);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x362ca10d) ^ (D[1] & 0x00000005) ^ (c[0] & 0x5362ca10);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x6c59421a) ^ (D[1] & 0x0000000a) ^ (c[0] & 0xa6c59421);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xcf60b6f9) ^ (D[1] & 0x00000008) ^ (c[0] & 0x8cf60b6f);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x89135f3f) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xd89135f3);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x05f48cb3) ^ (D[1] & 0x00000007) ^ (c[0] & 0x705f48cb);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x0be91966) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xe0be9196);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32k_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 6 7 10 11 15 16 17 19 20 26 28 29 30 32)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32k_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x17d232cd) ^ (D[1] & 0x0000001c) ^ (c[0] & 0x01c17d23);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x38765757) ^ (D[1] & 0x00000024) ^ (c[0] & 0x02438765);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x673e9c63) ^ (D[1] & 0x00000054) ^ (c[0] & 0x054673e9);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xce7d38c6) ^ (D[1] & 0x000000a8) ^ (c[0] & 0x0a8ce7d3);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0x8b284341) ^ (D[1] & 0x0000014d) ^ (c[0] & 0x14d8b284);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x16508682) ^ (D[1] & 0x0000029b) ^ (c[0] & 0x29b16508);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x3b733fc9) ^ (D[1] & 0x0000052a) ^ (c[0] & 0x52a3b733);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x61344d5f) ^ (D[1] & 0x00000a48) ^ (c[0] & 0xa4861344);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xc2689abe) ^ (D[1] & 0x00000490) ^ (c[0] & 0x490c2689);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x84d1357c) ^ (D[1] & 0x00000921) ^ (c[0] & 0x92184d13);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x1e705835) ^ (D[1] & 0x0000025f) ^ (c[0] & 0x25f1e705);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x2b3282a7) ^ (D[1] & 0x000004a2) ^ (c[0] & 0x4a22b328);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x5665054e) ^ (D[1] & 0x00000944) ^ (c[0] & 0x94456650);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xacca0a9c) ^ (D[1] & 0x00000288) ^ (c[0] & 0x288acca0);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x59941538) ^ (D[1] & 0x00000511) ^ (c[0] & 0x51159941);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xa4fa18bd) ^ (D[1] & 0x00000a3e) ^ (c[0] & 0xa3ea4fa1);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x5e2603b7) ^ (D[1] & 0x00000461) ^ (c[0] & 0x4615e260);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xab9e35a3) ^ (D[1] & 0x000008de) ^ (c[0] & 0x8deab9e3);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x573c6b46) ^ (D[1] & 0x000001bd) ^ (c[0] & 0x1bd573c6);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0xb9aae441) ^ (D[1] & 0x00000366) ^ (c[0] & 0x366b9aae);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x6487fa4f) ^ (D[1] & 0x000006d1) ^ (c[0] & 0x6d16487f);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xc90ff49e) ^ (D[1] & 0x00000da2) ^ (c[0] & 0xda2c90ff);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0x921fe93c) ^ (D[1] & 0x00000b45) ^ (c[0] & 0xb45921fe);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x243fd278) ^ (D[1] & 0x0000068b) ^ (c[0] & 0x68b243fd);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x487fa4f0) ^ (D[1] & 0x00000d16) ^ (c[0] & 0xd16487fa);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x90ff49e0) ^ (D[1] & 0x00000a2c) ^ (c[0] & 0xa2c90ff4);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x362ca10d) ^ (D[1] & 0x00000445) ^ (c[0] & 0x445362ca);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x6c59421a) ^ (D[1] & 0x0000088a) ^ (c[0] & 0x88a6c594);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xcf60b6f9) ^ (D[1] & 0x00000108) ^ (c[0] & 0x108cf60b);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x89135f3f) ^ (D[1] & 0x0000020d) ^ (c[0] & 0x20d89135);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x05f48cb3) ^ (D[1] & 0x00000407) ^ (c[0] & 0x40705f48);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x0be91966) ^ (D[1] & 0x0000080e) ^ (c[0] & 0x80e0be91);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32k_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 2 4 6 7 10 11 15 16 17 19 20 26 28 29 30 32)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32k_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x17d232cd) ^ (D[1] & 0x025dd01c) ^ (c[0] & 0x25dd01c1);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x38765757) ^ (D[1] & 0x06e67024) ^ (c[0] & 0x6e670243);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x673e9c63) ^ (D[1] & 0x0f913054) ^ (c[0] & 0xf9130546);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xce7d38c6) ^ (D[1] & 0x0f2260a8) ^ (c[0] & 0xf2260a8c);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0x8b284341) ^ (D[1] & 0x0c19114d) ^ (c[0] & 0xc19114d8);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x16508682) ^ (D[1] & 0x0832229b) ^ (c[0] & 0x832229b1);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x3b733fc9) ^ (D[1] & 0x0239952a) ^ (c[0] & 0x239952a3);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x61344d5f) ^ (D[1] & 0x062efa48) ^ (c[0] & 0x62efa486);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xc2689abe) ^ (D[1] & 0x0c5df490) ^ (c[0] & 0xc5df490c);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x84d1357c) ^ (D[1] & 0x08bbe921) ^ (c[0] & 0x8bbe9218);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x1e705835) ^ (D[1] & 0x032a025f) ^ (c[0] & 0x32a025f1);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x2b3282a7) ^ (D[1] & 0x0409d4a2) ^ (c[0] & 0x409d4a22);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x5665054e) ^ (D[1] & 0x0813a944) ^ (c[0] & 0x813a9445);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xacca0a9c) ^ (D[1] & 0x00275288) ^ (c[0] & 0x0275288a);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0x59941538) ^ (D[1] & 0x004ea511) ^ (c[0] & 0x04ea5115);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xa4fa18bd) ^ (D[1] & 0x02c09a3e) ^ (c[0] & 0x2c09a3ea);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x5e2603b7) ^ (D[1] & 0x07dce461) ^ (c[0] & 0x7dce4615);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xab9e35a3) ^ (D[1] & 0x0de418de) ^ (c[0] & 0xde418dea);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x573c6b46) ^ (D[1] & 0x0bc831bd) ^ (c[0] & 0xbc831bd5);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0xb9aae441) ^ (D[1] & 0x05cdb366) ^ (c[0] & 0x5cdb366b);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x6487fa4f) ^ (D[1] & 0x09c6b6d1) ^ (c[0] & 0x9c6b6d16);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xc90ff49e) ^ (D[1] & 0x038d6da2) ^ (c[0] & 0x38d6da2c);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0x921fe93c) ^ (D[1] & 0x071adb45) ^ (c[0] & 0x71adb459);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x243fd278) ^ (D[1] & 0x0e35b68b) ^ (c[0] & 0xe35b68b2);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x487fa4f0) ^ (D[1] & 0x0c6b6d16) ^ (c[0] & 0xc6b6d164);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x90ff49e0) ^ (D[1] & 0x08d6da2c) ^ (c[0] & 0x8d6da2c9);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x362ca10d) ^ (D[1] & 0x03f06445) ^ (c[0] & 0x3f064453);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x6c59421a) ^ (D[1] & 0x07e0c88a) ^ (c[0] & 0x7e0c88a6);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xcf60b6f9) ^ (D[1] & 0x0d9c4108) ^ (c[0] & 0xd9c4108c);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x89135f3f) ^ (D[1] & 0x0965520d) ^ (c[0] & 0x965520d8);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x05f48cb3) ^ (D[1] & 0x00977407) ^ (c[0] & 0x09774070);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x0be91966) ^ (D[1] & 0x012ee80e) ^ (c[0] & 0x12ee80e0);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32q_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 3 5 7 8 14 16 22 24 31 32)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32q_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x8dfdfeff) ^ (D[1] & 0x00000006) ^ (c[0] & 0x68dfdfef);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x96060301) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xb9606030);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x2c0c0602) ^ (D[1] & 0x00000007) ^ (c[0] & 0x72c0c060);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xd5e5f2fb) ^ (D[1] & 0x00000008) ^ (c[0] & 0x8d5e5f2f);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xabcbe5f6) ^ (D[1] & 0x00000001) ^ (c[0] & 0x1abcbe5f);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xda6a3513) ^ (D[1] & 0x00000005) ^ (c[0] & 0x5da6a351);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0xb4d46a26) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xbb4d46a2);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xe4552ab3) ^ (D[1] & 0x00000001) ^ (c[0] & 0x1e4552ab);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x4557ab99) ^ (D[1] & 0x00000005) ^ (c[0] & 0x54557ab9);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x8aaf5732) ^ (D[1] & 0x0000000a) ^ (c[0] & 0xa8aaf573);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x155eae64) ^ (D[1] & 0x00000005) ^ (c[0] & 0x5155eae6);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x2abd5cc8) ^ (D[1] & 0x0000000a) ^ (c[0] & 0xa2abd5cc);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x557ab990) ^ (D[1] & 0x00000004) ^ (c[0] & 0x4557ab99);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xaaf57320) ^ (D[1] & 0x00000008) ^ (c[0] & 0x8aaf5732);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xd81718bf) ^ (D[1] & 0x00000007) ^ (c[0] & 0x7d81718b);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xb02e317e) ^ (D[1] & 0x0000000f) ^ (c[0] & 0xfb02e317);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0xeda19c03) ^ (D[1] & 0x00000009) ^ (c[0] & 0x9eda19c0);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xdb433806) ^ (D[1] & 0x00000003) ^ (c[0] & 0x3db43380);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0xb686700c) ^ (D[1] & 0x00000007) ^ (c[0] & 0x7b686700);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x6d0ce018) ^ (D[1] & 0x0000000f) ^ (c[0] & 0xf6d0ce01);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0xda19c030) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xeda19c03);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xb4338060) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xdb433806);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xe59afe3f) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xde59afe3);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xcb35fc7e) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xbcb35fc7);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x1b960603) ^ (D[1] & 0x00000001) ^ (c[0] & 0x11b96060);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x372c0c06) ^ (D[1] & 0x00000002) ^ (c[0] & 0x2372c0c0);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x6e58180c) ^ (D[1] & 0x00000004) ^ (c[0] & 0x46e58180);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xdcb03018) ^ (D[1] & 0x00000008) ^ (c[0] & 0x8dcb0301);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xb9606030) ^ (D[1] & 0x00000001) ^ (c[0] & 0x1b960603);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x72c0c060) ^ (D[1] & 0x00000003) ^ (c[0] & 0x372c0c06);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0xe58180c0) ^ (D[1] & 0x00000006) ^ (c[0] & 0x6e58180c);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x46feff7f) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xb46feff7);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32q_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 3 5 7 8 14 16 22 24 31 32)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32q_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x8dfdfeff) ^ (D[1] & 0x000000f6) ^ (c[0] & 0x0f68dfdf);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x96060301) ^ (D[1] & 0x0000011b) ^ (c[0] & 0x11b96060);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x2c0c0602) ^ (D[1] & 0x00000237) ^ (c[0] & 0x2372c0c0);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xd5e5f2fb) ^ (D[1] & 0x00000498) ^ (c[0] & 0x498d5e5f);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xabcbe5f6) ^ (D[1] & 0x00000931) ^ (c[0] & 0x931abcbe);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xda6a3513) ^ (D[1] & 0x00000295) ^ (c[0] & 0x295da6a3);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0xb4d46a26) ^ (D[1] & 0x0000052b) ^ (c[0] & 0x52bb4d46);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xe4552ab3) ^ (D[1] & 0x00000aa1) ^ (c[0] & 0xaa1e4552);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x4557ab99) ^ (D[1] & 0x000005b5) ^ (c[0] & 0x5b54557a);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x8aaf5732) ^ (D[1] & 0x00000b6a) ^ (c[0] & 0xb6a8aaf5);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x155eae64) ^ (D[1] & 0x000006d5) ^ (c[0] & 0x6d5155ea);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x2abd5cc8) ^ (D[1] & 0x00000daa) ^ (c[0] & 0xdaa2abd5);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x557ab990) ^ (D[1] & 0x00000b54) ^ (c[0] & 0xb54557ab);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xaaf57320) ^ (D[1] & 0x000006a8) ^ (c[0] & 0x6a8aaf57);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xd81718bf) ^ (D[1] & 0x00000da7) ^ (c[0] & 0xda7d8171);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xb02e317e) ^ (D[1] & 0x00000b4f) ^ (c[0] & 0xb4fb02e3);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0xeda19c03) ^ (D[1] & 0x00000669) ^ (c[0] & 0x669eda19);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xdb433806) ^ (D[1] & 0x00000cd3) ^ (c[0] & 0xcd3db433);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0xb686700c) ^ (D[1] & 0x000009a7) ^ (c[0] & 0x9a7b6867);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x6d0ce018) ^ (D[1] & 0x0000034f) ^ (c[0] & 0x34f6d0ce);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0xda19c030) ^ (D[1] & 0x0000069e) ^ (c[0] & 0x69eda19c);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xb4338060) ^ (D[1] & 0x00000d3d) ^ (c[0] & 0xd3db4338);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xe59afe3f) ^ (D[1] & 0x00000a8d) ^ (c[0] & 0xa8de59af);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xcb35fc7e) ^ (D[1] & 0x0000051b) ^ (c[0] & 0x51bcb35f);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x1b960603) ^ (D[1] & 0x00000ac1) ^ (c[0] & 0xac11b960);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x372c0c06) ^ (D[1] & 0x00000582) ^ (c[0] & 0x582372c0);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x6e58180c) ^ (D[1] & 0x00000b04) ^ (c[0] & 0xb046e581);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xdcb03018) ^ (D[1] & 0x00000608) ^ (c[0] & 0x608dcb03);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xb9606030) ^ (D[1] & 0x00000c11) ^ (c[0] & 0xc11b9606);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x72c0c060) ^ (D[1] & 0x00000823) ^ (c[0] & 0x82372c0c);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0xe58180c0) ^ (D[1] & 0x00000046) ^ (c[0] & 0x046e5818);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x46feff7f) ^ (D[1] & 0x0000007b) ^ (c[0] & 0x07b46fef);
    _PARITY(value);
    _H(31,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32q_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 3 5 7 8 14 16 22 24 31 32)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32q_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x8dfdfeff) ^ (D[1] & 0x08e640f6) ^ (c[0] & 0x8e640f68);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x96060301) ^ (D[1] & 0x092ac11b) ^ (c[0] & 0x92ac11b9);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x2c0c0602) ^ (D[1] & 0x02558237) ^ (c[0] & 0x25582372);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xd5e5f2fb) ^ (D[1] & 0x0c4d4498) ^ (c[0] & 0xc4d4498d);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xabcbe5f6) ^ (D[1] & 0x089a8931) ^ (c[0] & 0x89a8931a);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xda6a3513) ^ (D[1] & 0x09d35295) ^ (c[0] & 0x9d35295d);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0xb4d46a26) ^ (D[1] & 0x03a6a52b) ^ (c[0] & 0x3a6a52bb);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xe4552ab3) ^ (D[1] & 0x0fab0aa1) ^ (c[0] & 0xfab0aa1e);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x4557ab99) ^ (D[1] & 0x07b055b5) ^ (c[0] & 0x7b055b54);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x8aaf5732) ^ (D[1] & 0x0f60ab6a) ^ (c[0] & 0xf60ab6a8);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0x155eae64) ^ (D[1] & 0x0ec156d5) ^ (c[0] & 0xec156d51);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x2abd5cc8) ^ (D[1] & 0x0d82adaa) ^ (c[0] & 0xd82adaa2);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0x557ab990) ^ (D[1] & 0x0b055b54) ^ (c[0] & 0xb055b545);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xaaf57320) ^ (D[1] & 0x060ab6a8) ^ (c[0] & 0x60ab6a8a);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xd81718bf) ^ (D[1] & 0x04f32da7) ^ (c[0] & 0x4f32da7d);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0xb02e317e) ^ (D[1] & 0x09e65b4f) ^ (c[0] & 0x9e65b4fb);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0xeda19c03) ^ (D[1] & 0x0b2af669) ^ (c[0] & 0xb2af669e);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0xdb433806) ^ (D[1] & 0x0655ecd3) ^ (c[0] & 0x655ecd3d);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0xb686700c) ^ (D[1] & 0x0cabd9a7) ^ (c[0] & 0xcabd9a7b);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x6d0ce018) ^ (D[1] & 0x0957b34f) ^ (c[0] & 0x957b34f6);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0xda19c030) ^ (D[1] & 0x02af669e) ^ (c[0] & 0x2af669ed);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xb4338060) ^ (D[1] & 0x055ecd3d) ^ (c[0] & 0x55ecd3db);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xe59afe3f) ^ (D[1] & 0x025bda8d) ^ (c[0] & 0x25bda8de);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xcb35fc7e) ^ (D[1] & 0x04b7b51b) ^ (c[0] & 0x4b7b51bc);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x1b960603) ^ (D[1] & 0x01892ac1) ^ (c[0] & 0x1892ac11);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0x372c0c06) ^ (D[1] & 0x03125582) ^ (c[0] & 0x31255823);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x6e58180c) ^ (D[1] & 0x0624ab04) ^ (c[0] & 0x624ab046);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xdcb03018) ^ (D[1] & 0x0c495608) ^ (c[0] & 0xc495608d);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xb9606030) ^ (D[1] & 0x0892ac11) ^ (c[0] & 0x892ac11b);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0x72c0c060) ^ (D[1] & 0x01255823) ^ (c[0] & 0x12558237);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0xe58180c0) ^ (D[1] & 0x024ab046) ^ (c[0] & 0x24ab046e);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x46feff7f) ^ (D[1] & 0x0c73207b) ^ (c[0] & 0xc73207b4);
    _PARITY(value);
    _H(31,value);

}
/**
* @internal prvCpssDxChBrgFdbHashCrc_64_d36 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 4 7 9 10 12 13 17 19 21 22 23 24 27 29 31 32 33 35 37 38 39 40 45 46 47 52 53 54 55 57 62 64)
*       -- data width: 36
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_64_d36
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x172963d5) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x50000000) ^ (c[1] & 0xc172963d);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x397ba47f) ^ (D[1] & 0x00000004) ^ (c[0] & 0xf0000000) ^ (c[1] & 0x4397ba47);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x72f748fe) ^ (D[1] & 0x00000008) ^ (c[0] & 0xe0000000) ^ (c[1] & 0x872f748f);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe5ee91fc) ^ (c[0] & 0xc0000000) ^ (c[1] & 0x0e5ee91f);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xdcf4402d) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xd0000000) ^ (c[1] & 0xddcf4402);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xb9e8805a) ^ (D[1] & 0x0000000b) ^ (c[0] & 0xa0000000) ^ (c[1] & 0xbb9e8805);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x73d100b4) ^ (D[1] & 0x00000007) ^ (c[0] & 0x40000000) ^ (c[1] & 0x773d100b);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xf08b62bd) ^ (D[1] & 0x00000002) ^ (c[0] & 0xd0000000) ^ (c[1] & 0x2f08b62b);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xe116c57a) ^ (D[1] & 0x00000005) ^ (c[0] & 0xa0000000) ^ (c[1] & 0x5e116c57);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0xd504e921) ^ (D[1] & 0x00000007) ^ (c[0] & 0x10000000) ^ (c[1] & 0x7d504e92);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xbd20b197) ^ (D[1] & 0x00000003) ^ (c[0] & 0x70000000) ^ (c[1] & 0x3bd20b19);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x7a41632e) ^ (D[1] & 0x00000007) ^ (c[0] & 0xe0000000) ^ (c[1] & 0x77a41632);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xe3aba589) ^ (D[1] & 0x00000002) ^ (c[0] & 0x90000000) ^ (c[1] & 0x2e3aba58);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xd07e28c7) ^ (D[1] & 0x00000009) ^ (c[0] & 0x70000000) ^ (c[1] & 0x9d07e28c);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xa0fc518e) ^ (D[1] & 0x00000003) ^ (c[0] & 0xe0000000) ^ (c[1] & 0x3a0fc518);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x41f8a31c) ^ (D[1] & 0x00000007) ^ (c[0] & 0xc0000000) ^ (c[1] & 0x741f8a31);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x83f14638) ^ (D[1] & 0x0000000e) ^ (c[0] & 0x80000000) ^ (c[1] & 0xe83f1463);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0x10cbefa5) ^ (D[1] & 0x00000001) ^ (c[0] & 0x50000000) ^ (c[1] & 0x110cbefa);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x2197df4a) ^ (D[1] & 0x00000002) ^ (c[0] & 0xa0000000) ^ (c[1] & 0x22197df4);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x5406dd41) ^ (D[1] & 0x00000008) ^ (c[0] & 0x10000000) ^ (c[1] & 0x85406dd4);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0xa80dba82) ^ (c[0] & 0x20000000) ^ (c[1] & 0x0a80dba8);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0x473216d1) ^ (D[1] & 0x0000000d) ^ (c[0] & 0x10000000) ^ (c[1] & 0xd473216d);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0x994d4e77) ^ (D[1] & 0x00000006) ^ (c[0] & 0x70000000) ^ (c[1] & 0x6994d4e7);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x25b3ff3b) ^ (D[1] & 0x00000001) ^ (c[0] & 0xb0000000) ^ (c[1] & 0x125b3ff3);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x5c4e9da3) ^ (D[1] & 0x0000000e) ^ (c[0] & 0x30000000) ^ (c[1] & 0xe5c4e9da);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb89d3b46) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x60000000) ^ (c[1] & 0xcb89d3b4);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x713a768c) ^ (D[1] & 0x00000009) ^ (c[0] & 0xc0000000) ^ (c[1] & 0x9713a768);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xf55d8ecd) ^ (D[1] & 0x0000000e) ^ (c[0] & 0xd0000000) ^ (c[1] & 0xef55d8ec);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xeabb1d9a) ^ (D[1] & 0x0000000d) ^ (c[0] & 0xa0000000) ^ (c[1] & 0xdeabb1d9);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xc25f58e1) ^ (D[1] & 0x00000007) ^ (c[0] & 0x10000000) ^ (c[1] & 0x7c25f58e);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x84beb1c2) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x20000000) ^ (c[1] & 0xf84beb1c);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x1e540051) ^ (D[1] & 0x00000003) ^ (c[0] & 0x10000000) ^ (c[1] & 0x31e54005);
    _PARITY(value);
    _H(31,value);
    value = (D[0] & 0x2b816377) ^ (D[1] & 0x0000000a) ^ (c[0] & 0x70000000) ^ (c[1] & 0xa2b81637);
    _PARITY(value);
    _H(32,value);
    value = (D[0] & 0x402ba53b) ^ (D[1] & 0x00000008) ^ (c[0] & 0xb0000000) ^ (c[1] & 0x8402ba53);
    _PARITY(value);
    _H(33,value);
    value = (D[0] & 0x80574a76) ^ (c[0] & 0x60000000) ^ (c[1] & 0x080574a7);
    _PARITY(value);
    _H(34,value);
    value = (D[0] & 0x1787f739) ^ (D[1] & 0x0000000d) ^ (c[0] & 0x90000000) ^ (c[1] & 0xd1787f73);
    _PARITY(value);
    _H(35,value);
    value = (D[0] & 0x2f0fee72) ^ (D[1] & 0x0000000a) ^ (c[0] & 0x20000001) ^ (c[1] & 0xa2f0fee7);
    _PARITY(value);
    _H(36,value);
    value = (D[0] & 0x4936bf31) ^ (D[1] & 0x00000008) ^ (c[0] & 0x10000002) ^ (c[1] & 0x84936bf3);
    _PARITY(value);
    _H(37,value);
    value = (D[0] & 0x85441db7) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x70000004) ^ (c[1] & 0xc85441db);
    _PARITY(value);
    _H(38,value);
    value = (D[0] & 0x1da158bb) ^ (D[1] & 0x00000005) ^ (c[0] & 0xb0000008) ^ (c[1] & 0x51da158b);
    _PARITY(value);
    _H(39,value);
    value = (D[0] & 0x2c6bd2a3) ^ (D[1] & 0x00000006) ^ (c[0] & 0x30000010) ^ (c[1] & 0x62c6bd2a);
    _PARITY(value);
    _H(40,value);
    value = (D[0] & 0x58d7a546) ^ (D[1] & 0x0000000c) ^ (c[0] & 0x60000020) ^ (c[1] & 0xc58d7a54);
    _PARITY(value);
    _H(41,value);
    value = (D[0] & 0xb1af4a8c) ^ (D[1] & 0x00000008) ^ (c[0] & 0xc0000040) ^ (c[1] & 0x8b1af4a8);
    _PARITY(value);
    _H(42,value);
    value = (D[0] & 0x635e9518) ^ (D[1] & 0x00000001) ^ (c[0] & 0x80000080) ^ (c[1] & 0x1635e951);
    _PARITY(value);
    _H(43,value);
    value = (D[0] & 0xc6bd2a30) ^ (D[1] & 0x00000002) ^ (c[0] & 0x00000100) ^ (c[1] & 0x2c6bd2a3);
    _PARITY(value);
    _H(44,value);
    value = (D[0] & 0x9a5337b5) ^ (D[1] & 0x00000009) ^ (c[0] & 0x50000200) ^ (c[1] & 0x99a5337b);
    _PARITY(value);
    _H(45,value);
    value = (D[0] & 0x238f0cbf) ^ (D[1] & 0x0000000f) ^ (c[0] & 0xf0000400) ^ (c[1] & 0xf238f0cb);
    _PARITY(value);
    _H(46,value);
    value = (D[0] & 0x50377aab) ^ (D[1] & 0x00000002) ^ (c[0] & 0xb0000800) ^ (c[1] & 0x250377aa);
    _PARITY(value);
    _H(47,value);
    value = (D[0] & 0xa06ef556) ^ (D[1] & 0x00000004) ^ (c[0] & 0x60001000) ^ (c[1] & 0x4a06ef55);
    _PARITY(value);
    _H(48,value);
    value = (D[0] & 0x40ddeaac) ^ (D[1] & 0x00000009) ^ (c[0] & 0xc0002000) ^ (c[1] & 0x940ddeaa);
    _PARITY(value);
    _H(49,value);
    value = (D[0] & 0x81bbd558) ^ (D[1] & 0x00000002) ^ (c[0] & 0x80004000) ^ (c[1] & 0x281bbd55);
    _PARITY(value);
    _H(50,value);
    value = (D[0] & 0x0377aab0) ^ (D[1] & 0x00000005) ^ (c[0] & 0x00008000) ^ (c[1] & 0x50377aab);
    _PARITY(value);
    _H(51,value);
    value = (D[0] & 0x11c636b5) ^ (D[1] & 0x00000006) ^ (c[0] & 0x50010000) ^ (c[1] & 0x611c636b);
    _PARITY(value);
    _H(52,value);
    value = (D[0] & 0x34a50ebf) ^ (c[0] & 0xf0020000) ^ (c[1] & 0x034a50eb);
    _PARITY(value);
    _H(53,value);
    value = (D[0] & 0x7e637eab) ^ (D[1] & 0x0000000c) ^ (c[0] & 0xb0040000) ^ (c[1] & 0xc7e637ea);
    _PARITY(value);
    _H(54,value);
    value = (D[0] & 0xebef9e83) ^ (D[1] & 0x00000004) ^ (c[0] & 0x30080000) ^ (c[1] & 0x4ebef9e8);
    _PARITY(value);
    _H(55,value);
    value = (D[0] & 0xd7df3d06) ^ (D[1] & 0x00000009) ^ (c[0] & 0x60100000) ^ (c[1] & 0x9d7df3d0);
    _PARITY(value);
    _H(56,value);
    value = (D[0] & 0xb89719d9) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x90200000) ^ (c[1] & 0xfb89719d);
    _PARITY(value);
    _H(57,value);
    value = (D[0] & 0x712e33b2) ^ (D[1] & 0x0000000f) ^ (c[0] & 0x20400000) ^ (c[1] & 0xf712e33b);
    _PARITY(value);
    _H(58,value);
    value = (D[0] & 0xe25c6764) ^ (D[1] & 0x0000000e) ^ (c[0] & 0x40800000) ^ (c[1] & 0xee25c676);
    _PARITY(value);
    _H(59,value);
    value = (D[0] & 0xc4b8cec8) ^ (D[1] & 0x0000000d) ^ (c[0] & 0x81000000) ^ (c[1] & 0xdc4b8cec);
    _PARITY(value);
    _H(60,value);
    value = (D[0] & 0x89719d90) ^ (D[1] & 0x0000000b) ^ (c[0] & 0x02000000) ^ (c[1] & 0xb89719d9);
    _PARITY(value);
    _H(61,value);
    value = (D[0] & 0x05ca58f5) ^ (D[1] & 0x0000000b) ^ (c[0] & 0x54000000) ^ (c[1] & 0xb05ca58f);
    _PARITY(value);
    _H(62,value);
    value = (D[0] & 0x0b94b1ea) ^ (D[1] & 0x00000006) ^ (c[0] & 0xa8000000) ^ (c[1] & 0x60b94b1e);
    _PARITY(value);
    _H(63,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_64_d44 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 4 7 9 10 12 13 17 19 21 22 23 24 27 29 31 32 33 35 37 38 39 40 45 46 47 52 53 54 55 57 62 64)
*       -- data width: 44
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_64_d44
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x172963d5) ^ (D[1] & 0x0000066c) ^ (c[0] & 0x3d500000) ^ (c[1] & 0x66c17296);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x397ba47f) ^ (D[1] & 0x00000ab4) ^ (c[0] & 0x47f00000) ^ (c[1] & 0xab4397ba);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x72f748fe) ^ (D[1] & 0x00000568) ^ (c[0] & 0x8fe00000) ^ (c[1] & 0x56872f74);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe5ee91fc) ^ (D[1] & 0x00000ad0) ^ (c[0] & 0x1fc00000) ^ (c[1] & 0xad0e5ee9);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xdcf4402d) ^ (D[1] & 0x000003cd) ^ (c[0] & 0x02d00000) ^ (c[1] & 0x3cddcf44);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xb9e8805a) ^ (D[1] & 0x0000079b) ^ (c[0] & 0x05a00000) ^ (c[1] & 0x79bb9e88);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x73d100b4) ^ (D[1] & 0x00000f37) ^ (c[0] & 0x0b400000) ^ (c[1] & 0xf3773d10);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xf08b62bd) ^ (D[1] & 0x00000802) ^ (c[0] & 0x2bd00000) ^ (c[1] & 0x802f08b6);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xe116c57a) ^ (D[1] & 0x00000005) ^ (c[0] & 0x57a00000) ^ (c[1] & 0x005e116c);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0xd504e921) ^ (D[1] & 0x00000667) ^ (c[0] & 0x92100000) ^ (c[1] & 0x667d504e);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xbd20b197) ^ (D[1] & 0x00000aa3) ^ (c[0] & 0x19700000) ^ (c[1] & 0xaa3bd20b);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x7a41632e) ^ (D[1] & 0x00000547) ^ (c[0] & 0x32e00000) ^ (c[1] & 0x5477a416);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xe3aba589) ^ (D[1] & 0x00000ce2) ^ (c[0] & 0x58900000) ^ (c[1] & 0xce2e3aba);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xd07e28c7) ^ (D[1] & 0x00000fa9) ^ (c[0] & 0x8c700000) ^ (c[1] & 0xfa9d07e2);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xa0fc518e) ^ (D[1] & 0x00000f53) ^ (c[0] & 0x18e00000) ^ (c[1] & 0xf53a0fc5);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x41f8a31c) ^ (D[1] & 0x00000ea7) ^ (c[0] & 0x31c00000) ^ (c[1] & 0xea741f8a);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x83f14638) ^ (D[1] & 0x00000d4e) ^ (c[0] & 0x63800000) ^ (c[1] & 0xd4e83f14);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0x10cbefa5) ^ (D[1] & 0x00000cf1) ^ (c[0] & 0xfa500000) ^ (c[1] & 0xcf110cbe);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x2197df4a) ^ (D[1] & 0x000009e2) ^ (c[0] & 0xf4a00000) ^ (c[1] & 0x9e22197d);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x5406dd41) ^ (D[1] & 0x000005a8) ^ (c[0] & 0xd4100000) ^ (c[1] & 0x5a85406d);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0xa80dba82) ^ (D[1] & 0x00000b50) ^ (c[0] & 0xa8200000) ^ (c[1] & 0xb50a80db);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0x473216d1) ^ (D[1] & 0x000000cd) ^ (c[0] & 0x6d100000) ^ (c[1] & 0x0cd47321);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0x994d4e77) ^ (D[1] & 0x000007f6) ^ (c[0] & 0xe7700000) ^ (c[1] & 0x7f6994d4);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x25b3ff3b) ^ (D[1] & 0x00000981) ^ (c[0] & 0xf3b00000) ^ (c[1] & 0x98125b3f);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x5c4e9da3) ^ (D[1] & 0x0000056e) ^ (c[0] & 0xda300000) ^ (c[1] & 0x56e5c4e9);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb89d3b46) ^ (D[1] & 0x00000adc) ^ (c[0] & 0xb4600000) ^ (c[1] & 0xadcb89d3);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x713a768c) ^ (D[1] & 0x000005b9) ^ (c[0] & 0x68c00000) ^ (c[1] & 0x5b9713a7);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xf55d8ecd) ^ (D[1] & 0x00000d1e) ^ (c[0] & 0xecd00000) ^ (c[1] & 0xd1ef55d8);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xeabb1d9a) ^ (D[1] & 0x00000a3d) ^ (c[0] & 0xd9a00000) ^ (c[1] & 0xa3deabb1);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xc25f58e1) ^ (D[1] & 0x00000217) ^ (c[0] & 0x8e100000) ^ (c[1] & 0x217c25f5);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x84beb1c2) ^ (D[1] & 0x0000042f) ^ (c[0] & 0x1c200000) ^ (c[1] & 0x42f84beb);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x1e540051) ^ (D[1] & 0x00000e33) ^ (c[0] & 0x05100000) ^ (c[1] & 0xe331e540);
    _PARITY(value);
    _H(31,value);
    value = (D[0] & 0x2b816377) ^ (D[1] & 0x00000a0a) ^ (c[0] & 0x37700000) ^ (c[1] & 0xa0a2b816);
    _PARITY(value);
    _H(32,value);
    value = (D[0] & 0x402ba53b) ^ (D[1] & 0x00000278) ^ (c[0] & 0x53b00000) ^ (c[1] & 0x278402ba);
    _PARITY(value);
    _H(33,value);
    value = (D[0] & 0x80574a76) ^ (D[1] & 0x000004f0) ^ (c[0] & 0xa7600000) ^ (c[1] & 0x4f080574);
    _PARITY(value);
    _H(34,value);
    value = (D[0] & 0x1787f739) ^ (D[1] & 0x00000f8d) ^ (c[0] & 0x73900000) ^ (c[1] & 0xf8d1787f);
    _PARITY(value);
    _H(35,value);
    value = (D[0] & 0x2f0fee72) ^ (D[1] & 0x00000f1a) ^ (c[0] & 0xe7200000) ^ (c[1] & 0xf1a2f0fe);
    _PARITY(value);
    _H(36,value);
    value = (D[0] & 0x4936bf31) ^ (D[1] & 0x00000858) ^ (c[0] & 0xf3100000) ^ (c[1] & 0x8584936b);
    _PARITY(value);
    _H(37,value);
    value = (D[0] & 0x85441db7) ^ (D[1] & 0x000006dc) ^ (c[0] & 0xdb700000) ^ (c[1] & 0x6dc85441);
    _PARITY(value);
    _H(38,value);
    value = (D[0] & 0x1da158bb) ^ (D[1] & 0x00000bd5) ^ (c[0] & 0x8bb00000) ^ (c[1] & 0xbd51da15);
    _PARITY(value);
    _H(39,value);
    value = (D[0] & 0x2c6bd2a3) ^ (D[1] & 0x000001c6) ^ (c[0] & 0x2a300000) ^ (c[1] & 0x1c62c6bd);
    _PARITY(value);
    _H(40,value);
    value = (D[0] & 0x58d7a546) ^ (D[1] & 0x0000038c) ^ (c[0] & 0x54600000) ^ (c[1] & 0x38c58d7a);
    _PARITY(value);
    _H(41,value);
    value = (D[0] & 0xb1af4a8c) ^ (D[1] & 0x00000718) ^ (c[0] & 0xa8c00000) ^ (c[1] & 0x718b1af4);
    _PARITY(value);
    _H(42,value);
    value = (D[0] & 0x635e9518) ^ (D[1] & 0x00000e31) ^ (c[0] & 0x51800000) ^ (c[1] & 0xe31635e9);
    _PARITY(value);
    _H(43,value);
    value = (D[0] & 0xc6bd2a30) ^ (D[1] & 0x00000c62) ^ (c[0] & 0xa3000001) ^ (c[1] & 0xc62c6bd2);
    _PARITY(value);
    _H(44,value);
    value = (D[0] & 0x9a5337b5) ^ (D[1] & 0x00000ea9) ^ (c[0] & 0x7b500002) ^ (c[1] & 0xea99a533);
    _PARITY(value);
    _H(45,value);
    value = (D[0] & 0x238f0cbf) ^ (D[1] & 0x00000b3f) ^ (c[0] & 0xcbf00004) ^ (c[1] & 0xb3f238f0);
    _PARITY(value);
    _H(46,value);
    value = (D[0] & 0x50377aab) ^ (D[1] & 0x00000012) ^ (c[0] & 0xaab00008) ^ (c[1] & 0x01250377);
    _PARITY(value);
    _H(47,value);
    value = (D[0] & 0xa06ef556) ^ (D[1] & 0x00000024) ^ (c[0] & 0x55600010) ^ (c[1] & 0x024a06ef);
    _PARITY(value);
    _H(48,value);
    value = (D[0] & 0x40ddeaac) ^ (D[1] & 0x00000049) ^ (c[0] & 0xaac00020) ^ (c[1] & 0x04940dde);
    _PARITY(value);
    _H(49,value);
    value = (D[0] & 0x81bbd558) ^ (D[1] & 0x00000092) ^ (c[0] & 0x55800040) ^ (c[1] & 0x09281bbd);
    _PARITY(value);
    _H(50,value);
    value = (D[0] & 0x0377aab0) ^ (D[1] & 0x00000125) ^ (c[0] & 0xab000080) ^ (c[1] & 0x1250377a);
    _PARITY(value);
    _H(51,value);
    value = (D[0] & 0x11c636b5) ^ (D[1] & 0x00000426) ^ (c[0] & 0x6b500100) ^ (c[1] & 0x42611c63);
    _PARITY(value);
    _H(52,value);
    value = (D[0] & 0x34a50ebf) ^ (D[1] & 0x00000e20) ^ (c[0] & 0xebf00200) ^ (c[1] & 0xe2034a50);
    _PARITY(value);
    _H(53,value);
    value = (D[0] & 0x7e637eab) ^ (D[1] & 0x00000a2c) ^ (c[0] & 0xeab00400) ^ (c[1] & 0xa2c7e637);
    _PARITY(value);
    _H(54,value);
    value = (D[0] & 0xebef9e83) ^ (D[1] & 0x00000234) ^ (c[0] & 0xe8300800) ^ (c[1] & 0x234ebef9);
    _PARITY(value);
    _H(55,value);
    value = (D[0] & 0xd7df3d06) ^ (D[1] & 0x00000469) ^ (c[0] & 0xd0601000) ^ (c[1] & 0x469d7df3);
    _PARITY(value);
    _H(56,value);
    value = (D[0] & 0xb89719d9) ^ (D[1] & 0x00000ebf) ^ (c[0] & 0x9d902000) ^ (c[1] & 0xebfb8971);
    _PARITY(value);
    _H(57,value);
    value = (D[0] & 0x712e33b2) ^ (D[1] & 0x00000d7f) ^ (c[0] & 0x3b204000) ^ (c[1] & 0xd7f712e3);
    _PARITY(value);
    _H(58,value);
    value = (D[0] & 0xe25c6764) ^ (D[1] & 0x00000afe) ^ (c[0] & 0x76408000) ^ (c[1] & 0xafee25c6);
    _PARITY(value);
    _H(59,value);
    value = (D[0] & 0xc4b8cec8) ^ (D[1] & 0x000005fd) ^ (c[0] & 0xec810000) ^ (c[1] & 0x5fdc4b8c);
    _PARITY(value);
    _H(60,value);
    value = (D[0] & 0x89719d90) ^ (D[1] & 0x00000bfb) ^ (c[0] & 0xd9020000) ^ (c[1] & 0xbfb89719);
    _PARITY(value);
    _H(61,value);
    value = (D[0] & 0x05ca58f5) ^ (D[1] & 0x0000019b) ^ (c[0] & 0x8f540000) ^ (c[1] & 0x19b05ca5);
    _PARITY(value);
    _H(62,value);
    value = (D[0] & 0x0b94b1ea) ^ (D[1] & 0x00000336) ^ (c[0] & 0x1ea80000) ^ (c[1] & 0x3360b94b);
    _PARITY(value);
    _H(63,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_64_d60 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in] D                        - array of 32bit words, contain data.
* @param[in] c                        - initial crvalue (words array)
*
* @param[out] h                        - the haswords array.
*                                       None.
*
* @note -- polynomial: (0 1 4 7 9 10 12 13 17 19 21 22 23 24 27 29 31 32 33 35 37 38 39 40 45 46 47 52 53 54 55 57 62 64)
*       -- data width: 60
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_64_d60
(
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0x172963d5) ^ (D[1] & 0x0c3e466c) ^ (c[0] & 0x72963d50) ^ (c[1] & 0xc3e466c1);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x397ba47f) ^ (D[1] & 0x0442cab4) ^ (c[0] & 0x97ba47f0) ^ (c[1] & 0x442cab43);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0x72f748fe) ^ (D[1] & 0x08859568) ^ (c[0] & 0x2f748fe0) ^ (c[1] & 0x88595687);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0xe5ee91fc) ^ (D[1] & 0x010b2ad0) ^ (c[0] & 0x5ee91fc0) ^ (c[1] & 0x10b2ad0e);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xdcf4402d) ^ (D[1] & 0x0e2813cd) ^ (c[0] & 0xcf4402d0) ^ (c[1] & 0xe2813cdd);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0xb9e8805a) ^ (D[1] & 0x0c50279b) ^ (c[0] & 0x9e8805a0) ^ (c[1] & 0xc50279bb);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x73d100b4) ^ (D[1] & 0x08a04f37) ^ (c[0] & 0x3d100b40) ^ (c[1] & 0x8a04f377);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0xf08b62bd) ^ (D[1] & 0x0d7ed802) ^ (c[0] & 0x08b62bd0) ^ (c[1] & 0xd7ed802f);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0xe116c57a) ^ (D[1] & 0x0afdb005) ^ (c[0] & 0x116c57a0) ^ (c[1] & 0xafdb005e);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0xd504e921) ^ (D[1] & 0x09c52667) ^ (c[0] & 0x504e9210) ^ (c[1] & 0x9c52667d);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xbd20b197) ^ (D[1] & 0x0fb40aa3) ^ (c[0] & 0xd20b1970) ^ (c[1] & 0xfb40aa3b);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x7a41632e) ^ (D[1] & 0x0f681547) ^ (c[0] & 0xa41632e0) ^ (c[1] & 0xf6815477);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xe3aba589) ^ (D[1] & 0x02ee6ce2) ^ (c[0] & 0x3aba5890) ^ (c[1] & 0x2ee6ce2e);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0xd07e28c7) ^ (D[1] & 0x09e29fa9) ^ (c[0] & 0x07e28c70) ^ (c[1] & 0x9e29fa9d);
    _PARITY(value);
    _H(13,value);
    value = (D[0] & 0xa0fc518e) ^ (D[1] & 0x03c53f53) ^ (c[0] & 0x0fc518e0) ^ (c[1] & 0x3c53f53a);
    _PARITY(value);
    _H(14,value);
    value = (D[0] & 0x41f8a31c) ^ (D[1] & 0x078a7ea7) ^ (c[0] & 0x1f8a31c0) ^ (c[1] & 0x78a7ea74);
    _PARITY(value);
    _H(15,value);
    value = (D[0] & 0x83f14638) ^ (D[1] & 0x0f14fd4e) ^ (c[0] & 0x3f146380) ^ (c[1] & 0xf14fd4e8);
    _PARITY(value);
    _H(16,value);
    value = (D[0] & 0x10cbefa5) ^ (D[1] & 0x0217bcf1) ^ (c[0] & 0x0cbefa50) ^ (c[1] & 0x217bcf11);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x2197df4a) ^ (D[1] & 0x042f79e2) ^ (c[0] & 0x197df4a0) ^ (c[1] & 0x42f79e22);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x5406dd41) ^ (D[1] & 0x0460b5a8) ^ (c[0] & 0x406dd410) ^ (c[1] & 0x460b5a85);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0xa80dba82) ^ (D[1] & 0x08c16b50) ^ (c[0] & 0x80dba820) ^ (c[1] & 0x8c16b50a);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0x473216d1) ^ (D[1] & 0x0dbc90cd) ^ (c[0] & 0x73216d10) ^ (c[1] & 0xdbc90cd4);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0x994d4e77) ^ (D[1] & 0x074767f6) ^ (c[0] & 0x94d4e770) ^ (c[1] & 0x74767f69);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0x25b3ff3b) ^ (D[1] & 0x02b08981) ^ (c[0] & 0x5b3ff3b0) ^ (c[1] & 0x2b089812);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x5c4e9da3) ^ (D[1] & 0x095f556e) ^ (c[0] & 0xc4e9da30) ^ (c[1] & 0x95f556e5);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb89d3b46) ^ (D[1] & 0x02beaadc) ^ (c[0] & 0x89d3b460) ^ (c[1] & 0x2beaadcb);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x713a768c) ^ (D[1] & 0x057d55b9) ^ (c[0] & 0x13a768c0) ^ (c[1] & 0x57d55b97);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0xf55d8ecd) ^ (D[1] & 0x06c4ed1e) ^ (c[0] & 0x55d8ecd0) ^ (c[1] & 0x6c4ed1ef);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0xeabb1d9a) ^ (D[1] & 0x0d89da3d) ^ (c[0] & 0xabb1d9a0) ^ (c[1] & 0xd89da3de);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xc25f58e1) ^ (D[1] & 0x072df217) ^ (c[0] & 0x25f58e10) ^ (c[1] & 0x72df217c);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x84beb1c2) ^ (D[1] & 0x0e5be42f) ^ (c[0] & 0x4beb1c20) ^ (c[1] & 0xe5be42f8);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0x1e540051) ^ (D[1] & 0x00898e33) ^ (c[0] & 0xe5400510) ^ (c[1] & 0x0898e331);
    _PARITY(value);
    _H(31,value);
    value = (D[0] & 0x2b816377) ^ (D[1] & 0x0d2d5a0a) ^ (c[0] & 0xb8163770) ^ (c[1] & 0xd2d5a0a2);
    _PARITY(value);
    _H(32,value);
    value = (D[0] & 0x402ba53b) ^ (D[1] & 0x0664f278) ^ (c[0] & 0x02ba53b0) ^ (c[1] & 0x664f2784);
    _PARITY(value);
    _H(33,value);
    value = (D[0] & 0x80574a76) ^ (D[1] & 0x0cc9e4f0) ^ (c[0] & 0x0574a760) ^ (c[1] & 0xcc9e4f08);
    _PARITY(value);
    _H(34,value);
    value = (D[0] & 0x1787f739) ^ (D[1] & 0x05ad8f8d) ^ (c[0] & 0x787f7390) ^ (c[1] & 0x5ad8f8d1);
    _PARITY(value);
    _H(35,value);
    value = (D[0] & 0x2f0fee72) ^ (D[1] & 0x0b5b1f1a) ^ (c[0] & 0xf0fee720) ^ (c[1] & 0xb5b1f1a2);
    _PARITY(value);
    _H(36,value);
    value = (D[0] & 0x4936bf31) ^ (D[1] & 0x0a887858) ^ (c[0] & 0x936bf310) ^ (c[1] & 0xa8878584);
    _PARITY(value);
    _H(37,value);
    value = (D[0] & 0x85441db7) ^ (D[1] & 0x092eb6dc) ^ (c[0] & 0x5441db70) ^ (c[1] & 0x92eb6dc8);
    _PARITY(value);
    _H(38,value);
    value = (D[0] & 0x1da158bb) ^ (D[1] & 0x0e632bd5) ^ (c[0] & 0xda158bb0) ^ (c[1] & 0xe632bd51);
    _PARITY(value);
    _H(39,value);
    value = (D[0] & 0x2c6bd2a3) ^ (D[1] & 0x00f811c6) ^ (c[0] & 0xc6bd2a30) ^ (c[1] & 0x0f811c62);
    _PARITY(value);
    _H(40,value);
    value = (D[0] & 0x58d7a546) ^ (D[1] & 0x01f0238c) ^ (c[0] & 0x8d7a5460) ^ (c[1] & 0x1f0238c5);
    _PARITY(value);
    _H(41,value);
    value = (D[0] & 0xb1af4a8c) ^ (D[1] & 0x03e04718) ^ (c[0] & 0x1af4a8c0) ^ (c[1] & 0x3e04718b);
    _PARITY(value);
    _H(42,value);
    value = (D[0] & 0x635e9518) ^ (D[1] & 0x07c08e31) ^ (c[0] & 0x35e95180) ^ (c[1] & 0x7c08e316);
    _PARITY(value);
    _H(43,value);
    value = (D[0] & 0xc6bd2a30) ^ (D[1] & 0x0f811c62) ^ (c[0] & 0x6bd2a300) ^ (c[1] & 0xf811c62c);
    _PARITY(value);
    _H(44,value);
    value = (D[0] & 0x9a5337b5) ^ (D[1] & 0x033c7ea9) ^ (c[0] & 0xa5337b50) ^ (c[1] & 0x33c7ea99);
    _PARITY(value);
    _H(45,value);
    value = (D[0] & 0x238f0cbf) ^ (D[1] & 0x0a46bb3f) ^ (c[0] & 0x38f0cbf0) ^ (c[1] & 0xa46bb3f2);
    _PARITY(value);
    _H(46,value);
    value = (D[0] & 0x50377aab) ^ (D[1] & 0x08b33012) ^ (c[0] & 0x0377aab0) ^ (c[1] & 0x8b330125);
    _PARITY(value);
    _H(47,value);
    value = (D[0] & 0xa06ef556) ^ (D[1] & 0x01666024) ^ (c[0] & 0x06ef5560) ^ (c[1] & 0x1666024a);
    _PARITY(value);
    _H(48,value);
    value = (D[0] & 0x40ddeaac) ^ (D[1] & 0x02ccc049) ^ (c[0] & 0x0ddeaac0) ^ (c[1] & 0x2ccc0494);
    _PARITY(value);
    _H(49,value);
    value = (D[0] & 0x81bbd558) ^ (D[1] & 0x05998092) ^ (c[0] & 0x1bbd5580) ^ (c[1] & 0x59980928);
    _PARITY(value);
    _H(50,value);
    value = (D[0] & 0x0377aab0) ^ (D[1] & 0x0b330125) ^ (c[0] & 0x377aab00) ^ (c[1] & 0xb3301250);
    _PARITY(value);
    _H(51,value);
    value = (D[0] & 0x11c636b5) ^ (D[1] & 0x0a584426) ^ (c[0] & 0x1c636b50) ^ (c[1] & 0xa5844261);
    _PARITY(value);
    _H(52,value);
    value = (D[0] & 0x34a50ebf) ^ (D[1] & 0x088ece20) ^ (c[0] & 0x4a50ebf0) ^ (c[1] & 0x88ece203);
    _PARITY(value);
    _H(53,value);
    value = (D[0] & 0x7e637eab) ^ (D[1] & 0x0d23da2c) ^ (c[0] & 0xe637eab0) ^ (c[1] & 0xd23da2c7);
    _PARITY(value);
    _H(54,value);
    value = (D[0] & 0xebef9e83) ^ (D[1] & 0x0679f234) ^ (c[0] & 0xbef9e830) ^ (c[1] & 0x679f234e);
    _PARITY(value);
    _H(55,value);
    value = (D[0] & 0xd7df3d06) ^ (D[1] & 0x0cf3e469) ^ (c[0] & 0x7df3d060) ^ (c[1] & 0xcf3e469d);
    _PARITY(value);
    _H(56,value);
    value = (D[0] & 0xb89719d9) ^ (D[1] & 0x05d98ebf) ^ (c[0] & 0x89719d90) ^ (c[1] & 0x5d98ebfb);
    _PARITY(value);
    _H(57,value);
    value = (D[0] & 0x712e33b2) ^ (D[1] & 0x0bb31d7f) ^ (c[0] & 0x12e33b20) ^ (c[1] & 0xbb31d7f7);
    _PARITY(value);
    _H(58,value);
    value = (D[0] & 0xe25c6764) ^ (D[1] & 0x07663afe) ^ (c[0] & 0x25c67640) ^ (c[1] & 0x7663afee);
    _PARITY(value);
    _H(59,value);
    value = (D[0] & 0xc4b8cec8) ^ (D[1] & 0x0ecc75fd) ^ (c[0] & 0x4b8cec81) ^ (c[1] & 0xecc75fdc);
    _PARITY(value);
    _H(60,value);
    value = (D[0] & 0x89719d90) ^ (D[1] & 0x0d98ebfb) ^ (c[0] & 0x9719d902) ^ (c[1] & 0xd98ebfb8);
    _PARITY(value);
    _H(61,value);
    value = (D[0] & 0x05ca58f5) ^ (D[1] & 0x070f919b) ^ (c[0] & 0x5ca58f54) ^ (c[1] & 0x70f919b0);
    _PARITY(value);
    _H(62,value);
    value = (D[0] & 0x0b94b1ea) ^ (D[1] & 0x0e1f2336) ^ (c[0] & 0xb94b1ea8) ^ (c[1] & 0xe1f23360);
    _PARITY(value);
    _H(63,value);

}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32a_d76_noInitValueByNumBits function
* @endinternal
*
* @brief   This function calculates CRC 32a bits according to 76 bits DATA.
*         from the CRC32 only numOfBits are calculated.
* @param[in] numOfBits                - amount of bits in words data array
* @param[in] D                        - array of 32bit words, contain data
*
* @param[out] h                        - the hasbits array
*                                       None.
*
* @note based on HW file : sip_mt_crc_32a_d76.v
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d76_noInitValueByNumBits
(
    IN  GT_U32  numOfBits,
    IN  GT_U32  *D,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf7011641) ^ (D[1] & 0xb4e5b025) ^ (D[2] & 0x0000031e);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x19033ac3) ^ (D[1] & 0xdd2ed06e) ^ (D[2] & 0x00000523);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xc50763c7) ^ (D[1] & 0x0eb810f9) ^ (D[2] & 0x00000959);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x8a0ec78e) ^ (D[1] & 0x1d7021f3) ^ (D[2] & 0x000002b2);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xe31c995d) ^ (D[1] & 0x8e05f3c2) ^ (D[2] & 0x0000067a);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x313824fb) ^ (D[1] & 0xa8ee57a0) ^ (D[2] & 0x00000feb);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x627049f6) ^ (D[1] & 0x51dcaf40) ^ (D[2] & 0x00000fd7);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x33e185ad) ^ (D[1] & 0x175ceea5) ^ (D[2] & 0x00000cb0);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x90c21d1b) ^ (D[1] & 0x9a5c6d6f) ^ (D[2] & 0x00000a7e);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x21843a36) ^ (D[1] & 0x34b8dadf) ^ (D[2] & 0x000004fd);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xb409622d) ^ (D[1] & 0xdd94059b) ^ (D[2] & 0x00000ae4);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x9f13d21b) ^ (D[1] & 0x0fcdbb12) ^ (D[2] & 0x000006d7);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xc926b277) ^ (D[1] & 0xab7ec600) ^ (D[2] & 0x00000eb0);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x924d64ee) ^ (D[1] & 0x56fd8c01) ^ (D[2] & 0x00000d61);
    _PARITY(value);
    _H(13,value);

    if(numOfBits <= 14) return;

    value = (D[0] & 0x249ac9dc) ^ (D[1] & 0xadfb1803) ^ (D[2] & 0x00000ac2);
    _PARITY(value);
    _H(14,value);

    if(numOfBits <= 15) return;

    value = (D[0] & 0x493593b8) ^ (D[1] & 0x5bf63006) ^ (D[2] & 0x00000585);
    _PARITY(value);
    _H(15,value);

    if(numOfBits <= 16) return;

    value = (D[0] & 0x656a3131) ^ (D[1] & 0x0309d029) ^ (D[2] & 0x00000814);
    _PARITY(value);
    _H(16,value);

    if(numOfBits <= 17) return;

    value = (D[0] & 0xcad46262) ^ (D[1] & 0x0613a052) ^ (D[2] & 0x00000028);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x95a8c4c4) ^ (D[1] & 0x0c2740a5) ^ (D[2] & 0x00000050);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2b518988) ^ (D[1] & 0x184e814b) ^ (D[2] & 0x000000a0);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x56a31310) ^ (D[1] & 0x309d0296) ^ (D[2] & 0x00000140);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xad462620) ^ (D[1] & 0x613a052c) ^ (D[2] & 0x00000280);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xad8d5a01) ^ (D[1] & 0x7691ba7c) ^ (D[2] & 0x0000061e);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xac1ba243) ^ (D[1] & 0x59c6c4dc) ^ (D[2] & 0x00000f22);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x58374486) ^ (D[1] & 0xb38d89b9) ^ (D[2] & 0x00000e44);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb06e890c) ^ (D[1] & 0x671b1372) ^ (D[2] & 0x00000c89);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x97dc0459) ^ (D[1] & 0x7ad396c0) ^ (D[2] & 0x00000a0c);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x2fb808b2) ^ (D[1] & 0xf5a72d81) ^ (D[2] & 0x00000418);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x5f701164) ^ (D[1] & 0xeb4e5b02) ^ (D[2] & 0x00000831);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xbee022c8) ^ (D[1] & 0xd69cb604) ^ (D[2] & 0x00000063);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x7dc04590) ^ (D[1] & 0xad396c09) ^ (D[2] & 0x000000c7);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0xfb808b20) ^ (D[1] & 0x5a72d812) ^ (D[2] & 0x0000018f);
    _PARITY(value);
    _H(31,value);
}

/**
* @internal prvCpssDxChBrgFdbHashCrc_32a_d76 function
* @endinternal
*
* @brief   This function calculates CRC 32a bits according to 76 bits DATA.
*         from the CRC32 only numOfBits are calculated.
* @param[in] numOfBits                - amount of bits in words data array
* @param[in] D                        - array of 32bit words, contain data
*                                      C           - array of 32bit words, contain init values of CRC
*
* @param[out] h                        - the hasbits array
*                                       None.
*
* @note based on HW file : sip_mt_crc_32a_d76.v
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d76
(
    IN  GT_U32  numOfBits,
    IN  GT_U32  *D,
    IN  GT_U32  *c,
    OUT GT_U32  *h
)
{
    GT_U32 value;

    value = (D[0] & 0xf7011641) ^ (D[1] & 0xb4e5b025) ^ (D[2] & 0x0000031e) ^ (c[0] & 0x31eb4e5b);
    _PARITY(value);
    _H(0,value);
    value = (D[0] & 0x19033ac3) ^ (D[1] & 0xdd2ed06e) ^ (D[2] & 0x00000523) ^ (c[0] & 0x523dd2ed);
    _PARITY(value);
    _H(1,value);
    value = (D[0] & 0xc50763c7) ^ (D[1] & 0x0eb810f9) ^ (D[2] & 0x00000959) ^ (c[0] & 0x9590eb81);
    _PARITY(value);
    _H(2,value);
    value = (D[0] & 0x8a0ec78e) ^ (D[1] & 0x1d7021f3) ^ (D[2] & 0x000002b2) ^ (c[0] & 0x2b21d702);
    _PARITY(value);
    _H(3,value);
    value = (D[0] & 0xe31c995d) ^ (D[1] & 0x8e05f3c2) ^ (D[2] & 0x0000067a) ^ (c[0] & 0x67a8e05f);
    _PARITY(value);
    _H(4,value);
    value = (D[0] & 0x313824fb) ^ (D[1] & 0xa8ee57a0) ^ (D[2] & 0x00000feb) ^ (c[0] & 0xfeba8ee5);
    _PARITY(value);
    _H(5,value);
    value = (D[0] & 0x627049f6) ^ (D[1] & 0x51dcaf40) ^ (D[2] & 0x00000fd7) ^ (c[0] & 0xfd751dca);
    _PARITY(value);
    _H(6,value);
    value = (D[0] & 0x33e185ad) ^ (D[1] & 0x175ceea5) ^ (D[2] & 0x00000cb0) ^ (c[0] & 0xcb0175ce);
    _PARITY(value);
    _H(7,value);
    value = (D[0] & 0x90c21d1b) ^ (D[1] & 0x9a5c6d6f) ^ (D[2] & 0x00000a7e) ^ (c[0] & 0xa7e9a5c6);
    _PARITY(value);
    _H(8,value);
    value = (D[0] & 0x21843a36) ^ (D[1] & 0x34b8dadf) ^ (D[2] & 0x000004fd) ^ (c[0] & 0x4fd34b8d);
    _PARITY(value);
    _H(9,value);
    value = (D[0] & 0xb409622d) ^ (D[1] & 0xdd94059b) ^ (D[2] & 0x00000ae4) ^ (c[0] & 0xae4dd940);
    _PARITY(value);
    _H(10,value);
    value = (D[0] & 0x9f13d21b) ^ (D[1] & 0x0fcdbb12) ^ (D[2] & 0x000006d7) ^ (c[0] & 0x6d70fcdb);
    _PARITY(value);
    _H(11,value);
    value = (D[0] & 0xc926b277) ^ (D[1] & 0xab7ec600) ^ (D[2] & 0x00000eb0) ^ (c[0] & 0xeb0ab7ec);
    _PARITY(value);
    _H(12,value);
    value = (D[0] & 0x924d64ee) ^ (D[1] & 0x56fd8c01) ^ (D[2] & 0x00000d61) ^ (c[0] & 0xd6156fd8);
    _PARITY(value);
    _H(13,value);

    if(numOfBits <= 14) return;

    value = (D[0] & 0x249ac9dc) ^ (D[1] & 0xadfb1803) ^ (D[2] & 0x00000ac2) ^ (c[0] & 0xac2adfb1);
    _PARITY(value);
    _H(14,value);

    if(numOfBits <= 15) return;

    value = (D[0] & 0x493593b8) ^ (D[1] & 0x5bf63006) ^ (D[2] & 0x00000585) ^ (c[0] & 0x5855bf63);
    _PARITY(value);
    _H(15,value);

    if(numOfBits <= 16) return;

    value = (D[0] & 0x656a3131) ^ (D[1] & 0x0309d029) ^ (D[2] & 0x00000814) ^ (c[0] & 0x8140309d);
    _PARITY(value);
    _H(16,value);

    if(numOfBits <= 17) return;

    value = (D[0] & 0xcad46262) ^ (D[1] & 0x0613a052) ^ (D[2] & 0x00000028) ^ (c[0] & 0x0280613a);
    _PARITY(value);
    _H(17,value);
    value = (D[0] & 0x95a8c4c4) ^ (D[1] & 0x0c2740a5) ^ (D[2] & 0x00000050) ^ (c[0] & 0x0500c274);
    _PARITY(value);
    _H(18,value);
    value = (D[0] & 0x2b518988) ^ (D[1] & 0x184e814b) ^ (D[2] & 0x000000a0) ^ (c[0] & 0x0a0184e8);
    _PARITY(value);
    _H(19,value);
    value = (D[0] & 0x56a31310) ^ (D[1] & 0x309d0296) ^ (D[2] & 0x00000140) ^ (c[0] & 0x140309d0);
    _PARITY(value);
    _H(20,value);
    value = (D[0] & 0xad462620) ^ (D[1] & 0x613a052c) ^ (D[2] & 0x00000280) ^ (c[0] & 0x280613a0);
    _PARITY(value);
    _H(21,value);
    value = (D[0] & 0xad8d5a01) ^ (D[1] & 0x7691ba7c) ^ (D[2] & 0x0000061e) ^ (c[0] & 0x61e7691b);
    _PARITY(value);
    _H(22,value);
    value = (D[0] & 0xac1ba243) ^ (D[1] & 0x59c6c4dc) ^ (D[2] & 0x00000f22) ^ (c[0] & 0xf2259c6c);
    _PARITY(value);
    _H(23,value);
    value = (D[0] & 0x58374486) ^ (D[1] & 0xb38d89b9) ^ (D[2] & 0x00000e44) ^ (c[0] & 0xe44b38d8);
    _PARITY(value);
    _H(24,value);
    value = (D[0] & 0xb06e890c) ^ (D[1] & 0x671b1372) ^ (D[2] & 0x00000c89) ^ (c[0] & 0xc89671b1);
    _PARITY(value);
    _H(25,value);
    value = (D[0] & 0x97dc0459) ^ (D[1] & 0x7ad396c0) ^ (D[2] & 0x00000a0c) ^ (c[0] & 0xa0c7ad39);
    _PARITY(value);
    _H(26,value);
    value = (D[0] & 0x2fb808b2) ^ (D[1] & 0xf5a72d81) ^ (D[2] & 0x00000418) ^ (c[0] & 0x418f5a72);
    _PARITY(value);
    _H(27,value);
    value = (D[0] & 0x5f701164) ^ (D[1] & 0xeb4e5b02) ^ (D[2] & 0x00000831) ^ (c[0] & 0x831eb4e5);
    _PARITY(value);
    _H(28,value);
    value = (D[0] & 0xbee022c8) ^ (D[1] & 0xd69cb604) ^ (D[2] & 0x00000063) ^ (c[0] & 0x063d69cb);
    _PARITY(value);
    _H(29,value);
    value = (D[0] & 0x7dc04590) ^ (D[1] & 0xad396c09) ^ (D[2] & 0x000000c7) ^ (c[0] & 0x0c7ad396);
    _PARITY(value);
    _H(30,value);
    value = (D[0] & 0xfb808b20) ^ (D[1] & 0x5a72d812) ^ (D[2] & 0x0000018f) ^ (c[0] & 0x18f5a72d);
    _PARITY(value);
    _H(31,value);
}


