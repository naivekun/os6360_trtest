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
* @file cpssDxChTmGlueDropUT.c
*
* @brief Unit tests for cpssDxChTmGlueDrop, that provides
* CPSS DxCh implementation for TM Glue Drop Unit APIs.
*
* @version   2
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDrop.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDropQueueProfileIdSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    IN  GT_U32    profileId
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueDropQueueProfileIdSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with queueId [0, 16, 1000, 16383],
                            profileId [0..7]
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueDropQueueProfileIdGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3.  Call  with bad parameter queueId [16390]
    Expected: GT_BAD_PARAM.
    1.4.  Call  with wrong(out-of-range) profileId [8]
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    queueId = 0;
    GT_U32    profileId = 0;
    GT_U32    profileIdGet;

  /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with queueId [0, 16, 1000, 16383],
                                     profileId [0..7]
            Expected: GT_OK.
            1.2. Call cpssDxChTmGlueDropQueueProfileIdGet with the same parameters.
            Expected: GT_OK and the same values read.
        */
        /* Call function with queueId [0], profileId [7] */
        queueId   = 0;
        profileId = 7;
        st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileIdGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueDropQueueProfileIdGet: %d, %d", dev, profileIdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet, "get another profileId than was set: %d", dev);

        /* Call function with queueId [16], profileId [5] */
        queueId   = 16;
        profileId = 5;
        st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileIdGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueDropQueueProfileIdGet: %d, %d", dev, profileIdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet, "get another profileId than was set: %d", dev);


        /* Call function with queueId [1000], profileId [2] */
        queueId   = 1000;
        profileId = 2;
        st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileIdGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueDropQueueProfileIdGet: %d, %d", dev, profileIdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet, "get another profileId than was set: %d", dev);

        /* Call function with queueId [16383], profileId [1] */
        queueId   = 16383;
        profileId = 1;
        st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileIdGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                          "cpssDxChTmGlueDropQueueProfileIdGet: %d, %d", dev, profileIdGet);
        /* Verifying value */
        UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet, "get another profileId than was set: %d", dev);

        /*
            1.3.  Call  with wrong queueId [16390]
            Expected: GT_BAD_PARAM.
        */

        queueId   = 16390;
        profileId = 1;
        st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, queueId, profileId);

        /*
            1.4.  Call  with wrong(out-of-range) profileId [8]
            Expected: GT_OUT_OF_RANGE.
        */
        queueId   = 88;
        profileId = 8;
        st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, queueId, profileId);
    }

    queueId   = 0;
    profileId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDropQueueProfileIdSet(dev, queueId, profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDropQueueProfileIdGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    queueId,
    OUT GT_U32   *profileIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueDropQueueProfileIdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with queueId [0, 16, 1000, 16383].
    Expected: GT_OK.
    1.2.  Call  with wrong queueId [16384]
    Expected: GT_BAD_PARAM
    1.3. Call with wrong profileIdPtr [NULL]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32      queueId = 0;
    GT_U32      profileId = 0;

   /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        queueId = 0;
        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueId);

        /*  1.1.  */
        queueId = 16;
        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueId);

        /*  1.1.  */
        queueId = 1000;
        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueId);

        /*  1.1.  */
        queueId = 16383;
        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, queueId);

        /*  1.2.  */
        queueId = 16384;
        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, queueId);

        /*  1.3.  */
        queueId = 0;
        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, queueId);
    }
    queueId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDropQueueProfileIdGet(dev, queueId, &profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDropProfileDropMaskSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    IN  CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueDropProfileDropMaskSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with profileId [0..7],
                            tc [0..15]
    Alternate the bits in struct CPSS_DXCH_TM_GLUE_DROP_MASK_STC.
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueDropQueueProfileIdGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3.  Call  with bad param profileId [8]
    Expected: GT_BAD_PARAM.
    1.4.  Call  with bad param tc [16]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    tc = 0;
    GT_U32    profileId = 0;
    CPSS_DXCH_TM_GLUE_DROP_MASK_STC  dropMaskCfg = {0};
    GT_U8     i, j;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with profileId [0..7],
                                    tc [0..15]
            Expected: GT_OK.
            1.2. Call cpssDxChTmGlueDropProfileDropMaskGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        for (i=0; i<8; i++)
        {
            for (j=0; j<16; j++)
            {
                profileId = i;
                tc  = j;
                dropMaskCfg.qTailDropUnmask = i%2;
                dropMaskCfg.qWredDropUnmask = j%2;
                dropMaskCfg.aTailDropUnmask = i%2;
                dropMaskCfg.aWredDropUnmask = j%2;
                dropMaskCfg.bTailDropUnmask = i%2;
                dropMaskCfg.bWredDropUnmask = j%2;
                dropMaskCfg.cTailDropUnmask = i%2;
                dropMaskCfg.cWredDropUnmask = j%2;
                dropMaskCfg.portTailDropUnmask =i%2;
                dropMaskCfg.portWredDropUnmask =j%2;
                dropMaskCfg.outOfResourceDropUnmask = i%2;
                dropMaskCfg.redPacketsDropMode = CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ASK_TM_RESP_E;
                st = cpssDxChTmGlueDropProfileDropMaskSet(dev, profileId, tc, &dropMaskCfg);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileId, tc);
            }
        }

        for (i=0; i<8; i++)
        {
            for (j=0; j<16; j++)
            {
                profileId = i;
                tc  = j;
                st = cpssDxChTmGlueDropProfileDropMaskGet(dev, profileId, tc, &dropMaskCfg);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileId, tc);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                  "cpssDxChTmGlueDropProfileDropMaskSet: %d, %d, %d", dev, profileId, tc);
                /* Verifying value */
                UTF_VERIFY_EQUAL3_STRING_MAC(i, profileId,
                                             "get another profileId than was set: dev= %d  set= %d read= %d", dev, i, profileId);
                UTF_VERIFY_EQUAL3_STRING_MAC(j, tc,
                                             "get another tc than was set: dev= %d  set= %d read= %d", dev, j, tc);
                UTF_VERIFY_EQUAL3_STRING_MAC(i%2, dropMaskCfg.qTailDropUnmask,
                                             "get another qTailDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, i%2, dropMaskCfg.qTailDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(j%2, dropMaskCfg.qWredDropUnmask,
                                             "get another qWredDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, j%2, dropMaskCfg.qWredDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(i%2, dropMaskCfg.aTailDropUnmask,
                                             "get another aTailDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, i%2, dropMaskCfg.aTailDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(j%2, dropMaskCfg.aWredDropUnmask,
                                             "get another aWredDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, j%2, dropMaskCfg.aWredDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(i%2, dropMaskCfg.bTailDropUnmask,
                                             "get another bTailDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, i%2, dropMaskCfg.bTailDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(j%2, dropMaskCfg.bWredDropUnmask,
                                             "get another bWredDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, j%2, dropMaskCfg.bWredDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(i%2, dropMaskCfg.cTailDropUnmask,
                                             "get another cTailDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, i%2, dropMaskCfg.cTailDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(j%2, dropMaskCfg.cWredDropUnmask,
                                             "get another cWredDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, j%2, dropMaskCfg.cWredDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(i%2, dropMaskCfg.portTailDropUnmask,
                                             "get another portTailDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, i%2, dropMaskCfg.portTailDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(j%2, dropMaskCfg.portWredDropUnmask,
                                             "get another portWredDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, j%2, dropMaskCfg.portWredDropUnmask);
                UTF_VERIFY_EQUAL3_STRING_MAC(i%2, dropMaskCfg.outOfResourceDropUnmask,
                                             "get another outOfResourceDropUnmask than was set: dev= %d  set= %d read= %d",
                                             dev, i%2, dropMaskCfg.outOfResourceDropUnmask);
            }
        }

        /* Call function with profileId [8] */
        profileId = 8;
        tc  = 4;
        st = cpssDxChTmGlueDropProfileDropMaskSet(dev, profileId, tc, &dropMaskCfg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileId, tc);

        /* Call function with tc [16] */
        profileId = 7;
        tc  = 16;
        st = cpssDxChTmGlueDropProfileDropMaskSet(dev, profileId, tc, &dropMaskCfg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileId, tc);

        /* Call function with dropMaskCfg [NULL] */
        profileId = 7;
        tc  = 6;
        st = cpssDxChTmGlueDropProfileDropMaskSet(dev, profileId, tc, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profileId, tc);

    }

    profileId = 0;
    tc  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDropProfileDropMaskSet(dev, profileId, tc, &dropMaskCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDropProfileDropMaskSet(dev,profileId, tc, &dropMaskCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDropProfileDropMaskGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   profileId,
    IN  GT_U32                                   tc,
    OUT CPSS_DXCH_TM_GLUE_DROP_MASK_STC         *dropMaskCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueDropProfileDropMaskGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with profileId [0..7].
                            tc [0..15]
    Expected: GT_OK.
    1.2.  Call  with bad param profileId [8]
    Expected: GT_BAD_PARAM
    1.3.  Call  with bad param tc [16]
    Expected: GT_BAD_PARAM
    1.4. Call with wrong dropMaskCfgPtr [NULL]
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32      profileId;
    GT_U32      tc;
    CPSS_DXCH_TM_GLUE_DROP_MASK_STC  dropMaskCfg = {0};
    GT_U32      i, j;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        for (i=0; i<8; i++)
        {
            for (j=0; j<16;j++)
            {
                profileId = i;
                tc = j;
                st = cpssDxChTmGlueDropProfileDropMaskGet(dev, profileId, tc, &dropMaskCfg);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileId, tc);
            }
        }

        /*  1.2.  */
        profileId = 8;
        tc = 2;
        st = cpssDxChTmGlueDropProfileDropMaskGet(dev, profileId, tc, &dropMaskCfg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileId, tc);

        /*  1.3.  */
        profileId = 5;
        tc = 16;
        st = cpssDxChTmGlueDropProfileDropMaskGet(dev, profileId, tc, &dropMaskCfg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, profileId, tc);

        /*  1.3.  */
        profileId = 5;
        tc = 6;
        st = cpssDxChTmGlueDropProfileDropMaskGet(dev, profileId, tc, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, profileId, tc);
    }

    profileId = 0;
    tc = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDropProfileDropMaskGet(dev, profileId, tc, &dropMaskCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDropProfileDropMaskGet(dev, profileId, tc, &dropMaskCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDropTcToCosSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    IN  GT_U32    cos
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueDropTcToCosSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with tmTc [0..15],
                            cos [0..7]
    Expected: GT_OK.
    1.2. Call cpssDxChTmGlueDropQueueProfileIdGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3.  Call  with bad param tmTc [16]
    Expected: GT_BAD_PARAM.
    1.4.  Call  with wrong(out-of-range) cos [8]
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS st    = GT_OK;

    GT_U8     dev;
    GT_U32    tmTc = 0;
    GT_U32    cos = 0;
    GT_U8     i;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with tmTc [0..15],
                                    cos [0..7]
            Expected: GT_OK.
            1.2. Call cpssDxChTmGlueDropQueueProfileIdGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        for (i=0; i<16; i++)
        {
            tmTc = i;
            cos  = i%8;
            st = cpssDxChTmGlueDropTcToCosSet(dev, tmTc, cos);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tmTc, cos);
        }

        for (i=0; i<16; i++)
        {
            tmTc = i;
            st = cpssDxChTmGlueDropTcToCosGet(dev, tmTc, &cos);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tmTc, cos);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                              "cpssDxChTmGlueDropQueueProfileIdGet: %d, %d, %d", dev, tmTc, cos);
            /* Verifying value */
            UTF_VERIFY_EQUAL3_STRING_MAC(i%8, cos, "get another profileId than was set: dev= %d  set= %d read= %d", dev, i%8, cos);
        }


        /* Call function with tmTc [16] */
        tmTc = 16;
        cos  = 4;
        st = cpssDxChTmGlueDropTcToCosSet(dev, tmTc, cos);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, tmTc, cos);


        /* Call function with cos [8] */
        tmTc = 5;
        cos  = 8;
        st = cpssDxChTmGlueDropTcToCosSet(dev, tmTc, cos);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, tmTc, cos);

    }

    tmTc = 0;
    cos  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDropTcToCosSet(dev, tmTc, cos);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDropTcToCosSet(dev, tmTc, cos);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueDropTcToCosGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    tmTc,
    OUT GT_U32   *cosPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueDropTcToCosGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with tmTc [0..15].
    Expected: GT_OK.
    1.2.  Call  with bad param tmTc [16]
    Expected: GT_BAD_PARAM
    1.3. Call with wrong cosPtr [NULL]
    Expected: GT_OUT_OF_RANGE.
*/

    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0 ;

    GT_U32      tmTc = 0;
    GT_U32      cosPtr;
    GT_U8       i;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        for (i=0; i<16; i++)
        {
            tmTc = i;
            st = cpssDxChTmGlueDropTcToCosGet(dev, tmTc, &cosPtr);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tmTc, cosPtr);
        }

        /*  1.1.  */
        tmTc = 16;
        st = cpssDxChTmGlueDropTcToCosGet(dev, tmTc, &cosPtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, tmTc, cosPtr);

        /*  1.3.  */
        tmTc = 0;
        st = cpssDxChTmGlueDropTcToCosGet(dev, tmTc, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, tmTc);
    }
    tmTc = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueDropTcToCosGet(dev, tmTc, &cosPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueDropTcToCosGet(dev, tmTc, &cosPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgGen suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTmGlueDrop)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDropQueueProfileIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDropQueueProfileIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDropProfileDropMaskSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDropProfileDropMaskGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDropTcToCosSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueDropTcToCosGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTmGlueDrop)

