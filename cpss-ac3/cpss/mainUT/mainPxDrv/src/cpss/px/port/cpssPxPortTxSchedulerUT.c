/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxPortTxSchedulerUT.c
*
* @brief Unit tests for cpssPxPortTxScheduler APIs
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/port/cpssPxPortTxScheduler.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>


/* Default valid value for port id */
#define PORT_TX_VALID_PHY_PORT_CNS  0


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerProfileIdSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all applicable ports.
    1.1.1. Call cpssPxPortTxSchedulerProfileIdSet with profile:
           [CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E,
            ...,
            CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E]
           Expected: GT_OK.
    1.1.2. Call cpssPxPortTxSchedulerProfileIdGet with non-null profilePtr.
           Expected: GT_OK and the same profile as was set.
    1.1.3. Call with wrong enum values of profile.
           Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non-applicable ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function with out of bound value of
         port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function for CPU port number.
         Expected: GT_OK.
    2. Go over all non-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /* go over all profiles */
            for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
                 profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
                 profile++)
            {
                /*
                    1.1.1. Call cpssPxPortTxSchedulerProfileIdSet with profile
                           [CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E,
                            ...
                            CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E]
                    Expected: GT_OK
                */
                st = cpssPxPortTxSchedulerProfileIdSet(dev, port, profile);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profile);

                /*
                    1.1.2. Call cpssPxPortTxSchedulerProfileIdGet
                    Expected: GT_OK and the same profile.
                */
                st = cpssPxPortTxSchedulerProfileIdGet(dev, port, &profileGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssPxPortTxSchedulerProfileIdGet: %d, %d", dev, port);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(profile, profileGet,
                        "get another profileSetPtr than was set: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with wrong enum values profile.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortTxSchedulerProfileIdSet(dev, port, profile),
                                profile);
        }

        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxSchedulerProfileIdSet(dev, port, profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value of port number.                     */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxSchedulerProfileIdSet(dev, port, profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /*      for CPU port number.                                */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxSchedulerProfileIdSet(dev, port, profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE. */
    port = PORT_TX_VALID_PHY_PORT_CNS;
    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerProfileIdSet(dev, port, profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerProfileIdSet(dev, port, profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   *profileSetPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerProfileIdGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all applicable ports.
    1.1.1. Call cpssPxPortTxSchedulerProfileIdGet with non-null profileSetPtr.
           Expected: GT_OK.
    1.1.2. Call cpssPxPortTxSchedulerProfileIdGet with profileSetPtr[NULL].
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non-applicable ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function with out of bound value of
         port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function for CPU port number.
         Expected: GT_OK.
    2. Go over all non-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null profileSetPtr.
                Expected: GT_OK.
            */
            st = cpssPxPortTxSchedulerProfileIdGet(dev, port, &profileSet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with profileSetPtr[NULL]
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxSchedulerProfileIdGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxSchedulerProfileIdGet(dev, port, &profileSet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value for port number.                    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxSchedulerProfileIdGet(dev, port, &profileSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /*      for CPU port number.                                       */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxSchedulerProfileIdGet(dev, port, &profileSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE. */
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerProfileIdGet(dev, port, &profileSet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerProfileIdGet(dev, port, &profileSet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerWrrMtuSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerWrrMtuSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerWrrMtuSet with for all valid wrrMtu.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerWrrMtuGet with not NULL wrrMtuPtr.
         Expected: GT_OK and the same wrrMtu.
    1.3. Call with wrong enum values of wrrMtu
         Expected: GT_BAD_PARAM.
    2. Go over all non-active or not-applicable devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtuGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all available resolutions of WRR weights */
        for (wrrMtu = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64_E;
             wrrMtu <= CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64K_E;
             wrrMtu++)
        {
            /*
                1.1. Call cpssPxPortTxSchedulerWrrMtuSet for all valid MTUs.
                Expected: GT_OK.
            */
            st = cpssPxPortTxSchedulerWrrMtuSet(dev, wrrMtu);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, wrrMtu);

            /*
                1.2. Call cpssPxPortTxSchedulerWrrMtuGet with not NULL wrrModePtr.
                Expected: GT_OK and the same wrrMtu.
            */
            st = cpssPxPortTxSchedulerWrrMtuGet(dev, &wrrMtuGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxSchedulerWrrMtuGet: %d", dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(wrrMtu, wrrMtuGet,
                    "get another wrrMode than was set: %d", dev);
        }

        /*
            1.3. Call with wrong enum values wrrMtu
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortTxSchedulerWrrMtuSet(dev, wrrMtu),
                            wrrMtu);
    }

    wrrMtu = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_2K_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerWrrMtuSet(dev, wrrMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerWrrMtuSet(dev, wrrMtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerWrrMtuGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   *wrrMtuPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerWrrMtuGet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerWrrMtuGet with not null wrrMtu.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerWrrMtuGet with wrrMtu[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all non-active or not-applicable devices.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssPxPortTxSchedulerWrrMtuGet with not NULL wrrMtu.
            Expected: GT_OK.
        */
        st = cpssPxPortTxSchedulerWrrMtuGet(dev, &wrrMtu);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssPxPortTxSchedulerWrrMtuGet: %d", dev);

        /*
            1.2. Call cpssPxPortTxSchedulerWrrMtuGet with wrrMode[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxSchedulerWrrMtuGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                "cpssPxPortTxSchedulerWrrMtuGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerWrrMtuGet(dev, &wrrMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerWrrMtuGet(dev, &wrrMtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerWrrProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  GT_U32                                      wrrWeight
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerWrrProfileSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerWrrProfileSet with all valid params.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerWrrProfileGet with valid tcQueue, profile and
         valid pointer to wrrWeightGet.
         Expected: GT_OK and the same wrrWeight as in 1.1.
    1.3. Call function with tcQueue[5], wrrWeight[7] and wrong enum values profile.
         Expected: GT_BAD_PARAM.
    1.4. Call function with out of range tcQueue[CPSS_TC_RANGE_CNS], wrrWeight[65]
         and profile[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
         Expected: GT_BAD_PARAM.
    1.5. Call with tcQueue[1], wrrWeight[0xFF] (can not see any constraints for it)
         and profileSet[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
         Expected: GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tcQueue;
    GT_U32      wrrWeight;
    GT_U32      wrrWeightGet;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        wrrWeight = 0;

        /* go over all valid tcQueue */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            /* go over all valid profile */
            for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
                 profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
                 profile++)
            {
                /* Pseudorandom wrrWeight. Used pseudorandom value instead of cpssOsRand */
                /* to have possibility to reproduce UT with the same parameters.         */
                wrrWeight = (wrrWeight + 181) & 0xFF;

                /*
                    1.1. Call cpssPxPortTxSchedulerWrrProfileSet with valid tcQueue,
                         valid profile and valid wrrWeight.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxSchedulerWrrProfileSet(dev, profile, tcQueue, wrrWeight);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profile, tcQueue, wrrWeight);

                /*
                    1.2. Call cpssPxPortTxSchedulerWrrProfileGet with the same params.
                    Expected: GT_OK and the same wrrWeight.
                */
                st = cpssPxPortTxSchedulerWrrProfileGet(dev, profile, tcQueue, &wrrWeightGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, tcQueue);

                /* check wrrWeight */
                UTF_VERIFY_EQUAL3_STRING_MAC(wrrWeight, wrrWeightGet,
                        "got another wrrWeight than was set: %d, %d, %d",
                        dev, profile, tcQueue);
            }
        }

        /*
            1.3. Call with tcQueue[5], wrrWeight[7] and wrong enum values profile.
            Expected: GT_BAD_PARAM.
        */
        tcQueue = 5;
        wrrWeight = 7;
        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxSchedulerWrrProfileSet(dev, profile, tcQueue, wrrWeight),
                profile);

        /*
            1.4. Call with out of range tcQueue[CPSS_TC_RANGE_CNS], wrrWeight[65]
                 and profile[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = CPSS_TC_RANGE_CNS;
        wrrWeight = 65;
        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        st = cpssPxPortTxSchedulerWrrProfileSet(dev, profile, tcQueue, wrrWeight);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tcQueue, wrrWeight);

        /*
            1.5. Call with tcQueue[1], wrrWeight[0xFF] (can not see any constraints for it)
                 and profileSet[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
            Expected: GT_OK.
        */
        tcQueue = 1;
        wrrWeight = 0xFF;
        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        st = cpssPxPortTxSchedulerWrrProfileSet(dev, profile, tcQueue, wrrWeight);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profile, tcQueue, wrrWeight);
    }

    tcQueue = 1;
    wrrWeight = 15;
    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerWrrProfileSet(dev, profile, tcQueue, wrrWeight);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerWrrProfileSet(dev, profile, tcQueue, wrrWeight);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerWrrProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT GT_U32                                      wrrWeight
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerWrrProfileGet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerWrrProfileGet with all valid params.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerWrrProfileGet with valid tcQueue, profile and
         null-pointer to wrrWeight.
         Expected: GT_BAD_PTR.
    1.3. Call function with tcQueue[5], valid pointer to wrrWeight and wrong enum
         values profile.
         Expected: GT_BAD_PARAM.
    1.4. Call function with out of range tcQueue[CPSS_TC_RANGE_CNS], valid pointer
         to wrrWeight and profile[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tcQueue;
    GT_U32      wrrWeight;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid tcQueue */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            /* go over all valid profile */
            for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
                 profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
                 profile++)
            {
                /*
                    1.1. Call cpssPxPortTxSchedulerWrrProfileGet with valid tcQueue,
                         valid profile and valid pointer to wrrWeight.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxSchedulerWrrProfileGet(dev, profile, tcQueue, &wrrWeight);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profile, tcQueue, wrrWeight);
            }
        }

        /*
            1.2. Call with tcQueue[2], profile[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E]
                 and null-pointer to wrrWeight.
            Expected: GT_BAD_PTR.
        */
        tcQueue = 2;
        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        st = cpssPxPortTxSchedulerWrrProfileGet(dev, profile, tcQueue, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profile, tcQueue);

        /*
            1.3. Call with tcQueue[5], valid pointer to wrrWeight and wrong enum
                 values of profile.
            Expected: GT_BAD_PARAM.
        */
        tcQueue = 5;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxSchedulerWrrProfileGet(dev, profile, tcQueue, &wrrWeight),
                profile);

        /*
            1.4. Call with out of range tcQueue[CPSS_TC_RANGE_CNS],
                 valid pointer to wrrWeight and
                 profile[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = CPSS_TC_RANGE_CNS;
        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        st = cpssPxPortTxSchedulerWrrProfileGet(dev, profile, tcQueue, &wrrWeight);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tcQueue, wrrWeight);
    }

    tcQueue = 1;
    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerWrrProfileGet(dev, profile, tcQueue, &wrrWeight);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerWrrProfileGet(dev, profile, tcQueue, &wrrWeight);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerArbitrationGroupSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     arbGroup
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerArbitrationGroupSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerArbitrationGroupSet with all valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerArbitrationGroupGet with valid parameters.
         Expected: GT_OK and the same group as set.
    1.3. Call with out of range tcQueue[CPSS_TC_RANGE_CNS],
         valid arbGroup[CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E] and
         valid profileSet[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
         Expected: GT_BAD_PARAM.
    1.4. Call with valid tcQueue[5], wrong enum values arbGroup and valid
         profileSet[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
         Expected: GT_BAD_PARAM.
    1.5. Call with tcQueue[7],
         valid arbGroup[CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E] and
         wrong profileSet.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tcQueue;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     group;
    CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     groupGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid tcQueue values */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            /* go over all valid profiles */
            for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
                 profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
                 profile++)
            {
                /* go over all valid groups */
                for (group = CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E;
                     group <= CPSS_PX_PORT_TX_SCHEDULER_SP_ARB_GROUP_E;
                     group++)
                {
                    /*
                        1.1. Call cpssPxPortTxSchedulerArbitrationGroupSet with
                             all valid parameters.
                        Expected: GT_OK.
                    */
                    st = cpssPxPortTxSchedulerArbitrationGroupSet(dev,
                            profile, tcQueue, group);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profile,
                            tcQueue, group);

                    /*
                        1.2. Call cpssPxPortTxSchedulerArbitrationGroupGet with
                             the same params.
                        Expected: GT_OK and the same value of group.
                    */
                    st = cpssPxPortTxSchedulerArbitrationGroupGet(dev, profile,
                            tcQueue, &groupGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, tcQueue);

                    /* verify group */
                    UTF_VERIFY_EQUAL3_STRING_MAC(group, groupGet,
                            "got another group than was set: %d, %d, %d",
                            dev, tcQueue, profile);
                }
            }
        }

        /*
            1.3. Call with out of range tcQueue[CPSS_TC_RANGE_CNS = 8],
                 arbGroup[CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E],
                 profileSet[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = CPSS_TC_RANGE_CNS;
        group = CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E;
        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        st = cpssPxPortTxSchedulerArbitrationGroupSet(dev, profile, tcQueue, group);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tcQueue, group);

        /*
            1.4. Call with tcQueue [5],
                          wrong enum values arbGroup ,
                          profileSet [CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = 5;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxSchedulerArbitrationGroupSet(dev, profile, tcQueue, group),
                group);

        /*
            1.5. Call with tcQueue[7],
                 arbGroup [CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E],
                 wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        tcQueue = 7;
        group = CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxSchedulerArbitrationGroupSet(dev, profile, tcQueue, group),
                profile);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    tcQueue = 7;
    group = CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E;
    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_2_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerArbitrationGroupSet(dev, profile, tcQueue, group);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerArbitrationGroupSet(dev, profile, tcQueue, group);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerArbitrationGroupGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     *arbGroup
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerArbitrationGroupGet)
{
/*
    1. Go over all active devices.
    1.1. Call with valid parameters.
         Expected: GT_OK.
    1.2. Call with out of range tcQueue[CPSS_TC_RANGE_CNS] and
         valid profileSet[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
         Expected: GT_BAD_PARAM.
    1.3. Call with valid tcQueue[7] and wrong profileSet.
         Expected: GT_BAD_PARAM.
    1.4. Call with group[NULL]
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tcQueue;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     group;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid tcQueue values */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            /* go over all valid profiles */
            for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
                 profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
                 profile++)
            {
                /*
                    1.1. Call function with all valid params.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxSchedulerArbitrationGroupGet(dev, profile,
                        tcQueue, &group);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, tcQueue);
            }
        }

        /*
            1.2. Call with out of range tcQueue[CPSS_TC_RANGE_CNS = 8],
                 profileSet[CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E].
            Expected: GT_BAD_PARAM.
        */
        tcQueue = CPSS_TC_RANGE_CNS;
        profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

        st = cpssPxPortTxSchedulerArbitrationGroupGet(dev, profile, tcQueue, &group);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tcQueue);

        /*
            1.3. Call with tcQueue[7], wrong enum values profileSet.
            Expected: GT_BAD_PARAM.
        */
        tcQueue = 7;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxSchedulerArbitrationGroupGet(dev, profile, tcQueue, &group),
                profile);
        /*
            1.4. Call with group[NULL]
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxSchedulerArbitrationGroupGet(dev, profile, tcQueue, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profile, tcQueue);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    tcQueue = 7;
    group = CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E;
    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_2_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerArbitrationGroupGet(dev, profile, tcQueue, &group);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerArbitrationGroupGet(dev, profile, tcQueue, &group);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerProfileByteCountChangeEnableSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode,
    IN  CPSS_ADJUST_OPERATION_ENT                       bcOp
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerProfileByteCountChangeEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerProfileByteCountChangeEnableSet with valid
         parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerProfileByteCountChangeEnableGet.
         Expected: GT_OK and the same bcOp and bcMode as set.
    1.3. Call function with wrong bcMode value.
         Expected: GT_BAD_PARAM.
    1.4. Call function with wrong bcOp value.
         Expected: GT_BAD_PARAM.
    1.5. Call with out of bound profile value and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      i;
    CPSS_ADJUST_OPERATION_ENT                       bcOp;
    CPSS_ADJUST_OPERATION_ENT                       bcOpGet;
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode;
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcModeGet;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        /* go over all valid profiles */
        for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
             profile++)
        {
            /* go over all valid bcMode */
            for (bcMode = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_DISABLE_ALL_E;
                 bcMode <= CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E;
                 bcMode++)
            {
                /* go over all valid bcOp */
                for (i = 0; i <= 1; i++)
                {
                    bcOp = (i == 0)
                           ? CPSS_ADJUST_OPERATION_ADD_E
                           : CPSS_ADJUST_OPERATION_SUBSTRUCT_E;
                    /*
                        1.1. Call cpssPxPortTxSchedulerProfileByteCountChangeEnableSet
                             with valid parameters.
                        Expected: GT_OK.
                    */

                    st = cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(dev,
                            profile, bcMode, bcOp);
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                            "cpssPxPortTxSchedulerProfileByteCountChangeEnableSet: %d %d %d %d",
                            dev, profile, bcMode, bcOp);

                    /*
                        1.2. Call cpssPxPortTxSchedulerProfileByteCountChangeEnableGet
                        Expected: GT_OK and the same values.
                    */
                    st = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(dev,
                            profile, &bcModeGet, &bcOpGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssPxPortTxSchedulerProfileByteCountChangeEnableGet: %d %d",
                            dev, profile);

                    /* verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(bcMode, bcModeGet,
                               "got another bcValue then was set: %d %d", dev, profile);

                    /* verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(bcOp, bcOpGet,
                               "got another bcValue then was set: %d %d", dev, profile);
                }
            }

            /*
                1.3. Call function with wrong bcMode value.
                Expected: GT_BAD_PARAM.
            */
            bcOp = CPSS_ADJUST_OPERATION_ADD_E;

            UTF_ENUMS_CHECK_MAC(
                    cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(dev, profile,
                                                                         bcMode, bcOp),
                    bcMode);

            /*
                1.4. Call function with wrong bcOp value.
                Expected: GT_BAD_PARAM.
            */
            bcMode = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_DISABLE_ALL_E;

            UTF_ENUMS_CHECK_MAC(
                    cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(dev, profile,
                                                                         bcMode, bcOp),
                    bcOp);
        }

        /*
            1.5. Call with out of bound profile value and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        profile = 16;
        bcMode = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E;
        bcOp = CPSS_ADJUST_OPERATION_SUBSTRUCT_E;

        st = cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(dev, profile,
                                                                  bcMode, bcOp);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, bcMode, bcOp);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    profile = 0;
    bcMode = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E;
    bcOp = CPSS_ADJUST_OPERATION_SUBSTRUCT_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(dev, profile,
                                                                  bcMode, bcOp);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, profile,
                                    bcMode, bcOp);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(dev, profile, bcMode, bcOp);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, bcMode, bcOp);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerProfileByteCountChangeEnableGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  *bcMode,
    OUT CPSS_ADJUST_OPERATION_ENT                       *bcOp
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerProfileByteCountChangeEnableGet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerProfileByteCountChangeEnableGet with all
         valid parameters.
         Expected: GT_OK.
    1.2. Call function with wrong bcMode[NULL].
         Expected: GT_BAD_PTR.
    1.3. Call function with wrong bcOp[NULL].
         Expected: GT_BAD_PTR.
    1.4. Call with out of bound profile value and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_ADJUST_OPERATION_ENT                       bcOp;
    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        /* go over all valid profiles */
        for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
             profile++)
        {
            /*
                1.1. Call cpssPxPortTxSchedulerProfileByteCountChangeEnableGet
                Expected: GT_OK and the same values.
            */
            st = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(dev,
                    profile, &bcMode, &bcOp);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

            /*
                1.2. Call function with bcMode[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(dev,
                    profile, NULL, &bcOp);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profile);

            /*
                1.3. Call function with bcOp[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(dev,
                    profile, &bcMode, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profile);
        }

        /*
            1.4. Call with out of bound profile value and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        profile = 16;
        bcMode = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E;
        bcOp = CPSS_ADJUST_OPERATION_SUBSTRUCT_E;

        st = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(dev,
                profile, &bcMode, &bcOp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(dev, profile,
                &bcMode, &bcOp);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, profile,
                bcMode, bcOp);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(dev, profile,
            &bcMode, &bcOp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerShaperByteCountChangeValueSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  bcValue
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerShaperByteCountChangeValueSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call cpssPxPortTxSchedulerShaperByteCountChangeValueSet with valid
           parameters.
           Expected: GT_OK.
    1.1.2. Call cpssPxPortTxSchedulerShaperByteCountChangeValueGet with non-NULL
           pointer to bcValue and the same other parameters.
           Expected: GT_OK and the same bcValue.
    1.1.3. Call with out of range bcValue[64] and valid other parameters.
           Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For active device check function with out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For active device check function for CPU port number.
         Expected: GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32      bcValue;
    GT_U32      bcValueGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            for (bcValue = 0; bcValue < 64; bcValue++)
            {
                /*
                    1.1.1. Call with bcValue[0..63].
                    Expected: GT_OK.
                */
                st = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(dev, port,
                        bcValue);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, bcValue);

                /*
                    1.1.2. Call cpssPxPortTxSchedulerShaperByteCountChangeValueGet
                           with non-NULL pointers and other valid parameters.
                    Expected: GT_OK and the same values as set.
                */
                st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port,
                        &bcValueGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssPxPortTxSchedulerShaperByteCountChangeValueGet: %d", dev);

                /* verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(bcValue, bcValueGet,
                        "got another bcValue then was set: %d", dev);
            }

            /*
                1.1.3. Call with out of range bcValue[64] and other parameters
                       same as in 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            bcValue = 64;

            st = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(dev, port, bcValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, bcValue);
        }

        bcValue = 0;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(dev, port, bcValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value for port number.                    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(dev, port, bcValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /*      for CPU port number.                                */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(dev, port, bcValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    bcValue = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(dev, port, bcValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerShaperByteCountChangeValueSet(dev, port, bcValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerShaperByteCountChangeValueGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *bcValue
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerShaperByteCountChangeValueGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call function with valid pointer to bcValue.
           Expected: GT_OK.
    1.1.2. Call function with bcValue[NULL] and valid other parameters.
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For active device check function with out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For active device check function for CPU port number.
         Expected: GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = PORT_TX_VALID_PHY_PORT_CNS;
    GT_U32      bcValue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid pointer to bcValue.
                Expected: GT_OK.
            */
            st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port, &bcValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with  bcValue[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port, &bcValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value for port number.                    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port, &bcValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /*      for CPU port number.                                */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port, &bcValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    bcValue = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port, &bcValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerShaperByteCountChangeValueGet(dev, port, &bcValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerProfileCountModeSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerProfileCountModeSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerProfileCountModeSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerProfileCountModeGet with non-NULL pointer
         to wrrMode and the same other parameters.
         Expected: GT_OK and the same wrrMode.
    1.3. Call with out of range profile[16] and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrModeGet;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
             profile++)
        {
            for (wrrMode = CPSS_PX_PORT_TX_SCHEDULER_WRR_PACKET_MODE_E;
                 wrrMode <= CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E;
                 wrrMode++)
            {
                /*
                    1.1. Call cpssPxPortTxSchedulerProfileCountModeSet.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxSchedulerProfileCountModeSet(dev, profile, wrrMode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, wrrMode);

                /*
                    1.2. Call cpssPxPortTxSchedulerProfileCountModeGet.
                    Expected: GT_OK and the same values.
                */
                st = cpssPxPortTxSchedulerProfileCountModeGet(dev, profile, &wrrModeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssPxPortTxSchedulerProfileCountModeGet: %d %d",
                        dev, profile);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(wrrMode, wrrModeGet,
                        "got another mode then was set: %d %d", dev, profile);
            }
        }

        /*
            1.3. Call with out of range profile[16] and other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        profile = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) 16;
        wrrMode = CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E;

        st = cpssPxPortTxSchedulerProfileCountModeSet(dev, profile, wrrMode);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, wrrMode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    profile = 0;
    wrrMode = CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerProfileCountModeSet(dev, profile, wrrMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, profile, wrrMode);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerProfileCountModeSet(dev, profile, wrrMode);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, wrrMode);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxSchedulerProfileCountModeGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      *wrrMode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxSchedulerProfileCountModeGet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxSchedulerProfileCountModeGet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxSchedulerProfileCountModeGet with wrrMode[NULL].
         Expected: GT_BAD_PTR.
    1.3. Call with out of range profile[16] and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E;
             profile++)
        {
            /*
                1.1. Call function with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxSchedulerProfileCountModeGet(dev, profile, &wrrMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

            /*
                1.2. Call function with wrrMode[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxSchedulerProfileCountModeGet(dev, profile, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profile);
        }

        /*
            1.3. Call with out of range profile[16] and other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        profile = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) 16;

        st = cpssPxPortTxSchedulerProfileCountModeGet(dev, profile, &wrrMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    profile = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxSchedulerProfileCountModeGet(dev, profile, &wrrMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, profile);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxSchedulerProfileCountModeGet(dev, profile, &wrrMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profile);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortTxScheduler suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortTxScheduler)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerProfileIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerProfileIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerWrrMtuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerWrrMtuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerWrrProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerWrrProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerArbitrationGroupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerArbitrationGroupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerProfileByteCountChangeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerProfileByteCountChangeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerShaperByteCountChangeValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerShaperByteCountChangeValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerProfileCountModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxSchedulerProfileCountModeGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortTxScheduler)

