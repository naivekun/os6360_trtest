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
* @file cpssDxChPhySmiPreInitUT.c
*
* @brief Unit tests for cpssDxChPhySmiPreInit.
*
* @version   13
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/generic/phy/private/prvCpssGenPhySmi.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhySmi.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define PHY_SMI_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyAutoPollNumOfPortsSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi0,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi1
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyAutoPollNumOfPortsSet) */
GT_VOID cpssDxChPhyAutoPollNumOfPortsSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with all acceptable combinations.
        The acceptable combinations for Auto Poll number of ports are:
*       |----------------------------------------------|
*       |   SMI 0   |   SMI 1   |   SMI 2   |   SMI 3  |
*       |-----------|-----------|-----------|----------|
*       |     8     |    16     |     8     |    16    |
*       |     8     |    12     |     8     |    12    |
*       |    12     |    12     |    12     |    12    |
*       |    16     |    8      |    16     |    8     |
*       |----------------------------------------------|
    Expected: GT_OK for each combination.
    1.2. Call cpssDxChPhyAutoPollNumOfPortsGet with the same params.
    Expected: GT_OK and the same value.
    1.3. Call with wrong combination [8 - 8, 16 - 16].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values autoPollNumOfPortsSmi0.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong enum values autoPollNumOfPortsSmi1.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong enum values autoPollNumOfPortsSmi2.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong enum values autoPollNumOfPortsSmi3.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi0 = 0xffffffff;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi1 = 0xffffffff;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi2 = 0xffffffff;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi3 = 0xffffffff;

    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi0Get = 0xffffffff;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi1Get = 0xffffffff;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi2Get = 0xffffffff;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi3Get = 0xffffffff;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices, ALL families*/
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E
                                     | UTF_LION2_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with all acceptable combinations.
                The acceptable combinations for Auto Poll number of ports are:
*       |----------------------------------------------|
*       |   SMI 0   |   SMI 1   |   SMI 2   |   SMI 3  |
*       |-----------|-----------|-----------|----------|
*       |     8     |    16     |     8     |    16    |
*       |     8     |    12     |     8     |    12    |
*       |    12     |    12     |    12     |    12    |
*       |    16     |    8      |    16     |    8     |
*       |----------------------------------------------|
            Expected: GT_OK for each combination.
        */
        if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev))
        {
            st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                                       autoPollNumOfPortsSmi1,
                                                       autoPollNumOfPortsSmi2,
                                                       autoPollNumOfPortsSmi3);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);

            continue;
        }

        /* call with  8 - 16, 8 - 16  combination */
        autoPollNumOfPortsSmi0 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;
        autoPollNumOfPortsSmi1 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E;
        autoPollNumOfPortsSmi2 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;
        autoPollNumOfPortsSmi3 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E;

        st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                                   autoPollNumOfPortsSmi1,
                                                   autoPollNumOfPortsSmi2,
                                                   autoPollNumOfPortsSmi3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhyAutoPollNumOfPortsGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                                   &autoPollNumOfPortsSmi1Get,
                                                   &autoPollNumOfPortsSmi2Get,
                                                   &autoPollNumOfPortsSmi3Get);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi0,
                                     autoPollNumOfPortsSmi0Get,
            "get another autoPollNumOfPortsSmi0 than was set: dev = %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi1,
                                     autoPollNumOfPortsSmi1Get,
            "get another autoPollNumOfPortsSmi1 than was set: dev = %d", dev);

        if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                         autoPollNumOfPortsSmi2Get,
                "get another autoPollNumOfPortsSmi2 than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                         autoPollNumOfPortsSmi3Get,
                "get another autoPollNumOfPortsSmi3 than was set: dev = %d", dev);

        }
        else
        {
            /* Verifying values */
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                         autoPollNumOfPortsSmi2Get,
                "get same autoPollNumOfPortsSmi2 that was set: dev = %d", dev);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                         autoPollNumOfPortsSmi3Get,
                "get same autoPollNumOfPortsSmi3 that was set: dev = %d", dev);
        }
        /* call with  8 - 12, 8 - 12  combination */
        autoPollNumOfPortsSmi0 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;
        autoPollNumOfPortsSmi1 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E;
        autoPollNumOfPortsSmi2 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;
        autoPollNumOfPortsSmi3 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E;

        st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                                   autoPollNumOfPortsSmi1,
                                                   autoPollNumOfPortsSmi2,
                                                   autoPollNumOfPortsSmi3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhyAutoPollNumOfPortsGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                                   &autoPollNumOfPortsSmi1Get,
                                                   &autoPollNumOfPortsSmi2Get,
                                                   &autoPollNumOfPortsSmi3Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi0,
                                     autoPollNumOfPortsSmi0Get,
            "get another autoPollNumOfPortsSmi0 than was set: dev = %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi1,
                                     autoPollNumOfPortsSmi1Get,
            "get another autoPollNumOfPortsSmi1 than was set: dev = %d", dev);
        if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                         autoPollNumOfPortsSmi2Get,
                "get another autoPollNumOfPortsSmi2 than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                         autoPollNumOfPortsSmi3Get,
                "get another autoPollNumOfPortsSmi3 than was set: dev = %d", dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                         autoPollNumOfPortsSmi2Get,
                "get same autoPollNumOfPortsSmi2 that was set: dev = %d", dev);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                         autoPollNumOfPortsSmi3Get,
                "get same autoPollNumOfPortsSmi3 that was set: dev = %d", dev);
        }


        /* call with  12 - 12, 12 - 12  combination */
        autoPollNumOfPortsSmi0 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E;
        autoPollNumOfPortsSmi1 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E;
        autoPollNumOfPortsSmi2 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E;
        autoPollNumOfPortsSmi3 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E;

        st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                                   autoPollNumOfPortsSmi1,
                                                   autoPollNumOfPortsSmi2,
                                                   autoPollNumOfPortsSmi3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhyAutoPollNumOfPortsGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                                   &autoPollNumOfPortsSmi1Get,
                                                   &autoPollNumOfPortsSmi2Get,
                                                   &autoPollNumOfPortsSmi3Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi0,
                                     autoPollNumOfPortsSmi0Get,
            "get another autoPollNumOfPortsSmi0 than was set: dev = %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi1,
                                     autoPollNumOfPortsSmi1Get,
            "get another autoPollNumOfPortsSmi1 than was set: dev = %d", dev);
        if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                         autoPollNumOfPortsSmi2Get,
                "get another autoPollNumOfPortsSmi2 than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                         autoPollNumOfPortsSmi3Get,
                "get another autoPollNumOfPortsSmi3 than was set: dev = %d", dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                             autoPollNumOfPortsSmi2Get,
                "get same autoPollNumOfPortsSmi2 that was set: dev = %d", dev);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                             autoPollNumOfPortsSmi3Get,
                "get same autoPollNumOfPortsSmi3 that was set: dev = %d", dev);
        }

        /* call with  16 - 8, 16 - 8 combination */
        autoPollNumOfPortsSmi0 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E;
        autoPollNumOfPortsSmi1 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;
        autoPollNumOfPortsSmi2 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E;
        autoPollNumOfPortsSmi3 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;

        st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                                   autoPollNumOfPortsSmi1,
                                                   autoPollNumOfPortsSmi2,
                                                   autoPollNumOfPortsSmi3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhyAutoPollNumOfPortsGet with the same params.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                                   &autoPollNumOfPortsSmi1Get,
                                                   &autoPollNumOfPortsSmi2Get,
                                                   &autoPollNumOfPortsSmi3Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi0,
                                     autoPollNumOfPortsSmi0Get,
            "get another autoPollNumOfPortsSmi0 than was set: dev = %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi1,
                                     autoPollNumOfPortsSmi1Get,
            "get another autoPollNumOfPortsSmi1 than was set: dev = %d", dev);

        if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                         autoPollNumOfPortsSmi2Get,
                "get another autoPollNumOfPortsSmi2 than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                         autoPollNumOfPortsSmi3Get,
                "get another autoPollNumOfPortsSmi3 than was set: dev = %d", dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi2,
                                         autoPollNumOfPortsSmi2Get,
                "get same autoPollNumOfPortsSmi2 that was set: dev = %d", dev);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(autoPollNumOfPortsSmi3,
                                         autoPollNumOfPortsSmi3Get,
                "get same autoPollNumOfPortsSmi3 that was set: dev = %d", dev);
        }

        /*
            1.3. Call with wrong combination [8 - 8, 16 - 16].
            Expected: GT_BAD_PARAM.
        */

        autoPollNumOfPortsSmi0 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;
        autoPollNumOfPortsSmi1 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;
        autoPollNumOfPortsSmi2 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E;
        autoPollNumOfPortsSmi3 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E;

        st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                                   autoPollNumOfPortsSmi1,
                                                   autoPollNumOfPortsSmi2,
                                                   autoPollNumOfPortsSmi3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.4. Call with wrong enum values autoPollNumOfPortsSmi0.
            Expected: GT_BAD_PARAM.
        */
        autoPollNumOfPortsSmi1 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPhyAutoPollNumOfPortsSet
                            (dev, autoPollNumOfPortsSmi0, autoPollNumOfPortsSmi1,
                                  autoPollNumOfPortsSmi2, autoPollNumOfPortsSmi3),
                            autoPollNumOfPortsSmi0);

        /*
            1.4. Call with wrong enum values autoPollNumOfPortsSmi1.
            Expected: GT_BAD_PARAM.
        */
        autoPollNumOfPortsSmi0 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPhyAutoPollNumOfPortsSet
                            (dev, autoPollNumOfPortsSmi0, autoPollNumOfPortsSmi1,
                                  autoPollNumOfPortsSmi2, autoPollNumOfPortsSmi3),
                            autoPollNumOfPortsSmi1);
        /*
            1.4. Call with wrong enum values autoPollNumOfPortsSmi2.
            Expected: GT_BAD_PARAM.
        */
        autoPollNumOfPortsSmi3 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPhyAutoPollNumOfPortsSet
                            (dev, autoPollNumOfPortsSmi0, autoPollNumOfPortsSmi1,
                                  autoPollNumOfPortsSmi2, autoPollNumOfPortsSmi3),
                            autoPollNumOfPortsSmi2);

        /*
            1.4. Call with wrong enum values autoPollNumOfPortsSmi3.
            Expected: GT_BAD_PARAM.
        */
        autoPollNumOfPortsSmi2 = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChPhyAutoPollNumOfPortsSet
                            (dev, autoPollNumOfPortsSmi0, autoPollNumOfPortsSmi1,
                                  autoPollNumOfPortsSmi2, autoPollNumOfPortsSmi3),
                            autoPollNumOfPortsSmi3);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator - iterate non-active devices*/
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
                         | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                                   autoPollNumOfPortsSmi1,
                                                   autoPollNumOfPortsSmi2,
                                                   autoPollNumOfPortsSmi3);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0,
                                               autoPollNumOfPortsSmi1,
                                               autoPollNumOfPortsSmi2,
                                               autoPollNumOfPortsSmi3);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyAutoPollNumOfPortsGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi0Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi1Ptr
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi2Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi3Ptr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyAutoPollNumOfPortsGet) */
GT_VOID cpssDxChPhyAutoPollNumOfPortsGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with not null pointers.
    Expected: GT_OK.
    1.2. Call with wrong autoPollNumOfPortsSmi0Ptr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with wrong autoPollNumOfPortsSmi1Ptr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong autoPollNumOfPortsSmi2Ptr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with wrong autoPollNumOfPortsSmi3Ptr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi0Get = 1;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi1Get = 1;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi2Get = 1;
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT autoPollNumOfPortsSmi3Get = 1;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices, ALL families*/
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
              UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pointers.
            Expected: GT_OK.
        */
        if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev))
        {
            st = cpssDxChPhyAutoPollNumOfPortsSet(dev, autoPollNumOfPortsSmi0Get,
                                                       autoPollNumOfPortsSmi1Get,
                                                       autoPollNumOfPortsSmi2Get,
                                                       autoPollNumOfPortsSmi3Get);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);

            continue;
        }

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                                   &autoPollNumOfPortsSmi1Get,
                                                   &autoPollNumOfPortsSmi2Get,
                                                   &autoPollNumOfPortsSmi3Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong autoPollNumOfPortsSmi0Ptr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, NULL, &autoPollNumOfPortsSmi1Get,
                                                   NULL, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with wrong autoPollNumOfPortsSmi1Ptr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                                   NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with wrong autoPollNumOfPortsSmi2Ptr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, NULL, NULL, NULL,
                                                   &autoPollNumOfPortsSmi3Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with wrong autoPollNumOfPortsSmi3Ptr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, NULL, NULL,
                                                   &autoPollNumOfPortsSmi2Get, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator - iterate non-active devices*/
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
               UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                                   &autoPollNumOfPortsSmi1Get,
                                                   &autoPollNumOfPortsSmi2Get,
                                                   &autoPollNumOfPortsSmi3Get);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhyAutoPollNumOfPortsGet(dev, &autoPollNumOfPortsSmi0Get,
                                               &autoPollNumOfPortsSmi1Get,
                                               &autoPollNumOfPortsSmi2Get,
                                               &autoPollNumOfPortsSmi3Get);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortSmiInterfacePortGroupGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32  *smiInterfacePortGroupPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhyPortSmiInterfacePortGroupGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion, Lion2)
    1.1.1. Call with valid smiInterfacePortGroupPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid smiInterfacePortGroupPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st      = GT_OK;
    GT_U8                           devNum  = 0;
    GT_PHYSICAL_PORT_NUM            portNum = PHY_SMI_VALID_PHY_PORT_CNS;
    GT_U32                          smiInterfacePortGroup;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                 UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E |
                 UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with valid smiInterfacePortGroupPtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                        &smiInterfacePortGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with invalid smiInterfacePortGroupPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                        &smiInterfacePortGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                    &smiInterfacePortGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PHY_SMI_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_XCAT2_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                    &smiInterfacePortGroup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                &smiInterfacePortGroup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortSmiInterfacePortGroupSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  smiInterfacePortGroup
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhyPortSmiInterfacePortGroupSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion, Lion2)
    1.1.1. Call with smiInterfacePortGroup [0, 2, 3(for Lion)/7 (for others)].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPhyPortSmiInterfacePortGroupGet().
    Expected: GT_OK and the same parameters value as was set in 1.1.1.
    1.1.3. Call with out of range
        smiInterfacePortGroup[4(for Lion)/8 (for others)]
        and other parameters as 1.1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS                   st             = GT_OK;
    GT_U8                       devNum         = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_PHYSICAL_PORT_NUM        portNum        = PHY_SMI_VALID_PHY_PORT_CNS;
    GT_U32                      smiInterfacePortGroup       = 0;
    GT_U32                      smiInterfacePortGroupRet    = 0;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                 UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E |
                 UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(devNum, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvUtfDeviceFamilyGet: %d", devNum);

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with smiInterfacePortGroup [0, 2, 3(for Lion)/7 (for others)].
                Expected: GT_OK.
            */
            smiInterfacePortGroup = 0;

            st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                        smiInterfacePortGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPhyPortSmiInterfacePortGroupGet().
                Expected: GT_OK and the same parameters value as was set in 1.1.1.
            */
            st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                     &smiInterfacePortGroupRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(smiInterfacePortGroup,
                                        smiInterfacePortGroupRet,
                                        devNum, portNum);

            /*
                1.1.1. Call with smiInterfacePortGroup [0, 2, 3(for Lion)/7 (for others)].
                Expected: GT_OK.
            */
            smiInterfacePortGroup = 2;

            st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                        smiInterfacePortGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPhyPortSmiInterfacePortGroupGet().
                Expected: GT_OK and the same parameters value as was set in 1.1.1.
            */
            st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                     &smiInterfacePortGroupRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(smiInterfacePortGroup,
                                        smiInterfacePortGroupRet,
                                        devNum, portNum);

            /*
                1.1.1. Call with smiInterfacePortGroup [0, 2, 3(for Lion)/7 (for others)].
                Expected: GT_OK.
            */
            if (devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
            {
                smiInterfacePortGroup = 3;
            }
            else
            {
                smiInterfacePortGroup = 7;
            }

            st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                        smiInterfacePortGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPhyPortSmiInterfacePortGroupGet().
                Expected: GT_OK and the same parameters value as was set in 1.1.1.
            */
            st = cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum,
                                                     &smiInterfacePortGroupRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(smiInterfacePortGroup,
                                        smiInterfacePortGroupRet,
                                        devNum, portNum);

            /* restore valid values */
            smiInterfacePortGroup = 0;

            /*
                1.1.3. Call with out of range
                    smiInterfacePortGroup[4(for Lion)/8 (for others)]
                    and other parameters as 1.1.1.
                Expected: NOT GT_OK.
            */
            if (devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
            {
                smiInterfacePortGroup = 4;
            }
            else
            {
                smiInterfacePortGroup = 8;
            }

            st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                        smiInterfacePortGroup);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* restore valid values */
            smiInterfacePortGroup = 0;

        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                        smiInterfacePortGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                    smiInterfacePortGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PHY_SMI_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_XCAT2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                    smiInterfacePortGroup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum,
                                                smiInterfacePortGroup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPhySmiPreInit suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPhySmiPreInit)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyAutoPollNumOfPortsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyAutoPollNumOfPortsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortSmiInterfacePortGroupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortSmiInterfacePortGroupSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPhySmiPreInit)



