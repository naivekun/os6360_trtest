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
* @file cpssPxIngressHashUT.c
*
* @brief Unit tests for cpssPxIngressHash that provides
* CPSS implementation for packet hash calculation.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


UTF_TEST_CASE_MAC(cpssPxIngressHashUdeEthertypeSet)
{
/*
    1. ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with all valid parameters and validate
    Expected: GT_OK.
    1.2. Call with unexpected hashPacketType
    Expected: GT_BAD_PARAM.
    2. Call with not active and non-applicable devices
    Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM.
*/
    #define CASE_COUNT 2
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT packetTypes[CASE_COUNT] = {
                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E,
                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E
                };
    GT_U16      etherTypes[CASE_COUNT] = {0x5, 0x6};
    GT_U32      ii;
    GT_BOOL     valid;
    GT_U16      etherType;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(ii = 0; ii < CASE_COUNT; ii++)
        {
            st = cpssPxIngressHashUdeEthertypeSet(dev, packetTypes[ii],
                etherTypes[ii], GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, packetTypes[ii],
                etherTypes[ii]);
        }

        /* Validate */
        for(ii = 0; ii < CASE_COUNT; ii++)
        {
            st = cpssPxIngressHashUdeEthertypeGet(dev, packetTypes[ii],
                &etherType, &valid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, packetTypes[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, valid, dev, packetTypes[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(etherTypes[ii], etherType, dev,
                packetTypes[ii]);
        }

        /* Now with invalid validity */
        for(ii = 0; ii < CASE_COUNT; ii++)
        {
            st = cpssPxIngressHashUdeEthertypeSet(dev, packetTypes[ii],
                etherTypes[ii], GT_FALSE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, packetTypes[ii],
                etherTypes[ii]);
        }

        /* Validate */
        for(ii = 0; ii < CASE_COUNT; ii++)
        {
            st = cpssPxIngressHashUdeEthertypeGet(dev, packetTypes[ii],
                &etherType, &valid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, packetTypes[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FALSE, valid, dev, packetTypes[ii]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(etherTypes[ii], etherType, dev,
                packetTypes[ii]);
        }

        /*  1.2. Call with unexpected hashPacketType
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashUdeEthertypeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 0x1, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashUdeEthertypeSet(dev, packetTypes[0],
            etherTypes[0], GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashUdeEthertypeSet(dev, packetTypes[0],
        etherTypes[0], GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    #undef CASE_COUNT
}

UTF_TEST_CASE_MAC(cpssPxIngressHashUdeEthertypeGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with unexpected hashPacketType
        Expected: GT_BAD_PARAM.
        1.3. Call with invalid pointer on ethertype
        Expected: GT_BAD_PTR.
        1.4. Call with invalid pointer on 'valid' argument
        Expected: GT_BAD_PTR.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     valid;
    GT_U16      etherType;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        st = cpssPxIngressHashUdeEthertypeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, &etherType, &valid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E);

        /*  1.2. Call with unexpected hashPacketType
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashUdeEthertypeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, &etherType, &valid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        /*  1.3. Call with invalid pointer on ethertype
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressHashUdeEthertypeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, NULL, &valid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E);

        /*  1.4. Call with invalid pointer on 'valid' argument
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressHashUdeEthertypeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, &etherType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashUdeEthertypeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, &etherType, &valid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
    Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashUdeEthertypeGet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, &etherType, &valid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPacketTypeEntrySet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        1.2. Call with offset out of range
        Expected: GT_OUT_OF_RANGE
        1.3. Call with udbpArr=NULL
        Expected: GT_BAD_PTR
        1.4. Call with bad hashPacketType
        Expected: GT_BAD_PARAM
        1.5. Call with bad anchor
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT    pt;
    GT_U32 ii;

    CPSS_PX_INGRESS_HASH_UDBP_STC udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] =
    {
        /* 0  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  0, {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE}},
        /* 1  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  1, {GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE }},
        /* 2  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E,  2, {GT_FALSE, GT_FALSE, GT_TRUE , GT_FALSE}},
        /* 3  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  3, {GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE }},
        /* 4  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  4, {GT_FALSE, GT_TRUE , GT_FALSE, GT_FALSE}},
        /* 5  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E,  5, {GT_FALSE, GT_TRUE , GT_FALSE, GT_TRUE }},
        /* 6  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  6, {GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE}},
        /* 7  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  7, {GT_FALSE, GT_TRUE , GT_TRUE , GT_TRUE }},
        /* 8  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E,  8, {GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE}},
        /* 9  */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  9, {GT_TRUE , GT_FALSE, GT_FALSE, GT_TRUE }},
        /* 10 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E, 10, {GT_TRUE , GT_FALSE, GT_TRUE , GT_FALSE}},
        /* 11 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E, 11, {GT_TRUE , GT_FALSE, GT_TRUE , GT_TRUE }},
        /* 12 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 12, {GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE}},
        /* 13 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E, 13, {GT_TRUE , GT_TRUE , GT_FALSE, GT_TRUE }},
        /* 14 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E, 14, {GT_TRUE , GT_TRUE , GT_TRUE , GT_FALSE}},
        /* 15 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 15, {GT_TRUE , GT_TRUE , GT_TRUE , GT_TRUE }},
        /* 16 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  0, {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE}},
        /* 17 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E,  1, {GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE }},
        /* 18 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  2, {GT_FALSE, GT_FALSE, GT_TRUE , GT_FALSE}},
        /* 19 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  3, {GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE }},
        /* 20 */ {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E,  4, {GT_FALSE, GT_TRUE , GT_FALSE, GT_FALSE}}
    };
    CPSS_PX_INGRESS_HASH_UDBP_STC udbpArrGet[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    GT_BOOL valid = GT_FALSE;
    GT_BOOL validGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
        {
            udbpArr[16].offset = 59;
            udbpArr[17].offset = 60;
            udbpArr[18].offset = 61;
            udbpArr[19].offset = 62;
            udbpArr[20].offset = 63;
        }
        /*  1.1. Call with all valid parameters and validate
            Expected: GT_OK. */
        valid = GT_FALSE;
        for(pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;
            pt < CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; pt++)
        {
            st = cpssPxIngressHashPacketTypeEntrySet(dev, pt, udbpArr,
                valid = (valid == GT_TRUE) ? GT_FALSE : GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pt, valid);
        }

        /* Validate */
        valid = GT_FALSE;
        for(pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;
            pt < CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; pt++)
        {
            st = cpssPxIngressHashPacketTypeEntryGet(dev, pt, udbpArrGet,
                &validGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pt);
            for(ii = 0; ii < CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS; ii++)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(udbpArr[ii].anchor,
                                            udbpArrGet[ii].anchor);
                UTF_VERIFY_EQUAL0_PARAM_MAC(udbpArr[ii].offset,
                                            udbpArrGet[ii].offset);
                UTF_VERIFY_EQUAL0_PARAM_MAC(udbpArr[ii].nibbleMaskArr[0],
                                            udbpArrGet[ii].nibbleMaskArr[0]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(udbpArr[ii].nibbleMaskArr[1],
                                            udbpArrGet[ii].nibbleMaskArr[1]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(udbpArr[ii].nibbleMaskArr[2],
                                            udbpArrGet[ii].nibbleMaskArr[2]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(udbpArr[ii].nibbleMaskArr[3],
                                            udbpArrGet[ii].nibbleMaskArr[3]);
            }
            valid = (valid == GT_TRUE) ? GT_FALSE : GT_TRUE;
            UTF_VERIFY_EQUAL0_PARAM_MAC(valid, validGet);
        }

        /*  1.2. Call with offset out of range
            Expected: GT_OUT_OF_RANGE */
        pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
        {
            udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS - 1].offset = 64;
        }
        else
        {
            udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS - 1].offset = 16;
        }
        st = cpssPxIngressHashPacketTypeEntrySet(dev, pt, udbpArr, valid);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, pt, valid);
        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS - 1].offset = 4;

        /*  1.3. Call with udbpArr=NULL
            Expected: GT_BAD_PTR */
        st = cpssPxIngressHashPacketTypeEntrySet(dev, pt, NULL, valid);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, pt, valid);

        /*  1.4. Call with bad hashPacketType
            Expected: GT_BAD_PARAM */
        st = cpssPxIngressHashPacketTypeEntrySet(dev, 99, udbpArr, valid);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 99, valid);

        /*  1.5. Call with bad anchor
            Expected: GT_BAD_PARAM */
        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS - 1].anchor = 99;
        st = cpssPxIngressHashPacketTypeEntrySet(dev, pt, udbpArr, valid);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, pt, valid);
        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS - 1].anchor =
            CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E;
    }
    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPacketTypeEntrySet(dev, pt, udbpArr, valid);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pt,
            valid);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashPacketTypeEntrySet(dev, pt, udbpArr, valid);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, pt, valid);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPacketTypeEntryGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with valid parameter equal NULL
        Expected: GT_BAD_PTR.
        1.3. Call with bad hashPacketType
        Expected: GT_BAD_PARAM.
        1.4. Call with udbpArr parameter equal NULL
        Expected: GT_BAD_PTR.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT    pt;
    GT_BOOL                                 validGet;
    CPSS_PX_INGRESS_HASH_UDBP_STC udbpArrGet[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;
            pt < CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; pt++)
        {
            /*  1.1. Call with all valid parameters
                Expected: GT_OK. */
            st = cpssPxIngressHashPacketTypeEntryGet(dev, pt, udbpArrGet,
                &validGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pt);

            /*  1.2. Call with all valid parameter equal NULL
                Expected: GT_BAD_PTR. */
            st = cpssPxIngressHashPacketTypeEntryGet(dev, pt, udbpArrGet,
                NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, pt);

            /*  1.3. Call with bad hashPacketType
                Expected: GT_BAD_PARAM. */
            st = cpssPxIngressHashPacketTypeEntryGet(dev, 99, udbpArrGet,
                &validGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 99);

            /*  1.4. Call with udbpArr parameter equal NULL
                Expected: GT_BAD_PTR. */
            st = cpssPxIngressHashPacketTypeEntryGet(dev, pt, NULL,
                &validGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, pt);
        }
    }
    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPacketTypeEntryGet(dev, pt, udbpArrGet, &validGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashPacketTypeEntryGet(dev, pt, udbpArrGet, &validGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pt);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPacketTypeHashModeSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        1.2. Call with bit offsets out of range
        Expected: GT_BAD_PARAM.
        1.3. Call with bit offsets array =NULL
        Expected: GT_BAD_PTR for 'SELECTED BITS' mode, GT_OK else.
        1.4. Call with bad hashPacketType
        Expected: GT_BAD_PARAM
        1.5. Call with bad hashMode
        Expected: GT_BAD_PARAM
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT    pt;
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_PX_INGRESS_HASH_MODE_ENT           hashMode = CPSS_PX_INGRESS_HASH_MODE_CRC32_E;
    CPSS_PX_INGRESS_HASH_MODE_ENT           hashModeGet;
    GT_U32                                  ii;
    GT_U32  bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] =
        {1, 2, 3, 4, 5, 6, 343};
    GT_U32  bitOffsetsBadArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] =
        {344, 345, 346, 348, 5, 6, 343};
    GT_U32  bitOffsetsArrGet[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    GT_U32 hashSelectedBits;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        hashSelectedBits = (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev)) ? 
            CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS :
            CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS-1;

        for(pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;
            pt < CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; pt++)
        {
            for(hashMode = CPSS_PX_INGRESS_HASH_MODE_CRC32_E;
                hashMode <= CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E;
                hashMode++)
            {
                /*  1.1. Call with all valid parameters
                    Expected: GT_OK. */
                st = cpssPxIngressHashPacketTypeHashModeSet(dev, pt, hashMode,
                    bitOffsetsArr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pt, hashMode);

                /* Validate */
                st = cpssPxIngressHashPacketTypeHashModeGet(dev, pt,
                    &hashModeGet, bitOffsetsArrGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pt);
                if(CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E == hashMode)
                {
                    for(ii = 0; ii < hashSelectedBits;
                        ii++)
                    {
                        UTF_VERIFY_EQUAL0_PARAM_MAC(bitOffsetsArr[ii],
                            bitOffsetsArrGet[ii]);
                    }
                }
                UTF_VERIFY_EQUAL0_PARAM_MAC(hashMode, hashModeGet);

                /* 1.2. Call with all bit offsets out of range
                   Expected: GT_BAD_PARAM. */
                st = cpssPxIngressHashPacketTypeHashModeSet(dev, pt, hashMode,
                    bitOffsetsBadArr);
                if(CPSS_PX_INGRESS_HASH_MODE_CRC32_E == hashMode)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pt, hashMode);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, pt,
                        hashMode);
                }

                /* 1.3. Call with all bit offsets array =NULL
                   Expected: GT_BAD_PTR for 'SELECTED BITS' mode, GT_OK else. */
                st = cpssPxIngressHashPacketTypeHashModeSet(dev, pt, hashMode,
                    NULL);
                if(CPSS_PX_INGRESS_HASH_MODE_CRC32_E == hashMode)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pt, hashMode);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, pt,
                        hashMode);
                }

                /*  1.4. Call with bad hashPacketType
                    Expected: GT_BAD_PARAM */
                st = cpssPxIngressHashPacketTypeHashModeSet(dev, 99, hashMode,
                    bitOffsetsBadArr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 99,
                    hashMode);

                /*  1.5. Call with bad hashMode
                    Expected: GT_BAD_PARAM */
                st = cpssPxIngressHashPacketTypeHashModeSet(dev, pt, 99,
                    bitOffsetsBadArr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, pt, 99);
            }
        }
    }
    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPacketTypeHashModeSet(dev, pt, hashMode,
            bitOffsetsArr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pt,
            hashMode);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashPacketTypeHashModeSet(dev, pt, hashMode,
        bitOffsetsArr);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, pt, hashMode);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPacketTypeHashModeGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with NULL hashModePtr
        Expected: GT_BAD_PTR.
        1.3. Call with bad hashPacketType
        Expected: GT_BAD_PARAM
        1.4. Call with NULL bitOffsetsArr
        Expected: GT_BAD_PTR
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT    pt;
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_PX_INGRESS_HASH_MODE_ENT           hashModeGet;
    GT_U32  bitOffsetsArrGet[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(pt = CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E;
            pt < CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; pt++)
        {
            /*  1.1. Call with all valid parameters
                Expected: GT_OK. */
            st = cpssPxIngressHashPacketTypeHashModeGet(dev, pt, &hashModeGet,
                bitOffsetsArrGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pt);

            /* 1.2. Call with NULL hashModePtr
               Expected: GT_BAD_PTR. */
            st = cpssPxIngressHashPacketTypeHashModeGet(dev, pt, NULL,
                bitOffsetsArrGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, pt, NULL);

            /* 1.3. Call with bad hashPacketType
               Expected: GT_BAD_PARAM */
            st = cpssPxIngressHashPacketTypeHashModeGet(dev, 99, &hashModeGet,
                bitOffsetsArrGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 99);

            /* 1.4. Call with NULL bitOffsetsArr
               Expected: GT_BAD_PTR */
            st = cpssPxIngressHashPacketTypeHashModeGet(dev, pt, &hashModeGet,
                NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, pt);
        }
    }
    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPacketTypeHashModeGet(dev, pt, &hashModeGet,
            bitOffsetsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pt);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashPacketTypeHashModeGet(dev, pt, &hashModeGet,
        bitOffsetsArrGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pt);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashSeedSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      crc32Seed;
    GT_U32      crc32SeedGet;

    crc32Seed = 0xDEADBEEF;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        st = cpssPxIngressHashSeedSet(dev, crc32Seed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, crc32Seed);

        /* Validate */
        st = cpssPxIngressHashSeedGet(dev, &crc32SeedGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(crc32Seed, crc32SeedGet);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashSeedSet(dev, crc32Seed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            crc32Seed);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashSeedSet(dev, crc32Seed);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, crc32Seed);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashSeedGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with NULL crc32SeedPtr
        Expected: GT_BAD_PTR.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      crc32SeedGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        st = cpssPxIngressHashSeedGet(dev, &crc32SeedGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. Call with NULL crc32SeedPtr
        Expected: GT_BAD_PTR. */
        st = cpssPxIngressHashSeedGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashSeedGet(dev, &crc32SeedGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashSeedGet(dev, &crc32SeedGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPortIndexModeSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        1.2. Call with invalid port number
        Expected: GT_BAD_PARAM.
        1.3. Call with invalid index mode
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum;
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexMode = CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E;
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexModeGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. ITERATE_DEVICE (Go over all Px devices) */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters and validate
            Expected: GT_OK. */
        for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            indexMode = (portNum % 2) ?
                CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E:
                CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E;
            st = cpssPxIngressHashPortIndexModeSet(dev, portNum, indexMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, indexMode);
        }
        /* Validate */
        for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            indexMode = (portNum % 2) ?
                CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E:
                CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E;
            st = cpssPxIngressHashPortIndexModeGet(dev, portNum, &indexModeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(indexMode, indexModeGet);
        }
        /*  1.2. Call with invalid port number
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashPortIndexModeSet(dev, portNum, indexMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum, indexMode);

        /*  1.3. Call with invalid hash packet type
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashPortIndexModeSet(dev, portNum - 1, 99);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum - 1, 99);
    }
    portNum = 0;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPortIndexModeSet(dev, portNum, indexMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum,
            indexMode);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashPortIndexModeSet(dev, portNum, indexMode);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum, indexMode);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPortIndexModeGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with invalid port number
        Expected: GT_BAD_PARAM.
        1.3. Call with NULL indexModePtr
        Expected: GT_BAD_PTR
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_PHYSICAL_PORT_NUM                        portNum;
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexModeGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. ITERATE_DEVICE (Go over all Px devices) */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            st = cpssPxIngressHashPortIndexModeGet(dev, portNum, &indexModeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
        }
        /*  1.2. Call with invalid port number
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashPortIndexModeGet(dev, portNum, &indexModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        portNum = 0;
        /*  1.3. Call with NULL indexModePtr
        Expected: GT_BAD_PTR */
        st = cpssPxIngressHashPortIndexModeGet(dev, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, portNum);
    }
    portNum = 0;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPortIndexModeGet(dev, portNum, &indexModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashPortIndexModeGet(dev, portNum, &indexModeGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashDesignatedPortsEntrySet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        1.2. Call with invalid entry index
        Expected: GT_BAD_PARAM.
        1.3. Call with invalid port bitmap
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    CPSS_PX_PORTS_BMP                           portsBmpGet;
    CPSS_PX_PORTS_BMP                           portsBmp[17];
    GT_U32                                      value = 1;
    GT_U32                                      ii;
    GT_U32                                      maxEntries;

    for(ii = 0; ii < 17; ii++)
    {
        portsBmp[ii] = value;
        value <<= 1;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. ITERATE_DEVICE (Go over all Px devices) */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        maxEntries =  (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev)) ? 
            CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS : 
            CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS/2;
        /*  1.1. Call with all valid parameters and validate
            Expected: GT_OK. */
        for(ii = 0; ii < maxEntries; ii++)
        {
            st = cpssPxIngressHashDesignatedPortsEntrySet(dev, ii,
                portsBmp[ii % 17]);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ii, portsBmp[ii % 17]);
        }
        /* Validate */
        for(ii = 0; ii < maxEntries; ii++)
        {
            st = cpssPxIngressHashDesignatedPortsEntryGet(dev, ii,
                &portsBmpGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);
            UTF_VERIFY_EQUAL0_PARAM_MAC(portsBmp[ii % 17], portsBmpGet);
        }

        ii = CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS;
        /*  1.2. Call with invalid entry index
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashDesignatedPortsEntrySet(dev, ii, portsBmp[0]);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, ii, portsBmp[0]);
        ii = 0;

        /*  1.3. Call with invalid port bitmap
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashDesignatedPortsEntrySet(dev, ii, 0x20000);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, ii, 0x20000);
    }
    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashDesignatedPortsEntrySet(dev, ii, portsBmp[0]);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, ii,
            portsBmp[0]);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashDesignatedPortsEntrySet(dev, ii, portsBmp[0]);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, ii, portsBmp[0]);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashDesignatedPortsEntryGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters
        Expected: GT_OK.
        1.2. Call with invalid entry index
        Expected: GT_BAD_PARAM.
        1.3. Call with NULL portsBmpPtr
        Expected: GT_BAD_PTR.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    CPSS_PX_PORTS_BMP                           portsBmpGet;
    GT_U32                                      ii;
    GT_U32                                      maxEntries;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. ITERATE_DEVICE (Go over all Px devices) */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        maxEntries =  (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev)) ? 
            CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS : 
            CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS/2;
        /*  1.1. Call with all valid parameters and validate
            Expected: GT_OK. */
        for(ii = 0; ii < maxEntries; ii++)
        {
            st = cpssPxIngressHashDesignatedPortsEntryGet(dev, ii,
                &portsBmpGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ii);
        }

        ii = CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS;
        /*  1.2. Call with invalid entry index
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashDesignatedPortsEntryGet(dev, ii, &portsBmpGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, ii);
        ii = 0;

        /*  1.3. Call with NULL portsBmpPtr
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressHashDesignatedPortsEntryGet(dev, ii, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, ii);
    }
    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashDesignatedPortsEntryGet(dev, 0, &portsBmpGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, 0);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashDesignatedPortsEntryGet(dev, 0, &portsBmpGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, 0);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashLagTableModeSet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        1.2  Call with wrong enum values.
        Expected: GT_BAD_PARAM.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT      tableMode;
    CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT      tableModeGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == 0)
        {
            /* Skip test for Pipe A0 devices */
            continue;
        }

        for (tableMode = CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E; 
              tableMode <= CPSS_PX_INGRESS_HASH_LAG_TABLE_DUAL_MODE_E; 
              tableMode++)
        {

            /*  1.1. Call with all valid parameters
                Expected: GT_OK. */
            st = cpssPxIngressHashLagTableModeSet(dev, tableMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tableMode);

            /* Validate */
            st = cpssPxIngressHashLagTableModeGet(dev, &tableModeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableMode, tableModeGet);

            /*  1.2  Call with wrong enum values.
                Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssPxIngressHashLagTableModeSet
                                (dev, tableMode), tableMode);
        }
    }

    tableMode = CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashLagTableModeSet(dev, tableMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            tableMode);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashLagTableModeSet(dev, tableMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tableMode);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashLagTableModeGet)
{
    /*
        1. ITERATE_DEVICE (Go over all Px devices)
        1.1. Call with all valid parameters and validate
        Expected: GT_OK.
        2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE.
        3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM.
    */
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT      tableMode;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == 0)
        {
            /* Skip test for Pipe A0 devices */
            continue;
        }

        /*  1.1. Call with all valid parameters
            Expected: GT_OK. */
        st = cpssPxIngressHashLagTableModeGet(dev, &tableMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    tableMode = CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E;

    /*  2. Call with not active and non-applicable devices
        Expected: GT_NOT_APPLICABLE_DEVICE. */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashLagTableModeGet(dev, &tableMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            tableMode);
    }

    /*  3. Call with out-of-bound value for device id
        Expected: GT_BAD_PARAM. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressHashLagTableModeGet(dev, &tableMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tableMode);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPacketTypeLagTableNumberSet)
{
/*
    ITERATE_DEVICE (All over all Px devices)
    1.1. Call with packetType[0..31], with valid enable.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.3 Call with lagTableNumber[2]
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType=0;
    GT_U32              lagTableNumber=0;
    GT_U32              lagTableNumberGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == 0)
        {
            /* Skip test for Pipe A0 devices */
            continue;
        }
        /*  1.1. Call with packetType[0..31], with valid lagTableNumber.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
        {
            st = cpssPxIngressHashPacketTypeLagTableNumberSet(dev, packetType, lagTableNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

            st = cpssPxIngressHashPacketTypeLagTableNumberGet(dev, packetType, &lagTableNumberGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(lagTableNumber, lagTableNumberGet,
                       "get another lagTableNumberGet than was set: %d", lagTableNumberGet);

            lagTableNumber++; 
            lagTableNumber %= 2;
        }

        /*  1.2 Call with out of range packetType[32] and other params from 1.1.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashPacketTypeLagTableNumberSet(dev, packetType, lagTableNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, packetType);

        /*  1.3 Call with lagTableNumber[2]
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashPacketTypeLagTableNumberSet(dev, packetType, 2);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, packetType);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    packetType = 0;
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPacketTypeLagTableNumberSet(dev, packetType, lagTableNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressHashPacketTypeLagTableNumberSet(dev, packetType, lagTableNumber);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssPxIngressHashPacketTypeLagTableNumberGet)
{
/*
    ITERATE_DEVICE (All over all Px devices)
    1.1. Call with packetType[0..31], with valid lagTableNumber.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType=0;
    GT_U32              lagTableNumber=0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == 0)
        {
            /* Skip test for Pipe A0 devices */
            continue;
        }
        /*  1.1. Call with packetType[0..31], with valid lagTableNumber.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
        {
            st = cpssPxIngressHashPacketTypeLagTableNumberGet(dev, packetType, &lagTableNumber);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);
        }

        /*  1.2 Call with out of range packetType[32] and other params from 1.1.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressHashPacketTypeLagTableNumberGet(dev, packetType, &lagTableNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, packetType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    packetType = 0;
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressHashPacketTypeLagTableNumberGet(dev, packetType, &lagTableNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressHashPacketTypeLagTableNumberGet(dev, packetType, &lagTableNumber);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxIngressHash suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxIngressHash)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashUdeEthertypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashUdeEthertypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPacketTypeEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPacketTypeEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPacketTypeHashModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPacketTypeHashModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashSeedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashSeedGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPortIndexModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPortIndexModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashDesignatedPortsEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashDesignatedPortsEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashLagTableModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashLagTableModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPacketTypeLagTableNumberSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressHashPacketTypeLagTableNumberGet)
UTF_SUIT_END_TESTS_MAC(cpssPxIngressHash)

