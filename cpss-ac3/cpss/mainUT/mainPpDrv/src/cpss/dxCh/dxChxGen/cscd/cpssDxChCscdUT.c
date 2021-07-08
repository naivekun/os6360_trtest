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
* @file cpssDxChCscdUT.c
*
* @brief Unit tests for cpssDxChCscd, that provides
* Cascading definitions -- Cheetah.
*
* @version   63
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChGEMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChXLGMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Defines */

/* Default valid value for port id */
#define CSCD_VALID_PHY_PORT_CNS  0

/* Invalid  tc  - Traffic Class of the packet on the source port (0..7)*/
#define CSCD_INVALID_TC_CNS      8

/* force trunkId to be in valid range for cascade trunks */
#define FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(_trunkId) ((_trunkId) & 0xFF)

/* string for GM fails */
#define GM_CSCD_GM_FAIL_STR     "CQ 153217: GM BC2 B0 mainUT crashes \n"

/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortTypeSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortTypeSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with portType [CPSS_CSCD_PORT_DSA_MODE_REGULAR_E /
                               CPSS_CSCD_PORT_DSA_MODE_EXTEND_E /
                               CPSS_CSCD_PORT_NETWORK_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdPortTypeGet.
    Expected: GT_OK and the same value.
    1.1.3. Call with wrong enum values portType .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st          = GT_OK;

    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_CSCD_PORT_TYPE_ENT portType    = CPSS_CSCD_PORT_DSA_MODE_REGULAR_E;

    CPSS_CSCD_PORT_TYPE_ENT portTypeGet = CPSS_CSCD_PORT_DSA_MODE_REGULAR_E;
    CPSS_CSCD_PORT_TYPE_ENT portTypeExpected = CPSS_CSCD_PORT_NETWORK_E;

    GT_PHYSICAL_PORT_NUM   utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/* port for 'UT iterations' */
    GT_BOOL     isRemotePort;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;
            isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);

            /*
                1.1.1. Call with portType [CPSS_CSCD_PORT_DSA_MODE_REGULAR_E /
                                           CPSS_CSCD_PORT_DSA_MODE_EXTEND_E /
                                           CPSS_CSCD_PORT_NETWORK_E].
                Expected: GT_OK.
            */

            /* Call function with portType [CPSS_CSCD_PORT_DSA_MODE_EXTEND_E] */
            portType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;

            st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);
                portTypeExpected = portType;
            }

            /* Call function with portType [CPSS_CSCD_PORT_DSA_MODE_REGULAR_E] */
            portType = CPSS_CSCD_PORT_DSA_MODE_REGULAR_E;

            st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
            if ((isRemotePort == GT_FALSE) ||
                (st != GT_NOT_SUPPORTED))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);
                portTypeExpected = portType;
            }

            /* Call function with portType [CPSS_CSCD_PORT_NETWORK_E] */
            portType = CPSS_CSCD_PORT_NETWORK_E;

            st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);
            portTypeExpected = portType;

            /*
                1.1.2. Call cpssDxChCscdPortTypeGet.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, &portTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(portTypeExpected, portTypeGet,
                       "get another portType than was set: %d/%d", dev, port);

            /*
                1.1.3. Call with wrong enum values portType .
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdPortTypeSet
                                (dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType),
                                portType);
        }

        portType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;

        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* 1.5. For eArch devices check EGRESS direction for MAC numbers */
            st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.5.1 For all active devices go over all available physical ports. */
            while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
            {
                port = utPhysicalPort;
                isRemotePort = prvCpssDxChPortRemotePortCheck(dev,port);

                /* Call function with portType [CPSS_CSCD_PORT_DSA_MODE_EXTEND_E] */
                portType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;

                st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_TX_E, portType);
                if ((isRemotePort == GT_FALSE) ||
                    (st != GT_NOT_SUPPORTED))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);
                    portTypeExpected = portType;
                }

                /*
                    1.5.2. Call cpssDxChCscdPortTypeGet.
                    Expected: GT_OK and the same value.
                */
                st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_TX_E, &portTypeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(portTypeExpected, portTypeGet,
                           "get another portType than was set: %d/%d", dev, port);

                /*
                    1.5.3. Call cpssDxChCscdPortTypeSet CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E.
                    Expected: GT_OK.
                */
                portType = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;
                st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_TX_E, portType);
                if ((isRemotePort == GT_FALSE) ||
                    (st != GT_NOT_SUPPORTED))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);
                    portTypeExpected = portType;
                }

                /*
                    1.5.4. Call cpssDxChCscdPortTypeGet.
                    Expected: GT_OK and the same value.
                */
                st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_TX_E, &portTypeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(portTypeExpected, portTypeGet,
                           "get another portType than was set: %d/%d", dev, port);
                /*
                    1.5.5. Call cpssDxChCscdPortTypeSet CPSS_CSCD_PORT_NETWORK_E.
                    Expected: GT_OK.
                */
                portType = CPSS_CSCD_PORT_NETWORK_E;
                st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_TX_E, portType);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);
                portTypeExpected = portType;

                /*
                    1.5.6. Call cpssDxChCscdPortTypeGet.
                    Expected: GT_OK and the same value.
                */
                st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_TX_E, &portTypeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(portTypeExpected, portTypeGet,
                           "get another portType than was set: %d/%d", dev, port);

            }

            portType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;

            st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.6. For all active devices go over all non available physical ports. */
            while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_FALSE))
            {
                port = utPhysicalPort;
                /* 1.6.1. Call function for each non-active port */
                st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_TX_E, portType);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
        }
    }

    portType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortTypeSet(dev, port, CPSS_PORT_DIRECTION_BOTH_E, portType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortTypeGet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT CPSS_CSCD_PORT_TYPE_ENT     *portTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortTypeGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with not null portType.
    Expected: GT_OK.
    1.1.2. Call with wrong portType [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    CPSS_CSCD_PORT_TYPE_ENT portType;

    GT_PHYSICAL_PORT_NUM    utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/*  port for 'UT iterations' */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;
            /*
                1.1.1. Call with not null portType.
                Expected: GT_OK.
            */
            st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, &portType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, portType);

            /*
                1.1.2. Call with wrong portType [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, portType);
        }

        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, &portType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, &portType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    portType = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, &portType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortTypeGet(dev, port,CPSS_PORT_DIRECTION_BOTH_E, &portType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDevMapTableSet
(
    IN GT_U8                                devNum,
    IN GT_U8                                targetHwDevNum,
    IN GT_U8                                sourceHwDevNum,
    IN GT_U8                                portNum,
    IN CPSS_CSCD_LINK_TYPE_STC              *cascadeLinkPtr,
    IN CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT   srcPortTrunkHashEn
    IN GT_BOOL                              egressAttributesLocallyEn
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDevMapTableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with targetHwDevNum [0 / 1 / 31],
                   sourceHwDevNum [31 / 1 / 0],
                   portNum[0 / 8 / 23],
                   valid cascadeLinkPtr [{2, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                         {63, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                         {127, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                         {0, CPSS_CSCD_LINK_TYPE_PORT_E} /
                                         {63, CPSS_CSCD_LINK_TYPE_PORT_E}]
                   srcPortTrunkHashEn [CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E /
                                       CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E /
                                       CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E],
                   egressAttributesLocallyEn[GT_FALSE, GT_TRUE(SIP5 only), GT_TRUE(SIP5 only)].
    Expected: GT_OK.
    1.2. Call cpssDxChCscdDevMapTableGet with non-NULL pointers,
                                         other params same as in 1.1.
    Expected: GT_OK and same parameters as written.
    1.3. Call with out of range targetHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range sourceHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM for Lion and above, GT_OK for others.
    1.6. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_TRUNK_E],
                   out of range cascadeLinkPtr->linkNum[128],
                   other params same as in 1.1.
    Expected: NOT GT_OK.
    1.7. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_PORT_E],
                   out of range cascadeLinkPtr->linkNum[64],
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.8. Call with cascadeLinkPtr [NULL],
                   other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.9. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_TRUNK_E],
                   out of range srcPortTrunkHashEn [wrong enum values],
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM for DxCh2 and above, GT_OK for DxCh.
    1.10. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_PORT_E],
                   out of range srcPortTrunkHashEn [wrong enum values] (not relevant),
                   other params same as in 1.1.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_U32      index  = 0;


    GT_HW_DEV_NUM                targetHwDevNum  = 0;
    GT_HW_DEV_NUM                sourceHwDevNum  = 0;
    GT_PORT_NUM                  portNum       = 0;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink   = {0, 0};
    CPSS_CSCD_LINK_TYPE_STC      cascadeLinkGet = {0, 0};
    CPSS_PP_FAMILY_TYPE_ENT      devFamily      = CPSS_MAX_FAMILY;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT hashEn    = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
    GT_BOOL                             egressAttributesLocallyEn = GT_FALSE;

    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT  hashEnGet = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
    GT_BOOL                             egressAttributesLocallyEnGet = GT_FALSE;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode;
    GT_TRUNK_ID                         trunkIdArr[] = {2, 63}; /* trunk IDs for the test */
    CPSS_TRUNK_MEMBER_STC               trunkMember[2]; /* trunk members */
    CPSS_PORTS_BMP_STC                  portMembersBmp; /* cascade trunk members */
    GT_U32                              ii,jj; /* iterator */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* create trunks for multi hemisphere device */
        if (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E || PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /* all members must be local device members and
              in same hemisphere */
            trunkMember[0].hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(dev);
            trunkMember[0].port   = 0; /* port group 0 */
            trunkMember[1].hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(dev);
            trunkMember[1].port   = 16; /* port group 1 */

            /* replace ports that already in pre-defined cascade trunk(s) */
            {
                for(jj = 0 ; jj < 2 ; jj++)
                {
                    do
                    {
                        /* check if those ports are not members of some cascade trunk in the system */
                        st = cpssDxChTrunkDbIsMemberOfTrunk(dev,&trunkMember[jj],NULL);
                        if(st == GT_OK)
                        {
                            /* port already in trunk ... try another port */
                            trunkMember[jj].port ++;/**/
                        }
                        else
                        {
                            /* the port not member of a trunk */
                            break;
                        }
                    }
                    while(1);
                }
            }


            for (ii = 0; ii < (sizeof(trunkIdArr)/sizeof(trunkIdArr[0])); ii++)
            {
                /* all members must be local device members and
                  in same hemisphere */
                if (ii == 1)
                {
                    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portMembersBmp);
                    portNum = (4 * 16);
                    if(portNum >= PRV_CPSS_PP_MAC(dev)->numOfPorts)
                    {
                        /* support bobcat2*/
                        portNum = PRV_CPSS_PP_MAC(dev)->numOfPorts - 1;
                    }
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembersBmp, portNum); /* port group 4 */
                    portNum = (6 * 16);
                    if(portNum >= PRV_CPSS_PP_MAC(dev)->numOfPorts)
                    {
                        /* support bobcat2*/
                        portNum = PRV_CPSS_PP_MAC(dev)->numOfPorts - 2;
                    }
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembersBmp, portNum); /* port group 6*/

                    if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        /* support the adding of ports from the 2 hemispheres ! */
                        portNum = 5 + (3 * 16);
                        if(portNum >= PRV_CPSS_PP_MAC(dev)->numOfPorts)
                        {
                            /* support bobcat2*/
                            portNum = PRV_CPSS_PP_MAC(dev)->numOfPorts - 3;
                        }

                        CPSS_PORTS_BMP_PORT_SET_MAC(&portMembersBmp, portNum); /* port group 1 */
                        portNum = 5 + (1 * 16);
                        CPSS_PORTS_BMP_PORT_SET_MAC(&portMembersBmp, portNum); /* port group 3 */
                    }

                    st = cpssDxChTrunkCascadeTrunkPortsSet(dev,trunkIdArr[ii],&portMembersBmp);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsSet: %d %d", dev, trunkIdArr[ii]);
                }
                else
                {
                    st = cpssDxChTrunkMembersSet(dev, trunkIdArr[ii], 2, trunkMember, 0, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet: %d %d", dev, trunkIdArr[ii]);
                }
            }
        }
        /*
            1.1. Call with targetHwDevNum [0 / 1 / 31],
                           sourceHwDevNum [31 / 1 / 0],
                           portNum[0 / 8 / 23],
                           valid cascadeLinkPtr [{2, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {127, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {0, CPSS_CSCD_LINK_TYPE_PORT_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_PORT_E}]
                           srcPortTrunkHashEn [CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E],
                           egressAttributesLocallyEn[GT_FALSE,
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only)].

            Expected: GT_OK.
        */
        targetHwDevNum = 0;
        sourceHwDevNum = 31;
        if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(sourceHwDevNum))
        {
            sourceHwDevNum &= ~1;/* clear LSBits --> make is even (30) */
        }
        portNum      = 0;
        hashEn       = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;

        cascadeLink.linkNum  = trunkIdArr[0];
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
        cascadeLink.linkNum  = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(cascadeLink.linkNum);

        egressAttributesLocallyEn = GT_FALSE;

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum,
                                    portNum, hashEn);

        /*
            1.2. Call cpssDxChCscdDevMapTableGet with non-NULL pointers,
                                                      other params same as in 1.1.
            Expected: GT_OK and same parameters as written
        */
        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                             portNum, &cascadeLinkGet, &hashEnGet, &egressAttributesLocallyEnGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCscdDevMapTableGet: %d", dev);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkNum, cascadeLinkGet.linkNum,
                   "get another cascadeLinkPtr->linkNum than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkType, cascadeLinkGet.linkType,
                   "get another cascadeLinkPtr->linkType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(hashEn, hashEnGet,
                   "get another srcPortTrunkHashEn than was set: %d", dev);

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressAttributesLocallyEn, egressAttributesLocallyEnGet,
                       "got another egressAttributesLocallyEn than was set: %d", dev);
        }

        /*
            1.1. Call with targetHwDevNum [0 / 1 / 31],
                           sourceHwDevNum [31 / 1 / 0],
                           portNum[0 / 8 / 23],
                           valid cascadeLinkPtr [{2, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {127, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {0, CPSS_CSCD_LINK_TYPE_PORT_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_PORT_E}]
                           srcPortTrunkHashEn [CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E],
                           egressAttributesLocallyEn[GT_FALSE,
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only)].

            Expected: GT_OK.
        */
        cascadeLink.linkNum  = 0;
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            egressAttributesLocallyEn = GT_TRUE;
        }

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum,
                                    portNum, hashEn);

        /*
            1.2. Call cpssDxChCscdDevMapTableGet with non-NULL pointers,
                                                 other params same as in 1.1.
            Expected: GT_OK and same parameters as written
        */
        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLinkGet, &hashEnGet, &egressAttributesLocallyEnGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCscdDevMapTableGet: %d", dev);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkNum, cascadeLinkGet.linkNum,
                   "get another cascadeLinkPtr->linkNum than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkType, cascadeLinkGet.linkType,
                   "get another cascadeLinkPtr->linkType than was set: %d", dev);

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressAttributesLocallyEn, egressAttributesLocallyEnGet,
                       "got another egressAttributesLocallyEn than was set: %d", dev);
        }

        /*
            1.1. Call with targetHwDevNum [0 / 1 / 31],
                           sourceHwDevNum [31 / 1 / 0],
                           portNum[0 / 8 / 23],
                           valid cascadeLinkPtr [{2, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {127, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {0, CPSS_CSCD_LINK_TYPE_PORT_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_PORT_E}]
                           srcPortTrunkHashEn [CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E],
                           egressAttributesLocallyEn[GT_FALSE,
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only)].
            Expected: GT_OK.
        */
        targetHwDevNum = 1;
        sourceHwDevNum = 1;
        portNum      = 8;
        hashEn       = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E;

        cascadeLink.linkNum  = trunkIdArr[1];
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
        cascadeLink.linkNum  = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(cascadeLink.linkNum);

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            egressAttributesLocallyEn = GT_TRUE;
        }

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);

        if((0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev)) && (devFamily > CPSS_PP_FAMILY_CHEETAH_E))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChCscdDevMapTableGet: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum,
                                    portNum, hashEn);

            /*
                1.2. Call cpssDxChCscdDevMapTableGet with non-NULL pointers,
                                                     other params same as in 1.1.
                Expected: Lion B0 and above - GT_OK and same parameters as written
                          Other GT_BAD_PARAM
            */
            st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                            portNum, &cascadeLinkGet, &hashEnGet, &egressAttributesLocallyEnGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCscdDevMapTableGet: %d", dev);

            /* validation values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkNum, cascadeLinkGet.linkNum,
                       "get another cascadeLinkPtr->linkNum than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkType, cascadeLinkGet.linkType,
                       "get another cascadeLinkPtr->linkType than was set: %d", dev);

            if (devFamily > CPSS_PP_FAMILY_CHEETAH_E)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(hashEn, hashEnGet,
                       "get another srcPortTrunkHashEn than was set: %d", dev);
            }

            if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(egressAttributesLocallyEn, egressAttributesLocallyEnGet,
                           "got another egressAttributesLocallyEn than was set: %d", dev);
            }
        }

        /*
            1.1. Call with targetHwDevNum [0 / 1 / 31],
                           sourceHwDevNum [31 / 1 / 0],
                           portNum[0 / 8 / 23],
                           valid cascadeLinkPtr [{2, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {127, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {0, CPSS_CSCD_LINK_TYPE_PORT_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_PORT_E}]
                           srcPortTrunkHashEn [CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E],
                           egressAttributesLocallyEn[GT_FALSE,
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only)].
            Expected: GT_OK.
        */
        targetHwDevNum = 31;
        if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(targetHwDevNum))
        {
            targetHwDevNum &= ~1;/* clear LSBits --> make is even (30) */
        }

        sourceHwDevNum = 0;
        portNum      = 23;
        hashEn       = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E;

        cascadeLink.linkNum  = trunkIdArr[1];
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
        cascadeLink.linkNum  = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(cascadeLink.linkNum);

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            egressAttributesLocallyEn = GT_TRUE;
        }

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum,
                                    portNum, hashEn);

        /*
            1.2. Call cpssDxChCscdDevMapTableGet with non-NULL pointers,
                                                 other params same as in 1.1.
            Expected: GT_OK and same parameters as written
        */
        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLinkGet, &hashEnGet, &egressAttributesLocallyEnGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCscdDevMapTableGet: %d", dev);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkNum, cascadeLinkGet.linkNum,
                   "get another cascadeLinkPtr->linkNum than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkType, cascadeLinkGet.linkType,
                   "get another cascadeLinkPtr->linkType than was set: %d", dev);
        if (devFamily > CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(hashEn, hashEnGet,
                   "get another srcPortTrunkHashEn than was set: %d", dev);
        }

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressAttributesLocallyEn, egressAttributesLocallyEnGet,
                       "got another egressAttributesLocallyEn than was set: %d", dev);
        }

        /*
            1.1. Call with targetHwDevNum [0 / 1 / 31],
                           sourceHwDevNum [31 / 1 / 0],
                           portNum[0 / 8 / 23],
                           valid cascadeLinkPtr [{2, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {127, CPSS_CSCD_LINK_TYPE_TRUNK_E} /
                                                 {0, CPSS_CSCD_LINK_TYPE_PORT_E} /
                                                 {63, CPSS_CSCD_LINK_TYPE_PORT_E}]
                           srcPortTrunkHashEn [CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E /
                                               CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E],
                           egressAttributesLocallyEn[GT_FALSE,
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only),
                                                     GT_TRUE(SIP5 only)].
            Expected: GT_OK.
        */
        cascadeLink.linkNum  = trunkIdArr[1];
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            egressAttributesLocallyEn = GT_TRUE;
        }

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum,
                                    portNum, hashEn);

        /*
            1.2. Call cpssDxChCscdDevMapTableGet with non-NULL pointers,
                                                 other params same as in 1.1.
            Expected: GT_OK and same parameters as written
        */
        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLinkGet, &hashEnGet, &egressAttributesLocallyEnGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCscdDevMapTableGet: %d", dev);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkNum, cascadeLinkGet.linkNum,
                   "get another cascadeLinkPtr->linkNum than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cascadeLink.linkType, cascadeLinkGet.linkType,
                   "get another cascadeLinkPtr->linkType than was set: %d", dev);

        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(egressAttributesLocallyEn, egressAttributesLocallyEnGet,
                       "got another egressAttributesLocallyEn than was set: %d", dev);
        }

        /*
            1.3. Call with out of range targetHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */

        /* call with targetHwDevNum = PRV_CPSS_MAX_PP_DEVICES_CNS */
        targetHwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, targetHwDevNum);

        targetHwDevNum = 0;

        /*
            1.4. Call with out of range sourceHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);

        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* call with sourceHwDevNum = 32 */
        sourceHwDevNum = 32;

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);

        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sourceHwDevNum);

        /* call with sourceHwDevNum = PRV_CPSS_MAX_PP_DEVICES_CNS */
        sourceHwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sourceHwDevNum);

        sourceHwDevNum = 0;

        /*
            1.5. Call with out of range portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM for Lion and above, GT_OK for others.
        */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);

        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);

        if(0 != UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
        }

        portNum = 0;

        /*
            1.6. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_TRUNK_E],
                           out of range cascadeLinkPtr->linkNum[128],
                           other params same as in 1.1.
            Expected: NOT GT_OK.
        */
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            cascadeLink.linkNum  = 1 + FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(0xFFFFFFFF);
        }
        else
        {
            cascadeLink.linkNum  = 128;
        }


        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cascadeLink.linkNum);

        cascadeLink.linkNum = trunkIdArr[0];

        /*
            1.7. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_PORT_E],
                           out of range cascadeLinkPtr->linkNum[64],
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
        cascadeLink.linkNum  = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, cascadeLink.linkNum);

        cascadeLink.linkNum = 0;

        /*
            1.8. Call with cascadeLinkPtr [NULL],
                           other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, NULL, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cascadeLinkPtr = NULL", dev);

        /*
            1.9. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_TRUNK_E],
                           out of range srcPortTrunkHashEn [wrong enum values],
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM for DxCh2 and above, GT_OK for DxCh.
        */
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
        cascadeLink.linkNum  = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(cascadeLink.linkNum);

        for(index = 0; index < utfInvalidEnumArrSize; index++)
        {
            hashEn = utfInvalidEnumArr[index];

            st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                            portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
            if (devFamily >= CPSS_PP_FAMILY_CHEETAH2_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, hashEn);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashEn);
            }
        }

        hashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;

        /*
            1.10. Call with cascadeLinkPtr->linkType[CPSS_CSCD_LINK_TYPE_PORT_E],
                           out of range srcPortTrunkHashEn [wrong enum values] (not relevant),
                           other params same as in 1.1.
            Expected: GT_OK.
        */
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

        for(index = 0; index < utfInvalidEnumArrSize; index++)
        {
            hashEn = utfInvalidEnumArr[index];

            st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                            portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashEn);
        }

        hashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;

        /* delete trunks for multi hemisphere device */
        if (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E || PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            for (ii = 0; ii < (sizeof(trunkIdArr)/sizeof(trunkIdArr[0])); ii++)
            {
                if (ii == 1)
                {
                    st = cpssDxChTrunkCascadeTrunkPortsSet(dev,trunkIdArr[ii],NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsSet: %d %d", dev, trunkIdArr[ii]);
                }
                else
                {
                    st = cpssDxChTrunkMembersSet(dev, trunkIdArr[ii], 0, NULL, 0, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet: %d %d", dev, trunkIdArr[ii]);
                }
            }
        }
    }

    targetHwDevNum = 0;
    sourceHwDevNum = 31;
    if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(sourceHwDevNum))
    {
        sourceHwDevNum &= ~1;/* clear LSBits --> make is even (30) */
    }
    portNum      = 0;
    hashEn       = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;

    cascadeLink.linkNum  = trunkIdArr[0];
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    cascadeLink.linkNum  = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(cascadeLink.linkNum);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdDevMapTableSet(dev, targetHwDevNum, sourceHwDevNum,
                                    portNum, &cascadeLink, hashEn, egressAttributesLocallyEn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDevMapTableGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                targetHwDevNum,
    IN GT_U8                                sourceHwDevNum,
    IN GT_U8                                portNum,
    OUT CPSS_CSCD_LINK_TYPE_STC             *cascadeLinkPtr,
    OUT CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT  *srcPortTrunkHashEnPtr
    OUT GT_BOOL                             *egressAttributesLocallyEnPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDevMapTableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with targetHwDevNum [0 / 1 / 31],
                   sourceHwDevNum [31 / 1 / 0],
                   portNum[0 / 8 / 23],
                   not NULL cascadeLinkPtr,
                   not NULL srcPortTrunkHashEnPtr.
    Expected: GT_OK.
    1.2. Call with out of range targetHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range sourceHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   other params same as in 1.1.
    Expected: GT_BAD_PARAM for Lion and above, GT_OK for others.
    1.5. Call with cascadeLinkPtr [NULL],
                   other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with srcPortTrunkHashEnPtr[NULL],
                   other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.7. For Bobcat2; Caelum; Bobcat3 only: call with egressAttributesLocallyEnPtr[NULL],
                   other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;

    GT_HW_DEV_NUM                targetHwDevNum  = 0;
    GT_HW_DEV_NUM                sourceHwDevNum  = 0;
    GT_PORT_NUM                  portNum       = 0;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink  = {0, 0};
    CPSS_PP_FAMILY_TYPE_ENT      devFamily    = CPSS_MAX_FAMILY;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT hashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
    GT_BOOL                      egressAttributesLocallyEn;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT mode;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Call with targetHwDevNum [0 / 1 / 31],
                           sourceHwDevNum [31 / 1 / 0],
                           portNum[0 / 8 / 23],
                           not NULL cascadeLinkPtr,
                           not NULL srcPortTrunkHashEnPtr.
            Expected: GT_OK.
        */

        /* call with targetHwDevNum = 0 */
        targetHwDevNum = 0;
        sourceHwDevNum = 31;
        if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(sourceHwDevNum))
        {
            sourceHwDevNum &= ~1;/* clear LSBits --> make is even (30) */
        }
        portNum      = 0;

        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum, portNum);

        /* call with targetHwDevNum = 1 */
        targetHwDevNum = 1;
        sourceHwDevNum = 1;
        portNum      = 8;

        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum, portNum);

        /* call with targetHwDevNum = 31 */
        targetHwDevNum = 31;
        if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(targetHwDevNum))
        {
            targetHwDevNum &= ~1;/* clear LSBits --> make is even (30) */
        }
        sourceHwDevNum = 1;
        portNum      = 23;

        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, targetHwDevNum, sourceHwDevNum, portNum);

       /*
            1.2. Call with out of range targetHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */

        /* call with targetHwDevNum = PRV_CPSS_MAX_PP_DEVICES_CNS */
        targetHwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, targetHwDevNum);

        targetHwDevNum = 0;

        /*
            1.3. Call with out of range sourceHwDevNum [32 / PRV_CPSS_MAX_PP_DEVICES_CNS]
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */

        /* call with sourceHwDevNum = 32 */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);

        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        sourceHwDevNum = 32;

        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sourceHwDevNum);

        /* call with sourceHwDevNum = PRV_CPSS_MAX_PP_DEVICES_CNS */
        sourceHwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sourceHwDevNum);

        sourceHwDevNum = 0;

        /*
            1.4. Call with out of range portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           other params same as in 1.1.
            Expected: GT_BAD_PARAM for Lion and above, GT_OK for others.
        */

        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);

        if(0 == UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, sourceHwDevNum);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);

        if(0 != UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
        }

        portNum = 0;

        /*
            1.5. Call with cascadeLinkPtr [NULL],
                           other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, NULL, &hashEn, &egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cascadeLinkPtr = NULL", dev);

        /*
            1.6. Call with srcPortTrunkHashEnPtr [NULL],
                           other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, NULL, &egressAttributesLocallyEn);
        if (devFamily > CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, srcPortTrunkHashEnPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, srcPortTrunkHashEnPtr = NULL", dev);
        }

        /*
            1.7. For Bobcat2; Caelum; Bobcat3 only: call with egressAttributesLocallyEnPtr[NULL],
                       other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                            portNum, &cascadeLink, &hashEn, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                            "%d, egressAttributesLocallyEnPtr = NULL", dev);
        }
    }

    targetHwDevNum = 0;
    sourceHwDevNum = 31;
    if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(sourceHwDevNum))
    {
        sourceHwDevNum &= ~1;/* clear LSBits --> make is even (30) */
    }
    portNum      = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                        portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdDevMapTableGet(dev, targetHwDevNum, sourceHwDevNum,
                                    portNum, &cascadeLink, &hashEn, &egressAttributesLocallyEn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdRemapQosModeSet
(
   IN GT_U8                             devNum,
   IN GT_U8                             portNum,
   IN CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT remapType
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdRemapQosModeSet)
{
/*
    ITERATE_DEVICES_PHY_ PORTS (DxChx)
    1.1.1. Call with remapType [CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E /
                                CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E /
                                CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E /
                                CPSS_DXCH_CSCD_QOS_REMAP_ALL_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdRemapQosModeGet
    Expected: GT_OK and same remapType.
    1.1.3. Call with wrong enum values remapType .
    Expected: GT_BAD_PARAM.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63]
                   and valid other parameters.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                         st        = GT_OK;

    GT_U8                             dev;
    GT_PHYSICAL_PORT_NUM              port      = CSCD_VALID_PHY_PORT_CNS;
    CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT remapType = CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E;
    CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT remapTypeGet = CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with remapType [CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E /
                                CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E /
                                CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E /
                                CPSS_DXCH_CSCD_QOS_REMAP_ALL_E].
                Expected: GT_OK.
            */

            /* Call function with remapType [CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E] */
            remapType = CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E;

            st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            /*  1.1.2.  */
            st = cpssDxChCscdRemapQosModeGet(dev, port, &remapTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            UTF_VERIFY_EQUAL1_STRING_MAC(remapType, remapTypeGet,
                       "get another remapType than was set: %d", dev);

            /* Call function with remapType [CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E] */
            remapType = CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E;

            st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            /*  1.1.2.  */
            st = cpssDxChCscdRemapQosModeGet(dev, port, &remapTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            UTF_VERIFY_EQUAL1_STRING_MAC(remapType, remapTypeGet,
                       "get another remapType than was set: %d", dev);

            /* Call function with remapType [CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E] */
            remapType = CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E;

            st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            /*  1.1.2.  */
            st = cpssDxChCscdRemapQosModeGet(dev, port, &remapTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            UTF_VERIFY_EQUAL1_STRING_MAC(remapType, remapTypeGet,
                       "get another remapType than was set: %d", dev);

            /* Call function with remapType [CPSS_DXCH_CSCD_QOS_REMAP_ALL_E] */
            remapType = CPSS_DXCH_CSCD_QOS_REMAP_ALL_E;

            st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            /*  1.1.2.  */
            st = cpssDxChCscdRemapQosModeGet(dev, port, &remapTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, remapType);

            UTF_VERIFY_EQUAL1_STRING_MAC(remapType, remapTypeGet,
                       "get another remapType than was set: %d", dev);
            /*
                1.1.3. Call with wrong enum values remapType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdRemapQosModeSet
                                (dev, port, remapType),
                                remapType);
        }

        remapType = CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    remapType = CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdRemapQosModeSet(dev, port, remapType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdRemapQosModeGet)
{
/*
    ITERATE_DEVICES_PHY_ PORTS (DxChx)
    1.1.1. Call with not NULL remapTypePtr.
    Expected: GT_OK.
    1.1.2. Call NULL remapType .
    Expected: GT_BAD_PTR.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63]
                   and valid other parameters.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                         st        = GT_OK;

    GT_U8                             dev;
    GT_PHYSICAL_PORT_NUM              port      = CSCD_VALID_PHY_PORT_CNS;
    CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT remapType = CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*  1.1.1.   */
            st = cpssDxChCscdRemapQosModeGet(dev, port, &remapType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.   */
            st = cpssDxChCscdRemapQosModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdRemapQosModeGet(dev, port, &remapType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdRemapQosModeGet(dev, port, &remapType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdRemapQosModeGet(dev, port, &remapType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdRemapQosModeGet(dev, port, &remapType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdRemapQosModeGet(dev, port, &remapType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCtrlQosSet
(
   IN GT_U8             devNum,
   IN GT_U8             ctrlTc,
   IN CPSS_DP_LEVEL_ENT ctrlDp,
   IN CPSS_DP_LEVEL_ENT cpuToCpuDp
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCtrlQosSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with ctrlTc [1, 2],
                   ctrlTc [CPSS_DP_GREEN_E /
                           CPSS_DP_RED_E]
                   and cpuToCpuDp [CPSS_DP_GREEN_E /
                                   CPSS_DP_RED_E].
    Expected: GT_OK.
    1.2. Call with out of range ctrlTc [CPSS_TC_RANGE_CNS = 8],
                   ctrlTc [CPSS_DP_GREEN_E]
                   and cpuToCpuDp [CPSS_DP_RED_E].
    Expected: GT_BAD_PARAM.
    1.3. Call with tc [1],
                   wrong enum values ctrlTc
                   and cpuToCpuDp [CPSS_DP_GREEN_E].
    Expected: GT_BAD_PARAM.
    1.4. Call with tc [2],
                   ctrlTc [CPSS_DP_GREEN_E]
                   and wrong enum values cpuToCpuDp .
    Expected: GT_BAD_PARAM.
    1.5. Call with ctrlTc [3],
                   ctrlTc [CPSS_DP_YELLOW_E]
                   and cpuToCpuDp [CPSS_DP_GREEN_E].
    Expected: NOT GT_OK for DxCh.
    1.6. Call with ctrlTc [4],
                   ctrlTc [CPSS_DP_GREEN_E]
                   and cpuToCpuDp [CPSS_DP_YELLOW_E].
    Expected: NOT GT_OK for DxCh.
*/
    GT_STATUS               st         = GT_OK;

    GT_U8                   dev;
    GT_U8                   ctrlTc        = 0;
    CPSS_DP_LEVEL_ENT       ctrlDp        = CPSS_DP_GREEN_E;
    CPSS_DP_LEVEL_ENT       cpuToCpuDp    = CPSS_DP_GREEN_E;
    GT_U8                   ctrlTcGet     = 0;
    CPSS_DP_LEVEL_ENT       ctrlDpGet     = CPSS_DP_GREEN_E;
    CPSS_DP_LEVEL_ENT       cpuToCpuDpGet = CPSS_DP_GREEN_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with ctrlTc [1, 2],
                           ctrlDp [CPSS_DP_GREEN_E /
                                   CPSS_DP_RED_E]
                            and cpuToCpuDp [CPSS_DP_GREEN_E /
                                            CPSS_DP_RED_E].
            Expected: GT_OK.
        */

        /* Call with ctrlTc [1], ctrlDp [CPSS_DP_GREEN_E], cpuToCpuDp [CPSS_DP_GREEN_E] */
        ctrlTc     = 1;
        ctrlDp     = CPSS_DP_GREEN_E;
        cpuToCpuDp = CPSS_DP_GREEN_E;

        st = cpssDxChCscdCtrlQosSet(dev, ctrlTc, ctrlDp, cpuToCpuDp);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ctrlTc, ctrlDp, cpuToCpuDp);

        /* 1.2. */
        st = cpssDxChCscdCtrlQosGet(dev, &ctrlTcGet, &ctrlDpGet, &cpuToCpuDpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ctrlTc, ctrlTcGet,
                   "get another ctrlTc than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ctrlDp, ctrlDpGet,
                   "get another ctrlDp than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuToCpuDp, cpuToCpuDpGet,
                   "get another cpuToCpuDp than was set: %d", dev);

        /* Call with ctrlTc [2], ctrlDp [CPSS_DP_RED_E], cpuToCpuDp [CPSS_DP_RED_E] */
        ctrlTc     = 2;
        ctrlDp     = CPSS_DP_RED_E;
        cpuToCpuDp = CPSS_DP_RED_E;

        st = cpssDxChCscdCtrlQosSet(dev, ctrlTc, ctrlDp, cpuToCpuDp);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, ctrlTc, ctrlDp, cpuToCpuDp);

        /* 1.2. */
        st = cpssDxChCscdCtrlQosGet(dev, &ctrlTcGet, &ctrlDpGet, &cpuToCpuDpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ctrlTc, ctrlTcGet,
                   "get another ctrlTc than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ctrlDp, ctrlDpGet,
                   "get another ctrlDp than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(cpuToCpuDp, cpuToCpuDpGet,
                   "get another cpuToCpuDp than was set: %d", dev);

        /*
            1.2. Call with out of range ctrlTc [CPSS_TC_RANGE_CNS = 8],
                   ctrlDp [CPSS_DP_GREEN_E]
                   and cpuToCpuDp [CPSS_DP_RED_E].
            Expected: GT_BAD_PARAM.
        */
        ctrlTc     = CPSS_TC_RANGE_CNS;
        ctrlDp     = CPSS_DP_GREEN_E;

        st = cpssDxChCscdCtrlQosSet(dev, ctrlTc, ctrlDp, cpuToCpuDp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, ctrlTc);

        /*
            1.3. Call with tc [1], wrong enum values ctrlTc
                           and cpuToCpuDp [CPSS_DP_GREEN_E].
            Expected: GT_BAD_PARAM.
        */
        ctrlTc     = 1;
        cpuToCpuDp = CPSS_DP_GREEN_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChCscdCtrlQosSet
                            (dev, ctrlTc, ctrlDp, cpuToCpuDp),
                            ctrlDp);

        /*
            1.4. Call with tc [2], ctrlTc [CPSS_DP_GREEN_E]
                           and wrong enum values cpuToCpuDp.
            Expected: GT_BAD_PARAM.
        */
        ctrlTc     = 2;
        ctrlDp     = CPSS_DP_GREEN_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChCscdCtrlQosSet
                            (dev, ctrlTc, ctrlDp, cpuToCpuDp),
                            cpuToCpuDp);

        /*
            1.5. Call with ctrlTc [3],
                   ctrlTc [CPSS_DP_YELLOW_E]
                   and cpuToCpuDp [CPSS_DP_GREEN_E].
            Expected: NOT GT_OK for DxCh.
        */
        ctrlTc     = 3;
        ctrlDp     = CPSS_DP_YELLOW_E;
        cpuToCpuDp = CPSS_DP_GREEN_E;

        st = cpssDxChCscdCtrlQosSet(dev, ctrlTc, ctrlDp, cpuToCpuDp);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ctrlDp = %d", dev, ctrlDp);

        /*
            1.6. Call with ctrlTc [4],
                   ctrlTc [CPSS_DP_GREEN_E]
                   and cpuToCpuDp [CPSS_DP_YELLOW_E].
            Expected: NOT GT_OK for DxCh.
        */
        ctrlTc     = 4;
        ctrlDp     = CPSS_DP_GREEN_E;
        cpuToCpuDp = CPSS_DP_YELLOW_E;

        st = cpssDxChCscdCtrlQosSet(dev, ctrlTc, ctrlDp, cpuToCpuDp);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, cpuToCpuDp = %d", dev, cpuToCpuDp);
    }

    ctrlTc     = 1;
    ctrlDp     = CPSS_DP_GREEN_E;
    cpuToCpuDp = CPSS_DP_GREEN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCtrlQosSet(dev, ctrlTc, ctrlDp, cpuToCpuDp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCtrlQosSet(dev, ctrlTc, ctrlDp, cpuToCpuDp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCtrlQosGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with not NULL ctrlTcPtr, ctrlDpPtr, cpuToCpuDpPtr.
    Expected: GT_OK.
    1.2. Call with NULL ctrlTcPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL ctrlDpPtr.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL cpuToCpuDpPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st         = GT_OK;

    GT_U8                   dev;
    GT_U8                   ctrlTc     = 0;
    CPSS_DP_LEVEL_ENT       ctrlDp     = CPSS_DP_GREEN_E;
    CPSS_DP_LEVEL_ENT       cpuToCpuDp = CPSS_DP_GREEN_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChCscdCtrlQosGet(dev, &ctrlTc, &ctrlDp, &cpuToCpuDp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChCscdCtrlQosGet(dev, NULL, &ctrlDp, &cpuToCpuDp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChCscdCtrlQosGet(dev, &ctrlTc, NULL, &cpuToCpuDp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.4.  */
        st = cpssDxChCscdCtrlQosGet(dev, &ctrlTc, &ctrlDp, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCtrlQosGet(dev, &ctrlTc, &ctrlDp, &cpuToCpuDp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCtrlQosGet(dev, &ctrlTc, &ctrlDp, &cpuToCpuDp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdRemapDataQosTblSet
(
   IN GT_U8              devNum,
   IN GT_U8              tc,
   IN CPSS_DP_LEVEL_ENT  dp,
   IN GT_U8              remapTc,
   IN CPSS_DP_LEVEL_ENT  remapDp
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdRemapDataQosTblSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with tc [1, 2],
                   dp [CPSS_DP_GREEN_E /
                       CPSS_DP_RED_E]
                   remapTc [3, 4]
                   and remapDp [CPSS_DP_GREEN_E /
                                CPSS_DP_RED_E].
    Expected: GT_OK.
    1.2. Call cpssDxChCscdRemapDataQosTblGet.
    Expected: GT_OK and same remapTc and remapDp.
    1.3. Call with out of range tc [CPSS_TC_RANGE_CNS = 8],
                   dp [CPSS_DP_GREEN_E],
                   remapTc [5]
                   and remapDp [CPSS_DP_RED_E].
    Expected: GT_BAD_PARAM.
    1.4. Call with tc [1],
                   dp [CPSS_DP_RED_E],
                   out of range remapTc [CPSS_TC_RANGE_CNS = 8]
                   and remapDp [CPSS_DP_GREEN_E].
    Expected: GT_BAD_PARAM.
    1.5. Call with tc [1],
                   wrong enum values dp
                   remapTc[2]
                   and remapDp [CPSS_DP_GREEN_E].
    Expected: GT_BAD_PARAM.
    1.6. Call with tc [2],
                   dp [CPSS_DP_GREEN_E]
                   remapTc[4]
                   and wrong enum values remapDp .
    Expected: GT_BAD_PARAM.
    1.7. Call with tc [3],
                   dp [CPSS_DP_YELLOW_E]
                   remapTc [5]
                   and remapDp [CPSS_DP_GREEN_E].
    Expected: NOT GT_OK for DxCh.
    1.8. Call with tc [4],
                   dp [CPSS_DP_GREEN_E]
                   remapTc [6]
                   and remapDp [CPSS_DP_YELLOW_E].
    Expected: NOT GT_OK for DxCh.
*/
    GT_STATUS               st         = GT_OK;

    GT_U8                   dev;
    GT_U8                   tc      = 0;
    CPSS_DP_LEVEL_ENT       dp      = CPSS_DP_GREEN_E;
    GT_U8                   remapTc = 0;
    CPSS_DP_LEVEL_ENT       remapDp = CPSS_DP_GREEN_E;
    GT_U8                   remapTcGet = 0;
    CPSS_DP_LEVEL_ENT       remapDpGet = CPSS_DP_GREEN_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tc [1, 2],
                           dp [CPSS_DP_GREEN_E /
                               CPSS_DP_RED_E]
                            remapTc [3, 4]
                            and remapDp [CPSS_DP_GREEN_E /
                                         CPSS_DP_RED_E].
            Expected: GT_OK.
        */

        /* Call with tc [1], dp [CPSS_DP_GREEN_E] */
        /* remapTc [3], remapDp [CPSS_DP_GREEN_E] */
        tc      = 1;
        dp      = CPSS_DP_GREEN_E;
        remapTc = 3;
        remapDp = CPSS_DP_GREEN_E;

        st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, tc, dp, remapTc, remapDp);

        /*  1.2.  */
        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTcGet, &remapDpGet);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, tc, dp, remapTc, remapDp);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(remapTc, remapTcGet,
                   "get another remapTc than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(remapDp, remapDpGet,
                   "get another remapDp than was set: %d", dev);

        /* Call with tc [2], dp [CPSS_DP_RED_E] */
        /* remapTc [4], remapDp [CPSS_DP_RED_E] */
        tc      = 2;
        dp      = CPSS_DP_RED_E;
        remapTc = 4;
        remapDp = CPSS_DP_RED_E;

        st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, tc, dp, remapTc, remapDp);

        /*  1.2.  */
        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTcGet, &remapDpGet);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, tc, dp, remapTc, remapDp);

        /* validation values */
        UTF_VERIFY_EQUAL1_STRING_MAC(remapTc, remapTcGet,
                   "get another remapTc than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(remapDp, remapDpGet,
                   "get another remapDp than was set: %d", dev);

        /*
            1.3. Call with out of range tc [CPSS_TC_RANGE_CNS = 8],
                           dp [CPSS_DP_GREEN_E],
                           remapTc [5]
                           and remapDp [CPSS_DP_RED_E].
            Expected: GT_BAD_PARAM.
        */
        tc      = CPSS_TC_RANGE_CNS;
        dp      = CPSS_DP_GREEN_E;
        remapTc = 5;

        st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tc);

        /*
            1.4. Call with tc [1],
                           dp [CPSS_DP_RED_E],
                           out of range remapTc [CPSS_TC_RANGE_CNS = 8]
                           and remapDp [CPSS_DP_GREEN_E].
            Expected: GT_BAD_PARAM.
        */
        tc      = 1;
        dp      = CPSS_DP_RED_E;
        remapTc = CPSS_TC_RANGE_CNS;
        remapDp = CPSS_DP_GREEN_E;

        st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, remapTc = %d", dev, remapTc);

        /*
            1.5. Call with tc [1], wrong enum values dp
                           remapTc[2] and remapDp [CPSS_DP_GREEN_E].
            Expected: GT_BAD_PARAM.
        */
        remapTc = 2;

        UTF_ENUMS_CHECK_MAC(cpssDxChCscdRemapDataQosTblSet
                            (dev, tc, dp, remapTc, remapDp),
                            remapDp);

        /*
            1.6. Call with tc [2], dp [CPSS_DP_GREEN_E], remapTc[4]
                   and wrong enum values remapDp.
            Expected: GT_BAD_PARAM.
        */
        tc      = 2;
        dp      = CPSS_DP_GREEN_E;
        remapTc = 4;

        UTF_ENUMS_CHECK_MAC(cpssDxChCscdRemapDataQosTblSet
                            (dev, tc, dp, remapTc, remapDp),
                            remapDp);

        /*
            1.7. Call with tc [3], dp [CPSS_DP_YELLOW_E], remapTc [5]
                           and remapDp [CPSS_DP_GREEN_E].
            Expected: NOT GT_OK for DxCh.
        */
        tc      = 3;
        dp      = CPSS_DP_YELLOW_E;
        remapTc = 5;
        remapDp = CPSS_DP_GREEN_E;

        st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, dp = %d", dev, dp);

        /*
            1.8. Call with tc [4],
                           dp [CPSS_DP_GREEN_E]
                           remapTc [6]
                           and remapDp [CPSS_DP_YELLOW_E].
            Expected: NOT GT_OK for DxCh.
        */
        tc      = 4;
        dp      = CPSS_DP_GREEN_E;
        remapTc = 6;
        remapDp = CPSS_DP_YELLOW_E;

        st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, remapDp = %d", dev, remapDp);
    }

    tc      = 1;
    dp      = CPSS_DP_GREEN_E;
    remapTc = 3;
    remapDp = CPSS_DP_GREEN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdRemapDataQosTblSet(dev, tc, dp, remapTc, remapDp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdRemapDataQosTblGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with tc [1, 2],
                   dp [CPSS_DP_GREEN_E /
                       CPSS_DP_RED_E]
               and not NULL remapTcPtr remapDpPtr.
    Expected: GT_OK.
    1.2. Call with out of range tc [CPSS_TC_RANGE_CNS = 8],
                   dp [CPSS_DP_GREEN_E].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enum values dp.
    Expected: GT_BAD_PARAM.
    1.4. Call with NULL remapTcPtr.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL remapDpPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st         = GT_OK;

    GT_U8                   dev;
    GT_U8                   tc      = 0;
    CPSS_DP_LEVEL_ENT       dp      = CPSS_DP_GREEN_E;
    GT_U8                   remapTc = 0;
    CPSS_DP_LEVEL_ENT       remapDp = CPSS_DP_GREEN_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        tc      = 1;
        dp      = CPSS_DP_GREEN_E;

        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTc, &remapDp);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tc, dp);

        /*  1.1.  */
        tc      = 2;
        dp      = CPSS_DP_RED_E;

        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTc, &remapDp);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tc, dp);

        /*  1.2.  */
        tc      = CPSS_TC_RANGE_CNS;

        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTc, &remapDp);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tc);
        tc      = 1;

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdRemapDataQosTblGet(dev, tc, dp,
                              &remapTc, &remapDp),
                              dp);

        /*  1.4.  */
        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, NULL, &remapDp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, tc);

        /*  1.5.  */
        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTc, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, tc);
    }

    tc      = 1;
    dp      = CPSS_DP_GREEN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTc, &remapDp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdRemapDataQosTblGet(dev, tc, dp, &remapTc, &remapDp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDsaSrcDevFilterSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enableOwnDevFltr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDsaSrcDevFilterSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with enableOwnDevFltr [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enableOwnDevFltr [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        /* Call function with enableOwnDevFltr [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChCscdDsaSrcDevFilterSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call GET function  with same params.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdDsaSrcDevFilterGet(dev, &stateGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
             "get another enableOwnDevFltr than was set: %d", dev);

        /* Call function with enableOwnDevFltr [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChCscdDsaSrcDevFilterSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call GET function  with same params.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdDsaSrcDevFilterGet(dev, &stateGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
             "get another enableOwnDevFltr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdDsaSrcDevFilterSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdDsaSrcDevFilterSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDsaSrcDevFilterGet
(
    IN GT_U8        devNum,
    OUT GT_BOOL      *enableOwnDevFltrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDsaSrcDevFilterGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL enableOwnDevFltrPtr.
    Expected: GT_OK.
    1.2. Call with enableOwnDevFltrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_BOOL     enableOwnDevFlt = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. */
        st = cpssDxChCscdDsaSrcDevFilterGet(dev, &enableOwnDevFlt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1.2. */
        st = cpssDxChCscdDsaSrcDevFilterGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enableOwnDevFltrPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdDsaSrcDevFilterGet(dev, &enableOwnDevFlt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdDsaSrcDevFilterGet(dev, &enableOwnDevFlt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      vlanTagTpid
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with enableOwnDevFltr [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32     vlanTpid    = 52;
    GT_U32     vlanTpidGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enableOwnDevFltr [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        /* Call function with enableOwnDevFltr [GT_FALSE] */

        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(dev, vlanTpid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanTpid);

        /*
            1.2. Call GET function  with same params.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(dev, &vlanTpidGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanTpidGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(vlanTpid, vlanTpidGet,
             "get another enableOwnDevFltr than was set: %d", dev);
        /*
           1.1. Call with enableOwnDevFltr [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
       */

        vlanTpid    = 0;
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(dev, vlanTpid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanTpid);

        /*
            1.2. Call GET function  with same params.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(dev, &vlanTpidGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanTpidGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(vlanTpid, vlanTpidGet,
             "get another enableOwnDevFltr than was set: %d", dev);

        vlanTpid    = 0xffff;
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(dev, vlanTpid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanTpid);

        /*
            1.2. Call GET function  with same params.
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(dev, &vlanTpidGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanTpidGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(vlanTpid, vlanTpidGet,
             "get another enableOwnDevFltr than was set: %d", dev);

        /*error case*/

        vlanTpid    = 0x10000;
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(dev, vlanTpid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, vlanTpid);

     }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(dev, vlanTpid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(dev, vlanTpid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet
(
  IN GT_U8        devNum,
  OUT GT_U32      *vlanTagTpidPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL enableOwnDevFltrPtr.
    Expected: GT_OK.
    1.2. Call with enableOwnDevFltrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32     vlanTpid = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. */
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(dev, &vlanTpid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1.2. */
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enableOwnDevFltrPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(dev, &vlanTpid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(dev, &vlanTpid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdHyperGInternalPortModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_U8                                portNum,
    IN  CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdHyperGInternalPortModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh1 DxCh2)
    1.1.1. Call with mode [CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E /
                           CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ACCELERATED_E].
    Expected: GT_OK for XG ports and GT_NOT_SUPPORTED for others.
    1.1.2. Call cpssDxChCscdHyperGInternalPortModeGet.
    Expected: GT_OK and same mode.
    1.1.3. Call with wrong enum values mode.
    Expected: GT_BAD_PARAM.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: NOT GT_OK.
*/
    GT_STATUS                            st   = GT_OK;

    GT_U8                                dev;
    GT_PHYSICAL_PORT_NUM                 port = CSCD_VALID_PHY_PORT_CNS;
    CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT mode = CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E;
    CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT modeGet = CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E;
    PRV_CPSS_PORT_TYPE_ENT               portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E  | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /*
                1.1.1. Call with mode [CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E /
                                       CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ACCELERATED_E].
                Expected: GT_OK for XG ports and GT_NOT_SUPPORTED for others.
            */

            /* Call with mode [CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E] */
            mode = CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E;

            st = cpssDxChCscdHyperGInternalPortModeSet(dev, port, mode);
            if (PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                /* 1.1.2. */
                st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &modeGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                    "get another mode than was set: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, mode);
            }

            /* Call with mode [CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ACCELERATED_E] */
            mode = CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ACCELERATED_E;

            st = cpssDxChCscdHyperGInternalPortModeSet(dev, port, mode);
            if (PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                /* 1.1.2. */
                st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &modeGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                    "get another mode than was set: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, mode);
            }

            /*
                1.1.3. Call with wrong enum values mode .
                Expected: GT_BAD_PARAM.
            */
            if (PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChCscdHyperGInternalPortModeSet
                                    (dev, port, mode),
                                    mode);
            }
        }

        mode = CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E;

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdHyperGInternalPortModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdHyperGInternalPortModeSet(dev, port, mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        /* 1.4. For active device check that function returns NOT GT_OK */
        /* for CPU port number.                                         */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdHyperGInternalPortModeSet(dev, port, mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mode = CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E;
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdHyperGInternalPortModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdHyperGInternalPortModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdHyperGInternalPortModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh1 DxCh2)
    1.1.1. Call with not NULL modePtr.
    Expected: GT_OK for XG ports and GT_NOT_SUPPORTED for others.
    1.1.2. Call with NULL modePtr.
    Expected: GT_BAD_PTR.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: NOT GT_OK.
*/
    GT_STATUS                            st   = GT_OK;

    GT_U8                                dev;
    GT_PHYSICAL_PORT_NUM                 port = CSCD_VALID_PHY_PORT_CNS;
    CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT mode = CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E;
    PRV_CPSS_PORT_TYPE_ENT               portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E  | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /*  1.1.1.  */
            st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &mode);
            if (PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, mode);
            }

            /*  1.1.2.  */
            st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        CPSS_TBD_BOOKMARK_EARCH /* remove casting */
        port = (GT_U8)UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        /* 1.4. For active device check that function returns NOT GT_OK */
        /* for CPU port number.                                         */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &mode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdHyperGInternalPortModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdHyperGPortCrcModeSet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT  crcMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdHyperGPortCrcModeSet)
{
/*
    ITERATE_DEVICES_PHY_ PORTS (DxCh2 and above)
    1.1.1. Call with portDirection [CPSS_PORT_DIRECTION_RX_E /
                                    CPSS_PORT_DIRECTION_TX_E /
                                    CPSS_PORT_DIRECTION_BOTH_E]
                     and crcMode [CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E /
                                  CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdHyperGPortCrcModeGet.
    Expected: GT_OK and same crcModeP.
    1.1.3. Call with wrong enum values portDirection
                     and crcMode [CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with portDirection [CPSS_PORT_DIRECTION_RX_E]
                     and wrong enum values crcMode .
    Expected: GT_BAD_PARAM.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                        st        = GT_OK;
    GT_STATUS                        rc        = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port;
    CPSS_PORT_DIRECTION_ENT          direction = CPSS_PORT_DIRECTION_RX_E;
    CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT mode      = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;
    CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT modeGet   = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;

    GT_PHYSICAL_PORT_NUM             utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/*  port for 'UT iterations' */
    CPSS_PP_FAMILY_TYPE_ENT            devFamily;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    GT_BOOL                            isSupported;
    GT_BOOL                            valid = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    /* MAC is not simulated in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH3_E);

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC(GM_CSCD_GM_FAIL_STR);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily;

        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;

            /* get type of port */
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with portDirection [CPSS_PORT_DIRECTION_RX_E /
                                                CPSS_PORT_DIRECTION_TX_E /
                                                CPSS_PORT_DIRECTION_BOTH_E]
                                 and crcMode [CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E /
                                              CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E].
                Expected: GT_OK.
            */

            /* Call with portDirection [CPSS_PORT_DIRECTION_RX_E] */
            /* crcMode [CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E]       */
            direction = CPSS_PORT_DIRECTION_RX_E;
            mode      = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, mode);
                continue;
            }

            st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);

            if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                    valid = portMapShadowPtr->valid;
                }
                else
                {
                    valid = GT_TRUE;
                }

                if (valid == GT_TRUE)
                {
                    switch(portMacType)
                    {
                        case PRV_CPSS_PORT_XG_E:
                        case PRV_CPSS_PORT_XLG_E:
                            rc = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                                        PRV_CPSS_PORT_XLG_E,
                                                                        /*OUT*/&isSupported);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                            if (isSupported == GT_TRUE)
                            {
                                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                                /*  1.1.2  */
                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                                /* validation values */
                                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);
                            }
                            break;

                        case PRV_CPSS_PORT_FE_E:
                        case PRV_CPSS_PORT_GE_E:
                            rc = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                                        PRV_CPSS_PORT_GE_E,
                                                                        /*OUT*/&isSupported);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                            if (isSupported == GT_TRUE)
                            {
                                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                                /*  1.1.2  */
                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                                /* validation values */
                                UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E, modeGet,"get another mode than was set: %d", dev);
                            }
                            break;


                        case PRV_CPSS_PORT_CG_E:
                            rc = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                                        PRV_CPSS_PORT_CG_E,
                                                                        /*OUT*/&isSupported);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                            if(portMacType == PRV_CPSS_PORT_CG_E)
                            {
                                isSupported = GT_FALSE;
                            }

                            if (isSupported == GT_TRUE)
                            {
                                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                                /*  1.1.3  */
                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                                /* validation values */
                                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction);
                            }
                            break;
                        default:
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED, dev, port, direction);
                    }
                }
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                /*  1.1.2  */
                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                /* validation values */
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);

            }


            /* Call with portDirection [CPSS_PORT_DIRECTION_TX_E] */
            /* crcMode [CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E]       */
            direction = CPSS_PORT_DIRECTION_TX_E;
            mode      = CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E;

            st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);

            if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {

                if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                    valid = portMapShadowPtr->valid;
                }

                if (valid == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                    /*  1.1.2  */
                    st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                    /* validation values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);
                }
                else /* port is not mapped , error shall be got */
                {
                    UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                /*  1.1.2  */
                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                /* validation values */
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);
            }


            /* Call with portDirection [CPSS_PORT_DIRECTION_BOTH_E] */
            /* crcMode [CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E]       */
            direction = CPSS_PORT_DIRECTION_BOTH_E;
            mode      = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;

            st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);
            if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                    valid = portMapShadowPtr->valid;
                }
                else
                {
                    valid = GT_TRUE;
                }

                if (valid == GT_TRUE)
                {
                    switch(portMacType)
                    {
                        case PRV_CPSS_PORT_XG_E:
                        case PRV_CPSS_PORT_XLG_E:

                        rc = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                                    PRV_CPSS_PORT_XLG_E,
                                                                    /*OUT*/&isSupported);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                        if (isSupported == GT_TRUE)
                        {
                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                            /*  1.1.2   can't get direction Both for XLG mac */
                            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, CPSS_PORT_DIRECTION_TX_E, &modeGet);
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, CPSS_PORT_DIRECTION_TX_E);
                            /* validation values */
                            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);

                            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, CPSS_PORT_DIRECTION_RX_E, &modeGet);
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, CPSS_PORT_DIRECTION_RX_E);
                            /* validation values */
                            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);
                        }
                        break;

                        case PRV_CPSS_PORT_FE_E:
                        case PRV_CPSS_PORT_GE_E:

                            rc = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                                        PRV_CPSS_PORT_GE_E,
                                                                        /*OUT*/&isSupported);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                            if (isSupported == GT_TRUE)
                            {
                                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                                /*  1.1.2   can*/
                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, CPSS_PORT_DIRECTION_TX_E, &modeGet);
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, CPSS_PORT_DIRECTION_TX_E);
                                /* validation values */
                                UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E, modeGet,"get another mode than was set: %d", dev);

                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, CPSS_PORT_DIRECTION_RX_E, &modeGet);
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, CPSS_PORT_DIRECTION_RX_E);
                                /* validation values */
                                UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E, modeGet,"get another mode than was set: %d", dev);
                            }
                            break;

                        case PRV_CPSS_PORT_CG_E:

                            rc = prvCpssDxChBobcat2PortMacIsSupported(dev, port,
                                                                        PRV_CPSS_PORT_CG_E,
                                                                        /*OUT*/&isSupported);
                            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                            if(portMacType == PRV_CPSS_PORT_CG_E)
                            {
                                isSupported = GT_FALSE;
                            }

                            if (isSupported == GT_TRUE)
                            {
                                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
                                /*  1.1.2   can't get direction Both for XLG mac */
                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &modeGet);
                                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, CPSS_PORT_DIRECTION_TX_E, &modeGet);
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, CPSS_PORT_DIRECTION_TX_E);
                                /* validation values */
                                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);

                                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, CPSS_PORT_DIRECTION_RX_E, &modeGet);
                                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, CPSS_PORT_DIRECTION_RX_E);
                                /* validation values */
                                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,"get another mode than was set: %d", dev);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, mode);
                            }
                            break;
                        default:
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED, dev, port, direction);
                    }
                }
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, mode);
            }



            /*
                1.1.2. Call with wrong enum values portDirection
                                 and crcMode [CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdHyperGPortCrcModeSet
                                (dev, port, direction, mode),
                                direction);

            /*
                1.1.3. Call with portDirection [CPSS_PORT_DIRECTION_RX_E]
                     and wrong enum values crcMode.
                Expected: GT_BAD_PARAM.
            */
            direction = CPSS_PORT_DIRECTION_RX_E;

            UTF_ENUMS_CHECK_MAC(cpssDxChCscdHyperGPortCrcModeSet
                                (dev, port, direction, mode),
                                mode);
        }

        direction = CPSS_PORT_DIRECTION_RX_E;
        mode      = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;

        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    direction = CPSS_PORT_DIRECTION_RX_E;
    mode      = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;
    port      = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdHyperGPortCrcModeSet(dev, port, direction, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdHyperGPortCrcModeGet)
{
/*
    ITERATE_DEVICES_PHY_ PORTS (DxCh2 and above)
    1.1.1. Call with portDirection [CPSS_PORT_DIRECTION_RX_E /
                                    CPSS_PORT_DIRECTION_TX_E]
                     and not NULL crcModePtr.
    Expected: GT_OK.
    1.1.2. Call with portDirection [CPSS_PORT_DIRECTION_BOTH_E]
                     and not NULL crcModePtr.
    Expected: NOT GT_OK.
    1.1.3. Call with wrong enum values portDirection.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with NULL crcModePtr.
    Expected: GT_BAD_PTR.
    1.2. Call with port [CPSS_CPU_PORT_NUM_CNS = 63] and valid other parameters.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                        st        = GT_OK;
    GT_STATUS                        rc        = GT_OK;
    GT_U8                            dev;
    GT_PHYSICAL_PORT_NUM             port;
    CPSS_PORT_DIRECTION_ENT          direction = CPSS_PORT_DIRECTION_RX_E;
    CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT mode      = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;
    CPSS_PP_FAMILY_TYPE_ENT            devFamily;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    GT_BOOL                          valid = GT_TRUE;
    GT_PHYSICAL_PORT_NUM             utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/*  port for 'UT iterations' */

    /* MAC is not simulated in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_DXCH_PP_MAC(dev)->genInfo.devFamily;

        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;
            /*  1.1.1.  */
            direction = CPSS_PORT_DIRECTION_RX_E;

            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction);
                continue;
            }

            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
            if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {

                if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                    valid = portMapShadowPtr->valid;
                }

                if (valid == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);
                }
                else /* port is not mapped , error shall be got */
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);
            }



            /*  1.1.1.  */
            direction = CPSS_PORT_DIRECTION_TX_E;
            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
            if ((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                (devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {

                if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                    valid = portMapShadowPtr->valid;
                }

                if (valid == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);
                }
                else /* port is not mapped , error shall be got */
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);
            }

            /*  1.1.2.  */
            direction = CPSS_PORT_DIRECTION_BOTH_E;

            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

            /*  1.1.3.  */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdHyperGPortCrcModeGet
                                (dev, port, direction, &mode),
                                direction);

            /*  1.1.4.  */
            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, direction);
        }

        direction = CPSS_PORT_DIRECTION_RX_E;

        st = prvUtfNextMacPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    direction = CPSS_PORT_DIRECTION_RX_E;
    port      = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdHyperGPortCrcModeGet(dev, port, direction, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverFastStackRecoveryEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverFastStackRecoveryEnableSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChCscdFastFailoverFastStackRecoveryEnableGet with non-null enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChCscdFastFailoverFastStackRecoveryEnableGet with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdFastFailoverFastStackRecoveryEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChCscdFastFailoverFastStackRecoveryEnableGet with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdFastFailoverFastStackRecoveryEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdFastFailoverFastStackRecoveryEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverFastStackRecoveryEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverFastStackRecoveryEnableGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverSecondaryTargetPortMapSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_PHYSICAL_PORT_NUM secondaryTargetPort
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverSecondaryTargetPortMapSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Loop on all phy ports for secondaryTargetPort.
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdFastFailoverSecondaryTargetPortMapGet with non-null secondaryTargetPortPtr.
    Expected: GT_OK and the same secondaryTargetPort.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port;

    GT_PHYSICAL_PORT_NUM   secondaryTargetPortTmp;
    GT_PHYSICAL_PORT_NUM   secondaryTargetPort      = 0;
    GT_PHYSICAL_PORT_NUM   secondaryTargetPortGet   = 0;

    GT_PHYSICAL_PORT_NUM   utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/*  port for 'UT iterations' */
    GT_PHYSICAL_PORT_NUM   utSecondaryTargetPortTmp = 0;/*  port for 'UT iterations' */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;
            st = prvUtfNextPhyPortReset(&utSecondaryTargetPortTmp, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /*1.1.1. Loop on all phy ports for secondaryTargetPort.*/
            while (GT_OK == prvUtfNextPhyPortGet(&utSecondaryTargetPortTmp, GT_TRUE))
            {
                secondaryTargetPortTmp = utSecondaryTargetPortTmp;
                /*
                    1.1.1.1. Call with secondaryTargetPort .
                    Expected: if PRV_CPSS_DXCH_LION2_FAST_FAILOVER_WA_E is not set :GT_OK.
                              if PRV_CPSS_DXCH_LION2_FAST_FAILOVER_WA_E is set and secondaryTargetPortTmp
                              and port is in the same hemisphere: GT_OK.
                              else: GT_BAD PARAM.
                */
                secondaryTargetPort = secondaryTargetPortTmp;
                st = cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(dev, port, secondaryTargetPort);

                if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_LION2_FAST_FAILOVER_WA_E))
                {
                    if (PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(dev,secondaryTargetPortTmp) ==
                        PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(dev,port))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, secondaryTargetPort);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, secondaryTargetPort);
                        continue;
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, secondaryTargetPort);
                }

                /*
                    1.1.1.2. Call cpssDxChCscdFastFailoverSecondaryTargetPortMapGet with non-null secondaryTargetPortPtr.
                    Expected: GT_OK and the same secondaryTargetPort.
                */
                st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, &secondaryTargetPortGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChCscdFastFailoverSecondaryTargetPortMapGet: %d, %d", dev, port);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(secondaryTargetPort, secondaryTargetPortGet,
                           "get another secondaryTargetPort than was set: %d", dev, port);

                if (GT_TRUE == prvUtfReduceLogSizeFlagGet())
                {
                    break;
                }
            }

            if (GT_TRUE == prvUtfReduceLogSizeFlagGet())
            {
                break;
            }


        }

        secondaryTargetPort = 0;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;

            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(dev, port, secondaryTargetPort);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(dev, port, secondaryTargetPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(dev, port, secondaryTargetPort);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* cpu port like other ports */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    secondaryTargetPort = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(dev, port, secondaryTargetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(dev, port, secondaryTargetPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverSecondaryTargetPortMapGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_PHYSICAL_PORT_NUM    *secondaryTargetPortPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverSecondaryTargetPortMapGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with non-NULL secondaryTargetPortPtr.
    Expected: GT_OK.
    1.1.2. Call with secondaryTargetPortPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port;

    GT_PHYSICAL_PORT_NUM   secondaryTargetPort = 0;

    GT_PHYSICAL_PORT_NUM   utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/*  port for 'UT iterations' */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;
            /*
                1.1.1. Call with non-NULL secondaryTargetPortPtr.
                Expected: GT_OK.
            */
            /* remove casting after updating UT to support earch */
            CPSS_TBD_BOOKMARK_EARCH
            st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, &secondaryTargetPort);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with secondaryTargetPortPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            /* remove casting after updating UT to support earch */
            CPSS_TBD_BOOKMARK_EARCH
            st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, secondaryTargetPortPtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, &secondaryTargetPort);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, &secondaryTargetPort);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, &secondaryTargetPort);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* cpu port like other ports */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, &secondaryTargetPort);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(dev, port, &secondaryTargetPort);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet with non-null enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverPortIsLoopedSet
(
  IN  GT_U8                 devNum,
  IN  GT_PHYSICAL_PORT_NUM  portNum,
  IN GT_BOOL                isLooped
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverPortIsLoopedSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call with isLooped [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdFastFailoverPortIsLoopedGet with non-null isLoopedPtr.
    Expected: GT_OK and the same isLooped.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;

    GT_BOOL   isLooped    = GT_FALSE;
    GT_BOOL   isLoopedGet = GT_FALSE;

    GT_PHYSICAL_PORT_NUM   utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/* port for 'UT iterations' */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;
            /*
                1.1.1. Call with isLooped [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call with isLooped [GT_FALSE] */
            isLooped = GT_FALSE;

            st = cpssDxChCscdFastFailoverPortIsLoopedSet(dev, port, isLooped);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, isLooped);

            /*
                1.1.2. Call cpssDxChCscdFastFailoverPortIsLoopedGet with non-null isLoopedPtr.
                Expected: GT_OK and the same isLooped.
            */
            st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLoopedGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdFastFailoverPortIsLoopedGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(isLooped, isLoopedGet,
                       "get another isLooped than was set: %d", dev, port);

            /* Call with isLooped [GT_TRUE] */
            isLooped = GT_TRUE;

            st = cpssDxChCscdFastFailoverPortIsLoopedSet(dev, port, isLooped);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, isLooped);

            /*
                1.1.2. Call cpssDxChCscdFastFailoverPortIsLoopedGet with non-null isLoopedPtr.
                Expected: GT_OK and the same isLooped.
            */
            st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLoopedGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdFastFailoverPortIsLoopedGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(isLooped, isLoopedGet,
                       "get another isLooped than was set: %d", dev, port);
        }

        isLooped = GT_FALSE;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdFastFailoverPortIsLoopedSet(dev, port, isLooped);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdFastFailoverPortIsLoopedSet(dev, port, isLooped);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdFastFailoverPortIsLoopedSet(dev, port, isLooped);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    isLooped = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverPortIsLoopedSet(dev, port, isLooped);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdFastFailoverPortIsLoopedSet(dev, port, isLooped);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdFastFailoverPortIsLoopedGet
(
  IN GT_U8                  devNum,
  IN GT_PHYSICAL_PORT_NUM   portNum,
  OUT GT_BOOL    *isLoopedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdFastFailoverPortIsLoopedGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call with non-NULL isLoopedPtr.
    Expected: GT_OK.
    1.1.2. Call with isLoopedPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;

    GT_BOOL     isLooped = GT_FALSE;

    GT_PHYSICAL_PORT_NUM    utPhysicalPort = CSCD_VALID_PHY_PORT_CNS;/* port for 'UT iterations' */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            port = utPhysicalPort;
            /*
                1.1.1. Call with non-NULL isLoopedPtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLooped);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with isLoopedPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, isLoopedPtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            port = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLooped);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLooped);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLooped);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLooped);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdFastFailoverPortIsLoopedGet(dev, port, &isLooped);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdPortBridgeBypassEnableSet
(
  IN  GT_U8         devNum,
  IN  GT_PORT_NUM   portNum,
  IN  GT_BOOL       enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortBridgeBypassEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdPortBridgeBypassEnableGet with non-null enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    GT_BOOL   enable    = GT_FALSE;
    GT_BOOL   enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call with enable [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChCscdPortBridgeBypassEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChCscdPortBridgeBypassEnableGet with non-null enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortBridgeBypassEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev, port);

            /* Call with enable [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChCscdPortBridgeBypassEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChCscdPortBridgeBypassEnableGet with non-null enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortBridgeBypassEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortBridgeBypassEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChCscdPortBridgeBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortBridgeBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortBridgeBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortBridgeBypassEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdPortBridgeBypassEnableGet
(
  IN GT_U8      devNum,
  IN GT_PORT_NUM      portNum,
  OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortBridgeBypassEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxChx)
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    GT_BOOL   enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortBridgeBypassEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdQosPortTcRemapEnableSet
(
  IN  GT_U8      devNum,
  IN  GT_U8      portNum,
  IN GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdQosPortTcRemapEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdQosPortTcRemapEnableGet with non-null enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    GT_BOOL   enable    = GT_FALSE;
    GT_BOOL   enableGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call with enable [GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChCscdQosPortTcRemapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChCscdQosPortTcRemapEnableGet with non-null enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdQosPortTcRemapEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev, port);

            /* Call with enable [GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChCscdQosPortTcRemapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChCscdQosPortTcRemapEnableGet with non-null enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdQosPortTcRemapEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdQosPortTcRemapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdQosPortTcRemapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdQosPortTcRemapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdQosPortTcRemapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdQosPortTcRemapEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdQosPortTcRemapEnableGet
(
  IN GT_U8      devNum,
  IN GT_U8      portNum,
  OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdQosPortTcRemapEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh3 and above)
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    GT_BOOL   enable = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdQosPortTcRemapEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdQosTcRemapTableSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              tc,
    IN  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC    *tcMappingsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdQosTcRemapTableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1. Call with tc [0 / 7] and tcMappingsPtr{forwardLocalTc [0 / 7],
                                                forwardStackTc [1 / 6],
                                                toAnalyzerLocalTc [2 / 5],
                                                toAnalyzerStackTc [3 / 4],
                                                toCpuLocalTc [4 / 3],
                                                toCpuStackTc [5 / 2],
                                                fromCpuLocalTc [6 / 1],
                                                fromCpuStackTc [7 / 0]}.
    Expected: GT_OK.
    1.2. Call cpssDxChCscdQosTcRemapTableGet with non-null tcMappingsPtr.
    Expected: GT_OK and the same tcMappings.
    1.3. Call  with out of range tc [ CSCD_INVALID_TC_CNS ].
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range tcMappingsPtr->forwardLocalTc [8]
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range tcMappingsPtr->forwardStackTc [8]
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with out of range tcMappingsPtr->toAnalyzerLocalTc [8]
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with out of range tcMappingsPtr->toAnalyzerStackTc [8]
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.8. Call with out of range tcMappingsPtr->toCpuLocalTc [8]
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.9. Call with out of range tcMappingsPtr->toCpuStackTc [8]
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.10. Call with out of range tcMappingsPtr->fromCpuLocalTc [8]
                    and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.11. Call with out of range tcMappingsPtr->fromCpuStackTc [8]
                    and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.12. Call with null tcMappings [NULL]
                    and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.13 Call  with out of range tcMappings structure members
           [CSCD_INVALID_TC_CNS].
    Expected: GT_BAD_PARAM for all members.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;

    GT_U32                            tc = 0;
    CPSS_DXCH_CSCD_QOS_TC_REMAP_STC   tcMappings;
    CPSS_DXCH_CSCD_QOS_TC_REMAP_STC   tcMappingsGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);


    cpssOsBzero((GT_VOID*)&tcMappings, sizeof(tcMappings));
    cpssOsBzero((GT_VOID*)&tcMappingsGet, sizeof(tcMappingsGet));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
            1.1. Call with tc [0 / 7] and tcMappingsPtr{forwardLocalTc [0 / 7],
                                                        forwardStackTc [1 / 6],
                                                        toAnalyzerLocalTc [2 / 5],
                                                        toAnalyzerStackTc [3 / 4],
                                                        toCpuLocalTc [4 / 3],
                                                        toCpuStackTc [5 / 2],
                                                        fromCpuLocalTc [6 / 1],
                                                        fromCpuStackTc [7 / 0]}.
            Expected: GT_OK.
            */

            /* Call with tcMappings [0] */
            tc = 0;

            tcMappings.forwardLocalTc    = 0;
            tcMappings.forwardStackTc    = 1;
            tcMappings.toAnalyzerLocalTc = 2;
            tcMappings.toAnalyzerStackTc = 3;
            tcMappings.toCpuLocalTc      = 4;
            tcMappings.toCpuStackTc      = 5;
            tcMappings.fromCpuLocalTc    = 6;
            tcMappings.fromCpuStackTc    = 7;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tc);

            /*
                1.2. Call cpssDxChCscdQosTcRemapTableGet with the same tc and non-null tcMappingsPtr.
                Expected: GT_OK and the same tcMappings.
            */
            st = cpssDxChCscdQosTcRemapTableGet(dev, tc, &tcMappingsGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdQosTcRemapTableGet: %d, %d", dev, tc);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.forwardLocalTc, tcMappingsGet.forwardLocalTc,
                       "get another tcMappingsPtr->forwardLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.forwardStackTc, tcMappingsGet.forwardStackTc,
                       "get another tcMappingsPtr->forwardStackTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toAnalyzerLocalTc, tcMappingsGet.toAnalyzerLocalTc,
                       "get another tcMappingsPtr->toAnalyzerLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toAnalyzerStackTc, tcMappingsGet.toAnalyzerStackTc,
                       "get another tcMappingsPtr->toAnalyzerStackTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toCpuLocalTc, tcMappingsGet.toCpuLocalTc,
                       "get another tcMappingsPtr->toCpuLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toCpuStackTc, tcMappingsGet.toCpuStackTc,
                       "get another tcMappingsPtr->toCpuStackTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.fromCpuLocalTc, tcMappingsGet.fromCpuLocalTc,
                       "get another tcMappingsPtr->fromCpuLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.fromCpuStackTc, tcMappingsGet.fromCpuStackTc,
                       "get another tcMappingsPtr->fromCpuStackTc than was set: %d", dev);

            /* Call with tc [7] */
            tc = 7;

            tcMappings.forwardLocalTc    = 7;
            tcMappings.forwardStackTc    = 6;
            tcMappings.toAnalyzerLocalTc = 5;
            tcMappings.toAnalyzerStackTc = 4;
            tcMappings.toCpuLocalTc      = 3;
            tcMappings.toCpuStackTc      = 2;
            tcMappings.fromCpuLocalTc    = 1;
            tcMappings.fromCpuStackTc    = 0;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tc);

            /*
                1.2. Call cpssDxChCscdQosTcRemapTableGet with the same tc and non-null tcMappingsPtr.
                Expected: GT_OK and the same tcMappingsPtr.
            */
            st = cpssDxChCscdQosTcRemapTableGet(dev, tc, &tcMappingsGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdQosTcRemapTableGet: %d, %d", dev, tc);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.forwardLocalTc, tcMappingsGet.forwardLocalTc,
                       "get another tcMappingsPtr->forwardLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.forwardStackTc, tcMappingsGet.forwardStackTc,
                       "get another tcMappingsPtr->forwardStackTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toAnalyzerLocalTc, tcMappingsGet.toAnalyzerLocalTc,
                       "get another tcMappingsPtr->toAnalyzerLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toAnalyzerStackTc, tcMappingsGet.toAnalyzerStackTc,
                       "get another tcMappingsPtr->toAnalyzerStackTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toCpuLocalTc, tcMappingsGet.toCpuLocalTc,
                       "get another tcMappingsPtr->toCpuLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.toCpuStackTc, tcMappingsGet.toCpuStackTc,
                       "get another tcMappingsPtr->toCpuStackTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.fromCpuLocalTc, tcMappingsGet.fromCpuLocalTc,
                       "get another tcMappingsPtr->fromCpuLocalTc than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(tcMappings.fromCpuStackTc, tcMappingsGet.fromCpuStackTc,
                       "get another tcMappingsPtr->fromCpuStackTc than was set: %d", dev);

            /*
                1.3. Call  with out of range tc [ CSCD_INVALID_TC_CNS ].
                Expected: GT_BAD_PARAM.
            */
            tc = CSCD_INVALID_TC_CNS;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, tc);

            tc = 0;

            /*
                1.4. Call with out of range tcMappingsPtr->forwardLocalTc [8]
                           and other parameters from 1.1.
                Expected: NOT GT_OK.
            */
            tcMappings.forwardLocalTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->forwardLocalTc = %d",
                                             dev, tcMappings.forwardLocalTc);

            tcMappings.forwardLocalTc = 0;

            /*
                1.5. Call with out of range tcMappingsPtr->forwardStackTc [8]
                               and other parameters from 1.1.
                Expected: NOT GT_OK.
            */

            tcMappings.forwardStackTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->forwardStackTc = %d",
                                             dev, tcMappings.forwardStackTc);

            tcMappings.forwardStackTc = 0;

            /*
                1.6. Call with out of range tcMappingsPtr->toAnalyzerLocalTc [8]
                               and other parameters from 1.1.
                Expected: NOT GT_OK.
            */

            tcMappings.toAnalyzerLocalTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->toAnalyzerLocalTc = %d",
                                             dev, tcMappings.toAnalyzerLocalTc);

            tcMappings.toAnalyzerLocalTc = 0;

            /*
                1.7. Call with out of range tcMappingsPtr->toAnalyzerStackTc [8]
                               and other parameters from 1.1.
                Expected: NOT GT_OK.
            */

            tcMappings.toAnalyzerStackTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->toAnalyzerStackTc = %d",
                                             dev, tcMappings.toAnalyzerStackTc);

            tcMappings.toAnalyzerStackTc = 0;

            /*
                1.8. Call with out of range tcMappingsPtr->toCpuLocalTc [8]
                               and other parameters from 1.1.
                Expected: NOT GT_OK.
            */

            tcMappings.toCpuLocalTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->toCpuLocalTc = %d",
                                             dev, tcMappings.toCpuLocalTc);

            tcMappings.toCpuLocalTc = 0;

            /*
                1.9. Call with out of range tcMappingsPtr->toCpuStackTc [8]
                               and other parameters from 1.1.
                Expected: NOT GT_OK.
            */

            tcMappings.toCpuStackTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->toCpuStackTc = %d",
                                             dev, tcMappings.toCpuStackTc);

            tcMappings.toCpuStackTc = 0;

            /*
                1.10. Call with out of range tcMappingsPtr->fromCpuLocalTc [8]
                                and other parameters from 1.1.
                Expected: NOT GT_OK.
            */

            tcMappings.fromCpuLocalTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->fromCpuLocalTc = %d",
                                             dev, tcMappings.fromCpuLocalTc);

            tcMappings.fromCpuLocalTc = 0;

            /*
                1.11. Call with out of range tcMappingsPtr->fromCpuStackTc [8]
                                and other parameters from 1.1.
                Expected: NOT GT_OK.
            */

            tcMappings.fromCpuStackTc = 8;

            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tcMappingsPtr->fromCpuStackTc = %d",
                                             dev, tcMappings.fromCpuStackTc);

            tcMappings.fromCpuStackTc = 0;

            /*
                1.12. Call with null tcMappings [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdQosTcRemapTableSet(dev, tc, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, tc);


            tc = 0;

            tcMappings.forwardLocalTc    = 0;
            tcMappings.forwardStackTc    = 1;
            tcMappings.toAnalyzerLocalTc = 2;
            tcMappings.toAnalyzerStackTc = 3;
            tcMappings.toCpuLocalTc      = 4;
            tcMappings.toCpuStackTc      = 5;
            tcMappings.fromCpuLocalTc    = 6;
            tcMappings.fromCpuStackTc    = 7;
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdQosTcRemapTableSet(dev, tc, &tcMappings);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdQosTcRemapTableGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             tc,
    OUT  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC   *tcMappingsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdQosTcRemapTableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with non-NULL tcMappingsPtr.
    Expected: GT_OK.
    1.1.2. Call with wrong tc [CSCD_INVALID_TC_CNS].
    Expected: GT_BAD_PARAM.
    1.1.3. Call with tcMappingsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st   = GT_OK;
    GT_U8                           dev;
    GT_PHYSICAL_PORT_NUM            port = CSCD_VALID_PHY_PORT_CNS;

    GT_U32                          tc = 0;
    CPSS_DXCH_CSCD_QOS_TC_REMAP_STC tcMappings;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL tcMappingsPtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdQosTcRemapTableGet(dev, tc, &tcMappings);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with wrong tc [CSCD_INVALID_TC_CNS].
                Expected: GT_BAD_PARAM.
            */
            tc = CSCD_INVALID_TC_CNS;

            st = cpssDxChCscdQosTcRemapTableGet(dev, tc, &tcMappings);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            tc = 0;

            /*
                1.1.2. Call with tcMappingsPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdQosTcRemapTableGet(dev, tc, NULL);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                    "%d, %d, tcMappingsPtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdQosTcRemapTableGet(dev, tc, &tcMappings);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdQosTcRemapTableGet(dev, tc, &tcMappings);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdOrigSrcPortFilterEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdOrigSrcPortFilterEnableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChCscdOrigSrcPortFilterEnableGet with non-null enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChCscdOrigSrcPortFilterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChCscdOrigSrcPortFilterEnableGet
                 with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdOrigSrcPortFilterEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdOrigSrcPortFilterEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChCscdOrigSrcPortFilterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChCscdOrigSrcPortFilterEnableGet with
                 non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdOrigSrcPortFilterEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdOrigSrcPortFilterEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdOrigSrcPortFilterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdOrigSrcPortFilterEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdOrigSrcPortFilterEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdOrigSrcPortFilterEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChCscdOrigSrcPortFilterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdOrigSrcPortFilterEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdOrigSrcPortFilterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdOrigSrcPortFilterEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDevMapLookupModeSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDevMapLookupModeSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with mode [CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E /
                         CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E /
                         CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E /
                         CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E /
                         CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E]
    Expected: GT_OK.
    1.2. Call cpssDxChCscdDevMapLookupModeGet with non-NULL modePtr
    Expected: GT_OK and same value as written.
    1.3. Call with out of range mode [wrong enum values]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode    = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   modeGet = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mode [CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E]
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChCscdDevMapLookupModeGet with non-NULL modePtr
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdDevMapLookupModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdDevMapLookupModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "got another mode then was set: %d", dev);

        /*
            1.1. Call with mode [CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E]
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChCscdDevMapLookupModeGet with non-NULL modePtr
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdDevMapLookupModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdDevMapLookupModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "got another mode then was set: %d", dev);

        /*
            1.1. Call with mode [CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E]
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChCscdDevMapLookupModeGet with non-NULL modePtr
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdDevMapLookupModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdDevMapLookupModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "got another mode then was set: %d", dev);

        /*
            1.1. Call with mode [CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E]
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChCscdDevMapLookupModeGet with non-NULL modePtr
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdDevMapLookupModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdDevMapLookupModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "got another mode then was set: %d", dev);

        /*
            1.1. Call with mode [CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E /
                                 CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E]
            Expected: GT_OK.
        */
        mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E;

        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChCscdDevMapLookupModeGet with non-NULL modePtr
            Expected: GT_OK and same value as written.
        */
        st = cpssDxChCscdDevMapLookupModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdDevMapLookupModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "got another mode then was set: %d", dev);

        /*
            1.3. Call with out of range mode [wrong enum values]
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdDevMapLookupModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdDevMapLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdDevMapLookupModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDevMapLookupModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDevMapLookupModeGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with non-NULL modePtr
    Expected: GT_OK.
    1.2. Call with NULL modePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL modePtr
            Expected: GT_OK.
        */
        st = cpssDxChCscdDevMapLookupModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with NULL modePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdDevMapLookupModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdDevMapLookupModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdDevMapLookupModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortLocalDevMapLookupEnableSet
(
    IN GT_U8                devNum,
    IN GT_U8                portNum,
    IN CPSS_DIRECTION_ENT   portDirection,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortLocalDevMapLookupEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1.1. Call with portDirection [CPSS_DIRECTION_INGRESS_E /
                                    CPSS_DIRECTION_EGRESS_E /
                                    CPSS_DIRECTION_BOTH_E].
                     enable [GT_FALSE / GT_TRUE]
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdPortLocalDevMapLookupEnableGet with non-NULL pointers,
                                                           other params same as in 1.1.1.
    Expected: GT_OK and the same values.
    1.1.3. Call with out of range portDirection [wrong enum values],
                     other params same as in 1.1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    CPSS_DIRECTION_ENT   direction = CPSS_DIRECTION_INGRESS_E;
    GT_BOOL              enable    = GT_FALSE;
    GT_BOOL              enableGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with portDirection [CPSS_DIRECTION_INGRESS_E /
                                                CPSS_DIRECTION_EGRESS_E /
                                                CPSS_DIRECTION_BOTH_E].
                                 enable [GT_FALSE / GT_TRUE]
                Expected: GT_OK.
            */
            direction = CPSS_DIRECTION_INGRESS_E;
            enable    = GT_FALSE;

            st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, enable);

            /*
                1.1.2. Call cpssDxChCscdPortLocalDevMapLookupEnableGet with non-NULL pointers,
                                                                       other params same as in 1.1.1.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);

            /*
                1.1.1. Call with portDirection [CPSS_DIRECTION_INGRESS_E /
                                                CPSS_DIRECTION_EGRESS_E /
                                                CPSS_DIRECTION_BOTH_E].
                                 enable [GT_FALSE / GT_TRUE]
                Expected: GT_OK.
            */
            direction = CPSS_DIRECTION_EGRESS_E;
            enable    = GT_TRUE;

            st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, enable);

            /*
                1.1.2. Call cpssDxChCscdPortLocalDevMapLookupEnableGet with non-NULL pointers,
                                                                       other params same as in 1.1.1.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);

            /*
                1.1.1. Call with portDirection [CPSS_DIRECTION_INGRESS_E /
                                                CPSS_DIRECTION_EGRESS_E /
                                                CPSS_DIRECTION_BOTH_E].
                                 enable [GT_FALSE / GT_TRUE]
                Expected: GT_OK.
            */
            direction = CPSS_DIRECTION_BOTH_E;

            st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, enable);

            /*
                1.1.2. Call cpssDxChCscdPortLocalDevMapLookupEnableGet with non-NULL pointers,
                                                                       other params same as in 1.1.1.
                Expected: GT_BAD_PARAM, use Egress or Ingress to get the same values.
            */
            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                       "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

            direction = CPSS_DIRECTION_EGRESS_E;
            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);

            direction = CPSS_DIRECTION_INGRESS_E;
            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);

            /*
                1.1.3. Call with out of range portDirection [wrong enum values],
                                 other params same as in 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdPortLocalDevMapLookupEnableSet
                                (dev, port, direction, enable),
                                direction);
        }

        direction = CPSS_DIRECTION_INGRESS_E;
        enable    = GT_FALSE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;
        direction = CPSS_DIRECTION_INGRESS_E;
        enable    = GT_TRUE;

        st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        direction = CPSS_DIRECTION_INGRESS_E;
        enable    = GT_FALSE;

        st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        direction = CPSS_DIRECTION_EGRESS_E;
        enable    = GT_TRUE;

        st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        direction = CPSS_DIRECTION_EGRESS_E;
        enable    = GT_FALSE;

        st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdPortLocalDevMapLookupEnableGet: %d",  dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

    }

    direction = CPSS_DIRECTION_INGRESS_E;
    enable    = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortLocalDevMapLookupEnableSet(dev, port, direction, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortLocalDevMapLookupEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_U8               portNum,
    IN CPSS_DIRECTION_ENT   portDirection,
    OUT GT_BOOL             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortLocalDevMapLookupEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion and above)
    1.1.1. Call with portDirection [CPSS_DIRECTION_INGRESS_E /
                                    CPSS_DIRECTION_EGRESS_E /
                                    CPSS_DIRECTION_BOTH_E].
                     non NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call with out of range portDirection [wrong enum values],
                     other params same as in 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with enablePtr [NULL],
                     other params same as in 1.1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    CPSS_DIRECTION_ENT   direction = CPSS_DIRECTION_INGRESS_E;
    GT_BOOL              enable    = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with portDirection [CPSS_DIRECTION_INGRESS_E /
                                                CPSS_DIRECTION_EGRESS_E /
                                                CPSS_DIRECTION_BOTH_E].
                                 non NULL enablePtr
                Expected: GT_OK.
            */

            /*call with direction = CPSS_DIRECTION_INGRESS_E */
            direction = CPSS_DIRECTION_INGRESS_E;

            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

            /*call with direction = CPSS_DIRECTION_EGRESS_E */
            direction = CPSS_DIRECTION_EGRESS_E;

            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

            /*call with direction = CPSS_DIRECTION_BOTH_E */
            direction = CPSS_DIRECTION_BOTH_E;

            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction);

            /*
                1.1.2. Call with out of range portDirection [wrong enum values],
                                 other params same as in 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdPortLocalDevMapLookupEnableGet
                                (dev, port, direction, &enable),
                                direction);

            /*
                1.1.3. Call with enablePtr [NULL],
                                 other params same as in 1.1.1.
                Expected: GT_BAD_PTR.
            */
            direction = CPSS_DIRECTION_EGRESS_E;
            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        direction = CPSS_DIRECTION_INGRESS_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortLocalDevMapLookupEnableGet(dev, port, direction, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisModeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisModeEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_BOOL                                 enable = GT_FALSE;
    GT_BOOL                                 enableGet;

    GT_PHYSICAL_PORT_NUM                   utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum,
                                                             enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisModeEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                             &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisModeEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);

            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum,
                                                             enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisModeEnableGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                             &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisModeEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum,
                                                             enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisModeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;

    GT_U8                       devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_BOOL                     enable;

    GT_PHYSICAL_PORT_NUM       utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call function with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                             &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &enable);

            /*
                1.1.2. Call function with NULL enablePtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, enablePtr = NULL", devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                             &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum) ;

        st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                         &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                         &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum,
                                                         &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisReservedDevNumSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_HW_DEV_NUM                       reservedHwDevNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisReservedDevNumSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with reservedHwDevNum [0 / BIT_10 / max].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisReservedDevNumGet.
    Expected: GT_OK and the same enable.
    1.1.3. Call with out of range of reservedHwDevNum.
    Expected: NOT GT_OK.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_HW_DEV_NUM           reservedHwDevNum;
    GT_HW_DEV_NUM           reservedDevNumGet;

    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                Call with reservedHwDevNum [0 / BIT_10 / max].
                Expected: GT_OK.
            */
            reservedHwDevNum = 0;

            st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                                 reservedHwDevNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, reservedHwDevNum);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisReservedDevNumGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                                 &reservedDevNumGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisReservedDevNumGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(reservedHwDevNum, reservedDevNumGet,
                "get another reservedHwDevNum than was set: %d, %d", devNum, portNum);

            /* Call with reservedHwDevNum [BIT_2]. */
            reservedHwDevNum = BIT_10;

            st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                                 reservedHwDevNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, reservedHwDevNum);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisReservedDevNumGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                                 &reservedDevNumGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisReservedDevNumGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(reservedHwDevNum, reservedDevNumGet,
                "get another reservedHwDevNum than was set: %d, %d", devNum, portNum);

            /* Call with reservedHwDevNum [max]. */
            reservedHwDevNum = BIT_12 - 1;

            st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                                 reservedHwDevNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, reservedHwDevNum);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisReservedDevNumGet.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                                 &reservedDevNumGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisReservedDevNumGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(reservedHwDevNum, reservedDevNumGet,
                "get another reservedHwDevNum than was set: %d, %d", devNum, portNum);

            /*
                1.1.3. Call with out of range of reservedHwDevNum.
                Expected: NOT GT_OK.
            */
            reservedHwDevNum = BIT_12;

            st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                                 reservedHwDevNum);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* Restore correct value. */
            reservedHwDevNum = BIT_10;
        }

        reservedHwDevNum = 0;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                                 reservedHwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                             reservedHwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                             reservedHwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;
    reservedHwDevNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                             reservedHwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum,
                                                         reservedHwDevNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisReservedDevNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_HW_DEV_NUM                       *reservedHwDevNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisReservedDevNumGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call function with not NULL reservedHwDevNumPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL reservedHwDevNumPtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                   st = GT_OK;

    GT_U8                       devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_HW_DEV_NUM               reservedHwDevNum;

    GT_PHYSICAL_PORT_NUM        utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call function with not NULL reservedHwDevNumPtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                                 &reservedHwDevNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &reservedHwDevNum);

            /*
                1.1.2. Call function with NULL reservedHwDevNumPtr.
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                                 NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, reservedHwDevNumPtr = NULL", devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                                 &reservedHwDevNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                             &reservedHwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                             &reservedHwDevNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                             &reservedHwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum,
                                                         &reservedHwDevNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisSrcIdSet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  GT_U32                       srcHwId
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisSrcIdSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with Call with srcHwId [0 / BIT_2 / max].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisSrcIdGet.
    Expected: GT_OK and get same value.
    1.1.3. Call function with out of range of srcHwId and others valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_HW_DEV_NUM           srcHwId;
    GT_HW_DEV_NUM           srcHwIdGet;

    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call with Call with srcHwId [0, BIT_2, max].
                Expected: GT_OK.
            */
            srcHwId = 0;

            st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, srcHwId);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisSrcIdGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum,
                                                        &srcHwIdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisSrcIdGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(srcHwId, srcHwIdGet,
                "get another srcHwId than was set: %d, %d", devNum, portNum);

            /* Call with srcHwId [BIT_2]. */
            srcHwId = BIT_10;

            st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, srcHwId);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisSrcIdGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum,
                                                        &srcHwIdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisSrcIdGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(srcHwId, srcHwIdGet,
                "get another srcHwId than was set: %d, %d", devNum, portNum);

            /* Call with srcHwId [max]. */
            srcHwId = BIT_12 - 1;

            st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, srcHwId);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisSrcIdGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum,
                                                        &srcHwIdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisSrcIdGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(srcHwId, srcHwIdGet,
                "get another srcHwId than was set: %d, %d", devNum, portNum);

            /*
                1.1.3. Call function with out of range of srcHwId and others valid params.
                Expected: NOT GT_OK.
            */
            srcHwId = BIT_12;

            st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* Restore correct value. */
            srcHwId = BIT_10;
        }

        srcHwId = 0;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    srcHwId = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcHwId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisSrcIdGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_U32                              *srcIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisSrcIdGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with not NULL srcIdPtr.
    Expected: GT_OK.
    1.1.2. Call with NULL srcIdPtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;

    GT_U8                                   devNum;
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_U32                                  srcId;

    GT_PHYSICAL_PORT_NUM                    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call with not NULL srcIdPtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, &srcId);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &srcId);

            /*
                1.1.2. Call with NULL srcIdPtr.
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, srcIdPtr = NULL", devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, &srcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, &srcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, &srcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, &srcId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, &srcId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDbRemoteHwDevNumModeSet
(
    IN GT_HW_DEV_NUM                    hwDevNum,
    IN CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT hwDevMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDbRemoteHwDevNumModeSet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh1_Diamond, DxCh2, DxCh3, xCat, Lion, xCat2, Lion2)
    1. Call function with hwDevNum [0 / BIT_6 / max] and
                         hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E /
                                    CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E] .
    Expected: GT_OK.
    2. Call cpssDxChCscdDbRemoteHwDevNumModeGet.
    Expected: GT_OK and get same value.
    3. Call function with out of range of hwDevNum and
                          hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E].
    Expected: NOT GT_OK.
    4. Call function with wrong odd hwDevNum [5] and
                          hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E].
    Expected: NOT GT_OK.
    5. Call function with wrong hwDevMode enum values and other valid params.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS                           st = GT_OK;

    GT_HW_DEV_NUM                       hwDevNum;
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT    hwDevMode;
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT    hwDevModeGet;
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT    hwDevModeForRestore;

    /* Save device mode for restore at test end */
    hwDevNum = 0;

    st = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevModeForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, %d", hwDevNum, hwDevModeForRestore);

    /*
        1. Call function with hwDevNum [0 / BIT_6 / max] and
                             hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E /
                                        CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E].
        Expected: GT_OK.
    */
    hwDevNum = 0;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /*
        2. Call cpssDxChCscdDbRemoteHwDevNumModeGet.
        Expected: GT_OK and get same value.
    */
    st = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, %d", hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(hwDevMode, hwDevModeGet,
        "get another hwDevNum than was set: %d, %d", hwDevNum, hwDevMode);

    /* Call with hwDevNum [0] and                                       */
    /*          hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E].        */
    hwDevNum = 0;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /*
        2. Call cpssDxChCscdDbRemoteHwDevNumModeGet.
        Expected: GT_OK and get same value.
    */
    st = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, %d", hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(hwDevMode, hwDevModeGet,
        "get another hwDevNum than was set: %d, %d", hwDevNum, hwDevMode);

    /* Call with hwDevNum [BIT_6] and                                   */
    /*          hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E].      */
    hwDevNum = BIT_6;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /*
        2. Call cpssDxChCscdDbRemoteHwDevNumModeGet.
        Expected: GT_OK and get same value.
    */
    st = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, %d", hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(hwDevMode, hwDevModeGet,
        "get another hwDevNum than was set: %d, %d", hwDevNum, hwDevMode);

    /* Call with hwDevNum [BIT_6] and                                   */
    /*          hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E].        */
    hwDevNum = BIT_6;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /*
        2. Call cpssDxChCscdDbRemoteHwDevNumModeGet.
        Expected: GT_OK and get same value.
    */
    st = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, %d", hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(hwDevMode, hwDevModeGet,
        "get another hwDevNum than was set: %d, %d", hwDevNum, hwDevMode);

    /* restore ... don't impact other tests ! */
    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);

    /* Call with hwDevNum [max] and                                     */
    /*          hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E].      */
    hwDevNum = BIT_11;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /*
        2. Call cpssDxChCscdDbRemoteHwDevNumModeGet.
        Expected: GT_OK and get same value.
    */
    st = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, %d", hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(hwDevMode, hwDevModeGet,
        "get another hwDevNum than was set: %d, %d", hwDevNum, hwDevMode);

    /* restore ... don't impact other tests ! */
    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);

    /* Call with hwDevNum [max] and                                     */
    /*          hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E].      */
    hwDevNum = BIT_12 - 1;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /*
        2. Call cpssDxChCscdDbRemoteHwDevNumModeGet.
        Expected: GT_OK and get same value.
    */
    st = cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, &hwDevModeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, %d", hwDevNum, hwDevMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(hwDevMode, hwDevModeGet,
        "get another hwDevNum than was set: %d, %d", hwDevNum, hwDevMode);

    /*
        3. Call function with out of range of hwDevNum and
                                hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E].
        Expected: NOT GT_OK.
    */
    hwDevNum = BIT_12;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /* Restore correct value. */
    hwDevNum = BIT_6;

    /*
        4. Call function with wrong odd hwDevNum [5] and
                                       hwDevMode [CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E].
        Expected: NOT GT_OK.
    */
    hwDevNum = 5;
    hwDevMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);
    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, hwDevNum, hwDevMode);

    /* Restore correct value. */
    hwDevNum = BIT_6;

    /*
        5. Call function with wrong hwDevMode enum values and other valid params
        Expected: GT_OUT_OF_RANGE.
    */
    hwDevNum = BIT_3;

    UTF_ENUMS_CHECK_MAC(cpssDxChCscdDbRemoteHwDevNumModeSet
                        (hwDevNum, hwDevMode),
                        hwDevMode);

    /* Restore device mode */
    hwDevNum = 0;

    st = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevModeForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "cpssDxChCscdDbRemoteHwDevNumModeSet: %d, %d", hwDevNum, hwDevModeForRestore);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdDbRemoteHwDevNumModeGet
(
    IN  GT_HW_DEV_NUM                       hwDevNum,
    OUT CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT    *hwDevModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdDbRemoteHwDevNumModeGet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh1_Diamond, DxCh2, DxCh3, xCat, Lion, xCat2, Lion2)
    1.1. Call with devNum [0 / BIT_6 / max] and not NULL hwDevModePtr.
    Expected: GT_OK.
    1.2. Call with out of range of devNum and others valid params.
    Expected: NOT GT_OK.
    1.3. Call with NULL hwDevModePtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_HW_DEV_NUM                       devNum;
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT    hwDevMode;

    /*
        1.1. Call with devNum [0 / BIT_6 / max] and not NULL hwDevModePtr.
        Expected: GT_OK.
    */
    devNum = 0;

    st = cpssDxChCscdDbRemoteHwDevNumModeGet(devNum, &hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &hwDevMode);

    /* Call with devNum [BIT_6] and not NULL hwDevModePtr. */
    devNum = BIT_6;

    st = cpssDxChCscdDbRemoteHwDevNumModeGet(devNum, &hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &hwDevMode);

    /* Call with devNum [max] and not NULL hwDevModePtr. */
    devNum = BIT_12 - 1;

    st = cpssDxChCscdDbRemoteHwDevNumModeGet(devNum, &hwDevMode);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &hwDevMode);

    /*
        1.2. Call with out of range of devNum and not NULL hwDevModePtr.
        Expected: NOT GT_OK.
    */
    devNum = BIT_12;

    st = cpssDxChCscdDbRemoteHwDevNumModeGet(devNum, &hwDevMode);
    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &hwDevMode);

    /* Restore correct values. */
    devNum = BIT_6;

    /*
        1.3. Call function with NULL hwDevModePtr.
        Expected: GT_BAD_PARAM.
    */
    devNum = 0;

    st = cpssDxChCscdDbRemoteHwDevNumModeGet(devNum, NULL);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                 "cpssDxChCscdDbRemoteHwDevNumModeGet: %d, hwDevModePtr = NULL", devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         portBaseEport,
    IN  GT_PORT_NUM                         trunkBaseEport
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with relevant portBaseEport [0/0x0F00/0x1FFF]
                              trunkBaseEport [0/0x0F00/0x1FFF].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet.
    Expected: GT_OK and get same value.
    1.1.3. Call function with out of range of portBaseEport[0x2000]
    and others valid params.
    Expected: NOT GT_OK.
    1.1.4. Call function with out of range of trunkBaseEport[0x2000]
    and others valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_PORT_NUM             portBaseEport;
    GT_PORT_NUM             portBaseEportGet;
    GT_PORT_NUM             trunkBaseEport;
    GT_PORT_NUM             trunkBaseEportGet;

    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
                with relevant value.
                Expected: GT_OK.
            */
            portBaseEport = 0;
            trunkBaseEport = 0;

            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                           portNum, portBaseEport, trunkBaseEport);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                    portNum, &portBaseEportGet, &trunkBaseEportGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(portBaseEport, portBaseEportGet,
                "get another portBaseEport than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(trunkBaseEport, trunkBaseEportGet,
                "get another trunkBaseEport than was set: %d, %d", devNum, portNum);

            /*
                1.1.1. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
                with relevant value.
                Expected: GT_OK.
            */
            portBaseEport = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum) / 2;
            trunkBaseEport = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum) / 2;

            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                           portNum, portBaseEport, trunkBaseEport);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                    portNum, &portBaseEportGet, &trunkBaseEportGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(portBaseEport, portBaseEportGet,
                "get another portBaseEport than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(trunkBaseEport, trunkBaseEportGet,
                "get another trunkBaseEport than was set: %d, %d", devNum, portNum);

            /*
                1.1.1. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
                with relevant value.
                Expected: GT_OK.
            */
            portBaseEport = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum)-1;
            trunkBaseEport = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum)-1;

            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                           portNum, portBaseEport, trunkBaseEport);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet.
                Expected: GT_OK and get same value.
            */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                    portNum, &portBaseEportGet, &trunkBaseEportGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(portBaseEport, portBaseEportGet,
                "get another portBaseEport than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(trunkBaseEport, trunkBaseEportGet,
                "get another trunkBaseEport than was set: %d, %d", devNum, portNum);

            /*
                1.1.3. Call function with out of range of portBaseEport[0x2000]
                and others valid params.
                Expected: NOT GT_OK.
            */
            portBaseEport = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);
            trunkBaseEport = 0;

            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                           portNum, portBaseEport, trunkBaseEport);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);


            /*
                1.1.4. Call function with out of range of trunkBaseEport[0x2000]
                and others valid params.
                Expected: NOT GT_OK.
            */
            portBaseEport = 0;
            trunkBaseEport = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum) ;

            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                           portNum, portBaseEport, trunkBaseEport);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }

        portBaseEport = 0;
        trunkBaseEport = 0;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                           portNum, portBaseEport, trunkBaseEport);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                       portNum, portBaseEport, trunkBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                       portNum, portBaseEport, trunkBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    portBaseEport = 0;
    trunkBaseEport = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                       portNum, portBaseEport, trunkBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,
                                   portNum, portBaseEport, trunkBaseEport);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *portBaseEportPtr,
    OUT GT_PORT_NUM                         *trunkBaseEportPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with not NULL portBaseEport, trunkBaseEport.
    Expected: GT_OK.
    1.1.2. Call with NULL portBaseEport.
    Expected: GT_BAD_PTR.
    1.1.3. Call with NULL trunkBaseEport.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_PORT_NUM             portBaseEport = 0;
    GT_PORT_NUM             trunkBaseEport = 0;

    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet
                with relevant value.
                Expected: GT_OK.
            */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                    portNum, &portBaseEport, &trunkBaseEport);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with NULL portBaseEport.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                    portNum, NULL, &trunkBaseEport);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);

            /*
                1.1.3. Call with NULL trunkBaseEport.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                    portNum, &portBaseEport, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        portBaseEport = 0;
        trunkBaseEport = 0;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                    portNum, &portBaseEport, &trunkBaseEport);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                portNum, &portBaseEport, &trunkBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                portNum, &portBaseEport, &trunkBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                                portNum, &portBaseEport, &trunkBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,
                            portNum, &portBaseEport, &trunkBaseEport);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              srcTrunkLsbAmount,
    IN  GT_U32                              srcPortLsbAmount,
    IN  GT_U32                              srcDevLsbAmount
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
                with relevant value srcTrunkLsbAmount[0/3/7]
                                    srcPortLsbAmount[6/2/0]
                                    srcDevLsbAmount[5/0/3].
    Expected: GT_OK.
    1.2. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet.
    Expected: GT_OK and the same value.
    1.3. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
         with out of range srcTrunkLsbAmount [8].
    Expected: NOT GT_OK.
    1.4. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
         with out of range srcPortLsbAmount [7].
    Expected: NOT GT_OK.
    1.5. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
         with out of range srcPortLsbAmount [6].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      srcTrunkLsbAmount = 0;
    GT_U32      srcPortLsbAmount = 0;
    GT_U32      srcDevLsbAmount = 0;
    GT_U32      srcTrunkLsbAmountGet = 0;
    GT_U32      srcPortLsbAmountGet = 0;
    GT_U32      srcDevLsbAmountGet = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
                 with relevant value.
            Expected: GT_OK.
        */
        srcTrunkLsbAmount = 0;
        srcPortLsbAmount = 6;
        srcDevLsbAmount = 5;

        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                          srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                          &srcTrunkLsbAmountGet, &srcPortLsbAmountGet, &srcDevLsbAmountGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(srcTrunkLsbAmount, srcTrunkLsbAmountGet,
                   "get another srcTrunkLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcPortLsbAmount, srcPortLsbAmountGet,
                   "get another srcPortLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcDevLsbAmount, srcDevLsbAmountGet,
                   "get another srcDevLsbAmount than was set: %d", dev);

        /* Call with srcTrunkLsbAmount [3], srcPortLsbAmount [2], srcDevLsbAmount[0] */
        srcTrunkLsbAmount = 3;
        srcPortLsbAmount = 2;
        srcDevLsbAmount = 0;

        /*
            1.1. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
                 with relevant value.
            Expected: GT_OK.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                          srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                          &srcTrunkLsbAmountGet, &srcPortLsbAmountGet, &srcDevLsbAmountGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(srcTrunkLsbAmount, srcTrunkLsbAmountGet,
                   "get another srcTrunkLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcPortLsbAmount, srcPortLsbAmountGet,
                   "get another srcPortLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcDevLsbAmount, srcDevLsbAmountGet,
                   "get another srcDevLsbAmount than was set: %d", dev);

        /* Call with srcTrunkLsbAmount [7], srcPortLsbAmount [0], srcDevLsbAmount[3] */
        srcTrunkLsbAmount = 7;
        srcPortLsbAmount = 0;
        srcDevLsbAmount = 3;

        /*
            1.1. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
                 with relevant value.
            Expected: GT_OK.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                          srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet with non-null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                          &srcTrunkLsbAmountGet, &srcPortLsbAmountGet, &srcDevLsbAmountGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(srcTrunkLsbAmount, srcTrunkLsbAmountGet,
                   "get another srcTrunkLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcPortLsbAmount, srcPortLsbAmountGet,
                   "get another srcPortLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcDevLsbAmount, srcDevLsbAmountGet,
                   "get another srcDevLsbAmount than was set: %d", dev);

        /*
            1.3. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
                 with srcTrunkLsbAmount[8] out of range.
            Expected: NOT GT_OK.
        */
        srcTrunkLsbAmount = 8;
        srcPortLsbAmount = 0;
        srcDevLsbAmount = 0;

        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                          srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        srcTrunkLsbAmount = 0;

        /*
            1.4. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
                 with srcPortLsbAmount[7] out of range.
            Expected: NOT GT_OK.
        */
        srcPortLsbAmount = 7;

        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                          srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        srcPortLsbAmount = 0;

        /*
            1.5. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
                 with srcDevLsbAmount[6] out of range.
            Expected: NOT GT_OK.
        */
        srcDevLsbAmount = 6;

        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                          srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        srcDevLsbAmount = 0;
    }
    srcTrunkLsbAmount = 0;
    srcPortLsbAmount = 0;
    srcDevLsbAmount = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                          srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(dev,
                      srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *srcTrunkLsbAmountPtr,
    OUT GT_U32                              *srcPortLsbAmountPtr,
    OUT GT_U32                              *srcDevLsbAmountPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
                with not NULL srcTrunkLsbAmountPtr,
                              srcPortLsbAmountPtr,
                              srcDevLsbAmountPtr.
    Expected: GT_OK.
    1.2. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
         with NULL srcTrunkLsbAmountPtr and other relevant value.
    Expected: GT_BAD_PTR.
    1.3. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
         with NULL srcPortLsbAmountPtr and other relevant value.
    Expected: GT_BAD_PTR.
    1.4. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
         with NULL srcDevLsbAmountPtr and other relevant value.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      srcTrunkLsbAmount = 0;
    GT_U32      srcPortLsbAmount  = 0;
    GT_U32      srcDevLsbAmount   = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
                        with not NULL srcTrunkLsbAmountPtr,
                                      srcPortLsbAmountPtr,
                                      srcDevLsbAmountPtr.
            Expected: GT_OK.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                          &srcTrunkLsbAmount, &srcPortLsbAmount, &srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
                 with NULL srcTrunkLsbAmountPtr and other relevant value.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                          NULL, &srcPortLsbAmount, &srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
                 with NULL srcPortLsbAmountPtr and other relevant value.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                      &srcTrunkLsbAmount, NULL, &srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
                 with NULL srcDevLsbAmountPtr and other relevant value.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                          &srcTrunkLsbAmount, &srcPortLsbAmount, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                          &srcTrunkLsbAmount, &srcPortLsbAmount, &srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(dev,
                      &srcTrunkLsbAmount, &srcPortLsbAmount, &srcDevLsbAmount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdMruCheckOnCascadePortEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortMruCheckOnCascadeEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with valid param.
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdPortMruCheckOnCascadeEnableGet.
    Expected: GT_OK and get enable same value.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable    = GT_FALSE;
    GT_BOOL                 enableGet = GT_FALSE;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call with valid param.
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call cpssDxChCscdPortMruCheckOnCascadeEnableGet.
                Expected: GT_OK and get enable same value.
            */
            st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdPortMruCheckOnCascadeEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);

            /*
                1.1.1. Call with valid param.
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call cpssDxChCscdPortMruCheckOnCascadeEnableGet.
                Expected: GT_OK and get enable same value.
            */
            st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdPortMruCheckOnCascadeEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortMruCheckOnCascadeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortMruCheckOnCascadeEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;

    GT_BOOL                 enable  = GT_FALSE;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*
                1.1.1. Call with valid param.
                Expected: GT_OK.
            */

            st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
                1.1.2. Call with NULL enablePtr.
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_FALSE))
        {
            portNum = utPhysicalPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);


}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet
                with relevant value srcPortLsbAmount[15/10/0]
                                    srcDevLsbAmount[10/0/15].
    Expected: GT_OK.
    1.2. Call cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet.
    Expected: GT_OK and the same values.
    1.3. Call with out of range srcPortLsbAmount [16].
    Expected: NOT GT_OK.
    1.4. Call with out of range srcPortLsbAmount [16].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      srcPortLsbAmount = 0;
    GT_U32      srcDevLsbAmount = 0;
    GT_U32      srcPortLsbAmountGet = 0;
    GT_U32      srcDevLsbAmountGet = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        srcPortLsbAmount = 15;
        srcDevLsbAmount = 10;

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(dev,
                          srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                                     &srcPortLsbAmountGet, &srcDevLsbAmountGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(srcPortLsbAmount, srcPortLsbAmountGet,
                   "get another srcPortLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcDevLsbAmount, srcDevLsbAmountGet,
                   "get another srcDevLsbAmount than was set: %d", dev);


        /*  1.1.  */
        srcPortLsbAmount = 10;
        srcDevLsbAmount = 0;

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(dev,
                          srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                                     &srcPortLsbAmountGet, &srcDevLsbAmountGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(srcPortLsbAmount, srcPortLsbAmountGet,
                   "get another srcPortLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcDevLsbAmount, srcDevLsbAmountGet,
                   "get another srcDevLsbAmount than was set: %d", dev);

        /*  1.1.  */
        srcPortLsbAmount = 0;
        srcDevLsbAmount = 15;

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(dev,
                          srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                                     &srcPortLsbAmountGet, &srcDevLsbAmountGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(srcPortLsbAmount, srcPortLsbAmountGet,
                   "get another srcPortLsbAmount than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(srcDevLsbAmount, srcDevLsbAmountGet,
                   "get another srcDevLsbAmount than was set: %d", dev);


        /*  1.3. */
        srcPortLsbAmount = 16;

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(dev,
                                           srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        srcPortLsbAmount = 0;

        /*  1.4.  */
        srcDevLsbAmount = 16;

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(dev,
                          srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        srcDevLsbAmount = 0;
    }

    srcPortLsbAmount = 0;
    srcDevLsbAmount = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(dev,
                          srcPortLsbAmount, srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(dev,
                      srcPortLsbAmount, srcDevLsbAmount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet
                with not NULL srcPortLsbAmountPtr,
                              srcDevLsbAmountPtr.
    Expected: GT_OK.
    1.2. Call with NULL srcPortLsbAmountPtr and other relevant value.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL srcDevLsbAmountPtr and other relevant value.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      srcPortLsbAmount  = 0;
    GT_U32      srcDevLsbAmount   = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                         &srcPortLsbAmount, &srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                      NULL, &srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                         &srcPortLsbAmount, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                         &srcPortLsbAmount, &srcDevLsbAmount);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(dev,
                     &srcPortLsbAmount, &srcDevLsbAmount);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid assignmentMode.
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet.
    Expected: GT_OK and same assignmentMode.
    1.1.3. Call with wrong enum value.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS               st = GT_OK;
    GT_STATUS               eVlanTagEtagStatus;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT  assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT  assignmentModeGet = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*  1.1.1.  */
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;

            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, assignmentMode);

            /*   1.1.2.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(assignmentMode, assignmentModeGet,
                "get another assignmentMode than was set: %d, %d", devNum, portNum);

            /*  1.1.1.  */
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E;

            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, assignmentMode);

            /*   1.1.2.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(assignmentMode, assignmentModeGet,
                "get another assignmentMode than was set: %d, %d", devNum, portNum);

            /*  1.1.1.  */
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E;

            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, assignmentMode);

            /*   1.1.2.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(assignmentMode, assignmentModeGet,
                "get another assignmentMode than was set: %d, %d", devNum, portNum);

            /*  1.1.1.  */
           assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E;

           eVlanTagEtagStatus = (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? GT_OK : GT_BAD_PARAM);

           st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
           UTF_VERIFY_EQUAL3_PARAM_MAC(eVlanTagEtagStatus, st, devNum, portNum, assignmentMode);

           if (eVlanTagEtagStatus == GT_OK)
           {
               /*   1.1.2.  */
               st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentModeGet);
               UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet: %d, %d", devNum, portNum);
               UTF_VERIFY_EQUAL2_STRING_MAC(assignmentMode, assignmentModeGet,
               "get another assignmentMode than was set: %d, %d", devNum, portNum);
           }
           /*  1.1.1.  */
           assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E;

           st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
           UTF_VERIFY_EQUAL3_PARAM_MAC(eVlanTagEtagStatus, st, devNum, portNum, assignmentMode);

           if (eVlanTagEtagStatus == GT_OK)
           {
               /*   1.1.2.  */
               st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentModeGet);
               UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet: %d, %d", devNum, portNum);
               UTF_VERIFY_EQUAL2_STRING_MAC(assignmentMode, assignmentModeGet,
                   "get another assignmentMode than was set: %d, %d", devNum, portNum);
           }

            /*  1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet
                                (devNum, portNum, assignmentMode),
                                assignmentMode);
        }

        assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL assignmentModePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL assignmentModePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;

    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT  assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*  1.1.1.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, assignmentMode);

            /*  1.1.2. */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, &assignmentMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid physicalPortBase[1].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet.
    Expected: GT_OK and same physicalPortBase.
    1.1.3. Call with out of range physicalPortBase
    Expected: NOT GT_OK
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  physicalPortBase = 0;
    GT_U32                  physicalPortBaseGet = 0;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;


            /*  1.1.1.  */
            physicalPortBase = 1;

            st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, physicalPortBase);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalPortBase);

            /*   1.1.2.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, &physicalPortBaseGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(physicalPortBase, physicalPortBaseGet,
                "get another physicalPortBase than was set: %d, %d", devNum, portNum);

            /*  1.1.3.  */
            physicalPortBase = 0xFFFFFFFF;

            st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, physicalPortBase);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalPortBase);

            physicalPortBase = 1;
        }

        physicalPortBase = 1;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, physicalPortBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    physicalPortBase = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, physicalPortBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, physicalPortBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL physicalPortBasePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL physicalPortBasePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;

    GT_U32                  physicalPortBase = 0;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*  1.1.1.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, &physicalPortBase);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, physicalPortBase);

            /*  1.1.2. */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, &physicalPortBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, &physicalPortBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, &physicalPortBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with enable[GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet.
    Expected: GT_OK and get enable same value.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable    = GT_FALSE;
    GT_BOOL                 enableGet = GT_FALSE;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*  1.1.1. */
            enable = GT_FALSE;

            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*  1.1.2.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);

            /*  1.1.1.  */
            enable = GT_TRUE;

            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*  1.1.2.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;

    GT_BOOL                 enable  = GT_FALSE;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextMacPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*   1.1.1.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*   1.1.2.  */
            st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with enable[GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet.
    Expected: GT_OK and get enable same value.
*/
    GT_STATUS               st;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable = GT_FALSE;
    GT_BOOL                 enableGet;
    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*  1.1.1. */
            enable = GT_FALSE;

            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*  1.1.2.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);

            /*  1.1.1.  */
            enable = GT_TRUE;

            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*  1.1.2.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable than was set: %d, %d", devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;
    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*   1.1.1.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*   1.1.2.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with remotePhysicalHwDevNum = 0/15, remotePhysicalPortNum = 0/26.
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet.
    Expected: GT_OK and get remotePhysicalHwDevNum, remotePhysicalPortNum same values.
*/
    GT_STATUS               st;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_HW_DEV_NUM           remotePhysicalHwDevNum = 0;
    GT_HW_DEV_NUM           remotePhysicalHwDevNumGet;
    GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum = 0;
    GT_PHYSICAL_PORT_NUM    remotePhysicalPortNumGet;
    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;
            /*  1.1.1. */
            remotePhysicalHwDevNum = 0;
            remotePhysicalPortNum = 0;

            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);

            /*  1.1.2.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, &remotePhysicalHwDevNumGet, &remotePhysicalPortNumGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(remotePhysicalHwDevNum, remotePhysicalHwDevNumGet,
                "get another remote device than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(remotePhysicalPortNum, remotePhysicalPortNumGet,
                "get another remote port than was set: %d, %d", devNum, portNum);

            /*  1.1.1.  */
            remotePhysicalHwDevNum = 15;
            remotePhysicalPortNum = 26;

            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);

            /*  1.1.2.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, &remotePhysicalHwDevNumGet, &remotePhysicalPortNumGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(remotePhysicalHwDevNum, remotePhysicalHwDevNumGet,
                "get another remote device than was set: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(remotePhysicalPortNum, remotePhysicalPortNumGet,
                "get another remote port than was set: %d, %d", devNum, portNum);
        }

        remotePhysicalHwDevNum = 0;
        remotePhysicalPortNum = 0;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL remotePhysicalHwDevNum, remotePhysicalPortNum.
    Expected: GT_OK.
    1.1.2. Call with NULL remotePhysicalHwDevNum, remotePhysicalPortNum.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_HW_DEV_NUM           remotePhysicalHwDevNum;
    GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum;
    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*   1.1.1.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, &remotePhysicalHwDevNum, &remotePhysicalPortNum);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);

            /*   1.1.2.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, NULL, &remotePhysicalPortNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);

            /*   1.1.3.  */
            st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, &remotePhysicalHwDevNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, &remotePhysicalHwDevNum, &remotePhysicalPortNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, &remotePhysicalHwDevNum, &remotePhysicalPortNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, &remotePhysicalHwDevNum, &remotePhysicalPortNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortStackAggregationConfigGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_HW_DEV_NUM        *aggHwDevNumPtr,
    OUT GT_PORT_NUM          *aggPortNumPtr,
    OUT GT_U32               *aggSrcIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortStackAggregationConfigGet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1.1. Call with valid aggHwDevNumPtr [non NULL],
                           aggPortNumPtr[non NULL],
                           aggSrcIdPtr  [non NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid value of aggHwDevNumPtr[NULL] and
           other parameters the same as 1.1.1
    Expected: GT_BAD_PTR.
    1.1.3. Call with invalid value of aggPortNumPtr[NULL] and
           other parameters the same as 1.1.1
    Expected: GT_BAD_PTR.
    1.1.4. Call with invalid value of aggSrcIdPtr[NULL] and
           other parameters the same as 1.1.1
    Expected: GT_BAD_PTR.

*/
    GT_STATUS            st         = GT_OK;
    GT_U8                devNum     = 0;
    GT_PHYSICAL_PORT_NUM portNum    = CSCD_VALID_PHY_PORT_CNS;

    GT_HW_DEV_NUM        aggHwDevNum  = 0;
    GT_PORT_NUM          aggPortNum = 0;
    GT_U32               aggSrcId   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
               1.1.1. Call with valid aggHwDevNumPtr [non NULL],
                                      aggPortNumPtr[non NULL],
                                      aggSrcIdPtr  [non NULL].
                Expected: GT_OK.
            */
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           &aggHwDevNum,
                                                           &aggPortNum,
                                                           &aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

           /*
               1.1.2. Call with invalid value of aggHwDevNumPtr[NULL] and
                      other parameters the same as 1.1.1
               Expected: GT_BAD_PTR.
            */
            aggPortNum = 0;
            aggSrcId   = 0;
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           NULL,
                                                           &aggPortNum,
                                                           &aggSrcId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, NULL", devNum, portNum);

            /*
                1.1.3. Call with invalid value of aggPortNumPtr[NULL] and
                       other parameters the same as 1.1.1
                Expected: GT_BAD_PTR.
            */
            aggHwDevNum  = 0;
            aggSrcId   = 0;
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           &aggHwDevNum,
                                                           NULL,
                                                           &aggSrcId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, NULL", devNum, portNum);

            /*
                1.1.4. Call with invalid value of aggSrcIdPtr[NULL] and
                       other parameters the same as 1.1.1
                Expected: GT_BAD_PTR.
            */
            aggHwDevNum  = 0;
            aggPortNum = 0;
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           &aggHwDevNum,
                                                           &aggPortNum,
                                                           NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, NULL", devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non active physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           &aggHwDevNum,
                                                           &aggPortNum,
                                                           &aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                       &aggHwDevNum,
                                                       &aggPortNum,
                                                       &aggSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /*
            1.4. For active device check that function returns GT_OK
            for CPU port number.
        */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                       &aggHwDevNum,
                                                       &aggPortNum,
                                                       &aggSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    portNum = CSCD_VALID_PHY_PORT_CNS;
    /*
        2. For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                       &aggHwDevNum,
                                                       &aggPortNum,
                                                       &aggSrcId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum, &aggHwDevNum,
                                                   &aggPortNum, &aggSrcId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortStackAggregationConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_HW_DEV_NUM        aggHwDevNum,
    IN  GT_PORT_NUM          aggPortNum,
    IN  GT_U32               aggSrcId
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortStackAggregationConfigSet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1.1. Call with aggHwDevNum     [0 / (BIT_5/2) / BIT_5 - 1],
                     retAggPortNum [0 / (BIT_6/2) / BIT_6 - 1],
                     aggSrcId      [0 / (BIT_5/2) / BIT_5 - 1]
    Expected: GT_OK.
    1.1.2 Call cpssDxChCscdPortStackAggregationConfigGet
    Expected: GT_OK and the same values that was set
    1.1.3. Call with out of range aggHwDevNum [BIT_5] and other parameters
           same as 1.1.1
    Expected: GT_OUT_OF_RANGE
    1.1.4. Call with out of range retAggPortNum [BIT_6] and other parameters
           same as 1.1.1
    Expected: GT_OUT_OF_RANGE
    1.1.5. Call with out of range aggSrcId [BIT_5] and other parameters
           same as 1.1.1
    Expected: GT_OUT_OF_RANGE

*/
    GT_STATUS            st            = GT_OK;
    GT_U8                devNum        = 0;
    GT_PHYSICAL_PORT_NUM portNum       = CSCD_VALID_PHY_PORT_CNS;

    GT_HW_DEV_NUM        aggHwDevNum     = 0;
    GT_PORT_NUM          aggPortNum    = 0;
    GT_U32               aggSrcId      = 0;

    GT_HW_DEV_NUM        retaggHwDevNum  = 0;
    GT_PORT_NUM          retAggPortNum = 0;
    GT_U32               retAggSrcId   = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /* 1.1.1. Call with aggHwDevNum   [0]
                                aggPortNum  [0]
                                aggSrcId    [0]
                Expected: GT_OK
            */
            aggHwDevNum  = 0;
            aggPortNum = 0;
            aggSrcId   = 0;
            st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                           aggHwDevNum,
                                                           aggPortNum,
                                                           aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                 1.1.2 Call cpssDxChCscdPortStackAggregationConfigGet
                 Expected: GT_OK and the same values that was set
            */
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           &retaggHwDevNum,
                                                           &retAggPortNum,
                                                           &retAggSrcId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChCscdPortStackAggregationConfigGet: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggHwDevNum, retaggHwDevNum,
                                         "get another aggHwDevNum value than was set: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggPortNum, retAggPortNum,
                                         "get another aggPortNum value than was set: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggSrcId, retAggSrcId,
                                         "get another aggSrcId value than was set: %d, %d",
                                         devNum, portNum);
            /*
                1.1.1. Call with aggHwDevNum     [BIT_5/2],
                                 retAggPortNum [BIT_6/2],
                                 aggSrcId      [BIT_5/2]
                Expected: GT_OK
            */
            aggHwDevNum  = BIT_5 / 2;
            aggPortNum = BIT_6 / 2;
            aggSrcId   = BIT_5 / 2;
            st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                           aggHwDevNum,
                                                           aggPortNum,
                                                           aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2 Call cpssDxChCscdPortStackAggregationConfigGet
                Expected: GT_OK and the same values that was set
            */
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           &retaggHwDevNum,
                                                           &retAggPortNum,
                                                           &retAggSrcId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChCscdPortStackAggregationConfigGet: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggHwDevNum, retaggHwDevNum,
                                         "get another aggHwDevNum value than was set: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggPortNum, retAggPortNum,
                                         "get another aggPortNum value than was set: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggSrcId, retAggSrcId,
                                         "get another aggSrcId value than was set: %d, %d",
                                         devNum, portNum);

            /*
                1.1.1. Call with aggHwDevNum     [BIT_5 - 1],
                                 retAggPortNum [BIT_6 - 1],
                                 aggSrcId      [BIT_5 - 1]
                Expected: GT_OK
            */
            aggHwDevNum =  BIT_5 - 1;
            aggPortNum = BIT_6 - 1;
            aggSrcId =   BIT_5 - 1;
            st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                           aggHwDevNum,
                                                           aggPortNum,
                                                           aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2 Call cpssDxChCscdPortStackAggregationConfigGet
                Expected: GT_OK and the same values that was set
            */
            st = cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum,
                                                           &retaggHwDevNum,
                                                           &retAggPortNum,
                                                           &retAggSrcId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChCscdPortStackAggregationConfigGet: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggHwDevNum, retaggHwDevNum,
                                         "get another aggHwDevNum value than was set: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggPortNum, retAggPortNum,
                                         "get another aggPortNum value than was set: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(aggSrcId, retAggSrcId,
                                         "get another aggSrcId value than was set: %d, %d",
                                         devNum, portNum);

            /*
                1.1.3. Call with out of range aggHwDevNum [BIT_5] and other
                       parameters same as 1.1.1
                Expected: GT_OUT_OF_RANGE
            */
            aggHwDevNum  = BIT_5;
            aggPortNum = 0;
            aggSrcId   = 0;
            st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                           aggHwDevNum,
                                                           aggPortNum,
                                                           aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum);

            /*
                1.1.4. Call with out of range retAggPortNum [BIT_6] and other
                       parameters same as 1.1.1
                Expected: GT_OUT_OF_RANGE
            */
            aggHwDevNum  = 0;
            aggPortNum = BIT_6;
            aggSrcId   = 0;
            st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                           aggHwDevNum,
                                                           aggPortNum,
                                                           aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum);

            /*
                1.1.5. Call with out of range aggSrcId [BIT_5] and other
                       parameters same as 1.1.1
                Expected: GT_OUT_OF_RANGE
            */
            aggHwDevNum  = 0;
            aggPortNum = 0;
            aggSrcId   = BIT_5;
            st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                           aggHwDevNum,
                                                           aggPortNum,
                                                           aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, portNum);
        }

        /* restore correct values */
        aggHwDevNum =  0;
        aggPortNum = 0;
        aggSrcId =   0;
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non active physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                           aggHwDevNum,
                                                           aggPortNum, aggSrcId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /*
           1.3. For active device check that function returns GT_BAD_PARAM
           for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                       aggHwDevNum, aggPortNum,
                                                       aggSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /*
            1.4. For active device check that function returns GT_OK
            for CPU port number.
        */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                       aggHwDevNum, aggPortNum,
                                                       aggSrcId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    portNum   = CSCD_VALID_PHY_PORT_CNS;
    /*
        2. For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                       aggHwDevNum, aggPortNum,
                                                       aggSrcId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum,
                                                   aggHwDevNum, aggPortNum,
                                                   aggSrcId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortStackAggregationEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortStackAggregationEnableSet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1.1 Call with valid enable [GT_TRUE/GT_FALSE]
    1.1.2 Call cpssDxChCscdPortStackAggregationEnableGet
    Expected: GT_OK and the same enable
*/
    GT_STATUS            st        = GT_OK;
    GT_U8                devNum    = 0;
    GT_PHYSICAL_PORT_NUM portNum      = CSCD_VALID_PHY_PORT_CNS;

    GT_BOOL              enable    = GT_FALSE;
    GT_BOOL              retEnable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
               1.1.1 Call with enable [GT_TRUE].
            */
            enable = GT_TRUE;
            st = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum,
                                                           enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
               1.1.2 Call cpssDxChCscdPortStackAggregationEnableGet
               Expected: GT_OK and the same enable
            */
            st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                           &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChCscdPortStackAggregationEnableGet: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                                         "get another enable value than was set: %d, %d",
                                         devNum, portNum);

            /*
               1.1.1. Call with enable [GT_FALSE]
            */
            enable = GT_FALSE;
            st = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum,
                                                           enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, enable);

            /*
               1.1.2. Call cpssDxChCscdPortStackAggregationEnableGet
               Expected: GT_OK and the same enable
            */
            st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                           &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChCscdPortStackAggregationEnableGet: %d, %d",
                                         devNum, portNum);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                                         "get another enable value than was set: %d, %d",
                                         devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non active physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        st = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /*
            1.4. For active device check that function returns GT_OK
            for CPU port number.
        */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    enable = GT_TRUE;
    portNum   = CSCD_VALID_PHY_PORT_CNS;

    /*
        2. For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortStackAggregationEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortStackAggregationEnableGet)
{
/*
    ITERATE_DEVICES (Lion2)

    1.1.1. Call with valid enablePtr [non-NULL]
    Expected: GT_OK.
    1.1.2. Call with invalid enablePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS              st        = GT_OK;
    GT_U8                  devNum    = 0;
    GT_PHYSICAL_PORT_NUM   portNum   = CSCD_VALID_PHY_PORT_CNS;
    GT_BOOL                enable    = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
               1.1.1. Call with valid enablePtr [non-NULL].
               Expected: GT_OK.
            */
            st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                           &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* 1.1.2. Call with invalid enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                           NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, NULL", devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non active physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                           &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /*
           1.3. For active device check that function returns GT_BAD_PARAM
                for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                       &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /*
           1.4. For active device check that function returns GT_OK
                for CPU port number.
        */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                       &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    portNum = CSCD_VALID_PHY_PORT_CNS;

    /*
       2. For not-active devices and devices from non-applicable family
          check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum,
                                                       &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortQosDsaModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid portQosDsaMode.
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdPortQosDsaModeSet.
    Expected: GT_OK and same portQosDsaMode.
    1.1.3. Call with wrong enum value.
    Expected: GT_BAD_PARAM
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E;
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaModeGet = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*  1.1.1.  */
            portQosDsaMode = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E;

            st = cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, portQosDsaMode);

            /*   1.1.2.  */
            st = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdPortQosDsaModeGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(portQosDsaMode, portQosDsaModeGet,
                "get another assignmentMode than was set: %d, %d", devNum, portNum);

            /*  1.1.1.  */
            portQosDsaMode = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E;
            st = cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, portQosDsaMode);

            /*   1.1.2.  */
            st = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChCscdPortQosDsaModeGet: %d, %d", devNum, portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(portQosDsaMode, portQosDsaModeGet,
                "get another portQosDsaMode than was set: %d, %d", devNum, portNum);

            /*  1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdPortQosDsaModeSet
                                (devNum, portNum, portQosDsaMode),
                                portQosDsaMode);
        }

        portQosDsaMode = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E;

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortQosDsaModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL portQosDsaModePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL portQosDsaModePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;

    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E;


    GT_PHYSICAL_PORT_NUM    utPhysicalPort;/* port for 'UT iterations' */

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&utPhysicalPort, GT_TRUE))
        {
            portNum = utPhysicalPort;

            /*  1.1.1.  */
            st = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, portQosDsaMode);

            /*  1.1.2. */
            st = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        st = prvUtfNextPhyPortReset(&utPhysicalPort, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdQosTcDpRemapTableSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    IN  GT_U32                                   newTc,
    IN  CPSS_DP_LEVEL_ENT                        newDp
);
*/
UTF_TEST_CASE_MAC(cpssDxChCscdQosTcDpRemapTableSet)
{

    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC   tcDpMappings;
    GT_U32                                     newTcGet;
    CPSS_DP_LEVEL_ENT                          newDpGet;
    GT_U32                                     notAppFamilyBmp;
    GT_U32                                     newTc = 3;
    CPSS_DP_LEVEL_ENT                          newDp = CPSS_DP_YELLOW_E;
    /* Bobcat3 relevant variables */
    GT_U32                                     newTcPfc = 3;
    GT_U32                                     newTcPfcGet;
    CPSS_DXCH_PORT_PROFILE_ENT                 srcProfileArr[] =
                                                    { CPSS_DXCH_PORT_PROFILE_NETWORK_E, CPSS_DXCH_PORT_PROFILE_CSCD_E,
                                                      CPSS_DXCH_PORT_PROFILE_FABRIC_E,CPSS_DXCH_PORT_PROFILE_LOOPBACK_E };

    CPSS_DXCH_PORT_PROFILE_ENT                 trgProfileArr[] =
                                                    { CPSS_DXCH_PORT_PROFILE_NETWORK_E, CPSS_DXCH_PORT_PROFILE_CSCD_E,
                                                      CPSS_DXCH_PORT_PROFILE_FABRIC_E,CPSS_DXCH_PORT_PROFILE_LOOPBACK_E };
    GT_U32 i,j;
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT       mCastPriority;
    GT_U32                                     queuesNum = 8; /* number of queues in TxQ */

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
        1.1. Call with all parameters in permitted ranges.
        Expected: GT_OK.
        */

        /* Call with tcDpMappings [0] */
        if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            queuesNum = 16;
        }
        else
        {
            queuesNum = 8;
        }

        newTc  = 3;
        newDp = CPSS_DP_YELLOW_E;
        tcDpMappings.dp              = 0;
        tcDpMappings.dsaTagCmd       = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
        tcDpMappings.isStack         = GT_TRUE;
        tcDpMappings.tc              = 5;
        /* Bobcat3 relevant fields */
        tcDpMappings.packetIsMultiDestination = GT_FALSE;
        tcDpMappings.targetPortTcProfile = CPSS_DXCH_PORT_PROFILE_NETWORK_E;

        st = cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call cpssDxChCscdQosTcDpRemapTableGet with the same tcDpMappings.
            Expected: GT_OK and the same newTc, newDp, newTcPfc.
        */

        st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTcGet, &newDpGet, &newTcPfcGet,&mCastPriority);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChCscdQosTcDpRemapTableGet");
        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(newTc,newTcGet,
                   "get another newTc than was set: %d, %d", newTc, newTcGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(newDp,newDpGet,
                   "get another newDp than was set: %d, %d", newDp, newDpGet);
        /*
            1.3. Call with out of range tcDpMappings.dsaTagCmd [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                            tcDpMappings.dsaTagCmd);
        /*
            1.4. Call with out of range tcDpMappings.dp [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                            tcDpMappings.dp);

        /*
            1.5. Call with out of range tcDpMappings.tc
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                            tcDpMappings.tc);

        /*
            1.6. Call with out of range newTc, newTcPfc
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                            newTc);

        /*
            1.7. Call with out of range newDp[wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                            newDp);

        /*
            1.8. Call with null tcDpMappings [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdQosTcDpRemapTableSet(dev, NULL,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, newTc, newDp);

        if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            /* Set initial values */
            newTc                           = 3;
            newTcPfc                        = 4;
            newDp                           = CPSS_DP_YELLOW_E;
            tcDpMappings.dp                 = 0;
            tcDpMappings.dsaTagCmd          = CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E;
            tcDpMappings.tc                 = 5;
            tcDpMappings.packetIsMultiDestination = GT_TRUE;

            for (i = 0; i < sizeof(srcProfileArr)/sizeof(srcProfileArr[0]); i++)
            {
                tcDpMappings.isStack = srcProfileArr[i];

                for (j = 0; j < sizeof(trgProfileArr)/sizeof(trgProfileArr[0]); j++)
                {
                    tcDpMappings.targetPortTcProfile = trgProfileArr[j];

                    st = cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings, newTc, newDp, newTcPfc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                    st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTcGet, &newDpGet, &newTcPfcGet,&mCastPriority);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChCscdQosTcDpRemapTableGet");
                    /* Verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(newTc, newTcGet,
                               "get another newTc than was set: %d, %d", newTc, newTcGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(newDp, newDpGet,
                               "get another newDp than was set: %d, %d", newDp, newDpGet);
                    /* Verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(newTcPfc, newTcPfcGet,
                               "get another newTcPfc than was set: %d, %d", newTc, newTcPfcGet);

                    /* Increment parameters values */
                    newTc = (newTc + 1) % queuesNum;
                    newTcPfc = (newTcPfc + 1) % 8;
                    newDp = (newDp + 1) % 3;
                    tcDpMappings.dp = (tcDpMappings.dp + 1) % 3;
                    tcDpMappings.dsaTagCmd = (tcDpMappings.dsaTagCmd + 1) % (CPSS_DXCH_NET_DSA_CMD_FORWARD_E + 1);
                    tcDpMappings.tc = (tcDpMappings.tc + 1) % 8;
                    tcDpMappings.packetIsMultiDestination = !tcDpMappings.packetIsMultiDestination;
                }
            }

            tcDpMappings.isStack                = CPSS_DXCH_PORT_PROFILE_NETWORK_E;
            tcDpMappings.targetPortTcProfile    = CPSS_DXCH_PORT_PROFILE_CSCD_E;

            /*
                 Call with out of range newTcPfc [out of range]
                 Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTcPfc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                                newTcPfc);
            /*
                 Call with out of range tcDpMappings.isStack [wrong enum values]
                 Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTcPfc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                                tcDpMappings.isStack);

            /*
                 Call with out of range tcDpMappings.targetPortTcProfile [wrong enum values]
                 Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTcPfc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E),
                                tcDpMappings.targetPortTcProfile);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdQosTcDpRemapTableSet(dev, &tcDpMappings,newTc,newDp,newTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdQosTcDpRemapTableGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    OUT  GT_U32                                  *remappedTcPtr,
    OUT  CPSS_DP_LEVEL_ENT                       *remappedDpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdQosTcDpRemapTableGet)
{
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC   tcDpMappings;
    GT_U32                                     newTc;
    CPSS_DP_LEVEL_ENT                          newDp;
    /* Bobcat3 related variables */
    GT_U32                                     newTcPfc;
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT       mCastPriority;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL tcDpMappings and other legal parameters
            Expected: GT_OK.
        */
        tcDpMappings.dp              = CPSS_DP_GREEN_E;
        tcDpMappings.dsaTagCmd       = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
        tcDpMappings.isStack         = GT_FALSE;
        tcDpMappings.tc              = 7;
        /* Bobcat3 related fields */
        tcDpMappings.packetIsMultiDestination = GT_TRUE;
        tcDpMappings.targetPortTcProfile = CPSS_DXCH_PORT_PROFILE_CSCD_E;

        st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, newTc, newDp, newTcPfc);

        /*
            1.2. Call with out of range tcDpMappings.dsaTagCmd [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority),
                            tcDpMappings.dsaTagCmd);
        /*
            1.3. Call with out of range tcDpMappings.dp [wrong enum values]
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority),
                            tcDpMappings.dp);

        /*
            1.4. Call with out of range tcDpMappings.tc
                and other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority),
                            tcDpMappings.tc);

        /*
            1.5. Call with null tcDpMappings [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdQosTcDpRemapTableGet(dev, NULL, &newTc, &newDp, &newTcPfc,&mCastPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, newTc, newDp);

        /*
            1.6. Call with null newTc,newTcPfc [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, NULL, &newDp, &newTcPfc,&mCastPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, newTc);
        /*
            1.6. Call with null newDp [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc,NULL, &newTcPfc,&mCastPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, newDp);

        if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
        {
            /*
                1.7. Call with null newTcPfc [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, NULL,&mCastPriority);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, newTcPfc);
            /*
                1.8. Call with out of range tcDpMappings.isStack[wrong enum values]
                and other valid params same as 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority),
                                tcDpMappings.isStack);

            /*
                1.9. Call with out of range tcDpMappings.targetPortTcProfile[wrong enum values]
                and other valid params same as 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority),
                                tcDpMappings.targetPortTcProfile);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdQosTcDpRemapTableGet(dev, &tcDpMappings, &newTc, &newDp, &newTcPfc,&mCastPriority);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdHashInDsaEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_PORT_DIRECTION_ENT direction,
    IN GT_BOOL                 enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdHashInDsaEnableSet)
{
/*
    1.   Iterate over all active devices:
    1.1. Iterate over active ports.
    1.1.1. Call cpssDxChCscdHashInDsaEnableSet with 'enable' in [GT_TRUE, GT_FALSE],
    'direction' in [RX, TX, BOTH] and check values with cpssDxChCscdHashInDsaEnableGet
    Expected: GT_OK.
    1.1.2. Call with wrong 'direction'
    Expected: GT_BAD_PARAM.
    1.2. Iterate over non available ports. Expected: GT_BAD_PARAM
    1.3. Call with port out of range.   Expected: GT_BAD_PARAM
    1.4. Call with CPU port. Expected: GT_OK
    2.   Call for not active devices.  Extected: GT_NOT_APPLICABLE_DEVICE
    3.   Call for device out of range. Extected: GT_NOT_APPLICABLE_DEVICE
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    phyPort;
    CPSS_PORT_DIRECTION_ENT directionArr[] = {CPSS_PORT_DIRECTION_RX_E,
                                              CPSS_PORT_DIRECTION_TX_E,
                                              CPSS_PORT_DIRECTION_BOTH_E};
    GT_BOOL                 enableArr[] = {GT_TRUE, GT_FALSE};
    CPSS_PORT_DIRECTION_ENT direction;
    GT_BOOL                 enable;
    GT_BOOL                 enableGet;
    GT_U32                  di;
    GT_U32                  ei;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE, GT_FALSE]
                Expected: GT_OK.
            */

            for (di = 0; di < sizeof(directionArr)/sizeof(directionArr[0]); di++)
            {
                direction = directionArr[di];
                for (ei = 0; ei < sizeof(enableArr)/sizeof(enableArr[0]); ei++)
                {
                    enable = enableArr[ei];
                    st = cpssDxChCscdHashInDsaEnableSet(dev, phyPort, direction, enable);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, phyPort, direction, enable);

                    if (direction == CPSS_PORT_DIRECTION_BOTH_E)
                    {
                        st = cpssDxChCscdHashInDsaEnableGet(
                            dev, phyPort, CPSS_PORT_DIRECTION_RX_E, &enableGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(
                            GT_OK, st, dev, phyPort, CPSS_PORT_DIRECTION_RX_E, enable);

                        UTF_VERIFY_EQUAL2_STRING_MAC(
                            enable, enableGet, "Get 'enable'different than set: %d, %d", dev, phyPort);

                        st = cpssDxChCscdHashInDsaEnableGet(
                            dev, phyPort, CPSS_PORT_DIRECTION_TX_E, &enableGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(
                            GT_OK, st, dev, phyPort, CPSS_PORT_DIRECTION_TX_E, enable);

                        UTF_VERIFY_EQUAL2_STRING_MAC(
                            enable, enableGet, "Get 'enable'different than set: %d, %d", dev, phyPort);
                    }
                    else
                    {
                        st = cpssDxChCscdHashInDsaEnableGet(dev, phyPort, direction, &enableGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, phyPort, direction, enable);

                        UTF_VERIFY_EQUAL2_STRING_MAC(
                            enable, enableGet, "Get 'enable'different than set: %d, %d", dev, phyPort);
                    }
                }
            }

            /*
                1.1.2. Call with wrong 'direction' values. Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdHashInDsaEnableSet
                                (dev, phyPort, direction, GT_TRUE),
                                direction);
        }

        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_FALSE))
        {
            st = cpssDxChCscdHashInDsaEnableSet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for phyPort number.                         */

        phyPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdHashInDsaEnableSet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU phyPort number.                                     */
        phyPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdHashInDsaEnableSet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);


    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdHashInDsaEnableSet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 1.3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdHashInDsaEnableSet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdHashInDsaEnableGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_PORT_DIRECTION_ENT direction,
    IN GT_BOOL                 enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdHashInDsaEnableGet)
{
/*
    1.   Iterate over all active devices:

    1.1. Iterate over active ports.
    1.1.1. Call with 'direction' [RX, TX]. Expected: GT_OK
    1.1.2. Call with 'direction' [BOTH].   Expected: GT_BAD_PARAM
    1.1.3. Call with wrong 'direction'.    Expected: GT_BAD_PARAM
    1.2. Iterate over non available ports. Expected: GT_BAD_PARAM
    1.3. Call with port out of range.      Expected: GT_BAD_PARAM
    1.4. Call with CPU port.               Expected: GT_OK
    2.   Call for not active devices.      Extected: GT_NOT_APPLICABLE_DEVICE
    3.   Call for device out of range.     Extected: GT_NOT_APPLICABLE_DEVICE
*/
    GT_STATUS               st           = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    phyPort;
    GT_BOOL                 enable;
    CPSS_PORT_DIRECTION_ENT direction;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_TRUE))
        {
            /*
                1.1.1. Call with direction [RX, TX]. Expected: GT_OK.
            */

            st = cpssDxChCscdHashInDsaEnableGet(dev, phyPort, CPSS_PORT_DIRECTION_RX_E, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

            st = cpssDxChCscdHashInDsaEnableGet(dev, phyPort, CPSS_PORT_DIRECTION_TX_E, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

            /*
                1.1.2. Call with direction [BOTH]. Expected: GT_BAD_PARAM.
            */
            st = cpssDxChCscdHashInDsaEnableGet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);

            /*
                1.1.3. Call with wrong 'direction' values. Expected: GT_BAD_PARAM.
            */
            direction = CPSS_PORT_DIRECTION_RX_E; /* to avoid compiler warning */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdHashInDsaEnableGet
                                (dev, phyPort, direction, &enable),
                                direction);
        }

        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_FALSE))
        {
            st = cpssDxChCscdHashInDsaEnableGet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for phyPort number.                         */

        phyPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdHashInDsaEnableGet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU phyPort number.                                     */
        phyPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdHashInDsaEnableGet(dev, phyPort, CPSS_PORT_DIRECTION_RX_E, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);


    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdHashInDsaEnableGet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 1.3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdHashInDsaEnableGet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortTcProfiletSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_PORT_DIRECTION_ENT direction,
    IN CPSS_DXCH_PORT_PROFILE_ENT portProfile
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortTcProfiletSet)
{
/*
    1.   Iterate over all active devices:
    1.1. Iterate over active ports.
    1.1.1. Call cpssDxChCscdPortTcProfiletSet with 'profile'
    in [CPSS_DXCH_PORT_PROFILE_NETWORK_E , CPSS_DXCH_PORT_PROFILE_CSCD_E,
        CPSS_DXCH_PORT_PROFILE_FABRIC_E, CPSS_DXCH_PORT_PROFILE_LOOPBACK_E],
    'direction' in [RX, TX, BOTH] and check values with cpssDxChCscdPortTcProfiletGet
    Expected: GT_OK.
    1.1.2. Call with wrong 'direction'
    Expected: GT_BAD_PARAM.
    1.1.3. Call with wrong 'portProfile'
    Expected: GT_BAD_PARAM.
    1.2. Iterate over non available ports. Expected: GT_BAD_PARAM
    1.3. Call with port out of range.   Expected: GT_BAD_PARAM
    1.4. Call with CPU port. Expected: GT_OK
    2.   Call for not active devices.  Extected: GT_NOT_APPLICABLE_DEVICE
    3.   Call for device out of range. Extected: GT_NOT_APPLICABLE_DEVICE
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    phyPort;
    CPSS_PORT_DIRECTION_ENT directionArr[] = {CPSS_PORT_DIRECTION_RX_E,
                                              CPSS_PORT_DIRECTION_TX_E,
                                              CPSS_PORT_DIRECTION_BOTH_E};
    GT_BOOL                 profileArr[] = {CPSS_DXCH_PORT_PROFILE_NETWORK_E,
                                            CPSS_DXCH_PORT_PROFILE_CSCD_E,
                                            CPSS_DXCH_PORT_PROFILE_FABRIC_E,
                                            CPSS_DXCH_PORT_PROFILE_LOOPBACK_E};
    CPSS_PORT_DIRECTION_ENT direction;
    CPSS_DXCH_PORT_PROFILE_ENT  profile;
    CPSS_DXCH_PORT_PROFILE_ENT  profileGet;
    GT_U32                  i;
    GT_U32                  j;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_TRUE))
        {
            /*
                1.1.1. Call cpssDxChCscdPortTcProfiletSet with 'profile'
                    in [CPSS_DXCH_PORT_PROFILE_NETWORK_E , CPSS_DXCH_PORT_PROFILE_CSCD_E,
                    CPSS_DXCH_PORT_PROFILE_FABRIC_E, CPSS_DXCH_PORT_PROFILE_LOOPBACK_E],
                    'direction' in [RX, TX, BOTH] and check values with cpssDxChCscdPortTcProfiletGet
                Expected: GT_OK.
            */
            for (i = 0; i < sizeof(directionArr)/sizeof(directionArr[0]); i++)
            {
                direction = directionArr[i];
                for (j = 0; j < sizeof(profileArr)/sizeof(profileArr[0]); j++)
                {
                    profile = profileArr[j];
                    st = cpssDxChCscdPortTcProfiletSet(dev, phyPort, direction, profile);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, phyPort, direction, profile);

                    if (direction == CPSS_PORT_DIRECTION_BOTH_E)
                    {
                        st = cpssDxChCscdPortTcProfiletGet(
                            dev, phyPort, CPSS_PORT_DIRECTION_RX_E, &profileGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(
                            GT_OK, st, dev, phyPort, CPSS_PORT_DIRECTION_RX_E, profile);

                        UTF_VERIFY_EQUAL2_STRING_MAC(
                            profile, profileGet, "Get 'profile' different than set: %d, %d", dev, phyPort);

                        st = cpssDxChCscdPortTcProfiletGet(
                            dev, phyPort, CPSS_PORT_DIRECTION_TX_E, &profileGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(
                            GT_OK, st, dev, phyPort, CPSS_PORT_DIRECTION_TX_E, profile);

                        UTF_VERIFY_EQUAL2_STRING_MAC(
                            profile, profileGet, "Get 'profile' different than set: %d, %d", dev, phyPort);
                    }
                    else
                    {
                        st = cpssDxChCscdPortTcProfiletGet(dev, phyPort, direction, &profileGet);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, phyPort, direction, profile);

                        UTF_VERIFY_EQUAL2_STRING_MAC(
                            profile, profileGet, "Get 'profile' different than set: %d, %d", dev, phyPort);
                    }
                }
            }

            /*
                1.1.2. Call with wrong 'direction' values. Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdPortTcProfiletSet
                                (dev, phyPort, direction, CPSS_DXCH_PORT_PROFILE_NETWORK_E),
                                direction);
            /*
                1.1.3. Call with wrong 'profile' values. Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdPortTcProfiletSet
                                (dev, phyPort, CPSS_PORT_DIRECTION_TX_E, profile),
                                profile);
        }

        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_FALSE))
        {
            st = cpssDxChCscdPortTcProfiletSet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, CPSS_DXCH_PORT_PROFILE_NETWORK_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for phyPort number.                         */

        phyPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortTcProfiletSet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, CPSS_DXCH_PORT_PROFILE_NETWORK_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU phyPort number.                                     */
        phyPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortTcProfiletSet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, CPSS_DXCH_PORT_PROFILE_NETWORK_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);


    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortTcProfiletSet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, CPSS_DXCH_PORT_PROFILE_NETWORK_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 1.3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortTcProfiletSet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, CPSS_DXCH_PORT_PROFILE_NETWORK_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCscdPortTcProfiletGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_PORT_DIRECTION_ENT direction,
    IN CPSS_DXCH_PORT_PROFILE_ENT * portProfilePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortTcProfiletGet)
{
/*
    1.   Iterate over all active devices:

    1.1. Iterate over active ports.
    1.1.1. Call with 'direction' [RX, TX]. Expected: GT_OK
    1.1.2. Call with 'direction' [BOTH].   Expected: GT_BAD_PARAM
    1.1.3. Call with wrong 'direction'.    Expected: GT_BAD_PARAM
    1.2. Iterate over non available ports. Expected: GT_BAD_PARAM
    1.3. Call with port out of range.      Expected: GT_BAD_PARAM
    1.4. Call with CPU port.               Expected: GT_OK
    2.   Call for not active devices.      Extected: GT_NOT_APPLICABLE_DEVICE
    3.   Call for device out of range.     Extected: GT_NOT_APPLICABLE_DEVICE
*/
    GT_STATUS               st           = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    phyPort;
    CPSS_DXCH_PORT_PROFILE_ENT  profile;
    CPSS_PORT_DIRECTION_ENT direction;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_TRUE))
        {
            /*
                1.1.1. Call with direction [RX, TX]. Expected: GT_OK.
            */

            st = cpssDxChCscdPortTcProfiletGet(dev, phyPort, CPSS_PORT_DIRECTION_RX_E, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

            st = cpssDxChCscdPortTcProfiletGet(dev, phyPort, CPSS_PORT_DIRECTION_TX_E, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

            /*
                1.1.2. Call with direction [BOTH]. Expected: GT_BAD_PARAM.
            */
            st = cpssDxChCscdPortTcProfiletGet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);

            /*
                1.1.3. Call with wrong 'direction' values. Expected: GT_BAD_PARAM.
            */
            direction = CPSS_PORT_DIRECTION_RX_E; /* to avoid compiler warning */
            UTF_ENUMS_CHECK_MAC(cpssDxChCscdPortTcProfiletGet
                                (dev, phyPort, direction, &profile),
                                direction);
        }

        st = prvUtfNextPhyPortReset(&phyPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&phyPort, GT_FALSE))
        {
            st = cpssDxChCscdPortTcProfiletGet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for phyPort number.                         */

        phyPort = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortTcProfiletGet(dev, phyPort, CPSS_PORT_DIRECTION_BOTH_E, &profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, phyPort);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU phyPort number.                                     */
        phyPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortTcProfiletGet(dev, phyPort, CPSS_PORT_DIRECTION_RX_E, &profile);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, phyPort);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
        UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_PUMA2_E | UTF_PUMA3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E);


    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortTcProfiletGet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, &profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 1.3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortTcProfiletGet(dev, 0, CPSS_PORT_DIRECTION_BOTH_E, &profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet
(
  IN  GT_U8      dev,
  IN  GT_U8      portNum,
  IN  GT_BOOL    acceptSkipSaLookup
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet with non-null acceptSkipSaLookupPtr.
    Expected: GT_OK and the same acceptSkipSaLookup.
    1.2. For all active devices go over all non available physical ports.
    1.2.1. Call function for each non-active port
    Expected: GT_BAD_PARAM
    1.3. For active device, out of bound ports check that function returns GT_BAD_PARAM
    1.4. For active device, cpu port check that function returns GT_OK
    2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.
    3. For active device, out of bound dev ID check that function returns GT_BAD_PARAM
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    GT_BOOL   acceptSkipSaLookup    = GT_FALSE;
    GT_BOOL   acceptSkipSaLookupGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call with enable [GT_FALSE] */
            acceptSkipSaLookup = GT_FALSE;

            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(dev, port, acceptSkipSaLookup);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, acceptSkipSaLookup);

            /*
                1.1.2. Call cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet with non-null enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookupGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(acceptSkipSaLookup, acceptSkipSaLookupGet,
                       "get another acceptSkipSaLookup than was set: %d", dev, port);

            /* Call with enable [GT_TRUE] */
            acceptSkipSaLookup = GT_TRUE;

            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(dev, port, acceptSkipSaLookup);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, acceptSkipSaLookup);

            /*
                1.1.2. Call cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet with non-null enablePtr.
                Expected: GT_OK and the same enable.
            */
            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookupGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(acceptSkipSaLookup, acceptSkipSaLookupGet,
                       "get another enable than was set: %d", dev, port);
        }

        acceptSkipSaLookup = GT_FALSE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(dev, port, acceptSkipSaLookup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(dev, port, acceptSkipSaLookup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(dev, port, acceptSkipSaLookup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(dev, port, acceptSkipSaLookup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(dev, port, acceptSkipSaLookup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet
(
  IN GT_U8      dev,
  IN GT_U8      portNum,
  OUT GT_BOOL  *acceptSkipSaLookupPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
    1.2.1. Call function for each non-active port
    Expected: GT_BAD_PARAM
    1.3. For active device, out of bound ports check that function returns GT_BAD_PARAM
    1.4. For active device, cpu port check that function returns GT_OK
    2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.
    3. For active device, out of bound dev ID check that function returns GT_BAD_PARAM
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = CSCD_VALID_PHY_PORT_CNS;

    GT_BOOL   acceptSkipSaLookup = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = CSCD_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(dev, port, &acceptSkipSaLookup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChCscd suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChCscd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDevMapTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDevMapTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdRemapQosModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdRemapQosModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCtrlQosSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCtrlQosGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdRemapDataQosTblSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdRemapDataQosTblGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDsaSrcDevFilterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDsaSrcDevFilterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdHyperGInternalPortModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdHyperGInternalPortModeGet)
/*    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdUcRouteTrunkHashTypeSet) */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdHyperGPortCrcModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdHyperGPortCrcModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverFastStackRecoveryEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverFastStackRecoveryEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverSecondaryTargetPortMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverSecondaryTargetPortMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverPortIsLoopedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdFastFailoverPortIsLoopedGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortBridgeBypassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortBridgeBypassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdQosPortTcRemapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdQosPortTcRemapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdQosTcRemapTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdQosTcRemapTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdOrigSrcPortFilterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdOrigSrcPortFilterEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDevMapLookupModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDevMapLookupModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortLocalDevMapLookupEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortLocalDevMapLookupEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisReservedDevNumSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisReservedDevNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisSrcIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisSrcIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDbRemoteHwDevNumModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdDbRemoteHwDevNumModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortMruCheckOnCascadeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortMruCheckOnCascadeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortStackAggregationConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortStackAggregationConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortStackAggregationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortStackAggregationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortQosDsaModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortQosDsaModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdQosTcDpRemapTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdQosTcDpRemapTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdHashInDsaEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdHashInDsaEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortTcProfiletSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortTcProfiletGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChCscd)

