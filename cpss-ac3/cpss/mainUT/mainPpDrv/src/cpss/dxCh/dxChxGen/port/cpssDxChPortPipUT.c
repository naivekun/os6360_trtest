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
* @file cpssDxChPortPipUT.c
*
* @brief Unit tests for cpssDxChPortPip, that provides
* CPSS implementation for Pre-Ingress Prioritization (PIP) .
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

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* PIP : prepare iterator for go over all active devices */
#define UT_PIP_RESET_DEV(_devPtr)  \
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC((_devPtr),     \
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | \
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |      \
        UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E)

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEnableSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet  = GT_FALSE;

    /* there is no RXDMA/IA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(enable = GT_FALSE ; enable <= GT_TRUE ; enable++)
        {
            st = cpssDxChPortPipGlobalEnableSet(dev, enable);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, enable);
            }

            st = cpssDxChPortPipGlobalEnableGet(dev, &enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEnableGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with stack [IPv4, IPv6, IPv46].
           Expected: GT_OK.
        */

        st = cpssDxChPortPipGlobalEnableGet(dev,  &enable);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        st = cpssDxChPortPipGlobalEnableGet(dev, NULL);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, enable);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, enable);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static GT_U32       maxIndexArr[CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E+1] = {
/*CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E */ 4,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_MPLS_E     */ 2,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_IPV4_E     */ 1,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_IPV6_E     */ 1,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E      */ 4};
static GT_BOOL      isCheckedIndexArr[CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E+1] = {
/*CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E */ GT_TRUE,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_MPLS_E     */ GT_TRUE,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_IPV4_E     */ GT_FALSE,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_IPV6_E     */ GT_FALSE,
/*CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E      */ GT_TRUE};
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    IN GT_U16       etherType
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEtherTypeProtocolSet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol;
    GT_U32       index;
    GT_U16       etherType;
    GT_U16       etherTypeGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        etherType = 0x5555;
        for(protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E ;
            protocol <= CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E ;
            protocol ++)
        {
            /* check valid protocol */
            for(index = 0 ; index < maxIndexArr[protocol] ; index ++)
            {
                etherType++;

                /* check valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, protocol,index,etherType);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
                }
            }

            if(isCheckedIndexArr[protocol] == GT_TRUE)
            {
                /* check NOT valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index,etherType);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
                }
            }
            else
            {
                /* index is ignored anyway */
                index = BIT_31;
                st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,(etherType+0x7777));
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, protocol,index,(etherType+0x7777));
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,(etherType+0x7777));
                }
            }

        }

        /* check that all values are 'saved' */
        etherType = 0x5555;
        for(protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E ;
            protocol <= CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E ;
            protocol ++)
        {
            /* check valid protocol */
            for(index = 0 ; index < maxIndexArr[protocol] ; index ++)
            {
                etherType++;

                /* check valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, protocol,index,etherType);
                    /* compare results */
                    if(isCheckedIndexArr[protocol] == GT_TRUE)
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(etherType, etherTypeGet, dev, protocol,index,etherType);
                    }
                    else
                    {
                        /* the 0x7777 came due to override when index == BIT_31*/
                        UTF_VERIFY_EQUAL4_PARAM_MAC((etherType+0x7777), etherTypeGet, dev, protocol,index,etherType);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
                }
            }
        }

        etherType = 0x8888;
        index = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalEtherTypeProtocolSet
                            (dev, protocol,index,etherType),
                            protocol);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E;
    index = 0;
    etherType = 0x6666;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index,etherType);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEtherTypeProtocolSet(dev, protocol,index,etherType);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index,etherType);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalEtherTypeProtocolGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol;
    GT_U32       index;

    GT_U16       etherTypeGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E ;
            protocol <= CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E ;
            protocol ++)
        {
            /* check valid protocol */
            for(index = 0 ; index < maxIndexArr[protocol] ; index ++)
            {
                /* check valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }

                /* check NULL pointer */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,NULL);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }
            }

            if(isCheckedIndexArr[protocol] == GT_TRUE)
            {
                /* check NOT valid index */
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }
            }
            else
            {
                /* index is ignored anyway */
                index = BIT_31;
                st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, protocol,index);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, protocol,index);
                }
            }
        }


        index = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalEtherTypeProtocolGet
                            (dev, protocol,index,&etherTypeGet),
                            protocol);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    protocol = CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E;
    index = 0;


    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalEtherTypeProtocolGet(dev, protocol,index,&etherTypeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipTrustEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipTrustEnableSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     trust;
    GT_BOOL     trustGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            for(trust = GT_FALSE ; trust <= GT_TRUE ; trust ++)
            {
                st = cpssDxChPortPipTrustEnableSet(dev,port,trust);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,trust);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,trust);
                }

                st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                    /* compare results */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(trust, trustGet, dev,port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
                }

            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        trust = GT_TRUE;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipTrustEnableSet(dev,port,trust);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,trust);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,trust);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    trust = GT_TRUE;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipTrustEnableSet(dev, port,trust);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipTrustEnableSet(dev, port,trust);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipTrustEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipTrustEnableGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     trustGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipTrustEnableGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipTrustEnableGet(dev,port,&trustGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipTrustEnableGet(dev, port,&trustGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipTrustEnableGet(dev, port,&trustGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  pipProfile
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipProfileSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     pipProfile;
    GT_U32     pipProfileGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            for(pipProfile = 0 ; pipProfile < 4 ; pipProfile ++)
            {
                st = cpssDxChPortPipProfileSet(dev,port,pipProfile);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,pipProfile);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,pipProfile);
                }

                st = cpssDxChPortPipProfileGet(dev,port,&pipProfileGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                    /* compare results */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(pipProfile, pipProfileGet, dev,port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
                }

            }

            /* check not valid profile */
            st = cpssDxChPortPipProfileSet(dev,port,pipProfile);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,port,pipProfile);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,pipProfile);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        pipProfile = 0;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipProfileSet(dev,port,pipProfile);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,pipProfile);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,pipProfile);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    pipProfile = 0;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipProfileSet(dev, port,pipProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipProfileSet(dev, port,pipProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *pipProfilePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipProfileGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     pipProfileGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipProfileGet(dev,port,&pipProfileGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* check NULL */
            st = cpssDxChPortPipProfileGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipProfileGet(dev,port,&pipProfileGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipProfileGet(dev, port,&pipProfileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipProfileGet(dev, port,&pipProfileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipVidClassificationEnableSet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            for(enable = GT_FALSE ; enable <= GT_TRUE ; enable ++)
            {
                st = cpssDxChPortPipVidClassificationEnableSet(dev,port,enable);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,enable);
                }

                st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
                    /* compare results */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(enable, enableGet, dev,port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
                }

            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        enable = GT_TRUE;
        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipVidClassificationEnableSet(dev,port,enable);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,enable);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipVidClassificationEnableSet(dev, port,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipVidClassificationEnableSet(dev, port,enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipVidClassificationEnableGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = 0;
    GT_BOOL     enableGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }

            /* called with NULL */
            st = cpssDxChPortPipVidClassificationEnableGet(dev,port,&enableGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipVidClassificationEnableGet(dev, port,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipVidClassificationEnableGet(dev, port,&enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipPrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  priority
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipPrioritySet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     priority;
    GT_U32     priorityGet;
    GT_U32     maxPriority;
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
        }
        else
        {
            maxPriority = 3;
        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E ;
                type <= CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E ;
                type ++)
            {
                /* check valid priority */
                for(priority = 0 ; priority < maxPriority ; priority++)
                {
                    st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,type);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                    }

                    st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,type);
                        /* compare results */
                        UTF_VERIFY_EQUAL3_PARAM_MAC(priority, priorityGet, dev,port,type);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                    }
                }

                /* check invalid priority */
                st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,port,type);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                }

            }


            /* check not valid type */
            priority = 0;
            st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,type);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
            }

            /*
                1.3. Call with wrong enum values type.
                Expected: NOT GT_OK.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPipPrioritySet
                                (dev,port,type,priority),
                                type);

        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;
        priority = 0;

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,priority);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,priority);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    priority = 0;
    port = 0;
    type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipPrioritySet(dev,port,type,priority);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipPriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    OUT GT_U32                  *priorityPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipPriorityGet)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = 0;
    GT_U32     priorityGet;
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E ;
                type <= CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E ;
                type ++)
            {
                st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,port,type);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                }

                /* check NULL pointer */
                st = cpssDxChPortPipPriorityGet(dev,port,type,NULL);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev,port,type);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
                }

            }

            /* check not valid type */
            st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev,port,type);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port,type);
            }

            /*
                1.3. Call with wrong enum values type.
                Expected: NOT GT_OK.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPipPriorityGet
                                (dev,port,type,&priorityGet),
                                type);

        }

        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;

        /* 1.1. For all active devices go over all "not available" 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,port);
            }
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;
    type = CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipPriorityGet(dev,port,type,&priorityGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static GT_U32       maxFieldIndexArr[CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E+1] = {
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E*/ 128,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E*/            8,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E*/       8,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E*/          8,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E*/        256,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E*/         256,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E*/         4,
    /*CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E */     4
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    IN  GT_U32                  priority
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalProfilePrioritySet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32                  pipProfile;
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type;
    GT_U32                  fieldIndex;
    GT_U32                  priority;
    GT_U32                  priorityGet;
    GT_U32                  maxPriority;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
        }
        else
        {
            maxPriority = 3;
        }

        /* check valid profile */
        for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                type++)
            {
                /* check valid fieldIndex */
                for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                {
                    if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                        fieldIndex >= 64 && /* instead of 256 */
                        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                    {
                        break;
                    }

                    /* check valid priority */
                    for(priority = 0 ; priority < maxPriority ; priority++)
                    {
                        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                    }

                    /* check not valid priority */
                    st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, pipProfile,type,fieldIndex,priority);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                    }

                }/*fieldIndex*/

                priority = 0;
                /* check not valid fieldIndex */
                st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex,priority);
                }
                else
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                }
            }/*type*/

            priority = 0;
            fieldIndex = 0;
            /* check not valid type */
            st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex,priority);
            }
            else
            {
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
            }

        }/*pipProfile*/

        priority = 0;
        fieldIndex = 0;
        type = 0;
        /* check not valid pipProfile */
        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex,priority);
        }
        else
        {
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
        }


        /* check that values are saved */
        /* set different profile to different fields ... and check at the end that
           the 'get' values match */
        {
            priority = 0;
            for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
            {
                for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                    type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                    type++)
                {
                    for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                    {
                        if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                            fieldIndex >= 64 && /* instead of 256 */
                            GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                                PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                        {
                            break;
                        }
                        priority++;
                        priority %= maxPriority;
                        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
                        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex,priority);
                        }
                    }/*fieldIndex*/
                }/*type*/
            }/*pipProfile*/

            priority = 0;
            for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
            {
                for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                    type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                    type++)
                {
                    for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                    {
                        if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                            fieldIndex >= 64 && /* instead of 256 */
                            GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                                PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                        {
                            break;
                        }
                        priority++;
                        priority %= maxPriority;
                        st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
                        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                        {
                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex);
                            /* compare the results */
                            UTF_VERIFY_EQUAL4_PARAM_MAC(priority, priorityGet, dev, pipProfile,type,fieldIndex);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
                        }
                    }/*fieldIndex*/
                }/*type*/
            }/*pipProfile*/
        }

        pipProfile = 0;
        fieldIndex = 0;
        priority = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalProfilePrioritySet
                            (dev ,pipProfile,type,fieldIndex,priority),
                            type);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    pipProfile = 0;
    fieldIndex = 0;
    priority = 0;
    type = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalProfilePrioritySet(dev ,pipProfile,type,fieldIndex,priority);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalProfilePriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *priorityPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalProfilePriorityGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32                  pipProfile;
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type;
    GT_U32                  fieldIndex;
    GT_U32                  priorityGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check valid profile */
        for(pipProfile = 0 ; pipProfile < 4 ; pipProfile++)
        {
            /* check valid type */
            for(type = CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E ;
                type <= CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E ;
                type++)
            {
                /* check valid fieldIndex */
                for(fieldIndex = 0 ; fieldIndex < maxFieldIndexArr[type] ; fieldIndex++)
                {
                    if(type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E &&
                        fieldIndex >= 64 && /* instead of 256 */
                        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
                            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
                    {
                        break;
                    }

                    st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, pipProfile,type,fieldIndex);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
                    }
                }/*fieldIndex*/

                /* check not valid fieldIndex */
                st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
                }
            }/*type*/

            fieldIndex = 0;
            /* check not valid type */
            st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
            }

        }/*pipProfile*/

        fieldIndex = 0;
        type = 0;
        /* check not valid pipProfile */
        st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
        }

        pipProfile = 0;
        fieldIndex = 0;
        /*
            1.3. Call with wrong enum values type.
            Expected: NOT GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPortPipGlobalProfilePriorityGet
                            (dev ,pipProfile,type,fieldIndex,&priorityGet),
                            type);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    pipProfile = 0;
    fieldIndex = 0;
    type = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, pipProfile,type,fieldIndex);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalProfilePriorityGet(dev ,pipProfile,type,fieldIndex,&priorityGet);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, pipProfile,type,fieldIndex);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U16                  vid
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalVidClassificationSet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    GT_U32       vidIndex;
    GT_U16       vidArr[3] = {_2K/*ok*/,_4K-1/*ok*/,_4K/*out of range*/};
    GT_U16       vidGet;


    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 4 ; index ++)
        {
            for(vidIndex = 0 ; vidIndex < 3; vidIndex++)
            {
                st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    if(vidIndex != 2)
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index,vidArr[vidIndex]);
                    }
                    else  /*vidIndex == 2*/
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, index,vidArr[vidIndex]);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index,vidArr[vidIndex]);
                }

                st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    if(vidIndex != 2)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                        UTF_VERIFY_EQUAL2_PARAM_MAC(vidArr[vidIndex], vidGet, dev, index);
                    }
                    else  /*vidIndex == 2*/
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

                        UTF_VERIFY_EQUAL2_PARAM_MAC(vidArr[vidIndex - 1], vidGet, dev, index);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
                }
            }
        }

        vidIndex = 0;

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index,vidArr[vidIndex]);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index,vidArr[vidIndex]);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    vidIndex = 0;
    index = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index,vidArr[vidIndex]);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalVidClassificationSet(dev,index,vidArr[vidIndex]);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index,vidArr[vidIndex]);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_U16                  *vidPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalVidClassificationGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    GT_U16       vidGet;


    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 4 ; index ++)
        {
            st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }

            /* check NULL pointer*/
            st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }
        }

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    index = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalVidClassificationGet(dev,index,&vidGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalMacDaClassificationEntrySet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entry;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entryGet;
    GT_U32      byteIndex;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        entry.macAddrValue.arEther[0] = 0x11;
        entry.macAddrValue.arEther[1] = 0x22;
        entry.macAddrValue.arEther[2] = 0x33;
        entry.macAddrValue.arEther[3] = 0x44;
        entry.macAddrValue.arEther[4] = 0x55;
        entry.macAddrValue.arEther[5] = 0x66;

        for(index = 0 ; index < 4 ; index ++)
        {
            for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
            {
                /* calc byte mask */
                entry.macAddrMask.arEther[byteIndex] = (GT_U8)
                    (0xff - entry.macAddrValue.arEther[byteIndex]);

                /* update byte value */
                entry.macAddrValue.arEther[byteIndex] ++;
            }

            st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }

            /* check NULL pointer*/
            st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }
        }

        /* check the save values */
        {
            entry.macAddrValue.arEther[0] = 0x11;
            entry.macAddrValue.arEther[1] = 0x22;
            entry.macAddrValue.arEther[2] = 0x33;
            entry.macAddrValue.arEther[3] = 0x44;
            entry.macAddrValue.arEther[4] = 0x55;
            entry.macAddrValue.arEther[5] = 0x66;

            for(index = 0 ; index < 4 ; index ++)
            {
                for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
                {
                    /* calc byte mask */
                    entry.macAddrMask.arEther[byteIndex] = (GT_U8)
                        (0xff - entry.macAddrValue.arEther[byteIndex]);

                    /* update byte value */
                    entry.macAddrValue.arEther[byteIndex] ++;
                }

                st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
                    /* compare values */
                    for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(entry.macAddrValue.arEther[byteIndex], entryGet.macAddrValue.arEther[byteIndex], dev, index);
                        UTF_VERIFY_EQUAL2_PARAM_MAC(entry.macAddrMask.arEther[byteIndex] , entryGet.macAddrMask.arEther[byteIndex] , dev, index);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
                }
            }
        }

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    index = 0;
    entry.macAddrValue.arEther[0] = 0x11;
    entry.macAddrValue.arEther[1] = 0x22;
    entry.macAddrValue.arEther[2] = 0x33;
    entry.macAddrValue.arEther[3] = 0x44;
    entry.macAddrValue.arEther[4] = 0x55;
    entry.macAddrValue.arEther[5] = 0x66;
    for(byteIndex = 0 ; byteIndex < 6 ; byteIndex++)
    {
        /* calc byte mask */
        entry.macAddrMask.arEther[byteIndex] = (GT_U8)
            (0xff - entry.macAddrValue.arEther[byteIndex]);

        /* update byte value */
        entry.macAddrValue.arEther[byteIndex] ++;
    }
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalMacDaClassificationEntrySet(dev,index,&entry);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalMacDaClassificationEntryGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       index;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entryGet;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 4 ; index ++)
        {
            st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }

            /* check NULL pointer*/
            st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
            }
        }

        /* check NOT valid index */
        st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    index = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalMacDaClassificationEntryGet(dev,index,&entryGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8        devNum,
    IN GT_U32       priority,
    IN GT_U32       threshold
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsSet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       priority;
    GT_U32       thresholdArr[3] = {512/*ok*/,_1K-1/*ok*/,_1K/*out of range*/};
    GT_U32       thresholdGet;
    GT_U32       thresholdIndex;
    GT_U32       thresholdSet;
    GT_U32       dpSelector[MAX_DP_CNS] = {0,};
    GT_U32       thresholdBack;
    GT_U32       maxDp;
    GT_U32       dpIter;
    GT_U32       maxPriority;

    /* there is no RXDMA/IA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
            thresholdArr[2] = _64K;
        }
        else
        {
            maxPriority = 3;
            thresholdArr[2] = _1K;
        }

        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.supportMultiDataPath)
        {
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                maxDp = PRV_CPSS_PP_MAC(dev)->multiPipe.numOfPipes;
                for(dpIter = 0; dpIter < maxDp; dpIter++)
                {
                    dpSelector[dpIter] = (1 << dpIter);
                }
            }
            else
            {
                maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
                for(dpIter = 0; dpIter < maxDp; dpIter++)
                {
                    if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.
                        info[dpIter].dataPathNumOfPorts)
                    {
                        dpSelector[dpIter] = (1 << dpIter);
                    }
                }
            }
        }
        for(priority = 0; priority < maxPriority; priority ++)
        {
            for(thresholdIndex = 0 ; thresholdIndex < 3 ; thresholdIndex ++)
            {
                st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
                    CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
                    thresholdArr[thresholdIndex]);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    if(thresholdIndex == 2)
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, priority,thresholdArr[thresholdIndex]);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, priority,thresholdArr[thresholdIndex]);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority,thresholdArr[thresholdIndex]);
                }

                st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                    CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, &thresholdGet);
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, priority , thresholdArr[thresholdIndex]);
                    if(thresholdIndex != 2)
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(thresholdArr[thresholdIndex], thresholdGet, dev, priority , thresholdArr[thresholdIndex]);
                    }
                    else /* thresholdIndex == 2 */
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(thresholdArr[thresholdIndex - 1], thresholdGet, dev, priority , thresholdArr[thresholdIndex - 1]);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority , thresholdArr[thresholdIndex]);
                }

                /* Set two different thresholds and read them then compare */
                if(dpSelector[0])
                {

                    thresholdSet = 200;

                    /* Backup value */
                    st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
                        &thresholdBack);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                    /* Set tresholds */
                    for(dpIter = 0; dpIter < MAX_DP_CNS; dpIter++)
                    {
                        if(dpSelector[dpIter] == 0)
                        {
                            break;
                        }
                        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
                            dpSelector[dpIter], priority, thresholdSet++);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                    }

                    /* Read back theresholds */
                    for(; dpIter; dpIter--)
                    {
                        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                            dpSelector[dpIter - 1], priority, &thresholdGet);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(--thresholdSet,
                            thresholdGet);
                    }

                    /* Set back value */
                    st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
                        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
                        thresholdBack);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                }
            }
        }

        /* check NOT valid dataPathBmp */
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.
                    supportMultiDataPath)
        {
            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                0xFFFFFFFE, priority, &thresholdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, 0xFFFFFFFE);
        }

        thresholdIndex = 0;

        /* check NOT valid priority */
        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++,
            thresholdArr[thresholdIndex]);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, priority,thresholdArr[thresholdIndex]);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority,thresholdArr[thresholdIndex]);
        }
        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++,
            thresholdArr[thresholdIndex]);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, priority,thresholdArr[thresholdIndex]);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority,thresholdArr[thresholdIndex]);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    priority = 0;
    thresholdIndex = 0;
    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev, priority,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, thresholdArr[thresholdIndex]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,
        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,
        thresholdArr[thresholdIndex]);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);

}

/*
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8        devNum,
    IN GT_U32       priority,
    OUT GT_U32       *thresholdPtr
);
*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      priority;
    GT_U32      thresholdGet;
    GT_U32      maxPriority;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            maxPriority = 4;
        }
        else
        {
            maxPriority = 3;
        }

        for(priority = 0; priority < maxPriority; priority ++)
        {
            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, &thresholdGet);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, priority);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority );
            }

            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
                CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, priority);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority );
            }

        }

        /* check NOT valid priority */
        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++, &thresholdGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
        }
        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority++, &thresholdGet);
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
        }

        /* check NOT valid dataPathBmp */
        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.
                    supportMultiDataPath)
        {
            st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev, 0xFFFFFFFE,
                priority, &thresholdGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, 0xFFFFFFFE);
        }
    }

    /* 3. For not active devices check that function returns non GT_OK.*/
    priority = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
            CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority,&thresholdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, priority);
    }

    /* 4.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalBurstFifoThresholdsGet(dev,
        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, &thresholdGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, priority);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64      countersArr[4]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipGlobalDropCounterGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U64      counterArr[4];

    /* there is no RXDMA/IA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortPipGlobalDropCounterGet(dev,counterArr);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev) && (! PRV_CPSS_SIP_6_CHECK_MAC(dev)))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev );
        }

        st = cpssDxChPortPipGlobalDropCounterGet(dev,NULL);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev) && (! PRV_CPSS_SIP_6_CHECK_MAC(dev)))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev );
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipGlobalDropCounterGet(dev,counterArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipGlobalDropCounterGet(dev,counterArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPortPipDropCounterGet)
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32       port;
    GT_U64       counter;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL2_PARAM_MAC(0 , counter.l[1], dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port );
            }

            st = cpssDxChPortPipDropCounterGet(dev,port,NULL);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port );
            }
        }

        /* check NOT valid port */
        st = prvUtfNextRxDmaPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available 'MAC' ports. */
        while(GT_OK == prvUtfNextRxDmaPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare iterator for go over all active devices */
    UT_PIP_RESET_DEV(&dev);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPipDropCounterGet(dev,port,&counter);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChPortPip suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortPip)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEtherTypeProtocolSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalEtherTypeProtocolGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipTrustEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipTrustEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipVidClassificationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipVidClassificationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipPrioritySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipPriorityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalProfilePrioritySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalProfilePriorityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalVidClassificationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalVidClassificationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalMacDaClassificationEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalMacDaClassificationEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalBurstFifoThresholdsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipGlobalDropCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPipDropCounterGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortPip)


