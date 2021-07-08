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
* @file cpssDxChIpUT.c
*
* @brief Unit tests for cpssDxChIp, that provides
* The CPSS DXCH Ip HW structures APIs
*
* @version   53
********************************************************************************
*/
/* includes */

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpNat.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Tests use this vlan id for testing VLAN functions */
#define IP_VLAN_TESTED_VLAN_ID_CNS 100


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv4PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  GT_U32                      routerTtiTcamColumn,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv4PrefixSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with routerTtiTcamRow [0 / 1],
                   routerTtiTcamColumn [1 / 2],
                   prefixPtr {vrId [100],
                              ipAddr [{200, 200, 10, 1 / 2}],
                              isMcSource [GT_FALSE / GT_TRUE],
                              mcGroupIndexRow [1],
                              mcGroupIndexColumn [3]}.
    Expected: GT_OK.
    1.2. Call cpssDxChIpv4PrefixGet with the same routerTtiTcamRow,
         routerTtiTcamColumn and non-NULL pointers.
    Expected: GT_OK, valid == GT_TRUE and the same values as was set.
    1.2. Call cpssDxChIpv4PrefixGet with the same routerTtiTcamRow,
         routerTtiTcamColumn and non-NULL pointers.
    Expected: GT_OK, valid == GT_TRUE and the same values as was set.
    1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with routerTtiTcamColumn [0 / 3]
    (Ch2 mc source can reside only on second and third columns)
    and other params from 1.1.
    Expected: GT_OK for DxCh3 and above, NOT GT_OK for DxCh2.
    1.6. Call with out of range routerTtiTcamColumn [5] and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with prefixPtr->isMcSource [GT_FALSE], out of range vrId [4096],
         and other params from 1.1.
    Expected: GT_OK for DxCh2 (not supported vrId) NOT GT_OK for DxCh3 and above.
    1.8. Call with prefixPtr->isMcSource [GT_TRUE],
                   out of range prefixPtr->mcGroupIndexRow [maxTtiTcamRow(dev)]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.9. Call with prefixPtr->isMcSource [GT_TRUE],
                   out of range prefixPtr->mcGroupIndexColumn [4]
                   and other params from 1.1.
    Expected: GT_OK for DxCh2 and NOT GT_OK for DxCh3 and above.
    1.10. Call with prefixPtr->isMcSource [GT_TRUE],
                   out of range prefixPtr->mcGroupIndexColumn [5]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.11. Call with prefixPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.12. Call with maskPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.13. Call cpssDxChIpv4PrefixInvalidate with routerTtiTcamRow [0 / 1],
                                                 routerTtiTcamColumn [1 / 2].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                      routerTtiTcamRow    = 0;
    GT_U32                      routerTtiTcamColumn = 0;
    CPSS_DXCH_IPV4_PREFIX_STC   prefix;
    CPSS_DXCH_IPV4_PREFIX_STC   mask;
    CPSS_DXCH_IPV4_PREFIX_STC   prefixGet;
    CPSS_DXCH_IPV4_PREFIX_STC   maskGet;
    GT_BOOL                     isEqual   = GT_FALSE;
    GT_BOOL                     isValid   = GT_FALSE;
    GT_U32                      maxTtiRow = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily = CPSS_MAX_FAMILY;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*) &prefix, sizeof(prefix));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        maxTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        /*
            1.1. Call with routerTtiTcamRow [0 / 1],
                           routerTtiTcamColumn [1 / 2],
                           prefixPtr {vrId [100],
                                      ipAddr [{200, 200, 10, 1 / 2}],
                                      isMcSource [GT_FALSE / GT_TRUE],
                                      mcGroupIndexRow [1],
                                      mcGroupIndexColumn [3]}.
            Expected: GT_OK.
        */
        cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));

        /* Call with routerTtiTcamRow [0] */
        routerTtiTcamRow    = 0;
        routerTtiTcamColumn = 1;

        prefix.vrId       = 100;
        prefix.isMcSource = GT_FALSE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 1;

        mask.isMcSource = GT_FALSE;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow, routerTtiTcamColumn);

        /*
            1.2. Call cpssDxChIpv4PrefixGet with the same routerTtiTcamRow,
                 routerTtiTcamColumn and non-NULL pointers.
            Expected: GT_OK, valid == GT_TRUE and the same values as was set.
        */
        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                   &isValid, &prefixGet, &maskGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpv4PrefixGet: %d, %d, %d",
                                     dev, routerTtiTcamRow, routerTtiTcamColumn);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isValid,
                   "cpssDxChIpv4PrefixGet: get valid == GT_FALSE: %d", dev);

        if (GT_TRUE == isValid)
        {
            if (CPSS_PP_FAMILY_CHEETAH2_E != devFamily)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prefix.vrId, prefixGet.vrId,
                           "get another prefixPtr->vrId than was set: %d", dev);
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(prefix.isMcSource, prefixGet.isMcSource,
                       "get another prefixPtr->isMcSource than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(prefix.ipAddr),
                                         (GT_VOID*) &(prefixGet.ipAddr),
                                         sizeof(prefix.ipAddr))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another prefixPtr->ipAddr than was set: %d", dev);

            isEqual = (mask.ipAddr.u32Ip == maskGet.ipAddr.u32Ip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another mask than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mask.isMcSource, maskGet.isMcSource,
                       "get another prefixPtr->isMcSource than was set: %d", dev);
        }

        /* Call with routerTtiTcamRow [1] */
        routerTtiTcamRow    = 1;
        routerTtiTcamColumn = 2;

        prefix.isMcSource = GT_TRUE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 2;

        prefix.mcGroupIndexRow    = 1;
        prefix.mcGroupIndexColumn = 3;

        mask.isMcSource = GT_TRUE;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow, routerTtiTcamColumn);

        /*
            1.2. Call cpssDxChIpv4PrefixGet with the same routerTtiTcamRow,
                 routerTtiTcamColumn and non-NULL pointers.
            Expected: GT_OK, valid == GT_TRUE and the same values as was set.
        */
        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                    &isValid, &prefixGet, &maskGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpv4PrefixGet: %d, %d, %d",
                                     dev, routerTtiTcamRow, routerTtiTcamColumn);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isValid,
                   "cpssDxChIpv4PrefixGet: get valid == GT_FALSE: %d", dev);

        if (GT_TRUE == isValid)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prefix.mcGroupIndexRow, prefixGet.mcGroupIndexRow,
                       "get another prefixPtr->mcGroupIndexRow than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(prefix.mcGroupIndexColumn, prefixGet.mcGroupIndexColumn,
                       "get another prefixPtr->mcGroupIndexColumn than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(prefix.isMcSource, prefixGet.isMcSource,
                       "get another prefixPtr->isMcSource than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(prefix.ipAddr),
                                         (GT_VOID*) &(prefixGet.ipAddr),
                                         sizeof(prefix.ipAddr))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another prefixPtr->ipAddr than was set: %d", dev);

            isEqual = ((mask.ipAddr.u32Ip == maskGet.ipAddr.u32Ip) &&
                       ((mask.mcGroupIndexRow & 0x3ff) == (maskGet.mcGroupIndexRow & 0x3ff)) &&
                       ((mask.mcGroupIndexColumn & 0x3) == (maskGet.mcGroupIndexColumn & 0x3)))
                                                                ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another mask than was set: %d", dev);
        }

        /*
            1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamRow = maxTtiRow;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        routerTtiTcamRow = 0;

        /*
            1.4. Call with routerTtiTcamColumn [0 / 3]
            (Ch2 mc source can reside only on second and third columns)
            and other params from 1.1.
            Expected: GT_OK for DxCh3 and above, NOT GT_OK for DxCh2.
        */
        /* Call with routerTtiTcamColumn [0] */
        routerTtiTcamColumn = 0;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        if (CPSS_PP_FAMILY_CHEETAH2_E == devFamily)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);
        }

        /* Call with routerTtiTcamColumn [3] */
        routerTtiTcamColumn = 3;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        if (CPSS_PP_FAMILY_CHEETAH2_E == devFamily)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                         dev, routerTtiTcamColumn);
        }

        /*
            1.6. Call with out of range routerTtiTcamColumn [5] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamColumn = 5;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);

        routerTtiTcamColumn = 1;

        /*
            1.7. Call with prefixPtr->isMcSource [GT_FALSE], out of range vrId [4096],
                 and other params from 1.1.
            Expected: GT_OK for DxCh2 (not supported vrId) NOT GT_OK for DxCh3 and above.
        */
        prefix.isMcSource = GT_FALSE;
        mask.isMcSource   = GT_FALSE;
        prefix.vrId       = 4096;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        if (CPSS_PP_FAMILY_CHEETAH2_E == devFamily)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, ->isMcSource = %d, ->vrId = %d",
                                         dev, prefix.isMcSource, prefix.vrId);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, ->isMcSource = %d, ->vrId = %d",
                                             dev, prefix.isMcSource, prefix.vrId);
        }

        prefix.vrId = 100;

        /*
            1.8. Call with prefixPtr->isMcSource [GT_TRUE],
                           out of range prefixPtr->mcGroupIndexRow [maxTtiTcamRow(dev)]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        prefix.isMcSource      = GT_TRUE;
        mask.isMcSource        = GT_TRUE;
        prefix.mcGroupIndexRow = maxTtiRow;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, ->isMcSource = %d, ->mcGroupIndexRow = %d",
                                         dev, prefix.isMcSource, prefix.mcGroupIndexRow);

        prefix.mcGroupIndexRow = 0;

        /*
            1.9. Call with prefixPtr->isMcSource [GT_TRUE],
                           out of range prefixPtr->mcGroupIndexColumn [4]
                           and other params from 1.1.
            Expected: GT_OK for DxCh2 and NOT GT_OK for DxCh3 and above.
        */
        prefix.mcGroupIndexColumn = 4;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        if (CPSS_PP_FAMILY_CHEETAH2_E == devFamily)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, ->isMcSource = %d, ->mcGroupIndexColumn = %d",
                              dev, prefix.isMcSource, prefix.mcGroupIndexColumn);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, ->isMcSource = %d, ->mcGroupIndexColumn = %d",
                              dev, prefix.isMcSource, prefix.mcGroupIndexColumn);
        }

        /*
            1.10. Call with prefixPtr->isMcSource [GT_TRUE],
                           out of range prefixPtr->mcGroupIndexColumn [5]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        prefix.mcGroupIndexColumn = 5;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, ->isMcSource = %d, ->mcGroupIndexColumn = %d",
                                         dev, prefix.isMcSource, prefix.mcGroupIndexColumn);

        prefix.mcGroupIndexColumn = 1;

        /*
            1.11. Call with prefixPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, NULL, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, prefixPtr = NULL", dev);

        /*
            1.12. Call with maskPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*
            1.13. Call cpssDxChIpv4PrefixInvalidate with routerTtiTcamRow [0 / 1],
                                                         routerTtiTcamColumn [1 / 2].
            Expected: GT_OK.
        */

        /* Call with routerTtiTcamRow [0] */
        routerTtiTcamRow    = 0;
        routerTtiTcamColumn = 1;

        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpv4PrefixInvalidate: %d, %d, %d",
                                     dev, routerTtiTcamRow, routerTtiTcamColumn);

        /* Call with routerTtiTcamRow [1] */
        routerTtiTcamRow    = 1;
        routerTtiTcamColumn = 2;

        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpv4PrefixInvalidate: %d, %d, %d",
                                     dev, routerTtiTcamRow, routerTtiTcamColumn);
    }

    cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));

    routerTtiTcamRow    = 0;
    routerTtiTcamColumn = 1;

    prefix.vrId       = 100;
    prefix.isMcSource = GT_FALSE;

    prefix.ipAddr.arIP[0] = 200;
    prefix.ipAddr.arIP[1] = 200;
    prefix.ipAddr.arIP[2] = 10;
    prefix.ipAddr.arIP[3] = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv4PrefixGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      routerTtiTcamRow,
    IN  GT_U32      routerTtiTcamColumn,
    OUT GT_BOOL     *validPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv4PrefixGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Set Ipv4 prefix for testing.
         Call cpssDxChIpv4PrefixSet with routerTtiTcamRow [0],
                                         routerTtiTcamColumn [1],
                                         prefixPtr {vrId [100],
                                                    ipAddr [{200, 200, 10, 1}],
                                                    isMcSource [GT_FALSE]}.
    Expected: GT_OK.
    1.2. Call with routerTtiTcamRow [0], routerTtiTcamColumn [1] and non-NULL pointers.
    Expected: GT_OK.
    1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range routerTtiTcamColumn [4]
                   and other params from 1.1.
    Expected: GT_OK for DxCh2 and NOT GT_OK for DxCh3.
    1.5. Call with out of range routerTtiTcamColumn [5]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with validPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call with prefixPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.8. Call with maskPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.9. Call cpssDxChIpv4PrefixInvalidate with routerTtiTcamRow [0],
                                                 routerTtiTcamColumn [1].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                      routerTtiTcamRow    = 0;
    GT_U32                      routerTtiTcamColumn = 0;
    CPSS_DXCH_IPV4_PREFIX_STC   prefix;
    CPSS_DXCH_IPV4_PREFIX_STC   mask;
    GT_BOOL                     isValid   = GT_FALSE;
    GT_U32                      maxTtiRow = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily = CPSS_MAX_FAMILY;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*) &prefix, sizeof(prefix));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        maxTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        /*
            1.1. Set Ipv4 prefix for testing.
                 Call cpssDxChIpv4PrefixSet with routerTtiTcamRow [0],
                                                 routerTtiTcamColumn [1],
                                                 prefixPtr {vrId [100],
                                                            ipAddr [{200, 200, 10, 1}],
                                                            isMcSource [GT_FALSE]}.
            Expected: GT_OK.
        */
        cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));

        routerTtiTcamRow    = 0;
        routerTtiTcamColumn = 1;

        prefix.vrId       = 100;
        prefix.isMcSource = GT_FALSE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 1;

        mask.isMcSource = GT_FALSE;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpv4PrefixSet: %d, %d, %d",
                                     dev, routerTtiTcamRow, routerTtiTcamColumn);

        /*
            1.2. Call with routerTtiTcamRow [0], routerTtiTcamColumn [1] and non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                   &isValid, &prefix, &mask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow, routerTtiTcamColumn);

        /*
            1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamRow = maxTtiRow;

        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                   &isValid, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        routerTtiTcamRow = 0;

        /*
            1.4. Call with out of range routerTtiTcamColumn [4]
                           and other params from 1.1.
            Expected: GT_OK for DxCh2 and NOT GT_OK for DxCh3.
        */
        routerTtiTcamColumn = 4;

        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                   &isValid, &prefix, &mask);
        if (CPSS_PP_FAMILY_CHEETAH2_E == devFamily)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                         dev, routerTtiTcamColumn);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);
        }

        /*
            1.5. Call with out of range routerTtiTcamColumn [5]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamColumn = 5;

        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                   &isValid, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);

        routerTtiTcamColumn = 1;

        /*
            1.6. Call with validPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                    NULL, &prefix, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, validPtr = NULL", dev);

        /*
            1.7. Call with prefixPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                    &isValid, NULL, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, prefixPtr = NULL", dev);

        /*
            1.8. Call with maskPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                    &isValid, &prefix, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*
            1.9. Call cpssDxChIpv4PrefixInvalidate with routerTtiTcamRow [0],
                                                         routerTtiTcamColumn [1].
            Expected: GT_OK.
        */
        routerTtiTcamRow    = 0;
        routerTtiTcamColumn = 1;

        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpv4PrefixInvalidate: %d, %d, %d",
                                     dev, routerTtiTcamRow, routerTtiTcamColumn);
    }

    routerTtiTcamRow    = 0;
    routerTtiTcamColumn = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                    &isValid, &prefix, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv4PrefixGet(dev, routerTtiTcamRow, routerTtiTcamColumn,
                                &isValid, &prefix, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv4PrefixInvalidate
(
    IN  GT_U8       devNum,
    IN  GT_U32      routerTtiTcamRow,
    IN  GT_U32      routerTtiTcamColumn
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv4PrefixInvalidate)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Set Ipv4 prefix for testing.
         Call cpssDxChIpv4PrefixSet with routerTtiTcamRow [0],
                                         routerTtiTcamColumn [1],
                                         prefixPtr {vrId [100],
                                                    ipAddr [{200, 200, 10, 1}],
                                                    isMcSource [GT_FALSE]}.
    Expected: GT_OK.
    1.2. Call with routerTtiTcamRow [0], routerTtiTcamColumn [1].
    Expected: GT_OK.
    1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
         and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range routerTtiTcamColumn [4] and other params from 1.1.
    Expected: GT_OK for DxCh2 and NOT GT_OK for DxCh3 and above.
    1.5. Call with out of range routerTtiTcamColumn [5] and other params from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                      routerTtiTcamRow    = 0;
    GT_U32                      routerTtiTcamColumn = 0;
    GT_U32                      maxTtiRow           = 0;
    CPSS_DXCH_IPV4_PREFIX_STC   prefix;
    CPSS_DXCH_IPV4_PREFIX_STC   mask;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily           = CPSS_MAX_FAMILY;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*) &prefix, sizeof(prefix));
    cpssOsBzero((GT_VOID*) &mask, sizeof(mask));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        maxTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        /*
            1.1. Set Ipv4 prefix for testing.
                 Call cpssDxChIpv4PrefixSet with routerTtiTcamRow [0],
                                                 routerTtiTcamColumn [1],
                                                 prefixPtr {vrId [100],
                                                            ipAddr [{200, 200, 10, 1}],
                                                            isMcSource [GT_FALSE]}.
            Expected: GT_OK.
        */
        cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));

        routerTtiTcamRow    = 0;
        routerTtiTcamColumn = 1;

        prefix.vrId       = 100;
        prefix.isMcSource = GT_FALSE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 1;

        mask.isMcSource = GT_FALSE;

        st = cpssDxChIpv4PrefixSet(dev, routerTtiTcamRow, routerTtiTcamColumn, &prefix, &mask);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpv4PrefixSet: %d, %d, %d",
                                     dev, routerTtiTcamRow, routerTtiTcamColumn);

        /*
            1.2. Call with routerTtiTcamRow [0], routerTtiTcamColumn [1].
            Expected: GT_OK.
        */
        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow, routerTtiTcamColumn);

        /*
            1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                 and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamRow = maxTtiRow;

        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        routerTtiTcamRow = 0;

        /*
            1.4. Call with out of range routerTtiTcamColumn [4] and other params from 1.1.
            Expected: GT_OK for DxCh2 and NOT GT_OK for DxCh3 and above.
        */
        routerTtiTcamColumn = 4;

        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        if (CPSS_PP_FAMILY_CHEETAH2_E == devFamily)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                         dev, routerTtiTcamColumn);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);
        }

        /*
            1.5. Call with out of range routerTtiTcamColumn [5] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamColumn = 5;

        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routerTtiTcamColumn = %d",
                                             dev, routerTtiTcamColumn);
        routerTtiTcamColumn = 0;
    }

    routerTtiTcamRow    = 0;
    routerTtiTcamColumn = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv4PrefixInvalidate(dev, routerTtiTcamRow, routerTtiTcamColumn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6PrefixSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with routerTtiTcamRow [0 / 1],
                   prefixPtr {vrId [100],
                              ipAddr [{200, 200, 10, 1 / 2}],
                              isMcSource [GT_FALSE / GT_TRUE],
                              mcGroupIndexRow [0 / 1]}.
    Expected: GT_OK.
    1.2. Call cpssDxChIpv6PrefixGet with the same routerTtiTcamRow and non-NULL pointers.
    Expected: GT_OK, valid == GT_TRUE and the same values as was set.
    1.2. Call cpssDxChIpv6PrefixGet with the same routerTtiTcamRow and non-NULL pointers.
    Expected: GT_OK, valid == GT_TRUE and the same values as was set.
    1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with prefixPtr->isMcSource [GT_FALSE],
         out of range vrId [4096], and other params from 1.1.
    Expected: GT_OK for DxCh2 (not supported vrId) NOT GT_OK for DxCh3 and above.
    1.5. Call with prefixPtr->isMcSource [GT_TRUE],
                   out of range prefixPtr->mcGroupIndexRow [maxTtiTcamRow(dev)]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with prefixPtr [NULL]
                    and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call with maskPtr [NULL]
                    and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.8. Call cpssDxChIpv6PrefixInvalidate with routerTtiTcamRow [0 / 1].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                      routerTtiTcamRow = 0;
    CPSS_DXCH_IPV6_PREFIX_STC   prefix;
    CPSS_DXCH_IPV6_PREFIX_STC   mask;
    CPSS_DXCH_IPV6_PREFIX_STC   prefixGet;
    CPSS_DXCH_IPV6_PREFIX_STC   maskGet;
    GT_BOOL                     isEqual   = GT_FALSE;
    GT_BOOL                     isValid   = GT_FALSE;
    GT_U32                      maxTtiRow = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily = CPSS_MAX_FAMILY;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&mask, sizeof(mask));
    cpssOsBzero((GT_VOID*)&maskGet, sizeof(maskGet));
    cpssOsBzero((GT_VOID*)&prefix, sizeof(prefix));
    cpssOsBzero((GT_VOID*)&prefixGet, sizeof(prefixGet));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        maxTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        /*
            1.1. Call with routerTtiTcamRow [0 / 1],
                           prefixPtr {vrId [100],
                                      ipAddr [{200, 200, 10, 1 / 2}],
                                      isMcSource [GT_FALSE / GT_TRUE],
                                      mcGroupIndexRow [0 / 1]}.
            Expected: GT_OK.
        */
        cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));
        cpssOsMemSet((GT_VOID*) &maskGet, 0xFF, sizeof(maskGet));
        cpssOsMemSet((GT_VOID*) &prefix, 0, sizeof(prefix));
        cpssOsMemSet((GT_VOID*) &prefixGet, 0, sizeof(prefixGet));


        /* Call with routerTtiTcamRow [0] */
        routerTtiTcamRow = 0;

        prefix.vrId       = 100;
        prefix.isMcSource = GT_FALSE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 1;

        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        /*
            1.2. Call cpssDxChIpv6PrefixGet with the same routerTtiTcamRow and non-NULL pointers.
            Expected: GT_OK, valid == GT_TRUE and the same values as was set.
        */
        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, &prefixGet, &maskGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpv6PrefixGet: %d, %d",
                                     dev, routerTtiTcamRow);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isValid,
                   "cpssDxChIpv4PrefixGet: get valid == GT_FALSE: %d", dev);

        if (GT_TRUE == isValid)
        {
            if (CPSS_PP_FAMILY_CHEETAH2_E != devFamily)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prefix.vrId, prefixGet.vrId,
                           "get another prefixPtr->vrId than was set: %d", dev);
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(prefix.isMcSource, prefixGet.isMcSource,
                       "get another prefixPtr->isMcSource than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(prefix.ipAddr),
                                         (GT_VOID*) &(prefixGet.ipAddr),
                                         sizeof(prefix.ipAddr))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another prefixPtr->ipAddr than was set: %d", dev);

            isEqual = ((mask.ipAddr.u32Ip[0] == maskGet.ipAddr.u32Ip[0]) &&
                       (mask.ipAddr.u32Ip[1] == maskGet.ipAddr.u32Ip[1]) &&
                       (mask.ipAddr.u32Ip[2] == maskGet.ipAddr.u32Ip[2]) &&
                       (mask.ipAddr.u32Ip[3] == maskGet.ipAddr.u32Ip[3])) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another mask than was set: %d", dev);
        }

        /* Call with routerTtiTcamRow [1] */
        routerTtiTcamRow = 1;

        prefix.isMcSource = GT_TRUE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 2;

        prefix.mcGroupIndexRow = 1;

        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        /*
            1.2. Call cpssDxChIpv6PrefixGet with the same routerTtiTcamRow and non-NULL pointers.
            Expected: GT_OK, valid == GT_TRUE and the same values as was set.
        */
        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, &prefixGet, &maskGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpv6PrefixGet: %d, %d, %d",
                                     dev, routerTtiTcamRow);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isValid,
                   "cpssDxChIpv4PrefixGet: get valid == GT_FALSE: %d", dev);

        if (GT_TRUE == isValid)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prefix.mcGroupIndexRow, prefixGet.mcGroupIndexRow,
                       "get another prefixPtr->mcGroupIndexRow than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(prefix.isMcSource, prefixGet.isMcSource,
                       "get another prefixPtr->isMcSource than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(prefix.ipAddr),
                                         (GT_VOID*) &(prefixGet.ipAddr),
                                         sizeof(prefix.ipAddr))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another prefixPtr->ipAddr than was set: %d", dev);

            isEqual = ((mask.ipAddr.u32Ip[0] == maskGet.ipAddr.u32Ip[0]) &&
                       (mask.ipAddr.u32Ip[1] == maskGet.ipAddr.u32Ip[1]) &&
                       (mask.ipAddr.u32Ip[2] == maskGet.ipAddr.u32Ip[2]) &&
                       (mask.ipAddr.u32Ip[3] == maskGet.ipAddr.u32Ip[3]) &&
                       ((mask.mcGroupIndexRow & 0x1fff) == (maskGet.mcGroupIndexRow & 0x1fff)))
                                                    ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another mask than was set: %d", dev);
        }

        /*
            1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamRow = maxTtiRow;

        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        routerTtiTcamRow = 0;

        /*
            1.4. Call with prefixPtr->isMcSource [GT_FALSE],
                 out of range vrId [4096], and other params from 1.1.
            Expected: GT_OK for DxCh2 (not supported vrId) NOT GT_OK for DxCh3 and above.
        */
        prefix.isMcSource = GT_FALSE;
        prefix.vrId       = 4096;

        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        if (CPSS_PP_FAMILY_CHEETAH2_E == devFamily)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, ->isMcSource = %d, ->vrId = %d",
                                         dev, prefix.isMcSource, prefix.vrId);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, ->isMcSource = %d, ->vrId = %d",
                                             dev, prefix.isMcSource, prefix.vrId);
        }

        prefix.vrId = 100;

        /*
            1.5. Call with prefixPtr->isMcSource [GT_TRUE],
                           out of range prefixPtr->mcGroupIndexRow [maxTtiTcamRow(dev)]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        prefix.isMcSource      = GT_TRUE;
        prefix.mcGroupIndexRow = maxTtiRow;

        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, ->isMcSource = %d, ->mcGroupIndexRow = %d",
                        dev, prefix.isMcSource, prefix.mcGroupIndexRow);

        prefix.mcGroupIndexRow = 0;

        /*
            1.6. Call with prefixPtr [NULL]
                            and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, NULL, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, prefixPtr = NULL", dev);

        /*
            1.7. Call with maskPtr [NULL]
                            and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*
            1.8. Call cpssDxChIpv6PrefixInvalidate with routerTtiTcamRow [0 / 1].
            Expected: GT_OK.
        */

        /* Call with routerTtiTcamRow [0] */
        routerTtiTcamRow = 0;

        st = cpssDxChIpv6PrefixInvalidate(dev, routerTtiTcamRow);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpv6PrefixInvalidate: %d, %d, %d",
                                     dev, routerTtiTcamRow);

        /* Call with routerTtiTcamRow [1] */
        routerTtiTcamRow = 1;

        st = cpssDxChIpv6PrefixInvalidate(dev, routerTtiTcamRow);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpv6PrefixInvalidate: %d, %d, %d",
                                     dev, routerTtiTcamRow);
    }

    cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));

    routerTtiTcamRow = 0;

    prefix.vrId       = 100;
    prefix.isMcSource = GT_FALSE;

    prefix.ipAddr.arIP[0] = 200;
    prefix.ipAddr.arIP[1] = 200;
    prefix.ipAddr.arIP[2] = 10;
    prefix.ipAddr.arIP[3] = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6PrefixGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    OUT GT_BOOL                     *validPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6PrefixGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Set Ipv6 prefix for testing.
         Call cpssDxChIpv6PrefixSet with routerTtiTcamRow [0],
                                         prefixPtr {vrId [100],
                                                    ipAddr [{200, 200, 10, 1}],
                                                    isMcSource [GT_FALSE]}.
    Expected: GT_OK.
    1.2. Call with routerTtiTcamRow [0]
                   and non-NULL pointers.
    Expected: GT_OK.
    1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with validPtr [NULL]
                    and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with prefixPtr [NULL]
                    and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with maskPtr [NULL]
                    and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call cpssDxChIpv6PrefixInvalidate with routerTtiTcamRow [0],
                                                routerTtiTcamColumn [1].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                      routerTtiTcamRow = 0;
    CPSS_DXCH_IPV6_PREFIX_STC   prefix;
    CPSS_DXCH_IPV6_PREFIX_STC   mask;
    GT_BOOL                     isValid   = GT_FALSE;
    GT_U32                      maxTtiRow = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&prefix, sizeof(prefix));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        maxTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        /*
            1.1. Set Ipv6 prefix for testing.
                 Call cpssDxChIpv6PrefixSet with routerTtiTcamRow [0],
                                                 prefixPtr {vrId [100],
                                                            ipAddr [{200, 200, 10, 1}],
                                                            isMcSource [GT_FALSE]}.
            Expected: GT_OK.
        */
        cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));

        routerTtiTcamRow = 0;

        prefix.vrId       = 100;
        prefix.isMcSource = GT_FALSE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 1;

        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpv6PrefixSet: %d, %d",
                                     dev, routerTtiTcamRow);

        /*
            1.2. Call with routerTtiTcamRow [0],
                           and non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, &prefix, &mask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        /*
            1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamRow = maxTtiRow;

        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, &prefix, &mask);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        routerTtiTcamRow = 0;

        /*
            1.4. Call with validPtr [NULL]
                            and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, NULL, &prefix, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, validPtr = NULL", dev);

        /*
            1.5. Call with prefixPtr [NULL]
                            and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, NULL, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, prefixPtr = NULL", dev);

        /*
            1.6. Call with maskPtr [NULL]
                            and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, &prefix, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, maskPtr = NULL", dev);

        /*
            1.9. Call cpssDxChIpv6PrefixInvalidate with routerTtiTcamRow [0].
            Expected: GT_OK.
        */
        routerTtiTcamRow = 0;

        st = cpssDxChIpv6PrefixInvalidate(dev, routerTtiTcamRow);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpv6PrefixInvalidate: %d, %d",
                                     dev, routerTtiTcamRow);
    }

    routerTtiTcamRow = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, &prefix, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6PrefixGet(dev, routerTtiTcamRow, &isValid, &prefix, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6PrefixInvalidate
(
    IN  GT_U8           devNum,
    IN  GT_U32          routerTtiTcamRow
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6PrefixInvalidate)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Set Ipv6 prefix for testing.
         Call cpssDxChIpv6PrefixSet with routerTtiTcamRow [0],
                                         prefixPtr {vrId [100],
                                                    ipAddr [{200, 200, 10, 1}],
                                                    isMcSource [GT_FALSE]}.
    Expected: GT_OK.
    1.2. Call with routerTtiTcamRow [0].
    Expected: GT_OK.
    1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                   and other params from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                      routerTtiTcamRow = 0;
    GT_U32                      maxTtiRow        = 0;
    CPSS_DXCH_IPV6_PREFIX_STC   prefix;
    CPSS_DXCH_IPV6_PREFIX_STC   mask;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    cpssOsBzero((GT_VOID*)&prefix, sizeof(prefix));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        maxTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        /*
            1.1. Set Ipv6 prefix for testing.
                 Call cpssDxChIpv6PrefixSet with routerTtiTcamRow [0],
                                                 prefixPtr {vrId [100],
                                                            ipAddr [{200, 200, 10, 1}],
                                                            isMcSource [GT_FALSE]}.
            Expected: GT_OK.
        */
        cpssOsMemSet((GT_VOID*) &mask, 0xFF, sizeof(mask));

        routerTtiTcamRow = 0;

        prefix.vrId       = 100;
        prefix.isMcSource = GT_FALSE;

        prefix.ipAddr.arIP[0] = 200;
        prefix.ipAddr.arIP[1] = 200;
        prefix.ipAddr.arIP[2] = 10;
        prefix.ipAddr.arIP[3] = 1;

        st = cpssDxChIpv6PrefixSet(dev, routerTtiTcamRow, &prefix, &mask);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpv6PrefixSet: %d, %d",
                                     dev, routerTtiTcamRow);

        /*
            1.2. Call with routerTtiTcamRow [0].
            Expected: GT_OK.
        */
        st = cpssDxChIpv6PrefixInvalidate(dev, routerTtiTcamRow);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);

        /*
            1.3. Call with out of range routerTtiTcamRow [maxTtiTcamRow(dev)]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        routerTtiTcamRow = maxTtiRow;

        st = cpssDxChIpv6PrefixInvalidate(dev, routerTtiTcamRow);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerTtiTcamRow);
    }

    routerTtiTcamRow = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6PrefixInvalidate(dev, routerTtiTcamRow);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6PrefixInvalidate(dev, routerTtiTcamRow);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLttWrite
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          lttTtiRow,
    IN  GT_U32                          lttTtiColumn,
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC      *lttEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLttWrite)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with lttTtiRow[tableSize - 1], lttTtiColumn [1],
            lttEntryPtr [routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
                         numOfPaths = 0;
                         routeEntryBaseIndex = 0;
                         ucRPFCheckEnable = GT_FALSE;
                         sipSaCheckMismatchEnable = GT_FALSE;
                         ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E].
    Expected: GT_OK.
    1.2. Call cpssDxChIpLttRead with the same params.
    Expected: GT_OK and same values as written.
    1.3. Check out-of-range lttTtiRow [tableSize], other params same as in 1.1.
    Expected: NOT GT_OK.
    1.4. Check out-of-range lttTtiColumn [6], other params same as in 1.1.
    Expected: NOT GT_OK.
    1.5. Check incorrect routeType. Call with lttEntryPtr
    [routeType = wrong enum values;], other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Check incorrect numOfPaths.  Call with lttEntryPtr [numOfPaths = 8 / 9],
    other params same as in 1.1.
    Expected: GT_OK only for lion B0 and above and not GT_OK for other.
    1.7. Check incorrect numOfPaths.  Call with numOfPaths [64 / 65],
    other params same as in 1.6.
    Expected: not GT_OK.
    1.8. Check incorrect routeEntryBaseIndex. Call with
    lttEntryPtr [numOfPaths = 0; routeEntryBaseIndex = tableSize;],
    other params same as in 1.1.
    Expected: non GT_OK.
    1.9. Check incorrect ipv6MCGroupScopeLevel.
         Call with wrong lttEntryPtr.ipv6MCGroupScopeLevel,
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Check for NULL-pointer. Call with lttEntryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     tableSize    = 0;

    GT_U32     lttTtiRow    = 0;
    GT_U32     lttTtiColumn = 0;

    CPSS_DXCH_IP_LTT_ENTRY_STC lttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC lttEntryGet;

    cpssOsBzero((GT_VOID*)&lttEntry, sizeof(lttEntry));
    cpssOsBzero((GT_VOID*)&lttEntryGet, sizeof(lttEntryGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        /*
            1.1. Call function with lttTtiRow[tableSize - 1], lttTtiColumn [1],
                    lttEntryPtr [routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
                                 numOfPaths = 0;
                                 routeEntryBaseIndex = 0;
                                 ucRPFCheckEnable = GT_FALSE;
                                 sipSaCheckMismatchEnable = GT_FALSE;
                                 ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E].
            Expected: GT_OK.
        */
        lttTtiRow    = tableSize - 1;
        lttTtiColumn = 1;

        lttEntry.routeType                = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        lttEntry.numOfPaths               = 0;
        lttEntry.routeEntryBaseIndex      = 0;
        lttEntry.ucRPFCheckEnable         = GT_FALSE;
        lttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        lttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);
        }

        /*
            1.2. Call cpssDxChIpLttRead with the same params.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, &lttEntryGet);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpLttRead: %d, %d",
                                             dev, lttTtiRow, lttTtiColumn);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpLttRead: %d, %d",
                                         dev, lttTtiRow, lttTtiColumn);

            if (GT_OK == st)
            {
                /* Verifying lttEntry fields */
                UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.routeType, lttEntryGet.routeType,
                    "get another lttEntryPtr->routeType than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.numOfPaths, lttEntryGet.numOfPaths,
                    "get another lttEntryPtr->numOfPaths than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.routeEntryBaseIndex,
                                             lttEntryGet.routeEntryBaseIndex,
                    "get another lttEntryPtr->routeEntryBaseIndex than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.ucRPFCheckEnable,
                                             lttEntryGet.ucRPFCheckEnable,
                    "get another lttEntryPtr->ucRPFCheckEnable than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.sipSaCheckMismatchEnable,
                                             lttEntryGet.sipSaCheckMismatchEnable,
                    "get another lttEntryPtr->sipSaCheckMismatchEnable than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.ipv6MCGroupScopeLevel,
                                             lttEntryGet.ipv6MCGroupScopeLevel,
                    "get another lttEntryPtr->ipv6MCGroupScopeLevel than was set: %d", dev);
            }
        }

        /*
            1.3. Check out-of-range lttTtiRow [tableSize], other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        lttTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);

        lttTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router - 1;

        /*
            1.4. Check out-of-range lttTtiColumn [6], other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        lttTtiColumn = 6;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);

        lttTtiColumn = 1;

        /*
            1.5. Check incorrect routeType. Call with lttEntryPtr
            [routeType = wrong enum values;], other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLttWrite
                               (dev, lttTtiRow, lttTtiColumn, &lttEntry),
                               lttEntry.routeType);
        }

        /*
            1.6. Check incorrect numOfPaths.  Call with lttEntryPtr [numOfPaths = 8 / 9],
            other params same as in 1.1.
            Expected: GT_OK only for lion B0 and above and not GT_OK for other.
        */
        /* call with lttEntry.numOfPaths = 8; */
        lttEntry.routeType  = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        lttEntry.numOfPaths = 8;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);

        if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "%d, lttEntryPtr->numOfPaths = %d", dev, lttEntry.numOfPaths);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                        "%d, lttEntryPtr->numOfPaths = %d", dev, lttEntry.numOfPaths);
        }

        /* call with lttEntry.numOfPaths = 9; */
        lttEntry.routeType  = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        lttEntry.numOfPaths = 9;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);

        if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "%d, lttEntryPtr->numOfPaths = %d", dev, lttEntry.numOfPaths);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                        "%d, lttEntryPtr->numOfPaths = %d", dev, lttEntry.numOfPaths);
        }

        /*
            1.7. Check incorrect numOfPaths.  Call with numOfPaths [64 / 65],
            other params same as in 1.6.
            Expected: not GT_OK.
        */
        /* call with lttEntry.numOfPaths = 64; */
        lttEntry.numOfPaths = 64;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, lttEntryPtr->numOfPaths = %d", dev, lttEntry.numOfPaths);

        /* call with lttEntry.numOfPaths = 65; */
        lttEntry.numOfPaths = 65;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, lttEntryPtr->numOfPaths = %d", dev, lttEntry.numOfPaths);

        lttEntry.numOfPaths = 0;

        /*
            1.8. Check incorrect routeEntryBaseIndex. Call with
            lttEntryPtr [numOfPaths = 0; routeEntryBaseIndex = tableSize;],
            other params same as in 1.1.
            Expected: non GT_OK.
        */
        lttEntry.numOfPaths          = 0;
        lttEntry.routeEntryBaseIndex =
                        PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;

        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, lttEntryPtr->routeEntryBaseIndex = %d",
                                         dev, lttEntry.routeEntryBaseIndex);

        lttEntry.routeEntryBaseIndex = tableSize - 1;

        /*
            1.9. Check incorrect ipv6MCGroupScopeLevel.
                 Call with wrong lttEntryPtr.ipv6MCGroupScopeLevel,
                 other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpLttWrite
                               (dev, lttTtiRow, lttTtiColumn, &lttEntry),
                               lttEntry.ipv6MCGroupScopeLevel);
        }

        /*
            1.10. Check for NULL-pointer. Call with lttEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, lttEntryPtr = NULL", dev);
    }

    cpssOsBzero((GT_VOID*)&lttEntry, sizeof(lttEntry));

    lttEntry.routeType             = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    lttEntry.numOfPaths            = 0;
    lttEntry.routeEntryBaseIndex   = 0;
    lttEntry.ucRPFCheckEnable      = GT_FALSE;
    lttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    lttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpLttRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          lttTtiRow,
    IN  GT_U32                          lttTtiColumn,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC     *lttEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpLttRead)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with lttTtiRow[tableSize - 1], lttTtiColumn [1].
    Expected: GT_OK.
    1.2. Check out-of-range lttTtiRow [tableSize], other params same as in 1.1.
    Expected: NOT GT_OK.
    1.3. Check out-of-range lttTtiColumn [6], other params same as in 1.1.
    Expected: NOT GT_OK.
    1.4. Check for NULL-pointer. Call with lttEntryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st = GT_OK;
    GT_U8      dev;

    GT_U32     lttTtiRow    = 0;
    GT_U32     lttTtiColumn = 0;

    CPSS_DXCH_IP_LTT_ENTRY_STC lttEntry;

    cpssOsBzero((GT_VOID*)&lttEntry, sizeof(lttEntry));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with lttTtiRow[tableSize - 1], lttTtiColumn [1].
            Expected: GT_OK.
        */
        lttTtiRow    = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router - 1;
        lttTtiColumn = 1;

        lttEntry.routeType                = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        lttEntry.numOfPaths               = 0;
        lttEntry.routeEntryBaseIndex      = 0;
        lttEntry.ucRPFCheckEnable         = GT_FALSE;
        lttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        lttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

        st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);
        }

        /*
            1.2. Check out-of-range lttTtiRow [tableSize], other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        lttTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router;

        st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);

        lttTtiRow = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router - 1;

        /*
            1.3. Check out-of-range lttTtiColumn [6], other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        lttTtiColumn = 6;

        st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lttTtiRow, lttTtiColumn);

        lttTtiColumn = 1;

        /*
            1.4. Check for NULL-pointer. Call with lttEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, lttEntryPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, &lttEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          baseRouteEntryIndex,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                          numOfRouteEntries
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRouteEntriesWrite)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with baseRouteEntryIndex [0],
         routeEntriesArray[0] [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                               cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                               appSpecificCpuCodeEnable = GT_FALSE;
                               unicastPacketSipFilterEnable = GT_FALSE;
                               ttlHopLimitDecEnable = GT_TRUE;
                               ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                               ingressMirror = GT_TRUE;
                               qosProfileMarkingEnable = GT_FALSE;
                               qosProfileIndex = 0;
                               qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                               modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                               modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                               countSet = CPSS_IP_CNT_SET0_E;
                               trapMirrorArpBcEnable = GT_FALSE;
                               sipAccessLevel = 0;
                               dipAccessLevel = 0;
                               ICMPRedirectEnable = GT_FALSE;
                               scopeCheckingEnable = GT_TRUE;
                               siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                               mtuProfileIndex = 0;
                               isTunnelStart = GT_FALSE;
                               nextHopVlanId = 100;
                               nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                 devPort {devNum = 0; portNum = 0} ,
                                                 trunkId = 0; vidx = 10; vlanId = 100 },
                               nextHopARPPointer = 0;
                               nextHopTunnelPointer = 0 } ],
           numOfRouteEntries [1].
    Expected: GT_OK.
    1.2. Call cpssDxChIpUcRouteEntriesRead with baseRouteEntryIndex [0],
    non-NULL routeEntriesArray, numOfRouteEntries [1].
    Expected: GT_OK and same values as written.
    1.3. Call function with routeEntriesArray[0]
                            [cmd = CPSS_PACKET_CMD_FORWARD_E ] (not supported),
                            other params same as in 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with wrong enum routeEntriesArray[0] [cmd = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call function with routeEntriesArray[0] [cpuCodeIdx = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call function with routeEntriesArray[0] [qosPrecedence = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call function with routeEntriesArray[0] [modifyUp = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.8. Call function with routeEntriesArray[0] [modifyDscp = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call function with routeEntriesArray[0] [countSet = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call function with routeEntriesArray[0] [siteId = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.11. Call function with routeEntriesArray[0] [mtuProfileIndex = 8 ],
                             other params same as in 1.1.
    Expected: non GT_OK.
    1.12. Call function with routeEntriesArray[0]
    [nextHopVlanId = PRV_CPSS_MAX_NUM_VLANS_CNS = 4096 ], other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.13. Call function with wrong baseRouteEntryIndex [tableSize],
    other params same as in 1.1.
    Expected: NOT GT_OK
    1.14. Call function with baseRouteEntryIndex [tableSize + numOfEntries],
                             other params same as in 1.1.
    Expected: NOT GT_OK.
    1.15. Call function with routeEntriesArray [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.16. Call function with numOfEntries [tableSize], other params same as in 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS                       st           = GT_OK;
    GT_STATUS                       expectedResult;
    GT_U8                           dev;
    GT_U32                          entryIndex   = 0;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC routeEntriesArray;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC routeEntriesArrayGet;
    GT_U32                          numOfEntries = 0;

    GT_BOOL                         isEqual      = GT_FALSE;
    GT_U32                          tableSize    = 0;

    cpssOsBzero((GT_VOID*)&routeEntriesArray, sizeof(routeEntriesArray));
    cpssOsBzero((GT_VOID*)&routeEntriesArrayGet, sizeof(routeEntriesArrayGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)-> fineTuning.tableSize.routerNextHop;

        /*
            1.1. Call function with baseRouteEntryIndex [0],
                 routeEntriesArray[0] [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                                       cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                                       appSpecificCpuCodeEnable = GT_FALSE;
                                       unicastPacketSipFilterEnable = GT_FALSE;
                                       ttlHopLimitDecEnable = GT_TRUE;
                                       ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                                       ingressMirror = GT_TRUE;
                                       ingressMirrorToAnalyzerIndex = 0;
                                       qosProfileMarkingEnable = GT_FALSE;
                                       qosProfileIndex = 0;
                                       qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                                       modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                       modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                       countSet = CPSS_IP_CNT_SET0_E;
                                       trapMirrorArpBcEnable = GT_FALSE;
                                       sipAccessLevel = 0;
                                       dipAccessLevel = 0;
                                       ICMPRedirectEnable = GT_FALSE;
                                       scopeCheckingEnable = GT_TRUE;
                                       siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                                       mtuProfileIndex = 0;
                                       isTunnelStart = GT_FALSE;
                                       nextHopVlanId = 100;
                                       nextHopVlanId1 = 100;
                                       nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                         devPort {devNum = 0; portNum = 0} ,
                                                         trunkId = 0; vidx = 10; vlanId = 100 },
                                       nextHopARPPointer = 0;
                                       nextHopTunnelPointer = 0 } ],
                   numOfRouteEntries [1].
            Expected: GT_OK.
        */
        entryIndex = 0;

        cpssOsBzero((GT_VOID*)&routeEntriesArray, sizeof(routeEntriesArray));
        cpssOsBzero((GT_VOID*)&routeEntriesArrayGet, sizeof(routeEntriesArrayGet));

        routeEntriesArray.type  = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

        routeEntriesArray.entry.regularEntry.cmd                   = CPSS_PACKET_CMD_ROUTE_E;
        routeEntriesArray.entry.regularEntry.cpuCodeIdx            = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
        routeEntriesArray.entry.regularEntry.ttlHopLimitDecEnable  = GT_TRUE;
        routeEntriesArray.entry.regularEntry.ingressMirror         = GT_TRUE;
        routeEntriesArray.entry.regularEntry.ingressMirrorToAnalyzerIndex = 0;
        routeEntriesArray.entry.regularEntry.qosProfileIndex       = 0;
        routeEntriesArray.entry.regularEntry.qosPrecedence         = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        routeEntriesArray.entry.regularEntry.modifyUp              = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.entry.regularEntry.modifyDscp            = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.entry.regularEntry.countSet              = CPSS_IP_CNT_SET0_E;
        routeEntriesArray.entry.regularEntry.trapMirrorArpBcEnable = GT_FALSE;
        routeEntriesArray.entry.regularEntry.sipAccessLevel        = 0;
        routeEntriesArray.entry.regularEntry.dipAccessLevel        = 0;
        routeEntriesArray.entry.regularEntry.ICMPRedirectEnable    = GT_FALSE;
        routeEntriesArray.entry.regularEntry.scopeCheckingEnable   = GT_TRUE;
        routeEntriesArray.entry.regularEntry.siteId                = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntriesArray.entry.regularEntry.mtuProfileIndex       = 0;
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            routeEntriesArray.entry.regularEntry.isTunnelStart         = GT_TRUE;
            routeEntriesArray.entry.regularEntry.isNat                 = GT_TRUE;
            routeEntriesArray.entry.regularEntry.nextHopNatPointer     = 2;
        }
        else
        {
            routeEntriesArray.entry.regularEntry.isTunnelStart     = GT_FALSE;
            routeEntriesArray.entry.regularEntry.isNat             = GT_FALSE;
            routeEntriesArray.entry.regularEntry.nextHopNatPointer = 0;
        }
        routeEntriesArray.entry.regularEntry.nextHopVlanId         = IP_VLAN_TESTED_VLAN_ID_CNS;
        routeEntriesArray.entry.regularEntry.nextHopVlanId1        = IP_VLAN_TESTED_VLAN_ID_CNS;
        routeEntriesArray.entry.regularEntry.nextHopARPPointer     = 0;
        routeEntriesArray.entry.regularEntry.nextHopTunnelPointer  = 0;


        routeEntriesArray.entry.regularEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
        routeEntriesArray.entry.regularEntry.qosProfileMarkingEnable         = GT_FALSE;
        routeEntriesArray.entry.regularEntry.appSpecificCpuCodeEnable        = GT_FALSE;
        routeEntriesArray.entry.regularEntry.unicastPacketSipFilterEnable    = GT_FALSE;

        routeEntriesArray.entry.regularEntry.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.hwDevNum  = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId         = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.vidx            = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.vlanId          = 0;

        numOfEntries = 1;

        if((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            /* Disable assignment of VID1 */
            st = cpssDxChIpUcRoutingVid1AssignEnableSet(dev, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                             dev, entryIndex, numOfEntries);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                     dev, entryIndex, numOfEntries);
        }

        /*
            1.2.1. Call cpssDxChIpUcRouteEntriesRead with baseRouteEntryIndex [0],
            non-NULL routeEntriesArray, numOfRouteEntries [1].
            Expected: GT_OK and same values as written.
        */
        routeEntriesArrayGet.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArrayGet, numOfEntries);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                             "cpssDxChIpUcRouteEntriesRead: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                             dev, entryIndex, numOfEntries);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
               "cpssDxChIpUcRouteEntriesRead: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                           dev, entryIndex, numOfEntries);

            if (GT_OK == st)
            {
                /* nextHopVlanId1 is not applicable when VID1 assignment is disabled */
                routeEntriesArrayGet.entry.regularEntry.nextHopVlanId1 = IP_VLAN_TESTED_VLAN_ID_CNS;

                /* Verifying routeEntriesArray fields */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &routeEntriesArray,
                                             (GT_VOID*) &routeEntriesArrayGet,
                                             sizeof(routeEntriesArray))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                           "get another routeEntriesArray than was set: %d", dev);
            }
        }

        if((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            /* Enable assignment of VID1 */
            st = cpssDxChIpUcRoutingVid1AssignEnableSet(dev, GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);

            st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                                 dev, entryIndex, numOfEntries);
            }
            else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                         dev, entryIndex, numOfEntries);
            }

            /*
                1.2.2. Call cpssDxChIpUcRouteEntriesRead with baseRouteEntryIndex [0],
                non-NULL routeEntriesArray, numOfRouteEntries [1].
                Expected: GT_OK and same values as written.
            */
            cpssOsBzero((GT_VOID*)&routeEntriesArrayGet, sizeof(routeEntriesArrayGet));
            st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArrayGet, numOfEntries);
            if (prvUtfIsPbrModeUsed())
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                                 "cpssDxChIpUcRouteEntriesRead: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                                 dev, entryIndex, numOfEntries);
            }
            else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "cpssDxChIpUcRouteEntriesRead: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                               dev, entryIndex, numOfEntries);

                if (GT_OK == st)
                {
                    /* QoS attributes are not applicable when VID1 assignment is enabled */
                    routeEntriesArrayGet.entry.regularEntry.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                    routeEntriesArrayGet.entry.regularEntry.qosProfileIndex = 0;
                    routeEntriesArrayGet.entry.regularEntry.qosProfileMarkingEnable = GT_FALSE;
                    routeEntriesArrayGet.entry.regularEntry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                    routeEntriesArrayGet.entry.regularEntry.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;

                    /* Verifying routeEntriesArray fields */
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*) &routeEntriesArray,
                                                 (GT_VOID*) &routeEntriesArrayGet,
                                                 sizeof(routeEntriesArray))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                               "get another routeEntriesArray than was set: %d", dev);
                }
            }

            /* Disable assignment of VID1 */
            st = cpssDxChIpUcRoutingVid1AssignEnableSet(dev, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d", dev);
        }

        /*
            1.3. Call function with routeEntriesArray[0]
                                    [cmd = CPSS_PACKET_CMD_FORWARD_E ] (not supported),
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        routeEntriesArray.entry.regularEntry.cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, routeEntriesArray[0].cmd = %d",
                                     dev, routeEntriesArray.entry.regularEntry.cmd);

        /*
            1.3. Call function with routeEntriesArray[0]
                                    [cmd = CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E ] (supported only for SIP5.15),
                                    other params same as in 1.1.
            Expected: GT_OK for SIP5.15 devices and GT_BAD_PARAM otherwise.
        */
        routeEntriesArray.entry.regularEntry.cmd = CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        expectedResult = (PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) ? GT_OK : GT_BAD_PARAM;
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedResult, st, "%d, routeEntriesArray[0].cmd = %d",
                                     dev, routeEntriesArray.entry.regularEntry.cmd);

        routeEntriesArray.entry.regularEntry.cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

        /*
            1.4. Call function with wrong enum routeEntriesArray[0] [cmd = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRouteEntriesWrite
                           (dev, entryIndex, &routeEntriesArray, numOfEntries),
                           routeEntriesArray.entry.regularEntry.cmd);
        }

        /*
            1.5. Call function with routeEntriesArray[0] [cpuCodeIdx = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRouteEntriesWrite
                               (dev, entryIndex, &routeEntriesArray, numOfEntries),
                               routeEntriesArray.entry.regularEntry.cpuCodeIdx);
        }

        /*
            1.6. Call function with routeEntriesArray[0] [qosPrecedence = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRouteEntriesWrite
                               (dev, entryIndex, &routeEntriesArray, numOfEntries),
                               routeEntriesArray.entry.regularEntry.qosPrecedence);
        }

        /*
            1.7. Call function with routeEntriesArray[0] [modifyUp = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRouteEntriesWrite
                               (dev, entryIndex, &routeEntriesArray, numOfEntries),
                               routeEntriesArray.entry.regularEntry.modifyUp);
        }

        /*
            1.8. Call function with routeEntriesArray[0] [modifyDscp = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRouteEntriesWrite
                               (dev, entryIndex, &routeEntriesArray, numOfEntries),
                               routeEntriesArray.entry.regularEntry.modifyDscp);
        }

        /*
            1.9. Call function with routeEntriesArray[0] [countSet = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRouteEntriesWrite
                               (dev, entryIndex, &routeEntriesArray, numOfEntries),
                               routeEntriesArray.entry.regularEntry.countSet);
        }

        /*
            1.10. Call function with routeEntriesArray[0] [siteId = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRouteEntriesWrite
                               (dev, entryIndex, &routeEntriesArray, numOfEntries),
                               routeEntriesArray.entry.regularEntry.siteId);
        }

        /*
            1.11. Call function with routeEntriesArray[0] [mtuProfileIndex = 8 ],
                                     other params same as in 1.1.
            Expected: non GT_OK.
        */
        routeEntriesArray.entry.regularEntry.siteId          = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntriesArray.entry.regularEntry.mtuProfileIndex = 8;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, routeEntriesArray[0].mtuProfileIndex = %d",
                            dev, routeEntriesArray.entry.regularEntry.mtuProfileIndex);

        routeEntriesArray.entry.regularEntry.mtuProfileIndex = 0;

        /*
            1.12. Call function with routeEntriesArray[0]
            [nextHopVlanId = PRV_CPSS_MAX_NUM_VLANS_CNS = 4096 ], other params same as in 1.1.
            Expected: GT_OUT_OF_RANGE.

            For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X:
            Call function with routeEntriesArray[0]
            [nextHopVlanId1 = PRV_CPSS_MAX_NUM_VLANS_CNS = 4096 ], other params same as in 1.1.
            Expected: GT_OUT_OF_RANGE.
        */
        routeEntriesArray.entry.regularEntry.nextHopVlanId = (GT_U16)(PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(dev) + 1);

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, routeEntriesArray[0].nextHopVlanId = %d",
                                 dev, routeEntriesArray.entry.regularEntry.nextHopVlanId);

        routeEntriesArray.entry.regularEntry.nextHopVlanId = IP_VLAN_TESTED_VLAN_ID_CNS;

        if((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            routeEntriesArray.entry.regularEntry.nextHopVlanId1 = (GT_U16)(PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(dev) + 1);

            st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                        "%d, routeEntriesArray[0].nextHopVlanId1 = %d",
                                     dev, routeEntriesArray.entry.regularEntry.nextHopVlanId1);

            routeEntriesArray.entry.regularEntry.nextHopVlanId1 = IP_VLAN_TESTED_VLAN_ID_CNS;
        }

        /*
            1.13. Call function with wrong baseRouteEntryIndex [tableSize],
            other params same as in 1.1.
            Expected: NOT GT_OK
        */
        entryIndex  = tableSize;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d",
                                 dev, entryIndex);
        entryIndex = 1;

        /*
            1.14. Call function with baseRouteEntryIndex [tableSize + numOfEntries],
                                     other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        entryIndex = tableSize + numOfEntries;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d",
                                     dev, entryIndex);

        entryIndex = 1;

        /*
            1.15. Call function with routeEntriesArray [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, NULL, numOfEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, routeEntriesArray = NULL", dev);

        /*
            1.16. Call function with numOfEntries [tableSize], other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        entryIndex = 0;
        numOfEntries = tableSize + 1;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, baseRouteEntryIndex = %d", dev, entryIndex);

        numOfEntries = 1;

        /*
            1.17. Call function with routeEntriesArray[0]
            [ingressMirrorToAnalyzerIndex = 7], other params same as in 1.1.
            Expected: GT_OUT_OF_RANGE.
        */
        if((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            routeEntriesArray.entry.regularEntry.ingressMirrorToAnalyzerIndex = PRV_CPSS_DXCH_IP_MIRROR_ANALYZER_MAX_INDEX_CNS + 1;

            st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                        "%d, routeEntriesArray[0].ingressMirrorToAnalyzerIndex = %d",
                                     dev, routeEntriesArray.entry.regularEntry.ingressMirrorToAnalyzerIndex);

            routeEntriesArray.entry.regularEntry.ingressMirrorToAnalyzerIndex = 0;
        }
    }

    entryIndex = 0;

    cpssOsBzero((GT_VOID*)&routeEntriesArray, sizeof(routeEntriesArray));

    routeEntriesArray.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

    routeEntriesArray.entry.regularEntry.cmd                   = CPSS_PACKET_CMD_DROP_SOFT_E;
    routeEntriesArray.entry.regularEntry.cpuCodeIdx            = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
    routeEntriesArray.entry.regularEntry.ttlHopLimitDecEnable  = GT_TRUE;
    routeEntriesArray.entry.regularEntry.ingressMirror         = GT_TRUE;
    routeEntriesArray.entry.regularEntry.ingressMirrorToAnalyzerIndex = 0;
    routeEntriesArray.entry.regularEntry.qosProfileIndex       = 0;
    routeEntriesArray.entry.regularEntry.qosPrecedence         = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    routeEntriesArray.entry.regularEntry.modifyUp              = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntriesArray.entry.regularEntry.modifyDscp            = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntriesArray.entry.regularEntry.countSet              = CPSS_IP_CNT_SET0_E;
    routeEntriesArray.entry.regularEntry.trapMirrorArpBcEnable = GT_FALSE;
    routeEntriesArray.entry.regularEntry.sipAccessLevel        = 0;
    routeEntriesArray.entry.regularEntry.dipAccessLevel        = 0;
    routeEntriesArray.entry.regularEntry.ICMPRedirectEnable    = GT_FALSE;
    routeEntriesArray.entry.regularEntry.scopeCheckingEnable   = GT_TRUE;
    routeEntriesArray.entry.regularEntry.siteId                = CPSS_IP_SITE_ID_INTERNAL_E;
    routeEntriesArray.entry.regularEntry.mtuProfileIndex       = 0;
    routeEntriesArray.entry.regularEntry.isTunnelStart         = GT_FALSE;
    routeEntriesArray.entry.regularEntry.isNat                 = GT_FALSE;
    routeEntriesArray.entry.regularEntry.nextHopVlanId         = IP_VLAN_TESTED_VLAN_ID_CNS;
    routeEntriesArray.entry.regularEntry.nextHopVlanId1        = IP_VLAN_TESTED_VLAN_ID_CNS;
    routeEntriesArray.entry.regularEntry.nextHopARPPointer     = 0;
    routeEntriesArray.entry.regularEntry.nextHopTunnelPointer  = 0;
    routeEntriesArray.entry.regularEntry.nextHopNatPointer     = 0;

    routeEntriesArray.entry.regularEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    routeEntriesArray.entry.regularEntry.qosProfileMarkingEnable         = GT_FALSE;
    routeEntriesArray.entry.regularEntry.appSpecificCpuCodeEnable        = GT_FALSE;
    routeEntriesArray.entry.regularEntry.unicastPacketSipFilterEnable    = GT_FALSE;

    routeEntriesArray.entry.regularEntry.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.hwDevNum  = 0;
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum = 0;
    routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId         = 0;
    routeEntriesArray.entry.regularEntry.nextHopInterface.vidx            = 0;
    routeEntriesArray.entry.regularEntry.nextHopInterface.vlanId          = 0;

    numOfEntries = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUcRouteEntriesRead
(
    IN GT_U8                            devNum,
    IN GT_U32                           baseRouteEntryIndex,
    OUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                           numOfRouteEntries
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRouteEntriesRead)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with baseRouteEntryIndex [0],
        routeEntriesArray[0] [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                               cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                               appSpecificCpuCodeEnable = GT_FALSE;
                               unicastPacketSipFilterEnable = GT_FALSE;
                               ttlHopLimitDecEnable = GT_TRUE;
                               ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                               ingressMirror = GT_TRUE;
                               ingressMirrorToAnalyzerIndex = 0;
                               qosProfileMarkingEnable = GT_FALSE;
                               qosProfileIndex = 0;
                               qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                               modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                               modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                               countSet = CPSS_IP_CNT_SET0_E;
                               trapMirrorArpBcEnable = GT_FALSE;
                               sipAccessLevel = 0;
                               dipAccessLevel = 0;
                               ICMPRedirectEnable = GT_FALSE;
                               scopeCheckingEnable = GT_TRUE;
                               siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                               mtuProfileIndex = 0;
                               isTunnelStart = GT_FALSE;
                               nextHopVlanId = 100;
                               nextHopVlanId1 = 100;
                               nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                 devPort {devNum = 0; portNum = 0} ,
                                                 trunkId = 0; vidx = 10; vlanId = 100 },
                               nextHopARPPointer = 0;
                               nextHopTunnelPointer = 0 } ],
        numOfRouteEntries [1].
    Expected: GT_OK.
    1.2. Call function with baseRouteEntryIndex [0], non-NULL  routeEntriesArray,
         numOfRouteEntries [1].
    Expected: GT_OK.
    1.3. Check reading at unused index. Call function with
    baseRouteEntryIndex [tableSize - 1], non-NULL routeEntriesArray, numOfRouteEntries [1].
    Expected: GT_OK.
    1.4. Call function with baseRouteEntryIndex [tableSize], other params same as in 1.2.
    Expected: NOT GT_OK.
    1.5. Call function with baseRouteEntryIndex [tableSize + numOfEntries],
    other params same as in 1.2.
    Expected: NOT GT_OK
    1.6. Call function with routeEntriesArray [NULL], other params same as in 1.2.
    Expected: GT_BAD_PTR.
    1.7. Call function with entryIndex [0], numOfEntries [tableSize]
    and other params same as in 1.2.
    Expected: NOT GT_OK
*/
    GT_STATUS                       st           = GT_OK;
    GT_U8                           dev;
    GT_U32                          entryIndex   = 0;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC routeEntriesArray;

    GT_U32                          numOfEntries = 0;
    GT_U32                          tableSize    = 0;

    cpssOsBzero((GT_VOID*)&routeEntriesArray, sizeof(routeEntriesArray));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)-> fineTuning.tableSize.routerNextHop;

        /*
            1.1. Call function with baseRouteEntryIndex [0],
                routeEntriesArray[0] [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                                       cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                                       appSpecificCpuCodeEnable = GT_FALSE;
                                       unicastPacketSipFilterEnable = GT_FALSE;
                                       ttlHopLimitDecEnable = GT_TRUE;
                                       ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                                       ingressMirror = GT_TRUE;
                                       ingressMirrorToAnalyzerIndex = 0;
                                       qosProfileMarkingEnable = GT_FALSE;
                                       qosProfileIndex = 0;
                                       qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                                       modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                       modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                       countSet = CPSS_IP_CNT_SET0_E;
                                       trapMirrorArpBcEnable = GT_FALSE;
                                       sipAccessLevel = 0;
                                       dipAccessLevel = 0;
                                       ICMPRedirectEnable = GT_FALSE;
                                       scopeCheckingEnable = GT_TRUE;
                                       siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                                       mtuProfileIndex = 0;
                                       isTunnelStart = GT_FALSE;
                                       nextHopVlanId = 100;
                                       nextHopVlanId1 = 100;
                                       nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                         devPort {devNum = 0; portNum = 0} ,
                                                         trunkId = 0; vidx = 10; vlanId = 100 },
                                       nextHopARPPointer = 0;
                                       nextHopTunnelPointer = 0 } ],
                numOfRouteEntries [1].
            Expected: GT_OK.
        */
        entryIndex = 0;

        routeEntriesArray.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

        routeEntriesArray.entry.regularEntry.cmd                   = CPSS_PACKET_CMD_DROP_SOFT_E;
        routeEntriesArray.entry.regularEntry.cpuCodeIdx            = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
        routeEntriesArray.entry.regularEntry.ttlHopLimitDecEnable  = GT_TRUE;
        routeEntriesArray.entry.regularEntry.ingressMirror         = GT_TRUE;
        routeEntriesArray.entry.regularEntry.ingressMirrorToAnalyzerIndex = 0;
        routeEntriesArray.entry.regularEntry.qosProfileIndex       = 0;
        routeEntriesArray.entry.regularEntry.qosPrecedence         = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        routeEntriesArray.entry.regularEntry.modifyUp              = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.entry.regularEntry.modifyDscp            = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntriesArray.entry.regularEntry.countSet              = CPSS_IP_CNT_SET0_E;
        routeEntriesArray.entry.regularEntry.trapMirrorArpBcEnable = GT_FALSE;
        routeEntriesArray.entry.regularEntry.sipAccessLevel        = 0;
        routeEntriesArray.entry.regularEntry.dipAccessLevel        = 0;
        routeEntriesArray.entry.regularEntry.ICMPRedirectEnable    = GT_FALSE;
        routeEntriesArray.entry.regularEntry.scopeCheckingEnable   = GT_TRUE;
        routeEntriesArray.entry.regularEntry.siteId                = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntriesArray.entry.regularEntry.mtuProfileIndex       = 0;
        routeEntriesArray.entry.regularEntry.isTunnelStart         = GT_FALSE;
        routeEntriesArray.entry.regularEntry.isNat                 = GT_FALSE;
        routeEntriesArray.entry.regularEntry.nextHopVlanId         = IP_VLAN_TESTED_VLAN_ID_CNS;
        routeEntriesArray.entry.regularEntry.nextHopVlanId1        = IP_VLAN_TESTED_VLAN_ID_CNS;
        routeEntriesArray.entry.regularEntry.nextHopARPPointer     = 0;
        routeEntriesArray.entry.regularEntry.nextHopTunnelPointer  = 0;
        routeEntriesArray.entry.regularEntry.nextHopNatPointer     = 0;

        routeEntriesArray.entry.regularEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
        routeEntriesArray.entry.regularEntry.qosProfileMarkingEnable         = GT_FALSE;
        routeEntriesArray.entry.regularEntry.appSpecificCpuCodeEnable        = GT_FALSE;
        routeEntriesArray.entry.regularEntry.unicastPacketSipFilterEnable    = GT_FALSE;

        routeEntriesArray.entry.regularEntry.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.hwDevNum  = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId         = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.vidx            = 0;
        routeEntriesArray.entry.regularEntry.nextHopInterface.vlanId          = 0;

        numOfEntries = 1;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "cpssDxChIpUcRouteEntriesWrite: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                           dev, entryIndex, numOfEntries);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "cpssDxChIpUcRouteEntriesWrite: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                       dev, entryIndex, numOfEntries);
        }

        /*
            1.2. Call function with baseRouteEntryIndex [0], non-NULL  routeEntriesArray,
                 numOfRouteEntries [1].
            Expected: GT_OK.
        */
        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArray, numOfEntries);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                             dev, entryIndex, numOfEntries);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                             dev, entryIndex, numOfEntries);
        }

        /*
            1.3. Check reading at edge index. Call function with
            baseRouteEntryIndex [tableSize - 1], non-NULL routeEntriesArray, numOfRouteEntries [1].
            Expected: GT_OK.
        */
        entryIndex = tableSize - 1;

        st = cpssDxChIpUcRouteEntriesWrite(dev, entryIndex, &routeEntriesArray, numOfEntries);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                       "cpssDxChIpUcRouteEntriesWrite: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                           dev, entryIndex, numOfEntries);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "cpssDxChIpUcRouteEntriesWrite: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                       dev, entryIndex, numOfEntries);
        }

        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArray, numOfEntries);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                     dev, entryIndex, numOfEntries);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                     dev, entryIndex, numOfEntries);
        }

        /*
            1.4. Call function with baseRouteEntryIndex [tableSize], other params same as in 1.2.
            Expected: NOT GT_OK.
        */
        entryIndex = tableSize;

        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArray, numOfEntries);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                 "%d, baseRouteEntryIndex = %d", dev, entryIndex);
        entryIndex = 0;

        /*
            1.5. Call function with baseRouteEntryIndex [tableSize + numOfEntries],
            other params same as in 1.2.
            Expected: NOT GT_OK
        */
        entryIndex = tableSize + numOfEntries;

        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d",
                                         dev, entryIndex);
        entryIndex = 0;

        /*
            1.6. Call function with routeEntriesArray [NULL], other params same as in 1.2.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, NULL, numOfEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, routeEntriesArray = NULL", dev);

        /*
            1.7. Call function with entryIndex [0], numOfEntries [tableSize]
            and other params same as in 1.2.
            Expected: NOT GT_OK
        */
        entryIndex = 0;
        numOfEntries = tableSize+1;

        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArray, numOfEntries);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d",
                                         dev, entryIndex);

        numOfEntries = 0;
    }

    entryIndex   = 0;
    numOfEntries = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    routeEntriesArray.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

    st = cpssDxChIpUcRouteEntriesRead(dev, entryIndex, &routeEntriesArray, numOfEntries);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          routeEntryIndex,
    IN CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMcRouteEntriesWrite)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with routeEntryIndex [0],
        routeEntryPtr [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                        cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                        appSpecificCpuCodeEnable = GT_FALSE;
                        ttlHopLimitDecEnable = GT_TRUE;
                        ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                        ingressMirror = GT_TRUE;
                        ingressMirrorToAnalyzerIndex = 0;
                        qosProfileMarkingEnable = GT_FALSE;
                        qosProfileIndex = 0;
                        qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                        modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                        modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                        countSet = CPSS_IP_CNT_SET0_E;
                        multicastRPFCheckEnable = GT_TRUE;
                        multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E;
                        multicastRPFVlan = 100;
                        multicastRPFRoutingSharedTreeIndex = 0;
                        multicastRPFFailCommandMode =
                            CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
                        RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                        scopeCheckingEnable = GT_FALSE;
                        siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                        mtuProfileIndex = 0;
                        internalMLLPointer = 0;
                        externalMLLPointer = 0}].
    Expected: GT_OK.
    1.2. Call cpssDxChIpMcRouteEntriesRead with the same
    routeEntryIndex [0] and non-NULL  routeEntryPtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with wrong routeEntryPtr [cmd = wrong enum values],
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong enum routeEntryPtr [cpuCodeIdx = wrong enum values],
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call function with routeEntryPtr [qosPrecedence = wrong enum values],
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call function with routeEntryPtr [modifyUp = wrong enum values],
    other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call function with routeEntryPtr [modifyDscp = wrong enum values],
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.8. Call function with routeEntryPtr [countSet = wrong enum values],
         other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call function with routeEntryPtr
    [multicastRPFFailCommandMode = wrong enum values], other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call function with routeEntryPtr [RPFFailCommand = wrong enum values],
                             other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.11. Call function with routeEntryPtr [siteId = wrong enum values],
    other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.12. Call function with routeEntryPtr [multicastIngressVlanCheck = wrong enum values],
    other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.13. Call function with routeEntryPtr [mtuProfileIndex = 8],
                             other params same as in 1.1.
    Expected: NON GT_OK.
    1.14. Call function with routeEntryPtr[ingressMirrorToAnalyzerIndex = 7],
    other params same as in 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.15. Call function with routeEntryPtr[multicastRPFRoutingSharedTreeIndex = 64],                           .
    other params same as in 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.16. Call function with routeEntryIndex [tableSize], other params same as in 1.1.
    Expected: NOT GT_OK
    1.17. Call function with routeEntryPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st         = GT_OK;
    GT_U8                           dev;
    GT_U32                          entryIndex = 0;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntry;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntryGet;
    GT_BOOL                         isEqual    = GT_FALSE;
    GT_U32                          tableSize  = 0;

    cpssOsBzero ((GT_VOID*)&routeEntry, sizeof(routeEntry));
    cpssOsMemSet((GT_VOID*)&routeEntryGet,0,sizeof(routeEntryGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop;

        /*
            1.1. Call function with routeEntryIndex [0],
                routeEntryPtr [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                                cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                                appSpecificCpuCodeEnable = GT_FALSE;
                                ttlHopLimitDecEnable = GT_TRUE;
                                ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                                ingressMirror = GT_TRUE;
                                ingressMirrorToAnalyzerIndex = 0;
                                qosProfileMarkingEnable = GT_FALSE;
                                qosProfileIndex = 0;
                                qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                                modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                countSet = CPSS_IP_CNT_SET0_E;
                                multicastRPFCheckEnable = GT_TRUE;
                                multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E;
                                multicastRPFVlan = 100;
                                multicastRPFRoutingSharedTreeIndex = 0;
                                multicastRPFFailCommandMode =
                                    CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
                                RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                                scopeCheckingEnable = GT_FALSE;
                                siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                                mtuProfileIndex = 0;
                                internalMLLPointer = 0;
                                externalMLLPointer = 0}].
            Expected: GT_OK.
        */
        entryIndex = 0;

        routeEntry.cmd                      = CPSS_PACKET_CMD_DROP_SOFT_E;
        routeEntry.cpuCodeIdx               = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
        routeEntry.appSpecificCpuCodeEnable = GT_FALSE;
        routeEntry.ttlHopLimitDecEnable     = GT_TRUE;
        routeEntry.ingressMirror            = GT_FALSE;
        routeEntry.ingressMirrorToAnalyzerIndex = 0;
        routeEntry.qosProfileMarkingEnable  = GT_FALSE;
        routeEntry.qosProfileIndex          = 0;
        routeEntry.qosPrecedence            = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        routeEntry.modifyUp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntry.modifyDscp               = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntry.countSet                 = CPSS_IP_CNT_SET0_E;
        routeEntry.multicastRPFCheckEnable  = GT_TRUE;
        routeEntry.multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E;
        routeEntry.multicastRPFVlan         = IP_VLAN_TESTED_VLAN_ID_CNS;
        routeEntry.multicastRPFRoutingSharedTreeIndex = 0;
        routeEntry.RPFFailCommand           = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        routeEntry.scopeCheckingEnable      = GT_FALSE;
        routeEntry.siteId                   = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntry.mtuProfileIndex          = 0;
        routeEntry.internalMLLPointer       = 0;
        routeEntry.externalMLLPointer       = 0;
        routeEntry.multicastRPFFailCommandMode     =
                                CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
        routeEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;

        st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        /*
            1.2. Call cpssDxChIpMcRouteEntriesRead with the same
            routeEntryIndex [0] and non-NULL  routeEntryPtr.
            Expected: GT_OK and same values as written.
        */

        st = cpssDxChIpMcRouteEntriesRead(dev, entryIndex, &routeEntryGet);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMcRouteEntriesRead: %d, %d",
                                             dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMcRouteEntriesRead: %d, %d",
                                         dev, entryIndex);

            if (GT_OK == st)
            {
                /* Verifying routeEntryPtr fields */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &routeEntry,
                                             (GT_VOID*) &routeEntryGet,
                                             sizeof(routeEntry))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                             "get another routeEntry than was set: %d", dev);
            }
        }

        /*
            1.3. Call function with wrong routeEntryPtr [cmd = wrong enum values],
                 other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                               (dev, entryIndex, &routeEntry),
                               routeEntry.cmd);
        }

        /*
            1.4. Call function with wrong enum routeEntryPtr [cpuCodeIdx = wrong enum values],
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                               (dev, entryIndex, &routeEntry),
                               routeEntry.cpuCodeIdx);
        }

        /*
            1.5. Call function with routeEntryPtr [qosPrecedence = wrong enum values],
                 other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                               (dev, entryIndex, &routeEntry),
                               routeEntry.qosPrecedence);
        }

        /*
            1.6. Call function with routeEntryPtr [modifyUp = wrong enum values],
            other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                               (dev, entryIndex, &routeEntry),
                               routeEntry.modifyUp);
        }

        /*
            1.7. Call function with routeEntryPtr [modifyDscp = wrong enum values],
                 other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                               (dev, entryIndex, &routeEntry),
                               routeEntry.modifyDscp);
        }

        /*
            1.8. Call function with routeEntryPtr [countSet = wrong enum values],
                 other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                               (dev, entryIndex, &routeEntry),
                               routeEntry.countSet);
        }

        /*
            1.9. Call function with routeEntryPtr
            [multicastRPFFailCommandMode = wrong enum values], other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                                (dev, entryIndex, &routeEntry),
                                routeEntry.multicastRPFFailCommandMode);
        }

        /*
            1.10. Call function with routeEntryPtr [RPFFailCommand = wrong enum values],
                                     other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                                (dev, entryIndex, &routeEntry),
                                routeEntry.RPFFailCommand);
        }

        /*
            1.11. Call function with routeEntryPtr [siteId = wrong enum values],
            other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                                (dev, entryIndex, &routeEntry),
                                routeEntry.siteId);
        }

        /*
            1.12. Call function with routeEntryPtr [multicastIngressVlanCheck = wrong enum values],
            other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            if (!prvUtfIsPbrModeUsed())
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChIpMcRouteEntriesWrite
                                    (dev, entryIndex, &routeEntry),
                                    routeEntry.multicastIngressVlanCheck);
            }
        }

        /*
            1.13. Call function with routeEntryPtr [mtuProfileIndex = 8],
                                     other params same as in 1.1.
            Expected: NON GT_OK.
        */
        routeEntry.siteId          = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntry.mtuProfileIndex = 8;

        st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, routeEntry->mtuProfileIndex = %d",
                                     dev, routeEntry.mtuProfileIndex);

        routeEntry.mtuProfileIndex = 0;

        if((PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            /*
                1.14. Call function with routeEntryPtr[ingressMirrorToAnalyzerIndex = 7],                           .
                other params same as in 1.1.
                Expected: GT_OUT_OF_RANGE.
            */
            routeEntry.ingressMirror = GT_TRUE;
            routeEntry.ingressMirrorToAnalyzerIndex = PRV_CPSS_DXCH_IP_MIRROR_ANALYZER_MAX_INDEX_CNS + 1;

            st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                        "%d, routeEntry.ingressMirrorToAnalyzerIndex = %d",
                                     dev, routeEntry.ingressMirrorToAnalyzerIndex);

            routeEntry.ingressMirrorToAnalyzerIndex = 0;
            routeEntry.ingressMirror = GT_FALSE;

            /*
                1.15. Call function with routeEntryPtr[multicastRPFRoutingSharedTreeIndex = 64],                           .
                other params same as in 1.1.
                Expected: GT_OUT_OF_RANGE.
            */
            routeEntry.multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E;
            routeEntry.multicastRPFRoutingSharedTreeIndex = PRV_CPSS_DXCH_IP_SHARED_TREE_MAX_INDEX_CNS + 1;

            st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                        "%d, routeEntry.multicastRPFRoutingSharedTreeIndex = %d",
                                     dev, routeEntry.multicastRPFRoutingSharedTreeIndex);

            routeEntry.multicastRPFRoutingSharedTreeIndex = 0;
            routeEntry.multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E;
        }

        /*
            1.16. Call function with routeEntryIndex [tableSize], other params same as in 1.1.
            Expected: NOT GT_OK
        */
        entryIndex = tableSize;

        st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, baseRouteEntryIndex = %d",
                         dev, entryIndex);

        entryIndex = 0;

        /*
            1.17. Call function with routeEntryPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, routeEntryPtr = NULL", dev);
    }

    entryIndex = 0;

    routeEntry.cmd                      = CPSS_PACKET_CMD_DROP_SOFT_E;
    routeEntry.cpuCodeIdx               = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
    routeEntry.appSpecificCpuCodeEnable = GT_FALSE;
    routeEntry.ttlHopLimitDecEnable     = GT_TRUE;
    routeEntry.ingressMirror            = GT_FALSE;
    routeEntry.ingressMirrorToAnalyzerIndex = 0;
    routeEntry.qosProfileMarkingEnable  = GT_FALSE;
    routeEntry.qosProfileIndex          = 0;
    routeEntry.qosPrecedence            = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    routeEntry.modifyUp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntry.modifyDscp               = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntry.countSet                 = CPSS_IP_CNT_SET0_E;
    routeEntry.multicastRPFCheckEnable  = GT_FALSE;
    routeEntry.multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E;
    routeEntry.multicastRPFVlan         = IP_VLAN_TESTED_VLAN_ID_CNS;
    routeEntry.multicastRPFRoutingSharedTreeIndex = 0;
    routeEntry.RPFFailCommand           = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    routeEntry.scopeCheckingEnable      = GT_FALSE;
    routeEntry.siteId                   = CPSS_IP_SITE_ID_INTERNAL_E;
    routeEntry.mtuProfileIndex          = 0;
    routeEntry.internalMLLPointer       = 0;
    routeEntry.externalMLLPointer       = 0;
    routeEntry.multicastRPFFailCommandMode     = CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
    routeEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMcRouteEntriesRead
(
    IN GT_U8                            devNum,
    IN GT_U32                           routeEntryIndex,
    OUT CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMcRouteEntriesRead)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Write test entry.
         Call cpssDxChIpMcRouteEntriesWrite with routeEntryIndex [0],
         routeEntryPtr [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                         cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                         appSpecificCpuCodeEnable = GT_FALSE;
                         ttlHopLimitDecEnable = GT_TRUE;
                         ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                         ingressMirror = GT_TRUE;
                         ingressMirrorToAnalyzerIndex = 0;
                         qosProfileMarkingEnable = GT_FALSE;
                         qosProfileIndex = 0;
                         qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                         modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                         modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                         countSet = CPSS_IP_CNT_SET0_E;
                         multicastRPFCheckEnable = GT_FALSE;
                         multicastIngressVlanCheck =
                            CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E;
                         multicastRPFVlan = 100;
                         multicastRPFRoutingSharedTreeIndex = 0;
                         multicastRPFFailCommandMode =
                            CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
                         RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                         scopeCheckingEnable = GT_FALSE;
                         siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                         mtuProfileIndex = 0;
                         internalMLLPointer = 0;
                         externalMLLPointer = 0 } ].
    Expected: GT_OK.
    1.2. Call function with routeEntryIndex [0], non-NULL  routeEntryPtr.
    Expected: GT_OK.
    1.3. Check reading at unused index.
    Call function with routeEntryIndex [1000 / 4095], non-NULL  routeEntryPtr.
    Expected:  GT_OK.
    1.4. Call function with routeEntryIndex [tableSize], other params same as in 1.2.
    Expected: NOT GT_OK
    1.5. Call function with routeEntryPtr [NULL], other params same as in 1.2.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st         = GT_OK;
    GT_U8                           dev;
    GT_U32                          entryIndex = 0;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntry;
    GT_U32                          tableSize  = 0;

    cpssOsBzero ((GT_VOID*)&routeEntry, sizeof(routeEntry));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)-> fineTuning.tableSize.routerNextHop;

        /*
            1.1. Write test entry.
                 Call cpssDxChIpMcRouteEntriesWrite with routeEntryIndex [0],
                 routeEntryPtr [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                                 cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                                 appSpecificCpuCodeEnable = GT_FALSE;
                                 ttlHopLimitDecEnable = GT_TRUE;
                                 ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                                 ingressMirror = GT_TRUE;
                                 ingressMirrorToAnalyzerIndex = 0;
                                 qosProfileMarkingEnable = GT_FALSE;
                                 qosProfileIndex = 0;
                                 qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                                 modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                 modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                 countSet = CPSS_IP_CNT_SET0_E;
                                 multicastRPFCheckEnable = GT_FALSE;
                                 multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E;
                                 multicastRPFVlan = 100;
                                 multicastRPFRoutingSharedTreeIndex = 0;
                                 multicastRPFFailCommandMode =
                                    CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
                                 RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                                 scopeCheckingEnable = GT_FALSE;
                                 siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                                 mtuProfileIndex = 0;
                                 internalMLLPointer = 0;
                                 externalMLLPointer = 0 } ].
            Expected: GT_OK.
        */
        entryIndex = 0;

        routeEntry.cmd                      = CPSS_PACKET_CMD_DROP_SOFT_E;
        routeEntry.cpuCodeIdx               = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
        routeEntry.appSpecificCpuCodeEnable = GT_FALSE;
        routeEntry.ttlHopLimitDecEnable     = GT_TRUE;
        routeEntry.ingressMirror            = GT_FALSE;
        routeEntry.ingressMirrorToAnalyzerIndex = 0;
        routeEntry.qosProfileMarkingEnable  = GT_FALSE;
        routeEntry.qosProfileIndex          = 0;
        routeEntry.qosPrecedence            = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        routeEntry.modifyUp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntry.modifyDscp               = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        routeEntry.countSet                 = CPSS_IP_CNT_SET0_E;
        routeEntry.multicastRPFCheckEnable  = GT_FALSE;
        routeEntry.multicastIngressVlanCheck = CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E;
        routeEntry.multicastRPFVlan         = IP_VLAN_TESTED_VLAN_ID_CNS;
        routeEntry.multicastRPFRoutingSharedTreeIndex = 0;
        routeEntry.RPFFailCommand           = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        routeEntry.scopeCheckingEnable      = GT_FALSE;
        routeEntry.siteId                   = CPSS_IP_SITE_ID_INTERNAL_E;
        routeEntry.mtuProfileIndex          = 0;
        routeEntry.internalMLLPointer       = 0;
        routeEntry.externalMLLPointer       = 0;
        routeEntry.multicastRPFFailCommandMode     = CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
        routeEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;

        st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMcRouteEntriesWrite: %d, %d",
                                             dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMcRouteEntriesWrite: %d, %d",
                                         dev, entryIndex);
        }

        /*
            1.2. Call function with routeEntryIndex [0], non-NULL  routeEntryPtr.
            Expected: GT_OK.
        */
        entryIndex = 0;

        st = cpssDxChIpMcRouteEntriesRead(dev, entryIndex, &routeEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        /*
            1.3. Check reading at end of table.
            Call function with routeEntryIndex [tableSize - 1], non-NULL  routeEntryPtr.
            Expected:  GT_OK.
        */
        /* Call with routeEntryIndex [tableSize - 1] */
        entryIndex = tableSize - 1;

        st = cpssDxChIpMcRouteEntriesWrite(dev, entryIndex, &routeEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMcRouteEntriesWrite: %d, %d",
                                     dev, entryIndex);

        st = cpssDxChIpMcRouteEntriesRead(dev, entryIndex, &routeEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        /*
            1.4. Call function with routeEntryIndex [tableSize], other params same as in 1.2.
            Expected: NOT GT_OK
        */
        entryIndex = tableSize;

        st = cpssDxChIpMcRouteEntriesRead(dev, entryIndex, &routeEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

        entryIndex = 0;

        /*
            1.5. Call function with routeEntryPtr [NULL], other params same as in 1.2.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpMcRouteEntriesRead(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, routeEntryPtr = NULL", dev);
    }

    entryIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        cpssOsMemSet((GT_VOID*)&routeEntry,0,sizeof(routeEntry));
        st = cpssDxChIpMcRouteEntriesRead(dev, entryIndex, &routeEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    cpssOsMemSet((GT_VOID*)&routeEntry,0,sizeof(routeEntry));
    st = cpssDxChIpMcRouteEntriesRead(dev, entryIndex, &routeEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterNextHopTableAgeBitsEntryWrite
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    IN  GT_U32  routerNextHopTableAgeBitsEntry
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterNextHopTableAgeBitsEntryWrite)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with routerNextHopTableAgeBitsEntryIndex = (tableSize / 32),
                            routerNextHopTableAgeBitsEntry [5].
    Expected GT_OK.
    1.2. Call cpssDxChIpRouterNextHopTableAgeBitsEntryRead with
    routerNextHopTableAgeBitsEntryIndex [0], non-NULL routerNextHopTableAgeBitsEntry.
    Expected: GT_OK and same value as written.
    1.3. Call function with routerNextHopTableAgeBitsEntryIndex [tableSize],
                            routerNextHopTableAgeBitsEntry [0].
    Expected: NOT GT_OK
    1.4. Call function with routerNextHopTableAgeBitsEntryIndex [0],
                            routerNextHopTableAgeBitsEntry [0xFFFFFFFE].
    Expected GT_OK.
*/
    GT_STATUS   st           = GT_OK;
    GT_U8       dev;
    GT_U32      entryIndex   = 0;
    GT_U32      bitsEntry    = 0;
    GT_U32      bitsEntryGet = 0;
    GT_U32      tableSize    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)-> fineTuning.tableSize.routerNextHop;

        /*
            1.1. Call function with routerNextHopTableAgeBitsEntryIndex = (tableSize / 32),
                                    routerNextHopTableAgeBitsEntry [5].
            Expected GT_OK.
        */
        bitsEntry  = 5;

        /* Call with routerNextHopTableAgeBitsEntryIndex [0] */
        entryIndex = 0;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryWrite(dev, entryIndex, bitsEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex, bitsEntry);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex, bitsEntry);
        }

        /* Call with routerNextHopTableAgeBitsEntryIndex [tableSize/32 - 1] */
        entryIndex = tableSize / 32 - 1;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryWrite(dev, entryIndex, bitsEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex, bitsEntry);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex, bitsEntry);
        }

        /*
            1.2. Call cpssDxChIpRouterNextHopTableAgeBitsEntryRead with
            routerNextHopTableAgeBitsEntryIndex [0], non-NULL routerNextHopTableAgeBitsEntry.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(dev, entryIndex, &bitsEntryGet);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChIpRouterNextHopTableAgeBitsEntryRead: %d, %d", dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChIpRouterNextHopTableAgeBitsEntryRead: %d, %d", dev, entryIndex);

            if (GT_OK == st)
            {
                /* Verifying routeEntryPtr fields */
                UTF_VERIFY_EQUAL1_STRING_MAC(bitsEntry, bitsEntryGet,
                           "get another routerNextHopTableAgeBitsEntry than was set: %d", dev);
            }
        }


        /*
            1.3. Call function with routerNextHopTableAgeBitsEntryIndex [tableSize],
                                    routerNextHopTableAgeBitsEntry [0].
            Expected: NOT GT_OK
        */
        entryIndex = tableSize / 32;
        bitsEntry  = 0;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryWrite(dev, entryIndex, bitsEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

        entryIndex = tableSize / 32 - 1;

        /*
            1.4. Call function with routerNextHopTableAgeBitsEntryIndex [0],
                                    routerNextHopTableAgeBitsEntry [0xFFFFFFFE].
            Expected GT_OK.
        */
        entryIndex = 0;
        bitsEntry  = 0xFFFFFFFE;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryWrite(dev, entryIndex, bitsEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, routerNextHopTableAgeBitsEntry = %d", dev, bitsEntry);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, routerNextHopTableAgeBitsEntry = %d", dev, bitsEntry);
        }
    }

    bitsEntry  = 5;
    entryIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterNextHopTableAgeBitsEntryWrite(dev, entryIndex, bitsEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterNextHopTableAgeBitsEntryWrite(dev, entryIndex, bitsEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterNextHopTableAgeBitsEntryRead
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    OUT GT_U32  *routerNextHopTableAgeBitsEntry
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterNextHopTableAgeBitsEntryRead)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with routerNextHopTableAgeBitsEntryIndex [0 / 128],
                            non-NULL  routerNextHopTableAgeBitsEntry.
    Expected: GT_OK.
    1.2. Call function with routerNextHopTableAgeBitsEntryIndex [256],
                            other params same as in 1.1.
    Expected: GT_OK for DxCh3 and NOT GT_OK for DxCh2,xCat.
    1.3. Call function with routerNextHopTableAgeBitsEntryIndex [500],
                            other params same as in 1.1.
    Expected: NOT GT_OK
    1.4. Call function with routerNextHopTableAgeBitsEntryIndex [0],
                                    routerNextHopTableAgeBitsEntry [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st           = GT_OK;
    GT_U8       dev;
    GT_U32      entryIndex   = 0;
    GT_U32      bitsEntry    = 0;
    GT_U32      tableSize    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)-> fineTuning.tableSize.routerNextHop;

        /*
            1.1. Call function with routerNextHopTableAgeBitsEntryIndex [0 / (tableSize/32 -1)],
                                    non-NULL  routerNextHopTableAgeBitsEntry.
            Expected: GT_OK.
        */
        /* Call with routerNextHopTableAgeBitsEntryIndex [0] */
        entryIndex = 0;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(dev, entryIndex, &bitsEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        /* Call with routerNextHopTableAgeBitsEntryIndex (tableSize/32 -1) */
        entryIndex = tableSize / 32 - 1;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(dev, entryIndex, &bitsEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        /*
            1.2. Call function with routerNextHopTableAgeBitsEntryIndex [tableSize/32],
                                    other params same as in 1.1.
            Expected: NOT GT_OK
        */
        entryIndex = tableSize / 32;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(dev, entryIndex, &bitsEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

        entryIndex = tableSize / 32 - 1;

        /*
            1.3. Call function with routerNextHopTableAgeBitsEntryIndex [0],
                                    routerNextHopTableAgeBitsEntry [NULL].
            Expected: GT_BAD_PTR.
        */
        entryIndex = 0;

        st = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                "%d, routerNextHopTableAgeBitsEntry = NULL", dev);
    }

    entryIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(dev, entryIndex, &bitsEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(dev, entryIndex, &bitsEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMLLPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_IP_MLL_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm,
    IN CPSS_DXCH_IP_MLL_PAIR_STC                    *mllPairEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMLLPairWrite)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with mllPairEntryIndex [0],
            mllPairWriteForm [CPSS_DXCH_IP_MLL_PAIR_READ_WRITE_WHOLE_E],
            mllPairEntryPtr [
                {firstMllNode {mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
                               isTunnelStart = GT_FALSE;
                               nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                 devPort {devNum = 0; portNum = 0};
                                                 trunkId = 0;
                                                 vidx = 0;
                                                 vlanId = 100 };
                               nextHopVlanId = 100;
                               nextHopTunnelPointer = 0;
                               ttlHopLimitThreshold = 25;
                               excludeSrcVlan = GT_FALSE;
                               last = GT_FALSE },
                     secondMllNode { same as firstMllNode },
                     nextPointer [0] } ].
    Expected: GT_OK.
    1.2. Call cpssDxChIpMLLPairRead with mllPairEntryIndex [0], non-NULL  mllPairEntryPtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with out of range mllPairEntryIndex [tableSize],
                            other params same as in 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with wrong enum mllPairWriteForm,
                        other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong mllPairEntry.firstMllNode.nextHopInterface.type
               enum values and other valid params.
    Expected: GT_BAD_PARAM.
    1.6. Call function with mllPairEntryPtr [NULL],
                            other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                            st               = GT_OK;
    GT_U8                                dev;
    GT_U32                               entryIndex       = 0;

    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm =
                                        CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

    CPSS_DXCH_IP_MLL_PAIR_STC            mllPairEntry;
    CPSS_DXCH_IP_MLL_PAIR_STC            mllPairEntryGet;

    cpssOsBzero((GT_VOID*)&mllPairEntry, sizeof(mllPairEntry));
    cpssOsBzero((GT_VOID*)&mllPairEntryGet, sizeof(mllPairEntryGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mllPairEntryIndex [0],
                    mllPairWriteForm [CPSS_DXCH_IP_MLL_PAIR_READ_WRITE_WHOLE_E],
                    mllPairEntryPtr [
                        {firstMllNode {mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
                                       isTunnelStart = GT_FALSE;
                                       nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                         devPort {devNum = 0; portNum = 0};
                                                         trunkId = 0;
                                                         vidx = 0;
                                                         vlanId = 100 };
                                       nextHopVlanId = 100;
                                       nextHopTunnelPointer = 0;
                                       ttlHopLimitThreshold = 25;
                                       excludeSrcVlan = GT_FALSE;
                                       last = GT_FALSE },
                             secondMllNode { same as firstMllNode },
                             nextPointer [0] } ].
            Expected: GT_OK.
        */
        entryIndex       = 0;
        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

        mllPairEntry.firstMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
        mllPairEntry.firstMllNode.isTunnelStart     = GT_FALSE;

        mllPairEntry.firstMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum  = 0;
        mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum = 0;
        mllPairEntry.firstMllNode.nextHopInterface.trunkId         = 0;
        mllPairEntry.firstMllNode.nextHopInterface.vidx            = 0;
        mllPairEntry.firstMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

        mllPairEntry.firstMllNode.nextHopVlanId        = IP_VLAN_TESTED_VLAN_ID_CNS;
        mllPairEntry.firstMllNode.nextHopTunnelPointer = 0;
        mllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        mllPairEntry.firstMllNode.ttlHopLimitThreshold = 25;
        mllPairEntry.firstMllNode.excludeSrcVlan       = GT_FALSE;
        mllPairEntry.firstMllNode.last                 = GT_FALSE;

        mllPairEntry.secondMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
        mllPairEntry.secondMllNode.isTunnelStart     = GT_TRUE;

        mllPairEntry.secondMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum  = 0;
        mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum = 0;
        mllPairEntry.secondMllNode.nextHopInterface.trunkId         = 0;
        mllPairEntry.secondMllNode.nextHopInterface.vidx            = 0;
        mllPairEntry.secondMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

        mllPairEntry.secondMllNode.nextHopVlanId        = IP_VLAN_TESTED_VLAN_ID_CNS;
        mllPairEntry.secondMllNode.nextHopTunnelPointer = 0;
        mllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        mllPairEntry.secondMllNode.ttlHopLimitThreshold = 25;
        mllPairEntry.secondMllNode.excludeSrcVlan       = GT_FALSE;
        mllPairEntry.secondMllNode.last                 = GT_FALSE;

        mllPairEntry.nextPointer = 0;

        st = cpssDxChIpMLLPairWrite(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex, mllPairWriteForm);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex, mllPairWriteForm);
        }

        /*
            1.2. Call cpssDxChIpMLLPairRead with mllPairEntryIndex [0], non-NULL  mllPairEntryPtr.
            Expected: GT_OK and same values as written.
        */

        st = cpssDxChIpMLLPairRead(dev, entryIndex, mllPairWriteForm, &mllPairEntryGet);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMLLPairRead: %d, %d",
                                             dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMLLPairRead: %d, %d",
                                         dev, entryIndex);

            if (GT_OK == st)
            {
            /* Verifying routeEntryPtr fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.mllRPFFailCommand,
                                         mllPairEntryGet.firstMllNode.mllRPFFailCommand,
                "get another mllPairEntryPtr->firstMllNode.mllRPFFailCommand than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.isTunnelStart,
                                         mllPairEntryGet.firstMllNode.isTunnelStart,
                "get another mllPairEntryPtr->firstMllNode.isTunnelStart than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopVlanId,
                                         mllPairEntryGet.firstMllNode.nextHopVlanId,
                "get another mllPairEntryPtr->firstMllNode.nextHopVlanId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopTunnelPointer,
                                         mllPairEntryGet.firstMllNode.nextHopTunnelPointer,
                "get another mllPairEntryPtr->firstMllNode.nextHopTunnelPointer than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.ttlHopLimitThreshold,
                                         mllPairEntryGet.firstMllNode.ttlHopLimitThreshold,
                "get another mllPairEntryPtr->firstMllNode.ttlHopLimitThreshold than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.excludeSrcVlan,
                                         mllPairEntryGet.firstMllNode.excludeSrcVlan,
                "get another mllPairEntryPtr->firstMllNode.excludeSrcVlan than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.last,
                                         mllPairEntryGet.firstMllNode.last,
                "get another mllPairEntryPtr->firstMllNode.last than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopInterface.type,
                                         mllPairEntryGet.firstMllNode.nextHopInterface.type,
                "get another mllPairEntryPtr->firstMllNode.nextHopInterface.type than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum,
                                         mllPairEntryGet.firstMllNode.nextHopInterface.devPort.hwDevNum,
                "get another mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.hwDevNum than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum,
                                         mllPairEntryGet.firstMllNode.nextHopInterface.devPort.portNum,
                "get another mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.portNum than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.mllRPFFailCommand,
                                         mllPairEntryGet.secondMllNode.mllRPFFailCommand,
                "get another mllPairEntryPtr->secondMllNode.mllRPFFailCommand than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.isTunnelStart,
                                         mllPairEntryGet.secondMllNode.isTunnelStart,
                "get another mllPairEntryPtr->secondMllNode.isTunnelStart than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopVlanId,
                                         mllPairEntryGet.secondMllNode.nextHopVlanId,
                "get another mllPairEntryPtr->secondMllNode.nextHopVlanId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopTunnelPointer,
                                         mllPairEntryGet.secondMllNode.nextHopTunnelPointer,
                "get another mllPairEntryPtr->secondMllNode.nextHopTunnelPointer than was set: %d", dev);
            if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH2_E)
                UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.tunnelStartPassengerType,
                                             mllPairEntryGet.secondMllNode.tunnelStartPassengerType,
                    "get another mllPairEntryPtr->secondMllNode.tunnelStartPassengerType than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.ttlHopLimitThreshold,
                                         mllPairEntryGet.secondMllNode.ttlHopLimitThreshold,
                "get another mllPairEntryPtr->secondMllNode.ttlHopLimitThreshold than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.excludeSrcVlan,
                                         mllPairEntryGet.secondMllNode.excludeSrcVlan,
                "get another mllPairEntryPtr->secondMllNode.excludeSrcVlan than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.last,
                                         mllPairEntryGet.secondMllNode.last,
                "get another mllPairEntryPtr->secondMllNode.last than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopInterface.type,
                                         mllPairEntryGet.secondMllNode.nextHopInterface.type,
                "get another mllPairEntryPtr->secondMllNode.nextHopInterface.type than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum,
                                         mllPairEntryGet.secondMllNode.nextHopInterface.devPort.hwDevNum,
                "get another mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.hwDevNum than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum,
                                         mllPairEntryGet.secondMllNode.nextHopInterface.devPort.portNum,
                "get another mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.portNum than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.nextPointer, mllPairEntryGet.nextPointer,
                "get another mllPairEntryPtr->nextPointer than was set: %d", dev);
            }
        }

        /*
            1.3. Call function with out of range mllPairEntryIndex [tableSize],
                                    other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs;

        st = cpssDxChIpMLLPairWrite(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex, mllPairWriteForm);

        entryIndex = 0;

        /*
            1.4. Call function with wrong enum mllPairWriteForm,
                                other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMLLPairWrite
                                (dev, entryIndex, mllPairWriteForm, &mllPairEntry),
                                mllPairWriteForm);
        }

        /*
            1.5. Call with wrong mllPairEntry.firstMllNode.nextHopInterface.type
                       enum values and other valid params.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMLLPairWrite
                                (dev, entryIndex, mllPairWriteForm, &mllPairEntry),
                                mllPairEntry.firstMllNode.nextHopInterface.type);
        }

        /*
            1.6. Call function with mllPairEntryPtr [NULL],
                                    other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

        st = cpssDxChIpMLLPairWrite(dev, entryIndex, mllPairWriteForm, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mllPairEntryPtr = NULL", dev);
    }

    entryIndex       = 0;
    mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

    cpssOsBzero((GT_VOID*)&mllPairEntry, sizeof(mllPairEntry));

    mllPairEntry.firstMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
    mllPairEntry.firstMllNode.isTunnelStart     = GT_FALSE;

    mllPairEntry.firstMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum  = 0;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum = 0;
    mllPairEntry.firstMllNode.nextHopInterface.trunkId         = 0;
    mllPairEntry.firstMllNode.nextHopInterface.vidx            = 0;
    mllPairEntry.firstMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

    mllPairEntry.firstMllNode.nextHopVlanId        = IP_VLAN_TESTED_VLAN_ID_CNS;
    mllPairEntry.firstMllNode.nextHopTunnelPointer = 0;
    mllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
    mllPairEntry.firstMllNode.ttlHopLimitThreshold = 25;
    mllPairEntry.firstMllNode.excludeSrcVlan       = GT_FALSE;
    mllPairEntry.firstMllNode.last                 = GT_FALSE;

    mllPairEntry.secondMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
    mllPairEntry.secondMllNode.isTunnelStart     = GT_FALSE;

    mllPairEntry.secondMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum  = 0;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum = 0;
    mllPairEntry.secondMllNode.nextHopInterface.trunkId         = 0;
    mllPairEntry.secondMllNode.nextHopInterface.vidx            = 0;
    mllPairEntry.secondMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

    mllPairEntry.secondMllNode.nextHopVlanId        = IP_VLAN_TESTED_VLAN_ID_CNS;
    mllPairEntry.secondMllNode.nextHopTunnelPointer = 0;
    mllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
    mllPairEntry.secondMllNode.ttlHopLimitThreshold = 25;
    mllPairEntry.secondMllNode.excludeSrcVlan       = GT_FALSE;
    mllPairEntry.secondMllNode.last                 = GT_FALSE;

    mllPairEntry.nextPointer = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMLLPairWrite(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMLLPairWrite(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMLLPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairReadForm,
    OUT CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMLLPairRead)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with mllPairEntryIndex [0],
            mllPairWriteForm [CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E],
            mllPairEntryPtr [
                {firstMllNode {mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
                                isTunnelStart = GT_FALSE;
                                nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                  devPort {devNum = 0; portNum = 0};
                                                  trunkId = 0;
                                                  vidx = 0;
                                                  vlanId = 100 };
                                nextHopVlanId = 100;
                                nextHopTunnelPointer = 0;
                                ttlHopLimitThreshold = 25;
                                excludeSrcVlan = GT_FALSE;
                                last = GT_FALSE },
                      secondMllNode { same as firstMllNode },
                      nextPointer [0] } ].
    Expected: GT_OK.
    1.2. Call function with mllPairEntryIndex [0], non-NULL  routeEntryPtr.
    Expected: GT_OK.
    1.3. Call function with out of range mllPairEntryIndex [tableSize], non-NULL  routeEntryPtr.
    Expected: NOT GT_OK.
    1.4. Call function with mllPairEntryPtr [NULL], other params same as in 1.2.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st         = GT_OK;
    GT_U8       dev;

    GT_U32      entryIndex = 0;

    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm =
                                            CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

    CPSS_DXCH_IP_MLL_PAIR_STC  mllPairEntry;

    cpssOsBzero((GT_VOID*)&mllPairEntry, sizeof(mllPairEntry));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mllPairEntryIndex [0],
                    mllPairWriteForm [CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E],
                    mllPairEntryPtr [
                        {firstMllNode {mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
                                        isTunnelStart = GT_FALSE;
                                        nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                          devPort {devNum = 0; portNum = 0};
                                                          trunkId = 0;
                                                          vidx = 0;
                                                          vlanId = 100 };
                                        nextHopVlanId = 100;
                                        nextHopTunnelPointer = 0;
                                        ttlHopLimitThreshold = 25;
                                        excludeSrcVlan = GT_FALSE;
                                        last = GT_FALSE },
                              secondMllNode { same as firstMllNode },
                              nextPointer [0] } ].
            Expected: GT_OK.
        */
        entryIndex       = 0;
        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

        mllPairEntry.firstMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
        mllPairEntry.firstMllNode.isTunnelStart     = GT_FALSE;

        mllPairEntry.firstMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum  = 0;
        mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum = 0;
        mllPairEntry.firstMllNode.nextHopInterface.trunkId         = 0;
        mllPairEntry.firstMllNode.nextHopInterface.vidx            = 0;
        mllPairEntry.firstMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

        mllPairEntry.firstMllNode.nextHopVlanId        = IP_VLAN_TESTED_VLAN_ID_CNS;
        mllPairEntry.firstMllNode.nextHopTunnelPointer = 0;
        mllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        mllPairEntry.firstMllNode.ttlHopLimitThreshold = 25;
        mllPairEntry.firstMllNode.excludeSrcVlan       = GT_FALSE;
        mllPairEntry.firstMllNode.last                 = GT_FALSE;

        mllPairEntry.secondMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
        mllPairEntry.secondMllNode.isTunnelStart     = GT_FALSE;

        mllPairEntry.secondMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum  = 0;
        mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum = 0;
        mllPairEntry.secondMllNode.nextHopInterface.trunkId         = 0;
        mllPairEntry.secondMllNode.nextHopInterface.vidx            = 0;
        mllPairEntry.secondMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

        mllPairEntry.secondMllNode.nextHopVlanId        = IP_VLAN_TESTED_VLAN_ID_CNS;
        mllPairEntry.secondMllNode.nextHopTunnelPointer = 0;
        mllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        mllPairEntry.secondMllNode.ttlHopLimitThreshold = 25;
        mllPairEntry.secondMllNode.excludeSrcVlan       = GT_FALSE;
        mllPairEntry.secondMllNode.last                 = GT_FALSE;

        mllPairEntry.nextPointer = 0;

        st = cpssDxChIpMLLPairWrite(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpMLLPairRead: %d, %d, %d",
                                             dev, entryIndex, mllPairWriteForm);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChIpMLLPairRead: %d, %d, %d",
                                         dev, entryIndex, mllPairWriteForm);
        }

        /*
            1.2. Call function with mllPairEntryIndex [0], non-NULL  routeEntryPtr.
            Expected: GT_OK.
        */
        entryIndex = 0;

        st = cpssDxChIpMLLPairRead(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        /*
            1.3. Call function with out of range mllPairEntryIndex [tableSize], non-NULL  routeEntryPtr.
            Expected: NOT GT_OK.
        */
        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs;

        st = cpssDxChIpMLLPairRead(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

        entryIndex = 0;

        /*
            1.4. Call function with mllPairEntryPtr [NULL], other params same as in 1.2.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpMLLPairRead(dev, entryIndex, mllPairWriteForm, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mllPairEntryPtr = NULL", dev);
    }

    entryIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMLLPairRead(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMLLPairRead(dev, entryIndex, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMLLLastBitWrite
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllPairEntryIndex,
    IN GT_BOOL                                          lastBit,
    IN CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT    mllEntryPart
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMLLLastBitWrite)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with mllPairEntryIndex [0],
                            lastBit [GT_FALSE],
                            mllEntryPart [CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E /
                                          CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E].
    Expected: GT_OK.
    1.2. Call function with out of range mllPairEntryIndex [1024],
                            other params same as in 1.1.
    Expected: NOT GT_OK.
    1.3. Call function with wrong mllEntryPart  enum values,
                            other params same as in 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st           = GT_OK;
    GT_U8      dev;
    GT_U32     entryIndex   = 0;
    GT_BOOL    lastBit      = GT_FALSE;

    CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT mllEntryPart =
                                CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mllPairEntryIndex [0],
                    lastBit [GT_FALSE],
                    mllEntryPart [CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E /
                                  CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E].
            Expected: GT_OK.
        */
        entryIndex = 0;
        lastBit    = GT_FALSE;

        /* Call with mllEntryPart [CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E] */
        mllEntryPart = CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E;

        st = cpssDxChIpMLLLastBitWrite(dev, entryIndex, lastBit, mllEntryPart);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, entryIndex, lastBit, mllEntryPart);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, entryIndex, lastBit, mllEntryPart);
        }

        /* Call with mllEntryPart [CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E] */
        mllEntryPart = CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E;

        st = cpssDxChIpMLLLastBitWrite(dev, entryIndex, lastBit, mllEntryPart);
        if (prvUtfIsPbrModeUsed())
        {
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, entryIndex, lastBit, mllEntryPart);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, entryIndex, lastBit, mllEntryPart);
        }

        /*
            1.2. Call function with out of range mllPairEntryIndex [tableSize],
                                    other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        entryIndex = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs;

        st = cpssDxChIpMLLLastBitWrite(dev, entryIndex, lastBit, mllEntryPart);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, entryIndex, lastBit, mllEntryPart);

        entryIndex = 0;

        /*
            1.3. Call function with wrong mllEntryPart enum values,
                                    other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        if (!prvUtfIsPbrModeUsed())
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpMLLLastBitWrite
                                (dev, entryIndex, lastBit, mllEntryPart),
                                mllEntryPart);
        }
    }

    entryIndex   = 0;
    lastBit      = GT_FALSE;
    mllEntryPart = CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMLLLastBitWrite(dev, entryIndex, lastBit, mllEntryPart);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMLLLastBitWrite(dev, entryIndex, lastBit, mllEntryPart);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterArpAddrWrite
(
    IN GT_U8                         devNum,
    IN GT_U32                        routerArpIndex,
    IN GT_ETHERADDR                  *arpMacAddrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterArpAddrWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with routerArpIndex [0 / tableSize - 1],
                            arpMacAddrPtr [AB:CD:EF:00:00:01].
    Expected: GT_OK.
    1.2. Call cpssDxChIpRouterArpAddrRead with the same routerArpIndex
    and non-NULL  arpMacAddrPtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with wrong routerArpIndex [tableSize],
    other params same as in 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with routerArpIndex [0], arpMacAddrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st             = GT_OK;
    GT_U8           dev;
    GT_U32          routerArpIndex = 0;
    GT_ETHERADDR    arpMacAddr;
    GT_ETHERADDR    arpMacAddrGet;
    GT_BOOL         isEqual        = GT_FALSE;
    GT_U32          tableSize      = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)->
                                fineTuning.tableSize.routerArp;
        /*
            1.1. Call function with routerArpIndex [0 / tableSize - 1],
                                    arpMacAddrPtr [AB:CD:EF:00:00:01].
            Expected: GT_OK.
        */
        arpMacAddr.arEther[0] = 0xAB;
        arpMacAddr.arEther[1] = 0xCD;
        arpMacAddr.arEther[2] = 0xEF;
        arpMacAddr.arEther[3] = 0;
        arpMacAddr.arEther[4] = 0;
        arpMacAddr.arEther[5] = 1;

        /* Call with routerArpIndex [0] */
        routerArpIndex = 0;

        st = cpssDxChIpRouterArpAddrWrite(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpIndex);

        /*
            1.2. Call cpssDxChIpRouterArpAddrRead with the same routerArpIndex
            and non-NULL  arpMacAddrPtr.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, &arpMacAddrGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpRouterArpAddrRead: %d, %d",
                                     dev, routerArpIndex);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &arpMacAddr,
                                     (GT_VOID*) &arpMacAddrGet,
                                     sizeof(arpMacAddr))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another arpMacAddr than was set: %d", dev);


        /* Call with routerArpIndex [tableSize - 1] */
        routerArpIndex = tableSize - 1;

        st = cpssDxChIpRouterArpAddrWrite(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpIndex);

        /* 1.2. Call cpssDxChIpRouterArpAddrRead with the same routerArpIndex */

        st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, &arpMacAddrGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpRouterArpAddrRead: %d, %d",
                                     dev, routerArpIndex);

        /* Verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &arpMacAddr,
                                     (GT_VOID*) &arpMacAddrGet,
                                     sizeof(arpMacAddr))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another arpMacAddr than was set: %d", dev);

        /*
            1.3. Call function with wrong routerArpIndex [tableSize],
            other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        routerArpIndex = tableSize;

        st = cpssDxChIpRouterArpAddrWrite(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpIndex);

        routerArpIndex = tableSize - 1;

        /*
            1.4. Call function with routerArpIndex [0], arpMacAddrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        routerArpIndex = 0;

        st = cpssDxChIpRouterArpAddrWrite(dev, routerArpIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, arpMacAddrPtr = NULL", dev);
    }

    arpMacAddr.arEther[0] = 0xAB;
    arpMacAddr.arEther[1] = 0xCD;
    arpMacAddr.arEther[2] = 0xEF;
    arpMacAddr.arEther[3] = 0;
    arpMacAddr.arEther[4] = 0;
    arpMacAddr.arEther[5] = 1;

    routerArpIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterArpAddrWrite(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterArpAddrWrite(dev, routerArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterArpAddrRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       routerArpIndex,
    OUT GT_ETHERADDR                 *arpMacAddrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterArpAddrRead)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with routerArpIndex [0 / tableSize - 1],
    non-NULL arpMacAddrPtr.
    Expected: GT_OK.
    1.2. Call function with wrong routerArpIndex [tableSize],
    other params same as in 1.1.
    Expected: NOT GT_OK.
    1.3. Call function with routerArpIndex [0], arpMacAddrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st             = GT_OK;
    GT_U8           dev;
    GT_U32          routerArpIndex = 0;
    GT_ETHERADDR    arpMacAddr;
    GT_U32          tableSize      = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)->
                                fineTuning.tableSize.routerArp;

        /*
            1.1. Call function with routerArpIndex [0 / tableSize - 1],
            non-NULL arpMacAddrPtr.
            Expected: GT_OK.
        */
        /* Call with routerArpIndex [0] */
        routerArpIndex = 0;

        st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpIndex);

        /* Call with routerArpIndex [tableSize - 1] */
        routerArpIndex = tableSize - 1;

        st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpIndex);

        /*
            1.2. Call function with wrong routerArpIndex [tableSize],
            other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        routerArpIndex = tableSize;

        st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, routerArpIndex);

        routerArpIndex = 0;

        /*
            1.3. Call function with routerArpIndex [0], arpMacAddrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, arpMacAddrPtr = NULL", dev);
    }

    routerArpIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterArpAddrRead(dev, routerArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpNatEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              natIndex,
    IN  CPSS_IP_NAT_TYPE_ENT                natType,
    IN  CPSS_DXCH_IP_NAT_ENTRY_UNT          *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpNatEntrySet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function cpssDxChIpNatEntrySet with
                            natType = CPSS_IP_NAT_TYPE_NAT44_E,
                            entryPtr:
                            natIndex [0 / tableSize - 1],
                            macDa [AB:CD:EF:00:00:01].
                            modifyDip = GT_TRUE;
                            newDip = 192.3.4.5;
                            modifySip = GT_FALSE;
                            newSip = 0.0.0.0;
                            modifyTcpUdpDstPort = GT_TRUE;
                            newTcpUdpDstPort = 1234;
                            modifyTcpUdpSrcPort = GT_FALSE;
                            newTcpUdpSrcPort = 5678;
    Expected: GT_OK.
    1.2. Call cpssDxChIpNatEntryGet with the same natIndex
    and non-NULL entryPtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with wrong natIndex [tableSize],
    other params same as in 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with wrong newTcpUdpDstPort [65536],
    other params same as in 1.1.
    Expected: NOT GT_OK.
    1.5. Call function with wrong newTcpUdpSrcPort [65536],
    other params same as in 1.1.
    Expected: NOT GT_OK.
    1.6. Call function with natIndex [0], entryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.7. Call function with wrong natType,
    other params same as in 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          natIndex = 0;
    CPSS_IP_NAT_TYPE_ENT            natType = CPSS_IP_NAT_TYPE_NAT44_E;
    CPSS_DXCH_IP_NAT_ENTRY_UNT      natEntry;
    CPSS_DXCH_IP_NAT_ENTRY_UNT      natEntryGet;
    GT_BOOL                         isEqual   = GT_FALSE;
    GT_U32                          tableSize = 0;
    CPSS_IP_NAT_TYPE_ENT            natTypeGet;

    /* ------------------------------------ */
    /* -------------- check NAT44 --------- */
    /* ------------------------------------ */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)->
                                fineTuning.tableSize.tunnelStart;
        /*
             1.1. Call function cpssDxChIpNatEntrySet with
                            natType = CPSS_IP_NAT_TYPE_NAT44_E,
                            entryPtr:
                            natIndex [0 / tableSize - 1],
                            macDa [AB:CD:EF:00:00:01].
                            modifyDip = GT_TRUE;
                            newDip = 192.3.4.5;
                            modifySip = GT_FALSE;
                            newSip = 1.2.3.4;
                            modifyTcpUdpDstPort = GT_TRUE;
                            newTcpUdpDstPort = 1234;
                            modifyTcpUdpSrcPort = GT_FALSE;
                            newTcpUdpSrcPort = 5678;
            Expected: GT_OK.
        */
        natEntry.nat44Entry.macDa.arEther[0] = 0xAB;
        natEntry.nat44Entry.macDa.arEther[1] = 0xCD;
        natEntry.nat44Entry.macDa.arEther[2] = 0xEF;
        natEntry.nat44Entry.macDa.arEther[3] = 0;
        natEntry.nat44Entry.macDa.arEther[4] = 0;
        natEntry.nat44Entry.macDa.arEther[5] = 1;
        natEntry.nat44Entry.modifyDip=GT_TRUE;
        natEntry.nat44Entry.newDip.arIP[0]=192;
        natEntry.nat44Entry.newDip.arIP[1]=3;
        natEntry.nat44Entry.newDip.arIP[2]=4;
        natEntry.nat44Entry.newDip.arIP[3]=5;
        natEntry.nat44Entry.modifySip=GT_FALSE;
        natEntry.nat44Entry.newSip.arIP[0]=1;
        natEntry.nat44Entry.newSip.arIP[1]=2;
        natEntry.nat44Entry.newSip.arIP[2]=3;
        natEntry.nat44Entry.newSip.arIP[3]=4;
        natEntry.nat44Entry.modifyTcpUdpDstPort=GT_TRUE;
        natEntry.nat44Entry.newTcpUdpDstPort=1234;
        natEntry.nat44Entry.modifyTcpUdpSrcPort=GT_FALSE;
        natEntry.nat44Entry.newTcpUdpSrcPort=5678;

        natType = CPSS_IP_NAT_TYPE_NAT44_E;
        /* Call with natIndex [0] */
        natIndex = 0;

        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, natIndex);
        }
        /*
            1.2. Call cpssDxChIpNatEntryGet with the same natIndex
            and non-NULL entryPtr.
            Expected: GT_OK and same values as written.
        */
        st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpNatEntryGet: %d, %d",
                                     dev, natIndex);

            /* Verifying values */

            isEqual = (natType == natTypeGet)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another entry natType than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &natEntry.nat44Entry.macDa,
                                         (GT_VOID*) &natEntryGet.nat44Entry.macDa,
                                         sizeof(GT_ETHERADDR))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another macDa than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifyDip == natEntryGet.nat44Entry.modifyDip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyDip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newDip.u32Ip == natEntryGet.nat44Entry.newDip.u32Ip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newDip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifySip == natEntryGet.nat44Entry.modifySip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifySip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newSip.u32Ip == natEntryGet.nat44Entry.newSip.u32Ip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newSip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifyTcpUdpDstPort == natEntryGet.nat44Entry.modifyTcpUdpDstPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyTcpUdpDstPort than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newTcpUdpDstPort == natEntryGet.nat44Entry.newTcpUdpDstPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newTcpUdpDstPort than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifyTcpUdpSrcPort == natEntryGet.nat44Entry.modifyTcpUdpSrcPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyTcpUdpSrcPort than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newTcpUdpSrcPort == natEntryGet.nat44Entry.newTcpUdpSrcPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newTcpUdpSrcPort than was set: %d", dev);

            /* Call with natIndex [tableSize - 1] */
            natIndex = tableSize - 1;

            st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, natIndex);

            /* 1.2. Call cpssDxChIpNatEntryGet with the same natIndex */

            st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpNatEntryGet: %d, %d",
                                         dev, natIndex);

            /* Verifying values */
            isEqual = (natType == natTypeGet)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another entry natType than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &natEntry.nat44Entry.macDa,
                                         (GT_VOID*) &natEntryGet.nat44Entry.macDa,
                                         sizeof(GT_ETHERADDR))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another macDa than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifyDip == natEntryGet.nat44Entry.modifyDip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyDip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newDip.u32Ip == natEntryGet.nat44Entry.newDip.u32Ip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newDip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifySip == natEntryGet.nat44Entry.modifySip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifySip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newSip.u32Ip == natEntryGet.nat44Entry.newSip.u32Ip)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newSip than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifyTcpUdpDstPort == natEntryGet.nat44Entry.modifyTcpUdpDstPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyTcpUdpDstPort than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newTcpUdpDstPort == natEntryGet.nat44Entry.newTcpUdpDstPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newTcpUdpDstPort than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.modifyTcpUdpSrcPort == natEntryGet.nat44Entry.modifyTcpUdpSrcPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyTcpUdpSrcPort than was set: %d", dev);
            isEqual = (natEntry.nat44Entry.newTcpUdpSrcPort == natEntryGet.nat44Entry.newTcpUdpSrcPort)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another newTcpUdpSrcPort than was set: %d", dev);
        }
        /*
            1.3. Call function with wrong natIndex [tableSize],
            other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        natIndex = tableSize;

        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, natIndex);
        }

        natIndex = tableSize - 1;

        /*
          1.4. Call function with wrong newTcpUdpDstPort [65536],
          other params same as in 1.1.
          Expected: NOT GT_OK.
        */
        natEntry.nat44Entry.newTcpUdpDstPort=65536;
        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, natIndex);
        }

        /*
          1.5. Call function with wrong newTcpUdpSrcPort [65536],
          other params same as in 1.1.
          Expected: NOT GT_OK.
        */
        natEntry.nat44Entry.newTcpUdpDstPort=1234;
        natEntry.nat44Entry.newTcpUdpSrcPort=65536;
        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, natIndex);
        }

        /*
            1.6. Call function with natIndex [0], natEntry [NULL].
            Expected: GT_BAD_PTR.
        */
        natIndex = 0;
        natEntry.nat44Entry.newTcpUdpSrcPort=5678;

        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, natEntry = NULL", dev);

        /*
            1.7. Call function with wrong natType,
            other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpNatEntrySet
                           (dev, natIndex, natType, &natEntry),
                            natType);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    /* ------------------------------------ */
    /* -------------- check NAT66 --------- */
    /* ------------------------------------ */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)->
                                fineTuning.tableSize.tunnelStart;
        /*
             1.1. Call function cpssDxChIpNatEntrySet with
                            natType = CPSS_IP_NAT_TYPE_NAT66_E,
                            entryPtr:
                            natIndex [0 / tableSize - 1],
                            macDa [AB:CD:EF:00:00:01].
                            modifyCommand = CPSS_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E;
                            address = FF:0:0:0:0:0:1111:2222:3333:4444:5555;
                            prefixSize=0 [legal values 0-63];

            Expected: GT_OK.
        */
        natEntry.nat66Entry.macDa.arEther[0] = 0xAB;
        natEntry.nat66Entry.macDa.arEther[1] = 0xCD;
        natEntry.nat66Entry.macDa.arEther[2] = 0xEF;
        natEntry.nat66Entry.macDa.arEther[3] = 0;
        natEntry.nat66Entry.macDa.arEther[4] = 0;
        natEntry.nat66Entry.macDa.arEther[5] = 1;
        natEntry.nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E;
        natEntry.nat66Entry.address.arIP[0] = 0xFF;
        natEntry.nat66Entry.address.arIP[1] = 0x0;
        natEntry.nat66Entry.address.arIP[2] = 0x0;
        natEntry.nat66Entry.address.arIP[3] = 0x0;
        natEntry.nat66Entry.address.arIP[4] = 0x0;
        natEntry.nat66Entry.address.arIP[5] = 0x0;
        natEntry.nat66Entry.address.arIP[6] = 0x11;
        natEntry.nat66Entry.address.arIP[7] = 0x11;
        natEntry.nat66Entry.address.arIP[8] = 0x22;
        natEntry.nat66Entry.address.arIP[9] = 0x22;
        natEntry.nat66Entry.address.arIP[10] = 0x33;
        natEntry.nat66Entry.address.arIP[11] = 0x33;
        natEntry.nat66Entry.address.arIP[12] = 0x44;
        natEntry.nat66Entry.address.arIP[13] = 0x44;
        natEntry.nat66Entry.address.arIP[14] = 0x55;
        natEntry.nat66Entry.address.arIP[15] = 0x55;
        natEntry.nat66Entry.prefixSize =64;

        natType = CPSS_IP_NAT_TYPE_NAT66_E;
        /* Call with natIndex [0] */
        natIndex = 0;

        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        if((!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))||
           ((natType==CPSS_IP_NAT_TYPE_NAT66_E)&&
            (!PRV_CPSS_SIP_5_15_CHECK_MAC(dev))))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, natIndex);

            /*
                1.2. Call cpssDxChIpNatEntryGet with the same natIndex
                and non-NULL entryPtr.
                Expected: GT_OK and same values as written.
            */
            st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpNatEntryGet: %d, %d",
                                     dev, natIndex);

            /* Verifying values */
            isEqual = (natType == natTypeGet)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another entry natType than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &natEntry.nat66Entry.macDa,
                                         (GT_VOID*) &natEntryGet.nat66Entry.macDa,
                                         sizeof(GT_ETHERADDR))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another macDa than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.modifyCommand == natEntryGet.nat66Entry.modifyCommand)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyCommand than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.address.u32Ip[0] == natEntryGet.nat66Entry.address.u32Ip[0])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.address.u32Ip[1] == natEntryGet.nat66Entry.address.u32Ip[1])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
             isEqual = (natEntry.nat66Entry.address.u32Ip[2] == natEntryGet.nat66Entry.address.u32Ip[2])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
             isEqual = (natEntry.nat66Entry.address.u32Ip[3] == natEntryGet.nat66Entry.address.u32Ip[3])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.prefixSize == natEntryGet.nat66Entry.prefixSize)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another prefixSize than was set: %d", dev);

            /* Call with natIndex [tableSize - 1] */
            natIndex = tableSize - 1;

            st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, natIndex);

            /* 1.2. Call cpssDxChIpNatEntryGet with the same natIndex */

            st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpNatEntryGet: %d, %d",
                                         dev, natIndex);


            /* Verifying values */
            isEqual = (natType == natTypeGet)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another entry natType than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &natEntry.nat66Entry.macDa,
                                         (GT_VOID*) &natEntryGet.nat66Entry.macDa,
                                         sizeof(GT_ETHERADDR))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another macDa than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.modifyCommand == natEntryGet.nat66Entry.modifyCommand)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another modifyCommand than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.address.u32Ip[0] == natEntryGet.nat66Entry.address.u32Ip[0])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.address.u32Ip[1] == natEntryGet.nat66Entry.address.u32Ip[1])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
             isEqual = (natEntry.nat66Entry.address.u32Ip[2] == natEntryGet.nat66Entry.address.u32Ip[2])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
             isEqual = (natEntry.nat66Entry.address.u32Ip[3] == natEntryGet.nat66Entry.address.u32Ip[3])? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another address than was set: %d", dev);
            isEqual = (natEntry.nat66Entry.prefixSize == natEntryGet.nat66Entry.prefixSize)? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another prefixSize than was set: %d", dev);

        }
        /*
            1.3. Call function with wrong natIndex [tableSize],
            other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        natIndex = tableSize;

        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        if((!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))||
           ((natType==CPSS_IP_NAT_TYPE_NAT66_E)&&
            (!PRV_CPSS_SIP_5_15_CHECK_MAC(dev))))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, natIndex);
        }

        natIndex = tableSize - 1;

        /*
          1.4. Call function with wrong prefixSize [0],
          other params same as in 1.1.
          Expected: NOT GT_OK.
        */
        natEntry.nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E;
        natEntry.nat66Entry.prefixSize=0;
        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);

        if((!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))||
           ((natType==CPSS_IP_NAT_TYPE_NAT66_E)&&
            (!PRV_CPSS_SIP_5_15_CHECK_MAC(dev))))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, natIndex);
        }

        natEntry.nat66Entry.prefixSize=65;
        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        if((!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))||
           ((natType==CPSS_IP_NAT_TYPE_NAT66_E)&&
            (!PRV_CPSS_SIP_5_15_CHECK_MAC(dev))))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, natIndex);
        }

         /*
          1.5. Call function with [modifyCommand = wrong enum values]
          other params same as in 1.1.
          Expected: NOT GT_OK.
        */
        natEntry.nat66Entry.prefixSize=63;
        UTF_ENUMS_CHECK_MAC(cpssDxChIpNatEntrySet
                            (dev, natIndex, natType, &natEntry),
                            natEntry.nat66Entry.modifyCommand);
        /*
            1.6. Call function with natIndex [0], natEntry [NULL].
            Expected: GT_BAD_PTR.
        */
        natIndex = 0;
        natEntry.nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E;

        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, natEntry = NULL", dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpNatEntrySet(dev, natIndex, natType, &natEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpNatEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             natIndex,
    IN   CPSS_IP_NAT_TYPE_ENT               natType,
    OUT  CPSS_DXCH_IP_NAT_ENTRY_UNT         *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpNatEntryGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with natIndex [0 / tableSize - 1],
    non-NULL natEntryPtr.
    Expected: GT_OK.
    1.2. Call function with wrong natIndex [tableSize],
    other params same as in 1.1.
    Expected: NOT GT_OK.
    1.3. Call function with natIndex [0], natEntryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st        = GT_OK;
    GT_U8                           dev;
    GT_U32                          natIndex  = 0;
    CPSS_IP_NAT_TYPE_ENT            natTypeGet;
    CPSS_DXCH_IP_NAT_ENTRY_UNT      natEntryGet;
    GT_U32                          tableSize = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_LION2_E );


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get table size for current device */
        tableSize = PRV_CPSS_DXCH_PP_MAC(dev)->
                                fineTuning.tableSize.tunnelStart;

        /*
            1.1. Call function with natIndex [0 / tableSize - 1],
            non-NULL natEntryPtr.
            Expected: GT_OK.
        */
        /* Call with natIndex [0] */
        natIndex = 0;

        st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, natIndex);
        }
        /* Call with natIndex [tableSize - 1] */
        natIndex = tableSize - 1;

        st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, natIndex);
        }
        /*
            1.2. Call function with wrong natIndex [tableSize],
            other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        natIndex = tableSize;

        st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, natIndex);
        }

        natIndex = 0;

        /*
            1.3. Call function with natIndex [0], natEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
         st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, natEntryGet = NULL", dev);
    }

    natIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpNatEntryGet(dev, natIndex, &natTypeGet, &natEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChIpNatDroppedPacketsCntGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *natDropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpNatDroppedPacketsCntGet)
{
/*
    ITERATE_DEVICES (Bobcat2 and above)
    1.1. Call function cpssDxChIpNatDroppedPacketsCntGet
        with not NULL natDropPktsPtr.
    Expected: GT_OK.
    1.2. Call function cpssDxChIpNatDroppedPacketsCntGet
        with NULL natDropPktsPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     natDropPktsGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChIpNatDroppedPacketsCntGet(
            dev, &natDropPktsGet);
        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        st = cpssDxChIpNatDroppedPacketsCntGet(
            dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpNatDroppedPacketsCntGet(
            dev, &natDropPktsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpNatDroppedPacketsCntGet(
        dev, &natDropPktsGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill MLL_PAIR table.
*/
UTF_TEST_CASE_MAC(cpssDxChIpFillMllPairTable)
{
/*
    ITERATE_DEVICE (DxCh2)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_MLL_PAIR_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in MLL_PAIR table.
         Call cpssDxChIpMLLPairWrite with mllPairEntryIndex [0 ... numEntries-1],
            mllPairWriteForm [CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E],
            mllPairEntryPtr [{firstMllNode {mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
                                            isTunnelStart = GT_FALSE;
                                            nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                              devPort {devNum = 0; portNum = 0};
                                                              trunkId = 0;
                                                              vidx = 0;
                                                              vlanId = 100 };
                                            nextHopVlanId = 100;
                                            nextHopTunnelPointer = 0;
                                            ttlHopLimitThreshold = 25;
                                            excludeSrcVlan = GT_FALSE;
                                            last = GT_FALSE },
                              secondMllNode { same as firstMllNode },
                              nextPointer [0] } ].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChIpMLLPairWrite with mllPairEntryIndex [numEntries]
         and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in MLL_PAIR table and compare with original.
         Call cpssDxChIpMLLPairRead with mllPairEntryIndex
         and non-NULL  mllPairEntryPtr.
    Expected: GT_OK and same values as written.
    1.5. Try to read entry with index out of range.
         Call cpssDxChIpMLLPairRead with mllPairEntryIndex [numEntries]
         and non-NULL  mllPairEntryPtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      numEntries  = 0;
    GT_U32      iTemp       = 0;
    GT_U32               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm =
                                        CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

    CPSS_DXCH_IP_MLL_PAIR_STC            mllPairEntry;
    CPSS_DXCH_IP_MLL_PAIR_STC            mllPairEntryGet;


    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &mllPairEntry, sizeof(mllPairEntry));
    cpssOsBzero((GT_VOID*) &mllPairEntryGet, sizeof(mllPairEntryGet));

    /* Fill the entry for MLL_PAIR table */
    mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

    cpssOsBzero((GT_VOID*)&mllPairEntry, sizeof(mllPairEntry));
    mllPairEntry.firstMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
    mllPairEntry.firstMllNode.isTunnelStart     = GT_FALSE;

    mllPairEntry.firstMllNode.nextHopInterface.type            = CPSS_INTERFACE_TRUNK_E;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum  = 0;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum = 0;
    mllPairEntry.firstMllNode.nextHopInterface.trunkId         = 0;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(mllPairEntry.firstMllNode.nextHopInterface.trunkId);
    mllPairEntry.firstMllNode.nextHopInterface.vidx            = 0;
    mllPairEntry.firstMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

    mllPairEntry.firstMllNode.nextHopVlanId                    = IP_VLAN_TESTED_VLAN_ID_CNS;
    mllPairEntry.firstMllNode.nextHopTunnelPointer             = 0;
    mllPairEntry.firstMllNode.tunnelStartPassengerType         = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
    mllPairEntry.firstMllNode.ttlHopLimitThreshold             = 25;
    mllPairEntry.firstMllNode.excludeSrcVlan                   = GT_FALSE;
    mllPairEntry.firstMllNode.last                             = GT_FALSE;

    mllPairEntry.secondMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
    mllPairEntry.secondMllNode.isTunnelStart     = GT_TRUE;

    mllPairEntry.secondMllNode.nextHopInterface.type            = CPSS_INTERFACE_TRUNK_E;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum  = 0;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum = 0;
    mllPairEntry.secondMllNode.nextHopInterface.trunkId         = 0;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(mllPairEntry.secondMllNode.nextHopInterface.trunkId);
    mllPairEntry.secondMllNode.nextHopInterface.vidx            = 0;
    mllPairEntry.secondMllNode.nextHopInterface.vlanId          = IP_VLAN_TESTED_VLAN_ID_CNS;

    mllPairEntry.secondMllNode.nextHopVlanId        = IP_VLAN_TESTED_VLAN_ID_CNS;
    mllPairEntry.secondMllNode.nextHopTunnelPointer = 221;
    mllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
    mllPairEntry.secondMllNode.ttlHopLimitThreshold = 25;
    mllPairEntry.secondMllNode.excludeSrcVlan       = GT_FALSE;
    mllPairEntry.secondMllNode.last                 = GT_FALSE;

    mllPairEntry.nextPointer = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_MLL_PAIR_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in MLL_PAIR table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* make every entry unique */
            mllPairEntry.firstMllNode.nextHopInterface.trunkId  = (GT_TRUNK_ID)(iTemp % 127);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(mllPairEntry.firstMllNode.nextHopInterface.trunkId);

            st = cpssDxChIpMLLPairWrite(dev, iTemp, mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChIpMLLPairWrite: %d, %d, %d", dev, iTemp, mllPairWriteForm);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChIpMLLPairWrite(dev, numEntries, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChIpMLLPairWrite: %d, %d, %d", dev, numEntries, mllPairWriteForm);

        /* 1.4. Read all entries in MLL_PAIR table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* restore unique entry before compare */
            mllPairEntry.firstMllNode.nextHopInterface.trunkId  = (GT_TRUNK_ID)(iTemp % 127);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(mllPairEntry.firstMllNode.nextHopInterface.trunkId);

            st = cpssDxChIpMLLPairRead(dev, iTemp, mllPairWriteForm, &mllPairEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMLLPairRead: %d, %d",
                                         dev, iTemp);

            /* Verifying routeEntryPtr fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.mllRPFFailCommand,
                                         mllPairEntryGet.firstMllNode.mllRPFFailCommand,
                "get another mllPairEntryPtr->firstMllNode.mllRPFFailCommand than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.isTunnelStart,
                                         mllPairEntryGet.firstMllNode.isTunnelStart,
                "get another mllPairEntryPtr->firstMllNode.isTunnelStart than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopVlanId,
                                         mllPairEntryGet.firstMllNode.nextHopVlanId,
                "get another mllPairEntryPtr->firstMllNode.nextHopVlanId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopTunnelPointer,
                                         mllPairEntryGet.firstMllNode.nextHopTunnelPointer,
                "get another mllPairEntryPtr->firstMllNode.nextHopTunnelPointer than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.ttlHopLimitThreshold,
                                         mllPairEntryGet.firstMllNode.ttlHopLimitThreshold,
                "get another mllPairEntryPtr->firstMllNode.ttlHopLimitThreshold than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.excludeSrcVlan,
                                         mllPairEntryGet.firstMllNode.excludeSrcVlan,
                "get another mllPairEntryPtr->firstMllNode.excludeSrcVlan than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.last,
                                         mllPairEntryGet.firstMllNode.last,
                "get another mllPairEntryPtr->firstMllNode.last than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopInterface.type,
                                         mllPairEntryGet.firstMllNode.nextHopInterface.type,
                "get another mllPairEntryPtr->firstMllNode.nextHopInterface.type than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum,
                                         mllPairEntryGet.firstMllNode.nextHopInterface.devPort.hwDevNum,
                "get another mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.hwDevNum than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum,
                                         mllPairEntryGet.firstMllNode.nextHopInterface.devPort.portNum,
                "get another mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.portNum than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.mllRPFFailCommand,
                                         mllPairEntryGet.secondMllNode.mllRPFFailCommand,
                "get another mllPairEntryPtr->secondMllNode.mllRPFFailCommand than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.isTunnelStart,
                                         mllPairEntryGet.secondMllNode.isTunnelStart,
                "get another mllPairEntryPtr->secondMllNode.isTunnelStart than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopVlanId,
                                         mllPairEntryGet.secondMllNode.nextHopVlanId,
                "get another mllPairEntryPtr->secondMllNode.nextHopVlanId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopTunnelPointer,
                                         mllPairEntryGet.secondMllNode.nextHopTunnelPointer,
                "get another mllPairEntryPtr->secondMllNode.nextHopTunnelPointer than was set: %d", dev);
            /*tunnel start passenger type is relevant only in ch3 and above*/
            if (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH2_E)
                UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.tunnelStartPassengerType,
                                             mllPairEntryGet.secondMllNode.tunnelStartPassengerType,
                    "get another mllPairEntryPtr->secondMllNode.tunnelStartPassengerType than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.ttlHopLimitThreshold,
                                         mllPairEntryGet.secondMllNode.ttlHopLimitThreshold,
                "get another mllPairEntryPtr->secondMllNode.ttlHopLimitThreshold than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.excludeSrcVlan,
                                         mllPairEntryGet.secondMllNode.excludeSrcVlan,
                "get another mllPairEntryPtr->secondMllNode.excludeSrcVlan than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.last,
                                         mllPairEntryGet.secondMllNode.last,
                "get another mllPairEntryPtr->secondMllNode.last than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopInterface.type,
                                         mllPairEntryGet.secondMllNode.nextHopInterface.type,
                "get another mllPairEntryPtr->secondMllNode.nextHopInterface.type than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum,
                                         mllPairEntryGet.secondMllNode.nextHopInterface.devPort.hwDevNum,
                "get another mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.hwDevNum than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum,
                                         mllPairEntryGet.secondMllNode.nextHopInterface.devPort.portNum,
                "get another mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.portNum than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.nextPointer, mllPairEntryGet.nextPointer,
                "get another mllPairEntryPtr->nextPointer than was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChIpMLLPairRead(dev, numEntries, mllPairWriteForm, &mllPairEntryGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMLLPairRead: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill UC route table.
*/
UTF_TEST_CASE_MAC(cpssDxChIpFillUcRouteTable)
{
/*
    ITERATE_DEVICE (DxCh2)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with
         table [CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E] and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in uc route table.
         Call cpssDxChIpUcRouteEntriesWrite with baseRouteEntryIndex [0 ... numEntries-1],
            routeEntriesArray[0] [{cmd = CPSS_PACKET_CMD_DROP_SOFT_E;
                                   cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                                   appSpecificCpuCodeEnable = GT_FALSE;
                                   unicastPacketSipFilterEnable = GT_FALSE;
                                   ttlHopLimitDecEnable = GT_TRUE;
                                   ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                                   ingressMirror = GT_TRUE;
                                   qosProfileMarkingEnable = GT_FALSE;
                                   qosProfileIndex = 0;
                                   qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                                   modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                   modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                                   countSet = CPSS_IP_CNT_SET0_E;
                                   trapMirrorArpBcEnable = GT_FALSE;
                                   sipAccessLevel = 0;
                                   dipAccessLevel = 0;
                                   ICMPRedirectEnable = GT_FALSE;
                                   scopeCheckingEnable = GT_TRUE;
                                   siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                                   mtuProfileIndex = 0;
                                   isTunnelStart = GT_FALSE;
                                   nextHopVlanId = 100;
                                   nextHopInterface {type = CPSS_INTERFACE_PORT_E;
                                                     devPort {devNum = 0; portNum = 0} ,
                                                     trunkId = 0; vidx = 10; vlanId = 100 },
                                   nextHopARPPointer = 0;
                                   nextHopTunnelPointer = 0 } ],
            numOfRouteEntries [1].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChIpUcRouteEntriesWrite with baseRouteEntryIndex [numEntries]
         and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in uc route table and compare with original.
         Call cpssDxChIpUcRouteEntriesRead with baseRouteEntryIndex
         and non-NULL routeEntriesArray, numOfRouteEntries [1].
    Expected: GT_OK and same values as written.
    1.5. Try to read entry with index out of range.
         Call cpssDxChIpUcRouteEntriesRead with baseRouteEntryIndex [numEntries].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual      = GT_FALSE;
    GT_U32      numEntries   = 0;
    GT_U32      iTemp        = 0;
    GT_U32      numOfEntries = 0;
    GT_U32               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC routeEntriesArray;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC routeEntriesArrayGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &routeEntriesArray, sizeof(routeEntriesArray));
    cpssOsBzero((GT_VOID*) &routeEntriesArrayGet, sizeof(routeEntriesArrayGet));

    /* Fill the entry for uc route table */
    routeEntriesArray.type                                     = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
    routeEntriesArray.entry.regularEntry.cmd                   = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
    routeEntriesArray.entry.regularEntry.cpuCodeIdx            = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
    routeEntriesArray.entry.regularEntry.ttlHopLimitDecEnable  = GT_TRUE;
    routeEntriesArray.entry.regularEntry.ingressMirror         = GT_TRUE;
    routeEntriesArray.entry.regularEntry.qosProfileIndex       = 0;
    routeEntriesArray.entry.regularEntry.qosPrecedence         = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    routeEntriesArray.entry.regularEntry.modifyUp              = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntriesArray.entry.regularEntry.modifyDscp            = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntriesArray.entry.regularEntry.countSet              = CPSS_IP_CNT_SET0_E;
    routeEntriesArray.entry.regularEntry.trapMirrorArpBcEnable = GT_FALSE;
    routeEntriesArray.entry.regularEntry.sipAccessLevel        = 0;
    routeEntriesArray.entry.regularEntry.dipAccessLevel        = 0;
    routeEntriesArray.entry.regularEntry.ICMPRedirectEnable    = GT_FALSE;
    routeEntriesArray.entry.regularEntry.scopeCheckingEnable   = GT_TRUE;
    routeEntriesArray.entry.regularEntry.siteId                = CPSS_IP_SITE_ID_INTERNAL_E;
    routeEntriesArray.entry.regularEntry.mtuProfileIndex       = 0;
    routeEntriesArray.entry.regularEntry.isTunnelStart         = GT_FALSE;
    routeEntriesArray.entry.regularEntry.isNat                 = GT_FALSE;
    routeEntriesArray.entry.regularEntry.nextHopVlanId         = IP_VLAN_TESTED_VLAN_ID_CNS;
    routeEntriesArray.entry.regularEntry.nextHopARPPointer     = 0;
    routeEntriesArray.entry.regularEntry.nextHopTunnelPointer  = 0;
    routeEntriesArray.entry.regularEntry.nextHopNatPointer     = 0;

    routeEntriesArray.entry.regularEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    routeEntriesArray.entry.regularEntry.qosProfileMarkingEnable         = GT_FALSE;
    routeEntriesArray.entry.regularEntry.appSpecificCpuCodeEnable        = GT_FALSE;
    routeEntriesArray.entry.regularEntry.unicastPacketSipFilterEnable    = GT_FALSE;

    routeEntriesArray.entry.regularEntry.nextHopInterface.type            = CPSS_INTERFACE_TRUNK_E;
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.hwDevNum  = 0;
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum = 0;
    routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId         = 0;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId);
    routeEntriesArray.entry.regularEntry.nextHopInterface.vidx            = 0;
    routeEntriesArray.entry.regularEntry.nextHopInterface.vlanId          = 0;

    numOfEntries = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in uc route table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* make every entry unique */
            routeEntriesArray.entry.regularEntry.sipAccessLevel = iTemp % 7;
            routeEntriesArray.entry.regularEntry.dipAccessLevel = iTemp % 7;
            routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId = (GT_TRUNK_ID)(iTemp % 127);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId);
            routeEntriesArray.entry.regularEntry.nextHopVlanId = (GT_U16)(iTemp % 4095);

            st = cpssDxChIpUcRouteEntriesWrite(dev, iTemp, &routeEntriesArray, numOfEntries);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpUcRouteEntriesWrite: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChIpUcRouteEntriesWrite(dev, numEntries, &routeEntriesArray, numOfEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpUcRouteEntriesWrite: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in uc route table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            routeEntriesArrayGet.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
            /* restore unique entry before compare */
            routeEntriesArray.entry.regularEntry.sipAccessLevel = iTemp % 7;
            routeEntriesArray.entry.regularEntry.dipAccessLevel = iTemp % 7;
            routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId = (GT_TRUNK_ID)(iTemp % 127);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(routeEntriesArray.entry.regularEntry.nextHopInterface.trunkId);
            routeEntriesArray.entry.regularEntry.nextHopVlanId = (GT_U16)(iTemp % 4095);

            st = cpssDxChIpUcRouteEntriesRead(dev, iTemp, &routeEntriesArrayGet, numOfEntries);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChIpUcRouteEntriesRead: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                       dev, iTemp, numOfEntries);

            /* Verifying routeEntriesArray fields */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &routeEntriesArray,
                                         (GT_VOID*) &routeEntriesArrayGet,
                                         sizeof(routeEntriesArray))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another routeEntriesArray than was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChIpUcRouteEntriesRead(dev, numEntries, &routeEntriesArrayGet, numOfEntries);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
            "cpssDxChIpUcRouteEntriesRead: %d, baseRouteEntryIndex = %d, numOfRouteEntries = %d",
                                         dev, numEntries, numOfEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill MC route tAble.
*/
UTF_TEST_CASE_MAC(cpssDxChIpFillMcRouteTable)
{
/*
    ITERATE_DEVICE (DxCh2)
    1.1. Get table Size
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in MC route table.
         Call cpssDxChIpMcRouteEntriesWrite with routeEntryIndex [0 ... numEntries-1],
            routeEntryPtr [{cmd = CPSS_PACKET_CMD_FORWARD_E;
                            cpuCodeIdx = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
                            appSpecificCpuCodeEnable = GT_FALSE;
                            ucSipFilterEnable = GT_FALSE;
                            ttlHopLimitDecEnable = GT_TRUE;
                            ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
                            ingressMirror = GT_TRUE;
                            qosProfileMarkingEnable = GT_FALSE;
                            qosProfileIndex = 0;
                            qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                            modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                            modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                            countSet = CPSS_IP_CNT_SET0_E;
                            multicastRPFCheckEnable = GT_TRUE;
                            multicastRPFVlan = 100;
                            multicastRPFFailCommandMode = CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
                            RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                            scopeCheckingEnable = GT_FALSE;
                            siteId = CPSS_IP_SITE_ID_INTERNAL_E;
                            mtuProfileIndex = 0;
                            internalMLLPointer = 0;
                            externalMLLPointer = 0}].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChIpMcRouteEntriesWrite with routeEntryIndex [numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in MC route table and compare with original.
         Call cpssDxChIpMcRouteEntriesRead with routeEntryIndex non-NULL routeEntryPtr.
    Expected: GT_OK and same values as written.
    1.5. Try to read entry with index out of range.
         Call cpssDxChIpMcRouteEntriesRead with routeEntryIndex [numEntries], non-NULL routeEntryPtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual      = GT_FALSE;
    GT_U32      numEntries   = 0;
    GT_U32      iTemp        = 0;
    GT_U32               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntry;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntryGet;;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &routeEntry, sizeof(routeEntry));
    cpssOsBzero((GT_VOID*) &routeEntryGet, sizeof(routeEntryGet));

    /* Fill the entry for MC route table */
    routeEntry.cmd                      = CPSS_PACKET_CMD_DROP_SOFT_E;
    routeEntry.cpuCodeIdx               = CPSS_DXCH_IP_CPU_CODE_IDX_0_E;
    routeEntry.appSpecificCpuCodeEnable = GT_FALSE;
    routeEntry.ttlHopLimitDecEnable     = GT_TRUE;
    routeEntry.ingressMirror            = GT_FALSE;
    routeEntry.qosProfileMarkingEnable  = GT_FALSE;
    routeEntry.qosProfileIndex          = 0;
    routeEntry.qosPrecedence            = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    routeEntry.modifyUp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntry.modifyDscp               = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    routeEntry.countSet                 = CPSS_IP_CNT_SET0_E;
    routeEntry.multicastRPFCheckEnable  = GT_TRUE;
    routeEntry.multicastRPFVlan         = IP_VLAN_TESTED_VLAN_ID_CNS;
    routeEntry.RPFFailCommand           = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    routeEntry.scopeCheckingEnable      = GT_FALSE;
    routeEntry.siteId                   = CPSS_IP_SITE_ID_INTERNAL_E;
    routeEntry.mtuProfileIndex          = 0;
    routeEntry.internalMLLPointer       = 0;
    routeEntry.externalMLLPointer       = 0;
    routeEntry.multicastRPFFailCommandMode     = CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
    routeEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in MC route table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* make every entry unique */
            routeEntry.multicastRPFVlan = (GT_U16)(iTemp % 4095);
            routeEntry.mtuProfileIndex  = iTemp % 7;

            st = cpssDxChIpMcRouteEntriesWrite(dev, iTemp, &routeEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChIpMcRouteEntriesWrite: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChIpMcRouteEntriesWrite(dev, numEntries, &routeEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChIpMcRouteEntriesWrite: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in MC route table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* restore unique entry before compare */
            routeEntry.multicastRPFVlan = (GT_U16)(iTemp % 4095);
            routeEntry.mtuProfileIndex  = iTemp % 7;

            st = cpssDxChIpMcRouteEntriesRead(dev, iTemp, &routeEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMcRouteEntriesRead: %d, %d",
                                         dev, iTemp);

            /* Verifying routeEntryPtr fields */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &routeEntry,
                                         (GT_VOID*) &routeEntryGet,
                                         sizeof(routeEntry))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another routeEntry than was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChIpMcRouteEntriesRead(dev, numEntries, &routeEntryGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMcRouteEntriesRead: %d, %d",
                                         dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill LookUp Translation table.
*/
UTF_TEST_CASE_MAC(cpssDxChIpFillLttTable)
{
/*
    ITERATE_DEVICE (DxCh2)
    1.1. Get table Size.
    1.2. Fill all entries in LookUp Translation table.
         Call cpssDxChIpLttWrite with lttTtiRow [0 ... maxlttTtiRow-1],
              lttTtiColumn [0 ... maxlttTtiColumn-1],
              lttEntryPtr [routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
                         numOfPaths = 0;
                         routeEntryBaseIndex = 0;
                         ucRPFCheckEnable = GT_FALSE;
                         sipSaCheckMismatchEnable = GT_FALSE;
                         ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChIpLttWrite with lttTtiRow [maxlttTtiRow],
         lttTtiColumn[maxlttTtiColumn] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in LookUp Translation table and compare with original.
         Call cpssDxChIpLttRead with lttTtiRow, lttTtiColumn and non-NULL lttEntryPtr.
    Expected: GT_OK and same values as written.
    1.5. Try to read entry with index out of range.
         Call cpssDxChIpLttRead with lttTtiRow [maxlttTtiRow],
         lttTtiColumn[maxlttTtiColumn] and non-NULL lttEntryPtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      lttTtiRow       = 0;
    GT_U32      lttTtiColumn    = 0;
    GT_U32      maxlttTtiRow    = 0;
    GT_U32      maxlttTtiColumn = 0;
    GT_U32               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    CPSS_DXCH_IP_LTT_ENTRY_STC lttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC lttEntryGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &lttEntry, sizeof(lttEntry));
    cpssOsBzero((GT_VOID*) &lttEntryGet, sizeof(lttEntryGet));

    /* Fill the entry for LookUp Translation table */
    lttEntry.routeType             = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    lttEntry.numOfPaths            = 0;
    lttEntry.routeEntryBaseIndex   = 0;
    lttEntry.ucRPFCheckEnable      = GT_FALSE;
    lttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    lttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev,
                    CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E, &maxlttTtiRow);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = prvUtfIterationReduce(dev,maxlttTtiRow,gmFillMaxIterations,step);
        }

        switch (PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            case CPSS_PP_FAMILY_CHEETAH2_E:
                maxlttTtiColumn = 5;
                break;
            default:
                maxlttTtiColumn = 4;
                break;
        }

        /* 1.2. Fill all entries in LookUp Translation table */
        for(lttTtiRow = 0; lttTtiRow < maxlttTtiRow; lttTtiRow+=step)
            for(lttTtiColumn = 0; lttTtiColumn < maxlttTtiColumn; ++lttTtiColumn)
            {
                st = cpssDxChIpLttWrite(dev, lttTtiRow, lttTtiColumn, &lttEntry);
                if (prvUtfIsPbrModeUsed())
                {
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                            "cpssDxChIpLttWrite: %d, %d, %d", dev, lttTtiRow, lttTtiColumn);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                            "cpssDxChIpLttWrite: %d, %d, %d", dev, lttTtiRow, lttTtiColumn);
                }
            }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChIpLttWrite(dev, 0, maxlttTtiColumn, &lttEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                 "cpssDxChIpLttWrite: %d, %d, %d", dev, 0, maxlttTtiColumn);

        st = cpssDxChIpLttWrite(dev, maxlttTtiRow, 0, &lttEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChIpLttWrite: %d, %d, %d", dev, maxlttTtiRow, 0);

        /* 1.4. Read all entries in LookUp Translation table and compare with original */
        for(lttTtiRow = 0; lttTtiRow < maxlttTtiRow; lttTtiRow+=step)
            for(lttTtiColumn = 0; lttTtiColumn < maxlttTtiColumn; ++lttTtiColumn)
            {
                st = cpssDxChIpLttRead(dev, lttTtiRow, lttTtiColumn, &lttEntryGet);
                if (prvUtfIsPbrModeUsed())
                {
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLttRead: %d, %d, %d", dev, lttTtiRow, lttTtiColumn);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                                 "cpssDxChIpLttRead: %d, %d, %d", dev, lttTtiRow, lttTtiColumn);

                    if (GT_OK == st)
                    {
                        /* Verifying LttEntry fields */
                        UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.routeType, lttEntryGet.routeType,
                            "get another lttEntryPtr->routeType than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.numOfPaths, lttEntryGet.numOfPaths,
                            "get another lttEntryPtr->numOfPaths than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.routeEntryBaseIndex,
                                                     lttEntryGet.routeEntryBaseIndex,
                            "get another lttEntryPtr->routeEntryBaseIndex than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.ucRPFCheckEnable,
                                                     lttEntryGet.ucRPFCheckEnable,
                            "get another lttEntryPtr->ucRPFCheckEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.sipSaCheckMismatchEnable,
                                                     lttEntryGet.sipSaCheckMismatchEnable,
                            "get another lttEntryPtr->sipSaCheckMismatchEnable than was set: %d", dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.ipv6MCGroupScopeLevel,
                                                     lttEntryGet.ipv6MCGroupScopeLevel,
                            "get another lttEntryPtr->ipv6MCGroupScopeLevel than was set: %d", dev);
                    }
                }
            }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChIpLttRead(dev, 0, maxlttTtiColumn, &lttEntryGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                        "cpssDxChIpLttRead: %d, %d, %d", dev, 0, maxlttTtiColumn);

        st = cpssDxChIpLttRead(dev, maxlttTtiRow, 0, &lttEntryGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                           "cpssDxChIpLttRead: %d, %d, %d", dev, maxlttTtiRow, 0);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill ARP / Tunnel start Table.
*/
UTF_TEST_CASE_MAC(cpssDxChIpFillRouterArpAddrTable)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with
         table [CPSS_DXCH_CFG_TABLE_ARP_TUNNEL_START_E] and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in ARP / Tunnel start table.
         Call cpssDxChIpRouterArpAddrWrite with routerArpIndex [0 ... numEntries-1],
                                                arpMacAddrPtr [AB:CD:EF:00:00:01].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChIpRouterArpAddrWrite with routerArpIndex [numEntries],
                                                arpMacAddrPtr [AB:CD:EF:00:00:01].
    Expected: NOT GT_OK.
    1.4. Read all entries in ARP / Tunnel start table and compare with original.
         Call cpssDxChIpRouterArpAddrRead with routerArpIndex and non-NULL  arpMacAddrPtr.
    Expected: GT_OK and same values as written.
    1.5. Try to read entry with index out of range.
         Call cpssDxChIpRouterArpAddrRead with routerArpIndex [numEntries]
         and non-NULL  arpMacAddrPtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual    = GT_FALSE;
    GT_U32      numEntries = 0;
    GT_U32      iTemp      = 0;
    GT_U32               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    GT_ETHERADDR    arpMacAddr;
    GT_ETHERADDR    arpMacAddrGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &arpMacAddr, sizeof(arpMacAddr));
    cpssOsBzero((GT_VOID*) &arpMacAddrGet, sizeof(arpMacAddrGet));

    /* Fill the entry for ARP / Tunnel start table */
    arpMacAddr.arEther[0] = 0xAB;
    arpMacAddr.arEther[1] = 0xCD;
    arpMacAddr.arEther[2] = 0xEF;
    arpMacAddr.arEther[3] = 0;
    arpMacAddr.arEther[4] = 0;
    arpMacAddr.arEther[5] = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev,
                    CPSS_DXCH_CFG_TABLE_ARP_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in ARP / Tunnel start table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* make every entry unique */
            arpMacAddr.arEther[3] = (GT_U8)(iTemp % 255);
            arpMacAddr.arEther[4] = (GT_U8)(iTemp % 255);
            arpMacAddr.arEther[5] = (GT_U8)(iTemp % 255);

            st = cpssDxChIpRouterArpAddrWrite(dev, iTemp, &arpMacAddr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChIpRouterArpAddrWrite: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChIpRouterArpAddrWrite(dev, numEntries, &arpMacAddr);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChIpRouterArpAddrWrite: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in ARP / Tunnel start table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* restore unique entry before compare */
            arpMacAddr.arEther[3] = (GT_U8)(iTemp % 255);
            arpMacAddr.arEther[4] = (GT_U8)(iTemp % 255);
            arpMacAddr.arEther[5] = (GT_U8)(iTemp % 255);

            st = cpssDxChIpRouterArpAddrRead(dev, iTemp, &arpMacAddrGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpRouterArpAddrRead: %d, %d",
                                         dev, iTemp);

            /* Verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &arpMacAddr,
                                         (GT_VOID*) &arpMacAddrGet,
                                         sizeof(arpMacAddr))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another arpMacAddr than was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChIpRouterArpAddrRead(dev, numEntries, &arpMacAddrGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChIpRouterArpAddrRead: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       ecmpEntryIndex,
    IN CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpEntryWrite)
{
/*
    ITERATE_DEVICES (eArch devices only)
    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
      ecmpEntryPtr {randomEnable = GT_FALSE;
                    numOfPaths = 1;
                    routeEntryBaseIndex = 0;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 1;
                    routeEntryBaseIndex = 0;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 609;
                    routeEntryBaseIndex = 0;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 4096;
                    routeEntryBaseIndex = 0;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 4096;
                    routeEntryBaseIndex = 967;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 1;
                    routeEntryBaseIndex = 24575;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [10841],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 1;
                    routeEntryBaseIndex = 24575;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [10841],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 1;
                    routeEntryBaseIndex = 24575;}.
      Expected: GT_OK.

      Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [12287],
      non-NULL ecmpEntryPtr.
      Expected: GT_OK and same values as written.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12288],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 1;
                    routeEntryBaseIndex = 24575;}.
      Expected: GT_OUT_OF_RANGE.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
      ecmpEntryPtr = NULL.
      Expected: GT_BAD_PTR.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 0;
                    routeEntryBaseIndex = 24575;}.
      Expected: GT_BAD_PARAM.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 4097;
                    routeEntryBaseIndex = 24575;}.
      Expected: GT_BAD_PARAM.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 4096;
                    routeEntryBaseIndex = 24576;}.
      Expected: GT_BAD_PARAM.

    - Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
      ecmpEntryPtr {randomEnable = GT_TRUE;
                    numOfPaths = 2;
                    routeEntryBaseIndex = 24575;}.
      Expected: GT_BAD_PARAM
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_U32              ecmpEntryIndex;
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntry;
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntryGet;
    GT_U32              notAppFamilyBmp;
    GT_U32              numNextHopEntries,numIpEcmpEntries;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);
    cpssOsMemSet(&ecmpEntry, 0, sizeof(CPSS_DXCH_IP_ECMP_ENTRY_STC));
    cpssOsMemSet(&ecmpEntryGet, 0, sizeof(CPSS_DXCH_IP_ECMP_ENTRY_STC));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(devNum, CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E, &numIpEcmpEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", devNum);

        st = cpssDxChCfgTableNumEntriesGet(devNum, CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &numNextHopEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", devNum);


        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
                              ecmpEntryPtr {randomEnable = GT_FALSE;
                              numOfPaths = 1;
                              routeEntryBaseIndex = 0;}.
           Expected: GT_OK */
        ecmpEntryIndex = 0;
        ecmpEntry.randomEnable = GT_FALSE;
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.routeEntryBaseIndex = 0;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 1;
                              routeEntryBaseIndex = 0;}.
           Expected: GT_OK */
        ecmpEntryIndex = 0;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.routeEntryBaseIndex = 0;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 609;
                              routeEntryBaseIndex = 0;}.
           Expected: GT_OK */
        ecmpEntryIndex = 0;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 609;
        ecmpEntry.routeEntryBaseIndex = 0;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 4096;
                              routeEntryBaseIndex = 0;}.
           Expected: GT_OK */
        ecmpEntryIndex = 0;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 4096;
        ecmpEntry.routeEntryBaseIndex = 0;

        if ((ecmpEntry.routeEntryBaseIndex + ecmpEntry.numOfPaths) > numNextHopEntries)
        {
            ecmpEntry.numOfPaths = numNextHopEntries - ecmpEntry.routeEntryBaseIndex;
        }
        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 4096;
                              routeEntryBaseIndex = 967;}.
           Expected: GT_OK */
        ecmpEntryIndex = 0;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 4096;
        ecmpEntry.routeEntryBaseIndex = 967;

        if ((ecmpEntry.routeEntryBaseIndex + ecmpEntry.numOfPaths) > numNextHopEntries)
        {
            ecmpEntry.numOfPaths = numNextHopEntries - ecmpEntry.routeEntryBaseIndex;
        }
        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 1;
                              routeEntryBaseIndex = 24575;}.
           Expected: GT_OK */
        ecmpEntryIndex = 0;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [10841],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 1;
                              routeEntryBaseIndex = 24575;}.
           Expected: GT_OK */
        ecmpEntryIndex = 10841 % numIpEcmpEntries;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 1;
                              routeEntryBaseIndex = 24575;}.
           Expected: GT_OK */
        ecmpEntryIndex =  numIpEcmpEntries - 1;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryRead with ecmpEntryIndex [0],
           non-NULL ecmpEntryPtr.
           Expected: GT_OK and same values as written. */
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpEcmpEntryRead: %d, %d",
                                     devNum, ecmpEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.randomEnable,
                                     ecmpEntryGet.randomEnable,
                                     "get another randomEnable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.numOfPaths,
                                     ecmpEntryGet.numOfPaths,
                                     "get another numOfPaths than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecmpEntry.routeEntryBaseIndex,
                                     ecmpEntryGet.routeEntryBaseIndex,
                                     "get another routeEntryBaseIndex than was set: %d", devNum);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12288],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 4096;
                              routeEntryBaseIndex = 24575;}.
           Expected: GT_OUT_OF_RANGE */
        ecmpEntryIndex = numIpEcmpEntries;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OUT_OF_RANGE, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
                              ecmpEntryPtr = NULL.
           Expected: GT_BAD_PTR */
        ecmpEntryIndex = numIpEcmpEntries-1;
        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d",
                                     devNum, ecmpEntryIndex);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 0;
                              routeEntryBaseIndex = 24575;}.
           Expected: GT_BAD_PARAM */
        ecmpEntryIndex = numIpEcmpEntries-1;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 0;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 4097;
                              routeEntryBaseIndex = 24575;}.
           Expected: GT_BAD_PARAM */
        ecmpEntryIndex = numIpEcmpEntries-1;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 4097;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [12287],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 4096;
                              routeEntryBaseIndex = 24576;}.
           Expected: GT_BAD_PARAM */
        ecmpEntryIndex = numIpEcmpEntries-1;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 4096;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);
        /* Call cpssDxChIpEcmpEntryWrite with ecmpEntryIndex [0],
                              ecmpEntryPtr {randomEnable = GT_TRUE;
                              numOfPaths = 2;
                              routeEntryBaseIndex = 24575;}.
           Expected: GT_BAD_PARAM */
        ecmpEntryIndex = numIpEcmpEntries-1;
        ecmpEntry.randomEnable = GT_TRUE;
        ecmpEntry.numOfPaths = 2;
        ecmpEntry.routeEntryBaseIndex = numNextHopEntries - 1;

        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL5_STRING_MAC(GT_BAD_PARAM, st,
                                     "cpssDxChIpEcmpEntryWrite: %d, %d, %d, %d, %d",
                                     devNum, ecmpEntryIndex,
                                     ecmpEntry.randomEnable,
                                     ecmpEntry.numOfPaths,
                                     ecmpEntry.routeEntryBaseIndex);

    }

    ecmpEntryIndex = 0;
    ecmpEntry.randomEnable = GT_FALSE;
    ecmpEntry.numOfPaths = 1;
    ecmpEntry.routeEntryBaseIndex = 0;

    /* 2. For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &ecmpEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ecmpEntryIndex,
    OUT CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpEntryRead)
{
/*
    ITERATE_DEVICES (eArch devices only)
    - Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 0
      Expected: GT_OK.
    - Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 7890
      Expected: GT_OK.
    - Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 12287
      Expected: GT_OK.
    - Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 12288
      Expected: GT_OUT_OF_RANGE.
    - Call function with NULL ecmpEntryPtr.
      Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    GT_U32              ecmpEntryIndex;
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntry;
    GT_U32              notAppFamilyBmp;
    GT_U32              numIpEcmpEntries;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);
    cpssOsMemSet(&ecmpEntry, 0, sizeof(CPSS_DXCH_IP_ECMP_ENTRY_STC));

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(devNum, CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E, &numIpEcmpEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", devNum);

        /* Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 0.
            Expected: GT_OK */
        ecmpEntryIndex = 0;
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ecmpEntryIndex, &ecmpEntry);

        /* Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 7890.
            Expected: GT_OK */
        ecmpEntryIndex = 7890 % numIpEcmpEntries;
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ecmpEntryIndex, &ecmpEntry);

        /* Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 12287.
            Expected: GT_OK */
        ecmpEntryIndex = numIpEcmpEntries - 1;
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ecmpEntryIndex, &ecmpEntry);

        /* Call function with not NULL ecmpEntryPtr and ecmpEntryIndex = 12287.
            Expected: GT_OUT_OF_RANGE */
        ecmpEntryIndex = numIpEcmpEntries;
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, ecmpEntryIndex, &ecmpEntry);

        /* Call function with NULL ecmpEntryPtr.
            Expected: GT_BAD_PTR */
        ecmpEntryIndex = 0;
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
            "%d, %d, status = NULL", devNum, ecmpEntryIndex);
    }

    ecmpEntryIndex = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpHashNumBitsSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       startBit,
    IN GT_U32                       numOfBits
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpHashNumBitsSet)
{
/*
    ITERATE_DEVICES (eArch devices only)
    - Call with startBit = 0, numOfBits = 1.
      Expected: GT_OK.
    - Call with startBit = 16, numOfBits = 1.
      Expected: GT_OK.
    - Call with startBit = 31, numOfBits = 1.
      Expected: GT_OK.
    - Call with startBit = 32, numOfBits = 1.
      Expected: GT_OUT_OF_RANGE.
    - Call with startBit = 16, numOfBits = 0.
      Expected: GT_OUT_OF_RANGE.
    - Call with startBit = 16, numOfBits = 8.
      Expected: GT_OK.
    - Call with startBit = 16, numOfBits = 16.
      Expected: GT_OK.
    - Call with startBit = 16, numOfBits = 17.
      Expected: GT_OUT_OF_RANGE.
    - Call with startBit = 22, numOfBits = 10.
      Expected: GT_OK.
    - Call with startBit = 22, numOfBits = 11.
      Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  startBit, startBitGet;
    GT_U32                  numOfBits, numOfBitsGet;
    GT_U32                  notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        startBit = 0;
        numOfBits = 1;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBit, numOfBits);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBitGet, &numOfBitsGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBitGet, numOfBitsGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(startBit, startBitGet,
                        "get another startBit than was set: ", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfBits, numOfBitsGet,
                        "get another numOfBits than was set: ", dev);

        startBit = 16;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBit, numOfBits);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBitGet, &numOfBitsGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBitGet, numOfBitsGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(startBit, startBitGet,
                        "get another startBit than was set: ", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfBits, numOfBitsGet,
                        "get another numOfBits than was set: ", dev);

        startBit = 31;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBit, numOfBits);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBitGet, &numOfBitsGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBitGet, numOfBitsGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(startBit, startBitGet,
                        "get another startBit than was set: ", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfBits, numOfBitsGet,
                        "get another numOfBits than was set: ", dev);

        startBit = 32;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, startBit, numOfBits);

        startBit = 16;
        numOfBits = 0;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, startBit, numOfBits);

        numOfBits = 8;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBit, numOfBits);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBitGet, &numOfBitsGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBitGet, numOfBitsGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(startBit, startBitGet,
                        "get another startBit than was set: ", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfBits, numOfBitsGet,
                        "get another numOfBits than was set: ", dev);

        numOfBits = 16;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBit, numOfBits);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBitGet, &numOfBitsGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBitGet, numOfBitsGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(startBit, startBitGet,
                        "get another startBit than was set: ", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfBits, numOfBitsGet,
                        "get another numOfBits than was set: ", dev);

        numOfBits = 17;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, startBit, numOfBits);

        startBit = 22;
        numOfBits = 10;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBit, numOfBits);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBitGet, &numOfBitsGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, startBitGet, numOfBitsGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(startBit, startBitGet,
                        "get another startBit than was set: ", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfBits, numOfBitsGet,
                        "get another numOfBits than was set: ", dev);

        numOfBits = 11;

        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, startBit, numOfBits);
    }

    startBit = 0;
    numOfBits = 1;

    /* For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpHashNumBitsSet(dev, startBit, numOfBits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpHashNumBitsGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *startBitPtr,
    OUT GT_U32                      *numOfBitsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpHashNumBitsGet)
{
    /*
    ITERATE_DEVICES (eArch devices only)
    - Call function with not NULL startBitPtr and not NULL numOfBitsPtr
      Expected: GT_OK.
    - Call function with not NULL startBitPtr and NULL numOfBitsPtr
      Expected: GT_BAD_PTR.
    - Call function with NULL startBitPtr and not NULL numOfBitsPtr
      Expected: GT_BAD_PTR.
    */
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  startBit;
    GT_U32                  numOfBits;
    GT_U32                  notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBit, &numOfBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBit, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChIpEcmpHashNumBitsGet(dev, NULL, &numOfBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBit, &numOfBits);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpHashNumBitsGet(dev, &startBit, &numOfBits);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpHashSeedValueSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       seed
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpHashSeedValueSet)
{
/*
    ITERATE_DEVICES (eArch devices only)
    - Call with seed = 1
      Expected: GT_OK.
    - Call with seed = 0xb47ac093
      Expected: GT_OK.
    - Call with seed = 0xffffffff
      Expected: GT_OK.
    - Call with seed = 0
      Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  seed, seedGet;
    GT_U32                  notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        seed = 1;

        st = cpssDxChIpEcmpHashSeedValueSet(dev, seed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, seed);

        st = cpssDxChIpEcmpHashSeedValueGet(dev, &seedGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, seedGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(seed, seedGet,
                        "get another seed than was set: ", dev);

        seed = 0xb47ac093;

        st = cpssDxChIpEcmpHashSeedValueSet(dev, seed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, seed);

        st = cpssDxChIpEcmpHashSeedValueGet(dev, &seedGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, seedGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(seed, seedGet,
                        "get another seed than was set: ", dev);

        seed = 0xffffffff;

        st = cpssDxChIpEcmpHashSeedValueSet(dev, seed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, seed);

        st = cpssDxChIpEcmpHashSeedValueGet(dev, &seedGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, seedGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(seed, seedGet,
                        "get another seed than was set: ", dev);

        seed = 0;

        st = cpssDxChIpEcmpHashSeedValueSet(dev, seed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, seed);
    }

    seed = 1;

    /* For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpEcmpHashSeedValueSet(dev, seed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpHashSeedValueSet(dev, seed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpHashSeedValueGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *seedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpHashSeedValueGet)
{
    /*
    ITERATE_DEVICES (eArch devices only)
    - Call function with not NULL seedPtr
      Expected: GT_OK.
    - Call function with NULL seedPtr
      Expected: GT_BAD_PTR.
    */
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  seed;
    GT_U32                  notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChIpEcmpHashSeedValueGet(dev, &seed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChIpEcmpHashSeedValueGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpEcmpHashSeedValueGet(dev, &seed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpHashSeedValueGet(dev, &seed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/

UTF_TEST_CASE_MAC(cpssDxChIpEcmpIndirectNextHopEntryTable)
{

    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                      entryIndex;
    GT_U32                      nextHopIndex;
    GT_U32                      nextHopIndexGet;
    GT_U32                      maxNextHopIndex;
    GT_U32                      maxEntryIndex;



    /* this feature is on Aldrin2 devices  only*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev,(UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS)) ;

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        maxEntryIndex = prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(dev);

        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &maxNextHopIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        entryIndex = 0;
        nextHopIndex = maxNextHopIndex/2;

        st = cpssDxChIpEcmpIndirectNextHopEntrySet(dev, entryIndex,nextHopIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex,nextHopIndex);

        st = cpssDxChIpEcmpIndirectNextHopEntryGet(dev, entryIndex,&nextHopIndexGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryIndex,nextHopIndex);

        UTF_VERIFY_EQUAL2_STRING_MAC(nextHopIndex, nextHopIndexGet, "nextHopIndex : %d is not equal to nextHopIndexGet %d ", nextHopIndex,nextHopIndexGet);


        st = cpssDxChIpEcmpIndirectNextHopEntrySet(dev, entryIndex,maxNextHopIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, entryIndex,nextHopIndex);


        entryIndex = maxEntryIndex;
        nextHopIndex = 0;

        st = cpssDxChIpEcmpIndirectNextHopEntrySet(dev, entryIndex,nextHopIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, entryIndex,nextHopIndex);

        st = cpssDxChIpEcmpIndirectNextHopEntryGet(dev, entryIndex,&nextHopIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, entryIndex,nextHopIndex);




    }

    entryIndex = 0;
    nextHopIndex = 0;

    /* For not active devices check that function returns non GT_OK. */

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev,(UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS)) ;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpEcmpIndirectNextHopEntrySet(dev, entryIndex,nextHopIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpEcmpIndirectNextHopEntryGet(dev, entryIndex,&nextHopIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;


    st = cpssDxChIpEcmpIndirectNextHopEntrySet(dev, entryIndex,nextHopIndex);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, entryIndex,nextHopIndex);

    st = cpssDxChIpEcmpIndirectNextHopEntryGet(dev, entryIndex,&nextHopIndex);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, entryIndex,nextHopIndex);

}





/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChIp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChIp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv4PrefixSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv4PrefixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv4PrefixInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6PrefixSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6PrefixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6PrefixInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLttWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpLttRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRouteEntriesWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRouteEntriesRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMcRouteEntriesWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMcRouteEntriesRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterNextHopTableAgeBitsEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterNextHopTableAgeBitsEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMLLPairWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMLLPairRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMLLLastBitWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterArpAddrWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterArpAddrRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpNatEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpNatEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpNatDroppedPacketsCntGet)

    /* Test filling Tables */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFillMllPairTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFillUcRouteTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFillMcRouteTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFillLttTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFillRouterArpAddrTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpHashNumBitsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpHashNumBitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpHashSeedValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpHashSeedValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpIndirectNextHopEntryTable)
UTF_SUIT_END_TESTS_MAC(cpssDxChIp)

