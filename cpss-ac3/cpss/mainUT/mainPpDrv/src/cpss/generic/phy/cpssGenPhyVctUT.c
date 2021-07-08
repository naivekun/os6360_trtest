/*******************************************************************************
*              Copyright 2006, MARVELL TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
*/
/**
********************************************************************************
* @file cpssGenPhyVctUT.c
*
* @brief Unit tests for cpssGenPhyVct , that provides
* CPSS Marvell Virtual Cable Tester functionality.
*
*
* @version   8
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */

#include <cpss/generic/phy/cpssGenPhyVct.h>
#include <cpss/generic/phy/private/prvCpssGenPhySmi.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Invalid enum */
#define PHY_GEN_INVALID_ENUM_CNS    0x5AAAAAA5

static GT_STATUS prvDoesDeviceSupportVct(GT_U8 dev, GT_PHYSICAL_PORT_NUM port);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssVctCableExtendedStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               port,
    OUT CPSS_VCT_CABLE_EXTENDED_STATUS_STC  *extendedCableStatus
)
*/
UTF_TEST_CASE_MAC(cpssVctCableExtendedStatusGet)
{
/*
    ITERATE_DEVICE_PHY_PORT (All devices)
    1.1.1. Call with not NULL extendedCableStatus.
    Expected: GT_OK.
    1.1.2. Call with extendedCableStatus [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port;

    CPSS_VCT_CABLE_EXTENDED_STATUS_STC  extendedCableStatus;
    PRV_CPSS_PORT_TYPE_ENT              portType;

    cpssOsBzero((GT_VOID*) &extendedCableStatus, sizeof(extendedCableStatus));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* check if the device supports VCT */
            st = prvDoesDeviceSupportVct(dev, port);
            if (GT_OK != st)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            if (portType != PRV_CPSS_PORT_XG_E)
            {
                /*
                    1.1.1. Call with not NULL extendedCableStatus.
                    Expected: GT_OK.
                */
                st = cpssVctCableExtendedStatusGet(dev, port, &extendedCableStatus);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call with extendedCableStatus [NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssVctCableExtendedStatusGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, extendedCableStatusPtr = NULL", dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssVctCableExtendedStatusGet(dev, port, &extendedCableStatus);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssVctCableExtendedStatusGet(dev, port, &extendedCableStatus);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssVctCableExtendedStatusGet(dev, port, &extendedCableStatus);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssVctCableExtendedStatusGet(dev, port, &extendedCableStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssVctCableExtendedStatusGet(dev, port, &extendedCableStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssVctCableStatusGet
(
    IN  GT_U8                     dev,
    IN  GT_U8                     port,
    IN  CPSS_VCT_ACTION_ENT       vctAction,
    OUT CPSS_VCT_CABLE_STATUS_STC *cableStatus
)
*/
UTF_TEST_CASE_MAC(cpssVctCableStatusGet)
{
/*
    ITERATE_DEVICE_PHY_PORT (All devices)
    1.1.1. Call with vctAction [CPSS_VCT_START_E / CPSS_VCT_GET_RES_E] and not NULL cableStatus.
    Expected: GT_OK.
    1.1.2. Call with vctAction [0x5AAAAAA5] and not NULL cableStatus.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with cableStatus [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_U32      ii;

    CPSS_VCT_ACTION_ENT       vctAction = CPSS_VCT_START_E;
    CPSS_VCT_CABLE_STATUS_STC cableStatus;
    PRV_CPSS_PORT_TYPE_ENT              portType;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* skip to reduce size of CPSS API log */
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &cableStatus, sizeof(cableStatus));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports and start VCT process. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* check if the device supports VCT */
            st = prvDoesDeviceSupportVct(dev, port);
            if (GT_OK != st)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            if(portType != PRV_CPSS_PORT_XG_E)
            {
                /*
                    1.1.1. Call with vctAction [CPSS_VCT_START_E / CPSS_VCT_GET_RES_E] and not NULL cableStatus.
                    Expected: GT_OK.
                */
                vctAction = CPSS_VCT_START_E;
                st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, vctAction);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all available physical ports and Get VCT results. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* check if the device supports VCT */
            st = prvDoesDeviceSupportVct(dev, port);
            if (GT_OK != st)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            if(portType != PRV_CPSS_PORT_XG_E)
            {
                /*
                    1.2.1. Call with vctAction [CPSS_VCT_GET_RES_E] and not NULL cableStatus.
                    Expected: GT_OK.
                */
                vctAction = CPSS_VCT_GET_RES_E;
                ii = 0;
                do
                {
                    /* prevent infinite loops in case HW problem.
                      SMI usual clock is 2Mhz.
                      Each call uses at least one SMI register read - 64 Bits
                      per SMI read transaction. So minimal time is
                      64/(2000000) = 32 micro seconds.
                      I == 200 000 is approx 6.4 seconds or more.
                      This is sufficient time to finish VCT on all 4 pairs. */
                    if (ii == 200000)
                    {
                        break;
                    }
                    ii++;
                    st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);

                    if((ii % 20000) == 0)
                    {
                        utfPrintKeepAlive();
                    }

                }while(st == GT_NOT_READY);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, vctAction);
                /*
                    1.2.2. Call with vctAction [0x5AAAAAA5] and not NULL cableStatus.
                    Expected: GT_BAD_PARAM.
                */
                vctAction = PHY_GEN_INVALID_ENUM_CNS;

                st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, vctAction);

                vctAction = CPSS_VCT_START_E;
                /*
                    1.2.3. Call with cableStatus [NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssVctCableStatusGet(dev, port, vctAction, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, cableStatusPtr = NULL", dev, port);
            }
        }

        vctAction = CPSS_VCT_START_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    vctAction = CPSS_VCT_START_E;
    port      = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssVctCableStatusGet(dev, port, vctAction, &cableStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssVctSetCableDiagConfig
(
    IN  GT_U8               dev,
    IN  GT_U8               port,
    IN  CPSS_VCT_CONFIG_STC *configPtr
)
*/
UTF_TEST_CASE_MAC(cpssVctSetCableDiagConfig)
{
/*
    ITERATE_DEVICE (All devices)
    1.1.1. Call with config{ length[CPSS_VCT_CABLE_LESS_10M_E / CPSS_VCT_CABLE_GREATER_10M_E] }
    Expected: GT_OK.
    1.1.2. Call with config{ length[0x5AAAAAA5] }.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with configPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;

    CPSS_VCT_CONFIG_STC config;


    cpssOsBzero((GT_VOID*) &config, sizeof(config));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* check if the device supports VCT */
            st = prvDoesDeviceSupportVct(dev, port);
            if (GT_OK != st)
            {
                continue;
            }

            /*
                1.1.1. Call with config{ length[CPSS_VCT_CABLE_LESS_10M_E / CPSS_VCT_CABLE_GREATER_10M_E] }
                Expected: GT_OK.
            */
            config.length = CPSS_VCT_CABLE_LESS_10M_E;

            st = cpssVctSetCableDiagConfig(dev, port, &config);
            if (GT_NOT_SUPPORTED == st)
            {   /* the PHY does not support the feature */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            config.length = CPSS_VCT_CABLE_GREATER_10M_E;

            st = cpssVctSetCableDiagConfig(dev, port, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with config{ length[0x5AAAAAA5] }.
                Expected: GT_BAD_PARAM.
            */
            config.length = PHY_GEN_INVALID_ENUM_CNS;

            st = cpssVctSetCableDiagConfig(dev, port, &config);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "%d, %d, configPtr->length = %d",
                                         dev, port, config.length);

            config.length = CPSS_VCT_CABLE_LESS_10M_E;

            /*
                1.1.3. Call with configPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssVctSetCableDiagConfig(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, configPtr = NULL", dev, port);
        }

        config.length = CPSS_VCT_CABLE_LESS_10M_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssVctSetCableDiagConfig(dev, port, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssVctSetCableDiagConfig(dev, port, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssVctSetCableDiagConfig(dev, port, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    config.length = CPSS_VCT_CABLE_LESS_10M_E;
    port          = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssVctSetCableDiagConfig(dev, port, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssVctSetCableDiagConfig(dev, port, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssGenPhyVct suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssGenPhyVct)
    UTF_SUIT_DECLARE_TEST_MAC(cpssVctCableExtendedStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssVctCableStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssVctSetCableDiagConfig)
UTF_SUIT_END_TESTS_MAC(cpssGenPhyVct)

/**
* @internal prvDoesDeviceSupportVct function
* @endinternal
*
* @brief   This routine check if the given device supports VCT.
*
* @param[in] dev                      - Device Number.
* @param[in] port                     - Port Number.
*
* @retval GT_OK                    - if the device supports VCT
* @retval GT_NOT_SUPPORTED         - otherwise
*/
static GT_STATUS prvDoesDeviceSupportVct
(
    IN  GT_U8 dev,
    IN  GT_PHYSICAL_PORT_NUM port
)
{
    GT_STATUS status;
    GT_U16    value;

    if(NULL == (&(PRV_CPSS_PP_MAC(dev)->phyInfo))->genVctBindFunc.cpssPhyRegisterWrite)
    {
        /* relevant for 'EMULATOR'*/
        return GT_NOT_SUPPORTED;
    }

    /* set page 0 */
    status = (&(PRV_CPSS_PP_MAC(dev)->phyInfo))->genVctBindFunc.cpssPhyRegisterWrite(dev, port, 22, 0);
    if (status != GT_OK)
    {
        return status;
    }

    /* read Marvel OUI */
    status = (&(PRV_CPSS_PP_MAC(dev)->phyInfo))->genVctBindFunc.cpssPhyRegisterRead(dev, port, PRV_CPSS_PHY_ID0,&value);

    if (status != GT_OK)
    {
        return status;
    }

    if (value != PRV_CPSS_MARVELL_OUI_MSB)
    {
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

