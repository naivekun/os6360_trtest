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
* @file cpssPxIngressUT.c
*
* @brief Unit tests for cpssPxIngress that provides
* CPSS implementation for Pipe ingress processing.
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

#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/* defines */

/* Default valid value for port id */
#define PORT_CTRL_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressTpidEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressTpidEntrySet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with entryIndex[0..3], with valid tpidEntryPtr.
    Expected: GT_OK.
    1.2. Call with out of range entryIndex[4] and other params from 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with entryIndex[0] and tpidEntryPtr->size == 3.
    Expected: GT_BAD_PARAM.
    1.4. Call with entryIndex[0] and tpidEntryPtr->size == 26.
    Expected: GT_BAD_PARAM.
    1.5. Call with entryIndex[0] and tpidEntryPtr->size == 0.
    Expected: GT_BAD_PARAM.
    1.6. Call with tpidEntryPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      entry;

    CPSS_PX_INGRESS_TPID_ENTRY_STC tpidEntryArr[] = {{0x1020, 2, GT_TRUE}, {0x3040, 4, GT_FALSE}, {0x5060, 8, GT_TRUE}, {0x8080, 8, GT_FALSE}};
    CPSS_PX_INGRESS_TPID_ENTRY_STC *tpidEntry = tpidEntryArr;
    CPSS_PX_INGRESS_TPID_ENTRY_STC tpidEntryGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with entryIndex[0..3], with valid tpidEntryPtr.
            Expected: GT_OK. */
        for (entry = 0; entry < CPSS_PX_INGRESS_TPID_TABLE_ENTRIES_MAX_CNS; entry++)
        {
            st = cpssPxIngressTpidEntrySet(dev, entry, &tpidEntry[entry]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, entry);

            st = cpssPxIngressTpidEntryGet(dev, entry, &tpidEntryGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, entry);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tpidEntry[entry].val, tpidEntryGet.val,
                       "get another tpidEntryGet.val than was set: %d", tpidEntryGet.val);
            UTF_VERIFY_EQUAL1_STRING_MAC(tpidEntry[entry].size, tpidEntryGet.size,
                       "get another tpidEntryGet.size than was set: %d", tpidEntryGet.size);
            UTF_VERIFY_EQUAL1_STRING_MAC(tpidEntry[entry].valid, tpidEntryGet.valid,
                       "get another tpidEntryGet.valid than was set: %d", tpidEntryGet.valid);
        }
        /*  1.2. Call with out of range entryIndex[4] and other params from 1.1.
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxIngressTpidEntrySet(dev, entry, &tpidEntry[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, entry);

        /*  1.3. Call with entryIndex[0] and tpidEntryPtr->size == 3.
            Expected: GT_BAD_PARAM. */
        tpidEntry[0].size = 3;
        st = cpssPxIngressTpidEntrySet(dev, 0, &tpidEntry[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, entry);

        /*  1.4. Call with entryIndex[0] and tpidEntryPtr->size == 26.
            Expected: GT_BAD_PARAM. */
        tpidEntry[0].size = 26;
        st = cpssPxIngressTpidEntrySet(dev, 0, &tpidEntry[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, entry);

        /*  1.5. Call with entryIndex[0] and tpidEntryPtr->size == 0.
            Expected: GT_BAD_PARAM. */
        tpidEntry[0].size = 0;
        st = cpssPxIngressTpidEntrySet(dev, 0, &tpidEntry[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, entry);

        /* Restore TPID entry size */
        tpidEntry[0].size = 2;

        /*  1.6. Call with tpidEntryPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressTpidEntrySet(dev, 0, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, entry);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressTpidEntrySet(dev, 0, &tpidEntry[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressTpidEntrySet(dev, 0, &tpidEntry[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssPxIngressTpidEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressTpidEntryGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with entryIndex[0..3], with valid tpidEntryPtr.
    Expected: GT_OK.
    1.2. Call with out of range entryIndex[4].
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with tpidEntryPtr == NULL.
    Expected: GT_BAD_PTR.

*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_U32      entry;

    CPSS_PX_INGRESS_TPID_ENTRY_STC tpidEntryGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with entryIndex[0..3], with valid tpidEntryPtr.
            Expected: GT_OK. */
        for (entry = 0; entry < CPSS_PX_INGRESS_TPID_TABLE_ENTRIES_MAX_CNS; entry++)
        {
            st = cpssPxIngressTpidEntryGet(dev, entry, &tpidEntryGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, entry);
        }

        /*  1.2. Call with out of range entryIndex[4].
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxIngressTpidEntryGet(dev, entry, &tpidEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, entry);

        /*  1.3. Call with tpidEntryPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressTpidEntryGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, entry);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressTpidEntryGet(dev, 0, &tpidEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressTpidEntryGet(dev, 0, &tpidEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressEtherTypeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    IN  GT_U16                          etherType,
    IN  GT_BOOL                         valid
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressEtherTypeSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
     1.1. Call with all CPSS_PX_INGRESS_ETHERTYPE_ENT members.
    Expected: GT_OK.
    1.2. Call with wrong enum values table.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_INGRESS_ETHERTYPE_ENT configType;
    GT_U16                        etherType[] = {0x7800, 0x7810, 0x7820, 0x7830, 0x40};
    GT_BOOL                       valid;
    GT_U16                        etherTypeGet;
    GT_BOOL                       validGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    valid = GT_TRUE;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all CPSS_PX_INGRESS_ETHERTYPE_ENT members.
            Expected: GT_OK. */
        for (configType = 0; configType < CPSS_PX_INGRESS_ETHERTYPE_LAST_E; configType++)
        {
            st = cpssPxIngressEtherTypeSet(dev, configType, etherType[configType], valid);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, configType);

            st = cpssPxIngressEtherTypeGet(dev, configType, &etherTypeGet, &validGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, configType);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(etherType[configType], etherTypeGet,
                       "get another etherTypeGet than was set: %d", etherTypeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(valid, validGet,
                       "get another validGet than was set: %d", validGet);

            valid = !valid;
        }

        /*  1.2. Call with wrong enum values table.
            Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssPxIngressEtherTypeSet
                            (dev, configType, etherType[0], valid),
                            configType);
    }

    /* Reset value to valid config type */
    configType = CPSS_PX_INGRESS_ETHERTYPE_IPV4_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressEtherTypeSet(dev, configType, etherType[configType], valid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressEtherTypeSet(dev, configType, etherType[configType], valid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssPxIngressEtherTypeGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    OUT GT_U16                          *etherTypePtr,
    OUT GT_BOOL                         *validPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressEtherTypeGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
     1.1. Call with all CPSS_PX_INGRESS_ETHERTYPE_ENT members.
    Expected: GT_OK.
    1.2. Call with wrong enum values table.
    Expected: GT_BAD_PARAM.
    1.2. Call with NULL pointer parameters.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_INGRESS_ETHERTYPE_ENT configType;
    GT_U16                        etherTypeGet;
    GT_BOOL                       validGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with all CPSS_PX_INGRESS_ETHERTYPE_ENT members.
            Expected: GT_OK. */
        for (configType = 0; configType < CPSS_PX_INGRESS_ETHERTYPE_LAST_E; configType++)
        {
            st = cpssPxIngressEtherTypeGet(dev, configType, &etherTypeGet, &validGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, configType);
        }

        /*  1.2. Call with wrong enum values table.
            Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssPxIngressEtherTypeGet
                            (dev, configType, &etherTypeGet, &validGet),
                            configType);

        /*  1.2. Call with NULL pointer parameters.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressEtherTypeGet(dev, configType, NULL, &validGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, configType);

        st = cpssPxIngressEtherTypeGet(dev, configType, &etherTypeGet, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, configType);
    }

    /* Reset value to valid config type */
    configType = CPSS_PX_INGRESS_ETHERTYPE_IPV4_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressEtherTypeGet(dev, configType, &etherTypeGet, &validGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressEtherTypeGet(dev, configType, &etherTypeGet, &validGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortMacDaByteOffsetSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          offset
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortMacDaByteOffsetSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices and all available physical ports)
    1.1. Call with all valid(even)/invalid(odd) MAC DA byte offsets.
        Expected: GT_OK         - for valid offsets.
                  GT_BAD_PARAM  - for invalid offsets.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM.
    1.3. Call function for out of bound value for port number.
        Expected: GT_BAD_PARAM.
    1.4. Call with out of range MAC DA byte offsets.
        Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      offset = 0;
    GT_U32      offsetGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for (offset = 0; offset <= CPSS_PX_INGRESS_MAC_DA_BYTE_OFFSET_MAX_CNS; offset++)
            {
                st = cpssPxIngressPortMacDaByteOffsetSet(dev, port, offset);
                UTF_VERIFY_EQUAL3_PARAM_MAC((offset & 1) ? GT_BAD_PARAM : GT_OK, st, dev, port, offset);
                if (st == GT_OK)
                {
                    st = cpssPxIngressPortMacDaByteOffsetGet(dev, port, &offsetGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(offset, offsetGet,
                                                 "get another offset value than was set: %d, %d", dev, port);
                }
            }
        }

        /* Set valid MAC DA byte offset */
        offset = 10;

        /* 1.2. For all active devices go over all non available physical ports.
            Expected: GT_BAD_PARAM.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port
               Expected: GT_BAD_PARAM.*/
            st = cpssPxIngressPortMacDaByteOffsetSet(dev, port, offset);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. Call function for out of bound value for port number.
           Expected: GT_BAD_PARAM. */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortMacDaByteOffsetSet(dev, port, offset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. Call with out of range MAC DA byte offsets.
            Expected: GT_BAD_PARAM  */
        offset = CPSS_PX_INGRESS_MAC_DA_BYTE_OFFSET_MAX_CNS + 2;
        st = cpssPxIngressPortMacDaByteOffsetSet(dev, port, offset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, offset);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortMacDaByteOffsetSet(dev, port, offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortMacDaByteOffsetSet(dev, port, offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortMacDaByteOffsetGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *offsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortMacDaByteOffsetGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices and all available physical ports)
    1.1. Call with non-null offsetPtr.
         Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM.
    1.3. For active device call function with out of bound value for port number.
        Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      offsetGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports.
                Expected: GT_OK. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxIngressPortMacDaByteOffsetGet(dev, port, &offsetGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports.
            Expected: GT_BAD_PARAM. */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortMacDaByteOffsetGet(dev, port, &offsetGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device call function with out of bound value for port number.
                Expected: GT_BAD_PARAM.*/
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortMacDaByteOffsetGet(dev, port, &offsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortMacDaByteOffsetGet(dev, port, &offsetGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortMacDaByteOffsetGet(dev, port, &offsetGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortPacketTypeKeySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortPacketTypeKeySet)
{
/*
    ITERATE_DEVICE (Go over all Px devices for all physical ports and all source port profiles)
    1.1. Call with valid portKeyPtr.
    Expected: GT_OK.
    1.2. Call with out of range portKeyPtr->srcPortProfile.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with all non-valid portKeyPtr.
    Expected: GT_BAD_PTR.
    1.4. For all active devices go over all non available physical ports
    Expected: GT_BAD_PARAM
    1.5. For out of bound value for port number.
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKeyGet;
    GT_U32      udbp;
    GT_U32      portProfile;
    CPSS_PX_UDB_PAIR_KEY_STC    portUdbPairArr[] = {{CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  0}, {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  8},
                                                    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 16}, {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 24},
                                                    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  0}, {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E, 16},
                                                    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E, 32}, {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E, 64}};


    cpssOsMemCpy(portKey.portUdbPairArr, portUdbPairArr, CPSS_PX_UDB_PAIRS_MAX_CNS * sizeof(CPSS_PX_UDB_PAIR_KEY_STC));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for (portProfile = 0; portProfile < CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS; portProfile++)
            {
                portKey.srcPortProfile = portProfile;

                st = cpssPxIngressPortPacketTypeKeySet(dev, port, &portKey);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                st = cpssPxIngressPortPacketTypeKeyGet(dev, port, &portKeyGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(portKey.srcPortProfile, portKeyGet.srcPortProfile,
                                                 "get another portKeyGet.srcPortProfile than was set: %d, %d", dev, port);

                for (udbp = 0; udbp < CPSS_PX_UDB_PAIRS_MAX_CNS; udbp++)
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(portKey.portUdbPairArr[udbp].udbAnchorType,
                                                 portKeyGet.portUdbPairArr[udbp].udbAnchorType,
                                                 "get another portUdbPairArr[%d].udbAnchorType than was set: %d, %d", udbp, dev, port);
                    UTF_VERIFY_EQUAL3_STRING_MAC(portKey.portUdbPairArr[udbp].udbByteOffset,
                                                 portKeyGet.portUdbPairArr[udbp].udbByteOffset,
                                                 "get another portUdbPairArr[%d].udbByteOffset than was set: %d, %d", udbp, dev, port);
                }
            }

            /*  1.2. Call with out of range portKeyPtr->srcPortProfile.
                Expected: GT_OUT_OF_RANGE. */
            portKey.srcPortProfile = portProfile;

            st = cpssPxIngressPortPacketTypeKeySet(dev, port, &portKey);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);

            /*  1.3. Call with all non-valid portKeyPtr.
                Expected: GT_BAD_PTR. */
            st = cpssPxIngressPortPacketTypeKeySet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        /* Set valid port profile */
        portKey.srcPortProfile = 0;

        /* 1.4. For all active devices go over all non available physical ports.
           Expected: GT_BAD_PARAM */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortPacketTypeKeySet(dev, port, &portKey);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.5. For out of bound value for port number.
           Expected: GT_BAD_PARAM */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortPacketTypeKeySet(dev, port, &portKey);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortPacketTypeKeySet(dev, port, &portKey);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortPacketTypeKeySet(dev, port, &portKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortPacketTypeKeyGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortPacketTypeKeyGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices for all physical ports)
    1.1. Call with valid portKeyPtr.
    Expected: GT_OK.
    1.2. Call with non-valid portKeyPtr.
    Expected: GT_BAD_PTR.
    1.3. For all active devices go over all non available physical ports.
    Expected: GT_BAD_PARAM.
    1.4. For out of bound value for port number.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKeyGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  1.1. For all active devices go over all available physical ports.
            Expected: GT_OK */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxIngressPortPacketTypeKeyGet(dev, port, &portKeyGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /*  1.2. Call with non-valid portKeyPtr.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressPortPacketTypeKeyGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

        /*  1.3. For all active devices go over all non available physical ports.
            Expected: GT_BAD_PARAM. */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortPacketTypeKeyGet(dev, port, &portKeyGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.4. For out of bound value for port number.
           Expected: GT_BAD_PARAM. */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortPacketTypeKeyGet(dev, port, &portKeyGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortPacketTypeKeyGet(dev, port, &portKeyGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortPacketTypeKeyGet(dev, port, &portKeyGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPacketTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                     packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPacketTypeKeyEntrySet)
{
/*
    ITERATE_DEVICE (All over all Px devices)
    1.1. Call with packetType[0..31], with valid keyDataPtr and keyMaskPtr.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with keyData.profileIndex[128] .
    Expected: GT_BAD_PARAM.
    1.4. Call with keyMask.profileIndex[128] .
    Expected: GT_BAD_PARAM.
    1.5. Call with keyDataPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with keyMaskPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call with Mask/Data ip2me index out of range (PIPE A1)
    Expected: GT_OUT_OF_RANGE.
    1.8. Call with Mask/Data ip2me index out of range (PIPE A0)
    Expected: GT_OK.
*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType;

    GT_U32                                      profileIndex = 0;
    GT_U32                                      ip2meIndex = 0;

    GT_ETHERADDR   macAddrData =                {{0xab, 0xcd, 0xef, 0x00, 0x00, 0x01}};
    GT_ETHERADDR   macAddrMask =                {{0xff, 0xff, 0xff, 0x00, 0x00, 0x01}};
    GT_BOOL        isEqual;

    CPSS_PX_UDB_PAIR_DATA_STC                   udbPairsDataArr[] = {{{0x01, 0x02}}, {{0x03, 0x04}}, {{0x05, 0x06}}, {{0x07, 0x08}}};
    CPSS_PX_UDB_PAIR_DATA_STC                   udbPairsMaskArr[] = {{{0xff, 0xff}}, {{0xff, 0xff}}, {{0xff, 0xff}}, {{0xff, 0xff}}};

    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyDataGet;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMaskGet;

    cpssOsMemCpy(keyData.udbPairsArr, udbPairsDataArr, CPSS_PX_UDB_PAIRS_MAX_CNS * sizeof(CPSS_PX_UDB_PAIR_DATA_STC));
    cpssOsMemCpy(keyMask.udbPairsArr, udbPairsMaskArr, CPSS_PX_UDB_PAIRS_MAX_CNS * sizeof(CPSS_PX_UDB_PAIR_DATA_STC));
    cpssOsMemCpy(&keyData.macDa, &macAddrData,sizeof(GT_ETHERADDR));
    cpssOsMemCpy(&keyMask.macDa, &macAddrMask,sizeof(GT_ETHERADDR));

    keyData.etherType = 0x8800;
    keyData.isLLCNonSnap = GT_TRUE;
    keyMask.etherType = 0xffff;
    keyMask.isLLCNonSnap = GT_TRUE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with packetType[0..31], with valid keyDataPtr and keyMaskPtr.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
        {
            for (profileIndex = 0; profileIndex < CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS; profileIndex++)
            {
                for (ip2meIndex = 0; ip2meIndex < 8; ip2meIndex++)
                {
                    keyData.profileIndex = profileIndex;
                    keyMask.profileIndex = profileIndex;
                    keyData.ip2meIndex = ip2meIndex;
                    keyMask.ip2meIndex = ip2meIndex;

                    st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, packetType, keyData.profileIndex, keyData.ip2meIndex);

                    st = cpssPxIngressPacketTypeKeyEntryGet(dev, packetType, &keyDataGet, &keyMaskGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, packetType, keyDataGet.profileIndex, keyDataGet.ip2meIndex);

                    /* Verifying values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(keyData.etherType, keyDataGet.etherType,
                            "get another keyDataGet.etherType than was set: %d", keyDataGet.etherType);
                    UTF_VERIFY_EQUAL1_STRING_MAC(keyData.isLLCNonSnap, keyDataGet.isLLCNonSnap,
                            "get another keyDataGet.isLLCNonSnap than was set: %d", keyDataGet.isLLCNonSnap);
                    UTF_VERIFY_EQUAL1_STRING_MAC(keyMask.etherType, keyMaskGet.etherType,
                            "get another keyMaskGet.etherType than was set: %d", keyMaskGet.etherType);
                    UTF_VERIFY_EQUAL1_STRING_MAC(keyMask.isLLCNonSnap, keyMaskGet.isLLCNonSnap,
                            "get another keyMaskGet.isLLCNonSnap than was set: %d", keyMaskGet.isLLCNonSnap);


                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyData.macDa, (GT_VOID*)&keyDataGet.macDa, sizeof(GT_ETHERADDR)))
                            ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                                "get another keyDataGet.macDa than was set: %d", dev);
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyMask.macDa, (GT_VOID*)&keyMaskGet.macDa, sizeof(GT_ETHERADDR)))
                            ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                                "get another keyMaskGet.macDa than was set: %d", dev);


                    UTF_VERIFY_EQUAL1_STRING_MAC(keyData.profileIndex, keyDataGet.profileIndex,
                            "get another keyDataGet.profileIndex than was set: %d", keyDataGet.profileIndex);
                    UTF_VERIFY_EQUAL1_STRING_MAC(keyMask.profileIndex, keyMaskGet.profileIndex,
                            "get another keyMaskGet.profileIndex than was set: %d", keyMaskGet.profileIndex);

                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyData.udbPairsArr,
                                                (GT_VOID*)&keyDataGet.udbPairsArr,
                                                CPSS_PX_UDB_PAIRS_MAX_CNS * sizeof(CPSS_PX_UDB_PAIR_DATA_STC)))
                                                ? GT_TRUE : GT_FALSE;

                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                                "get another keyDataGet.udbPairsArr than was set: %d", dev);
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyMask.udbPairsArr,
                                                (GT_VOID*)&keyMaskGet.udbPairsArr,
                                                CPSS_PX_UDB_PAIRS_MAX_CNS * sizeof(CPSS_PX_UDB_PAIR_DATA_STC)))
                                                ? GT_TRUE : GT_FALSE;

                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                                "get another keyMask.udbPairsArr than was set: %d", dev);
                }

                if (cpssDeviceRunCheck_onEmulator()) 
                {
                    profileIndex += 15; /*reduce iterations due to performance issue !!! */
                }
            }

            keyData.etherType += 0x100;
            keyData.isLLCNonSnap = !keyData.isLLCNonSnap;

            if(cpssDeviceRunCheck_onEmulator())
            {
                packetType += 5;/*reduce iterations due to performance issue !!! */
            }
        }

        /*  1.2 Call with out of range packetType[32] and other params from 1.1.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, packetType);

        /*  1.3. Call with keyData.profileIndex[128] .
            Expected: GT_BAD_PARAM. */
        packetType = 0;
        keyData.profileIndex = profileIndex;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, packetType);

        /*  1.4. Call with keyMask.profileIndex[128] .
            Expected: GT_BAD_PARAM. */
        keyMask.profileIndex = profileIndex;
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, packetType);

        /*  1.5. Call with keyDataPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, NULL, &keyMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, packetType);

        /*  1.6. Call with keyMaskPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, packetType);

        keyData.profileIndex = 0;
        keyMask.profileIndex = 0;
        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
        {
            /*  1.7. Call with ip2me index out of range (PIPE A1)
                Expected: GT_OUT_OF_RANGE. */
            keyData.ip2meIndex = 8;
            keyMask.ip2meIndex = 0;
            st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, packetType);

            keyData.ip2meIndex = 0;
            keyMask.ip2meIndex = 8;
            st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, packetType);
        }
        else
        {
            /*  1.8. Call with ip2me index out of range (PIPE A0)
                Expected: GT_OK. */
            keyData.ip2meIndex = 8;
            keyMask.ip2meIndex = 0;
            st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

            keyData.ip2meIndex = 0;
            keyMask.ip2meIndex = 8;
            st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    packetType = 0;
    keyData.profileIndex  = keyMask.profileIndex = 0;
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPacketTypeKeyEntrySet(dev, packetType, &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPacketTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPacketTypeKeyEntryGet)
{
/*
    ITERATE_DEVICE (All over all Px devices)
    1.1. Call with packetType[0..31], with valid keyDataPtr and keyMaskPtr.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_BAD_PARAM
    1.3. Call with keyDataPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with keyMaskPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType = 0;

    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyDataGet;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMaskGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with packetType[0..31], with valid keyDataPtr and keyMaskPtr.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
        {
            st = cpssPxIngressPacketTypeKeyEntryGet(dev, packetType, &keyDataGet, &keyMaskGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);
            if (GT_FALSE == PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(0, keyDataGet.ip2meIndex, packetType);
                UTF_VERIFY_EQUAL1_PARAM_MAC(0, keyMaskGet.ip2meIndex, packetType);
            }
        }

        /*  1.2  Call with out of range packetType[32] and other params from 1.1.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressPacketTypeKeyEntryGet(dev, packetType, &keyDataGet, &keyMaskGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, packetType);

        packetType = 0;
        /*  1.3. Call with keyDataPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressPacketTypeKeyEntryGet(dev, packetType, NULL, &keyMaskGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, packetType);

        /*  1.4. Call with keyMaskPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressPacketTypeKeyEntryGet(dev, packetType, &keyDataGet, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, packetType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPacketTypeKeyEntryGet(dev, packetType, &keyDataGet, &keyMaskGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPacketTypeKeyEntryGet(dev, packetType, &keyDataGet, &keyMaskGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortMapPacketTypeFormatEntrySet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    IN  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortMapPacketTypeFormatEntrySet)
{
/*
    ITERATE_DEVICE (All over all Px devices for source and destination port map tables)
    1.1. Call with packetType[0..31], with valid packetTypeFormatPtr.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with packetTypeFormatPtr.indexConst < MIN.
    Expected: GT_BAD_PARAM.
    1.4. Call with packetTypeFormatPtr.indexConst > MAX.
    Expected: GT_BAD_PARAM.
    1.5. Call with packetTypeFormatPtr.indexMax > MAX.
    Expected: GT_BAD_PARAM.
    1.6. Call with not valid packetTypeFormatPtr.bitFieldArr.
    Expected: GT_BAD_PARAM.
    1.7. Call with packetTypeFormatPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.8  Call with wrong enum values table type.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType=0;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    GT_BOOL             isEqual;
    GT_U32              i;

    typedef struct {
        GT_32 minVal;
        GT_32 maxVal;
        GT_U32 mask;
    } SIGNED_RANGE_STC;

    typedef struct {
        GT_U32 minVal;
        GT_U32 maxVal;
    } UNSIGNED_RANGE_STC;

    SIGNED_RANGE_STC indexConst[] = {{-2048, 2047, 0xfff}, {-4096, 4095, 0x1fff}};
    UNSIGNED_RANGE_STC indexMax[] = {{0, 4095}, {0, 8191}};

    CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC bitFieldArr[] = {{8, 0, 2}, {16, 2, 4}, {32, 4, 3}, {0, 0, 0}};
    CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC bitField3[3] = {{64, 0, 0},  {0, 8, 0}, {0, 0, 9}};
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetFormat;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetFormatGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsMemCpy(&packetFormat.bitFieldArr, &bitFieldArr,
                 CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_MAX_CNS * sizeof(CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
        {
            packetFormat.indexConst = indexConst[tableType].minVal;
            packetFormat.indexMax = indexMax[tableType].minVal;

            /*  1.1. Call with packetType[0..31], with valid packetTypeFormatPtr.
                Expected: GT_OK. */
            for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
            {
                st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

                st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev, tableType, packetType, &packetFormatGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(packetFormat.indexMax, packetFormatGet.indexMax,
                           "get another packetFormatGet.indexMax than was set: %d", packetFormatGet.indexMax);
                UTF_VERIFY_EQUAL1_STRING_MAC(packetFormat.indexConst, packetFormatGet.indexConst,
                           "get another packetFormatGet.indexConst than was set: %d", packetFormatGet.indexConst);
                isEqual = (0 == cpssOsMemCmp((GT_VOID*)&packetFormat.bitFieldArr, (GT_VOID*)&packetFormatGet.bitFieldArr,
                           CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_MAX_CNS * sizeof(CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC)))
                          ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                             "get another packetFormatGet.bitFieldArr than was set: %d", dev);

                packetFormat.indexConst = indexConst[tableType].maxVal - packetType;
                packetFormat.indexMax = indexMax[tableType].maxVal - packetType;
            }
            /*  1.2 Call with out of range packetType[32] and other params from 1.1.
                Expected: GT_OUT_OF_RANGE. */
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, tableType, packetType);

            /*  1.3. Call with packetTypeFormatPtr.indexConst < minVal.
                Expected: GT_BAD_PARAM. */
            packetType = 10;
            packetFormat.indexConst = indexConst[tableType].minVal - 1;
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);

            /*  1.4. Call with packetTypeFormatPtr.indexConst > maxVal.
                Expected: GT_BAD_PARAM. */
            packetFormat.indexConst = indexConst[tableType].maxVal + 1;
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);

            /*  1.5. Call with packetTypeFormatPtr.indexMax > maxVal.
                Expected: GT_BAD_PARAM. */
            packetFormat.indexConst = 0;
            packetFormat.indexMax = indexMax[tableType].maxVal + 1;
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);

            /*  1.6. Call with not valid packetTypeFormatPtr.bitFieldArr.
                Expected: GT_BAD_PARAM. */
            packetFormat.indexConst = 0;
            packetFormat.indexMax = indexMax[tableType].maxVal;
            for (i = 0; i < 2; i++)
            {
                packetFormat.bitFieldArr[3] = bitField3[i];
                st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);
            }

            /* reset bit bitFieldArr[3] */
            packetFormat.bitFieldArr[3] = bitFieldArr[3];

            /*  1.7. Call with packetTypeFormatPtr == NULL and other params from 1.1.
                Expected: GT_BAD_PTR. */
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, tableType, packetType);

            /*  1.8  Call with wrong enum values table type.
                Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssPxIngressPortMapPacketTypeFormatEntrySet
                                (dev, tableType, packetType, &packetFormat),
                                 tableType);
        }
    }

    for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
    {
        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* go over all non active devices */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssPxIngressPortMapPacketTypeFormatEntrySet(dev, tableType, packetType, &packetFormat);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);

        packetType += 15;
    }

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortMapPacketTypeFormatEntryGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    OUT CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortMapPacketTypeFormatEntryGet)
{
/*
    ITERATE_DEVICE (All over all Px devices for source and destination port map tables)
    1.1. Call with packetType[0..31], with valid packetTypeFormatPtr.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with packetTypeFormatPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.4  Call with wrong enum values table type.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType=0;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetFormatGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
        {
            /*  1.1. Call with packetType[0..31], with valid packetTypeFormatPtr.
                Expected: GT_OK. */
            for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
            {
                st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev, tableType, packetType, &packetFormatGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);
            }
            /*  1.2 Call with out of range packetType[32] and other params from 1.1.
                Expected: GT_OUT_OF_RANGE. */
            st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev, tableType, packetType, &packetFormatGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, tableType, packetType);

            /* Reset packet type to be valid */
            packetType = 10;

            /*  1.3. Call with packetTypeFormatPtr == NULL and other params from 1.1.
                Expected: GT_BAD_PTR. */
            st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev, tableType, packetType, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, tableType, packetType);

            /*  1.4  Call with wrong enum values table type.
                Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssPxIngressPortMapPacketTypeFormatEntryGet
                                (dev, tableType, packetType, &packetFormatGet),
                                 tableType);
        }
    }

    for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
    {
        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* go over all non active devices */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev, tableType, packetType, &packetFormatGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssPxIngressPortMapPacketTypeFormatEntryGet(dev, tableType, packetType, &packetFormatGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, packetType);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortMapEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    IN  CPSS_PX_PORTS_BMP                           portsBmp,
    IN  GT_BOOL                                     bypassLagDesignatedBitmap
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortMapEntrySet)
{
/*
    ITERATE_DEVICE (All over all Px devices for source and destination port map tables)
    1.1. Call with entryIndex[0..maxVal].
    Expected: GT_OK.
    1.2. Call with out of range entryIndex[maxVal+1].
    Expected: GT_OUT_OF_RANGE.
    1.3  Call with wrong enum values table type.
    Expected: GT_BAD_PARAM.
    1.4  Call with not valid port bitmap.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    GT_U32 entryIndex = 0;
    GT_U32 indexMax[] = {4096, 8192};

    CPSS_PX_PORTS_BMP portBitmap = 0;
    CPSS_PX_PORTS_BMP portBitmapGet;
    GT_BOOL bypassLagDesignatedBitmapGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
        {
            /*  1.1. Call with entryIndex[0..maxVal].
                Expected: GT_OK. */
            for (entryIndex = 0; entryIndex < indexMax[tableType]; entryIndex++)
            {
                st = cpssPxIngressPortMapEntrySet(dev, tableType, entryIndex, portBitmap, GT_FALSE);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tableType, entryIndex);

                st = cpssPxIngressPortMapEntryGet(dev, tableType, entryIndex, &portBitmapGet, 
                                                  &bypassLagDesignatedBitmapGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tableType, entryIndex);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(portBitmap, portBitmapGet,
                           "get another portBitmapGet than was set: %d", entryIndex);

                if (tableType == CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E)
                {
                    st = cpssPxIngressPortMapEntrySet(dev, tableType, entryIndex, portBitmap, GT_TRUE); 
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tableType, entryIndex);

                    st = cpssPxIngressPortMapEntryGet(dev, tableType, entryIndex, &portBitmapGet, 
                                                      &bypassLagDesignatedBitmapGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tableType, entryIndex);

                    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
                    {
                        /* Verifying values */
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, bypassLagDesignatedBitmapGet,
                                   "get another bypassLagDesignatedBitmapGet than was set: %d", entryIndex);
                    }
                    else
                    {
                        /* Verifying values */
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, bypassLagDesignatedBitmapGet,
                                   "get another bypassLagDesignatedBitmapGet than was set: %d", entryIndex);

                    }
                }

                /* Max 17 bits */
                portBitmap = (portBitmap + 1) & 0x1FFFF;
            }
            /*  1.2  Call with out of range entryIndex[maxVal+1].
                Expected: GT_OUT_OF_RANGE. */
            st = cpssPxIngressPortMapEntrySet(dev, tableType, entryIndex, portBitmap,GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, tableType, entryIndex);

            /* Reset entryIndex to be valid */
            entryIndex = 100;

            /*  1.3  Call with wrong enum values table type.
                Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssPxIngressPortMapEntrySet
                                (dev, tableType, entryIndex, portBitmap, 0),
                                 tableType);

            /*  1.4  Call with not valid port bitmap.
                Expected: GT_BAD_PARAM. */
            st = cpssPxIngressPortMapEntrySet(dev, tableType, entryIndex, 0xffffff, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, entryIndex);
        }
    }

    for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
    {
        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* go over all non active devices */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssPxIngressPortMapEntrySet(dev, tableType, entryIndex, portBitmap, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, entryIndex);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssPxIngressPortMapEntrySet(dev, tableType, entryIndex, portBitmap, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, entryIndex);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortMapEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr,
    OUT GT_BOOL                                     *bypassLagDesignatedBitmapPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortMapEntryGet)
{
/*
    ITERATE_DEVICE (All over all Px devices for source and destination port map tables)
    1.1. Call with entryIndex[0..maxVal].
    Expected: GT_OK.
    1.2. Call with out of range entryIndex[maxVal+1].
    Expected: GT_OUT_OF_RANGE.
    1.3  Call with wrong enum values table type.
    Expected: GT_BAD_PARAM.
    1.4  Call with portsBmpPtr == NULL.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    GT_U32 entryIndex = 0;
    GT_U32 indexMax[] = {4096, 8192};

    CPSS_PX_PORTS_BMP portBitmapGet;
    GT_BOOL     bypassLagDesignatedBitmapGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
        {
            /*  1.1. Call with entryIndex[0..maxVal].
                Expected: GT_OK. */
            for (entryIndex = 0; entryIndex < indexMax[tableType]; entryIndex++)
            {
                st = cpssPxIngressPortMapEntryGet(dev, tableType, entryIndex, &portBitmapGet,
                                                  &bypassLagDesignatedBitmapGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tableType, entryIndex);
            }
            /*  1.2  Call with out of range entryIndex[maxVal+1].
                Expected: GT_OUT_OF_RANGE. */
            st = cpssPxIngressPortMapEntryGet(dev, tableType, entryIndex, &portBitmapGet,
                                              &bypassLagDesignatedBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, tableType, entryIndex);

            /* Reset entryIndex to be valid */
            entryIndex = 100;

            /*  1.3  Call with wrong enum values table type.
                Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssPxIngressPortMapEntryGet
                                (dev, tableType, entryIndex, &portBitmapGet, &bypassLagDesignatedBitmapGet),
                                 tableType);

            /*  1.4  Call with portsBmpPtr == NULL.
                Expected: GT_BAD_PTR. */
            st = cpssPxIngressPortMapEntryGet(dev, tableType, entryIndex, NULL, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, tableType, entryIndex);
        }
    }

    for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
    {
        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* go over all non active devices */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssPxIngressPortMapEntryGet(dev, tableType, entryIndex, &portBitmapGet,
                                              &bypassLagDesignatedBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, entryIndex);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssPxIngressPortMapEntryGet(dev, tableType, entryIndex, &portBitmapGet, 
                                          &bypassLagDesignatedBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, tableType, entryIndex);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortTargetEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortTargetEnableSet)
{
/*
    ITERATE_DEVICE (All over all Px devices for all physical ports)
    1.1. Call with enable = GT_TRUE/GT_FALSE .
        Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM.
    1.3. Call function for out of bound value for port number.
        Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxIngressPortTargetEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssPxIngressPortTargetEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                         "get another enableGet value than was set: %d %d", dev, port);
            /* Revert value */
            enable = !enable;
        }

        /* 1.2. For all active devices go over all non available physical ports.
            Expected: GT_BAD_PARAM.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortTargetEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. Call function for out of bound value for port number.
           Expected: GT_BAD_PARAM. */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortTargetEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortTargetEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortTargetEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortTargetEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortTargetEnableGet)
{
/*
    ITERATE_DEVICE (All over all Px devices for all physical ports)
    1.1. For all active devices go over all available physical ports.
        Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM.
    1.3. Call function for out of bound value for port number.
        Expected: GT_BAD_PARAM.
    1.4. Call function with NULL pointer.
        Expected: GT_BAD_PTR.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxIngressPortTargetEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports.
            Expected: GT_BAD_PARAM.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortTargetEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. Call function for out of bound value for port number.
            Expected: GT_BAD_PARAM. */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortTargetEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. Call function with NULL pointer.
            Expected: GT_BAD_PTR. */
        port = PORT_CTRL_VALID_PHY_PORT_CNS;
        st = cpssPxIngressPortTargetEnableGet(dev, port, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortTargetEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortTargetEnableGet(dev, port, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortDsaTagEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortDsaTagEnableGet)
{
/*
    ITERATE_DEVICE (All over all Px devices for all physical ports)
    1.1. For all active devices go over all available physical ports.
        Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM.
    1.3. Call function for out of bound value for port number.
        Expected: GT_BAD_PARAM.
    1.4. Call function with NULL pointer.
        Expected: GT_BAD_PTR.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxIngressPortDsaTagEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports.
            Expected: GT_BAD_PARAM.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortDsaTagEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. Call function for out of bound value for port number.
            Expected: GT_BAD_PARAM. */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortDsaTagEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. Call function with NULL pointer.
            Expected: GT_BAD_PTR. */
        port = PORT_CTRL_VALID_PHY_PORT_CNS;
        st = cpssPxIngressPortDsaTagEnableGet(dev, port, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortDsaTagEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortDsaTagEnableGet(dev, port, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPacketTypeErrorGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT GT_U32                                      *errorCounterPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPacketTypeErrorGet)
{
/*
    ITERATE_DEVICE (All over all Px devices)
    1.1. For all active devices call function with valid parameters.
         Expected: GT_OK.
    1.2. Call function with NULL pointers.
         Expected: GT_BAD_PTR.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32                                      errorCounterGet;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  errorKeyGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. For all active devices call function with valid parameters.
            Expected: GT_OK. */
        st = cpssPxIngressPacketTypeErrorGet(dev, &errorCounterGet, &errorKeyGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. Call function with NULL pointer.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressPacketTypeErrorGet(dev, NULL, &errorKeyGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssPxIngressPacketTypeErrorGet(dev, &errorCounterGet, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPacketTypeErrorGet(dev, &errorCounterGet, &errorKeyGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPacketTypeErrorGet(dev, &errorCounterGet, &errorKeyGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressForwardingPortMapExceptionSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  CPSS_PX_PORTS_BMP                           portsBmp
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressForwardingPortMapExceptionSet)
{
/*
    ITERATE_DEVICE (All over all Px devices for source and destination port map tables)
    1.1. Call function with valid parameteres.
    Expected: GT_OK.
    1.2  Call with wrong enum CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT values.
    Expected: GT_BAD_PARAM.
    1.3  Call with not valid port bitmap.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_PORTS_BMP portBitmap = 0x1234;
    CPSS_PX_PORTS_BMP portBitmapGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
        {
            /*  1.1. Call Call function wit valid parameteres.
                Expected: GT_OK. */
            st = cpssPxIngressForwardingPortMapExceptionSet(dev, tableType, portBitmap);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tableType);

            st = cpssPxIngressForwardingPortMapExceptionGet(dev, tableType, &portBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tableType);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(portBitmap, portBitmapGet,
                       "get another portBitmapGet than was set: %d", dev);

            /* Max 17 bits */
            portBitmap = (~portBitmap) & 0x1FFFF;

            /*  1.2  Call with wrong enum CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT values.
                Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssPxIngressForwardingPortMapExceptionSet
                                (dev, tableType, portBitmap),
                                 tableType);

            /*  1.3  Call with not valid port bitmap.
                Expected: GT_BAD_PARAM. */
            st = cpssPxIngressForwardingPortMapExceptionSet(dev, tableType, 0xffffff);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tableType);
        }
    }

    for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
    {
        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* go over all non active devices */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssPxIngressForwardingPortMapExceptionSet(dev, tableType, portBitmap);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tableType);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssPxIngressForwardingPortMapExceptionSet(dev, tableType, portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tableType);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressForwardingPortMapExceptionGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressForwardingPortMapExceptionGet)
{
/*
    ITERATE_DEVICE (All over all Px devices for source and destination port map tables)
    1.1. Call function with valid parameteres.
    Expected: GT_OK.
    1.2  Call with wrong enum CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT values.
    Expected: GT_BAD_PARAM.
    1.3  Call with NULL pointer parameter.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_PORTS_BMP portBitmapGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
        {
            /*  1.1. Call Call function wit valid parameteres.
                Expected: GT_OK. */
            st = cpssPxIngressForwardingPortMapExceptionGet(dev, tableType, &portBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tableType);

            /*  1.2  Call with wrong enum CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT values.
                Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssPxIngressForwardingPortMapExceptionGet
                                (dev, tableType, &portBitmapGet),
                                 tableType);

            /*  1.3  Call with NULL pointer parameter.
                Expected: GT_BAD_PTR. */
            st = cpssPxIngressForwardingPortMapExceptionGet(dev, tableType, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, tableType);
        }
    }

    for (tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E; tableType < CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST; tableType++)
    {
        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* go over all non active devices */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssPxIngressForwardingPortMapExceptionGet(dev, tableType, &portBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tableType);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssPxIngressForwardingPortMapExceptionGet(dev, tableType, &portBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tableType);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortDsaTagEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortDsaTagEnableSet)
{
/*
    ITERATE_DEVICE (All over all Px devices for all physical ports)
    1.1. Call with enable = GT_TRUE/GT_FALSE .
        Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM.
    1.3. Call function for out of bound value for port number.
        Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssPxIngressPortDsaTagEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssPxIngressPortDsaTagEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                         "get another enableGet value than was set: %d %d", dev, port);
            /* Revert value */
            enable = !enable;
        }

        /* 1.2. For all active devices go over all non available physical ports.
            Expected: GT_BAD_PARAM.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortDsaTagEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. Call function for out of bound value for port number.
           Expected: GT_BAD_PARAM. */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortDsaTagEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortDsaTagEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortDsaTagEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPacketTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PACKET_TYPE             packetType,
    OUT GT_BOOL                         *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPacketTypeKeyEntryEnableGet)
{
/*
    ITERATE_DEVICE (All over all Px devices for all packet types)
    1.1. Call with packetType[0..31], with valid enablePtr.
        Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
        Expected: GT_BAD_PARAM.
    1.3. Call with enablePtr == NULL and other params from 1.1.
        Expected: GT_BAD_PTR.
*/

    GT_STATUS           st     = GT_OK;

    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType=0;
    GT_BOOL             enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with packetType[0..31], with valid enablePtr.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
        {
            st = cpssPxIngressPacketTypeKeyEntryEnableGet(dev, packetType, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);
        }

        /*  1.2  Call with out of range packetType[32] and other params from 1.1.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressPacketTypeKeyEntryEnableGet(dev, packetType, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, packetType);

        packetType = 0;
        /*  1.3. Call with enablePtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxIngressPacketTypeKeyEntryEnableGet(dev, packetType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, packetType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPacketTypeKeyEntryEnableGet(dev, packetType, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPacketTypeKeyEntryEnableGet(dev, packetType, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPacketTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  GT_BOOL                                     enable
);
*/
UTF_TEST_CASE_MAC(cpssPxIngressPacketTypeKeyEntryEnableSet)
{
/*
    ITERATE_DEVICE (All over all Px devices)
    1.1. Call with packetType[0..31], with valid enable.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType=0;
    GT_BOOL             enable = GT_TRUE;
    GT_BOOL             enableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with packetType[0..31], with valid keyDataPtr.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS; packetType++)
        {
            st = cpssPxIngressPacketTypeKeyEntryEnableSet(dev, packetType, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

            st = cpssPxIngressPacketTypeKeyEntryEnableGet(dev, packetType, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enableGet than was set: %d", enableGet);

            enable = !enable;
        }

        /*  1.2 Call with out of range packetType[32] and other params from 1.1.
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxIngressPacketTypeKeyEntryEnableSet(dev, packetType, enable);
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
        st = cpssPxIngressPacketTypeKeyEntryEnableSet(dev, packetType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPacketTypeKeyEntryEnableSet(dev, packetType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressPortRedirectSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORTS_BMP               targetPortsBmp
)
*/
UTF_TEST_CASE_MAC(cpssPxIngressPortRedirectSet)
{
/*

    ITERATE_DEVICES_PHY_PORTS (PX)
    1.1.1. Call with valid port bitmap[0/BIT_8/BIT_17-1].
    Expected: GT_OK
    1.1.2  Call with not valid port bitmap[BIT_17].
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS                   st              = GT_OK;
    GT_U8                       dev             = 0;
    GT_PHYSICAL_PORT_NUM        port            = 0;
    CPSS_PX_PORTS_BMP           portBitmap      = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with valid port bitmap[0].
               Expected: GT_OK
            */
            portBitmap = 0;
            st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portBitmap);

            /* 1.1.1. Call with valid port bitmap[BIT_8].
               Expected: GT_OK
            */
            portBitmap = BIT_8;
            st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portBitmap);

            /* 1.1.1. Call with valid port bitmap[BIT17-1].
               Expected: GT_OK
            */
            portBitmap = BIT_17-1;
            st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portBitmap);

            /* 1.1.2  Call with out of range port bitmap[BIT_17].
               Expected: GT_BAD_PTR
            */
            portBitmap = BIT_17;
            st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "%d, %d, %d",
                                                dev, port, portBitmap);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        portBitmap = 0;
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;
    portBitmap = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxIngressPortRedirectSet(dev, port, portBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressFilteringEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    IN  GT_BOOL                 enable
)
*/
UTF_TEST_CASE_MAC(cpssPxIngressFilteringEnableSet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with with valid enable params and validate.
    Expected: GT_OK.
    1.2. Call with with invalid direction.
    Expected: GT_BAD_PARAM.
    1.3. Call with with invalid packet type.
    Expected: GT_BAD_PARAM.
    2. Go over not applicalble devices
    Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS                   st              = GT_OK;
    GT_U8                       dev             = 0;
    GT_BOOL                     enable = 0;
    GT_BOOL                     enableGet = 0;
    CPSS_PX_PACKET_TYPE         packetType = 0;
    CPSS_DIRECTION_ENT          direction = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == GT_FALSE)
        {
            enable = GT_TRUE;
            direction = CPSS_DIRECTION_INGRESS_E;
            packetType = 0;
            st = cpssPxIngressFilteringEnableSet(dev, direction, packetType,
                enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
                direction, packetType, enable);
            continue;
        }


        /*  1.1. Call with with valid enable params and validate.
            Expected: GT_OK. */
        enable = GT_FALSE;
        for(packetType = 0; packetType < 32; packetType++)
        {
            for(direction = CPSS_DIRECTION_INGRESS_E;
                direction <= CPSS_DIRECTION_EGRESS_E; direction++)
            {
                enable = (enable == GT_TRUE) ? GT_FALSE : GT_TRUE;
                st = cpssPxIngressFilteringEnableSet(dev, direction, packetType,
                    enable);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, direction, packetType,
                    enable);
            }
        }

        /* Verify */
        enable = GT_FALSE;
        for(packetType = 0; packetType < 32; packetType++)
        {
            for(direction = CPSS_DIRECTION_INGRESS_E;
                direction <= CPSS_DIRECTION_EGRESS_E; direction++)
            {
                enable = (enable == GT_TRUE) ? GT_FALSE : GT_TRUE;
                st = cpssPxIngressFilteringEnableGet(dev, direction, packetType,
                    &enableGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, packetType);
                UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
            }
        }

        /*  1.2. Call with with invalid direction.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressFilteringEnableSet(dev, 100, packetType, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, 100, packetType, enable);

        /*  1.3. Call with with invalid packet type.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressFilteringEnableSet(dev, direction, 32, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, direction, 32, enable);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  2. Go over not applicalble devices
        Expected: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressFilteringEnableSet(dev, direction, packetType, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, direction,
            packetType, enable);
    }

    /*  3. Call function with out of bound value for device id.
        Expected: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressFilteringEnableSet(dev, direction, packetType, enable);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, direction,
        packetType, enable);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressFilteringEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    OUT GT_BOOL                *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxIngressFilteringEnableGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with with valid enable params and validate.
    Expected: GT_OK.
    1.2. Call with with invalid direction.
    Expected: GT_BAD_PARAM.
    1.3. Call with with invalid packet type.
    Expected: GT_BAD_PARAM.
    2. Go over not applicalble devices
    Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS                   st              = GT_OK;
    GT_U8                       dev             = 0;
    GT_BOOL                     enable;
    GT_BOOL                     enableGet;
    CPSS_PX_PACKET_TYPE         packetType = 0;
    CPSS_DIRECTION_ENT          direction = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == GT_FALSE)
        {
            direction = CPSS_DIRECTION_INGRESS_E;
            packetType = 0;
            st = cpssPxIngressFilteringEnableGet(dev, direction, packetType,
                &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
                direction, packetType);
            continue;
        }

        /*  1.1. Call with with valid enable params and validate.
            Expected: GT_OK. */
        enable = GT_FALSE;
        for(packetType = 0; packetType < 32; packetType++)
        {
            for(direction = CPSS_DIRECTION_INGRESS_E;
                direction <= CPSS_DIRECTION_EGRESS_E; direction++)
            {
                enable = (enable == GT_TRUE) ? GT_FALSE : GT_TRUE;
                st = cpssPxIngressFilteringEnableGet(dev, direction, packetType,
                    &enableGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, packetType);
            }
        }

        /*  1.2. Call with with invalid direction.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressFilteringEnableGet(dev, 100, packetType, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, 100, packetType);

        /*  1.3. Call with with invalid packet type.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressFilteringEnableGet(dev, direction, 32, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, direction, 32);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  2. Go over not applicalble devices
    Expected: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressFilteringEnableGet(dev, direction, packetType, &enableGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, direction,
            packetType);
    }

    /*  3. Call function with out of bound value for device id.
        Expected: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressFilteringEnableGet(dev, direction, packetType, &enableGet);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, direction, packetType);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressIp2MeEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxIngressIp2MeEntrySet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with with valid enable params and validate.
    Expected: GT_OK.
    1.2. Call with with invalid entryIndex.
    Expected: GT_BAD_PARAM.
    1.3. Call with with prefix out of length.
    Expected: GT_OUT_OF_RANGE.
    1.4. Call with with NULL ptr to IP2ME entry
    Expected: GT_BAD_PTR.
    2. Go over not applicalble devices
    Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS                   st              = GT_OK;
    GT_U8                       dev             = 0;
    GT_U32                      entryIndex      = 0;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntryGet;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC entryArr[] =
    {
        {GT_TRUE, 1,  GT_FALSE,  {{0xFF000000}}},
        {GT_TRUE, 32, GT_FALSE,  {{0xFF000000}}},
        {GT_TRUE, 1,  GT_TRUE,   {{0x00000000}}},/*IPv6*/
        {GT_TRUE, 128,  GT_TRUE, {{0x00000000}}},/*IPv6*/
        {GT_FALSE, 16, GT_FALSE, {{0x00000000}}},
        {GT_FALSE, 16,  GT_TRUE, {{0x00000000}}},/*IPv6*/
        {GT_TRUE,  16, GT_FALSE, {{0x0000FF00}}},
    };
    entryArr[2].ipAddr.ipv6Addr.u32Ip[0] = 0xFF000000;
    entryArr[2].ipAddr.ipv6Addr.u32Ip[1] = 0x00FF0000;
    entryArr[2].ipAddr.ipv6Addr.u32Ip[2] = 0x0000FF00;
    entryArr[2].ipAddr.ipv6Addr.u32Ip[3] = 0x000000FF;
    entryArr[3].ipAddr.ipv6Addr.u32Ip[0] = 0xFF000000;
    entryArr[3].ipAddr.ipv6Addr.u32Ip[1] = 0x00FF0000;
    entryArr[3].ipAddr.ipv6Addr.u32Ip[2] = 0x0000FF00;
    entryArr[3].ipAddr.ipv6Addr.u32Ip[3] = 0x000000FF;
    entryArr[5].ipAddr.ipv6Addr.u32Ip[0] = 0xFF000000;
    entryArr[5].ipAddr.ipv6Addr.u32Ip[1] = 0x00FF0000;
    entryArr[5].ipAddr.ipv6Addr.u32Ip[2] = 0x0000FF00;
    entryArr[5].ipAddr.ipv6Addr.u32Ip[3] = 0x000000FF;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == GT_FALSE)
        {
            st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, &entryArr[0]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
                entryIndex);

            /* Call with with NULL ptr to IP2ME entry
             Expected: GT_NOT_APPLICABLE_DEVICE. */
            entryIndex = 1;
            st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
                entryIndex);
            continue;
        }

        /*  1.1. Call with with valid enable params and validate.
            Expected: GT_OK. */
        for(entryIndex = 1; entryIndex < 8; entryIndex++)
        {
            st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, &entryArr[entryIndex - 1]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        for(entryIndex = 1; entryIndex < 8; entryIndex++)
        {
            st = cpssPxIngressIp2MeEntryGet(dev, entryIndex, &ip2meEntryGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].valid,
                ip2meEntryGet.valid);
            UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].prefixLength,
                ip2meEntryGet.prefixLength);
            UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].isIpv6,
                ip2meEntryGet.isIpv6);
            if(GT_FALSE == ip2meEntryGet.isIpv6)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].ipAddr.ipv4Addr.u32Ip,
                    ip2meEntryGet.ipAddr.ipv4Addr.u32Ip);
            }
            else
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].ipAddr.ipv6Addr.u32Ip[0],
                    ip2meEntryGet.ipAddr.ipv6Addr.u32Ip[0]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].ipAddr.ipv6Addr.u32Ip[1],
                    ip2meEntryGet.ipAddr.ipv6Addr.u32Ip[1]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].ipAddr.ipv6Addr.u32Ip[2],
                    ip2meEntryGet.ipAddr.ipv6Addr.u32Ip[2]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(entryArr[entryIndex - 1].ipAddr.ipv6Addr.u32Ip[3],
                    ip2meEntryGet.ipAddr.ipv6Addr.u32Ip[3]);
            }
        }
        entryIndex = 1;

        /*  1.2. Call with with invalid entryIndex.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressIp2MeEntrySet(dev, 8, &entryArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, entryIndex);

        st = cpssPxIngressIp2MeEntrySet(dev, 0, &entryArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, entryIndex);

        /*  1.3. Call with with prefix out of length.
            Expected: GT_OUT_OF_RANGE. */
        entryArr[0].prefixLength = 0;
        st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, &entryArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, 0);

        entryArr[0].prefixLength = 33;
        st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, &entryArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, 0);
        entryArr[0].prefixLength = 1;

        entryArr[2].prefixLength = 129;
        st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, &entryArr[2]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, 0);

        /*  1.4. Call with with NULL ptr to IP2ME entry
            Expected: GT_BAD_PTR. */
        entryIndex = 1;
        st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, entryIndex);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*  2. Go over not applicalble devices
        Expected: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, &entryArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            entryIndex);
    }

    /*  3. Call function with out of bound value for device id.
        Expected: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressIp2MeEntrySet(dev, entryIndex, &entryArr[0]);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
        entryIndex);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxIngressIp2MeEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxIngressIp2MeEntryGet)
{
/*
    ITERATE_DEVICES (Pipe)
    1.1. Call with with valid enable params and validate.
    Expected: GT_OK.
    1.2. Call with with invalid entryIndex.
    Expected: GT_BAD_PARAM.
    1.3. Call with with NULL ptr to IP2ME entry
    Expected: GT_BAD_PTR.
    2. Go over not applicalble devices
    Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS                   st              = GT_OK;
    GT_U8                       dev             = 0;
    GT_U32                      entryIndex      = 0;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntryGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev) == GT_FALSE)
        {
            st = cpssPxIngressIp2MeEntryGet(dev, entryIndex, &ip2meEntryGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
                entryIndex);

            /* Call with with NULL ptr to IP2ME entry
               Expected: GT_NOT_APPLICABLE_DEVICE. */
            entryIndex = 1;
            st = cpssPxIngressIp2MeEntryGet(dev, entryIndex, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
                entryIndex);
            continue;
        }

        /*  1.1. Call with with valid enable params and validate.
            Expected: GT_OK. */
        for(entryIndex = 1; entryIndex < 8; entryIndex++)
        {
            st = cpssPxIngressIp2MeEntryGet(dev, entryIndex, &ip2meEntryGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }

        /*  1.2. Call with with invalid entryIndex.
            Expected: GT_BAD_PARAM. */
        st = cpssPxIngressIp2MeEntryGet(dev, 8, &ip2meEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, entryIndex);

        st = cpssPxIngressIp2MeEntryGet(dev, 0, &ip2meEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, entryIndex);

        /*  1.3. Call with with NULL ptr to IP2ME entry
            Expected: GT_BAD_PTR. */
        entryIndex = 1;
        st = cpssPxIngressIp2MeEntryGet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, entryIndex);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    entryIndex = 1;

    /*  2. Go over not applicalble devices
        Expected: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxIngressIp2MeEntryGet(dev, entryIndex, &ip2meEntryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            entryIndex);
    }

    /*  3. Call function with out of bound value for device id.
        Expected: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxIngressIp2MeEntryGet(dev, entryIndex, &ip2meEntryGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
        entryIndex);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxIngress suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxIngress)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressTpidEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressTpidEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressEtherTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressEtherTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortMacDaByteOffsetSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortMacDaByteOffsetGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortPacketTypeKeySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortPacketTypeKeyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPacketTypeKeyEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPacketTypeKeyEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortMapPacketTypeFormatEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortMapPacketTypeFormatEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortMapEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortMapEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortTargetEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortTargetEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPacketTypeErrorGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressForwardingPortMapExceptionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressForwardingPortMapExceptionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortDsaTagEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortDsaTagEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPacketTypeKeyEntryEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPacketTypeKeyEntryEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressPortRedirectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressFilteringEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressFilteringEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressIp2MeEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxIngressIp2MeEntryGet)
UTF_SUIT_END_TESTS_MAC(cpssPxIngress)


