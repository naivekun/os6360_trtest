/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChPortLbUT.c
*
* DESCRIPTION:
*       Unit Tests for CPSS DxCh Forwarding to Loopback/Service Port feature
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortLoopback.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>

#define PRV_UTF_MAX_CPU_CODE 255
#define PRV_UTF_MAX_ANALYZER_INDEX 6
#define PRV_UTF_MAX_LB_PROFILE 3


/* loop-back ports iterator initializer.
   SIP5..SIP5.15: prvUtfNextTxqPortReset
   SIP5.20      : prvUtfNextPhyPortReset */
typedef GT_STATUS (*PRV_LB_PORT_ITER_RESET_FN)
(
    INOUT GT_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_U8        dev
);

/* loopback ports iterator
   SIP5..SIP5.15: prvUtfNextTxqPortGet
   SIP5.20      : prvUtfNextPhyPortGet. */
typedef GT_STATUS (*PRV_LB_PORT_ITER_NEXT_FN)
(
    INOUT GT_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_BOOL      activeIs
);

/* get loopback port iterator functions (reset and next)
   In case of SIP5.20 devices any physical port can be specified as loopback port
   In case of SIP5..SIP5.15 devices only physical port assigned with txq port can
   be specified as loopback port. So iterators are different:
   SIP5..SIP5.15: prvUtfNextTxqPortReset, prvUtfNextTxqPortGet
   SIP5.20      : prvUtfNextPhyPortReset, prvUtfNextPhyPortGet.
*/
static GT_VOID prvUtfLbPortIteratorGet
(
    IN  GT_U8 dev,
    OUT PRV_LB_PORT_ITER_RESET_FN *iterResetFn,
    OUT PRV_LB_PORT_ITER_NEXT_FN  *iterNextFn
)
{
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
    {
        *iterResetFn = prvUtfNextPhyPortReset;
        *iterNextFn  = prvUtfNextPhyPortGet;
    }
    else
    {
        *iterResetFn = prvUtfNextTxqPortReset;
        *iterNextFn  = prvUtfNextTxqPortGet;
    }
}



/*
GT_STATUS cpssDxChPortLoopbackEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  srcProfile,
    IN GT_U32  trgProfile,
    IN GT_U32  tc,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackEnableSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
      1.1. Call Set-function with valid parameters.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: G_OK.
      1.1. Call with invalid srcProfile.
           Expected: GT_BAD_PARAM.
      1.2. Call with invalid trgProfile.
           Expected: GT_BAD_PARAM.
      1.3. Call with invalid tc(trafic class)
           Expected: GT_BAD_PARAM.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_STATUS   st = GT_OK;
    GT_U32      notAppFamilyBmp;
    GT_U32      srcProfile=0;
    GT_U32      trgProfile=0;
    GT_U32      tcArr[3] = {0, 3, 7}; /* tested traffic classes */
    GT_U32      i;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check with valid parameters */
        for (srcProfile = 0; srcProfile <= PRV_UTF_MAX_LB_PROFILE; srcProfile++)
        {
            for (trgProfile = 0; trgProfile <= PRV_UTF_MAX_LB_PROFILE; trgProfile++)
            {
                for (i = 0; i < sizeof(tcArr)/sizeof(tcArr[0]); i++)
                {
                    for (enable = GT_FALSE; 1; enable = GT_TRUE)
                    {
                        st = cpssDxChPortLoopbackEnableSet(
                            dev, srcProfile, trgProfile, tcArr[i], enable);
                        UTF_VERIFY_EQUAL5_PARAM_MAC(
                            GT_OK, st, dev, srcProfile, trgProfile, tcArr[i], enable);

                        st = cpssDxChPortLoopbackEnableGet(
                            dev, srcProfile, trgProfile, tcArr[i], &enableGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(
                            GT_OK, st, dev, srcProfile, trgProfile, tcArr[i]);

                        UTF_VERIFY_EQUAL4_PARAM_MAC(
                            enable, enableGet, dev, srcProfile, trgProfile, tcArr[i]);

                        if (enable == GT_TRUE)
                        {
                            break;
                        }
                    }
                }
            }
        }
        /* assign valid parameters */
        srcProfile = 0;
        trgProfile = 0;

        /* 1.1 check with invalid srcProfile */
        st = cpssDxChPortLoopbackEnableSet(
            dev, PRV_UTF_MAX_LB_PROFILE+1, trgProfile, tcArr[0], enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.2 check with invalid trgProfile */
        st = cpssDxChPortLoopbackEnableSet(
            dev, srcProfile, PRV_UTF_MAX_LB_PROFILE+1, tcArr[0], enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 check with invalid tc */
        st = cpssDxChPortLoopbackEnableSet(dev, srcProfile, trgProfile, 9,
                                          enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackEnableSet(
            dev, srcProfile, trgProfile, tcArr[0], enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackEnableSet(dev, srcProfile, trgProfile, tcArr[0],
                                     enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPortLoopbackEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  srcProfile,
    IN GT_U32  trgProfile,
    IN GT_U32  tc,
    IN GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackEnableGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
      1.1. Call with valid parameters.
           Expected: G_OK.
      1.1. Call with invalid srcProfile.
           Expected: GT_BAD_PARAM.
      1.2. Call with invalid trgProfile.
           Expected: GT_BAD_PARAM.
      1.3. Call with invalid tc(trafic class)
           Expected: GT_BAD_PARAM.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_STATUS   st = GT_OK;
    GT_U32      notAppFamilyBmp;
    GT_U32      srcProfile=0;
    GT_U32      trgProfile=0;
    GT_U32      tcArr[3] = {0, 3, 7}; /* tested traffic classes */
    GT_BOOL     enable;
    GT_U32      i;
    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check with valid parameters */
        for (srcProfile = 0; srcProfile <= PRV_UTF_MAX_LB_PROFILE; srcProfile++)
        {
            for (trgProfile = 0; trgProfile <= PRV_UTF_MAX_LB_PROFILE; trgProfile++)
            {
                for (i = 0; i < sizeof(tcArr)/sizeof(tcArr[0]); i++)
                {
                    st = cpssDxChPortLoopbackEnableGet(
                        dev, srcProfile, trgProfile, tcArr[i], &enable);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(
                        GT_OK, st, dev, srcProfile, trgProfile, tcArr[i]);
                }
            }
        }
        /* assign valid parameters */
        srcProfile = 0;
        trgProfile = 0;

        /* 1.1 check with invalid srcProfile */
        st = cpssDxChPortLoopbackEnableGet(
            dev, PRV_UTF_MAX_LB_PROFILE+1, trgProfile, tcArr[0], &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.2 check with invalid trgProfile */
        st = cpssDxChPortLoopbackEnableGet(
            dev, srcProfile, PRV_UTF_MAX_LB_PROFILE+1, tcArr[0], &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 check with invalid tc */
        st = cpssDxChPortLoopbackEnableGet(dev, srcProfile, trgProfile, 9,
                                          &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackEnableGet(
            dev, srcProfile, trgProfile, tcArr[0], &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackEnableGet(
        dev, srcProfile, trgProfile, tcArr[0], &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet
(
    IN GT_U8                devNum,
    IN GT_BOOL              fromCpuEn,
    IN GT_BOOL              singleTargetEn,
    IN GT_BOOL              multiTargetEn
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
      1.1. Call Set-function with valid parameters.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: G_OK.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_STATUS   st = GT_OK;
    GT_U32      notAppFamilyBmp;

    GT_BOOL     fromCpuEn;
    GT_BOOL     singleTargetEn;
    GT_BOOL     multiTargetEn;

    GT_BOOL     fromCpuEnGet;
    GT_BOOL     singleTargetEnGet;
    GT_BOOL     multiTargetEnGet;

    GT_U32      i;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            /* for singleTargetEn in [false, true]:
                 for multiTargetEn in [false, true]:
                   for fromCpuEn in [false, true]:
                     ... */
            singleTargetEn = (i & BIT_4 ? GT_TRUE : GT_FALSE);
            multiTargetEn  = (i & BIT_2 ? GT_TRUE : GT_FALSE);
            fromCpuEn      = (i & BIT_1 ? GT_TRUE : GT_FALSE);

            st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
                dev, singleTargetEn, multiTargetEn, fromCpuEn);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st,
                dev, singleTargetEn, multiTargetEn, fromCpuEn);

            st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(
                dev, &singleTargetEnGet, &multiTargetEnGet, &fromCpuEnGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL3_PARAM_MAC(
                singleTargetEn, singleTargetEnGet, dev, multiTargetEn, fromCpuEn);
            UTF_VERIFY_EQUAL3_PARAM_MAC(
                multiTargetEn, multiTargetEnGet, dev, singleTargetEn, fromCpuEn);
            UTF_VERIFY_EQUAL3_PARAM_MAC(
                fromCpuEn, fromCpuEn, dev, singleTargetEn, multiTargetEn);
        }
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
            dev, GT_FALSE, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(
        dev, GT_FALSE, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
  GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet
  (
  IN  GT_U8                devNum,
  OUT GT_BOOL              *fromCpuEnPtr,
  OUT GT_BOOL              *singleTargetEnPtr,
  OUT GT_BOOL              *multiTargetEnPtr
  )
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
         Call with valid parameters.
         Expected: G_OK.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_STATUS   st = GT_OK;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     fromCpuEn;
    GT_BOOL     singleTargetEn;
    GT_BOOL     multiTargetEn;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(
            dev, &fromCpuEn, &singleTargetEn, &multiTargetEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(
            dev, &fromCpuEn, &singleTargetEn, &multiTargetEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(
            dev, &fromCpuEn, &singleTargetEn, &multiTargetEn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_PHYSICAL_PORT_NUM   loopackPortNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices over all available physical ports.
      1.1. Call Set-function with valid parameters.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: G_OK.
      1.2. Call with invalid portNum
           Expected: GT_BAD_PARAM
      1.3. Call with invalid loopbackPortNum
           Expected: GT_BAD_PARAM

      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_U32               notAppFamilyBmp;
    GT_PHYSICAL_PORT_NUM port;
    GT_PHYSICAL_PORT_NUM lbPort;    /* loopback port to be written */
    GT_PHYSICAL_PORT_NUM lbPortGet; /* loopback port to be read*/
    GT_STATUS            st;
    PRV_LB_PORT_ITER_RESET_FN lbPortIterResetFn; /* loopback port iterator reset function */
    PRV_LB_PORT_ITER_NEXT_FN lbPortIterNextFn;   /* loopback port iterator next function */
    GT_PHYSICAL_PORT_NUM validPort   = (GT_PHYSICAL_PORT_NUM)-1; /* valid port */
    GT_PHYSICAL_PORT_NUM validLbPort = (GT_PHYSICAL_PORT_NUM)-1; /* valid loopback port */

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* init loopback port iterator functions */
        prvUtfLbPortIteratorGet(dev, &lbPortIterResetFn, &lbPortIterNextFn);

        /* 1.1. go over all valid physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            validPort = port;
            /* go over all valid loopback ports */
            st = lbPortIterResetFn(&lbPort, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            while(GT_OK == lbPortIterNextFn(&lbPort, GT_TRUE))
            {
                st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
                    dev, port, lbPort);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lbPort);

                st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
                    dev, port, &lbPortGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL2_PARAM_MAC(lbPort, lbPortGet, dev, port);

                validLbPort = lbPort;
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2. Call with invalid portNum */
            st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
                dev, port, validLbPort);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, validLbPort);
        }

        st = lbPortIterResetFn(&lbPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == lbPortIterNextFn(&lbPort, GT_FALSE))
        {
            /* 1.3. Call with invalid loopbackPortNum */
            st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
                dev, validPort, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, validPort, lbPort);
        }
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
            dev, validPort, validLbPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
            dev, validPort, validLbPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               loopackPortNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices.
      1.1. Call with all available physical ports.
           Expected: G_OK.
      1.2. Call with invalid portNum
           Expected: GT_BAD_PARAM
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_U32               notAppFamilyBmp;
    GT_PHYSICAL_PORT_NUM port;
    GT_PHYSICAL_PORT_NUM lbPort; /* loopback port */
    GT_STATUS            st;
    GT_PHYSICAL_PORT_NUM validPort = (GT_PHYSICAL_PORT_NUM)-1;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. go over all valid physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            validPort = port;
            st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
                dev, port, &lbPort);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2. Call with invalid portNum */
            st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
                dev, port, &lbPort);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
            dev, validPort, &lbPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet(
            dev, validPort, &lbPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackPktTypeToCpuSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN GT_PHYSICAL_PORT_NUM     loopbackPortNum,
    IN GT_BOOL                  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeToCpuSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices.
      1.1. Call Set-function with valid CPU code (0..255), loopoback port.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: G_OK.
      1.2. Check CPU code
      1.3. Call with invalid CPU code
           Expected: GT_BAD_PARAM
      1.4. Call with invalid loobackPortNum
           Expected: GT_BAD_PARAM
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_U32               notAppFamilyBmp;
    GT_PHYSICAL_PORT_NUM lbPort;    /* loopback port to be writtern */
    GT_PHYSICAL_PORT_NUM lbPortGet; /* loopback port to be read*/
    GT_U32               i;
    GT_BOOL              enable;
    GT_BOOL              enableGet;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    GT_STATUS            st;
    PRV_LB_PORT_ITER_RESET_FN lbPortIterResetFn; /* loopback port iterator reset function */
    PRV_LB_PORT_ITER_NEXT_FN  lbPortIterNextFn;  /* loopback port iterator next function */

    GT_PHYSICAL_PORT_NUM     validLbPort   = (GT_PHYSICAL_PORT_NUM)-1;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* init loopback port iterator functions */
        prvUtfLbPortIteratorGet(dev, &lbPortIterResetFn, &lbPortIterNextFn);

        /* check all CPU codes */
        for (cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
             cpuCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
             cpuCode++)
        {
            /* go over all valid loopback ports */
            st = lbPortIterResetFn(&lbPort, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            while(GT_OK == lbPortIterNextFn(&lbPort, GT_TRUE))
            {
                for (i = 0, enable = GT_FALSE; i<2; i++, enable = GT_TRUE)
                {
                    st = cpssDxChPortLoopbackPktTypeToCpuSet(
                        dev, cpuCode, lbPort, enable);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(
                        GT_OK, st, dev, cpuCode, lbPort, enable);

                    st = cpssDxChPortLoopbackPktTypeToCpuGet(
                        dev, cpuCode, &lbPortGet, &enableGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cpuCode);

                    UTF_VERIFY_EQUAL3_PARAM_MAC(
                        lbPort, lbPortGet, dev, cpuCode, enable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(
                        enable, enableGet, dev, cpuCode, lbPort);
                }
                validLbPort = lbPort;
            }
        }

        /* 1.2. check cpuCode = CPSS_NET_ALL_CPU_OPCODES_E */
        lbPort = validLbPort;
        for (i=0, enable = GT_TRUE; i<2; i++, enable = GT_FALSE)
        {
            st = cpssDxChPortLoopbackPktTypeToCpuSet(
                dev, CPSS_NET_ALL_CPU_OPCODES_E, lbPort, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(
                GT_OK, st, dev, cpuCode, lbPort, enable);

            for (cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
                 cpuCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
                 cpuCode++)
            {
                st = cpssDxChPortLoopbackPktTypeToCpuGet(
                    dev, cpuCode, &lbPortGet, &enableGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cpuCode);

                UTF_VERIFY_EQUAL3_PARAM_MAC(
                    lbPort, lbPortGet, dev, cpuCode, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(
                    enable, enableGet, dev, cpuCode, lbPort);
            }
        }

        /* 1.3. call with invalid CPU code */
        st = cpssDxChPortLoopbackPktTypeToCpuSet(
            dev, PRV_UTF_MAX_CPU_CODE + 1, validLbPort, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, validLbPort);


        st = lbPortIterResetFn(&lbPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == lbPortIterNextFn(&lbPort, GT_FALSE))
        {
            /* 1.4. call with invalid loopback port */
            st = cpssDxChPortLoopbackPktTypeToCpuSet(
                dev, 0, lbPort, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, lbPort);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeToCpuSet(
            dev, 0, validLbPort, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackPktTypeToCpuSet(
            dev, 0, validLbPort, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackPktTypeToCpuGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT GT_PHYSICAL_PORT_NUM     *loopbackPortNumPtr,
    OUT GT_BOOL                  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeToCpuGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices.
      1.1. Call with valid parameters.
           Expected: G_OK.
      1.2. Call with invalid CPU code
           Expected: GT_BAD_PARAM
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_U32               notAppFamilyBmp;
    GT_U32               cpuCode;
    GT_PHYSICAL_PORT_NUM lbPort; /* loopback port */
    GT_BOOL              enable;
    GT_STATUS            st;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check all CPU codes */
        for (cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
             cpuCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
             cpuCode++)
        {
            st = cpssDxChPortLoopbackPktTypeToCpuGet(
                dev, cpuCode, &lbPort, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cpuCode);
        }
        /* 1.2. call with invalid CPU code */
        st = cpssDxChPortLoopbackPktTypeToCpuGet(
            dev, PRV_UTF_MAX_CPU_CODE + 1, &lbPort, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, lbPort);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeToCpuGet(
            dev, 0, &lbPort, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackPktTypeToCpuGet(
            dev, 0, &lbPort, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackPktTypeToAnalyzerSet
(
    IN GT_U8                devNum,
    IN GT_U32               analyzerIndex,
    IN GT_PHYSICAL_PORT_NUM loopbackPortNum,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeToAnalyzerSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices.
      1.1. Call Set-function with valid parameters.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: G_OK.
      1.2. Call with invalid analyzer index
           Expected: GT_BAD_PARAM
      1.3. Call with invalid loopback port
           Expected: GT_BAD_PARAM
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_U32               notAppFamilyBmp;
    GT_U32               analyzerIndex;
    GT_PHYSICAL_PORT_NUM lbPort;    /* loopback port to be written*/
    GT_PHYSICAL_PORT_NUM lbPortGet; /* loopback port to be read */
    GT_BOOL              enable;
    GT_BOOL              enableGet;
    GT_U32               i;
    GT_STATUS            st;
    PRV_LB_PORT_ITER_RESET_FN lbPortIterResetFn; /* loopback port iterator reset function */
    PRV_LB_PORT_ITER_NEXT_FN  lbPortIterNextFn;   /* loopback port iterator next function */
    GT_PHYSICAL_PORT_NUM validLbPort = (GT_PHYSICAL_PORT_NUM)-1;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* init loopback port iterator functions */
        prvUtfLbPortIteratorGet(dev, &lbPortIterResetFn, &lbPortIterNextFn);

        /* check all analyzer indexes */
        for (analyzerIndex = 0; analyzerIndex <= PRV_UTF_MAX_ANALYZER_INDEX; analyzerIndex++)
        {
            /* go over all valid loopback ports */
            st = lbPortIterResetFn(&lbPort, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            while(GT_OK == lbPortIterNextFn(&lbPort, GT_TRUE))
            {
                for (i = 0, enable = GT_FALSE; i < 2; i++, enable = GT_TRUE)
                {
                    st = cpssDxChPortLoopbackPktTypeToAnalyzerSet(
                        dev, analyzerIndex, lbPort, enable);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(
                        GT_OK, st, dev, analyzerIndex, lbPort, i);

                    st = cpssDxChPortLoopbackPktTypeToAnalyzerGet(
                        dev, analyzerIndex, &lbPortGet, &enableGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, analyzerIndex);

                    UTF_VERIFY_EQUAL3_PARAM_MAC(
                        lbPort ,lbPortGet, dev, analyzerIndex, enable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(
                        enable, enableGet ,lbPortGet, dev, analyzerIndex);

                }
                validLbPort = lbPort;
            }
        }
        /* 1.2. call with invalid analyzer index */
        st = cpssDxChPortLoopbackPktTypeToAnalyzerSet(
            dev, PRV_UTF_MAX_ANALYZER_INDEX + 1, validLbPort, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, validLbPort);

        st = lbPortIterResetFn(&lbPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == lbPortIterNextFn(&lbPort, GT_FALSE))
        {
            /* 1.3. Call with invalid loopback port */
            st = cpssDxChPortLoopbackPktTypeToAnalyzerSet(
                dev, 0, lbPort, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, lbPort);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeToAnalyzerSet(
            dev, 0, validLbPort, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackPktTypeToAnalyzerSet(
            dev, 0, validLbPort, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPortLoopbackPktTypeToAnalyzerGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               analyzerIndex,
    OUT GT_PHYSICAL_PORT_NUM *loopbackPortNumPtr,
    OUT GT_BOOL              *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackPktTypeToAnalyzerGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices.
      1.1. Call with valid parameters.
           Expected: G_OK.
      1.2. Call with invalid analyzer index
           Expected: GT_BAD_PARAM
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_U32               notAppFamilyBmp;
    GT_U32               analyzerIndex;
    GT_PHYSICAL_PORT_NUM lbPort; /* loopback port */
    GT_BOOL              enable;
    GT_STATUS            st;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check all analyzer indexes */
        for (analyzerIndex = 0; analyzerIndex <= PRV_UTF_MAX_ANALYZER_INDEX; analyzerIndex++)
        {
            st = cpssDxChPortLoopbackPktTypeToAnalyzerGet(
                dev, analyzerIndex, &lbPort, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, analyzerIndex);
        }

        /* 1.2. call with invalid analyzer index */
        st = cpssDxChPortLoopbackPktTypeToAnalyzerGet(
            dev, PRV_UTF_MAX_ANALYZER_INDEX + 1, &lbPort, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackPktTypeToAnalyzerGet(dev, 0, &lbPort, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackPktTypeToAnalyzerGet(dev, 0, &lbPort, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DIRECTION_ENT   direction,
    IN  GT_U32               profile
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackProfileSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
      1.1. Call Set-function with valid parameters.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: G_OK.
      1.2  Call with wrong profile
           Expected: GT_BAD_PARAM
      1.3  Call with wrong direction
           Expected: GT_BAD_PARAM
      1.4. Call with wrong portNum
           Expected: GT_BAD_PARAM
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32               srcProfile; /* source loopback profile */
    GT_U32               srcProfileGet; /* source loopback profile to be read*/
    GT_U32               trgProfile; /* target loopback profile */
    GT_U32               trgProfileGet;  /* target loopback profile to be read*/
    GT_STATUS            st;
    GT_U32               i;
    GT_U32               notAppFamilyBmp;
    GT_PHYSICAL_PORT_NUM validPort = (GT_PHYSICAL_PORT_NUM)-1;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* got over all active ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* check for profiles 0,3 */
            for (i = 0; i < 2; i++)
            {
                /* 1.1. Call with valid parameters */

                /* assign source, target profiles with non-equal values */
                srcProfile = i    * PRV_UTF_MAX_LB_PROFILE;
                trgProfile = (1-i)* PRV_UTF_MAX_LB_PROFILE;

                /* assign source profile */
                st = cpssDxChPortLoopbackProfileSet(
                    dev, port,  CPSS_DIRECTION_INGRESS_E, srcProfile);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, srcProfile);

                /* assign target profile */
                st = cpssDxChPortLoopbackProfileSet(
                    dev, port,  CPSS_DIRECTION_EGRESS_E, trgProfile);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, trgProfile);

                /* read source profile and check it is the same as it was
                 * assigned before. A reading is done after target profile
                 * assignment to ensure source profile is not rewritten */
                st = cpssDxChPortLoopbackProfileGet(
                    dev, port,  CPSS_DIRECTION_INGRESS_E, &srcProfileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(srcProfile, srcProfileGet,
                                            dev, port);


                /* assign source profile again to ensure it doesn't
                 * rewrite target profile */
                st = cpssDxChPortLoopbackProfileSet(
                    dev, port,  CPSS_DIRECTION_INGRESS_E, srcProfile);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, srcProfile);

                /* read target profile and check it is the same as it was
                 * assigned before */
                st = cpssDxChPortLoopbackProfileGet(
                    dev, port,  CPSS_DIRECTION_EGRESS_E, &trgProfileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(trgProfile, trgProfileGet,
                                            dev, port);

                /* call Set with direction = BOTH. Check both source, target
                 * loopback profiles are the same*/
                st = cpssDxChPortLoopbackProfileSet(
                    dev, port,  CPSS_DIRECTION_BOTH_E, srcProfile);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, srcProfile);

                st = cpssDxChPortLoopbackProfileGet(
                    dev, port,  CPSS_DIRECTION_INGRESS_E, &srcProfileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                st = cpssDxChPortLoopbackProfileGet(
                    dev, port,  CPSS_DIRECTION_EGRESS_E, &trgProfileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL2_PARAM_MAC(srcProfile,    srcProfileGet,
                                            dev, port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(srcProfileGet, trgProfileGet,
                                            dev, port);
            }
            validPort = port;

            /* 1.2  Call with wrong profile */
            st = cpssDxChPortLoopbackProfileSet(
                dev, port, CPSS_DIRECTION_INGRESS_E , PRV_UTF_MAX_LB_PROFILE + 1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.2  Call with wrong direction */
            st = cpssDxChPortLoopbackProfileSet(
                dev, port, CPSS_DIRECTION_BOTH_E+1, 0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.3  Call with wrong direction */
            st = cpssDxChPortLoopbackProfileSet(dev, port,
                                              0, PRV_UTF_MAX_LB_PROFILE + 1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        }


        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.4. Call with wrong portNum */
            st = cpssDxChPortLoopbackProfileSet(dev, port,
                                                CPSS_DIRECTION_INGRESS_E, 0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackProfileSet(dev, validPort,
                                            CPSS_DIRECTION_INGRESS_E, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackProfileSet(dev, validPort,
                                        CPSS_DIRECTION_INGRESS_E, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *srcProfilePtr,
    OUT GT_U32               *trgProfilePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackProfileGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
      1.1. Iterate all available physical ports.
           Expected: G_OK.
      1.2. Call with wrong direction
           Expected: GT_BAD_PARAM
      1.3. Call with wrong portNum
           Expected: GT_BAD_PARAM
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_PHYSICAL_PORT_NUM validPort = (GT_PHYSICAL_PORT_NUM)-1;
    GT_U32               profile;
    GT_STATUS            st;
    GT_U32               notAppFamilyBmp;
    CPSS_DIRECTION_ENT   direction;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* got over all active ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call with valid parameters */
            for (direction = CPSS_DIRECTION_INGRESS_E;
                 direction <= CPSS_DIRECTION_EGRESS_E;
                 direction++)
            {
                st = cpssDxChPortLoopbackProfileGet(dev, port,
                                                    direction, &profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* 1.2 call with wrong direction */
            st = cpssDxChPortLoopbackProfileGet(dev, port, direction, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            validPort = port;
        }


        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2. Call with wrong portNum */
            st = cpssDxChPortLoopbackProfileGet(dev, port, CPSS_DIRECTION_INGRESS_E,
                                           &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackProfileGet(
            dev, validPort, CPSS_DIRECTION_INGRESS_E, &profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackProfileGet(
        dev, validPort, CPSS_DIRECTION_INGRESS_E, &profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackEvidxMappingSet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vidxOffset,
    IN  GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackEvidxMappingSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
      1.1. Call Set-function with valid parameters.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: GT_OK.
      1.2. Call vith invalid vidx
           Expected: GT_BAD_PARAM.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_STATUS   st = GT_OK;
    GT_U32      vidxMax;
    GT_U32      vidxOffset;
    GT_U32      vidxOffsetGet;
    GT_U32      notAppFamilyBmp;
    GT_U32      i;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        vidxMax = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.vidxNum-1;
        for (i = 0; i< 2; i++)
        {
            enable = BIT2BOOL_MAC(i);

            /* 1.1. Call Set-function with valid parameters.
                    Call Get-function to ensure read values are the same as
                    written ones. */

            /* check vidx = {0, maxVidx} */
            for (vidxOffset = 0; vidxOffset <= vidxMax; vidxOffset+= vidxMax)
            {
                st = cpssDxChPortLoopbackEvidxMappingSet(
                    dev, vidxOffset, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(
                    GT_OK, st, dev, vidxOffset, enable);

                st = cpssDxChPortLoopbackEvidxMappingGet(
                    dev, &vidxOffsetGet, &enableGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL2_PARAM_MAC(vidxOffset, vidxOffsetGet,
                                            dev, enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(enable, enableGet,
                                            dev, vidxOffset);
            }

            /* 1.2. Call with invalid vidx */
            vidxOffset = vidxMax + 1;
            st = cpssDxChPortLoopbackEvidxMappingSet(dev, vidxOffset, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st,
                                        dev, vidxOffset, enable);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackEvidxMappingSet(dev, 0, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackEvidxMappingSet(dev, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChPortLoopbackEvidxMappingGet
(
    IN  GT_U8                devNum,
    OUT GT_U16               *vidxPtr
    OUT GT_BOOL              *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackEvidxMappingGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
          Expected: GT_OK.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_STATUS   st = GT_OK;
    GT_U32      vidxOffset;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = cpssDxChPortLoopbackEvidxMappingGet(dev, &vidxOffset, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackEvidxMappingGet(dev, &vidxOffset, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackEvidxMappingGet(dev, &vidxOffset, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackEnableEgressMirroringSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackEnableEgressMirroringSet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
      1.1. Call Set-function with valid parameters.
           Call Get-function to ensure read values are the same as
           written ones.
           Expected: G_OK.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_STATUS   st = GT_OK;
    GT_U32      notAppFamilyBmp;
    GT_U32      i;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; i< 2; i++)
        {
            enable = (i ? GT_TRUE: GT_FALSE);
            st = cpssDxChPortLoopbackEnableEgressMirroringSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            st = cpssDxChPortLoopbackEnableEgressMirroringGet(dev, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackEnableEgressMirroringSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackEnableEgressMirroringSet(dev, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPortLoopbackEnableEgressMirroringGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortLoopbackEnableEgressMirroringGet)
{
    /*
      ITERATE_DEVICES (sip5):
      1. Iterate active devices
         Expected: G_OK.
      2. Call for for not-active/not-applicable devices
         Expected: GT_NOT_APPLICABLE_DEVICE.
      3. Call from non-applicable families.
         Expected: GT_NOT_APPLICABLE_DEVICE.
      4. Call with out of bound value for device id.
         Expected: GT_BAD_PARAM.
    */

    GT_U8       dev;
    GT_BOOL     enable;
    GT_STATUS   st = GT_OK;
    GT_U32      notAppFamilyBmp;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortLoopbackEnableEgressMirroringGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortLoopbackEnableEgressMirroringGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* call with invalid device number */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortLoopbackEnableEgressMirroringGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortLoopback suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortLoopback)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeToCpuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeToCpuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeToAnalyzerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackPktTypeToAnalyzerGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackEvidxMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackEvidxMappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackEnableEgressMirroringSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortLoopbackEnableEgressMirroringGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortLoopback)
