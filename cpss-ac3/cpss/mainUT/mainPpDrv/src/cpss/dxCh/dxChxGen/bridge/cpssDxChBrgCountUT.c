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
* @file cpssDxChBrgCountUT.c
*
* @brief Unit tests for cpssDxChBrgCountUT, that provides
* Ingress Bridge Counters facility implementation.
*
* @version   17
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgCount.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntDropCntrModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT dropMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntDropCntrModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Check unsupported dropMode.
    Call with dropMode [CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E /
                        CPSS_DXCH_BRG_DROP_CNTR_SYN_WITH_DATA_E /
                        CPSS_DXCH_BRG_DROP_CNTR_VLAN_MRU_E]
                        (these modes are not possible).
    Expected: NOT GT_OK for DxCh and GT_OK for DxCh2.
    1.2. Check supported dropMode.
    Call with dropMode [CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E /
                        CPSS_DXCH_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E /
                        CPSS_DXCH_BRG_DROP_CNTR_RATE_LIMIT_E /
                        CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E].
    Expected: GT_OK.
    1.3. Call cpssDxChBrgCntDropCntrModeGet with non-NULL dropModePtr.
    Expected: GT_OK and same value as written.
    1.4. Call with out of range dropMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8                   dev;
    CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT dropMode;
    CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT dropModeRet;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
           1.1. Check unsupported dropMode. Call with dropMode
                       [CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E /
                        CPSS_DXCH_BRG_DROP_CNTR_SYN_WITH_DATA_E /
                        CPSS_DXCH_BRG_DROP_CNTR_VLAN_MRU_E]
                       (these modes are not possible).
           Expected: NOT GT_OK for DxCh and GT_OK for DxCh2.
        */

        dropMode = CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E;

        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        if (CPSS_PP_FAMILY_CHEETAH_E == devFamily)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                         "Cheetah device: %d, %d", dev, dropMode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "Cheetah2 device: %d, %d", dev, dropMode);
        }


        dropMode = CPSS_DXCH_BRG_DROP_CNTR_SYN_WITH_DATA_E;

        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        if (CPSS_PP_FAMILY_CHEETAH_E == devFamily)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                         "Cheetah device: %d, %d", dev, dropMode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "Cheetah2 device: %d, %d", dev, dropMode);
        }

        dropMode = CPSS_DXCH_BRG_DROP_CNTR_VLAN_MRU_E;

        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        if (CPSS_PP_FAMILY_CHEETAH_E == devFamily)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                         "Cheetah device: %d, %d", dev, dropMode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "Cheetah2 device: %d, %d", dev, dropMode);
        }

        /*
           1.2. Check supported dropMode. Call with dropMode
                      [CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E /
                      CPSS_DXCH_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E /
                      CPSS_DXCH_BRG_DROP_CNTR_RATE_LIMIT_E /
                      CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E].
          Expected: GT_OK.
        */

        dropMode = CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E;

        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropMode);

        dropMode = CPSS_DXCH_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E;

        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropMode);

        dropMode = CPSS_DXCH_BRG_DROP_CNTR_RATE_LIMIT_E;

        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropMode);

        dropMode = CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E;

        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropMode);

        /*
           1.3. Call cpssDxChBrgCntDropCntrModeGet with non-NULL dropModePtr.
           Expected: GT_OK and same value as written.
        */

        st = cpssDxChBrgCntDropCntrModeGet(dev, &dropModeRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgCntDropCntrModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dropMode, dropModeRet,
         "cpssDxChBrgCntDropCntrModeGet: get another dropMode than was set: dev = %d", dev);

        /*
           1.4. Call with wrong enum values dropMode.
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgCntDropCntrModeSet
                            (dev, dropMode),
                            dropMode);
    }

    dropMode = CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntDropCntrModeSet(dev, dropMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntDropCntrModeGet
(
    IN   GT_U8                                   devNum,
    OUT  CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT     *dropModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntDropCntrModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL dropModePtr.
    Expected: GT_OK.
    1.2. Call function with dropModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT     dropMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL dropModePtr. Expected: GT_OK.*/

        st = cpssDxChBrgCntDropCntrModeGet(dev, &dropMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with dropModePtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChBrgCntDropCntrModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntDropCntrModeGet(dev, &dropMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntDropCntrModeGet(dev, &dropMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntMacDaSaSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *saAddrPtr,
    IN  GT_ETHERADDR    *daAddrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntMacDaSaSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with saAddrPtr [AB:CD:EF:00:00:01],
    daAddrPtr [AB:CD:EF:00:00:02].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgCntMacDaSaGet with non-NULL saAddrPtr  and daAddrPtr.
    Expected: GT_OK and same values as written.
    1.3. Call function with saAddrPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call function with daAddrPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st = GT_OK;
    GT_U8          dev;
    GT_ETHERADDR   saAddr = {{0xAB, 0xCD, 0xEF, 0x00, 0x00, 0x01}};
    GT_ETHERADDR   daAddr = {{0xAB, 0xCD, 0xEF, 0x00, 0x00, 0x02}};
    GT_ETHERADDR   saAddrRet;
    GT_ETHERADDR   daAddrRet;
    GT_BOOL        isEqual;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with saAddrPtr [AB:CD:EF:00:00:01],  daAddrPtr
           [AB:CD:EF:00:00:02]. Expected: GT_OK.    */

        st = cpssDxChBrgCntMacDaSaSet(dev, &saAddr, &daAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call cpssDxChBrgCntMacDaSaGet with non-NULL saAddrPtr
           and daAddrPtr. Expected: GT_OK and same values as written.   */

        cpssOsBzero((GT_VOID*) &saAddrRet, sizeof(saAddrRet));
        cpssOsBzero((GT_VOID*) &daAddrRet, sizeof(daAddrRet));

        st = cpssDxChBrgCntMacDaSaGet(dev, &saAddrRet, &daAddrRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgCntMacDaSaGet: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&saAddr, (GT_VOID*)&saAddrRet, sizeof (saAddr)))
                  ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another saAddr than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&daAddr, (GT_VOID*)&daAddrRet, sizeof (daAddr)))
                  ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another daAddr than was set: %d", dev);

        /* 1.3. Call function with saAddrPtr [NULL], other params same as
           in 1.1. Expected: GT_BAD_PTR.    */

        st = cpssDxChBrgCntMacDaSaSet(dev, NULL, &daAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, saAddr = NULL", dev);

        /* 1.4. Call function with daAddrPtr [NULL], other params same as
           in 1.1. Expected: GT_BAD_PTR.    */

        st = cpssDxChBrgCntMacDaSaSet(dev, &saAddr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, daAddr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntMacDaSaSet(dev, &saAddr, &daAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntMacDaSaSet(dev, &saAddr, &daAddr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntMacDaSaGet
(
    IN   GT_U8           devNum,
    OUT  GT_ETHERADDR    *saAddrPtr,
    OUT  GT_ETHERADDR    *daAddrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntMacDaSaGet)
{
/*
ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL saAddrPtr, daAddrPtr.
    Expected: GT_OK.
    1.2. Call function with saAddrPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call function with daAddrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_ETHERADDR            saAddr;
    GT_ETHERADDR            daAddr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL saAddrPtr,
           daAddrPtr. Expected: GT_OK.  */

        st = cpssDxChBrgCntMacDaSaGet(dev, &saAddr, &daAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with saAddrPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChBrgCntMacDaSaGet(dev, NULL, &daAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, saAddrPtr = NULL", dev);

        /* 1.3. Call function with daAddrPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChBrgCntMacDaSaGet(dev, &saAddr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, daAddrPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntMacDaSaGet(dev, &saAddr, &daAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntMacDaSaGet(dev, &saAddr, &daAddr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntBridgeIngressCntrModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    IN  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode,
    IN  GT_U8                               port,
    IN  GT_U16                              vlan
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntBridgeIngressCntrModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E /
                              CPSS_DXCH_BRG_CNT_SET_ID_1_E],
         setMode [CPSS_BRG_CNT_MODE_3_E], port [0], vlan [100].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgCntBridgeIngressCntrModeGet with
         cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E]
         and non-NULL setModePtr, portPtr, vlanPtr.
    Expected: GT_OK and same values as written.
    1.3. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
         setMode [CPSS_BRG_CNT_MODE_0_E],
         port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (should be ignored),
         vlan [PRV_CPSS_MAX_NUM_VLANS_CNS] (should be ignored).
    Expected: GT_OK.
    1.4. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
         setMode [CPSS_BRG_CNT_MODE_1_E],
         port [0], vlan [PRV_CPSS_MAX_NUM_VLANS_CNS] (should be ignored).
    Expected: GT_OK.
    1.5. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_1_E],
         setMode [CPSS_BRG_CNT_MODE_2_E],
         port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (should be ignored), vlan [100].
    Expected: GT_OK.
    1.6. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
         setMode [CPSS_BRG_CNT_MODE_1_E / CPSS_BRG_CNT_MODE_3_E],
         port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (incorrect), vlan [100].
    Expected: GT_BAD_PARAM.
    1.7. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
         setMode [CPSS_BRG_CNT_MODE_2_E / CPSS_BRG_CNT_MODE_3_E],
         port [0], vlan [PRV_CPSS_MAX_NUM_VLANS_CNS] (incorrect).
    Expected: GT_BAD_PARAM.
    1.8. Call with cntrSetId , other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call with setMode , other params same as in 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId;
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode;
    GT_PORT_NUM                         port;
    GT_U16                              vlan;
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setModeRet;
    GT_PORT_NUM                         portRet;
    GT_U16                              vlanRet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E /
            CPSS_DXCH_BRG_CNT_SET_ID_1_E],  setMode [CPSS_BRG_CNT_MODE_3_E],
            port [0], vlan [100].
            Expected: GT_OK.
        */

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;
        setMode = CPSS_BRG_CNT_MODE_3_E;
        port = 0;
        vlan = 100;

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, cntrSetId, setMode, port, vlan);

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, cntrSetId, setMode, port, vlan);

        /*
           1.2. Call cpssDxChBrgCntBridgeIngressCntrModeGet with
           cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E] and non-NULL setModePtr,
           portPtr, vlanPtr.
           Expected: GT_OK and same values as written.
        */

        st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId,
                                        &setModeRet, &portRet, &vlanRet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChBrgCntBridgeIngressCntrModeGet: %d, %d",
                                     dev, cntrSetId);

        if (GT_OK == st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(setMode, setModeRet,
                "cpssDxChBrgCntBridgeIngressCntrModeGet: get another setMode than was set: dev = %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(port, portRet,
                "cpssDxChBrgCntBridgeIngressCntrModeGet: get another port than was set: dev = %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(vlan, vlanRet,
                "cpssDxChBrgCntBridgeIngressCntrModeGet: get another vlan than was set: dev = %d", dev);
        }

        /*
           1.3. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
           setMode [CPSS_BRG_CNT_MODE_0_E], port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
           (should be ignored), vlan [PRV_CPSS_MAX_NUM_VLANS_CNS] (should be ignored).
           Expected: GT_OK.
        */

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;
        setMode = CPSS_BRG_CNT_MODE_0_E;
        port = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
        vlan = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, cntrSetId,
                                    setMode, port, vlan);

        /*
            1.4. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
            setMode [CPSS_BRG_CNT_MODE_1_E], port [0], vlan
            [PRV_CPSS_MAX_NUM_VLANS_CNS] (should be ignored).
            Expected: GT_OK.
        */

        setMode = CPSS_BRG_CNT_MODE_1_E;
        port = 0;

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, vlan = %d",
                                     dev, cntrSetId, vlan);

        /*
           1.5. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_1_E],
           setMode [CPSS_BRG_CNT_MODE_2_E], port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
           (should be ignored), vlan [100].
           Expected: GT_OK.
        */

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;
        setMode = CPSS_BRG_CNT_MODE_2_E;
        port = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
        vlan = 100;

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, port = %d",
                                     dev, cntrSetId, port);

        /*
           1.6. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
           setMode [CPSS_BRG_CNT_MODE_1_E / CPSS_BRG_CNT_MODE_3_E], port
           [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] (incorrect), vlan [100].
           Expected: non GT_OK.
        */

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;
        setMode = CPSS_BRG_CNT_MODE_1_E;
        port = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
        vlan = 100;

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, port = %d",
                                     dev, cntrSetId, port);

        setMode = CPSS_BRG_CNT_MODE_3_E;

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, port = %d",
                                     dev, cntrSetId, port);

        /*
           1.7. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E],
           setMode [CPSS_BRG_CNT_MODE_2_E / CPSS_BRG_CNT_MODE_3_E], port [0],
           vlan [PRV_CPSS_MAX_NUM_VLANS_CNS] (incorrect).
           Expected: non GT_OK.
        */

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;
        setMode = CPSS_BRG_CNT_MODE_2_E;
        port = 0;
        vlan = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, vlan = %d",
                                     dev, cntrSetId, vlan);

        setMode = CPSS_BRG_CNT_MODE_3_E;

        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, vlan = %d",
                                     dev, cntrSetId, vlan);

        /*
            1.8. Call with cntrSetId, other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        setMode = CPSS_BRG_CNT_MODE_3_E;
        port = 0;
        vlan = 100;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgCntBridgeIngressCntrModeSet
                            (dev, cntrSetId, setMode, port, vlan),
                            cntrSetId);

        /*
            1.9. Call with wrong setMode, other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgCntBridgeIngressCntrModeSet
                            (dev, cntrSetId, setMode, port, vlan),
                            setMode);
    }

    setMode = CPSS_BRG_CNT_MODE_3_E;
    cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;
    port = 0;
    vlan = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                    setMode, port, vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntBridgeIngressCntrModeSet(dev, cntrSetId,
                                                setMode, port, vlan);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntBridgeIngressCntrModeGet
(
    IN   GT_U8                               devNum,
    IN   CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    OUT  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     *setModePtr,
    OUT  GT_U8                               *portPtr,
    OUT  GT_U16                              *vlanPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntBridgeIngressCntrModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E /
                              CPSS_DXCH_BRG_CNT_SET_ID_1_E],
         non-NULL setModePtr, portPtr, vlanPtr.
    Expected: GT_OK.
    1.2. Call with wrong cntrSetId.
    Expected: GT_BAD_PARAM.
    1.3. Call with setModePtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with portPtr [NULL] (can be NULL), other params same as in 1.1.
    Expected: GT_OK.
    1.5. Call with vlanPtr [NULL] (can be NULL), other params same as in 1.1.
    Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId;
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode;
    GT_PORT_NUM                         port;
    GT_U16                              vlan;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E /
                                        CPSS_DXCH_BRG_CNT_SET_ID_1_E],
            non-NULL setModePtr, portPtr, vlanPtr.
            Expected: GT_OK.
        */
        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;

        st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId, &setMode,
                                                    &port, &vlan);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetId);

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;

        st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId, &setMode,
                                                    &port, &vlan);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetId);

        /*
            1.1.2. Call with wrong cntrSetId (invalid enums).
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgCntBridgeIngressCntrModeGet
                            (dev, cntrSetId, &setMode, &port, &vlan),
                            cntrSetId);

        /*
           1.1.3. Call with setModePtr [NULL], other params same as in 1.1.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId, NULL,
                                                    &port, &vlan);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, setModePtr = NULL", dev);

        /*
            1.1.4. Call with portPtr [NULL] (can be NULL), other params same as in 1.1.
            Expected: GT_OK.
        */
        st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId, &setMode,
                                                    NULL, &vlan);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portPtr = NULL", dev);

        /*
            1.1.5. Call with vlanPtr [NULL] (can be NULL), other params same as in 1.1.
            Expected: GT_OK.
        */
        st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId, &setMode,
                                                    &port, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, vlanPtr = NULL", dev);
    }

    cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId, &setMode,
                                                    &port, &vlan);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntBridgeIngressCntrModeGet(dev, cntrSetId, &setMode,
                                                &port, &vlan);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntDropCntrGet
(
    IN   GT_U8      devNum,
    OUT  GT_U32     *dropCntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntDropCntrGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL dropCntPtr.
    Expected: GT_OK.
    1.2. Call function with dropCntPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  dropCnt;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL dropCntPtr. Expected: GT_OK.*/

        st = cpssDxChBrgCntDropCntrGet(dev, &dropCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with dropCntPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChBrgCntDropCntrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropCntPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntDropCntrGet(dev, &dropCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntDropCntrGet(dev, &dropCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntDropCntrSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     dropCnt
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntDropCntrSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with dropCnt [0 / 0xFFFF / 1000].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgCntDropCntrGet with non-NULL dropCntPtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  dropCnt;
    GT_U32                  dropCntRet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call function with dropCnt [0 / 0xFFFF / 1000].
           Expected: GT_OK.
        */
        dropCnt = 0;

        st = cpssDxChBrgCntDropCntrSet(dev, dropCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCnt);

        dropCnt = 0xFFFF;

        st = cpssDxChBrgCntDropCntrSet(dev, dropCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCnt);

        dropCnt = 1000;

        st = cpssDxChBrgCntDropCntrSet(dev, dropCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCnt);

        /*
           1.2. Call cpssDxChBrgCntDropCntrGet with non-NULL dropCntPtr.
           Expected: GT_OK and same value as written.
        */

        st = cpssDxChBrgCntDropCntrGet(dev, &dropCntRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgCntDropCntrGet: %d", dev);

        if (GT_OK == st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(dropCnt, dropCntRet,
             "cpssDxChBrgCntDropCntrGet: get another dropCnt than was set: dev = %d", dev);
        }
    }

    dropCnt = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntDropCntrSet(dev, dropCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntDropCntrSet(dev, dropCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntHostGroupCntrsGet
(
    IN   GT_U8                              devNum,
    OUT  CPSS_DXCH_BRIDGE_HOST_CNTR_STC     *hostGroupCntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntHostGroupCntrsGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL hostGroupCntPtr.
    Expected: GT_OK.
    1.2. Call function with hostGroupCntPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL hostGroupCntPtr.
           Expected: GT_OK. */

        st = cpssDxChBrgCntHostGroupCntrsGet(dev, &hostGroupCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with hostGroupCntPtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChBrgCntHostGroupCntrsGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, hostGroupCntPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntHostGroupCntrsGet(dev, &hostGroupCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntHostGroupCntrsGet(dev, &hostGroupCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntMatrixGroupCntrsGet
(
    IN   GT_U8      devNum,
    OUT  GT_U32     *matrixCntSaDaPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntMatrixGroupCntrsGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL matrixCntSaDaPktsPtr.
    Expected: GT_OK.
    1.2. Call function with matrixCntSaDaPktsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  matrixCntSaDaPkts;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL matrixCntSaDaPktsPtr. Expected: GT_OK. */

        st = cpssDxChBrgCntMatrixGroupCntrsGet(dev, &matrixCntSaDaPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with matrixCntSaDaPktsPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChBrgCntMatrixGroupCntrsGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, matrixCntSaDaPktsPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntMatrixGroupCntrsGet(dev, &matrixCntSaDaPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntMatrixGroupCntrsGet(dev, &matrixCntSaDaPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntBridgeIngressCntrsGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    OUT CPSS_BRIDGE_INGRESS_CNTR_STC        *ingressCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntBridgeIngressCntrsGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E /
                                       CPSS_DXCH_BRG_CNT_SET_ID_1_E],
         non-NULL ingressCntrPtr.
    Expected: GT_OK.
    1.2. Call function with wrong cntrSetId, non-NULL ingressCntrPtr.
    Expected: GT_BAD_PARAM.
    1.3. Call function with ingressCntrPtr [NULL],
    other param same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId;
    CPSS_BRIDGE_INGRESS_CNTR_STC        ingressCntr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with cntrSetId [CPSS_DXCH_BRG_CNT_SET_ID_0_E
                                             / CPSS_DXCH_BRG_CNT_SET_ID_1_E],
                 non-NULL ingressCntrPtr.
            Expected: GT_OK.
        */
        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;

        st = cpssDxChBrgCntBridgeIngressCntrsGet(dev, cntrSetId, &ingressCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetId);

        cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;

        st = cpssDxChBrgCntBridgeIngressCntrsGet(dev, cntrSetId, &ingressCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntrSetId);

        /*
           1.2. Call function with wrong cntrSetId, non-NULL ingressCntrPtr.
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgCntBridgeIngressCntrsGet
                            (dev, cntrSetId, &ingressCntr),
                            cntrSetId);

        /*
            1.3. Call function with ingressCntrPtr [NULL], other param same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgCntBridgeIngressCntrsGet(dev, cntrSetId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ingressCntrPtr = NULL", dev);
    }

    cntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntBridgeIngressCntrsGet(dev, cntrSetId, &ingressCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntBridgeIngressCntrsGet(dev, cntrSetId, &ingressCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntLearnedEntryDiscGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *countValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntLearnedEntryDiscGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non-NULL countValuePtr.
    Expected: GT_OK.
    1.2. Call function with countValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  countValue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL countValuePtr. Expected: GT_OK.*/

        st = cpssDxChBrgCntLearnedEntryDiscGet(dev, &countValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with countValuePtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChBrgCntLearnedEntryDiscGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, countValuePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntLearnedEntryDiscGet(dev, &countValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntLearnedEntryDiscGet(dev, &countValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntPortGroupDropCntrSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN  GT_U32               dropCnt
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntPortGroupDropCntrSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1.1 Call function with dropCnt [0 / 0x1000 / 0xFFFF ].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgCntPortGroupDropCntrGet with non-NULL dropCntPtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS            st = GT_OK;
    GT_U8                dev;
    GT_PORT_GROUPS_BMP   portGroupsBmp = 1;
    GT_U32               dropCnt = 0;
    GT_U32               dropCntRet;
    GT_U32               portGroupId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1 Call function with dropCnt [0 / 0x1000 / 0xFFFF ].
                Expected: GT_OK.
            */

            /* call with dropCnt = 0 */
            dropCnt = 0;

            st = cpssDxChBrgCntPortGroupDropCntrSet(dev, portGroupsBmp, dropCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCnt);

            /*
                1.1.2. Call cpssDxChBrgCntPortGroupDropCntrGet with non-NULL dropCntPtr.
                Expected: GT_OK and same value as written.
            */

            st = cpssDxChBrgCntPortGroupDropCntrGet(dev, portGroupsBmp, &dropCntRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgCntPortGroupDropCntrGet: %d", dev);

            if (GT_OK == st)
            {
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(dropCnt, dropCntRet,
                 "cpssDxChBrgCntPortGroupDropCntrGet: get another dropCnt than was set: dev = %d", dev);
            }


            /* call with dropCnt = 1000 */
            dropCnt = 1000;

            st = cpssDxChBrgCntPortGroupDropCntrSet(dev, portGroupsBmp, dropCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCnt);

            /*
                1.1.2. Call cpssDxChBrgCntPortGroupDropCntrGet with non-NULL dropCntPtr.
                Expected: GT_OK and same value as written.
            */

            st = cpssDxChBrgCntPortGroupDropCntrGet(dev, portGroupsBmp, &dropCntRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgCntPortGroupDropCntrGet: %d", dev);
            if (GT_OK == st)
            {
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(dropCnt, dropCntRet,
                 "cpssDxChBrgCntPortGroupDropCntrGet: get another dropCnt than was set: dev = %d", dev);
            }


            /* call with dropCnt = 0xFFFF */
            dropCnt = 0xFFFF;

            st = cpssDxChBrgCntPortGroupDropCntrSet(dev, portGroupsBmp, dropCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCnt);

            /*
                1.1.2. Call cpssDxChBrgCntPortGroupDropCntrGet with non-NULL dropCntPtr.
                Expected: GT_OK and same value as written.
            */

            st = cpssDxChBrgCntPortGroupDropCntrGet(dev, portGroupsBmp, &dropCntRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgCntPortGroupDropCntrGet: %d", dev);
            if (GT_OK == st)
            {
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(dropCnt, dropCntRet,
                 "cpssDxChBrgCntPortGroupDropCntrGet: get another dropCnt than was set: dev = %d", dev);
            }

            dropCnt = 10;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChBrgCntPortGroupDropCntrSet(dev, portGroupsBmp, dropCnt);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChBrgCntPortGroupDropCntrSet(dev, portGroupsBmp, dropCnt);

        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;
    dropCnt = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgCntPortGroupDropCntrSet(dev, portGroupsBmp, dropCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgCntPortGroupDropCntrSet(dev, portGroupsBmp, dropCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntPortGroupDropCntrGet
(
    IN   GT_U8               devNum,
    IN   GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT  GT_U32              *dropCntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntPortGroupDropCntrGet)
{
    GT_U32 regAddr;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_LION2_E, "JIRA-6756");

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet())->bridgeRegs.dropIngrCntr;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x02040150;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 16,
                                 cpssDxChBrgCntDropCntrGet,
                                 cpssDxChBrgCntDropCntrSet,
                                 cpssDxChBrgCntPortGroupDropCntrGet,
                                 cpssDxChBrgCntPortGroupDropCntrSet);
}

#ifdef ASIC_SIMULATION
#ifndef FORCE_GM_USE

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntPortGroupHostGroupCntrsGet
(
    IN   GT_U8                              devNum,
    IN   GT_PORT_GROUPS_BMP                 portGroupsBmp,
    OUT  CPSS_DXCH_BRIDGE_HOST_CNTR_STC     *hostGroupCntPtr
)
*/

static GT_STATUS prvUtfCpssDxChBrgCntHostInPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    OUT  GT_U32                   *hostInPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntHostGroupCntrsGet(devNum, &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostInPktsPtr = hostGroupCnt.gtHostInPkts;
    return GT_OK;

}
static GT_STATUS prvUtfCpssDxChBrgCntHostOutPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    OUT  GT_U32                   *hostOutPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntHostGroupCntrsGet(devNum, &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostOutPktsPtr = hostGroupCnt.gtHostOutPkts;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntHostOutBroadcastPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    OUT  GT_U32                   *hostOutBroadcastPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntHostGroupCntrsGet(devNum, &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostOutBroadcastPktsPtr = hostGroupCnt.gtHostOutBroadcastPkts;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntHostOutMulticastPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    OUT  GT_U32                   *hostOutMulticastPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntHostGroupCntrsGet(devNum, &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostOutMulticastPktsPtr = hostGroupCnt.gtHostOutMulticastPkts;
    return GT_OK;

}


static GT_STATUS prvUtfCpssDxChBrgCntPortGroupHostInPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    IN   GT_PORT_GROUPS_BMP       portGroupsBmp,
    OUT  GT_U32                   *hostInPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntPortGroupHostGroupCntrsGet(devNum,portGroupsBmp,
                                                  &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostInPktsPtr = hostGroupCnt.gtHostInPkts;
    return GT_OK;

}
static GT_STATUS prvUtfCpssDxChBrgCntPortGroupHostOutPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    IN   GT_PORT_GROUPS_BMP       portGroupsBmp,
    OUT  GT_U32                   *hostOutPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntPortGroupHostGroupCntrsGet(devNum,portGroupsBmp,
                                                  &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostOutPktsPtr = hostGroupCnt.gtHostOutPkts;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupHostOutBroadcastPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    IN   GT_PORT_GROUPS_BMP       portGroupsBmp,
    OUT  GT_U32                   *hostOutBroadcastPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntPortGroupHostGroupCntrsGet(devNum,portGroupsBmp,
                                                  &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostOutBroadcastPktsPtr = hostGroupCnt.gtHostOutBroadcastPkts;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupHostOutMulticastPktsGroupCntrsGet
(
    IN   GT_U8                    devNum,
    IN   GT_PORT_GROUPS_BMP       portGroupsBmp,
    OUT  GT_U32                   *hostOutMulticastPktsPtr
)
{
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC     hostGroupCnt; /* host group counter */
    GT_STATUS                          st;           /* return status */

    st = cpssDxChBrgCntPortGroupHostGroupCntrsGet(devNum,portGroupsBmp,
                                                  &hostGroupCnt);
    if(st != GT_OK)
    {
        return st;
    }

    *hostOutMulticastPktsPtr = hostGroupCnt.gtHostOutMulticastPkts;
    return GT_OK;

}

UTF_TEST_CASE_MAC(cpssDxChBrgCntPortGroupHostGroupCntrsGet)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC    *addrPtr;
    GT_U32 regAddr;

    addrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet());

    regAddr = addrPtr->bridgeRegs.hostInPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400BC;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                  prvUtfCpssDxChBrgCntHostInPktsGroupCntrsGet,
                  NULL,
                  prvUtfCpssDxChBrgCntPortGroupHostInPktsGroupCntrsGet,
                  NULL);

    regAddr = addrPtr->bridgeRegs.hostOutPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400C0;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                 prvUtfCpssDxChBrgCntHostOutPktsGroupCntrsGet,
                 NULL,
                 prvUtfCpssDxChBrgCntPortGroupHostOutPktsGroupCntrsGet,
                 NULL);

    regAddr = addrPtr->bridgeRegs.hostOutMcPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400CC;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                 prvUtfCpssDxChBrgCntHostOutMulticastPktsGroupCntrsGet,
                 NULL,
                 prvUtfCpssDxChBrgCntPortGroupHostOutMulticastPktsGroupCntrsGet,
                 NULL);

    regAddr = addrPtr->bridgeRegs.hostOutBrdPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400D0;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                 prvUtfCpssDxChBrgCntHostOutBroadcastPktsGroupCntrsGet,
                 NULL,
                 prvUtfCpssDxChBrgCntPortGroupHostOutBroadcastPktsGroupCntrsGet,
                 NULL);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntPortGroupMatrixGroupCntrsGet
(
    IN   GT_U8                     devNum,
    IN   GT_PORT_GROUPS_BMP        portGroupsBmp,
    OUT  GT_U32                    *matrixCntSaDaPktsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntPortGroupMatrixGroupCntrsGet)
{
    GT_U32 regAddr;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_LION2_E, "JIRA-6759");

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet())->bridgeRegs.matrixPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400D4;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                                 cpssDxChBrgCntMatrixGroupCntrsGet,
                                 NULL,
                                 cpssDxChBrgCntPortGroupMatrixGroupCntrsGet,
                                 NULL);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntPortGroupBridgeIngressCntrsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    OUT CPSS_BRIDGE_INGRESS_CNTR_STC        *ingressCntrPtr
)
*/

static GT_STATUS prvUtfCpssDxChBrgCntBrgInFramesCntrSetId0CntrsGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32    *gtBrgInFramesCntrSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgInFramesCntrSetId0 = ingressCntr.gtBrgInFrames;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntBrgInFramesCntrSetId1CntrsGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *gtBrgInFramesCntrSetId1
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgInFramesCntrSetId1 = ingressCntr.gtBrgInFrames;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgInFramesCntrSetId0CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgInFramesCntrSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgInFramesCntrSetId0 = ingressCntr.gtBrgInFrames;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgInFramesCntrSetId1CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgInFramesCntrSetId1
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgInFramesCntrSetId1 = ingressCntr.gtBrgInFrames;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntBrgVlanIngFilterDiscSetId0CntrsGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32    *gtBrgVlanIngFilterDiscSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr;   /* bridge ingress counter */
    GT_STATUS                          st;            /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgVlanIngFilterDiscSetId0 = ingressCntr.gtBrgVlanIngFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntBrgVlanIngFilterDiscSetId1CntrsGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32    *gtBrgVlanIngFilterDiscSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr;   /* bridge ingress counter */
    GT_STATUS                          st;            /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgVlanIngFilterDiscSetId0 = ingressCntr.gtBrgVlanIngFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgVlanIngFilterDiscSetId0CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgVlanIngFilterDiscSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgVlanIngFilterDiscSetId0 = ingressCntr.gtBrgVlanIngFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgVlanIngFilterDiscSetId1CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgVlanIngFilterDiscSetId1
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgVlanIngFilterDiscSetId1 = ingressCntr.gtBrgVlanIngFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntBrgSecFilterDiscSetId0CntrsGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32    *gtBrgSecFilterDiscSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr;   /* bridge ingress counter */
    GT_STATUS                          st;            /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgSecFilterDiscSetId0 = ingressCntr.gtBrgSecFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntBrgSecFilterDiscSetId1CntrsGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32    *gtBrgSecFilterDiscSetId1
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr;   /* bridge ingress counter */
    GT_STATUS                          st;            /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgSecFilterDiscSetId1 = ingressCntr.gtBrgSecFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgSecFilterDiscSetId0CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgSecFilterDiscSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgSecFilterDiscSetId0 = ingressCntr.gtBrgSecFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgSecFilterDiscSetId1CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgSecFilterDiscSetId1
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgSecFilterDiscSetId1 = ingressCntr.gtBrgSecFilterDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntBrgLocalPropDiscSetId0CntrsGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32    *gtBrgLocalPropDiscSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr;   /* bridge ingress counter */
    GT_STATUS                          st;            /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgLocalPropDiscSetId0 = ingressCntr.gtBrgLocalPropDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntBrgLocalPropDiscSetId1CntrsGet
(
    IN   GT_U8     devNum,
    OUT  GT_U32    *gtBrgLocalPropDiscSetId1
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr;   /* bridge ingress counter */
    GT_STATUS                          st;            /* return status */

    st = cpssDxChBrgCntBridgeIngressCntrsGet(devNum,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgLocalPropDiscSetId1 = ingressCntr.gtBrgLocalPropDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgLocalPropDiscSetId0CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgLocalPropDiscSetId0
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_0_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgLocalPropDiscSetId0 = ingressCntr.gtBrgLocalPropDisc;
    return GT_OK;

}

static GT_STATUS prvUtfCpssDxChBrgCntPortGroupBrgLocalPropDiscSetId1CntrsGet
(
    IN   GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT  GT_U32                 *gtBrgLocalPropDiscSetId1
)
{
    CPSS_BRIDGE_INGRESS_CNTR_STC       ingressCntr; /* bridge ingress counter */
    GT_STATUS                          st;          /* return status */

    st = cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, portGroupsBmp,
                                             CPSS_DXCH_BRG_CNT_SET_ID_1_E,
                                             &ingressCntr);
    if(st != GT_OK)
    {
        return st;
    }

    *gtBrgLocalPropDiscSetId1 = ingressCntr.gtBrgLocalPropDisc;
    return GT_OK;

}

UTF_TEST_CASE_MAC(cpssDxChBrgCntPortGroupBridgeIngressCntrsGet)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC    *addrPtr;
    GT_U32 regAddr;

    addrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet());

    regAddr = addrPtr->bridgeRegs.brgCntrSet[0].inPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400E0;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgInFramesCntrSetId0CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgInFramesCntrSetId0CntrsGet,
          NULL);

    regAddr = addrPtr->bridgeRegs.brgCntrSet[1].inPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400F4;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgInFramesCntrSetId1CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgInFramesCntrSetId1CntrsGet,
          NULL);

    regAddr = addrPtr->bridgeRegs.brgCntrSet[0].inFltPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400E4;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgVlanIngFilterDiscSetId0CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgVlanIngFilterDiscSetId0CntrsGet,
          NULL);

    regAddr = addrPtr->bridgeRegs.brgCntrSet[1].inFltPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400F8;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgVlanIngFilterDiscSetId1CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgVlanIngFilterDiscSetId1CntrsGet,
          NULL);

    regAddr = addrPtr->bridgeRegs.brgCntrSet[0].secFltPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400E8;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgSecFilterDiscSetId0CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgSecFilterDiscSetId0CntrsGet,
          NULL);

    regAddr = addrPtr->bridgeRegs.brgCntrSet[1].secFltPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400FC;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgSecFilterDiscSetId1CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgSecFilterDiscSetId1CntrsGet,
          NULL);

    regAddr = addrPtr->bridgeRegs.brgCntrSet[0].brgFltPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x020400EC;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgLocalPropDiscSetId0CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgLocalPropDiscSetId0CntrsGet,
          NULL);

    regAddr = addrPtr->bridgeRegs.brgCntrSet[1].brgFltPckt;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x02040100;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
          prvUtfCpssDxChBrgCntBrgLocalPropDiscSetId1CntrsGet,
          NULL,
          prvUtfCpssDxChBrgCntPortGroupBrgLocalPropDiscSetId1CntrsGet,
          NULL);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgCntPortGroupLearnedEntryDiscGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *countValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgCntPortGroupLearnedEntryDiscGet)
{
    GT_U32 regAddr;


    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_LION2_E, "JIRA-6758");

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet())->bridgeRegs.learnedDisc;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x06000030;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                                 cpssDxChBrgCntLearnedEntryDiscGet,
                                 NULL,
                                 cpssDxChBrgCntPortGroupLearnedEntryDiscGet,
                                 NULL);
}
#endif /* FORCE_GM_USE  */
#endif /* ASIC_SIMULATION */

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgCount suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgCount)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntDropCntrModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntDropCntrModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntMacDaSaSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntMacDaSaGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntBridgeIngressCntrModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntBridgeIngressCntrModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntDropCntrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntHostGroupCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntMatrixGroupCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntBridgeIngressCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntLearnedEntryDiscGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntPortGroupDropCntrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntPortGroupDropCntrGet)
#ifdef ASIC_SIMULATION
#ifndef FORCE_GM_USE
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntPortGroupHostGroupCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntPortGroupMatrixGroupCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntPortGroupBridgeIngressCntrsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgCntPortGroupLearnedEntryDiscGet)
#endif /* FORCE_GM_USE */
#endif /* ASIC_SIMULATION */
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgCount)



