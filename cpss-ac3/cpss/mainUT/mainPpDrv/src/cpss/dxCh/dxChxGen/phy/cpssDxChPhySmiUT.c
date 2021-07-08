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
* @file cpssDxChPhySmiUT.c
*
* @brief Unit tests for cpssDxChPhySmi.
*
* @version   31
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
/* #include <cpss/common/cpssTypes.h> */
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhySmi.h>
#include <cpss/common/smi/cpssGenSmi.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define PHY_SMI_VALID_PHY_PORT_CNS  0

#define IS_PHY_SMI_IF_SUPPORTED(devNum, _portType)    \
            (PRV_CPSS_PORT_XG_E > _portType)

/* ch1,2,3,xcat ports 24..27 are XG ports, xcat2 has no XG ports
in Lion, Lion2 all ports are XG */
#define IS_XG_PORT_SUPPORT_MAC(dev,port)      \
    ((((port) >= 24) && (!PRV_CPSS_SIP_5_CHECK_MAC(dev))) ? 1 :    \
    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||         \
     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)||        \
     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)||        \
     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)||        \
     (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)||        \
     ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) && (port>=48))) ? 1 : 0)

/* check if port support autopolling of PHY */
#define IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,_port)      \
(((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) && ((_port) < 48)) ? 1 : \
  ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) && ((_port) < 48)) ? 1 : \
  ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) && (((_port) >= 256) && ((_port) < (256+48)))) ? 1 : \
  ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) && ((_port) < 48)) ? 1 : \
  (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 0 : \
  (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev) == GT_TRUE) ? 0 : \
 ((PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev, _port) < 24)? 1 : 0))


static GT_U32 prvUtfIsXgPortSupportedCheck
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_U32 macPortNum;

    if(((portNum) >= 24) && (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        return 1;
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)     ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)    ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
    {
        return 1;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {

        if(GT_OK != prvUtfPortMacNumberGet(devNum, portNum, &macPortNum))
        {
            return 0xFFFFFFFF;
        }

        if((PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E) && (portNum >= 48))
        {
            return 1;
        }
        if((PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) && ((portNum >= 48) && (macPortNum != 62)))
        {
            return 1;
        }
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortAddrSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_U8   phyAddr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyPortAddrSet) */
GT_VOID cpssDxChPhyPortAddrSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with phyAddr [12].
    Expected: GT_OK for non 10Gbps port;
              GT_NOT_SUPPORTED for 10Gbps port.
    1.1.2. Check for out of range phy address. For non
           10Gbps port call function with phyAddr [32].
    Expected: NON GT_OK.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U8   phyAddr = 0;
    GT_U8   phyAddFromHwPtr = 0;
    GT_U8   phyAddFromDbPtr = 0;
    GT_U8   phyAddFromDbRestore;

    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    PRV_CPSS_PORT_TYPE_ENT      portType;
    GT_BOOL                     isFlexLink;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices, ALL families*/
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlexLink);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortIsFlexLinkGet: %d, %d", dev, port);

            /* 1.1.1. Call function with phyAddr [12]. Expected:        */
            /* GT_OK for non 10Gbps port;                               */
            /* GT_NOT_SUPPORTED for  10Gbps port.                       */
            phyAddr = 12;

            if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
            {
                st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr,
                                            &phyAddFromDbRestore);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }

            st = cpssDxChPhyPortAddrSet(dev, port, phyAddr);

            if((PRV_CPSS_PORT_XG_E > portType) || (isFlexLink == GT_TRUE))
            {/* non 10Gbps or Flex link ports */
                if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                           "Cheetah device, non 10Gbps port: %d, %d, %d",
                                                    dev, port, phyAddr);
                }
                else
                {
                    /*CPU port 32 in Aldrin is not supported*/
                    if ((PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev)) && port ==32)
                    {
                            UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_SUPPORTED, st,
                           "Cheetah device, non 10Gbps port: %d, %d, %d",
                                                    dev, port, phyAddr);
                    }
                    else{
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                               "Cheetah device, non 10Gbps port: %d, %d, %d",
                                                        dev, port, phyAddr);
                    }
                }

                st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr,
                                            &phyAddFromDbPtr);
                if(phyAddFromHwPtr == 0xFF)
                {
                    /* HW value was not set , because no HW support ... use the DB */
                    phyAddFromHwPtr = phyAddFromDbPtr;
                }

                if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
                {

                    if (phyAddFromDbRestore == 0xFF)
                    {
                        PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(dev, port) =  phyAddFromDbRestore;
                    }
                    else
                    {
                        st = cpssDxChPhyPortAddrSet(dev, port, phyAddFromDbRestore);

                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }

                }

                UTF_VERIFY_EQUAL2_STRING_MAC(phyAddr, phyAddFromHwPtr ,
                     "get another <phyAddr> value than was set: %d, %d", dev, port);
            }
            else
            {/* 10Gbps */
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,
                        "Cheetah device, 10Gbps port: %d, %d", dev, port);
            }

            /*
               1.1.2. Check for out of range phy address.
               For non 10Gbps port call function with phyAddr [BIT_5].
               Expected: GT_OUT_OF_RANGE.
            */
            if((PRV_CPSS_PORT_XG_E > portType) || (isFlexLink == GT_TRUE))
            {
                phyAddr = BIT_5;

                st = cpssDxChPhyPortAddrSet(dev, port, phyAddr);
                if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, phyAddr);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, phyAddr);
                }
            }
        }

        phyAddr = 12;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPortAddrSet(dev, port, phyAddr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPortAddrSet(dev, port, phyAddr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    phyAddr = 12;
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator - iterate ONLY non-active devices! Because  */
    /* for non-Ch & 10G port we expect "non GT_OK", but not GT_BAD_PARAM   */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPortAddrSet(dev, port, phyAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */
    /* phyAddr ==12 */

    st = cpssDxChPhyPortAddrSet(dev, port, phyAddr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U8   *phyAddFromHwPtr,
    OUT GT_U8   *phyAddFromDbPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyPortAddrGet) */
GT_VOID cpssDxChPhyPortAddrGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with not null pointers.
    Expected: GT_OK for non 10Gbps port;
              GT_NOT_SUPPORTED for 10Gbps port.
    1.1.2. Call with wrong phyAddFromHwPtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with wrong phyAddFromDbPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U8   phyAddFromHwPtr = 0;
    GT_U8   phyAddFromDbPtr = 0;

    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    PRV_CPSS_PORT_TYPE_ENT      portType;
    GT_BOOL                     isFlexLink;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices, ALL families*/
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlexLink);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortIsFlexLinkGet: %d, %d", dev, port);

            /*
                1.1.1. Call function with not null pointers.
                Expected: GT_OK for non 10Gbps port;
                          GT_NOT_SUPPORTED for 10Gbps port.
            */

            st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr, &phyAddFromDbPtr);

            if((PRV_CPSS_PORT_XG_E > portType) || (isFlexLink == GT_TRUE))
            {/* non 10Gbps or flex ports */
                if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                          "Cheetah device, non 10Gbps port: %d, %d", dev, port);
                }
                else
                {
                     /*CPU port 32 in Aldrin is not supported*/
                    if ((PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev)) && port ==32)
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,
                          "Cheetah device, non 10Gbps port: %d, %d", dev, port);
                    }
                    else{
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                              "Cheetah device, non 10Gbps port: %d, %d", dev, port);
                    }
                }
            }
            else
            {/* 10Gbps */
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,
                          "Cheetah device, 10Gbps port: %d, %d", dev, port);
            }

            /*
                1.1.2. Call with wrong phyAddFromHwPtr [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChPhyPortAddrGet(dev, port, NULL, &phyAddFromDbPtr);

            if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                      "Cheetah device, non 10Gbps port: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                      "Cheetah device, non 10Gbps port: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with wrong phyAddFromDbPtr [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr, NULL);

            if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                      "Cheetah device, non 10Gbps port: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                      "Cheetah device, non 10Gbps port: %d, %d", dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr, &phyAddFromDbPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr, &phyAddFromDbPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator - iterate ONLY non-active devices! Because  */
    /* for non-Ch & 10G port we expect "non GT_OK", but not GT_BAD_PARAM   */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr, &phyAddFromDbPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhyPortAddrGet(dev, port, &phyAddFromHwPtr, &phyAddFromDbPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortSmiRegisterRead
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     phyReg,
    OUT GT_U16    *dataPtr
)
*/

/* UTF_TEST_CASE_MAC(cpssDxChPhyPortSmiRegisterRead) */
GT_VOID cpssDxChPhyPortSmiRegisterReadUT(GT_VOID)
{
/*
ITERATE_DEVICES_PHY_PORTS
1.1.1. Call function with non-NULL dataPtr and phyReg [12].
Expected: GT_OK for non 10Gbps port;
          GT_NOT_SUPPORTED for 10Gbps port.
1.1.2. Check for out of range SMI register. For non 10Gbps port call
       function with phyReg [32] and non-NULL dataPtr.
Expected: NON GT_OK.
1.1.3. Check for NULL pointer support. For non 10Gbps port call function
       with phyRegr [12] and dataPtr [NULL].
Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U8   phyReg;
    GT_U16  data;

    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    PRV_CPSS_PORT_TYPE_ENT          portType;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    /* this function is highly dependent on ASIC family, so lets run if for xcat only */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  | UTF_LION_E | UTF_LION2_E );
    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_ALDRIN_E, CPSS-6057);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call function with non-NULL dataPtr and phyReg [12].  */
            /* Expected: GT_OK for non 10Gbps port;     */
            /* GT_NOT_SUPPORTED for 10Gbps port and;     */
            phyReg = 12;

            st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, &data);

            if (IS_PHY_SMI_IF_SUPPORTED(dev,portType))
            {/* non 10Gbps */
                if (0 == prvUtfIsXgPortSupportedCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                 "DXCH device, non 10Gbps port: %d, %d, %d",
                                 dev, port, phyReg);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_INITIALIZED, st,
                                 "DXCH device, non 10Gbps port: %d, %d, %d",
                                 dev, port, phyReg);
                }
            }
            else
            {/* 10Gbps */
                if(GE_NOT_SUPPORTED(portTypeOptions))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,
                                             "DXCH device, 10Gbps port: %d, %d",
                                             dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_INITIALIZED, st,
                                             "DXCH device, 10Gbps port: %d, %d",
                                             dev, port);
                }
            }

            if (IS_PHY_SMI_IF_SUPPORTED(dev,portType))
            {
                /* 1.1.2. Check for out of range SMI register.                   */
                /* For non 10Gbps port call function with                        */
                /* phyReg [32] and non-NULL dataPtr. Expected: NON GT_OK.        */
                phyReg = 32;

                st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, &data);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, phyReg);

                /* 1.1.3. Check for NULL pointer support. For                    */
                /* non 10Gbps port call function with phyRegr [12] and           */
                /* dataPtr [NULL]. Expected: GT_BAD_PTR.                         */
                phyReg = 12;

                st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, NULL);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, NULL",
                                             dev, port, phyReg);
            }
        }

        phyReg = 12;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, &data);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, &data);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    phyReg = 12;
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator*/

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */
    /* phyReg == 12 */

    st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, &data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortSmiRegisterWrite
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     phyReg,
    IN  GT_U16    data
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyPortSmiRegisterWrite) */
GT_VOID cpssDxChPhyPortSmiRegisterWriteUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with phyReg [22] and data [0xA].
    Expected: GT_OK for non 10Gbps port;
              GT_NOT_SUPPORTED for 10Gbps port.
    1.1.2. Call cpssDxChPhyPortSmiRegisterRead function with non-NULL
           dataPtr and phyReg [22].
    Expected: GT_OK and dataPtr [0xA] for non 10Gbps port;
              GT_NOT_SUPPORTED for 10Gbps port.
    1.1.3. Check for out of range SMI register. For non 10Gbps port call
           function with phyAddr [32] and data [0xA].
    Expected: NON GT_OK.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U8   phyReg;
    GT_U16  data;

    GT_U16  retData;

    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    PRV_CPSS_PORT_TYPE_ENT          portType;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    GT_BOOL currPhyPollStatus = GT_FALSE;

    data = 0xA;

    /* this function is highly dependent on ASIC family, so lets run if for xcat only */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E  | UTF_LION_E | UTF_LION2_E );
    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_ALDRIN_E, CPSS-6057);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /*Caelum CPU port*/
            if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev) && port == 90)
            {
                continue;
            }
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (IS_PHY_SMI_IF_SUPPORTED(dev,portType))
            {/* non 10Gbps */
                if (0 == prvUtfIsXgPortSupportedCheck(dev,port))
                {
                    if(IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,port) == GT_TRUE)
                    {
                        st = cpssDxChPhyAutonegSmiGet(dev, port, &currPhyPollStatus);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "cpssDxChPhyAutonegSmiGet: %d, %d", dev, port);

                        /* swtich off phy polling by ASIC to prevent phy page change */
                        st = cpssDxChPhyAutonegSmiSet(dev, port, GT_FALSE);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                     "cpssDxChPhyAutonegSmiSet: %d, %d, GT_FALSE", dev, port);
                    }
                }
            }

            /* 1.1.1. Call function with data [0xA] and phyReg [22 dec].    */
            /* Expected: GT_OK for non 10Gbps port;                         */
            /* GT_NOT_SUPPORTED for 10Gbps port and;                        */
            phyReg = 22; /* page address register exists on every page in all Marvell PHY's AFAIK */

            st = cpssDxChPhyPortSmiRegisterWrite(dev, port, phyReg, data);

            if (IS_PHY_SMI_IF_SUPPORTED(dev,portType))
            {/* non 10Gbps */
                if (0 == prvUtfIsXgPortSupportedCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                                                 "Cheetah device, non 10Gbps port: %d, %d, %d, %d",
                                                 dev, port, phyReg, data);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_NOT_INITIALIZED, st,
                                                 "Cheetah device, non 10Gbps port: %d, %d, %d, %d",
                                                 dev, port, phyReg, data);
                }
            }
            else
            {/* 10Gbps */
                if(GE_NOT_SUPPORTED(portTypeOptions))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,
                                             "DXCH device, 10Gbps port: %d, %d",
                                             dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_INITIALIZED, st,
                                             "DXCH device, 10Gbps port: %d, %d",
                                             dev, port);
                }
            }

            /* 1.1.2. Call cpssDxChPhyPortSmiRegisterRead function with */
            /* non-NULL dataPtr and phyReg [12]. Expected: GT_OK and    */
            /* dataPtr [234] for non 10Gbps port;                       */
            /* GT_NOT_SUPPORTED for 10Gbps port.                        */

            st = cpssDxChPhyPortSmiRegisterRead(dev, port, phyReg, &retData);

/* Next is valid for ExMx, but is it valid for DxCh?
#ifdef ASIC_SIMULATION
simulation not support the "read" actions to return values other then 0
            retData = data;
#endif  ASIC_SIMULATION*/

#ifdef ASIC_SIMULATION
            retData = data;
#endif  /*ASIC_SIMULATION*/

            if (IS_PHY_SMI_IF_SUPPORTED(dev,portType))
            {/* non 10Gbps */
                if (0 == prvUtfIsXgPortSupportedCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                     "cpssDxChPhyPortSmiRegisterRead: non 10Gbps port: %d, %d, %d",
                                                 dev, port, phyReg);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_INITIALIZED, st,
                     "cpssDxChPhyPortSmiRegisterRead: non 10Gbps port: %d, %d, %d",
                                                 dev, port, phyReg);
                }
                if (GT_OK == st)
                {
                    /*Cetus CPU port*/
                    if (!(PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) && port == 90))
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC(data, retData,
                                     "read another data than was written: %d, %d, %d",
                                                     dev, port, phyReg);
                    }
                }
            }
            else
            {/* 10Gbps */
                if(GE_NOT_SUPPORTED(portTypeOptions))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_SUPPORTED, st,
                                             "DXCH device, 10Gbps port: %d, %d",
                                             dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_INITIALIZED, st,
                                             "DXCH device, 10Gbps port: %d, %d",
                                             dev, port);
                }
            }

            if (IS_PHY_SMI_IF_SUPPORTED(dev,portType))
            {
                /* 1.1.3. Check for out of range SMI register.               */
                /* For non 10Gbps port call function with                    */
                /* phyReg  [32] and data [234]. Expected: NON GT_OK.         */
                phyReg = 32;

                st = cpssDxChPhyPortSmiRegisterWrite(dev, port, phyReg, data);
                if (0 == prvUtfIsXgPortSupportedCheck(dev,port))
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, phyReg);

                    if(IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,port) == GT_TRUE)
                    {
                        /* restore phy polling mode for port */
                        st = cpssDxChPhyAutonegSmiSet(dev, port, currPhyPollStatus);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                     "cpssDxChPhyAutonegSmiSet: %d, %d", dev, port);
                    }
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_NOT_INITIALIZED, st, dev, port, phyReg);
                }
            }
        }
        phyReg = 22;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPortSmiRegisterWrite(dev, port, phyReg, data);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPortSmiRegisterWrite(dev, port, phyReg, data);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    phyReg = 22;
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator - iterate ONLY non-active devices! Because  */
    /* for non-Ch & 10G port we expect "non GT_OK", but not GT_BAD_PARAM   */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPortSmiRegisterWrite(dev, port, phyReg, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */
    /* phyReg == 22 */

    st = cpssDxChPhyPortSmiRegisterWrite(dev, port, phyReg, data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPort10GSmiRegisterRead
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     phyId,
    IN  GT_BOOL   useExternalPhy,
    IN  GT_U16    phyReg,
    IN  GT_U8     phyDev,
    OUT GT_U16    *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhyPort10GSmiRegisterRead)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with non-NULL dataPtr and phyId [12],
           useExternalPhy [GT_TRUE and GT_FALSE], phyReg [270], phyDev [20].
    Expected: GT_OK for 10Gbps port;
              GT_NOT_SUPPORTED for non 10Gbps port.
    1.1.2. Check for out of range ID of external 10G PHY.
           For 10Gbps port call function with phyId [32],
           useExternalPhy [GT_TRUE], phyDev [20], phyReg [2],
           and non-NULL dataPtr.
    Expected: NON GT_OK.
    1.1.3. Check for out of range PHY device id. For 10Gbps port call
           function with phyId [12], useExternalPhy [GT_TRUE],
           phyDev [32], phyReg [2], and non-NULL dataPtr.
    Expected: NON GT_OK.
    1.1.4. Check for NULL pointer support. For 10Gbps port call
           function with phyId [12], useExternalPhy [GT_TRUE],
           phyDev [20], phyReg [2], and dataPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U8   phyId;
    GT_BOOL useExternalPhy;
    GT_U16  phyReg;
    GT_U8   phyDev;
    GT_U16  data;

    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    PRV_CPSS_PORT_TYPE_ENT      portType;

    /* this function is highly dependent on ASIC family, so lets run if for CH3 only */
    /* TBD: fix useExternalPhy to false for CH3 */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E  | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_AC3X_E | UTF_ALDRIN2_E);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* 1.1.1. Call function with non-NULL dataPtr and phyId [12],   */
            /* useExternalPhy [GT_TRUE and GT_FALSE], phyDev [20],          */
            /* phyReg [270].                                                */
            /* Expected: GT_OK for 10Gbps port;                             */
            /* GT_NOT_SUPPORTED for non 10Gbps port.                        */
            phyId = 12;
            phyDev = 20;
            phyReg = 270;
            useExternalPhy = GT_TRUE;

            st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, &data);

            UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                                         "Cheetah device, 10Gbps port: %d, %d, %d, %d, %d, %d",
                                         dev, port, phyId, useExternalPhy, phyReg, phyDev);

            useExternalPhy = GT_FALSE;

            st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, &data);

            if(PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_CHEETAH3_E)
            {
                UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                                             "Cheetah device, 10Gbps port: %d, %d, %d, %d, %d, %d",
                                             dev, port, phyId, useExternalPhy, phyReg, phyDev);
            }
            else
            {
                /* ch3 and above */
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                             "Cheetah3 and above device , useExternalPhy = GT_FALSE: %d",
                                             dev);
            }

            if (PRV_CPSS_PORT_XG_E == portType)
            {
                /* 1.1.2. Check for out of range ID of external 10G PHY.  */
                /* For 10Gbps port call function with                     */
                /* phyId [32], useExternalPhy [GT_TRUE], phyDev [20],     */
                /* phyReg [2], and non-NULL dataPtr. Expected: NON GT_OK. */
                phyId = 32;
                useExternalPhy = GT_TRUE;
                phyDev = 20;
                phyReg = 2;

                st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                       phyReg, phyDev, &data);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, phyId);

                /* 1.1.3. Check for out of range PHY device id.           */
                /* For 10Gbps port call function with                     */
                /* phyId [12], useExternalPhy [GT_TRUE], phyDev [32],     */
                /* phyReg [2], and non-NULL dataPtr. Expected: NON GT_OK. */
                phyId = 12;
                useExternalPhy = GT_TRUE;
                phyDev = 32;
                phyReg = 2;

                st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                       phyReg, phyDev, &data);
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                                 "%d, %d, phyDev = %d", dev, port, phyDev);

                /* 1.1.4. Check for NULL pointer support.                   */
                /* For 10Gbps port call function with                       */
                /* phyId [12], useExternalPhy [GT_TRUE], phyDev [20],       */
                /* phyReg [2], and dataPtr [NULL]. Expected: GT_BAD_PTR.    */
                phyId = 12;
                useExternalPhy = GT_TRUE;
                phyDev = 20;
                phyReg = 2;

                st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                       phyReg, phyDev, NULL);
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "%d, %d, dataPtr = NULL", dev, port);
            }
        }

        phyId = 12;
        useExternalPhy = GT_TRUE;
        phyDev = 20;
        phyReg = 2;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, &data);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                               phyReg, phyDev, &data);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    phyId = 12;
    useExternalPhy = GT_TRUE;
    phyDev = 20;
    phyReg = 2;
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                               phyReg, phyDev, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                           phyReg, phyDev, &data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPort10GSmiRegisterWrite
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     phyId,
    IN  GT_BOOL   useExternalPhy,
    IN  GT_U16    phyReg,
    IN  GT_U8     phyDev,
    IN  GT_U16    data
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhyPort10GSmiRegisterWrite)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with dataPtr [56] and phyId [12],
           useExternalPhy [GT_TRUE and GT_FALSE], phyDev [20], phyReg [270].
    Expected: GT_OK for Cheetah devices and 10Gbps port;
              GT_NOT_SUPPORTED for Cheetah device and non 10Gbps port.
    1.1.2. Call function cpssDxChPhyPort10GSmiRegisterRead with non-NULL dataPtr
           and phyId [12], useExternalPhy [GT_TRUE and GT_FALSE],
           phyDev [20], phyReg [270].
    Expected: GT_OK  and dataPtr [56] for 10Gbps port;
              GT_NOT_SUPPORTED for non 10Gbps port.
    1.1.3. Check for out of range ID of external 10Gbps PHY. For 10Gbps
           port call function with phyId [32], useExternalPhy [GT_TRUE],
           phyDev [20], phyReg [2], and non-NULL dataPtr.
    Expected: NON GT_OK.
    1.1.4. Check for out of range PHY device id. For 10Gbps port call
           function with phyId [12], useExternalPhy [GT_TRUE], phyDev [32],
           phyReg [2], and non-NULL dataPtr.
    Expected: NON GT_OK.
*/

    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_U8                   phyId;
    GT_BOOL                 useExternalPhy;
    GT_U16                  phyReg;
    GT_U8                   phyDev;
    GT_U16                  data;
    GT_U16                  retData;
    GT_STATUS               st;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    data = 0x800a;

    /* this function is highly dependent on ASIC family, so lets run if for CH1-2 only */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_DIAMOND_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E  | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* no sence to write to not existing registers */
            if (PRV_CPSS_PORT_XG_E != portType)
                continue;

            /* 1.1.1. Call function with dataPtr [56] and phyId [12],       */
            /* useExternalPhy [GT_TRUE and GT_FALSE], phyDev [20],          */
            /* phyReg [270]. Expected:                                      */
            /* GT_OK for 10Gbps port;                                       */
            /* GT_NOT_SUPPORTED for Cheetah device and non 10Gbps port and; */
            /* GT_BAD_PARAM for 10Gbps port;                                */

            /* 1.1.2. Call function cpssDxChPhyPort10GSmiRegisterRead       */
            /* with non-NULL dataPtr and phyId [12],                        */
            /* useExternalPhy [GT_TRUE and GT_FALSE], phyDev [20],          */
            /* phyReg [270]. Expected:                                      */
            /* GT_OK  and dataPtr [56] for 10Gbps port;                     */
            /* GT_NOT_SUPPORTED for non 10Gbps port                         */
            phyId = 0;
            phyDev = 5;
            phyReg = 0x8000;

            /* it's impossible to build generic test for any type of external PHY */
            useExternalPhy = GT_TRUE;

            st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, data);

            UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                        "Cheetah device, 10Gbps port: %d, %d, %d, %d, %d, %d",
                        dev, port, phyId, useExternalPhy, phyReg, phyDev);

            /* 1.1.2. for useExternalPhy == GT_TRUE */
            st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, &retData);
/* Next is valid for ExMx, but is it valid for DxCh? */
#ifdef ASIC_SIMULATION
/* simulation not support the "read" actions to return values other then 0 */
            retData = data;
#endif  /*ASIC_SIMULATION*/

            if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                /* Reading dev/reg through XSMI will return 0xFFFFFFFF in SIP5
                 * if XSMI is not connected to any phy. Just ignore read value */
                retData = data;
            }
            UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
            "cpssDxChPhyPort10GSmiRegisterRead: Cheetah device, 10Gbps port: %d, %d, %d, %d, %d, %d",
                      dev, port, phyId, useExternalPhy, phyReg, phyDev);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL6_STRING_MAC(data, retData,
                "read another data than was written: %d, %d, %d, %d, %d, %d",
                           dev, port, phyId, useExternalPhy, phyReg, phyDev);
            }

            /* 1.1.1 for useExternalPhy == GT_FALSE */
            useExternalPhy = GT_FALSE;

            st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, data);

            if(PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_CHEETAH3_E)
            {
                UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                     "Cheetah device, 10Gbps port: %d, %d, %d, %d, %d, %d",
                         dev, port, phyId, useExternalPhy, phyReg, phyDev);
            }
            else
            {
                /* ch3 and above */
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                             "Cheetah3 and above device , useExternalPhy = GT_FALSE: %d",
                                             dev);
            }

            /* 1.1.2. for useExternalPhy == GT_FALSE */
            st = cpssDxChPhyPort10GSmiRegisterRead(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, &retData);
/* Next is valid for ExMx, but is it valid for DxCh? */
#ifdef ASIC_SIMULATION
/* simulation not support the "read" actions to return values other then 0 */
            retData = data;
#endif    /*ASIC_SIMULATION*/

            if(PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_CHEETAH3_E)
            {
                UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, st,
                "cpssDxChPhyPort10GSmiRegisterRead: Cheetah device, 10Gbps port: %d, %d, %d, %d, %d, %d",
                           dev, port, phyId, useExternalPhy, phyReg, phyDev);
                if (GT_OK == st)
                {
                    UTF_VERIFY_EQUAL6_STRING_MAC(data, retData,
                    "read another data than was written: %d, %d, %d, %d, %d, %d",
                            dev, port, phyId, useExternalPhy, phyReg, phyDev);
                }
            }
            else
            {
                /* ch3 and above */
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                             "Cheetah3 and above device , useExternalPhy = GT_FALSE: %d",
                                             dev);
            }

            /* 1.1.2. Check for out of range ID of external 10G PHY.  */
            /* For 10Gbps port call function with                     */
            /* phyId [32], useExternalPhy [GT_TRUE], phyDev [20],     */
            /* phyReg [2], and data [56]. Expected: NON GT_OK.        */
            phyId = 32;
            useExternalPhy = GT_TRUE;
            phyDev = 5;
            phyReg = 0x8000;

            st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, data);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, phyId);

            /* 1.1.3. Check for out of range PHY device id.           */
            /* For Cheetah device and 10Gbps port call function with  */
            /* phyId [12], useExternalPhy [GT_TRUE], phyDev [32],     */
            /* phyReg [2], and data [56]. Expected: NON GT_OK.        */
            phyId = 0;
            useExternalPhy = GT_FALSE;
            phyDev = 32;
            phyReg = 0x8000;

            st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                                   phyReg, phyDev, data);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                                             "%d, %d, phyDev = %d", dev, port, phyDev);
        }

        phyId = 0;
        useExternalPhy = GT_FALSE;
        phyDev = 5;
        phyReg = 0x8000;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                                    phyReg, phyDev, data);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                               phyReg, phyDev, data);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    phyId = 0;
    useExternalPhy = GT_FALSE;
    phyDev = 5;
    phyReg = 0x8000;
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator - iterate ONLY non-active devices! Because  */
    /* for non-Ch & 10G port we expect "non GT_OK", but not GT_BAD_PARAM   */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                               phyReg, phyDev, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhyPort10GSmiRegisterWrite(dev, port, phyId, useExternalPhy,
                                           phyReg, phyDev, data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyAutonegSmiGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enabledPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyAutonegSmiGet) */
GT_VOID cpssDxChPhyAutonegSmiGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Check for NULL pointer support.  Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     enable;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call function with non-NULL enablePtr. */
            /* Expected: GT_OK                               */
            st = cpssDxChPhyAutonegSmiGet(dev, port, &enable);

            if (IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,port) && !GE_NOT_SUPPORTED(portTypeOptions))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "non 10Gbps port: %d, %d, %d", dev, port);
            }
            else
            {
                 UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                    "10Gbps port: %d, %d", dev, port);
            }

            /* 1.1.2. Check for NULL pointer support.                     */
            /* call function with enablePtr [NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChPhyAutonegSmiGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyAutonegSmiGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyAutonegSmiGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyAutonegSmiGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhyAutonegSmiGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyAutonegSmiSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL enabled
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyAutonegSmiSet) */
GT_VOID cpssDxChPhyAutonegSmiSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK for Cheetah devices.
    1.1.2. Call function cpssDxChPhyAutonegSmiGet with non-NULL enablePtr.
    Expected: GT_OK and enablePtr the same as was set.
*/
    GT_STATUS st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     enable;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;
    GT_BOOL     retEnable;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call function with enable [GT_TRUE; GT_FALSE]. */
            /* Expected: GT_OK for Cheetah devices;                  */
            /* GT_BAD_PARAM for non-Cheetah device.                  */

            /* 1.1.2. Call function cpssDxChPhyAutonegSmiGet            */
            /* with non-NULL enablePtr. Expected: GT_OK and enablePtr   */
            /* the same as was set for Cheetah devices;                 */
            /* GT_BAD_PARAM for non-Cheetah device.                     */

            enable = GT_TRUE;

            st = cpssDxChPhyAutonegSmiSet(dev, port, enable);

            if (IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,port) && !GE_NOT_SUPPORTED(portTypeOptions))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "device: %d, %d, %d", dev, port, enable);

                /* 1.1.2. for enable == GT_TRUE */
                st = cpssDxChPhyAutonegSmiGet(dev, port, &retEnable);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPhyAutonegSmiGet: Cheetah device: %d, %d",
                                             dev, port);
                if (GT_OK == st)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                         "get another <enable> value than was set: %d, %d",
                                                 dev, port);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                        "10Gbps port: %d, %d", dev, port);
            }

            /*1.1.1. with enable == GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChPhyAutonegSmiSet(dev, port, enable);

            if (IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,port) && !GE_NOT_SUPPORTED(portTypeOptions))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "Cheetah device: %d, %d, %d", dev, port, enable);

                /* 1.1.2. for enable == GT_FALSE */
                st = cpssDxChPhyAutonegSmiGet(dev, port, &retEnable);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChPhyAutonegSmiGet: Cheetah device: %d, %d",
                                             dev, port);
                if (GT_OK == st)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                        "get another <enable> value than was set: %d, %d",
                                                 dev, port);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                        "10Gbps port: %d, %d", dev, port);
            }
        }

        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyAutonegSmiSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyAutonegSmiSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyAutonegSmiSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhyAutonegSmiSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhySmiAutoMediaSelectSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_U32   autoMediaSelect
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhySmiAutoMediaSelectSet) */
GT_VOID cpssDxChPhySmiAutoMediaSelectSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with autoMediaSelect [0x21].
    Expected: GT_OK.
    1.1.2. Call function cpssDxChPhySmiAutoMediaSelectGet
           with non-NULL autoMediaSelectPtr.
    Expected: GT_OK and autoMediaSelectPtr [0x21].
    1.1.3. Check for out of range autoMediaSelect. Call
           function with autoMediaSelect [0x41].
    Expected: NON GT_OK.
*/
    GT_STATUS st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32      autoMediaSelect;
    GT_U32      retAutoMediaSelect;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call function with autoMediaSelect [0x1]. */
            /* Expected: GT_OK */
            autoMediaSelect = 0x1;

            st = cpssDxChPhySmiAutoMediaSelectSet(dev, port, autoMediaSelect);

            if (IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,port) && !GE_NOT_SUPPORTED(portTypeOptions))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "Cheetah device: %d, %d, %d",
                                             dev, port, autoMediaSelect);

                /* 1.1.2. Call function cpssDxChPhySmiAutoMediaSelectGet    */
                /* with non-NULL autoMediaSelectPtr. Expected:              */
                /* GT_OK and autoMediaSelectPtr [0x21] */
                st = cpssDxChPhySmiAutoMediaSelectGet(dev, port, &retAutoMediaSelect);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChPhySmiAutoMediaSelectGet: Cheetah device: %d, %d", dev, port);
                if (GT_OK == st)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(autoMediaSelect, retAutoMediaSelect,
                    "get another <autoMediaSelect> value than was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                        "10Gbps port: %d, %d", dev, port);
            }

            /* 1.1.3. Check for out of range autoMediaSelect.    */
            /* Call function with autoMediaSelect [0x41].        */
            /* Expected: NON GT_OK.                              */
            autoMediaSelect = 0x41;

            st = cpssDxChPhySmiAutoMediaSelectSet(dev, port, autoMediaSelect);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, autoMediaSelect);
        }

        autoMediaSelect = 0x21;

        /* 1.2. For all active devices go over all non available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhySmiAutoMediaSelectSet(dev, port, autoMediaSelect);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhySmiAutoMediaSelectSet(dev, port, autoMediaSelect);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    autoMediaSelect = 0x21;
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhySmiAutoMediaSelectSet(dev, port, autoMediaSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhySmiAutoMediaSelectSet(dev, port, autoMediaSelect);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhySmiAutoMediaSelectGet
(

    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_U32   *autoMediaSelectPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhySmiAutoMediaSelectGet) */
GT_VOID cpssDxChPhySmiAutoMediaSelectGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with non-NULL autoMediaSelectPtr.
    Expected: GT_OK.
    1.1.2. Check for NULL pointer support. Call function
           with autoMediaSelectPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32      autoMediaSelect;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get port type */
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call function with non-NULL autoMediaSelectPtr.   */
            /* Expected: GT_OK */
            st = cpssDxChPhySmiAutoMediaSelectGet(dev, port, &autoMediaSelect);

            if (IS_AUTO_POLLING_PORT_SUPPORT_MAC(dev,port) && !GE_NOT_SUPPORTED(portTypeOptions))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "Cheetah device: %d, %d",
                                             dev, port);

                /* 1.1.2. Check for NULL pointer support.                    */
                /* Call function with autoMediaSelectPtr [NULL].             */
                /* Expected: GT_BAD_PTR.                                     */
                st = cpssDxChPhySmiAutoMediaSelectGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                             "%d, %d, NULL", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                        "10Gbps port: %d, %d", dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhySmiAutoMediaSelectGet(dev, port, &autoMediaSelect);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhySmiAutoMediaSelectGet(dev, port, &autoMediaSelect);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhySmiAutoMediaSelectGet(dev, port, &autoMediaSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhySmiAutoMediaSelectGet(dev, port, &autoMediaSelect);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortSmiInit
(
    IN GT_U8    devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhyPortSmiInit)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call function with non-NULL autoMediaSelectPtr.
    Expected: GT_OK.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call function with non-NULL autoMediaSelectPtr.   */
        /* Expected: GT_OK */
        st = cpssDxChPhyPortSmiInit(dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPortSmiInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhyPortSmiInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortSmiInterfaceSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  smiInterface
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyPortSmiInterfaceSet) */
GT_VOID cpssDxChPhyPortSmiInterfaceSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat and above)
    1.1.1. Call function with smiInterface [CPSS_PHY_SMI_INTERFACE_0_E /
                                            CPSS_PHY_SMI_INTERFACE_1_E].
    Expected: GT_OK and GT_NOT_SUPPORTED - for non flex ports
    1.1.2. Call get function with the same params.
    Expected: GT_OK and the same smiInterface.
    1.1.3. Call with wrong enum values smiInterface.
    Expected: GT_BAD_PARAM and GT_NOT_SUPPORTED - for non flex ports.
*/
    GT_STATUS st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    CPSS_PHY_SMI_INTERFACE_ENT  smiInterface = 0;
    CPSS_PHY_SMI_INTERFACE_ENT  smiInterfaceGet = 2;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with smiInterface [CPSS_PHY_SMI_INTERFACE_0_E /
                                                        CPSS_PHY_SMI_INTERFACE_1_E].
                Expected: GT_OK and GT_NOT_SUPPORTED - for non flex ports.
            */

            /*call with  smiInterface = CPSS_PHY_SMI_INTERFACE_0_E; */
            smiInterface = CPSS_PHY_SMI_INTERFACE_0_E;

            st = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiInterface);
            if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call get function with the same params.
                Expected: GT_OK and the same smiInterface.
            */

            st = cpssDxChPhyPortSmiInterfaceGet(dev, port, &smiInterfaceGet);
            if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(smiInterface, smiInterfaceGet,
                     "get another <smiInterface> value than was set: %d, %d", dev, port);


                /*call with  smiInterface = CPSS_PHY_SMI_INTERFACE_1_E; */

                smiInterface = CPSS_PHY_SMI_INTERFACE_1_E;

                st = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiInterface);

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call get function with the same params.
                    Expected: GT_OK and the same smiInterface.
                */

                st = cpssDxChPhyPortSmiInterfaceGet(dev, port, &smiInterfaceGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(smiInterface, smiInterfaceGet,
                     "get another <smiInterface> value than was set: %d, %d", dev, port);


                /*
                    1.1.3. Call with wrong enum values smiInterface.
                    Expected: GT_BAD_PARAM
                */
            }
            UTF_ENUMS_CHECK_MAC(cpssDxChPhyPortSmiInterfaceSet
                                (dev, port, smiInterface),
                                smiInterface);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiInterface);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiInterface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiInterface);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyPortSmiInterfaceGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT CPSS_PHY_SMI_INTERFACE_ENT  *smiInterfacePtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChPhyPortSmiInterfaceGet) */
GT_VOID cpssDxChPhyPortSmiInterfaceGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS(xCat and above)
    1.1.1. Call function with  not null smiInterface.
    Expected: GT_OK and GT_NOT_SUPPORTED - for non flex ports
    1.1.2. Call with wrong smiInterface [NULL].
    Expected: GT_BAD_PTR and GT_NOT_SUPPORTED - for non flex ports.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_PHY_SMI_INTERFACE_ENT  smiInterface = 0;
    CPSS_PHY_SMI_INTERFACE_ENT  smiInterfaceGet = 2;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with  not null smiInterface.
                Expected: GT_OK and GT_NOT_SUPPORTED - for non flex ports.
            */

            st = cpssDxChPhyPortSmiInterfaceSet(dev, port, smiInterface);

            st = cpssDxChPhyPortSmiInterfaceGet(dev, port, &smiInterfaceGet);

            if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with wrong smiInterface [NULL].
                Expected: GT_BAD_PTR and GT_NOT_SUPPORTED - for non flex ports.
            */

            st = cpssDxChPhyPortSmiInterfaceGet(dev, port, NULL);
            if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPhyPortSmiInterfaceGet(dev, port, &smiInterfaceGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPhyPortSmiInterfaceGet(dev, port, &smiInterfaceGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PHY_SMI_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyPortSmiInterfaceGet(dev, port, &smiInterfaceGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0    */

    st = cpssDxChPhyPortSmiInterfaceGet(dev, port, &smiInterfaceGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhySmiMdcDivisionFactorGet
(
    IN  GT_U8                                 dev,
    IN  GT_U8                                 portNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  *divisionFactorPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhySmiMdcDivisionFactorGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(DxChXcat and above)
    1.1. Call with not null divisionFactor pointer.
    Expected: GT_OK.
    1.2. Call api with wrong divisionFactorPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                 dev;
    GT_PHYSICAL_PORT_NUM                  portNum = 0;
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  divisionFactor;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1. Call with not null divisionFactor pointer.
                Expected: GT_OK.
            */

            st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactor);
            if (GT_TRUE == prvCpssDxChPortRemotePortCheck(dev,portNum))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "dev = %d port = %d",dev,portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "dev = %d port = %d",dev,portNum);
            }

            /*
                1.2. Call api with wrong divisionFactorPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                           "%d, divisionFactorPtr = NULL", dev);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)&&
            (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E))
        {/* in Lion it's global parameter - no check for portNum */
            /* 1.2. For all active devices go over all non available physical ports. */
            while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active portNum */
                st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactor);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for portNum number.                         */
            portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

            st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactor);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactor);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactor);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhySmiMdcDivisionFactorSet
(
    IN  GT_U8                                dev,
    IN  GT_U8                                portNum,
    IN  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhySmiMdcDivisionFactorSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(DxChXcat and above)
    1.1. Call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E],
    Expected: GT_OK.
    1.2. Call cpssDxChPhySmiMdcDivisionFactorGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong divisionFactor [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                dev;
    GT_PHYSICAL_PORT_NUM                 portNum = 0;
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactorGet = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;
    GT_BOOL                              isRemotePort;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,portNum);
            /*
                1.1. Call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E /
                                              CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E /
                                              CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E /
                                              CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E /
                                              CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E /
                                              CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E],
                Expected: GT_OK.
            */

            /* call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E] */
            divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;

            st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
            if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {
                if (isRemotePort)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
                }
                else
                {
                /* BC2_B0 and above */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                }
            }
            else
            {
                /* BC2_A0 */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }

            /* call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E] */
            divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E;

            st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
            if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)
                && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E))
            {
                if (isRemotePort)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                }

                /*
                    1.2. Call cpssDxChPhySmiMdcDivisionFactorGet
                           with the same parameters.
                    Expected: GT_OK and the same values than was set.
                */
                st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactorGet);
                if (isRemotePort)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                              "cpssDxChPhySmiMdcDivisionFactorGet: %d ", dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                              "cpssDxChPhySmiMdcDivisionFactorGet: %d ", dev);
                    /* Verifying values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(divisionFactor, divisionFactorGet,
                                   "got another divisionFactor then was set: %d", dev);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
            }

            /* call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E] */
            divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E;

            st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
            if (isRemotePort)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

                /*
                    1.2. Call cpssDxChPhySmiMdcDivisionFactorGet
                           with the same parameters.
                    Expected: GT_OK and the same values than was set.
                */
                st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactorGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChPhySmiMdcDivisionFactorGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(divisionFactor, divisionFactorGet,
                               "got another divisionFactor then was set: %d", dev);

                /* call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E] */
                divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E;
                st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

                /*
                    1.2. Call cpssDxChPhySmiMdcDivisionFactorGet
                           with the same parameters.
                    Expected: GT_OK and the same values than was set.
                */
                st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactorGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChPhySmiMdcDivisionFactorGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(divisionFactor, divisionFactorGet,
                               "got another divisionFactor then was set: %d", dev);

                /* call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E] */
                divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E;
                st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);

                /* BC2_B0 and above */
                if ((PRV_CPSS_SIP_5_10_CHECK_MAC(dev)) || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

                    st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactorGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                              "cpssDxChPhySmiMdcDivisionFactorGet: %d ", dev);

                    /* CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E relevant only for BC2_B0 and above */
                        /* Verifying values */
                        UTF_VERIFY_EQUAL1_STRING_MAC(divisionFactor, divisionFactorGet,
                                   "got another divisionFactor then was set: %d", dev);
                }
                else
                {
                    /* BC2_A0 */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
                }

                /* call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E] */
                divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E;
                st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
                if ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)||
                    (PRV_CPSS_SIP_5_25_CHECK_MAC(dev)))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

                    /*
                        1.2. Call cpssDxChPhySmiMdcDivisionFactorGet
                               with the same parameters.
                        Expected: GT_OK and the same values than was set.
                    */
                    st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactorGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                              "cpssDxChPhySmiMdcDivisionFactorGet: %d ", dev);

                    /* Verifying values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(divisionFactor, divisionFactorGet,
                                   "got another divisionFactor then was set: %d", dev);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                }

                /* call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E] */
                divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E;
                st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
                if (PRV_CPSS_SIP_5_25_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

                    /*
                        1.2. Call cpssDxChPhySmiMdcDivisionFactorGet
                               with the same parameters.
                        Expected: GT_OK and the same values than was set.
                    */
                    st = cpssDxChPhySmiMdcDivisionFactorGet(dev, portNum, &divisionFactorGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                              "cpssDxChPhySmiMdcDivisionFactorGet: %d ", dev);

                    /* Verifying values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(divisionFactor, divisionFactorGet,
                                   "got another divisionFactor then was set: %d", dev);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                }
            }

            /*
                1.3. Call api with wrong divisionFactor [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPhySmiMdcDivisionFactorSet
                                (dev, portNum, divisionFactor),
                                divisionFactor);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)&&
           (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E))
        {
            /* 1.2. For all active devices go over all non available physical ports. */
            while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active portNum */
                st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for portNum number.                         */
            portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

            st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
    }

    /* restore correct values */
    divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhySmiMdcDivisionFactorSet(dev, portNum, divisionFactor);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyXsmiMdcDivisionFactorGet
(
    IN  GT_U8                                 dev,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  *divisionFactorPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhyXsmiMdcDivisionFactorGet)
{
/*
    ITERATE_DEVICES(DxChXcat and above)
    1.1. Call with not null divisionFactorPtr.
    Expected: GT_OK.
    1.2. Call api with wrong divisionFactorPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                 dev;
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  divisionFactor;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null divisionFactorPtr.
            Expected: GT_OK.
        */
        st = cpssDxChPhyXsmiMdcDivisionFactorGet(dev,
                                                 CPSS_PHY_XSMI_INTERFACE_0_E,
                                                 &divisionFactor);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong divisionFactorPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPhyXsmiMdcDivisionFactorGet(dev,
                                                 CPSS_PHY_XSMI_INTERFACE_0_E,
                                                 NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, divisionFactorPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyXsmiMdcDivisionFactorGet(dev,
                                                 CPSS_PHY_XSMI_INTERFACE_0_E,
                                                 &divisionFactor);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhyXsmiMdcDivisionFactorGet(dev,
                                             CPSS_PHY_XSMI_INTERFACE_0_E,
                                             &divisionFactor);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPhyXsmiMdcDivisionFactorSet
(
    IN  GT_U8                                dev,
    IN  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhyXsmiMdcDivisionFactorSet)
{
/*
    ITERATE_DEVICES(DxChXcat and above)
    1.1 call cpssDxChPhyXsmiMdcDivisionFactorSet for every
        xsmiInf in
            CPSS_PHY_XSMI_INTERFACE_0_E,
            CPSS_PHY_XSMI_INTERFACE_1_E,
            CPSS_PHY_XSMI_INTERFACE_2_E,
            CPSS_PHY_XSMI_INTERFACE_3_E]
        and for every divisionFactor in
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E],
    Expected: GT_OK or GT_BAD_PARAM depending on the device

    1.2. if GT_OK, call cpssDxChPhyXsmiMdcDivisionFactorGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.

    1.3. Call api with wrong divisionFactor [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                dev;
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor;
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactorGet;
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInf; /* XSMI interface */
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiArr[] = { CPSS_PHY_XSMI_INTERFACE_0_E,
                                              CPSS_PHY_XSMI_INTERFACE_1_E,
                                              CPSS_PHY_XSMI_INTERFACE_2_E,
                                              CPSS_PHY_XSMI_INTERFACE_3_E };
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  isFactorGood;  /* 0 - if division factor is applicable. !0 - otherwise */
    GT_U32                  isXsmiInfGood; /* 0 - if XSMI interface is applicable. !0 - otherwise */
    GT_U32                  xsmiIx;        /* XSMI interfaces loop iterator */


    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_PP_MAC(dev)->devFamily;
        /*
            1.1. call cpssDxChPhyXsmiMdcDivisionFactorSet with all pair
            xsmiInf x xsmiInterface
        */
        for (xsmiIx = 0; xsmiIx < sizeof(xsmiArr)/sizeof(xsmiArr[0]); xsmiIx++)
        {
            xsmiInf = xsmiArr[xsmiIx];

            for (divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;
                 divisionFactor <= CPSS_PHY_SMI_MDC_DIVISION_FACTOR_MAX_E;
                 divisionFactor++)
            {

                /* check if this divisionFactor is applicable for the device */
                isFactorGood = 1;
                if((devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                   (devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
                {
                    isFactorGood =
                        (divisionFactor == CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E ||
                         divisionFactor == CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E);
                }
                else if ((devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
                         (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)||
                         (PRV_CPSS_SIP_5_CHECK_MAC(dev))||
                         (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
                {
                    isFactorGood =
                        (divisionFactor == CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E ||
                          divisionFactor == CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E ||
                          divisionFactor == CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E ||
                          divisionFactor == CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E);
                }

                /* check if this XSMI interface is applicable for the device
                   Devices till sip5 ignore XSMI interface.
                   sip5 uses XSMI interfaces 0,1
                   sip5.20 uses XSMI interfaces 0..3
                 */


                isXsmiInfGood = 1;
                if(xsmiInf == CPSS_PHY_XSMI_INTERFACE_2_E ||
                   xsmiInf == CPSS_PHY_XSMI_INTERFACE_3_E)
                {
                    isXsmiInfGood =
                        (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ||
                         !PRV_CPSS_SIP_5_CHECK_MAC(dev));
                }

                st = cpssDxChPhyXsmiMdcDivisionFactorSet(dev, xsmiInf, divisionFactor);
                if (isFactorGood && isXsmiInfGood)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, xsmiInf, divisionFactor);

                    /*
                      1.2. Call cpssDxChPhyXsmiMdcDivisionFactorGet
                      with the same parameters.
                      Expected: GT_OK and the same values than was set.
                    */
                    st = cpssDxChPhyXsmiMdcDivisionFactorGet(dev, xsmiInf,
                                                             &divisionFactorGet);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                                 "cpssDxChPhyXsmiMdcDivisionFactorGet: %d, %d, %d",
                                                 dev, xsmiInf, divisionFactor);


                    /* Verifying values */
#ifdef ASIC_SIMULATION
                /* WM simulation doesn't support writing in MG units 1..3.
                   So in sip5.20 only CPSS_PHY_XSMI_INTERFACE_0_E is supported */
                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) && xsmiInf != CPSS_PHY_XSMI_INTERFACE_0_E)
                {
                    divisionFactorGet = divisionFactor;
                }
#endif  /*  ASIC_SIMULATION */

                    UTF_VERIFY_EQUAL3_STRING_MAC(divisionFactor, divisionFactorGet,
                                                 "got another divisionFactor then was set: %d, %d, %d",
                                                 dev, xsmiInf, divisionFactor);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, xsmiInf, divisionFactor);
                }
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable devFamily */
    /* check that function returns GT_BAD_PARAM.                        */

    divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhyXsmiMdcDivisionFactorSet(dev,
                                                 CPSS_PHY_XSMI_INTERFACE_0_E,
                                                 divisionFactor);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhyXsmiMdcDivisionFactorSet(dev,
                                             CPSS_PHY_XSMI_INTERFACE_0_E,
                                             divisionFactor);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

GT_VOID cpssDxChPhyXSmiMDCInvertSetUT(GT_VOID)
{
    GT_U8                                         devNum = 0;
    GT_STATUS                                     rc;
    GT_BOOL                                       InitialState;
    GT_BOOL                                       InvertMDC;

   /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                          UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Get XSMI MDC INVERT initial state */
        rc = cpssDxChPhyXSmiMDCInvertGet( devNum,/*OUT*/ &InitialState);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"Can't get initial Register state - %s : dev %d", "cpssDxChPhyxsmiMDCInvertGet", devNum );

        /* Set XSMI MDC INVERT */
        rc = cpssDxChPhyXSmiMDCInvertSet( devNum, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"Can't set Invert MDC - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertSet()", devNum );
        if (GT_OK != rc)
        {
            goto restore_initial_register_state;
        }

        /* Get XSMI MDC INVERT setting */
        rc = cpssDxChPhyXSmiMDCInvertGet( devNum,/*OUT*/ &InvertMDC);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"Can't get the register bit after setting Invert MDC - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertGet()", devNum );
        if (GT_OK != rc)
        {
            goto restore_initial_register_state;
        }

        /*if Get XMI MDC DO NOT INVERT - this is error */
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_TRUE, InvertMDC,"Can't set or get Invert MDC - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertGet()", devNum );
        if(GT_TRUE != InvertMDC)
        {
            goto restore_initial_register_state;
        }

        /* Set XSMI MDC DO NOT INVERT*/
        rc = cpssDxChPhyXSmiMDCInvertSet( devNum, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"Can't set NOT Invert MDC - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertSet()", devNum );
        if (GT_OK != rc)
        {
            goto restore_initial_register_state;
        }

        /* Get XSMI MDC INVERT setting */
        rc = cpssDxChPhyXSmiMDCInvertGet( devNum,/*OUT*/ &InvertMDC);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"Can't get the register bit after setting NOT Invert MDC - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertGet()", devNum );
        if (GT_OK != rc)
        {
            goto restore_initial_register_state;
        }

        /*if Get XSMI MDC INVERT - this is error */
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_FALSE, InvertMDC,"Can't set or get NOT Invert MDC - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertGet()", devNum );
        if(GT_FALSE != InvertMDC)
        {
            goto restore_initial_register_state;
        }

        /* testing for bad pointer for result - should return error */
        rc = cpssDxChPhyXSmiMDCInvertGet( devNum,/*OUT*/ (GT_BOOL*)NULL);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_BAD_PTR, rc,"Bad Pointer Error - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertGet()", devNum );

        /*Restore XSMI MDC INVERT initial state*/
restore_initial_register_state:
        rc = cpssDxChPhyXSmiMDCInvertSet( devNum, InitialState);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"Can't restore initial Register state - %s :  dev %d", "cpssDxChPhyxsmiMDCInvertSet()", devNum );


    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cppDxChPhySmi suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPhySmi)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortAddrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortAddrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortSmiRegisterRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortSmiRegisterWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPort10GSmiRegisterRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPort10GSmiRegisterWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyAutonegSmiGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyAutonegSmiSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhySmiAutoMediaSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhySmiAutoMediaSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortSmiInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortSmiInterfaceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyPortSmiInterfaceGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhySmiMdcDivisionFactorGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhySmiMdcDivisionFactorSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyXsmiMdcDivisionFactorGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyXsmiMdcDivisionFactorSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhyXSmiMDCInvertSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPhySmi)

