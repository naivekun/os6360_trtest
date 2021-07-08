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
* @file cpssDxChBrgMcUT.c
*
* @brief Unit tests for CPSS DXCh Multicast Group facility API.
*
* @version   28
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* get flood VIDX */
#define   TGF_BRG_FLOOD_VIDX_MAC(_dev) \
((GT_U16)((PRV_CPSS_PP_MAC(_dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)? 0x3FF : 0xFFF))

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcIpv6BytesSelectSet
(
    IN GT_U8  devNum,
    IN GT_U8  dipBytesSelectMap[4],
    IN GT_U8  sipBytesSelectMap[4]
);

*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcIpv6BytesSelectSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with dipBytesSelectMap [{1,2,4,0}] and sipBytesSelectMap [{3,5,7,15}]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgMcIpv6BytesSelectGet.
    Expected: GT_OK and the same dipBytesSelectMap and sipBytesSelectMap.
    1.3. Call function with out of range dipBytesSelectMap[{16, 0, 1, 2}]
         and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with out of range sipBytesSelectMap[{0, 1, 2, 16}]
         and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call function with null dipBytesSelectMap [NULL]
         and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call function with null sipBytesSelectMap[NULL]
         and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call function with equal values of bytes numbers
         dipBytesSelectMap[{0, 0, 1, 1}] and other valid parameters from 1.1.
    Expected: GT_OK.
    1.8. Call function with equal values of bytes numbers
         sipBytesSelectMap[{0, 0, 1, 1}] and other valid parameters from 1.1.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U8       dipBytesSelectMap[4] = {1,2,4,0};   /* correct values */
    GT_U8       sipBytesSelectMap[4] = {3,5,7,15};

    GT_U8       retDipBytesSelectMap[4] = {0,0,0,0};
    GT_U8       retSipBytesSelectMap[4] = {0,0,0,0};

    GT_BOOL     failureWas;

    /* arrays with predefined values for tests */
    GT_U8       dipBytesSelectMap_16_0_1_2[4] = {16, 0, 1, 2};
    GT_U8       sipBytesSelectMap_0_1_2_16[4] = {0, 1, 2, 16};
    GT_U8       ipBytesSelectMap_0_0_1_1[4]   = {0, 0, 1, 1};


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with dipBytesSelectMap [{1,2,4,0}] and sipBytesSelectMap*/
        /* [{3,5,7,15}] Expected: GT_OK.                                     */

        /*dipBytesSelectMap = {1,2,4,0};
        sipBytesSelectMap = {3,5,7,15};*/

        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, dipBytesSelectMap, sipBytesSelectMap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call cpssDxChBrgMcIpv6BytesSelectGet. Expected: GT_OK
           and the same dipBytesSelectMap and sipBytesSelectMap.    */

        st = cpssDxChBrgMcIpv6BytesSelectGet(dev,  retDipBytesSelectMap, retSipBytesSelectMap);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgMcIpv6BytesSelectGet: %d", dev);

        failureWas = (0 == cpssOsMemCmp((GT_VOID*)dipBytesSelectMap, (GT_VOID*)retDipBytesSelectMap,
                                      sizeof(dipBytesSelectMap))) ? GT_FALSE : GT_TRUE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, failureWas,
            "get another dipBytesSelectMap than was set: %d", dev);

        failureWas = (0 == cpssOsMemCmp((GT_VOID*)sipBytesSelectMap, (GT_VOID*)retSipBytesSelectMap,
                                      sizeof(sipBytesSelectMap))) ? GT_FALSE : GT_TRUE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, failureWas,
            "get another sipBytesSelectMap than was set: %d", dev);

        /* 1.3. Call function with out of range dipBytesSelectMap
           [{16, 0, 1, 2}] and other valid parameters from 1.1.
           Expected: NOT GT_OK. */

        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, dipBytesSelectMap_16_0_1_2, sipBytesSelectMap);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.4. Call function with out of range sipBytesSelectMap
           [{0, 1, 2, 16}] and other valid parameters from 1.1.
           Expected: NOT GT_OK. */

        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, dipBytesSelectMap, sipBytesSelectMap_0_1_2_16);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.5. Call function with null dipBytesSelectMap [NULL] and other
           valid parameters from 1.1. Expected: GT_BAD_PTR. */

        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, NULL, sipBytesSelectMap);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /* 1.6. Call function with null sipBytesSelectMap [NULL] and other
           valid parameters from 1.1. Expected: GT_BAD_PTR. */

        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, dipBytesSelectMap, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, sipBytesSelectMap = NULL", dev);


        /* 1.7. Call function with equal values of bytes numbers
           dipBytesSelectMap[{0, 0, 1, 1}] and other valid parameters
           from 1.1. Expected: GT_OK.   */

        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, ipBytesSelectMap_0_0_1_1, sipBytesSelectMap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* 1.8. Call function with equal values of bytes numbers
           sipBytesSelectMap[{0, 0, 1, 1}] and other valid parameters
           from 1.1. Expected: GT_OK.   */

        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, dipBytesSelectMap, ipBytesSelectMap_0_0_1_1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcIpv6BytesSelectSet(dev, dipBytesSelectMap, sipBytesSelectMap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcIpv6BytesSelectSet(dev, dipBytesSelectMap, sipBytesSelectMap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcIpv6BytesSelectGet
(
    IN  GT_U8  devNum,
    OUT GT_U8  dipBytesSelectMap[4],
    OUT GT_U8  sipBytesSelectMap[4]
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcIpv6BytesSelectGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-null dipBytesSelectMap and non-null sipBytesSelectMap.
    Expected: GT_OK.
    1.2. Call function with null dipBytesSelectMap[NULL] and non-null sipBytesSelectMap.
    Expected: GT_BAD_PTR.
    1.3. Call function with non-null dipBytesSelectMap and null sipBytesSelectMap [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U8       dipBytesSelectMap[4];
    GT_U8       sipBytesSelectMap[4];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-null dipBytesSelectMap and non-null
           sipBytesSelectMap. Expected: GT_OK.  */

        st = cpssDxChBrgMcIpv6BytesSelectGet(dev, dipBytesSelectMap, sipBytesSelectMap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with null dipBytesSelectMap[NULL] and
           non-null sipBytesSelectMap. Expected: GT_BAD_PTR.    */

        st = cpssDxChBrgMcIpv6BytesSelectGet(dev, NULL, sipBytesSelectMap);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /* 1.3. Call function with null sipBytesSelectMap[NULL] and
           non-null dipBytesSelectMap. Expected: GT_BAD_PTR.    */

        st = cpssDxChBrgMcIpv6BytesSelectGet(dev, dipBytesSelectMap, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, sipBytesSelectMap = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcIpv6BytesSelectGet(dev, dipBytesSelectMap, sipBytesSelectMap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcIpv6BytesSelectGet(dev, dipBytesSelectMap, sipBytesSelectMap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcEntryWrite
(
    IN GT_U8                devNum,
    IN GT_U16               vidx,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcEntryWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vidx [10] and portBitmapPtr->ports[{0x0000AAAA / (1 << 27)-1, 0}].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgMcEntryRead with the same vidx, non-NULL portBitmapPtr.
    Expected: GT_OK and the same portBitmapPtr.
    1.3. Call function with out of range vidx [0xFFF]
         and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with vidx [50] and null portBitmapPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Check portBitmapPtr with 28 bits used. Call with vidx [10]
         and portBitmapPtr->ports[{ 1 <<27, 0}].
    Expected: GT_OK for Cheetah2,3 and non GT_OK for Cheetah.
    1.6. Check portBitmapPtr with 29 bits used. Call with vidx [10]
         and portBitmapPtr->ports[{ 1<<28, 0}].
    Expected: non GT_OK.
    1.7. Check portBitmapPtr with bits 1-32 unused and bit 33 used.
         Call with vidx [10] and portBitmapPtr->ports[{ 0, 1}].
    Expected: non GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                dev;
    GT_U16               vidx;
    CPSS_PORTS_BMP_STC   portBitmap;
    CPSS_PORTS_BMP_STC   retPortBitmap;
    GT_BOOL              failureWas;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&retPortBitmap);

        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Call with vidx [10] and portBitmapPtr->ports
           [{0x0000AAAA / (1<<27)-1, 0}]. Expected: GT_OK. */

        vidx = 10;
        portBitmap.ports[0] = 0x0000AAAA;
        UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(dev,portBitmap);

        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        portBitmap.ports[0] = ((GT_U32)(1 << 27)) - 1;
        UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(dev,portBitmap);

        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        /* 1.2. Call cpssDxChBrgMcEntryRead with the same vidx, non-NULL
           portBitmapPtr. Expected: GT_OK and the same portBitmapPtr.   */

        st = cpssDxChBrgMcEntryRead(dev, vidx, &retPortBitmap);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChBrgMcEntryRead: %d, %d", dev, vidx);

        failureWas = (0 == cpssOsMemCmp((GT_VOID*)&portBitmap, (GT_VOID*)&retPortBitmap,
                                      sizeof(portBitmap))) ? GT_FALSE : GT_TRUE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, failureWas,
            "get another portBitmap than was set: %d, %d", dev, vidx);


        /* 1.3. Call function with out of range vidx [0xFFF] and other
           valid parameters from 1.1. Expected: NOT GT_OK.  */
        vidx = 0xFFF;
        portBitmap.ports[0] = 0x0000AAAA;
        UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(dev,portBitmap);

        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* the device supports the VIDX 0xFFF (no longer impact on UC)*/
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);
        }
        else
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        /* 1.3.1 Call function with out of range vidx [_12K-1] for all devices except xCat3
           and other valid parameters from 1.1. Expected: NOT_GT_OK / GT_OK for xCat3  */
        vidx = _12K-1;
        portBitmap.ports[0] = 0x0000AAAA;
        UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(dev,portBitmap);

        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);

        if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

            st = cpssDxChBrgMcEntryRead(dev, vidx, &retPortBitmap);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChBrgMcEntryRead: %d, %d", dev, vidx);

            failureWas = (0 == cpssOsMemCmp((GT_VOID*)&portBitmap, (GT_VOID*)&retPortBitmap,
                                          sizeof(portBitmap))) ? GT_FALSE : GT_TRUE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, failureWas,
                "get another portBitmap than was set: %d, %d", dev, vidx);
        }
        else
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);


        vidx = 10; /* restore */

        /* 1.4. Call function with vidx [50] and null portBitmapPtr [NULL].
           Expected: GT_BAD_PTR.    */
        vidx = 50;

        st = cpssDxChBrgMcEntryWrite(dev, vidx, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, vidx);

        /* 1.5. Check portBitmapPtr with 28 bits used. Call with vidx [10]
           and portBitmapPtr->ports[{ 1<<27, 0}]. Expected: GT_OK for
           Cheetah2,3 and non GT_OK for Cheetah.  */

        vidx = 10;
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);

        if(UTF_GET_MAX_PHY_PORTS_NUM_MAC(dev) < 31)
        {
            portBitmap.ports[0] = ((GT_U32)(1 << 27));
        }
        else
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,(UTF_GET_MAX_PHY_PORTS_NUM_MAC(dev) - 1));
            UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(dev,portBitmap);
        }

        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
               "not Cheetah device: %d, portBitmap.ports[0] = %d", dev, portBitmap.ports[0]);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
               "Cheetah device: %d, portBitmap.ports[0] = %d", dev, portBitmap.ports[0]);
        }

        /* 1.6. Check portBitmapPtr with 29 bits used. Call with vidx
          [10] and portBitmapPtr->ports[{ 1<<28, 0}].Expected: non GT_OK.*/

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
        if(UTF_GET_MAX_PHY_PORTS_NUM_MAC(dev) < 31)
        {
            portBitmap.ports[0] = ((GT_U32)(1 << 28));
        }
        else
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,(UTF_GET_MAX_PHY_PORTS_NUM_MAC(dev)));
        }

        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                         "%d, portBitmap.ports[0] = %d",
                                         dev, portBitmap.ports[0]);
        }
        else
        {
            /* the device supports 64 ports regardless to it's physical ports */
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "%d, portBitmap.ports[0] = %d",
                                         dev, portBitmap.ports[0]);
        }

        /* 1.7. Check portBitmapPtr with bits 1-32 unused and bit 33 used.
           Call with vidx [10] and portBitmapPtr->ports[{ 0, 1}].
           Expected: GT_OK. */

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);

        if(UTF_GET_MAX_PHY_PORTS_NUM_MAC(dev) < 31)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,32);
        }
        else
        {
            portBitmap = PRV_CPSS_PP_MAC(dev)->existingPorts;
        }

        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "%d, portBitmap.ports[1] = %d",
                                     dev, portBitmap.ports[1]);
    }

    vidx = 10;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
    portBitmap.ports[0] = 0x0000AAAA;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcEntryRead
(
    IN  GT_U8                devNum,
    IN  GT_U16               vidx,
    OUT CPSS_PORTS_BMP_STC   *portBitmapPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcEntryRead)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with vidx [10] and non-null portBitmapPtr.
    Expected: GT_OK.
    1.2. Call function with vidx [0xFFF] and non-null portBitmapPtr.
    Expected: GT_OK.
    1.3. Call function with out of range vidx [0x1000] and non-null portBitmapPtr.
    Expected: NOT GT_OK.
    1.4. Call function with vidx [10] and null portBitmapPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                dev;
    GT_U16               vidx;
    CPSS_PORTS_BMP_STC   portBitmap;
    GT_U32               vidxNum;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.0 Get number of VIDX groups in a device */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_VIDX_E, &vidxNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. Call function with vidx [10] and non-null portBitmapPtr.
           Expected: GT_OK. */
        vidx = 10;

        st = cpssDxChBrgMcEntryRead(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        /* 1.2. Call function with last valid vidx and non-null portBitmapPtr.
           Expected: GT_OK. */
        vidx = (GT_U16)(vidxNum - 1);

        st = cpssDxChBrgMcEntryRead(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        /* 1.3. Call function with first invalid vidx and non-null portBitmapPtr.
           Expected: NON GT_OK. */
        vidx = (GT_U16)vidxNum;
        /* check and skip valid flood VIDX case */
        if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev))
        {
            vidx++;
        }

        if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            vidx = (GT_U16)PRV_CPSS_DXCH_PP_MAC(dev)->accessTableInfoPtr[CPSS_DXCH_TABLE_MULTICAST_E].maxNumOfEntries;
        }

        st = cpssDxChBrgMcEntryRead(dev, vidx, &portBitmap);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        /* 1.4. Call function with flood vidx and non-null portBitmapPtr.
           Expected: GT_OK. */
        vidx = TGF_BRG_FLOOD_VIDX_MAC(dev);
        st = cpssDxChBrgMcEntryRead(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        vidx = 10; /* restore */

        /* 1.5. Call function with vidx [10] and null portBitmapPtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChBrgMcEntryRead(dev, vidx, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, vidx);
    }

    vidx = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcEntryRead(dev, vidx, &portBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcEntryRead(dev, vidx, &portBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcGroupDelete
(
    IN GT_U8    devNum,
    IN GT_U16   vidx
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcGroupDelete)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vidx [10].
    Expected: GT_OK.
    1.2. Call with vidx [max - 1].
    Expected: GT_OK.
    1.3. Call with out of range vidx [max].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U16      vidx;
    GT_U32               vidxNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.0 Get number of VIDX groups in a device */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_VIDX_E, &vidxNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with vidx [10].
            Expected: GT_OK.
        */
        vidx = 10;

        st = cpssDxChBrgMcGroupDelete(dev, vidx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        /*
            1.2. Call with vidx [max - 1] but skip flood VIDX.
            Expected: GT_OK.
        */
        vidx = (GT_U16)(vidxNum - 1);
        if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev))
        {
            vidx--;
        }

        st = cpssDxChBrgMcGroupDelete(dev, vidx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);


        /*
            1.3. Call with out of range vidx [max].
            Expected: NOT GT_OK.
        */
        vidx = (GT_U16)vidxNum;

        st = cpssDxChBrgMcGroupDelete(dev, vidx);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vidx);

        vidx = (GT_U16)(vidxNum - 1);
        if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev))
        {
            vidx--;
        }
    }

    vidx = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcGroupDelete(dev, vidx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcGroupDelete(dev, vidx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcMemberAdd
(
    IN GT_U8   devNum,
    IN GT_U16  vidx,
    IN GT_U8   portNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcMemberAdd)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vidx [10] and portNum [16].
    Expected: GT_OK.
    1.2. Call with vidx [10] and portNum [26].
    Expected: GT_OK.
    1.3. Call with vidx [10] and portNum [27].
    Expected: GT_OK for DxCh2 and NON GT_OK for others.
    1.4. Call with vidx [10] and out of range portNum [28].
    Expected: NON GT_OK.
    1.5. Call with vidx [max - 1] and portNum [16].
    Expected: GT_OK.
    1.6. Call with out of range vidx [max] and portNum[16].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U16      vidx;
    GT_U8       portNum;
    GT_U32      vidxNum;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.0 Get number of VIDX groups in a device */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_VIDX_E, &vidxNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with vidx [max - 1] but skip flood VIDX and portNum [16].
            Expected: GT_OK.
        */
        vidx = (GT_U16)(vidxNum - 1);
        if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev))
        {
            vidx--;
        }

        portNum = 12;

        st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);

        /*
            1.2. Call with vidx [max - 1] and portNum [26].
            Expected: GT_OK.
        */
        portNum = 26;

        st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) || PRV_CPSS_PHY_PORT_IS_EXIST_MAC(dev,portNum))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vidx, portNum);
        }

        /*
            1.3. Call with vidx [max - 1] and portNum [27].
            Expected: GT_OK for DxCh2 and NON GT_OK for others.
        */
        portNum = 27;

        st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);

        if(CPSS_PP_FAMILY_CHEETAH_E != devFamily)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }

        /*
            1.4. Call with vidx [max - 1] and out of range portNum [28].
            Expected: NON GT_OK.
        */
        portNum = 28;

        st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }
        else
        {
            /* the device supports 64 ports regardless to it's physical ports */
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }

        /*
            1.5. Call with vidx [max - 1] and portNum [16].
            Expected: GT_OK.
        */
        vidx = (GT_U16)(vidxNum - 1);
        if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev))
        {
            vidx--;
        }
        portNum = 12;

        st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);

        /*
            1.6. Call with out of range vidx [max] and portNum[16].
            Expected: NOT GT_OK.
        */
        vidx = (GT_U16)vidxNum;

        st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
    }

    /* set correct values */
    vidx = 10;
    portNum = 12;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcMemberAdd(dev, vidx, portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcMemberDelete
(
    IN GT_U8          devNum,
    IN GT_U16         vidx,
    IN GT_U8          portNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcMemberDelete)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vidx [max - 1] and portNum [16].
    Expected: GT_OK.
    1.2. Call with vidx [max - 1] and portNum [26].
    Expected: GT_OK.
    1.3. Call with vidx [max - 1] and portNum [27].
    Expected: GT_OK for DxCh2 and NON GT_OK for others.
    1.4. Call with vidx [max - 1] and out of range portNum [28].
    Expected: NON GT_OK.
    1.5. Call with vidx [max - 1] and portNum [16].
    Expected: GT_OK.
    1.6. Call with out of range vidx [max] and portNum[16].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_U16      vidx;
    GT_U8       portNum;
    GT_U32      vidxNum;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.0 Get number of VIDX groups in a device */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_VIDX_E, &vidxNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with vidx [max - 1] and portNum [16].
            Expected: GT_OK.
        */
        vidx = (GT_U16)(vidxNum - 1);
        if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev))
        {
            vidx--;
        }

        portNum = 12;

        st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);

        /*
            1.2. Call with vidx [max - 1] and portNum [26].
            Expected: GT_OK.
        */
        portNum = 26;

        st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) || PRV_CPSS_PHY_PORT_IS_EXIST_MAC(dev,portNum))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vidx, portNum);
        }

        /*
            1.3. Call with vidx [max - 1] and portNum [27].
            Expected: GT_OK for DxCh2 and NON GT_OK for others.
        */
        portNum = 27;

        st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);

        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }

        /*
            1.4. Call with vidx [max - 1] and out of range portNum [28].
            Expected: NON GT_OK.
        */
        portNum = 28;

        st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }
        else
        {
            /* the device supports 64 ports regardless to it's physical ports */
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
        }

        /*
            1.5. Call with vidx [max - 1] and portNum [16].
            Expected: GT_OK.
        */
        vidx = (GT_U16)(vidxNum - 1);
        if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev))
        {
            vidx--;
        }
        portNum = 12;

        st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);

        /*
            1.6. Call with out of range vidx [max] and portNum[16].
            Expected: NOT GT_OK.
        */
        vidx =(GT_U16)vidxNum;

        st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vidx, portNum);
    }

    /* set correct values */
    vidx = 10;
    portNum = 12;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcMemberDelete(dev, vidx, portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgMcInit
(
    IN GT_U8    devNum,
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcInit)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with corect devNum.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with corect devNum.
            Expected: GT_OK.
        */
        st = cpssDxChBrgMcInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgMcInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgMcInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill Multicast Group table.
*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcFillMulticastGroupTable)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_VIDX_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in Multicast Group table.
         Call cpssDxChBrgMcEntryWrite with vidx [0 ... numEntries-1]
                                           and portBitmapPtr->ports[{0x0000AAAA}].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChBrgMcEntryWrite with vidx [numEntries]
                                           and portBitmapPtr->ports[{0x0000AAAA}].
    Expected: NOT GT_OK.
    1.4. Read all entries in Multicast Group table and compare with original.
         Call cpssDxChBrgMcEntryRead with the same vidx, non-NULL portBitmapPtr.
    Expected: GT_OK and the same portBitmapPtr.
    1.5. Try to read entry with index out of range.
         Call cpssDxChBrgMcEntryRead with vidx [numEntries], non-NULL portBitmapPtr.
    Expected: NOT GT_OK.
    1.6. Delete all entries in Multicast Group table.
         Call cpssDxChBrgMcGroupDelete the same vidx as in 1.2.
    Expected: GT_OK.
    1.7. Try to delete entry with index out of range.
         Call cpssDxChBrgMcGroupDelete the same vidx [numEntries].
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32               numEntries = 0;
    GT_U16               vidx       = 0;
    GT_U16               step = 1;
    GT_U32   gmFillMaxIterations = 128;
    CPSS_PORTS_BMP_STC   portBitmap;
    CPSS_PORTS_BMP_STC   retPortBitmap;
    GT_BOOL              failureWas = GT_FALSE;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* Fill the entry for Multicast Group table */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&retPortBitmap);

    portBitmap.ports[0] = 0x0000AAAA;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(dev,portBitmap);

        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_VIDX_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        /* xCat3 use MC table for 802.1br BPE support and has 12K entries */
        if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            numEntries = _12K;
        }

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = (GT_U16)prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in Multicast Group table */
        for(vidx = 0; vidx < numEntries; vidx+=step)
        {
            st = cpssDxChBrgMcEntryWrite(dev, vidx, &portBitmap);
            /* skip flood VIDX */
            if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev) &&
                (GT_FALSE == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)))
            {
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgMcEntryWrite: %d, %d", dev, vidx);
            }
            else
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgMcEntryWrite: %d, %d", dev, vidx);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChBrgMcEntryWrite(dev, (GT_U16)numEntries, &portBitmap);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChBrgMcEntryWrite: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in Multicast Group table and compare with original */
        for(vidx = 0; vidx < numEntries; vidx+=step)
        {
            st = cpssDxChBrgMcEntryRead(dev, vidx, &retPortBitmap);
            /* take care of flood VIDX */
            if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev) &&
                (GT_FALSE == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgMcEntryRead: %d, %d", dev, vidx);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgMcEntryRead: %d, %d", dev, vidx);

                failureWas = (CPSS_PORTS_BMP_ARE_EQUAL_MAC(&portBitmap,&retPortBitmap))
                                              ? GT_FALSE : GT_TRUE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, failureWas,
                    "get another portBitmap than was set: %d, %d", dev, vidx);

            }
        }

        /* 1.5. Try to read entry with index out of range. */
        /* handle flood VIDX */
        st = cpssDxChBrgMcEntryRead(dev, (GT_U16)(numEntries + 1), &retPortBitmap);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgMcEntryRead: %d, %d", dev, numEntries);

        /* 1.6. Delete all entries in Multicast Group table */
        for(vidx = 0; vidx < numEntries; vidx+=step)
        {
            st = cpssDxChBrgMcGroupDelete(dev, vidx);
            /* skip flood VIDX */
            if (vidx == TGF_BRG_FLOOD_VIDX_MAC(dev) &&
                (GT_FALSE == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)))
            {
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgMcGroupDelete: %d, %d", dev, vidx);
            }
            else
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgMcGroupDelete: %d, %d", dev, vidx);
        }

        /* 1.7. Try to delete entry with index out of range. */
        st = cpssDxChBrgMcGroupDelete(dev, (GT_U16)numEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChBrgMcGroupDelete: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcPhysicalSourceFilteringEnableSet)
{
    /*
        ITERATE_DEVICES_VIRT_PORTS
        1.1.1. Call with relevant value
               enable[GT_FALSE/GT_TRUE]
        Expected: GT_OK.
        1.1.2. Call cpssDxChBrgMcPhysicalSourceFilteringEnableGet.
        Expected: GT_OK and the same enable.
    */
        GT_STATUS           st              = GT_OK;
        GT_U8               devNum          = 0;
        GT_PORT_NUM         portNum         = 0;

        GT_BOOL             enable = GT_FALSE;
        GT_BOOL             enableGet = GT_FALSE;

        GT_U32              notAppFamilyBmp = 0;

        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            st = prvUtfNextVirtPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
            {
                /*  1.1.1.  */
                enable = GT_FALSE;

                st = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum,
                                                                  enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                                  &enableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgMcPhysicalSourceFilteringEnableGet: %d", devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d", devNum);

                /*  1.1.1.  */
                enable = GT_TRUE;

                st = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum,
                                                                  enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                                  &enableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgMcPhysicalSourceFilteringEnableGet: %d", devNum);
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d", devNum);

            }
            portNum = 0;

            st = prvUtfNextVirtPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum,
                                                                  enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

            st = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum,
                                                              enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                             */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum,
                                                              enable);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }
        }

        /* 2. For not active devices check that function returns non GT_OK. */
        portNum = 0;

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* Go over all non active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum,
                                                              enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum,
                                                          enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgMcPhysicalSourceFilteringEnableGet)
{
    /*
        ITERATE_DEVICES_VIRT_PORTS
        1.1.1. Call with not NULL enablePtr.
        Expected: GT_OK.
        1.1.2. Call with NULL enablePtr.
        Expected: GT_BAD_PTR.
    */
        GT_STATUS           st              = GT_OK;
        GT_U8               devNum          = 0;
        GT_PORT_NUM         portNum         = 0;

        GT_BOOL             enable = GT_FALSE;


        GT_U32              notAppFamilyBmp = 0;

        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            st = prvUtfNextVirtPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
            {
                /*  1.1.1.  */
                st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                                  &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*  1.1.2.  */
                st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                                  NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
            }
            portNum = 0;

            st = prvUtfNextVirtPortReset(&portNum, devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                                  &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

            st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                              &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                             */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                              &enable);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }
        }

        /* 2. For not active devices check that function returns non GT_OK. */
        portNum = 0;

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

        /* Go over all non active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
        {
            st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                              &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }

        /* 3. Call function with out of bound value for device id */
        devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum,
                                                          &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgMc suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgMc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcIpv6BytesSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcIpv6BytesSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcGroupDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcMemberAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcMemberDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcPhysicalSourceFilteringEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcPhysicalSourceFilteringEnableGet)
    /* Test for filling VIDX Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgMcFillMulticastGroupTable)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgMc)

