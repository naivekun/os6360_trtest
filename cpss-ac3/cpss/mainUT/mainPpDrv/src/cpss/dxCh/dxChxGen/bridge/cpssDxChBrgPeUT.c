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
* @file cpssDxChBrgPeUT.c
*
* @brief Unit tests for cpssDxChBrgPe, that provides
* CPSS DxCh implementation for Bridge Generic APIs.
*
* @version   4
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPe.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Defines */

/* Default valid value for port id */
#define BRG_PE_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPeEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPeEnableSet)
{
/*
    ITERATE_DEVICES (xCat3)
    1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgPeEnableGet
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1  */
        enable = GT_FALSE;

        st = cpssDxChBrgPeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*  1.2  */
        st = cpssDxChBrgPeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d", dev);

        /*  1.1  */
        enable = GT_TRUE;

        st = cpssDxChBrgPeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*  1.2  */
        st = cpssDxChBrgPeEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPeEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPeEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPeEnableGet)
{
/*
    ITERATE_DEVICES (xCat3)
    1.1. Call with not NULL enable
    Expected: GT_OK.
    1.2. Call with NULL enable
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1  */
        st = cpssDxChBrgPeEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2  */
        st = cpssDxChBrgPeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPeEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPeEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPePortEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPePortEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat3)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM portNum  = BRG_PE_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_TRUE;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with status [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgPePortEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);

            st = cpssDxChBrgPePortEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enableGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                 "get another enable than was set: %d, %d", dev, portNum);

            /* Call function with status [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChBrgPePortEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);

            st = cpssDxChBrgPePortEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enableGet);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                 "get another enable than was set: %d, %d", dev, portNum);
        }

        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPePortEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgPePortEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPePortEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    portNum = BRG_PE_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPePortEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPePortEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPePortEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPePortEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat3)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st      = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM portNum    = BRG_PE_VALID_PHY_PORT_CNS;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgPePortEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum, enable);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgPePortEnableGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, portNum);
        }

        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChBrgPePortEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgPePortEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPePortEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    portNum = BRG_PE_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPePortEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPePortEnableGet(dev, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPePortPcidMcFilterEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 pcid,
    IN GT_BOOL                mcFilterEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPePortPcidMcFilterEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat3) & pcid[0 / BIT_6-1 / BIT_12-1]
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgPePortPcidMcFilterEnableGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong pcid [BIT_12].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM portNum = BRG_PE_VALID_PHY_PORT_CNS;
    GT_U32      pcid = 0;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet;

    GT_U32      ii;
    GT_U32      maxPcid;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            maxPcid = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.bpePointToPointEChannels;
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, maxPcid);

            for( ii = 0; ii < 2 ; ii++)
            {
                enable = (ii == 0) ? GT_TRUE : GT_FALSE ;

                /*
                    1.1.1. Call with pcid[0 / BIT_6-1 / maxPcid-1],
                    Expected: GT_OK.
                */

                /* call with pcid[0] */
                pcid = 0;

                st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.1.2. Call cpssDxChBrgPePortPcidMcFilterEnableGet
                           with the same parameters.
                    Expected: GT_OK and the same values than was set.
                */
                st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgPePortPcidMcFilterEnableGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "got another PCID then was set: %d", dev);

                /* call with pcid[BIT_6-1] */
                pcid = BIT_6-1;

                st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call get func again, 1.1.2. */
                st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgPePortPcidMcFilterEnableGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "got another PCID then was set: %d", dev);

                /* call with pcid[maxPcid-1] */
                pcid = maxPcid-1;

                st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call get func again, 1.1.2. */
                st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgPePortPcidMcFilterEnableGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "got another PCID then was set: %d", dev);

                /*
                    1.1.3. Call api with wrong pcid [maxPcid].
                    Expected: GT_BAD_PARAM.
                */
                pcid = maxPcid;

                st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
        }

        pcid = 0;

        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_PE_VALID_PHY_PORT_CNS;
    pcid = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPePortPcidMcFilterEnableSet(dev, portNum, pcid, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPePortPcidMcFilterEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32                 pcid,
    OUT GT_BOOL                *mcFilterEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPePortPcidMcFilterEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat3)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PHYSICAL_PORT_NUM portNum = BRG_PE_VALID_PHY_PORT_CNS;
    GT_U32      pcid = 0;
    GT_BOOL     enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_PE_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                                           UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPePortPcidMcFilterEnableGet(dev, portNum, pcid, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    IN GT_BOOL                  enable,
    IN GT_TRUNK_ID              trunkId
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet)
{
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PHYSICAL_PORT_NUM portNum = BRG_PE_VALID_PHY_PORT_CNS;
    GT_BOOL     enable,enableGet;
    GT_TRUNK_ID trunkId,trunkIdGet;
    GT_U32      notAppFamilyBmp = UTF_ALL_FAMILY_E & (~UTF_CPSS_PP_E_ARCH_CNS);/*supported only by e_arch*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        trunkId = 7;
        enable = GT_TRUE;

        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /* check enabling */

            enable = GT_TRUE;
            trunkId = 22;
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* check the get values */
            UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(trunkId, trunkIdGet, dev);

            /* check the get values */
            UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
            /*  not care about trunkIdGet
                UTF_VERIFY_EQUAL1_PARAM_MAC(trunkId, trunkIdGet, dev);
            */

            /* check enabling with bad trunkId */

            /* not valid trunkId */
            trunkId = 0;
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            /* overflow trunkId */
            trunkId = BIT_12;
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            /* good trunkId */
            trunkId = BIT_12-1;
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* check disabling */
            enable = GT_FALSE;
            trunkId = 15;

            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        }

        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        trunkId = 7;
        enable = GT_TRUE;

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_PE_VALID_PHY_PORT_CNS;
    trunkId = 7;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(dev, portNum, enable , trunkId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_TRUNK_ID              *trunkIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat3)
    1.1.1. Call with not NULL pcidPtr
    Expected: GT_OK.
    1.1.2. Call api with wrong pcidPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PHYSICAL_PORT_NUM portNum = BRG_PE_VALID_PHY_PORT_CNS;
    GT_BOOL     enableGet;
    GT_TRUNK_ID trunkIdGet;
    GT_U32      notAppFamilyBmp = UTF_ALL_FAMILY_E & (~UTF_CPSS_PP_E_ARCH_CNS);/*supported only by e_arch*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL pcidPtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enableGet [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, NULL , &trunkIdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enableGet = NULL", dev);

            /*
                1.1.2. Call api with wrong trunkIdGet [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, trunkIdGet = NULL", dev);
        }

        st = prvUtfNextPhyPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_PE_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(dev, portNum, &enableGet , &trunkIdGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgPe suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgPe)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPePortEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPePortEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPePortPcidMcFilterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPePortPcidMcFilterEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgPe)


