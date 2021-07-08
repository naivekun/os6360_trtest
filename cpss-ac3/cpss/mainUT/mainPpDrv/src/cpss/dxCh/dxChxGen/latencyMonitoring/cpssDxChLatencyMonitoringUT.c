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
* @file cpssDxChLatencyMonitoringUT.c
*
* @brief Unit tests for cpssDxChLatencyMonitoring that provides the CPSS DXCH LMU APIs
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/latencyMonitoring/cpssDxChLatencyMonitoring.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringPortCfgSet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE valid PORTS
    1.1.1 Iterate port valid types
    1.1.1.1 Run through valid indexes
    1.1.1.1.1 Call with valid params
    Expect: GT_OK
    1.1.1.1.2 Call getter and verify
    Expect: GT_OK
    1.1.1.2 Run with invalid index
    Expect: GT_OUT_OF_RANGE
    1.1.1.3 Call with invalid profile
    Expect: GT_OUT_OF_RANGE
    1.2 ITERATE invalid PORTS
    1.2.1 Call with invalid port
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          index       = 0;
    GT_U32                                          indexGet;
    GT_PHYSICAL_PORT_NUM                            portNum     = 0;
    CPSS_DIRECTION_ENT                              portType    = CPSS_DIRECTION_INGRESS_E;
    GT_U32                                          profile     = 0;
    GT_U32                                          profileGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1 ITERATE valid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /* 1.1.1 Iterate port valid types */
            for(portType = CPSS_DIRECTION_INGRESS_E; portType <= CPSS_DIRECTION_BOTH_E; portType++)
            {
                /*1.1.1.1 Run through valid indexes*/
                for(index = 0; index < 256; index++)
                {
                    profile++;
                    profile = profile % 512;
                    /* 1.1.1.1.1 Call with valid params
                       Expect: GT_OK */
                    st = cpssDxChLatencyMonitoringPortCfgSet(dev, portNum,
                        portType, index, profile);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st,
                        dev, portNum, portType, index, profile);

                    /*  1.1.1.1.2 Call getter and verify
                        Expect: GT_OK */
                    st = cpssDxChLatencyMonitoringPortCfgGet(dev, portNum,
                        (portType == CPSS_DIRECTION_BOTH_E)?
                        CPSS_DIRECTION_INGRESS_E:portType, &indexGet, &profileGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st,
                        dev, portNum, portType);

                    UTF_VERIFY_EQUAL0_PARAM_MAC(profile, profileGet);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(index, indexGet);
                }

                /* 1.1.1.2 Run with invalid index
                   Expect: GT_OUT_OF_RANGE */
                st = cpssDxChLatencyMonitoringPortCfgSet(dev, portNum,
                    portType, 256, profile);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OUT_OF_RANGE, st,
                    dev, portNum, portType, index, profile);

                /* 1.1.1.3 Call with invalid profile
                   Expect: GT_OUT_OF_RANGE */
                st = cpssDxChLatencyMonitoringPortCfgSet(dev, portNum,
                    portType, index, 512);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OUT_OF_RANGE, st,
                    dev, portNum, portType, index, 512);
            }
        }

        index = 0;
        profile = 0;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 ITERATE invalid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1 Call with invalid port
               Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringPortCfgSet(dev, portNum,
                portType, index, profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringPortCfgSet(dev, portNum, portType, index, profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringPortCfgSet(dev, portNum, portType, index, profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringPortCfgGet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE valid PORTS
    1.1.1 Iterate port valid types
    1.1.1.1 Call with valid params
    Expect: GT_OK
    1.1.1.2 Call with indexPtr = NULL
    Expect: GT_OK
    1.1.1.3 Call with profilePtr = NULL
    Expect: GT_OK
    1.2 ITERATE invalid PORTS
    1.2.1 Call with invalid port
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM

*/
    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          indexGet;
    GT_PHYSICAL_PORT_NUM                            portNum     = 0;
    CPSS_DIRECTION_ENT                              portType    = CPSS_DIRECTION_INGRESS_E;
    GT_U32                                          profileGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1 ITERATE valid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /* 1.1.1 Iterate port valid types */
            for(portType = CPSS_DIRECTION_INGRESS_E; portType <= CPSS_DIRECTION_EGRESS_E; portType++)
            {
                /* 1.1.1.1 Run through valid params
                   Expect: GT_OK */
                st = cpssDxChLatencyMonitoringPortCfgGet(dev, portNum,
                    (portType == CPSS_DIRECTION_BOTH_E)?
                    CPSS_DIRECTION_INGRESS_E:portType, &indexGet, &profileGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st,
                    dev, portNum, portType);

                /*  1.1.1.2 Call with indexPtr = NULL
                    Expect: GT_OK */
                st = cpssDxChLatencyMonitoringPortCfgGet(dev, portNum,
                    (portType == CPSS_DIRECTION_BOTH_E)?
                    CPSS_DIRECTION_INGRESS_E:portType, NULL, &profileGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st,
                    dev, portNum, portType);

                /*  1.1.1.3 Call with profilePtr = NULL
                    Expect: GT_OK */
                st = cpssDxChLatencyMonitoringPortCfgGet(dev, portNum,
                    (portType == CPSS_DIRECTION_BOTH_E)?
                    CPSS_DIRECTION_INGRESS_E:portType, &indexGet, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st,
                    dev, portNum, portType);
            }
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 ITERATE invalid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1 Call with invalid port
               Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringPortCfgGet(dev, portNum,
                portType, &indexGet, &profileGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringPortCfgGet(dev, portNum, portType,
            &indexGet, &profileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringPortCfgGet(dev, portNum, portType,
        &indexGet, &profileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringEnableSet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE valid PORTS portNum1, portNum2
    1.1.1 Iterate enable [GT_FALSE, GT_TRUE]
    Expect: GT_OK
    1.1.2 Get and validate value [GT_FALSE, GT_TRUE]
    Expect: GT_OK
    1.2 ITERATE invalid portNum1 and valid portNum2
    1.2.1 Call with invalid portNum1 and valid portNum2
    Expect: GT_BAD_PARAM
    1.3 ITERATE invalid portNum1 and valid portNum2
    1.3.1 Call with invalid portNum1 and valid portNum2
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM

*/
    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          enable      = 0;
    GT_U32                                          enableGet;
    GT_PHYSICAL_PORT_NUM                            portNum1    = 0;
    GT_PHYSICAL_PORT_NUM                            portNum2    = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum1, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfNextMacPortReset(&portNum2, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1 ITERATE valid PORTS portNum1, portNum2 */
        while(1)
        {
            st = prvUtfNextMacPortGet(&portNum1, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }

            st = prvUtfNextMacPortGet(&portNum2, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }

            for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
            {

                /*  1.1.1 Iterate enable [GT_FALSE, GT_TRUE]
                    Expect: GT_OK */
                st = cpssDxChLatencyMonitoringEnableSet(dev, portNum1, portNum2,
                    enable);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum1, portNum2,
                    enable);

                /*  1.1.2 Get and validate value [GT_FALSE, GT_TRUE]
                    Expect: GT_OK */
                st = cpssDxChLatencyMonitoringEnableGet(dev, portNum1, portNum2,
                    &enableGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum1, portNum2);

                UTF_VERIFY_EQUAL2_PARAM_MAC(enable, enableGet, portNum1, portNum2);
            }
        }

        st = prvUtfNextMacPortReset(&portNum1, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        st = prvUtfNextMacPortReset(&portNum2, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 ITERATE invalid portNum1 and valid portNum2 */
        while(1)
        {
            st = prvUtfNextMacPortGet(&portNum1, GT_FALSE);
            if(GT_OK != st)
            {
                break;
            }
            st = prvUtfNextMacPortGet(&portNum2, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }

            /*  1.2.1 Call with invalid portNum1 and valid portNum2
                Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringEnableSet(dev, portNum1, portNum2,
                enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum1,
                portNum2, enable);
        }

        /* 1.3 ITERATE invalid portNum1 and valid portNum2 */
        while(1)
        {
            st = prvUtfNextMacPortGet(&portNum1, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }
            st = prvUtfNextMacPortGet(&portNum2, GT_FALSE);
            if(GT_OK != st)
            {
                break;
            }

            /*  1.3.1 Call with invalid portNum1 and valid portNum2
                Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringEnableSet(dev, portNum1, portNum2,
                enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum1,
                portNum2, enable);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringEnableSet(dev, portNum1, portNum2, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringEnableSet(dev, portNum1, portNum2, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringEnableGet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE valid PORTS portNum1, portNum2
    1.1.1 Call with valid parameters
    Expect: GT_OK
    1.1.2 Call with enablePtr = NULL
    Expect: GT_BAD_PTR
    1.2 ITERATE invalid portNum1 and valid portNum2
    1.2.1 Call with invalid portNum1 and valid portNum2
    Expect: GT_BAD_PARAM
    1.3 ITERATE invalid portNum1 and valid portNum2
    1.3.1 Call with invalid portNum1 and valid portNum2
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM

*/
    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          enableGet;
    GT_PHYSICAL_PORT_NUM                            portNum1    = 0;
    GT_PHYSICAL_PORT_NUM                            portNum2    = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum1, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfNextMacPortReset(&portNum2, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1 ITERATE valid PORTS portNum1, portNum2 */
        while(1)
        {
            st = prvUtfNextMacPortGet(&portNum1, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }

            st = prvUtfNextMacPortGet(&portNum2, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }

            /*  1.1.1 Call with valid parameters
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringEnableGet(dev, portNum1, portNum2,
                &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum1, portNum2);

            /*  1.1.2 Call with enablePtr = NULL
                Expect: GT_BAD_PTR */
            st = cpssDxChLatencyMonitoringEnableGet(dev, portNum1, portNum2,
                NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, portNum1, portNum2);
        }

        st = prvUtfNextMacPortReset(&portNum1, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        st = prvUtfNextMacPortReset(&portNum2, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 ITERATE invalid portNum1 and valid portNum2 */
        while(1)
        {
            st = prvUtfNextMacPortGet(&portNum1, GT_FALSE);
            if(GT_OK != st)
            {
                break;
            }
            st = prvUtfNextMacPortGet(&portNum2, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }

            /*  1.2.1 Call with invalid portNum1 and valid portNum2
                Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringEnableGet(dev, portNum1, portNum2,
                &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum1,
                portNum2);
        }

        /* 1.3 ITERATE invalid portNum1 and valid portNum2 */
        while(1)
        {
            st = prvUtfNextMacPortGet(&portNum1, GT_TRUE);
            if(GT_OK != st)
            {
                break;
            }
            st = prvUtfNextMacPortGet(&portNum2, GT_FALSE);
            if(GT_OK != st)
            {
                break;
            }

            /*  1.3.1 Call with invalid portNum1 and valid portNum2
                Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringEnableGet(dev, portNum1, portNum2,
                &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum1,
                portNum2);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringEnableGet(dev, portNum1, portNum2,
            &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringEnableGet(dev, portNum1, portNum2,
        &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringSamplingProfileSet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE through latency profiles
    1.1.1 Call with valid parameters
    Expect: GT_OK
    1.1.2 Call getter and verify data
    Expect: GT_OK
    1.2  Call with invalid sampling profile
    Expect: GT_OUT_OF_RANGE
    1.3  Call with invalid latency profile
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                       st                 = GT_OK;
    GT_U8                                           dev                = 0;
    GT_U32                                          latencyProfile     = 0;
    GT_U32                                          samplingProfile    = 0;
    GT_U32                                          samplingProfileGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 ITERATE through latency profiles */
        for(latencyProfile = 0; latencyProfile < 512; latencyProfile++)
        {
            /*  1.1.1 Call with valid parameters
                Expect: GT_OK */
            samplingProfile++;
            samplingProfile %= 8;
            st = cpssDxChLatencyMonitoringSamplingProfileSet(dev,
                latencyProfile, samplingProfile);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, latencyProfile,
                samplingProfile);

            /*  1.1.2 Call getter and verify data
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringSamplingProfileGet(dev,
                latencyProfile, &samplingProfileGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, latencyProfile);
            UTF_VERIFY_EQUAL3_PARAM_MAC(samplingProfile, samplingProfileGet,
                dev, latencyProfile, samplingProfile);
        }
        samplingProfile = 8;
        latencyProfile  = 0;

        /*  1.2  Call with invalid sampling profile
            Expect: GT_OUT_OF_RANGE */
        st = cpssDxChLatencyMonitoringSamplingProfileSet(dev,
            latencyProfile, samplingProfile);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, latencyProfile,
            samplingProfile);
        samplingProfile = 0;
        latencyProfile  = 512;

        /*  1.3  Call with invalid latency profile
            Expect: GT_BAD_PARAM */
        st = cpssDxChLatencyMonitoringSamplingProfileSet(dev,
            latencyProfile, samplingProfile);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, latencyProfile,
            samplingProfile);
        samplingProfile = 0;
        latencyProfile  = 0;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringSamplingProfileSet(dev, latencyProfile,
            samplingProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringSamplingProfileSet(dev, latencyProfile,
        samplingProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringSamplingProfileGet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE through latency profiles
    1.1.1 Call with valid parameters
    Expect: GT_OK
    1.1.2 Call with samplingProfilePtr = NULL
    Expect: GT_BAD_PTR
    1.2  Call with invalid latency profile
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                       st                 = GT_OK;
    GT_U8                                           dev                = 0;
    GT_U32                                          latencyProfile     = 0;
    GT_U32                                          samplingProfileGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 ITERATE through latency profiles */
        for(latencyProfile = 0; latencyProfile < 512; latencyProfile++)
        {
            /*  1.1.1 Call with valid parameters
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringSamplingProfileGet(dev,
                latencyProfile, &samplingProfileGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, latencyProfile);

            /*  1.1.2 Call with samplingProfilePtr = NULL
                Expect: GT_BAD_PTR */
            st = cpssDxChLatencyMonitoringSamplingProfileGet(dev,
                latencyProfile, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, latencyProfile);
        }
        latencyProfile  = 512;

        /*  1.2  Call with invalid latency profile
            Expect: GT_BAD_PARAM */
        st = cpssDxChLatencyMonitoringSamplingProfileGet(dev,
            latencyProfile, &samplingProfileGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, latencyProfile,
            &samplingProfileGet);
        latencyProfile  = 0;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringSamplingProfileGet(dev, latencyProfile,
            &samplingProfileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringSamplingProfileGet(dev, latencyProfile,
        &samplingProfileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringSamplingConfigurationSet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE sampling profiles
    1.1.1 ITERATE through latency sampling modes
    1.1.1.1 Call with valid parameters
    Expect: GT_OK
    1.1.1.2 Call getter and verify data
    1.2  Call with invalid sampling profile
    Expect: GT_BAD_PARAM
    1.3  Call with invalid sampling mode
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                     st                   = GT_OK;
    GT_U8                                         dev                  = 0;
    GT_U32                                        samplingThreshold    = 0;
    GT_U32                                        samplingThresholdGet = 0;
    GT_U32                                        samplingProfile      = 0;
    CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode        =
        CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E;
    CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingModeGet     =
        CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 ITERATE through latency profiles */
        for(samplingProfile = 0; samplingProfile < 8; samplingProfile++)
        {
            samplingThreshold = 0xFFFFFFFF - samplingProfile;

            /* 1.1.1 ITERATE through latency sampling modes */
            for(samplingMode = CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E;
                samplingMode <= CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E;
                samplingMode++)
            {
                /*  1.1.1.1 Call with valid parameters
                    Expect: GT_OK */
                st = cpssDxChLatencyMonitoringSamplingConfigurationSet(dev,
                    samplingProfile, samplingMode, samplingThreshold);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, samplingProfile,
                    samplingMode, samplingThreshold);

                /*  1.1.1.2 Call getter and verify data
                    Expect: GT_OK */
                st = cpssDxChLatencyMonitoringSamplingConfigurationGet(dev,
                    samplingProfile, &samplingModeGet, &samplingThresholdGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, samplingMode,
                    samplingProfile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(samplingThreshold, samplingThresholdGet,
                    dev, samplingProfile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(samplingMode, samplingModeGet,
                    dev, samplingProfile);
            }
        }
        samplingProfile = 8;

        /*  1.2  Call with invalid sampling profile
            Expect: GT_BAD_PARAM */
        st = cpssDxChLatencyMonitoringSamplingConfigurationSet(dev,
            samplingProfile, samplingMode, samplingThreshold);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, samplingProfile,
            samplingMode, samplingThreshold);
        samplingProfile = 0;

        samplingMode = CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E + 1;

        /*  1.3  Call with invalid sampling mode
            Expect: GT_BAD_PARAM */
        st = cpssDxChLatencyMonitoringSamplingConfigurationSet(dev,
            samplingProfile, samplingMode, samplingThreshold);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, samplingProfile,
            samplingMode, samplingThreshold);
        samplingMode = CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringSamplingConfigurationSet(dev, samplingProfile,
            samplingMode, samplingThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringSamplingConfigurationSet(dev, samplingProfile,
        samplingMode, samplingThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringSamplingConfigurationGet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE sampling profiles
    1.1.1 Call with valid parameters
    Expect: GT_OK
    1.2  Call with invalid sampling profile
    Expect: GT_OUT_OF_RANGE
    1.3  Call with samplingProfileThresholdPtr = NULL
    Expect: GT_BAD_PTR
    1.4  Call with samplingProfileModePtr = NULL
    Expect: GT_BAD_PTR
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                      st                   = GT_OK;
    GT_U8                                          dev                  = 0;
    CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingModeGet = 0;
    GT_U32                                         samplingThresholdGet = 0;
    GT_U32                                         samplingProfile      = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 ITERATE through sampling profiles */
        for(samplingProfile = 0; samplingProfile < 8; samplingProfile++)
        {
            /*  1.1.1 Call with valid parameters
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringSamplingConfigurationGet(dev,
                samplingProfile, &samplingModeGet, &samplingThresholdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, samplingProfile);
        }
        samplingProfile = 8;

        /*  1.2  Call with invalid sampling profile
            Expect: GT_BAD_PARAM */
        st = cpssDxChLatencyMonitoringSamplingConfigurationGet(dev,
            samplingProfile, &samplingModeGet, &samplingThresholdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, samplingProfile);
        samplingProfile = 0;

        /*  1.3  Call with samplingProfileThresholdPtr = NULL
            Expect: GT_BAD_PTR */
        st = cpssDxChLatencyMonitoringSamplingConfigurationGet(dev,
            samplingProfile, &samplingModeGet, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, samplingProfile);

        /*  1.4  Call with samplingProfileModePtr = NULL
            Expect: GT_BAD_PTR */
        st = cpssDxChLatencyMonitoringSamplingConfigurationGet(dev,
            samplingProfile, NULL, &samplingThresholdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, samplingProfile);
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringSamplingConfigurationGet(dev, samplingProfile,
            &samplingModeGet, &samplingThresholdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringSamplingConfigurationGet(dev, samplingProfile,
        &samplingModeGet, &samplingThresholdGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringStatGet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE LMUs
    1.1.1 Iterate through latency profiles
    1.1.1.1 Call with given LMU and latency profile
    Expect: GT_OK pr GT_BAD_PARAM depending on lmuBmp
    1.2  Call with statisticsPtr = NULL
    Expect: GT_BAD_PTR
    1.3  Call with bad latency profile
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                     st                   = GT_OK;
    GT_U8                                         dev                  = 0;
    GT_U32                                        latencyProfile       = 0;
    GT_U32                                        ii;
    GT_LATENCY_MONITORING_UNIT_BMP                lmuBmp               = 1;
    CPSS_DXCH_LATENCY_MONITORING_STAT_STC         statisticsGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 ITERATE LMUs */
        for(ii = 0; ii < 32; ii++)
        {
            /* 1.1.1 Iterate through latency profiles */
            lmuBmp = 1 << ii;
            for(latencyProfile = 0; latencyProfile < 512; latencyProfile++)
            {
                /*  1.1.1.1 Call with given LMU and latency profile
                    Expect: GT_OK pr GT_BAD_PARAM depending on lmuBmp */
                st = cpssDxChLatencyMonitoringStatGet(dev, lmuBmp, latencyProfile,
                    &statisticsGet);
                if(U32_GET_FIELD_MAC(ii, 3, 2) >= PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, lmuBmp, latencyProfile);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lmuBmp, latencyProfile);
                }
            }
        }

        /*  1.2  Call with statisticsPtr = NULL
            Expect: GT_BAD_PTR */
        latencyProfile = 0;
        lmuBmp = 1;
        st = cpssDxChLatencyMonitoringStatGet(dev, lmuBmp, latencyProfile,
            NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, lmuBmp, latencyProfile);

        /*  1.3  Call with bad latency profile
            Expect: GT_BAD_PARAM */
        latencyProfile = 512;
        st = cpssDxChLatencyMonitoringStatGet(dev, lmuBmp, latencyProfile,
            &statisticsGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, lmuBmp, latencyProfile);
        latencyProfile = 0;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringStatGet(dev, lmuBmp, latencyProfile,
            &statisticsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringStatGet(dev, lmuBmp, latencyProfile,
        &statisticsGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringCfgSet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE LMUs
    1.1.1 Iterate through latency profiles
    1.1.1.1 Valid LMU
    1.1.1.1.1 Call with valid parameters
    Expect: GT_OK
    1.1.1.1.2 Verify data written
    Expect: GT_OK
    1.1.1.1.3 Call with invalid rangeMin
    Expect: GT_OUT_OF_RANGE
    1.1.1.1.4 Call with invalid rangeMax
    Expect: GT_OUT_OF_RANGE
    1.1.1.1.5 Call with invalid notification threshold
    Expect: GT_OUT_OF_RANGE
    1.1.1.2 Invalid LMU
    Expect: GT_BAD_PARAM
    1.2  Call with bad latency profile
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                     st                   = GT_OK;
    GT_U8                                         dev                  = 0;
    GT_U32                                        latencyProfile       = 0;
    GT_U32                                        ii;
    GT_LATENCY_MONITORING_UNIT_BMP                lmuBmp               = 1;
    CPSS_DXCH_LATENCY_MONITORING_CFG_STC          lmuConfig;
    CPSS_DXCH_LATENCY_MONITORING_CFG_STC          lmuConfigGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 ITERATE LMUs */
        for(ii = 0; ii < 32; ii++)
        {
            /* 1.1.1 Iterate through latency profiles */
            lmuBmp = 1 << ii;
            for(latencyProfile = 0; latencyProfile < 512; latencyProfile++)
            {
                if(U32_GET_FIELD_MAC(ii, 3, 2) < PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles)
                {
                    /*  1.1.1.1 Valid LMU */
                    /*  1.1.1.1.1 Call with valid parameters
                        Expect: GT_OK */
                    lmuConfig.rangeMin = 0x3FFFFFFF - 512 - latencyProfile;
                    lmuConfig.rangeMax = 0x3FFFFFFF - latencyProfile;
                    lmuConfig.notificationThresh = 0x3FFFFFFF - 1024 - latencyProfile;
                    st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp,
                        latencyProfile, &lmuConfig);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lmuBmp,
                        latencyProfile);

                    /*  1.1.1.1.2 Verify data written
                        Expect: GT_OK */
                    st = cpssDxChLatencyMonitoringCfgGet(dev, lmuBmp,
                        latencyProfile, &lmuConfigGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lmuBmp,
                        latencyProfile);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(lmuConfig.rangeMin,
                        lmuConfigGet.rangeMin, dev, lmuBmp, latencyProfile);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(lmuConfig.rangeMax,
                        lmuConfigGet.rangeMax, dev, lmuBmp, latencyProfile);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(lmuConfig.notificationThresh,
                        lmuConfigGet.notificationThresh, dev, lmuBmp,
                        latencyProfile);

                    /*  1.1.1.1.3 Call with invalid rangeMin
                        Expect: GT_OUT_OF_RANGE */
                    lmuConfig.rangeMin = 0x3FFFFFFF + 1;
                    st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp,
                        latencyProfile, &lmuConfig);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, lmuBmp,
                        latencyProfile);
                    lmuConfig.rangeMin = 0x3FFFFFFF - 512 - latencyProfile;

                    /*  1.1.1.1.4 Call with invalid rangeMax
                        Expect: GT_OUT_OF_RANGE */
                    lmuConfig.rangeMax = 0x3FFFFFFF + 1;
                    st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp,
                        latencyProfile, &lmuConfig);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, lmuBmp,
                        latencyProfile);
                    lmuConfig.rangeMax = 0x3FFFFFFF - latencyProfile;

                    /*  1.1.1.1.5 Call with invalid notification threshold
                        Expect: GT_OUT_OF_RANGE */
                    lmuConfig.notificationThresh = 0x3FFFFFFF + 1;
                    st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp,
                        latencyProfile, &lmuConfig);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, lmuBmp,
                        latencyProfile);
                    lmuConfig.notificationThresh = 0x3FFFFFFF - 1024 - latencyProfile;
                }
                else
                {
                    /* 1.1.1.2 Invalid LMU */
                    st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp,
                        latencyProfile, &lmuConfig);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, lmuBmp,
                        latencyProfile);
                }
            }
        }

        /*  1.2  Call with bad latency profile
            Expect: GT_BAD_PARAM */
        latencyProfile = 512;
        st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp, latencyProfile,
            &lmuConfig);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, lmuBmp,
            latencyProfile);
        latencyProfile = 0;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp, latencyProfile,
            &lmuConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringCfgSet(dev, lmuBmp, latencyProfile,
        &lmuConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringCfgGet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE LMUs
    1.1.1 Iterate through latency profiles
    1.1.1.1 Call with given LMU and latency profile
    Expect: GT_OK pr GT_BAD_PARAM depending on lmuBmp
    1.2  Call with lmuConfigPtr = NULL
    Expect: GT_BAD_PTR
    1.3  Call with bad latency profile
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                     st                   = GT_OK;
    GT_U8                                         dev                  = 0;
    GT_U32                                        latencyProfile       = 0;
    GT_U32                                        ii;
    GT_LATENCY_MONITORING_UNIT_BMP                lmuBmp               = 1;
    CPSS_DXCH_LATENCY_MONITORING_CFG_STC          lmuConfigGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1 ITERATE LMUs */
        for(ii = 0; ii < 32; ii++)
        {
            /* 1.1.1 Iterate through latency profiles */
            lmuBmp = 1 << ii;
            for(latencyProfile = 0; latencyProfile < 512; latencyProfile++)
            {
                /*  1.1.1.1 Call with given LMU and latency profile
                    Expect: GT_OK pr GT_BAD_PARAM depending on lmuBmp */
                st = cpssDxChLatencyMonitoringCfgGet(dev, lmuBmp, latencyProfile,
                    &lmuConfigGet);
                if(U32_GET_FIELD_MAC(ii, 3, 2) >= PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, lmuBmp, latencyProfile);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lmuBmp, latencyProfile);
                }
            }
        }

        /*  1.2  Call with lmuConfigPtr = NULL
            Expect: GT_BAD_PTR */
        latencyProfile = 0;
        lmuBmp = 1;
        st = cpssDxChLatencyMonitoringCfgGet(dev, lmuBmp, latencyProfile,
            NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, lmuBmp, latencyProfile);

        /*  1.3  Call with bad latency profile
            Expect: GT_BAD_PARAM */
        latencyProfile = 512;
        st = cpssDxChLatencyMonitoringCfgGet(dev, lmuBmp, latencyProfile,
            &lmuConfigGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, lmuBmp, latencyProfile);
        latencyProfile = 0;
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringCfgGet(dev, lmuBmp, latencyProfile,
            &lmuConfigGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringCfgGet(dev, lmuBmp, latencyProfile,
        &lmuConfigGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringPortEnableSet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE valid PORTS
    1.1.1 Iterate port valid types
    1.1.1.1 Call with valid params
    Expect: GT_OK
    1.1.1.2 Call getter and verify
    Expect: GT_OK
    1.2 ITERATE invalid PORTS
    1.2.1 Call with invalid port
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM
*/
    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_PHYSICAL_PORT_NUM                            portNum     = 0;
    GT_U32                                          enable      = 0;
    GT_U32                                          enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1 ITERATE valid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /* 1.1.1 Call with valid params
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringPortEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);

            /*  1.1.2 Call getter and verify
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringPortEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 ITERATE invalid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1 Call with invalid port
               Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringPortEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringPortEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
       Expect: GT_BAD_PARAM */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringPortEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

UTF_TEST_CASE_MAC(cpssDxChLatencyMonitoringPortEnableGet)
{
/*
    1. ITERATE_DEVICES
    1.1 ITERATE valid PORTS
    1.1.1 Call with valid params
    Expect: GT_OK
    1.1.2 Call with enablePtr = NULL
    Expect: GT_OK
    1.2 ITERATE invalid PORTS
    1.2.1 Call with invalid port
    Expect: GT_BAD_PARAM
    2. Call with not active device numbers
    Expect: GT_NOT_APPLICABLE_DEVICE
    3. Call with bad value of device id
    Expect: GT_BAD_PARAM

*/
    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          enableGet;
    GT_PHYSICAL_PORT_NUM                            portNum     = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1 ITERATE valid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /* 1.1.1 Run through valid params
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringPortEnableGet(dev, portNum,
                 &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

            /*  1.1.2 Call with enablePtr = NULL
                Expect: GT_OK */
            st = cpssDxChLatencyMonitoringPortEnableGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2 ITERATE invalid PORTS */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1 Call with invalid port
               Expect: GT_BAD_PARAM */
            st = cpssDxChLatencyMonitoringPortEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 2. Call with not active device numbers
       Expect: GT_NOT_APPLICABLE_DEVICE */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChLatencyMonitoringPortEnableGet(dev, portNum, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChLatencyMonitoringPortEnableGet(dev, portNum, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChLatencyMonitoring suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChLatencyMonitoring)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringPortCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringPortCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringSamplingProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringSamplingProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringSamplingConfigurationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringSamplingConfigurationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringStatGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringPortEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLatencyMonitoringPortEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChLatencyMonitoring)
