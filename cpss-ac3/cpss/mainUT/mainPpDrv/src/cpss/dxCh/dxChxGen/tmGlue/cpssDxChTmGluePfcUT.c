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
* @file cpssDxChTmGluePfcUT.c
*
* @brief Traffic Manager Glue - PFC unit tests.
*
* @version   3
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlue.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueFlowControl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#define UT_TM_GLUE_PFC_MAX_CNODE_CNS 512
#define UT_TM_GLUE_PFC_MAX_PORT_CNS  64
#define UT_TM_GLUE_PFC_MAX_TC_CNS     8

static GT_U8 cNodesArr[UT_TM_GLUE_PFC_MAX_CNODE_CNS];

/* cpssDxChTmGlueInit is called from cpssTmLibInit,
   verify GluePfc is initialized for system where TM has not initialized,
   currently its relevant to cpssDxChTmGluePfcTmTcPort2CNodeSet API */
static void cpssDxChTmGluePfcVerifyInit
(
    IN  GT_U8 devNum
)
{
    GT_STATUS st;

    st = cpssDxChTmGluePfcTmTcPort2CNodeSet(devNum, 0, 0, 0);

    if (st == GT_NOT_INITIALIZED)
    {
        cpssDxChTmGlueInit(devNum);
    }
}

static void cpssDxChTmGluePfcUtVerifyCnodeInstance
(
    IN  GT_U8 devNum,
    OUT GT_STATUS *stPtr
)
{
    UTF_PHYSICAL_PORT_NUM   port = 0;
    GT_U32                  tc = 0;
    GT_U32                  cNode;

    cpssOsMemSet(cNodesArr, 0, sizeof(cNodesArr));

    for (port = 0; port < UT_TM_GLUE_PFC_MAX_PORT_CNS; port++)
    {
        for (tc = 0; tc < UT_TM_GLUE_PFC_MAX_TC_CNS; tc++)
        {
            *stPtr = cpssDxChTmGluePfcTmTcPort2CNodeGet(devNum, port, tc, &cNode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, *stPtr, devNum, cNode);

            /* verify cNodeValueGet < UT_TM_GLUE_PFC_MAX_CNODE_CNS */
            if (cNode >= UT_TM_GLUE_PFC_MAX_CNODE_CNS)
            {
                *stPtr = GT_OUT_OF_RANGE;
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, *stPtr, "%d, %d, %d, %d", devNum, port, tc, cNode);
            }

            cNodesArr[cNode]++;
        }
    }

    /* verify cNode one instance */
    *stPtr = GT_OUT_OF_RANGE;
    for (cNode = 0; cNode < UT_TM_GLUE_PFC_MAX_CNODE_CNS; cNode++)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(1, cNodesArr[cNode], cNode);
    }

    *stPtr = GT_OK;
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGluePfcTmTcPort2CNodeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   tc,
    IN GT_U32                   cNodeValue
);
*/
UTF_TEST_CASE_MAC(cpssDxChTmGluePfcTmTcPort2CNodeSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1.1. Call with cNodeValue [0 and 511] and tc [0, 4].
    Expected: GT_OK.
    1.1.2. Call with cNodeValue [256] and out of range tc [CPSS_TC_RANGE_CNS = 8]
    Expected: GT_BAD_PARAM.
    1.1.3. Call with cNodeValue [512] and tc [4]
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st              = GT_OK;
    GT_U8       dev;
    UTF_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      tc              = 0;
    GT_U32      cNodeValue      = 0;
    GT_U32      cNodeValueGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssDxChTmGluePfcVerifyInit(dev);

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* API supports up to 64 ports */
            if(port > CPSS_CPU_PORT_NUM_CNS)
            {
                break;
            }

            /* verify c Node appear once in table */
            cpssDxChTmGluePfcUtVerifyCnodeInstance(dev, &st);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call with cNodeValue [0 and 511] and tc [0, 4].
               Expected: GT_OK.
            */

            /* Call function with cNodeValue = 0 and tc = 0, 7*/
            cNodeValue = 0;
            tc = 0;

            st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, tc, cNodeValue);

            st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValueGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tc);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cNodeValue, cNodeValueGet,
               "got another C node value then was set: %d", dev);

            /* Call function with cNodeValue = 511, tc = 4 */
            cNodeValue = 511;
            tc = 4;

            st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, tc, cNodeValue);

            st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValueGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tc);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cNodeValue, cNodeValueGet,
                       "got another C node value then was set: %d", dev);

            /* 1.1.2. Call with cNodeValue [256] and out of range tc [CPSS_TC_RANGE_CNS = 8]
               Expected: GT_BAD_PARAM.
            */

            cNodeValue = 256;
            tc = CPSS_TC_RANGE_CNS;

            st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, tc, cNodeValue);

            /*  1.1.3. Call with cNodeValue [512] and tc [4]
                Expected: GT_OUT_OF_RANGE.
            */
            cNodeValue = 512;
            tc = 4;

            st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, tc, cNodeValue);
        }

        cNodeValue = 256;
        tc = 2;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* cNodeValue = 256, tc = 2                 */
            st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* cNodeValue == 256, tc == 2                                 */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        /* cNodeValue == 256, tc == 2                                 */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0          */
    /* cNodeValue == 256  */
    /* tc == 2       */

    st = cpssDxChTmGluePfcTmTcPort2CNodeSet(dev, port, tc, cNodeValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGluePfcTmTcPort2CNodeGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   tc,
    OUT GT_U32                 *cNodeValuePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTmGluePfcTmTcPort2CNodeGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call function with non-NULL cNodeValuePtr.
    Expected: GT_OK.
    1.2. Call function with cNodeValuePtr [NULL] .
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st              = GT_OK;
    GT_U8       dev;
    UTF_PHYSICAL_PORT_NUM port  = 0;
    GT_U32      tc              = 0;
    GT_U32      cNodeValue;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*cpssDxChTmGluePfcVerifyInit(dev);*/

        tc = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* API supports up to 64 ports */
            if(port > CPSS_CPU_PORT_NUM_CNS)
            {
                break;
            }
            /*
               1.1.1. Call function with non-NULL cNodeValuePtr.
               Expected: GT_OK.
            */
            st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cNodeValue);

            /*
               1.1.2. Call function with cNodeValuePtr == 0.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        /* 1.5. For active device check that function returns GT_BAD_PARAM */
        /* for tc == 8.                                     */
        port = 0;
        tc = 8;

        st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    tc = 4;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGluePfcTmTcPort2CNodeGet(dev, port, tc, &cNodeValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGluePfcResponseModeSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode
);
*/
UTF_TEST_CASE_MAC(cpssDxChTmGluePfcResponseModeSet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with responseMode[CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E].
    Expected: GT_OK.
    1.2. Call cpssDxChTmGluePfcResponseModeGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;
    CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseModeGet = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with responseMode[CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E] */
        responseMode = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;

        st = cpssDxChTmGluePfcResponseModeSet(dev, 0, responseMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTmGluePfcResponseModeGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGluePfcResponseModeGet(dev, 0, &responseModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(responseMode, responseModeGet,
                                         "got another response mode: %d", responseModeGet);
        }

        /*
            1.3. Verify get correct value with Egress flow control enable
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueFlowControlEnableSet(dev, GT_TRUE, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGluePfcResponseModeGet(dev, 0, &responseModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(responseMode, responseModeGet,
                                         "got another response mode: %d", responseModeGet);
        }

        /* call with responseMode[CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E] */
        responseMode = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E;

        st = cpssDxChTmGluePfcResponseModeSet(dev, 0, responseMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call cpssDxChTmGluePfcResponseModeGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGluePfcResponseModeGet(dev, 0, &responseModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(responseMode, responseModeGet,
                                         "got another response mode: %d", responseModeGet);
        }

        /*
            1.3. Verify get correct value with Egress flow control disable
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChTmGlueFlowControlEnableSet(dev, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTmGluePfcResponseModeGet(dev, 0, &responseModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(responseMode, responseModeGet,
                                         "got another response mode: %d", responseModeGet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGluePfcResponseModeSet(dev, 0, responseMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGluePfcResponseModeSet(dev, 0, responseMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGluePfcResponseModeGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT *responseModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGluePfcResponseModeGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with non-null responseModePtr.
    Expected: GT_OK.
    1.2. Call with responseModePtr [NULL].
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode    = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTmGluePfcResponseModeGet(dev, 0, &responseMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with responseModePtr [NULL].
        Expected: NON GT_OK.
        */
        st = cpssDxChTmGluePfcResponseModeGet(dev, 0, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChTmGluePfcResponseModeGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGluePfcResponseModeGet(dev, 0, &responseMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGluePfcResponseModeGet(dev, 0, &responseMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGluePfcResponseModeSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode
);

GT_STATUS cpssDxChTmGluePfcResponseModeGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT *responseModePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTmGluePfcResponseModeSetAndGetByPort)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call with responseMode[CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E].
    Expected: GT_OK.
    1.2. Call cpssDxChTmGluePfcResponseModeGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;
    CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseModeGet = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;
    UTF_PHYSICAL_PORT_NUM physicalPort = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        if (!PRV_CPSS_SIP_5_15_CHECK_MAC(dev)) /* !IS_BOBK_DEV_MAC(dev) */
        {
            continue;
        }

        st = prvUtfNextPhyPortReset(&physicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&physicalPort, GT_TRUE))
        {
            /* call with responseMode[CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E]
               Expected: GT_OK.
            */
            responseMode = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;
            st = cpssDxChTmGluePfcResponseModeSet(dev, physicalPort, responseMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, physicalPort);

            /*
                Call cpssDxChTmGluePfcResponseModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChTmGluePfcResponseModeGet(dev, physicalPort, &responseModeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(responseMode, responseModeGet,
                                             "got another response mode: port: %d, mode:  %d", physicalPort, responseModeGet);
            }

            /* call with responseMode[CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E] */
            responseMode = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E;

            st = cpssDxChTmGluePfcResponseModeSet(dev, physicalPort, responseMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, physicalPort);

            /*
                Call cpssDxChTmGluePfcResponseModeGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChTmGluePfcResponseModeGet(dev, physicalPort, &responseModeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, physicalPort);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(responseMode, responseModeGet,
                                             "got another response mode: port: %d, mode:  %d", physicalPort, responseModeGet);
            }
        }

        /* Call with phisicalPort out of range
           Expected: GT_BAD_PARAM.
        */
        physicalPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) + 1;

        st = cpssDxChTmGluePfcResponseModeSet(dev, physicalPort, responseMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, physicalPort, dev);

        /* For active device check that function returns GT_OK */
        /* for CPU port number. */
        physicalPort = CPSS_CPU_PORT_NUM_CNS;

        responseMode = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;

        st = cpssDxChTmGluePfcResponseModeSet(dev, physicalPort, responseMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, physicalPort, dev);

        st = cpssDxChTmGluePfcResponseModeGet(dev, physicalPort, &responseModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, physicalPort);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(responseMode, responseModeGet,
                                         "got another response mode: port: %d, mode:  %d", physicalPort, responseModeGet);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    physicalPort = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGluePfcResponseModeSet(dev, physicalPort, responseMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGluePfcResponseModeSet(dev, physicalPort, responseMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGluePfcPortMappingSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_PHYSICAL_PORT_NUM     pfcPortNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChTmGluePfcPortMappingSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    UTF_PHYSICAL_PORT_NUM pfcPort = 0;
    UTF_PHYSICAL_PORT_NUM pfcPortGet = 0;
    UTF_PHYSICAL_PORT_NUM physicalPort = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&physicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev)) /* PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC */
        {
            st = cpssDxChTmGluePfcPortMappingSet(dev, 0, 0);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, physicalPort);
            continue;
        }

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&physicalPort, GT_TRUE))
        {
            /* 1.1.1. Call with pfcPort [0, 32, CPSS_CPU_PORT_NUM_CNS].
               Expected: GT_OK.
            */
            pfcPort = 0;

            st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, physicalPort, pfcPort);

            st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, physicalPort, pfcPortGet);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(pfcPort, pfcPortGet,
               "got wrong physical port value then was set: dev: %d, physicalPort: %d", dev, physicalPort);

            pfcPort = 32;

            st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, physicalPort, pfcPort);

            st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, physicalPort, pfcPortGet);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(pfcPort, pfcPortGet,
               "got wrong physical port value then was set: dev: %d, physicalPort: %d", dev, physicalPort);

            pfcPort = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, physicalPort, pfcPort);

            st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, physicalPort, pfcPortGet);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(pfcPort, pfcPortGet,
               "got wrong physical port value then was set: dev: %d, physicalPort: %d", dev, physicalPort);

            /* 1.1.2. Call with pfcPort out of range
               Expected: GT_BAD_PARAM.
            */

            pfcPort = CPSS_CPU_PORT_NUM_CNS + 1;

            st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pfcPort);
        }

        /* 1.1.2. Call with phisicalPort out of range
           Expected: GT_BAD_PARAM.
        */
        physicalPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev) + 1;

        st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pfcPort);


        pfcPort = 25;

        st = prvUtfNextPhyPortReset(&physicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&physicalPort, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* pfcPort = 25 */
            st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, physicalPort);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, physicalPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for pfcPort */
        physicalPort = 0;

        st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, (UTF_PHYSICAL_PORT_NUM)-1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pfcPort);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number. */
        /* physicalPort == 65 */
        physicalPort = CPSS_CPU_PORT_NUM_CNS;
        pfcPort = 47;

        st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, physicalPort);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    physicalPort = 0;
    pfcPort = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGluePfcPortMappingSet(dev, physicalPort, pfcPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGluePfcPortMappingGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_PHYSICAL_PORT_NUM    *pfcPortNumPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTmGluePfcPortMappingGet)
{
/*
    ITERATE_DEVICES (Bobcat2)
    1.1. Call function with non-NULL portNumPtr.
    Expected: GT_OK.
    1.2. Call function with pfcportNumPtr [NULL] .
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st              = GT_OK;
    GT_U8       dev;
    UTF_PHYSICAL_PORT_NUM pfcPort = 0;
    UTF_PHYSICAL_PORT_NUM pfcPortGet = 0;
    UTF_PHYSICAL_PORT_NUM physicalPort = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&physicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(!PRV_CPSS_SIP_5_10_CHECK_MAC(dev)) /* PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC */
        {
            st = cpssDxChTmGluePfcPortMappingGet(dev, 0, &pfcPortGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, physicalPort);
            continue;
        }

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&physicalPort, GT_TRUE))
        {
            /*
               1.1.1. Call function with non-NULL pfcPort.
               Expected: GT_OK.
            */
            st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, physicalPort);

            /* TODO: verify pfcPortGet <= CPSS_CPU_PORT_NUM_CNS
            UTF_VERIFY_LO2_STRING_MAC(pfcPort, <= CPSS_CPU_PORT_NUM_CNS + 1,
               "got wrong pfc port value then was set: dev: %d, physicalPort: %d", dev, physicalPort);*/

            /*
               1.1.2. Call function with NULL pfcPortPtr.
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, physicalPort);
        }

        /* 1.4. For active port check that function returns GT_OK */
        /* for CPU port number.                                     */
        physicalPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, physicalPort);


        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&physicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&physicalPort, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, physicalPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for physical port number. */
        st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pfcPort);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    physicalPort = 10;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGluePfcPortMappingGet(dev, physicalPort, &pfcPortGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTmGluePfc suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTmGluePfc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGluePfcTmTcPort2CNodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGluePfcTmTcPort2CNodeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGluePfcResponseModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGluePfcResponseModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGluePfcResponseModeSetAndGetByPort)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGluePfcPortMappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGluePfcPortMappingSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTmGluePfc)

