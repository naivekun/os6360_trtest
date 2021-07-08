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
* @file cpssPxPortTxTailDropUT.c
*
* @brief Unit tests for cpssPxPortTxTailDrop APIs
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC


#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>


/* Valid port num value used for testing */
#define PORT_TX_VALID_PHY_PORT_CNS  0


/* used by cpssPxPortTxTailDropProfileSet UT */
typedef struct
{
    GT_U32                                   portMaxBuffLimit;
    GT_U32                                   portMaxDescLimit;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT  portAlpha;
} PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TEST_VALUE_STC;

/* used by cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet UT */
typedef struct
{
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    GT_U32                                      dp;
    GT_U32                                      tc;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  enablers;
} PRV_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_TEST_VALUE_STC;

/* used by cpssPxPortTxTailDropProfileTcSharingSet UT */
typedef struct
{
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile;
    GT_U32                                          tc;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    mode;
    GT_U32                                          poolNum;
} PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_SHARING_VALUE_STC;

/* used by cpssPxPortTxTailDropProfileTcSet UT */
typedef struct
{
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    GT_U32                                      tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    params;
} PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_VALUE_STC;


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet
);
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileIdSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call cpssPxPortTxTailDropProfileIdSet with valid profile.
           Expected: GT_OK.
    1.1.2. Call cpssPxPortTxTailDropProfileIdGet with non-NULL profileGet and
           other parameters from 1.1.1
           Expected: GT_OK and same profileSetGet as was set.
    1.1.3. Call with wrong enum values profile.
           Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function for out of bound value for port
         number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function for CPU port number.
         Expected: GT_OK.
    2. Go over all not-active devices and devices from non-applicable family
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT profile;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT profileGet;

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
            for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
                 profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
                 profile++)
            {
                /*
                    1.1.1. Call cpssPxPortTxTailDropProfileIdSet with valid profile.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxTailDropProfileIdSet(dev, port, profile);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profile);

                /*
                    1.1.2. Call cpssPxPortTxTailDropProfileIdGet with non-NULL
                           profileGet and other parameters from 1.1.1
                    Expected: GT_OK and same profileSetGet as was set.
                */
                st = cpssPxPortTxTailDropProfileIdGet(dev, port, &profileGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssPxPortTxTailDropProfileIdGet: %d", dev, port);

                /* verify returned profile */
                UTF_VERIFY_EQUAL2_STRING_MAC(profile, profileGet,
                        "got another profile than was set: %d", dev, port);
            }

            /*
                1.1.3. Call with wrong enum values profile.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortTxTailDropProfileIdSet(dev, port, profile),
                                profile);
        }

        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxTailDropProfileIdSet(dev, port, profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxTailDropProfileIdSet(dev, port, profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxTailDropProfileIdSet(dev, port, profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
    port    = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileIdSet(dev, port, profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileIdSet(dev, port, profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   *profileSetPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileIdGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call cpssPxPortTxTailDropProfileIdGet with valid pointer to profile.
           Expected: GT_OK.
    1.1.2. Call cpssPxPortTxTailDropProfileIdGet with NULL pointer.
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function for out of bound value for port
         number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function for CPU port number.
         Expected: GT_OK.
    2. Go over all not-active devices and devices from non-applicable family
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT profile;

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
                1.1.1. Call cpssPxPortTxTailDropProfileIdGet with non-NULL
                       pointer to profile.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropProfileIdGet(dev, port, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxPortTxTailDropProfileIdGet with NULL pointer.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropProfileIdGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxTailDropProfileIdGet(dev, port, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value for port number.                    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxTailDropProfileIdGet(dev, port, &profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /*      for CPU port number.                                       */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxTailDropProfileIdGet(dev, port, &profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns  */
    /*    GT_NOT_APPLICABLE_DEVICE. */
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileIdGet(dev, port, &profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileIdGet(dev, port, &profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropUcEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropUcEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Get current tail drop enable state.
         Expected: GT_OK.
    1.2. Inverse tail drop enable state.
         Expected: GT_OK.
    1.3. Get new tail drop enable state.
         Expected: GT_OK and the same tail drop enable state as was set.
    1.4. Restore tail drop enable state.
         Expected: GT_OK.
    1.5. Check restored tail drop enable state.
         Expected: GT_OK and the same tail drop enable state as was set.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Get current tail drop enable state.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropUcEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Inverse tail drop enable state.
            Expected: GT_OK.
        */
        enable = (enableGet == GT_TRUE) ? GT_FALSE : GT_TRUE;

        st = cpssPxPortTxTailDropUcEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.3. Get new tail drop enable state.
            Expected: GT_OK and the same enable state as was set.
        */
        st = cpssPxPortTxTailDropUcEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify returned profile */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another tail drop enable state as was set: %d", dev);

        /*
            1.4. Restore tail drop enable state.
            Expected: GT_OK.
        */
        enable = (enableGet == GT_TRUE) ? GT_FALSE : GT_TRUE;

        st = cpssPxPortTxTailDropUcEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.5. Check restored tail drop enable state.
            Expected: GT_OK and the same enable state as was set.
        */
        st = cpssPxPortTxTailDropUcEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify returned profile */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another tail drop enable state as was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns  */
    /*    GT_NOT_APPLICABLE_DEVICE.                           */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropUcEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropUcEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropUcEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropUcEnableGet)
{
/*
    1. Go over all active devices.
    1.1. Call function with valid pointer to tail drop enable parameter.
         Expected: GT_OK.
    1.2. Call function with enable[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid pointer to tail drop enable parameter.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropUcEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with enable[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropUcEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns  */
    /*    GT_NOT_APPLICABLE_DEVICE.                           */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropUcEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropUcEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      portMaxBuffLimit,
    IN  GT_U32                                      portMaxDescLimit,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropProfileSet with valid profile, valid maxBuffLimit
         and valid maxDescLimit.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropProfileGet with valid parameters.
         Expected: GT_OK and the same values as was set.
    1.3. Call with wrong profile and valid maxBuffLimit, maxDescLimit, portAlpha.
         Expected: GT_BAD_PARAM.
    1.4. Call with valid profile, maxDescLimit, portAlpha and wrong maxBuffLimit.
         Expected: GT_OUT_OF_RANGE.
    1.5. Call with valid profile, maxBuffLimit, portAlpha and wrong maxDescLimit.
         Expected: GT_OUT_OF_RANGE.
    1.6. Call with valid profile, maxBuffLimit, maxDescLimit and wrong portAlpha.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     isEqual;
    GT_U32      maxBuffLimit;
    GT_U32      maxBuffLimitGet;
    GT_U32      maxDescLimit;
    GT_U32      maxDescLimitGet;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlphaGet;

    /* Different valid combinations of maxBuffLimit and maxDescLimit */
    const PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TEST_VALUE_STC validCombination[] = {
        /* portMaxBuffLimit,  portMaxDescLimit,  portAlpha */
        { 0,         0,       CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E },
        { 0,         0,       CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E },
        { 0,         0x1234,  CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E },
        { 0x1234,    0,       CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E },
        { 0,         0,       CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E },
        { 0,         0xFFFF,  CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E },
        { 0x0FFFFF,  0,       CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E },
        { 0x0FFFFF,  0xFFFF,  CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E }
    };
    GT_U32  countOfValidCombinations =
                        sizeof(validCombination) / sizeof(validCombination[0]);
    GT_U32  i;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid "profile" */
        for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
             profile++)
        {
            /* Call API with different valid combinations of */
            /* maxBuffLimit and maxDescLimit                 */
            for (i = 0; i < countOfValidCombinations; i++)
            {
                maxBuffLimit = validCombination[i].portMaxBuffLimit;
                maxDescLimit = validCombination[i].portMaxDescLimit;
                portAlpha    = validCombination[i].portAlpha;

                /*
                    1.1. Call with valid profile, valid maxBuffLimit and
                         valid maxDescLimit.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxTailDropProfileSet(dev, profile, maxBuffLimit,
                        maxDescLimit, portAlpha);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, profile,
                        maxBuffLimit, maxDescLimit, portAlpha);

                /*
                    1.2. Call cpssPxPortTxTailDropProfileGet.
                    Expected: GT_OK and the same values as was set.
                */
                st = cpssPxPortTxTailDropProfileGet(dev, profile,
                        &maxBuffLimitGet, &maxDescLimitGet, &portAlphaGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssPxPortTxTailDropProfileGet: %d", dev);

                /* verify variables */
                isEqual = ((maxBuffLimit == maxBuffLimitGet) &&
                           (maxDescLimit == maxDescLimitGet) &&
                           (portAlpha == portAlphaGet)) ? GT_TRUE : GT_FALSE;

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                        "got another values than was set: %d", dev);
            }
        }

        /*
            1.3. Call with wrong profile, valid maxBuffLimit, maxDescLimit and
                 portAlpha.
            Expected: GT_BAD_PARAM.
        */
        maxBuffLimit = 123;
        maxDescLimit = 234;
        portAlpha    = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileSet(dev, profile, maxBuffLimit,
                                               maxDescLimit, portAlpha),
                profile);

        /*
            1.4. Call with valid profile, maxDescLimit, portAlpha and wrong
                 maxBuffLimit.
            Expected: GT_OUT_OF_RANGE.
        */
        profile      = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        maxBuffLimit = 0x100000;  /* 0x0FFFFF + 1 */
        maxDescLimit = 234;
        portAlpha    = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        st = cpssPxPortTxTailDropProfileSet(dev, profile, maxBuffLimit,
                maxDescLimit, portAlpha);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile,
                maxBuffLimit, maxDescLimit, portAlpha);

        /*
            1.5. Call with valid profile, maxBuffLimit, portAlpha and wrong
                 maxDescLimit.
            Expected: GT_OUT_OF_RANGE.
        */
        profile      = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        maxBuffLimit = 123;
        maxDescLimit = 0x10000;   /* 0xFFFF + 1 */
        portAlpha    = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        st = cpssPxPortTxTailDropProfileSet(dev, profile, maxBuffLimit,
                maxDescLimit, portAlpha);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile,
                maxBuffLimit, maxDescLimit, portAlpha);

        /*
            1.6. Call with valid profile, maxBuffLimit, maxDescLimit and wrong
                 portAlpha.
            Expected: GT_BAD_PARAM.
        */
        profile      = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        maxBuffLimit = 123;
        maxDescLimit = 234;
        portAlpha    = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) 6; /* wrong */

        st = cpssPxPortTxTailDropProfileSet(dev, profile, maxBuffLimit,
                maxDescLimit, portAlpha);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, profile,
                maxBuffLimit, maxDescLimit, portAlpha);
    }

    /* restore  */
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
    maxBuffLimit = 123;
    maxDescLimit = 234;
    portAlpha    = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileSet(dev, profile, maxBuffLimit,
                maxDescLimit, portAlpha);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileSet(dev, profile, maxBuffLimit,
            maxDescLimit, portAlpha);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    OUT GT_U32                                      *portMaxBuffLimitPtr,
    OUT GT_U32                                      *portMaxDescLimitPtr,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     *portAlphaPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with all valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to portMaxBuffLimit[NULL] and valid other
         parameters.
         Expected: GT_BAD_PTR.
    1.3. Call API with wrong pointer to portMaxDescLimit[NULL] and valid other
         parameters.
         Expected: GT_BAD_PTR.
    1.4. Call API with wrong pointer to portAlpha[NULL] and valid other
         parameters.
         Expected: GT_BAD_PTR.
    1.5. Call with wrong profile and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      maxBuffLimit;
    GT_U32      maxDescLimit;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid "profile" */
        for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
             profile++)
        {
            /*
                1.1. Call API with all valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropProfileGet(dev, profile, &maxBuffLimit,
                    &maxDescLimit, &portAlpha);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

            /*
                1.2. Call API with wrong pointer to portMaxBuffLimit[NULL] and
                     valid other parameters.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropProfileGet(dev, profile, NULL,
                    &maxDescLimit, &portAlpha);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profile);

            /*
                1.3. Call API with wrong pointer to portMaxDescLimit[NULL] and
                     valid other parameters.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropProfileGet(dev, profile, &maxBuffLimit,
                    NULL, &portAlpha);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profile);

            /*
                1.4. Call API with wrong pointer to portAlpha[NULL] and valid
                     other parameters.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropProfileGet(dev, profile, &maxBuffLimit,
                    &maxDescLimit, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profile);
        }

        /*
            1.5. Call with wrong profile, valid pointer to maxBuffLimit and
                 valid pointer to maxDescLimit.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileGet(dev, profile, &maxBuffLimit,
                                               &maxDescLimit, &portAlpha),
                profile);
    }

    /* restore valid parameters value */
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileGet(dev, profile, &maxBuffLimit,
                &maxDescLimit, &portAlpha);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileGet(dev, profile, &maxBuffLimit,
            &maxDescLimit, &portAlpha);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropPacketModeLengthSet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          length
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropPacketModeLengthSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropPacketModeLengthSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropPacketModeLengthGet.
         Expected: GT_OK and the same values as was set.
    1.3. Call with out of range value of length[64].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      length;
    GT_U32      lengthGet;

    /* valid values of length */
    const GT_U32 validLength[] = {
        0, 24, 33, 54, 63
    };
    GT_U32 countOfValidLength = sizeof(validLength) / sizeof(validLength[0]);
    GT_U32 validLengthIndex;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (validLengthIndex = 0;
             validLengthIndex < countOfValidLength;
             validLengthIndex++)
        {
            length = validLength[validLengthIndex];

            /*
                1.1. Call cpssPxPortTxTailDropPacketModeLengthSet with valid
                     parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropPacketModeLengthSet(dev, length);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPortTxTailDropPacketModeLengthGet with valid
                     pointer to length.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortTxTailDropPacketModeLengthGet(dev, &lengthGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropPacketModeLengthGet: %d", dev);

            /* verify values */
            UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                    "got another length then was set: %d", dev);
        }

        /*
            1.3. Call with out of range value of length[64].
            Expected: GT_OUT_OF_RANGE.
        */
        length = 64;

        st = cpssPxPortTxTailDropPacketModeLengthSet(dev, length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* restore correct value */
    length = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropPacketModeLengthSet(dev, length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropPacketModeLengthSet(dev, length);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropPacketModeLengthGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *lengthPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropPacketModeLengthGet)
{
/*
    1. Go over all active devices.
    1.1. Call function with valid pointer to length.
         Expected: GT_OK.
    1.2. Call function with wrong pointer to length[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      length;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid pointer to length.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropPacketModeLengthGet(dev, &length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with wrong pointer to length[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropPacketModeLengthGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropPacketModeLengthGet(dev, &length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropPacketModeLengthGet(dev, &length);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileBufferConsumptionModeSet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileBufferConsumptionModeSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropProfileBufferConsumptionModeSet with valid
         parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropProfileBufferConsumptionModeGet.
         Expected: GT_OK and the same parameters as was set.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode;
    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   modeGet;

    /* valid parameters */
    const CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT validMode[] = {
        CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E,
        CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E
    };
    GT_U32 countOfValidMode = sizeof(validMode) / sizeof(validMode[0]);
    GT_U32 validModeIndex;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
             profile++)
        {
            for (validModeIndex = 0;
                 validModeIndex < countOfValidMode;
                 validModeIndex++)
            {
                mode = validMode[validModeIndex];

                /*
                    1.1. Call cpssPxPortTxTailDropProfileBufferConsumptionModeSet
                         with valid parameters.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxTailDropProfileBufferConsumptionModeSet(dev,
                        profile, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, mode);

                /*
                    1.2. Call cpssPxPortTxTailDropProfileBufferConsumptionModeGet.
                    Expected: GT_OK and the same value as was set.
                */
                st = cpssPxPortTxTailDropProfileBufferConsumptionModeGet(dev,
                        profile, &modeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssPxPortTxTailDropProfileBufferConsumptionModeGet: %d %d",
                        dev, profile);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                        "got another mode then was set: %d %d", dev, profile);
            }

            /*
                1.3. Call with valid profile and wrong mode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(
                    cpssPxPortTxTailDropProfileBufferConsumptionModeSet(dev, profile, mode),
                    mode);
        }

        /*
            1.4. Call with wrong value of profile and valid mode.
            Expected: GT_BAD_PARAM.
        */
        mode = CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileBufferConsumptionModeSet(dev, profile, mode),
                profile);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
    mode = CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileBufferConsumptionModeSet(dev, profile, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, profile, mode);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileBufferConsumptionModeSet(dev, profile, mode);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, mode);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileBufferConsumptionModeGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileBufferConsumptionModeGet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropProfileBufferConsumptionModeGet
         with valid pointer to mode.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropProfileBufferConsumptionModeGet
         with wrong pointer to mode[NULL].
         Expected: GT_BAD_PTR.
    1.3. Call with wrong value of profile and valid pointer to mode.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile;
    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
             profile++)
        {
            /*
                1.1. Call cpssPxPortTxTailDropProfileBufferConsumptionModeGet
                     with valid pointer to mode.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropProfileBufferConsumptionModeGet(dev, profile, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profile);

            /*
                1.2. Call cpssPxPortTxTailDropProfileBufferConsumptionModeGet
                     with wrong pointer to mode[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropProfileBufferConsumptionModeGet(dev, profile, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, profile);
        }

        /*
            1.3. Call with wrong value of profile and valid pointer to mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileBufferConsumptionModeGet(dev, profile, &mode),
                profile);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileBufferConsumptionModeGet(dev, profile, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, profile);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileBufferConsumptionModeGet(dev, profile, &mode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, profile);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMaskSharedBuffEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMaskSharedBuffEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Get current shared buffer indication masking state.
         Expected: GT_OK.
    1.2. Invert shared buffer indication masking state.
         Expected: GT_OK.
    1.3. Verify changes.
         Expected: GT_OK and the same "enable" as was set.
    1.4. Restore shared buffer indication masking state.
         Expected: GT_OK.
    1.5. Verify changes.
         Expected: GT_OK and the same "enable" as was set.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Get current shared buffer indication masking state.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropMaskSharedBuffEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Invert shared buffer indication masking state.
            Expected: GT_OK.
        */
        enable = (enableGet == GT_TRUE) ? GT_FALSE : GT_TRUE;

        st = cpssPxPortTxTailDropMaskSharedBuffEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.3. Verify changes.
            Expected: GT_OK and the same "enable" as was set.
        */
        st = cpssPxPortTxTailDropMaskSharedBuffEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d", dev);

        /*
            1.4. Restore shared buffer indication masking state.
            Expected: GT_OK.
        */
        enable = (enable == GT_TRUE) ? GT_FALSE : GT_TRUE;

        st = cpssPxPortTxTailDropMaskSharedBuffEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.5. Verify changes.
            Expected: GT_OK and the same "enable" as was set.
        */
        st = cpssPxPortTxTailDropMaskSharedBuffEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMaskSharedBuffEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, enable);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMaskSharedBuffEnableSet(dev, enable);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, enable);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMaskSharedBuffEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMaskSharedBuffEnableGet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropMaskSharedBuffEnableGet with valid pointer to
         enable.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropMaskSharedBuffEnableGet with wrong pointer to
         enable[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssPxPortTxTailDropMaskSharedBuffEnableGet with valid
                 pointer to enable.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropMaskSharedBuffEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortTxTailDropMaskSharedBuffEnableGet with wrong
                 pointer to enable[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropMaskSharedBuffEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMaskSharedBuffEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMaskSharedBuffEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedBuffMaxLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          maxSharedBufferLimit
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedBuffMaxLimitSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropSharedBuffMaxLimitSet with all valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropSharedBuffMaxLimitGet.
         Expected: GT_OK and the same limit as was set.
    1.3. Call cpssPxPortTxTailDropSharedBuffMaxLimitSet with out of range value
         of maxSharedBufferLimit.
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      maxSharedBufferLimit;
    GT_U32      maxSharedBufferLimitGet;

    /* valid maxSharedBufferLimit-s */
    const GT_U32  validMaxSharedBufferLimit[] = {
        0x0000, 0x0345, 0x1231, 0xFFFF
    };
    GT_U32  validValueIndex;
    GT_U32  countOfValidValues =
        sizeof(validMaxSharedBufferLimit) / sizeof(validMaxSharedBufferLimit[0]);


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        for (validValueIndex = 0;
             validValueIndex < countOfValidValues;
             validValueIndex++)
        {
            maxSharedBufferLimit = validMaxSharedBufferLimit[validValueIndex];

            /*
                1.1. Call cpssPxPortTxTailDropSharedBuffMaxLimitSet with valid
                     parameters.
                Expected: GT_OK
            */
            st = cpssPxPortTxTailDropSharedBuffMaxLimitSet(dev, maxSharedBufferLimit);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, maxSharedBufferLimit);

            /*
                1.2. Call cpssPxPortTxTailDropSharedBuffMaxLimitGet
                Expected: GT_OK and the same values.
            */
            st = cpssPxPortTxTailDropSharedBuffMaxLimitGet(dev, &maxSharedBufferLimitGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropSharedBuffMaxLimitGet: %d", dev);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(maxSharedBufferLimit, maxSharedBufferLimitGet,
                    "got another limit then was set: %d", dev);
        }


        /*
            1.3. Call cpssPxPortTxTailDropSharedBuffMaxLimitSet with out of range
                 value of maxSharedBufferLimit.
            Expected: GT_OUT_OF_RANGE.
        */
        maxSharedBufferLimit = 0x10000;

        st = cpssPxPortTxTailDropSharedBuffMaxLimitSet(dev, maxSharedBufferLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, maxSharedBufferLimit);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */
    maxSharedBufferLimit = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedBuffMaxLimitSet(dev, maxSharedBufferLimit);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, maxSharedBufferLimit);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedBuffMaxLimitSet(dev, maxSharedBufferLimit);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, maxSharedBufferLimit);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedBuffMaxLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *maxSharedBufferLimitPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedBuffMaxLimitGet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropSharedBuffMaxLimitGet with valid pointer to
         maxSharedBufferLimit.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropSharedBuffMaxLimitGet with wrong pointer to
         maxSharedBufferLimit.
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      maxSharedBufferLimit;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        /*
            1.1. Call cpssPxPortTxTailDropSharedBuffMaxLimitGet with valid
                 pointer to maxSharedBufferLimit.
            Expected: GT_OK
        */
        st = cpssPxPortTxTailDropSharedBuffMaxLimitGet(dev, &maxSharedBufferLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortTxTailDropSharedBuffMaxLimitGet with wrong
                 pointer to maxSharedBufferLimit[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropSharedBuffMaxLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedBuffMaxLimitGet(dev, &maxSharedBufferLimit);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedBuffMaxLimitGet(dev, &maxSharedBufferLimit);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet with
         valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet with
         the same parameters.
         Expected: GT_OK and the same values than was set.
    1.3. Call API with wrong profile values.
         Expected: GT_BAD_PARAM.
    1.4. Call API with wrong dp[3].
         Expected: GT_BAD_PARAM.
    1.5. Call API with wrong enablersPtr[NULL].
         Expected: GT_BAD_PTR.
    1.6. Call API with wrong tc[8].
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_BOOL     isEqual;
    GT_U8       dev;
    GT_U32      dp;
    GT_U32      tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  enablers;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  enablersGet;

    /* Valid values */
    const PRV_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_TEST_VALUE_STC validValue[] = {
        /*
            profile,
            dp, tc, enablers{ tcDpLimit, portLimit, tcLimit, sharedPoolLimit }
        */
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E,
            0, 0, { GT_TRUE, GT_TRUE, GT_TRUE, GT_TRUE }
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_2_E,
            1, 1, { GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE }
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_4_E,
            2, 3, { GT_FALSE, GT_TRUE, GT_FALSE, GT_TRUE }
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_8_E,
            0, 7, { GT_TRUE, GT_FALSE, GT_TRUE, GT_FALSE }
        }
    };
    GT_U32  countOfValidValues = sizeof(validValue) / sizeof(validValue[0]);
    GT_U32  i;


    /* clear enablers */
    cpssOsBzero((GT_VOID*) &enablers, sizeof(enablers));
    cpssOsBzero((GT_VOID*) &enablersGet, sizeof(enablersGet));


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over some valid test parameters combinations */
        for (i = 0; i < countOfValidValues; i++)
        {
            profile = validValue[i].profile;
            dp      = validValue[i].dp;
            tc      = validValue[i].tc;
            enablers.tcDpLimit       = validValue[i].enablers.tcDpLimit;
            enablers.portLimit       = validValue[i].enablers.portLimit;
            enablers.tcLimit         = validValue[i].enablers.tcLimit;
            enablers.sharedPoolLimit = validValue[i].enablers.sharedPoolLimit;

            /*
                1.1. Call cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet
                     with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(dev,
                    profile, dp, tc, &enablers);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet
                     with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
                    profile, dp, tc, &enablersGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet: %d", dev);

            /* verify values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &enablers, (GT_VOID*) &enablersGet,
                                         sizeof(enablers)))
                      ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                    "get another enablers than was set: %d", dev);
        }

        /*
            1.3. Call API with wrong profile values.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(dev,
                        profile, dp, tc, &enablers),
                profile);

        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

        /*
            1.4. Call API with wrong dp[3].
            Expected: GT_BAD_PARAM.
        */
        dp = 3;

        st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(dev,
                profile, dp, tc, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        dp = 0;

        /*
            1.5. Call API with wrong enablersPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(dev,
                profile, dp, tc, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "enablersPtr is NULL", dev);

        /*
            1.6. Call API with wrong tc[8].
            Expected: GT_BAD_PARAM.
        */
        tc = 8;

        st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(dev,
                profile, dp, tc, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        tc = 0;
    }

    /* restore correct values */
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_1_E;
    dp      = 0;
    tc      = 0;
    enablers.tcDpLimit       = GT_TRUE;
    enablers.portLimit       = GT_TRUE;
    enablers.tcLimit         = GT_TRUE;
    enablers.sharedPoolLimit = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(dev,
                profile, dp, tc, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(dev,
            profile, dp, tc, &enablers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters and valid pointer to enablers.
         Expected: GT_OK.
    1.2. Call API with valid parameters and wrong pointer to enablers[NULL].
         Expected: GT_BAD_PTR.
    1.3. Call API with wrong profile values.
         Expected: GT_BAD_PARAM.
    1.4. Call API with wrong dp[3].
         Expected: GT_BAD_PARAM.
    1.5. Call API with wrong tc[8].
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      dp;
    GT_U32      tc;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  enablers;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid profiles in step of 5 */
        for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
             profile += 5)
        {
            /* go over all valid traffic class in step of 2 */
            for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc += 2)
            {
                /* go over all valid drop precedence values */
                for (dp = 0; dp <= 2; dp++)
                {
                    /*
                        1.1. Call API with valid parameters and valid pointer to
                             enablers.
                        Expected: GT_OK.
                    */
                    st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
                            profile, dp, tc, &enablers);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profile, dp, tc);

                    /*
                        1.2. Call API with valid parameters and wrong pointer to
                             enablers[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
                            profile, dp, tc, NULL);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, profile, dp, tc);
                }
            }
        }

        /*
            1.3. Call API with wrong profile values.
            Expected: GT_BAD_PARAM.
        */
        dp = 0;
        tc = 0;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
                        profile, dp, tc, &enablers),
                profile);

        /*
            1.4. Call API with wrong dp[3].
            Expected: GT_BAD_PARAM.
        */
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        dp = 3;
        tc = 0;

        st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
                profile, dp, tc, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.5. Call API with wrong tc[8].
            Expected: GT_BAD_PARAM.
        */
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        dp = 0;
        tc = 8;

        st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
                profile, dp, tc, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* restore correct values */
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_1_E;
    dp = 0;
    tc = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
                profile, dp, tc, &enablers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(dev,
            profile, dp, tc, &enablers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropWrtdMasksSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropWrtdMasksSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropWrtdMasksSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropWrtdMasksGet with the same parameters.
         Expected: GT_OK and the same values than was set.
    1.3. Call API with out of range value of maskLsb.tcDp[21] and othen valid
         parameters.
         Expected: GT_OUT_OF_RANGE.
    1.4. Call API with out of range value of maskLsb.port[21] and othen valid
         parameters.
         Expected: GT_OUT_OF_RANGE.
    1.5. Call API with out of range value of maskLsb.tc[21] and othen valid
         parameters.
         Expected: GT_OUT_OF_RANGE.
    1.6. Call API with out of range value of maskLsb.pool[21] and othen valid
         parameters.
         Expected: GT_OUT_OF_RANGE.
    1.7. Call API with wrong maskLsbPtr[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     isEqual;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC  maskLsb;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC  maskLsbGet;

    /* valid combinations */
    const GT_U32 validCombination[][4] = {
        {  0,  0,  0,  0 },
        {  3,  4,  5,  6 },
        { 20,  0,  0,  0 },
        {  0, 20,  0,  0 },
        {  0,  0, 20,  0 },
        {  0,  0,  0, 20 },
        { 20, 20, 20, 20 }
    };
    GT_U32  countOfValidCombinations =
                        sizeof(validCombination) / sizeof(validCombination[0]);
    GT_U32  i;


    /* clear maskLsb */
    cpssOsBzero((GT_VOID*) &maskLsb, sizeof(maskLsb));
    cpssOsBzero((GT_VOID*) &maskLsbGet, sizeof(maskLsbGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidCombinations; i++)
        {
            maskLsb.tcDp = validCombination[i][0];
            maskLsb.port = validCombination[i][1];
            maskLsb.tc   = validCombination[i][2];
            maskLsb.pool = validCombination[i][3];

            /*
                1.1. Call cpssPxPortTxTailDropWrtdMasksSet with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropWrtdMasksSet(dev, &maskLsb);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPortTxTailDropWrtdMasksGet with the same
                     parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortTxTailDropWrtdMasksGet(dev, &maskLsbGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropWrtdMasksGet: %d ", dev);

            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &maskLsb,
                                         (GT_VOID*) &maskLsbGet,
                                         sizeof(maskLsb)))
                      ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                    "got another enablers than was set: %d", dev);
        }

        /*
            1.3. Call API with out of range value of maskLsb.tcDp[21] and othen
                 valid parameters.
            Expected: GT_OUT_OF_RANGE.
        */
        maskLsb.tcDp = 21;
        maskLsb.port = 0;
        maskLsb.tc   = 0;
        maskLsb.pool = 0;

        st = cpssPxPortTxTailDropWrtdMasksSet(dev, &maskLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.4. Call API with out of range value of maskLsb.port[21] and othen
                 valid parameters.
            Expected: GT_OUT_OF_RANGE.
        */
        maskLsb.tcDp = 0;
        maskLsb.port = 21;
        maskLsb.tc   = 0;
        maskLsb.pool = 0;

        st = cpssPxPortTxTailDropWrtdMasksSet(dev, &maskLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.5. Call API with out of range value of maskLsb.tc[21] and othen
                 valid parameters.
            Expected: GT_OUT_OF_RANGE.
        */
        maskLsb.tcDp = 0;
        maskLsb.port = 0;
        maskLsb.tc   = 21;
        maskLsb.pool = 0;

        st = cpssPxPortTxTailDropWrtdMasksSet(dev, &maskLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.6. Call API with out of range value of maskLsb.pool[21] and othen
                 valid parameters.
            Expected: GT_OUT_OF_RANGE.
        */
        maskLsb.tcDp = 0;
        maskLsb.port = 0;
        maskLsb.tc   = 0;
        maskLsb.pool = 21;

        st = cpssPxPortTxTailDropWrtdMasksSet(dev, &maskLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.7. Call API with wrong maskLsbPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropWrtdMasksSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "maskLsbPtr is NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropWrtdMasksSet(dev, &maskLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropWrtdMasksSet(dev, &maskLsb);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropWrtdMasksGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropWrtdMasksGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to maskLsb[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC  maskLsb;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropWrtdMasksGet(dev, &maskLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong pointer to maskLsb[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropWrtdMasksGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropWrtdMasksGet(dev, &maskLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropWrtdMasksGet(dev, &maskLsb);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileTcSharingSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    enableMode,
    IN  GT_U32                                          poolNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileTcSharingSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropProfileTcSharingSet with all valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropProfileTcSharingGet with the same valid
         parameters.
         Expected: GT_OK and the same parameters as was set.
    1.3. Call API with wrong value of profile and valid other parameters.
         Expected: GT_BAD_PARAM.
    1.4. Call API with wrong tc[8] and valid other parameters.
         Expected: GT_BAD_PARAM.
    1.5. Call API with wrong enableMode and valid other parameters.
         Expected: GT_BAD_PARAM.
    1.6. Call API with wrong poolNum[8] and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tc;
    GT_U32      poolNum;
    GT_U32      poolNumGet;
    GT_BOOL     isEqual;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    mode;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    modeGet;

    /* valid values */
    const PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_SHARING_VALUE_STC validValue[] = {
        /*
            profile, tc,
            mode, poolNum
        */
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E, 0,
            CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E, 0
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_4_E, 2,
            CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E, 1
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E, 4,
            CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E, 3
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_9_E, 6,
            CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E, 5
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E, 7,
            CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E, 7
        }
    };
    GT_U32  countOfValidValues = sizeof(validValue) / sizeof(validValue[0]);
    GT_U32  i;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidValues; i++)
        {
            profile = validValue[i].profile;
            tc      = validValue[i].tc;
            mode    = validValue[i].mode;
            poolNum = validValue[i].poolNum;

            /*
                1.1. Call cpssPxPortTxTailDropProfileTcSharingSet with all valid
                     parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropProfileTcSharingSet(dev, profile, tc, mode,
                                                         poolNum);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, profile, tc, mode, poolNum);

            /*
                1.2. Call cpssPxPortTxTailDropProfileTcSharingGet with the same
                     valid parameters.
                Expected: GT_OK and the same parameters as was set.
            */
            st = cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc,
                                                         &modeGet, &poolNumGet);
            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropProfileTcSharingGet: %d %d %d %d %d",
                    dev, profile, tc, mode, poolNum);

            /* verify values */
            isEqual = ((mode == modeGet) && (poolNum == poolNumGet))
                      ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value that was set: %d %d %d", dev, profile, tc);
        }

        /*
            1.3. Call API with wrong value of profile and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileTcSharingSet(dev, profile, tc, mode, poolNum),
                profile);

        /*
            1.4. Call API with wrong tc[8] and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        tc      = CPSS_TC_RANGE_CNS;
        mode    = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E;
        poolNum = 0;

        st = cpssPxPortTxTailDropProfileTcSharingSet(dev, profile, tc, mode,
                                                     poolNum);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc, mode,
                                    poolNum);

        /*
            1.5. Call API with wrong enableMode and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileTcSharingSet(dev, profile, tc, mode, poolNum),
                mode);

        /*
            1.6. Call API with wrong poolNum[8] and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_8_E;
        tc      = 2;
        mode    = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E;
        poolNum = 8;

        st = cpssPxPortTxTailDropProfileTcSharingSet(dev, profile, tc, mode,
                                                     poolNum);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc, mode,
                                    poolNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileTcSharingSet(dev, profile, tc, mode, poolNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileTcSharingSet(dev, profile, tc, mode, poolNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileTcSharingGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    *enableModePtr,
    IN  GT_U32                                          *poolNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileTcSharingGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with all valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong enableModePtr[NULL] and other valid parameters.
         Expected: GT_BAD_PTR.
    1.3. Call API with wrong poolNumPtr[NULL] and other valid parameters.
         Expected: GT_BAD_PTR.
    1.4. Call API with wrong value of profile and valid other parameters.
         Expected: GT_BAD_PARAM.
    1.5. Call API with wrong tc[8] and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tc;
    GT_U32      poolNum;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    mode;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over valid profiles in step of 5 */
        for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
             profile += 5)
        {
            /* go over valid tc in step of 2 */
            for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc += 2)
            {
                /*
                    1.1. Call API with all valid parameters.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc,
                                                             &mode, &poolNum);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, tc);

                /*
                    1.2. Call API with wrong enableModePtr[NULL] and valid other
                         parameters.
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc,
                                                             NULL, &poolNum);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profile, tc);

                /*
                    1.3. Call API with wrong poolNum[NULL] and valid other
                         parameters.
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc,
                                                             &mode, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profile, tc);
            }
        }

        /*
            1.4. Call API with wrong value of profile and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        tc = 0;

        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc, &mode,
                                                        &poolNum),
                profile);

        /*
            1.5. Call API with wrong tc[8] and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        tc      = CPSS_TC_RANGE_CNS;

        st = cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc, &mode,
                                                     &poolNum);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc);
    }

    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
    tc = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc, &mode,
                                                     &poolNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileTcSharingGet(dev, profile, tc, &mode,
                                                 &poolNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileTcSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileTcSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropProfileTcSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropProfileTcGet with the same parameters.
         Expected: GT_OK and the same values as was set.
    1.3. Call API with wrong profile and valid other parameters.
         Expected: GT_BAD_PARAM.
    1.4. Call API with wrong tc[8] and valid other parameters.
         Expected: GT_BAD_PARAM.
    1.5. Call API with wrong pointer to tailDropProfileParams[NULL].
         Expected: GT_BAD_PTR.
    1.6. Call API with wrong tailDropProfileParams.dp0MaxBuffNum[0x100000].
         Expected: GT_OUT_OF_RANGE.
    1.7. Call API with wrong tailDropProfileParams.dp0MaxDescNum[0x10000]
         Expected: GT_OUT_OF_RANGE.
    1.8. Call API with wrong tailDropProfileParams.dp1MaxBuffNum[0x100000].
         Expected: GT_OUT_OF_RANGE.
    1.9. Call API with wrong tailDropProfileParams.dp1MaxDescNum[0x10000]
         Expected: GT_OUT_OF_RANGE.
    1.10. Call API with wrong tailDropProfileParams.dp2MaxBuffNum[0x100000].
          Expected: GT_OUT_OF_RANGE.
    1.11. Call API with wrong tailDropProfileParams.dp2MaxDescNum[0x10000]
          Expected: GT_OUT_OF_RANGE.
    1.12. Call API with wrong tailDropProfileParams.tcMaxBuffNum[0x100000].
          Expected: GT_OUT_OF_RANGE.
    1.13. Call API with wrong tailDropProfileParams.tcMaxDescNum[0x10000]
          Expected: GT_OUT_OF_RANGE.
    1.14. Call API with wrong tailDropProfileParams.dp0QueueAlpha[6]
          Expected: GT_BAD_PARAM.
    1.15. Call API with wrong tailDropProfileParams.dp1QueueAlpha[6]
          Expected: GT_BAD_PARAM.
    1.16. Call API with wrong tailDropProfileParams.dp2QueueAlpha[6]
          Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tc;
    GT_BOOL     isEqual;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    tailDropProfileParams;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    tailDropProfileParamsGet;

    /* valid values */
    const PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_VALUE_STC validValue[] = {
        /*
            profile, tc,
            tailDropProfileParamsPtr{ dp0MaxBuffNum, dp0MaxDescNum,
                                      dp1MaxBuffNum, dp1MaxDescNum,
                                      dp2MaxBuffNum, dp2MaxDescNum,
                                      tcMaxBuffNum, tcMaxDescNum,
                                      dp0QueueAlpha, dp1QueueAlpha, dp2QueueAlpha }
        */
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E, 0,
            {
                0, 0, 0, 0, 0, 0, 0, 0,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E
            }
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_8_E, 3,
            {
                4, 8, 16, 32, 64, 128, 256, 512,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E
            }
        },
        {
            CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E, 7,
            {
                0xFFFFF, 0xFFFF, 0xFFFFF, 0xFFFF, 0xFFFFF, 0xFFFF, 0xFFFFF, 0xFFFF,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E,
                CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E
            }
        }
    };
    GT_U32  countOfValidValues = sizeof(validValue) / sizeof(validValue[0]);
    GT_U32  i;


    cpssOsBzero((GT_VOID*) &tailDropProfileParams, sizeof(tailDropProfileParams));
    cpssOsBzero((GT_VOID*) &tailDropProfileParamsGet, sizeof(tailDropProfileParamsGet));


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidValues; i++)
        {
            profile = validValue[i].profile;
            tc      = validValue[i].tc;
            tailDropProfileParams.dp0MaxBuffNum = validValue[i].params.dp0MaxBuffNum;
            tailDropProfileParams.dp0MaxDescNum = validValue[i].params.dp0MaxDescNum;
            tailDropProfileParams.dp1MaxBuffNum = validValue[i].params.dp1MaxBuffNum;
            tailDropProfileParams.dp1MaxDescNum = validValue[i].params.dp1MaxDescNum;
            tailDropProfileParams.dp2MaxBuffNum = validValue[i].params.dp2MaxBuffNum;
            tailDropProfileParams.dp2MaxDescNum = validValue[i].params.dp2MaxDescNum;
            tailDropProfileParams.tcMaxBuffNum  = validValue[i].params.tcMaxBuffNum;
            tailDropProfileParams.tcMaxDescNum  = validValue[i].params.tcMaxDescNum;
            tailDropProfileParams.dp0QueueAlpha = validValue[i].params.dp0QueueAlpha;
            tailDropProfileParams.dp1QueueAlpha = validValue[i].params.dp1QueueAlpha;
            tailDropProfileParams.dp2QueueAlpha = validValue[i].params.dp2QueueAlpha;

            /*
                1.1. Call cpssPxPortTxTailDropProfileTcSet with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc,
                                                  &tailDropProfileParams);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, tc);

            /*
                1.2. Call cpssPxPortTxTailDropProfileTcGet with the same parameters.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssPxPortTxTailDropProfileTcGet(dev, profile, tc,
                                                  &tailDropProfileParamsGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, tc);

            /* verify values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &tailDropProfileParams,
                                         (GT_VOID*) &tailDropProfileParamsGet,
                                         sizeof(tailDropProfileParams)))
                      ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isEqual,
                    "got another value that was set: %d %d %d", dev, profile, tc);
        }

        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
        tc      = 0;
        tailDropProfileParams.dp0MaxBuffNum = 0;
        tailDropProfileParams.dp0MaxDescNum = 0;
        tailDropProfileParams.dp1MaxBuffNum = 0;
        tailDropProfileParams.dp1MaxDescNum = 0;
        tailDropProfileParams.dp2MaxBuffNum = 0;
        tailDropProfileParams.dp2MaxDescNum = 0;
        tailDropProfileParams.tcMaxBuffNum  = 0;
        tailDropProfileParams.tcMaxDescNum  = 0;
        tailDropProfileParams.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        tailDropProfileParams.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        tailDropProfileParams.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /*
            1.3. Call API with wrong profile and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileTcSet(dev, profile, tc,
                                                 &tailDropProfileParams),
                profile);

        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

        /*
            1.4. Call API with wrong tc[8] and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        tc = CPSS_TC_RANGE_CNS;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc);

        tc = 0;

        /*
            1.5. Call API with wrong pointer to tailDropProfileParams[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profile, tc);

        /*
            1.6. Call API with wrong tailDropProfileParams.dp0MaxBuffNum[0x100000].
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.dp0MaxBuffNum = 0x100000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.dp0MaxBuffNum = 0;

        /*
            1.7. Call API with wrong tailDropProfileParams.dp0MaxDescNum[0x10000]
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.dp0MaxDescNum = 0x10000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.dp0MaxDescNum = 0;

        /*
            1.8. Call API with wrong tailDropProfileParams.dp1MaxBuffNum[0x100000].
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.dp1MaxBuffNum = 0x100000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.dp1MaxBuffNum = 0;

        /*
            1.9. Call API with wrong tailDropProfileParams.dp1MaxDescNum[0x10000]
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.dp1MaxDescNum = 0x10000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.dp1MaxDescNum = 0;

        /*
            1.10. Call API with wrong tailDropProfileParams.dp2MaxBuffNum[0x100000].
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.dp2MaxBuffNum = 0x100000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.dp2MaxBuffNum = 0;

        /*
            1.11. Call API with wrong tailDropProfileParams.dp2MaxDescNum[0x10000]
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.dp2MaxDescNum = 0x10000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.dp2MaxDescNum = 0;

        /*
            1.12. Call API with wrong tailDropProfileParams.tcMaxBuffNum[0x100000].
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.tcMaxBuffNum = 0x100000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.tcMaxBuffNum = 0;

        /*
            1.13. Call API with wrong tailDropProfileParams.tcMaxDescNum[0x10000]
            Expected: GT_OUT_OF_RANGE.
        */
        tailDropProfileParams.tcMaxDescNum = 0x10000;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, profile, tc);

        tailDropProfileParams.tcMaxDescNum = 0;

        /*
            1.14. Call API with wrong tailDropProfileParams.dp0QueueAlpha[6]
            Expected: GT_BAD_PARAM.
        */
        tailDropProfileParams.dp0QueueAlpha = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) 6;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc);

        tailDropProfileParams.dp0QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /*
            1.15. Call API with wrong tailDropProfileParams.dp1QueueAlpha[6]
            Expected: GT_BAD_PARAM.
        */
        tailDropProfileParams.dp1QueueAlpha = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) 6;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc);

        tailDropProfileParams.dp1QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        /*
            1.16. Call API with wrong tailDropProfileParams.dp2QueueAlpha[6]
            Expected: GT_BAD_PARAM.
        */
        tailDropProfileParams.dp2QueueAlpha = (CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT) 6;

        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc);

        tailDropProfileParams.dp2QueueAlpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
    }

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileTcSet(dev, profile, tc, &tailDropProfileParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropProfileTcGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropProfileTcGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to tailDropProfileParams[NULL] and valid
         other parameters.
         Expected: GT_BAD_PTR.
    1.3. Call API with wrong profile and valid other parameters.
         Expected: GT_BAD_PARAM.
    1.4. Call API with wrong tc[8] and valid other parameters.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      tc = 0;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile;
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    tailDropProfileParams;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over valid profiles in step of 5 */
        for (profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;
             profile <= CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E;
             profile += 5)
        {
            /* go over valid tc in step of 2 */
            for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc += 2)
            {
                /*
                    1.1. Call API with valid parameters.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxTailDropProfileTcGet(dev, profile, tc,
                                                      &tailDropProfileParams);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profile, tc);

                /*
                    1.2. Call API with wrong pointer to tailDropProfileParams[NULL]
                         and valid other parameters.
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortTxTailDropProfileTcGet(dev, profile, tc, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profile, tc);
            }
        }

        /*
            1.3. Call API with wrong profile and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(
                cpssPxPortTxTailDropProfileTcGet(dev, profile, tc,
                                                 &tailDropProfileParams),
                profile);

        profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

        /*
            1.4. Call API with wrong tc[8] and valid other parameters.
            Expected: GT_BAD_PARAM.
        */
        tc = CPSS_TC_RANGE_CNS;

        st = cpssPxPortTxTailDropProfileTcGet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profile, tc);

        tc = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */
    profile = CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropProfileTcGet(dev, profile, tc, &tailDropProfileParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropProfileTcGet(dev, profile, tc, &tailDropProfileParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropTcBuffNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tc,
    OUT GT_U32                  *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropTcBuffNumberGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call API with valid parameters.
           Expected: GT_OK.
    1.1.2. Call API with wrong pointer to numberOfBuffers[NULL].
           Expected: GT_BAD_PARAM.
    1.1.3. Call API with wrong valud of tc[8].
           Expected: GT_BAD_PARAM.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For active device check that function returns GT_BAD_PARAM for out of
         bound value for port number.
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
    GT_U32      tc;
    GT_U32      numberOfBuffers;
    GT_PHYSICAL_PORT_NUM    port;

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
            for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
            {
                /*
                    1.1.1. Call API with valid parameters.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, &numberOfBuffers);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tc);

                /*
                    1.1.2. Call API with wrong pointer to numberOfBuffers[NULL].
                    Expected: GT_BAD_PARAM.
                */
                st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, tc);
            }

            /*
                1.1.3. Call API with wrong valud of tc[8].
                Expected: GT_BAD_PARAM.
            */
            tc = CPSS_TC_RANGE_CNS;

            st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, &numberOfBuffers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tc);
        }

        tc = 0;

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, &numberOfBuffers);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tc);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value for port number.                    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, &numberOfBuffers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /*      for CPU port number.                                       */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, &numberOfBuffers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, &numberOfBuffers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropTcBuffNumberGet(dev, port, tc, &numberOfBuffers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMcastPcktDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          mcastMaxDescNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMcastPcktDescLimitSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropMcastPcktDescLimitSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropMcastPcktDescLimitGet.
         Expected: GT_OK and the same values as was set.
    1.3. Call API wrong value of mcastNum[0x10000].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      mcastNum;
    GT_U32      mcastNumGet;

    /* valid values */
    const GT_U32 validMcastNum[] = {
        0x00, 0x01, 0xFF, 0xFFF, 0xFFFF
    };
    GT_U32  countOfValidMcastNum = sizeof(validMcastNum) / sizeof(validMcastNum[0]);
    GT_U32  i;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidMcastNum; i++)
        {
            mcastNum = validMcastNum[i];

            /*
                1.1. Call cpssPxPortTxTailDropMcastPcktDescLimitSet with valid
                     parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropMcastPcktDescLimitSet(dev, mcastNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mcastNum);

            /*
                1.2. Call cpssPxPortTxTailDropMcastPcktDescLimitGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssPxPortTxTailDropMcastPcktDescLimitGet(dev, &mcastNumGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mcastNum);

            /* verify values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mcastNum, mcastNumGet,
                    "got another mcastNum than was set: %d", dev);
        }

        /*
            1.3. Call API wrong value of mcastNum[0x10000].
            Expected: GT_OUT_OF_RANGE.
        */
        mcastNum = 0x10000;

        st = cpssPxPortTxTailDropMcastPcktDescLimitSet(dev, mcastNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, mcastNum);
    }

    mcastNum = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMcastPcktDescLimitSet(dev, mcastNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMcastPcktDescLimitSet(dev, mcastNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMcastPcktDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          mcastMaxDescNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMcastPcktDescLimitGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid pointer to mcastNum.
         Expected: GT_OK.
    1.2. Call API with valid pointer .
         Expected: GT_OK and the same values as was set.
    1.3. Call API wrong value of mcastNum[0x10000].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      mcastNum;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid pointer to mcastNum.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropMcastPcktDescLimitGet(dev, &mcastNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong pointer to mcastNum[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropMcastPcktDescLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMcastPcktDescLimitGet(dev, &mcastNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMcastPcktDescLimitGet(dev, &mcastNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMcastBuffersLimitSet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          mcastMaxBuffNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMcastBuffersLimitSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropMcastBuffersLimitSet with valid mcastMaxBuffNum.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropMcastBuffersLimitGet.
         Expected: GT_OK and the same value than was set.
    1.3. Call API with out of range value of mcastMaxBuffNum[0x100000].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      mcastMaxBuffNum;
    GT_U32      mcastMaxBuffNumGet;

    /* valid values */
    const GT_U32 validValue[] = {
        0, 0x01, 0xFF, 0xFFF, 0xFFFF, 0xFFFFF
    };
    GT_U32  countOfValidValues = sizeof(validValue) / sizeof(validValue[0]);
    GT_U32  i;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidValues; i++)
        {
            mcastMaxBuffNum = validValue[i];

            /*
                1.1. Call cpssPxPortTxTailDropMcastBuffersLimitSet with valid
                     mcastMaxBuffNum.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropMcastBuffersLimitSet(dev, mcastMaxBuffNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPortTxTailDropMcastBuffersLimitGet.
                Expected: GT_OK and the same value than was set.
            */
            st = cpssPxPortTxTailDropMcastBuffersLimitGet(dev, &mcastMaxBuffNumGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropMcastBuffersLimitGet: %d ", dev);

            /* verify values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mcastMaxBuffNum, mcastMaxBuffNumGet,
                    "got another mcastMaxBuffNum then was set: %d", dev);
        }

        /*
            1.3. Call API with out of range value of mcastMaxBuffNum[0x100000].
            Expected: GT_OUT_OF_RANGE.
        */
        mcastMaxBuffNum = 0x100000;

        st = cpssPxPortTxTailDropMcastBuffersLimitSet(dev, mcastMaxBuffNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* restore correct values */
    mcastMaxBuffNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMcastBuffersLimitSet(dev, mcastMaxBuffNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMcastBuffersLimitSet(dev, mcastMaxBuffNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMcastBuffersLimitGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *mcastMaxBuffNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMcastBuffersLimitGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid pointer to mcastMaxBuffNum.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to mcastMaxBuffNum[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      mcastMaxBuffNum;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid pointer to mcastMaxBuffNum.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropMcastBuffersLimitGet(dev, &mcastMaxBuffNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong pointer to mcastMaxBuffNum[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropMcastBuffersLimitGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMcastBuffersLimitGet(dev, &mcastMaxBuffNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMcastBuffersLimitGet(dev, &mcastMaxBuffNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMcastDescNumberGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMcastDescNumberGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid pointer to number.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to number[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      number;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid pointer to number.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropMcastDescNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong pointer to number[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropMcastDescNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "numberPtr is NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMcastDescNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMcastDescNumberGet(dev, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropMcastBuffNumberGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropMcastBuffNumberGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid pointer to number.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to number[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      number;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid pointer to number.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropMcastBuffNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong pointer to number[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropMcastBuffNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "pointer is NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropMcastBuffNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropMcastBuffNumberGet(dev, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedPolicySet
(
    IN  GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT  policy
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedPolicySet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropSharedPolicySet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropSharedPolicyGet.
         Expected: GT_OK and the same policy as was set.
    1.3. Call with wrong enum values policy.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT  policy;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT  policyGet;

    /* valid policies */
    const CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT validPolicy[] = {
        CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_UNCONSTRAINED_E,
        CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_CONSTRAINED_E
    };
    GT_U32  countOfValidPolicy = sizeof(validPolicy) / sizeof(validPolicy[0]);
    GT_U32  i;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidPolicy; i++)
        {
            policy = validPolicy[i];

            /*
                1.1. Call cpssPxPortTxTailDropSharedPolicySet with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropSharedPolicySet(dev, policy);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPortTxTailDropSharedPolicyGet.
                Expected: GT_OK and the same policy as was set.
            */
            st = cpssPxPortTxTailDropSharedPolicyGet(dev, &policyGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* verify values */
            UTF_VERIFY_EQUAL1_STRING_MAC(policy, policyGet,
                    "get another policy as was set: %d", dev);
        }

        /*
            1.3. Call with wrong enum values policy.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortTxTailDropSharedPolicySet(dev, policy),
                            policy);
    }

    policy = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_UNCONSTRAINED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedPolicySet(dev, policy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedPolicySet(dev, policy);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedPolicyGet
(
    IN  GT_SW_DEV_NUM                                devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT  *policyPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedPolicyGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to policy[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT  policy;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropSharedPolicyGet(dev, &policy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong pointer to policy[NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssPxPortTxTailDropSharedPolicyGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedPolicyGet(dev, &policy);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedPolicyGet(dev, &policy);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedPoolLimitsSet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          poolNum,
    IN  GT_U32          maxBuffNum,
    IN  GT_U32          maxDescNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedPoolLimitsSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropSharedPoolLimitsSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropSharedPoolLimitsGet.
         Expected: GT_OK and the same values as was set.
    1.3. Call API with wrong poolNum[8].
         Expected: GT_BAD_PARAM.
    1.4. Call API with wrong maxBuffNum[0x100000].
         Expected: GT_OUT_OF_RANGE.
    1.5. Call API with wrong maxDescNum[0x10000].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      poolNum = 0;
    GT_U32      maxBuffNum;
    GT_U32      maxBuffNumGet;
    GT_U32      maxDescNum;
    GT_U32      maxDescNumGet;

    /* valid values */
    const GT_U32  validValue[][2] = {
        /* maxBuffNum, maxDescNum */
        { 0,       0      },
        { 123,     234    },
        { 0xFFFFF, 0xFFFF }
    };
    GT_U32  countOfValidValues = sizeof(validValue) / sizeof(validValue[0]);
    GT_U32  i;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over valid poolNum in step of 2 */
        for (poolNum = 0; poolNum < PRV_CPSS_PX_SHARED_POOL_NUM_CNS; poolNum += 2)
        {
            /* go over all valid values */
            for (i = 0; i < countOfValidValues; i++)
            {
                maxBuffNum = validValue[i][0];
                maxDescNum = validValue[i][1];

                /*
                    1.1. Call cpssPxPortTxTailDropSharedPoolLimitsSet with valid
                         parameters.
                    Expected: GT_OK.
                */
                st = cpssPxPortTxTailDropSharedPoolLimitsSet(dev, poolNum,
                        maxBuffNum, maxDescNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.2. Call cpssPxPortTxTailDropSharedPoolLimitsGet.
                    Expected: GT_OK and the same values as was set.
                */
                st = cpssPxPortTxTailDropSharedPoolLimitsGet(dev, poolNum,
                        &maxBuffNumGet, &maxDescNumGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssPxPortTxTailDropSharedPoolLimitsGet: %d ", dev);

                /* verify values */
                UTF_VERIFY_EQUAL1_STRING_MAC(maxBuffNum, maxBuffNumGet,
                        "got another maxBuffNum then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(maxDescNum, maxDescNumGet,
                        "got another maxDescNum then was set: %d", dev);
            }
        }

        /*
            1.3. Call API with wrong poolNum[8].
            Expected: GT_BAD_PARAM.
        */
        poolNum = PRV_CPSS_PX_SHARED_POOL_NUM_CNS;

        st = cpssPxPortTxTailDropSharedPoolLimitsSet(dev, poolNum, maxBuffNum,
                                                     maxDescNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        poolNum = 0;

        /*
            1.4. Call API with wrong maxBuffNum[0x100000].
            Expected: GT_OUT_OF_RANGE.
        */
        maxBuffNum = 0x100000;

        st = cpssPxPortTxTailDropSharedPoolLimitsSet(dev, poolNum, maxBuffNum,
                                                     maxDescNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        maxBuffNum = 0;

        /*
            1.5. Call API with wrong maxDescNum[0x10000].
            Expected: GT_OUT_OF_RANGE.
        */
        maxDescNum = 0x10000;

        st = cpssPxPortTxTailDropSharedPoolLimitsSet(dev, poolNum, maxBuffNum,
                                                     maxDescNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        maxDescNum = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedPoolLimitsSet(dev, poolNum, maxBuffNum,
                                                     maxDescNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedPoolLimitsSet(dev, poolNum, maxBuffNum,
                                                 maxDescNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedPoolLimitsGet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          poolNum,
    OUT GT_U32          *maxBuffNumPtr,
    OUT GT_U32          *maxDescNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedPoolLimitsGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to maxBuffNum and valid other parameters.
         Expected: GT_BAD_PTR.
    1.3. Call API with wrong pointer to maxDescNum and valid other parameters.
         Expected: GT_BAD_PTR.
    1.4. Call API with wrong poolNum[8].
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      poolNum;
    GT_U32      maxBuffNum;
    GT_U32      maxDescNum;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid poolNum */
        for (poolNum = 0; poolNum < PRV_CPSS_PX_SHARED_POOL_NUM_CNS; poolNum++)
        {
            /*
                1.1. Call API with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropSharedPoolLimitsGet(dev, poolNum,
                    &maxBuffNum, &maxDescNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call API with wrong pointer to maxBuffNum and valid
                     other parameters.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropSharedPoolLimitsGet(dev, poolNum,
                    NULL, &maxDescNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.3. Call API with wrong pointer to maxDescNum and valid
                     other parameters.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropSharedPoolLimitsGet(dev, poolNum,
                    &maxBuffNum, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        /*
            1.4. Call API with wrong poolNum[8].
            Expected: GT_BAD_PARAM.
        */
        poolNum = PRV_CPSS_PX_SHARED_POOL_NUM_CNS;

        st = cpssPxPortTxTailDropSharedPoolLimitsGet(dev, poolNum, &maxBuffNum,
                                                     &maxDescNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    poolNum = 0;

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedPoolLimitsGet(dev, poolNum, &maxBuffNum,
                                                     &maxDescNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedPoolLimitsGet(dev, poolNum, &maxBuffNum,
                                                 &maxDescNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedResourceDescNumberGet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          poolNum,
    OUT GT_U32          *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedResourceDescNumberGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong poolNum[8].
         Expected: GT_BAD_PARAM.
    1.3. Call API with wrong pointer to number[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      poolNum = 0;
    GT_U32      number;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid poolNum[0..7] */
        for (poolNum = 0; poolNum < CPSS_TC_RANGE_CNS; poolNum++)
        {
            /*
                1.1. Call API with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropSharedResourceDescNumberGet(dev, poolNum, &number);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call API with wrong poolNum[8].
            Expected: GT_BAD_PARAM.
        */
        poolNum = CPSS_TC_RANGE_CNS;

        st = cpssPxPortTxTailDropSharedResourceDescNumberGet(dev, poolNum, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        poolNum = 0;

        /*
            1.3. Call API with wrong numberPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropSharedResourceDescNumberGet(dev, poolNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "numberPtr is NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedResourceDescNumberGet(dev, poolNum, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedResourceDescNumberGet(dev, poolNum, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropSharedResourceBuffNumberGet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          poolNum,
    OUT GT_U32          *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropSharedResourceBuffNumberGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong poolNum[8].
         Expected: GT_BAD_PARAM.
    1.3. Call API with wrong pointer to number[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      poolNum = 0;
    GT_U32      number;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid poolNum[0..7] */
        for (poolNum = 0; poolNum < CPSS_TC_RANGE_CNS; poolNum++)
        {
            /*
                1.1. Call API with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropSharedResourceBuffNumberGet(dev, poolNum, &number);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call API with wrong poolNum[8].
            Expected: GT_BAD_PARAM.
        */
        poolNum = CPSS_TC_RANGE_CNS;

        st = cpssPxPortTxTailDropSharedResourceBuffNumberGet(dev, poolNum, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        poolNum = 0;

        /*
            1.3. Call API with wrong pointer to number[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropSharedResourceBuffNumberGet(dev, poolNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "numberPtr is NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropSharedResourceBuffNumberGet(dev, poolNum, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropSharedResourceBuffNumberGet(dev, poolNum, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropGlobalDescNumberGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropGlobalDescNumberGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to number[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      number;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropGlobalDescNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong numberPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropGlobalDescNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "numberPtr is NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropGlobalDescNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropGlobalDescNumberGet(dev, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropGlobalBuffNumberGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropGlobalBuffNumberGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid parameters.
         Expected: GT_OK.
    1.2. Call API with wrong pointer to number[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      number;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid parameters.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropGlobalBuffNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong numberPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropGlobalBuffNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "numberPtr is NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family  */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.          */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropGlobalBuffNumberGet(dev, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropGlobalBuffNumberGet(dev, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropDescNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropDescNumberGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call API with valid parameters.
           Expected: GT_OK.
    1.1.2. Call API with wrong pointer to number[NULL].
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function with out of bound value for port
         number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function with CPU port number.
         Expected: GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32      resNum;

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
                1.1.1. Call API with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropDescNumberGet(dev, port, &resNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call API with wrong pointer to number[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropDescNumberGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxTailDropDescNumberGet(dev, port, &resNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, resNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value for port number.                    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxTailDropDescNumberGet(dev, port, &resNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /*      for CPU port number.                                       */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxTailDropDescNumberGet(dev, port, &resNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropDescNumberGet(dev, port, &resNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropDescNumberGet(dev, port, &resNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropBuffNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *numPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropBuffNumberGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call API with valid parameters.
           Expected: GT_OK.
    1.1.2. Call API with wrong pointer to number[NULL].
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function with out of bound value for port
         number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function with CPU port number.
         Expected: GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_U32                  resNum;

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
                1.1.1. Call API with valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropBuffNumberGet(dev, port, &resNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call API with wrong pointer to number[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortTxTailDropBuffNumberGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextTxqPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortTxTailDropBuffNumberGet(dev, port, &resNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, resNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /*      for out of bound value for port number.                    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortTxTailDropBuffNumberGet(dev, port, &resNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /*      for CPU port number.                                 */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortTxTailDropBuffNumberGet(dev, port, &resNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_TX_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropBuffNumberGet(dev, port, &resNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropBuffNumberGet(dev, port, &resNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropDbaModeEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropDbaModeEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Get current tail drop DBA enable state.
         Expected: GT_OK.
    1.2. Inverse tail drop DBA enable state.
         Expected: GT_OK.
    1.3. Get new tail drop DBA enable state.
         Expected: GT_OK and the same tail drop DBA enable state as was set.
    1.4. Restore tail drop DBA enable state.
         Expected: GT_OK.
    1.5. Check restored tail drop DBA enable state.
         Expected: GT_OK and the same tail drop DBA enable state as was set.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Get current tail drop DBA enable state.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropDbaModeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Inverse tail drop DBA enable state.
            Expected: GT_OK.
        */
        enable = (enableGet == GT_TRUE) ? GT_FALSE : GT_TRUE;

        st = cpssPxPortTxTailDropDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.3. Get new tail drop DBA enable state.
            Expected: GT_OK and the same enable state as was set.
        */
        st = cpssPxPortTxTailDropDbaModeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify returned profile */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another tail drop DBA enable state as was set: %d", dev);

        /*
            1.4. Restore tail drop DBA enable state.
            Expected: GT_OK.
        */
        enable = (enableGet == GT_TRUE) ? GT_FALSE : GT_TRUE;

        st = cpssPxPortTxTailDropDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.5. Check restored tail drop DBA enable state.
            Expected: GT_OK and the same enable state as was set.
        */
        st = cpssPxPortTxTailDropDbaModeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* verify returned profile */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another tail drop DBA enable state as was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not active devices check that function returns  */
    /*    GT_NOT_APPLICABLE_DEVICE.                           */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropDbaModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropDbaModeEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropDbaModeEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropDbaModeEnableGet)
{
/*
    1. Go over all active devices.
    1.1. Call API with valid pointer to tail drop DBA enable parameter.
         Expected: GT_OK.
    1.2. Call API with wrong enable[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call API with valid pointer to tail drop DBA enable parameter.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropDbaModeEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call API with wrong enable[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropDbaModeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not active devices check that function returns  */
    /*    GT_NOT_APPLICABLE_DEVICE.                           */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropDbaModeEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropDbaModeEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropDbaAvailableBuffSet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          availableBuff
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropDbaAvailableBuffSet)
{
/*
    1. Go over all active devices.
    1.1. Call cpssPxPortTxTailDropDbaAvailableBuffSet with valid parameters.
         Expected: GT_OK.
    1.2. Call cpssPxPortTxTailDropDbaAvailableBuffGet.
         Expected: GT_OK and the same values as was set.
    1.3. Call with out of range value of availableBuff[0x100000].
         Expected: GT_OUT_OF_RANGE.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      availableBuff;
    GT_U32      availableBuffGet;

    /* valid values of availableBuff */
    const GT_U32 validBuffCount[] = {
        0, 0x0F, 0xFF, 0x0FFF, 0xFFFF, 0x0FFFFF
    };
    GT_U32 countOfValidBuffCount =
                        sizeof(validBuffCount) / sizeof(validBuffCount[0]);
    GT_U32 i;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < countOfValidBuffCount; i++)
        {
            availableBuff = validBuffCount[i];

            /*
                1.1. Call cpssPxPortTxTailDropDbaAvailableBuffSet with
                     valid parameters.
                Expected: GT_OK.
            */
            st = cpssPxPortTxTailDropDbaAvailableBuffSet(dev,
                    availableBuff);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssPxPortTxTailDropDbaAvailableBuffGet with
                     valid pointer to availableBuff.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPortTxTailDropDbaAvailableBuffGet(dev,
                    &availableBuffGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortTxTailDropDbaAvailableBuffGet: %d", dev);

            /* verify values */
            UTF_VERIFY_EQUAL1_STRING_MAC(availableBuff, availableBuffGet,
                    "got another availableBuff then was set: %d", dev);
        }

        /*
            1.3. Call with out of range value of availableBuff[64].
            Expected: GT_OUT_OF_RANGE.
        */
        availableBuff = 0x100000;

        st = cpssPxPortTxTailDropDbaAvailableBuffSet(dev, availableBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* restore correct value */
    availableBuff = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropDbaAvailableBuffSet(dev, availableBuff);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropDbaAvailableBuffSet(dev, availableBuff);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortTxTailDropDbaAvailableBuffGet
(
    IN  GT_SW_DEV_NUM   dev,
    OUT GT_U32          *availableBuffPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortTxTailDropDbaAvailableBuffGet)
{
/*
    1. Go over all active devices.
    1.1. Call function with valid pointer to availableBuff.
         Expected: GT_OK.
    1.2. Call function with wrong pointer to availableBuff[NULL].
         Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st;
    GT_U8       dev;
    GT_U32      buffCount;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid pointer to availableBuff.
            Expected: GT_OK.
        */
        st = cpssPxPortTxTailDropDbaAvailableBuffGet(dev, &buffCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with wrong pointer to availableBuff[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortTxTailDropDbaAvailableBuffGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortTxTailDropDbaAvailableBuffGet(dev, &buffCount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortTxTailDropDbaAvailableBuffGet(dev, &buffCount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortTxTailDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortTxTailDrop)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropUcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropUcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropPacketModeLengthSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropPacketModeLengthGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileBufferConsumptionModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileBufferConsumptionModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMaskSharedBuffEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMaskSharedBuffEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedBuffMaxLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedBuffMaxLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropWrtdMasksSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropWrtdMasksGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileTcSharingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileTcSharingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileTcSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropProfileTcGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropTcBuffNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMcastPcktDescLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMcastPcktDescLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMcastBuffersLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMcastBuffersLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMcastDescNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropMcastBuffNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedPolicySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedPolicyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedPoolLimitsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedPoolLimitsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedResourceDescNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropSharedResourceBuffNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropGlobalDescNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropGlobalBuffNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropDescNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropBuffNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropDbaModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropDbaModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropDbaAvailableBuffSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortTxTailDropDbaAvailableBuffGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortTxTailDrop)

