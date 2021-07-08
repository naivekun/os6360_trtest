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
* @file cpssDxChLpmUT.c
*
* @brief Unit tests for cpssDxChLpm that provides the CPSS DXCH LPM APIs
*
* @version   7
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLpmLeafEntryWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    IN CPSS_DXCH_LPM_LEAF_ENTRY_STC             *leafPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLpmLeafEntryWrite)
{
/*
    ITERATE_DEVICES (eArch devices only)
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = leavesNumber / 2, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = leavesNumber - 1, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = leavesNumber, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0, entryType = illegal enum value,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = nextHopsNumber / 2, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = nextHopsNumber - 1, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = nextHopsNumber, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
      index = ecmpEntriesNumber / 2, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
      index = ecmpEntriesNumber - 1, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_OK
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
      index = ecmpEntriesNumber, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E
      Expected: GT_OK
    - Call with leafIndex = 0, entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
      index = 0, ipv6MCGroupScopeLevel = illegal enum value
      Expected: GT_BAD_PARAM
*/
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          leafIndex;
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leaf, leafGet;
    GT_U32                          notAppFamilyBmp;
    GT_U32                          leavesNumber, nextHopsNumber, ecmpEntriesNumber;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        leavesNumber = PRV_CPSS_DXCH_PP_MAC(dev)->moduleCfg.ip.maxNumOfPbrEntries;
        nextHopsNumber = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;
        ecmpEntriesNumber = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.ecmpQos;

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */

        cpssOsMemSet(&leaf, 0, sizeof(CPSS_DXCH_LPM_LEAF_ENTRY_STC));

        leafIndex = 0;
        leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leaf.index = 0;
        leaf.ucRPFCheckEnable = GT_FALSE;
        leaf.sipSaCheckMismatchEnable = GT_FALSE;
        leaf.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           sipSaCheckMismatchEnable = GT_TRUE
           Expected: GT_OK */
        leafIndex = 0;
        leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leaf.index = 0;
        leaf.ucRPFCheckEnable = GT_FALSE;
        leaf.sipSaCheckMismatchEnable = GT_TRUE;
        leaf.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           ucRPFCheckEnable = GT_TRUE
           Expected: GT_OK */
        leafIndex = 0;
        leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leaf.index = 0;
        leaf.ucRPFCheckEnable = GT_TRUE;
        leaf.sipSaCheckMismatchEnable = GT_FALSE;
        leaf.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = leavesNumber / 2,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           leaf.ucRPFCheckEnable = GT_FALSE;
           Expected: GT_OK */
        leafIndex = leavesNumber / 2;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = leavesNumber - 1,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */
        leafIndex = leavesNumber - 1;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = leavesNumber,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_BAD_PARAM */
        leafIndex = leavesNumber;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */
        leafIndex = 0;
        leaf.entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        if PRV_CPSS_SIP_6_CHECK_MAC(dev)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);
        }

        if (!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                            "get another entryType than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                            "get another index than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                            "get another ucRPFCheckEnable than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                            "get another sipSaCheckMismatchEnable than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                            "get another ipv6MCGroupScopeLevel than was set: ", dev);
        }

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */
        leaf.entryType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        if PRV_CPSS_SIP_6_CHECK_MAC(dev)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);
        }

        if (!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                            "get another entryType than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                            "get another index than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                            "get another ucRPFCheckEnable than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                            "get another sipSaCheckMismatchEnable than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                            "get another ipv6MCGroupScopeLevel than was set: ", dev);
        }

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_BAD_PARAM */
        leaf.entryType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_BAD_PARAM */
        leaf.entryType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_BAD_PARAM */
        leaf.entryType = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);

        /* Call with
           leafIndex = 0,
           entryType = illegal enum value,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_BAD_PARAM */
        UTF_ENUMS_CHECK_MAC(cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf),
                            leaf.entryType);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = nextHopsNumber / 2,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */
        leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leaf.index = nextHopsNumber / 2;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = nextHopsNumber - 1,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */
        leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leaf.index = nextHopsNumber - 1;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = nextHopsNumber,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_BAD_PARAM */
        leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leaf.index = nextHopsNumber;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
           index = ecmpEntriesNumber / 2,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */
        leaf.entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
        leaf.index = ecmpEntriesNumber / 2;

        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        if PRV_CPSS_SIP_6_CHECK_MAC(dev)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);
        }

        if (!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);
        }

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
           index = ecmpEntriesNumber - 1,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_OK */
        leaf.entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
        leaf.index = ecmpEntriesNumber - 1;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        if PRV_CPSS_SIP_6_CHECK_MAC(dev)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);
        }

        if (!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                            "get another entryType than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                            "get another index than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                            "get another ucRPFCheckEnable than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                            "get another sipSaCheckMismatchEnable than was set: ", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                            "get another ipv6MCGroupScopeLevel than was set: ", dev);
        }

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E,
           index = ecmpEntriesNumber,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           Expected: GT_BAD_PARAM */
        leaf.entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
        leaf.index = ecmpEntriesNumber;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, leafIndex);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E
           Expected: GT_OK */
        leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
        leaf.index = 0;
        leaf.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leafGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, leafIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.entryType, leafGet.entryType,
                        "get another entryType than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.index, leafGet.index,
                        "get another index than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ucRPFCheckEnable, leafGet.ucRPFCheckEnable,
                        "get another ucRPFCheckEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.sipSaCheckMismatchEnable, leafGet.sipSaCheckMismatchEnable,
                        "get another sipSaCheckMismatchEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(leaf.ipv6MCGroupScopeLevel, leafGet.ipv6MCGroupScopeLevel,
                        "get another ipv6MCGroupScopeLevel than was set: ", dev);

        /* Call with
           leafIndex = 0,
           entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E,
           index = 0,
           ipv6MCGroupScopeLevel = illegal enum value
           Expected: GT_BAD_PARAM */
        UTF_ENUMS_CHECK_MAC(cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf),
                            leaf.ipv6MCGroupScopeLevel);
    }

    leafIndex = 0;
    leaf.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    leaf.index = 0;
    leaf.ucRPFCheckEnable = GT_FALSE;
    leaf.sipSaCheckMismatchEnable = GT_FALSE;
    leaf.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    /* For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmLeafEntryWrite(dev, leafIndex, &leaf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLpmLeafEntryRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  leafIndex,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLpmLeafEntryRead)
{
    /*
    ITERATE_DEVICES (eArch devices only)
    - Call with leafIndex = 0 and leafPtr != NULL
      Expected: GT_OK
    - Call with leafIndex = leavesNumber / 2 and leafPtr != NULL
      Expected: GT_OK
    - Call with leafIndex = leavesNumber - 1 and leafPtr != NULL
      Expected: GT_OK
    - Call with leafIndex = leavesNumber and leafPtr != NULL
      Expected: GT_BAD_PARAM
    - Call with leafIndex = 0 and leafPtr == NULL
      Expected: GT_BAD_PTR
    */
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          leafIndex;
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leaf;
    GT_U32                          notAppFamilyBmp;
    GT_U32                          leavesNumber;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        leavesNumber = PRV_CPSS_DXCH_PP_MAC(dev)->moduleCfg.ip.maxNumOfPbrEntries;
        leafIndex = 0;
        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        leafIndex = leavesNumber / 2;
        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        leafIndex = leavesNumber - 1;
        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        leafIndex = leavesNumber;
        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        leafIndex = 0;
        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    leafIndex = 0;
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leaf);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmLeafEntryRead(dev, leafIndex, &leaf);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLpmPortSipLookupEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChLpmPortSipLookupEnableSet)
{
/*
    1.1.1. Call with enable [GT_TRUE/ GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChLpmPortSipLookupEnableSet with  non-NULL enablePtr.
    Expected: GT_OK  and the same enable.
*/
    GT_STATUS    st   = GT_OK;
    GT_U8        dev;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE/ GT_FALSE].
                Expected: GT_OK.
            */

            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChLpmPortSipLookupEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChLpmPortSipLookupEnableGet with  non-NULL enablePtr.
                Expected: GT_OK  and the same enable.
            */
            st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChLpmPortSipLookupEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable value than was set: %d, %d", dev, port);

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChLpmPortSipLookupEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChLpmPortSipLookupEnableGet with  non-NULL enablePtr.
                Expected: GT_OK  and the same enable.
            */
            st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChLpmPortSipLookupEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable value than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLpmPortSipLookupEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChLpmPortSipLookupEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChLpmPortSipLookupEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_TRUE;
    port   = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
        /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmPortSipLookupEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmPortSipLookupEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChLpmPortSipLookupEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChLpmPortSipLookupEnableGet)
{
/*
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS    st   = GT_OK;
    GT_U8        dev;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable = GT_FALSE;
    GT_U32      notAppFamilyBmp;

     /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChLpmPortSipLookupEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmPortSipLookupEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChLpmLastLookupStagesBypassEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChLpmLastLookupStagesBypassEnableSet with relevant value
              enable[GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChLpmLastLookupStagesBypassEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL     defaultSatus;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
         /* keep default for restore */
        st = cpssDxChLpmLastLookupStagesBypassEnableGet(dev, &defaultSatus);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChLpmLastLookupStagesBypassEnableGet: %d", dev);


        /*  1.1.  */
        enable = GT_FALSE;

        st = cpssDxChLpmLastLookupStagesBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChLpmLastLookupStagesBypassEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChLpmLastLookupStagesBypassEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChLpmLastLookupStagesBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChLpmLastLookupStagesBypassEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChLpmLastLookupStagesBypassEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /* restore */
        st = cpssDxChLpmLastLookupStagesBypassEnableSet(dev, defaultSatus);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChLpmLastLookupStagesBypassEnableSet: %d", dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmLastLookupStagesBypassEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmLastLookupStagesBypassEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChLpmLastLookupStagesBypassEnableGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChLpmLastLookupStagesBypassEnableGet with not NULL enablePtr.
    Expected: GT_OK.
    1.3. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChLpmLastLookupStagesBypassEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChLpmLastLookupStagesBypassEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmLastLookupStagesBypassEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmLastLookupStagesBypassEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChLpmExceptionStatusGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChLpmExceptionStatusGet with not NULL hitExceptionPtr.
    Expected: GT_OK.
    1.2. Call with NULL hitExceptionPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with lpmEngine out of range .
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT  hitException=CPSS_DXCH_LPM_EXCEPTION_HIT_E;
    GT_U32      num_lpmEngine;
    GT_U8       i;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        num_lpmEngine =
            PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ?
                4 :
            PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E ?
                PRV_CPSS_PP_MAC(dev)->portGroupsInfo.numOfPortGroups :
            PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ?
                2 :
                1 ;

       for(i = 0 ; i < num_lpmEngine ; i++)
       {
            /*  1.1.  */
            st = cpssDxChLpmExceptionStatusGet(dev,i, &hitException);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
       }

        /*  1.2.  */
        st = cpssDxChLpmExceptionStatusGet(dev,0, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChLpmExceptionStatusGet(dev,num_lpmEngine, &hitException);
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmExceptionStatusGet(dev,0,&hitException);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmExceptionStatusGet(dev, 0,&hitException);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChLpmPbrBaseAndSizeGet)
{
/*
    ITERATE_DEVICES
    Call with legal parameters. Check that size is as configured by appDemo.
    Expected: GT_OK.
    Call cpssDxChLpmPbrBaseAndSizeGet with not NULL baseOffsetPtr.
    Expected: GT_BAD_PTR.
    Call cpssDxChLpmPbrBaseAndSizeGet with not NULL sizePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      baseOffset;
    GT_U32      size;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChLpmPbrBaseAndSizeGet(dev, &baseOffset, &size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) {        
            /* size is configured in getPpLogicalInitParamsSimple */
            UTF_VERIFY_EQUAL1_PARAM_MAC(1664*4, size, dev);
        }
        else{
            UTF_VERIFY_EQUAL1_PARAM_MAC(_8K, size, dev);
        }

        st = cpssDxChLpmPbrBaseAndSizeGet(dev, NULL, &size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChLpmPbrBaseAndSizeGet(dev, &baseOffset, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns error.                               */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmPbrBaseAndSizeGet(dev, &baseOffset, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmPbrBaseAndSizeGet(dev, &baseOffset, NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


UTF_TEST_CASE_MAC(cpssDxChLpmMemoryModeGet)
{
/*
    ITERATE_DEVICES
    Call with legal parameters. Check that size is as configured by appDemo.
    Expected: GT_OK.
    Call cpssDxChLpmMemoryModeGet with  NULL lpmMemoryMode.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT    lpmMemoryMode;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev,(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E)) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(dev) != GT_FALSE)
        {
            st = cpssDxChLpmMemoryModeGet(dev, &lpmMemoryMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* mode  is configured in appDemoBc2IpLpmRamDefaultConfigCalc */
            UTF_VERIFY_EQUAL1_PARAM_MAC(CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E, lpmMemoryMode, dev);

            st = cpssDxChLpmMemoryModeGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        else
        {
            st = cpssDxChLpmMemoryModeGet(dev, &lpmMemoryMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);
        }

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns error.                               */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev,(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLpmMemoryModeGet(dev, &lpmMemoryMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChLpmMemoryModeGet(dev, &lpmMemoryMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChLpm suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChLpm)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmLeafEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmLeafEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmPortSipLookupEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmPortSipLookupEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmLastLookupStagesBypassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmLastLookupStagesBypassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmExceptionStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmPbrBaseAndSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLpmMemoryModeGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChLpm)


