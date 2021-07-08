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
* @file tgfCommonTunnelUT.c
*
* @brief Enhanced UTs for CPSS Tunneling
*
* @version   158
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#else
#define PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(_devNum) GT_FALSE
#endif
#include <tunnel/prvTgfTunnelTermIpv4overIpv4.h>
#include <tunnel/prvTgfTunnelTermIpv4overGreIpv4.h>
#include <tunnel/prvTgfTunnelTermIpv4overIpv4RedirectToLTT.h>
#include <tunnel/prvTgfTunnelTermPortGroupIpv4overIpv4.h>
#include <tunnel/prvTgfTunnelTermIpv4overIpv4Trap.h>
#include <tunnel/prvTgfTunnelStartIpv4overGreIpv4.h>
#include <tunnel/prvTgfTunnelStartIpv6overIpv4.h>
#include <tunnel/prvTgfTunnelStartGeneric.h>
#include <tunnel/prvTgfTunnelTermIpv6overIpv4.h>
#include <tunnel/prvTgfTunnelMimStart.h>
#include <tunnel/prvTgfTunnelStartMpls.h>
#include <tunnel/prvTgfTunnelStartMplsTtlParallelLookup.h>
#include <tunnel/prvTgfTunnelMacInMacKey.h>
#include <tunnel/prvTgfDetailedTtiActionType2Entry.h>
#include <tunnel/prvTgfDetailedMimTunnelStart.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>
#include <tunnel/prvTgfTunnelTermGlobalPort2LocalPort.h>
#include <tunnel/prvTgfTunnelStartEtherOverIpv4GreRedirectToEgress.h>
#include <tunnel/prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEport.h>
#include <tunnel/prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressOffset.h>
#include <tunnel/prvTgfTunnelStartQInQ.h>
#include <tunnel/prvTgfTunnelTermEtherOverIpv4RedirectToEgress.h>
#include <tunnel/prvTgfTunnelTermEtherOverIpv4RedirectToEgressEport.h>
#include <tunnel/prvTgfTunnelTermEtherOverIpv4NoRedirectBridging.h>
#include <tunnel/prvTgfTunnelTermEtherOverIpv4GreRedirectToEgress.h>
#include <tunnel/prvTgfTunnelIpEtherOverIpv4Gre.h>
#include <tunnel/prvTgfEtherRedirectToEgress.h>
#include <tunnel/prvTgfEtherRedirectToEgressWithTS.h>
#include <tunnel/tgfTunnelStartGenericIpv4UdpTemplate.h>
#include <tunnel/tgfTunnelStartGenericIpv6Template.h>
#include <tunnel/prvTgfTunnelTermEtherOverMplsRedirectToEgress.h>
#include <tunnel/prvTgfTunnelTermEtherOverMplsPwLabel.h>
#include <tunnel/prvTgfTunnelTermEtherOverMplsPwLabelParallelLookup.h>
#include <tunnel/prvTgfTunnelTermParallelLookup.h>
#include <tunnel/prvTgfTunnelTermExceptionCpuCode.h>
#include <tunnel/tgfTunnelTermMirrorToIngressAnalyzer.h>
#include <tunnel/tgfTunnelTermSetMac2Me.h>
#include <tunnel/tgfTunnelTermSetIPclUdbConfigTableIndex.h>
#include <tunnel/tgfTunnelTcamCheckLookupOrder.h>
#include <tunnel/prvTgfTunnelTermTrigReqMpls.h>
#include <tunnel/prvTgfTunnelTermSourceIdModify.h>
#include <tunnel/prvTgfTunnelTermSourceEportAssignment.h>
#include <tunnel/tgfTunnelTermIpv6ExtensionHeaderSet.h>
#include <tunnel/prvTgfTunnelPwTtlExpiryVccvException.h>
#include <tunnel/prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup.h>
#include <tunnel/prvTgfTunnelPwCwBasedETree.h>
#include <tunnel/prfTgfTunnelDualLookupNestedVLan.h>
#include <tunnel/prvTgfTunnelTermTSPassengerTypeDualLookup.h>
#include <tunnel/prvTgfTunnelTermPbrDualLookup.h>
#include <tunnel/prvTgfTunnelTermEVlanPrecDualLookup.h>
#include <tunnel/prvTgfTunnelTermTtiDsaMetadata.h>
#include <tunnel/prvTgfTunnelTermTtiIpv4ArpUdb.h>
#include <tunnel/prvTgfTunnelTermTtiFullDb.h>
#include <tunnel/customer/cstTunnelMplsTests.h>
#include <tunnel/prvTgfTunnelIpv4Ecn.h>
#include <tunnel/tgfTunnelTermSip6Metadata.h>
#include <tunnel/prvTgfTunnelTermTunnelHeaderStart.h>



/* trill tests are not operational */
#include <tunnel/prvTgfTunnelTrillTunnelStart.h>
#include <tunnel/prvTgfTunnelTrillTti.h>
#include <tunnel/prvTgfTunnelStartTrillUnknownUcFlooding.h>
#include <tunnel/prvTgfTunnelStartTrillKnownUcEcmp.h>
#include <tunnel/prvTgfTunnelTrillTtiKnownUcTransit.h>
#include <tunnel/prvTgfTunnelTermTrillUnknownUcTransitAndTermination.h>
#include <tunnel/prvTgfTunnelStartTrillKnownMc.h>
#include <tunnel/prvTgfTunnelTrillTtiExceptionsCheck.h>
#ifdef CHX_FAMILY
#include <tunnel/customer/cstTunnelTermVrfId.h>
#include <tunnel/customer/cstTunnelStartMplsVpws.h>
#endif

#include <common/tgfTunnelGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfCosGen.h>


GT_VOID tgfTunnelTermMapMplsChannelToOamOpcodeTest(GT_VOID);
GT_VOID tgfTunnelTermSetIPclOverrideConfigTableIndexTest(GT_VOID);
GT_VOID tgfTunnelTermIpv6HeaderExceptions(GT_VOID);
GT_VOID tgfTunnelTermIpv6PclId(GT_VOID);
GT_VOID tgfTunnelTermIpv6LocalDevSourceIsTrunk(GT_VOID);
GT_VOID tgfTunnelTermIpv6L3DataWord0FirstNibble(GT_VOID);
GT_VOID tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId(GT_VOID);
GT_VOID prvTgfTunnelCapwapTest(GT_VOID);
GT_VOID prvTgfTunnelIpv6CapwapTest(GT_VOID);
GT_VOID prvTgfTunnelTermCncParallelLookup(GT_VOID);
GT_VOID prvTgfTunnelMplsControlWord(GT_VOID);
GT_VOID prvTgfTunnelIpv6MplsControlWordTest(GT_VOID);

#ifdef CHX_FAMILY
GT_STATUS cstTunnelStartIpv4overGreIpv4ConfigSet(GT_VOID);
#endif

/* original prvTgfPortsArray  */
static GT_U32   prvTgfPortsArrayOrig[PRV_TGF_MAX_PORTS_NUM_CNS];
/* original prvTgfPortsNum  */
static GT_U8   prvTgfPortsNumOrig;

#define SAVE_ORIG_PORTS_MAC                             \
    prvTgfPortsArrayOrig[0] = prvTgfPortsArray[0];      \
    prvTgfPortsArrayOrig[1] = prvTgfPortsArray[1];      \
    prvTgfPortsArrayOrig[2] = prvTgfPortsArray[2];      \
    prvTgfPortsArrayOrig[3] = prvTgfPortsArray[3];      \
    prvTgfPortsArrayOrig[4] = prvTgfPortsArray[4];      \
    prvTgfPortsArrayOrig[5] = prvTgfPortsArray[5];      \
    prvTgfPortsArrayOrig[6] = prvTgfPortsArray[6];      \
    prvTgfPortsArrayOrig[7] = prvTgfPortsArray[7];      \
    prvTgfPortsArrayOrig[8] = prvTgfPortsArray[8];      \
    prvTgfPortsArrayOrig[9] = prvTgfPortsArray[9];      \
    prvTgfPortsNumOrig = prvTgfPortsNum

#define RESTORE_ORIG_PORTS_MAC                          \
    prvTgfPortsArray[0] = prvTgfPortsArrayOrig[0];      \
    prvTgfPortsArray[1] = prvTgfPortsArrayOrig[1];      \
    prvTgfPortsArray[2] = prvTgfPortsArrayOrig[2];      \
    prvTgfPortsArray[3] = prvTgfPortsArrayOrig[3];      \
    prvTgfPortsArray[4] = prvTgfPortsArrayOrig[4];      \
    prvTgfPortsArray[5] = prvTgfPortsArrayOrig[5];      \
    prvTgfPortsArray[6] = prvTgfPortsArrayOrig[6];      \
    prvTgfPortsArray[7] = prvTgfPortsArrayOrig[7];      \
    prvTgfPortsArray[8] = prvTgfPortsArrayOrig[8];      \
    prvTgfPortsArray[9] = prvTgfPortsArrayOrig[9];      \
    prvTgfPortsNum      = prvTgfPortsNumOrig;

static GT_U32 ttiMultiPortGroupPortSendArray[CPSS_MAX_PORT_GROUPS_CNS] = {
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(0,4),/*  4*/ /*0x04*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(1,4),/* 20*/ /*0x14*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(2,4),/* 36*/ /*0x24*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(3,4),/* 52*/ /*0x34*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(4,4),/* 68*/ /*0x44*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(5,4),/* 84*/ /*0x54*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(6,4),/*100*/ /*0x64*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(7,4) /*116*/ /*0x74*/
};

/* local port 2 in port groups */
GT_U32 prvTgfTtiMultiPortGroupLocalPort2Array[CPSS_MAX_PORT_GROUPS_CNS] = {
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(0,2),/*  2*/ /*0x02*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(1,2),/* 18*/ /*0x12*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(2,2),/* 34*/ /*0x22*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(3,2),/* 50*/ /*0x32*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(4,2),/* 66*/ /*0x42*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(5,2),/* 82*/ /*0x52*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(6,2),/* 98*/ /*0x62*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(7,2) /*114*/ /*0x72*/
};

/* local port 8 in port groups */
GT_U32 prvTgfTtiMultiPortGroupLocalPort2_XLG_Array[CPSS_MAX_PORT_GROUPS_CNS] = {
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(0,8),/*  8*/ /*0x08*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(1,8),/* 24*/ /*0x18*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(2,8),/* 40*/ /*0x28*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(3,8),/* 56*/ /*0x38*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(4,8),/* 72*/ /*0x48*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(5,8),/* 88*/ /*0x58*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(6,8),/*104*/ /*0x68*/
    PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(7,8) /*120*/ /*0x78*/
};

#define COPY_ARRAY_MAC(trgArray,casting,srcArray,numElements)       \
    {                                                       \
        GT_U32  _ii;                                        \
        for(_ii=0 ; _ii < (GT_U32)(numElements) ;_ii++)     \
        {                                                   \
            (trgArray)[_ii] = (casting)(srcArray)[_ii];     \
        }                                                   \
    }

/* port number to send traffic from */
GT_U32 prvTgfSendPort;

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv4 over IPv4:
    1. configure VLAN, FDB entries;
       configure TTI rule;
    2. configure TTI 'old key' IPv4;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
    3. configure TTI UDB key IPv4 other;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4overIpv4)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    /* basic test */
    prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet(prvUtfVrfId, 0, GT_TRUE);

    /* Set Route configuration */
    prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet();

    /* Set TTI configuration */
    /* basic test */
    prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet(0, GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Enable IP lookup only if there is a MAC to Me entry */
        prvTgfTunnelTermIpv4overIpv4MacToMeEnableSet(GT_TRUE);

        /* Generate traffic - expect no traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic(GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Add MAC to Me entry */
        prvTgfTunnelTermIpv4overIpv4MacToMeSet();

        /* Generate traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Disable IP lookup only if there is a MAC to Me entry */
        prvTgfTunnelTermIpv4overIpv4MacToMeEnableSet(GT_FALSE);

        /* Remove MAC to Me entry */
        prvTgfTunnelTermIpv4overIpv4MacToMeDel();

        /* Generate traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);
    }

    if(prvTgfResetModeGet() == GT_TRUE)
    {
        prvTgfResetAndInitSystem();
    }
    else
    {
        /* Restore configuration */
        prvTgfTunnelTermIpv4overIpv4ConfigurationRestore(GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        prvTgfPclRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv4 over IPv4:
    1. configure VLAN, FDB entries;
       configure TTI rule;
    2. configure TTI 'old key' IPv4;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
    3. configure TTI UDB key IPv4 other;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4overIpv4EArch)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Set wrong TTI configuration - verify failure
    6. Generate traffic
    7. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet(prvUtfVrfId, 0, GT_TRUE); /* basic test */

    /* Set Route configuration */
    prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet();

    /* Set TTI configuration */
    /* basic test */
    prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet(0, GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Try to verride the TTI rule with illegal params */
    prvTgfTunnelTermIpv4overIpv4OverrideTtiRuleWithIllegalParams();

    /* Generate traffic - verify that a rollback was done correctly after
                          overriding the TTI rule with illegal params */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfTunnelTermIpv4overIpv4ConfigurationRestore(GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    prvTgfPclRestore();

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Set Base configuration */
        prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet(prvUtfVrfId, 1, GT_TRUE); /* UDB test */

        /* Set Route configuration */
        prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet();

        /* Set TTI configuration */
        /* UDB test */
        prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet(1, GT_TRUE);

        /* Generate traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Restore configuration */
        prvTgfTunnelTermIpv4overIpv4ConfigurationRestore(GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        prvTgfPclRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv4 over IPv4 with LTT config:
    configure VLAN, FDB entries;
    configure LTT entry and TTI rule;
    send tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4overIpv4RedirectToLTT)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E | UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NO support for LTT in PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelTermIpv4overIpv4RedirectToLttBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelTermIpv4overIpv4RedirectToLttRouteConfigurationSet(prvUtfVrfId);

    /* Set TTI configuration */
    prvTgfTunnelTermIpv4overIpv4RedirectToLttTtiConfigurationSet(prvUtfVrfId);

    /* Generate traffic */
    prvTgfTunnelTermIpv4overIpv4RedirectToLttTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermIpv4overIpv4RedirectToLttConfigurationRestore(prvUtfVrfId);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv4 over IPv4 for Multi PortGroup device:
    create tested portGroups and iterate thru ports from each portGroup;
    configure VLAN, FDB entries;
    configure TTI rule;
    send tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4overIpv4SinglePortGroup)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32 iteration;

    GT_PORT_GROUPS_BMP  portGroupsBmp;

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E |
                                        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    if(GT_FALSE == PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    SAVE_ORIG_PORTS_MAC;

    /******************** tested ports - port in each group *********************/
    iteration=0;
    if (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E)
    {
        prvTgfPortsArray[iteration++] = 0;        /*0x00*/
        prvTgfPortsArray[iteration++] = 24;       /*0x18*/
        prvTgfPortsArray[iteration++] = 32;       /*0x20*/
        prvTgfPortsArray[iteration++] = 57;       /*0x39*/
        prvTgfPortsArray[iteration++] = 73;       /*0x49*/
        prvTgfPortsArray[iteration++] = 88;       /*0x58*/
        prvTgfPortsArray[iteration++] = 96;       /*0x60*/
        prvTgfPortsArray[iteration++] = 112;      /*0x70*/
    }
    else
    {
        prvTgfPortsArray[iteration++] = 0;        /*0x00*/
        prvTgfPortsArray[iteration++] = 18;       /*0x12*/
        prvTgfPortsArray[iteration++] = 37;       /*0x25*/
        prvTgfPortsArray[iteration++] = 54;       /*0x36*/
        prvTgfPortsArray[iteration++] = 65;       /*0x41*/
        prvTgfPortsArray[iteration++] = 83;       /*0x53*/
        prvTgfPortsArray[iteration++] = 103;      /*0x67*/
        prvTgfPortsArray[iteration++] = 120;      /*0x78*/
    }


    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    prvTgfPortsNum = (GT_U8)(PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(prvTgfDevNum,iteration)
    {
        /* send port is different for each port group */
        prvTgfPortsArray[0] = ttiMultiPortGroupPortSendArray[iteration];

        prvTgfSendPort = ttiMultiPortGroupPortSendArray[iteration];

        /* write the rule only to a single port group */
        portGroupsBmp = 1 << iteration;

        /* Set Base configuration */
        prvTgfTunnelTermPortGroupIpv4overIpv4BaseConfigurationSet();

            /* Set Route configuration */
        prvTgfTunnelTermPortGroupIpv4overIpv4RouteConfigurationSet(prvUtfVrfId);

        /* Set TTI configuration */
        prvTgfTunnelTermPortGroupIpv4overIpv4TtiConfigurationSet(prvUtfVrfId,
                                                                 portGroupsBmp);

        /* Generate traffic */
        prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerate();

        /* Restore configuration */
        prvTgfTunnelTermPortGroupIpv4overIpv4PortConfigurationRestore(prvTgfSendPort);
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(prvTgfDevNum,iteration)

    /* Restore configuration */
    prvTgfTunnelTermPortGroupIpv4overIpv4ConfigurationRestore(prvUtfVrfId);

    RESTORE_ORIG_PORTS_MAC;

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv4 over IPv4 with valid\invalid rules:
    configure VLAN, FDB entries;
    configure TTI rule;
    send tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port;
    invalidate TTI rules;
    send tunneled traffic to match TTI rule;
    verify no traffic;
    send tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4overIpv4Invalidate)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration ( 'old key' IPv4 )
    4. Generate traffic
    5. Invalidate Rule
    6. Generate traffic
    7. Validate rule
    8. Generate traffic
    9. Restore configuration
    10. Set TTI configuration ( TTI UDB IPv4 other key' )
    11. Generate traffic
    12. Invalidate Rule
    13. Generate traffic
    14. Validate rule
    15. Generate traffic
    16. Restore configuration

*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    /* basic test */
    prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet(prvUtfVrfId, 0, GT_TRUE);

    /* Set Route configuration */
    prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet();

    /* Set TTI configuration */
    /* basic test */
    prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet(0, GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* invalidate TTI rules */
    prvTgfTunnelTermRuleValidStatusSet(GT_FALSE);

    /* Generate traffic -- check no traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic(GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* validate TTI rules */
    prvTgfTunnelTermRuleValidStatusSet(GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfTunnelTermIpv4overIpv4ConfigurationRestore(GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    prvTgfPclRestore();


    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Set Base configuration */
        /* UDB test */
        prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet(prvUtfVrfId, 1, GT_TRUE);

        /* Set Route configuration */
        prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet();

        /* Set TTI configuration */
        /* UDB test */
        prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet(1, GT_TRUE);

        /* Generate traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* invalidate TTI rules */
        prvTgfTunnelTermRuleValidStatusSet(GT_FALSE);

        /* Generate traffic -- check no traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic(GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* validate TTI rules */
        prvTgfTunnelTermRuleValidStatusSet(GT_TRUE);

        /* Generate traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        /* Restore configuration */
        prvTgfTunnelTermIpv4overIpv4ConfigurationRestore(GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        prvTgfPclRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv6 over IPv4:
    configure VLAN, FDB entries;
    configure TTI rule;
    send tunneled IPv6 over IPv4 traffic to match TTI rule;
    verify passenger IPv6 traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv6overIpv4)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration ( configure TTI 'old key' IPv4 )
    4. Generate traffic
    5. Restore configuration
    6. Set TTI configuration ( configure TTI UDB key IPv4 other )
    7. Generate traffic
    8. Restore configuration

*/

    GT_U32      prvUtfVrfId = 0;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    PRV_TGF_SKIP_PASS_OK_ON_FULL_LPM_MEM_IN_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelTermIpv6overIpv4BaseConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelTermIpv6overIpv4RouteConfigurationSet(prvUtfVrfId);

    /* Set TTI configuration */
    prvTgfTunnelTermIpv6overIpv4TtiConfigurationSet(prvUtfVrfId, GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermIpv6overIpv4TrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermIpv6overIpv4ConfigurationRestore(prvUtfVrfId, GT_FALSE);

    prvTgfPclRestore();

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Set Base configuration */
        prvTgfTunnelTermIpv6overIpv4BaseConfigurationSet();

        /* Set Route configuration */
        prvTgfTunnelTermIpv6overIpv4RouteConfigurationSet(prvUtfVrfId);

        /* Set TTI configuration */
        prvTgfTunnelTermIpv6overIpv4TtiConfigurationSet(prvUtfVrfId, GT_TRUE);

        /* Generate traffic */
        prvTgfTunnelTermIpv6overIpv4TrafficGenerate();

        /* Restore configuration */
        prvTgfTunnelTermIpv6overIpv4ConfigurationRestore(prvUtfVrfId, GT_TRUE);

        prvTgfPclRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv4 over IPv4 with action TRAP:
    configure VLAN, FDB entries;
    configure TTI rule with TRAP_TO_CPU action;
    send tunneled traffic to match TTI rule;
    verify traffic is trapped.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4overIpv4Trap)
{
/*
    1. Set Base configuration
    2. Set IP configuration
    3. Set TCAM configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelTermIpv4overIpv4TrapBaseConfigSet();

    /* Set IP configuration */
    prvTgfTunnelTermIpv4overIpv4TrapIpConfigSet(prvUtfVrfId);

    /* Set TCAM configuration */
    prvTgfTunnelTermIpv4overIpv4TrapTcamConfigSet(prvUtfVrfId);

    /* Generate traffic */
    prvTgfTunnelTermIpv4overIpv4TrapTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermIpv4overIpv4TrapConfigRestore(prvUtfVrfId);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination header offset:
    1. Configure VLAN, FDB entries.
    2. Configure TTI rule with L3 tunnel header offset.
    3. Send traffic and verify traffic on nextHop port.
    4. Configure TTI rule with L4 tunnel header offset.
    5. Send traffic and verify traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermTunnelHeaderStart)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32  prvUtfVrfId = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelTermTunnelHeaderStartBaseConfigurationSet(prvUtfVrfId);

    /* Set Route configuration */
    prvTgfTunnelTermTunnelHeaderStartRouteConfigurationSet();

    /* Set TTI configuration with L3 offset */
    prvTgfTunnelTermTunnelHeaderStartTtiConfigurationSet(GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermTunnelHeaderStartTrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS);

    /* Set TTI configuration with L4 offset */
    prvTgfTunnelTermTunnelHeaderStartTtiConfigurationSet(GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermTunnelHeaderStartTrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS);

    if(prvTgfResetModeGet() == GT_TRUE)
    {
        prvTgfResetAndInitSystem();
    }
    else
    {
        /* Restore configuration */
        prvTgfTunnelTermTunnelHeaderStartConfigurationRestore(GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

        prvTgfPclRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start IPv4 over GRE IPv4:
    configure VLAN, FDB entries;
    configure TS entry with GRE ethType;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartIpv4overGreIpv4)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelStartIpv4overGreIpv4BaseConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelStartIpv4overGreIpv4RouteConfigurationSet(prvUtfVrfId);

    /* Set Tunnel configuration */
    prvTgfTunnelStartIpv4overGreIpv4TunnelConfigurationSet();

    /* Generate traffic */
    prvTgfTunnelStartIpv4overGreIpv4TrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartIpv4overGreIpv4ConfigurationRestore(prvUtfVrfId);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start IPv6 over IPv4:
    configure VLAN, FDB entries;
    configure TS entry;
    send IPv6 traffic to match TS entry;
    verify tunneled IPv6 over IPv4 traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartIpv6overIpv4)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    PRV_TGF_SKIP_PASS_OK_ON_FULL_LPM_MEM_IN_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelStartIpv6overIpv4BaseConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelStartIpv6overIpv4RouteConfigurationSet(prvUtfVrfId);

    /* Set Tunnel configuration */
    prvTgfTunnelStartIpv6overIpv4TunnelConfigurationSet();

    /* Generate traffic */
    prvTgfTunnelStartIpv6overIpv4TrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartIpv6overIpv4ConfigurationRestore(prvUtfVrfId);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Generic tunnel start entry:
    configure VLAN, FDB entries;
    configure TS entry;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartGeneric)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;
    PRV_TGF_TUNNEL_START_GENERIC_TYPE_ENT genericType;
    GT_U32      notAppFamilyBmp;

    /* Supported by SIP6 devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelStartGenericBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelStartGenericRouteConfigurationSet(prvUtfVrfId);

    for (genericType = PRV_TGF_TUNNEL_START_GENERIC_SHORT_TYPE_E; genericType <= PRV_TGF_TUNNEL_START_GENERIC_LONG_TYPE_E; genericType++)
    {
        /* Set Tunnel configuration */
        prvTgfTunnelStartGenericTunnelConfigurationSet(genericType);

        /* Generate traffic */
        prvTgfTunnelStartGenericTrafficGenerate();
    }

    /* Restore configuration */
    prvTgfTunnelStartGenericConfigurationRestore(prvUtfVrfId);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    3.1.1 Test MIM Tunnel Start (Forwarded From TTI)
    3.1.2 Test MIM Tunnel Start (Forwarded From PCL)
*/
UTF_TEST_CASE_MAC(tgfTunnelMimStartFromTti)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMimStart(PRV_TGF_TUNNEL_MIM_START_FROM_TTI_E);

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelMimStartFromPcl)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMimStart(PRV_TGF_TUNNEL_MIM_START_FROM_PCL_E);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic MIM tunnel termination functionality:
    configure VLAN, FDB entries;
    configure MIM TTI key lookup MAC mode;
    configure TTI rule for MIM key;
    send MIM tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermMimBasic)
{
/*
    1. Set Base configuration ( 'old key' MIM )
    2. Generate traffic
    3. Restore configuration ( 'old key' MIM )
    4. Set Base configuration ( 'TTI UDB' MIM )
    5. Generate traffic
    6. Restore configuration ( 'TTI UDB' MIM )
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

     /* Set Inlif configuration */
    prvTgfTunnelTermMimInlifConfigurationSet();

    /* Set Base configuration */
    prvTgfTunnelTermMimBasicConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermMimBasicTrafficGenerate(GT_TRUE);

    /* Enable MAC in MAC lookup only if there is a MAC to Me entry */
    prvTgfTunnelTermMimMacToMeEnableSet(GT_TRUE);

    /* Generate traffic - expect no traffic */
    prvTgfTunnelTermMimBasicTrafficGenerate(GT_FALSE);

    /* Add MAC to Me entry */
    prvTgfTunnelTermMimMacToMeSet();

    /* Generate traffic */
    prvTgfTunnelTermMimBasicTrafficGenerate(GT_TRUE);

    /* Disable MAC in MAC lookup only if there is a MAC to Me entry */
    prvTgfTunnelTermMimMacToMeEnableSet(GT_FALSE);

    /* Remove MAC to Me entry */
    prvTgfTunnelTermMimMacToMeDel();

    /* Generate traffic */
    prvTgfTunnelTermMimBasicTrafficGenerate(GT_TRUE);

    /* Restore configuration */
    prvTgfTunnelTermBasicMimConfigRestore();

    prvTgfPclRestore();


    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Set Base configuration */
        prvTgfTunnelTermMimBasicConfigSet();

        /* Generate traffic */
        prvTgfTunnelTermMimBasicTrafficGenerate(GT_TRUE);

        /* Restore configuration */
        prvTgfTunnelTermBasicMimConfigRestore();

        prvTgfPclRestore();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test MIM tunnel termination - passenger tag recognition - tag0 or tag1:
    Sip5 feature.
    send MIM (802.1ah) with passenger with 2 vlan tags.
    once configuration will be to recognize outer from TPID0 and once to recognize from TPID1
    when outer tag in passenger is tag0 in both cases.
    So traffic will match TTI rule for outer from TPID0 and it should be verified on egress interface.
    There would be no TTI match for outer from TPID1 and no traffic is detected on egress interface.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermMimPassengerOuterTag0or1)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    tgfTunnelTermMimPassengerOuterTag0or1Test();
}

/*----------------------------------------------------------------------------*/
/*
    3.2.1 MPLS LSR Push 1 label
    3.2.2 MPLS LSR Push 2 label
    3.2.3 MPLS LSR Push 3 label
*/
UTF_TEST_CASE_MAC(tgfTunnelMplsLsrPush1Label)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMplsPush(PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E);

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelMplsLsrPush2Label)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMplsPush(PRV_TGF_TUNNEL_MPLS_LSR_PUSH_2_LABEL_E);

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelMplsLsrPush3Label)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    /* Puma support only 2 MPLS labels in the tunnel start */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMplsPush(PRV_TGF_TUNNEL_MPLS_LSR_PUSH_3_LABEL_E);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test that checking egress interface based exp2exp mapping (for MPLS LSR) config:
    configure VLANs, enable TTI lookup for MPLS key and MAC DA mode;
    set TTI rules for MPLS Push key; set a X-over-MPLS Tunnel Start Entry;
    set egress QoS settings that mapped exp of tunnel start label to new value;
    send MPLS traffic to match TTI rule; verify exp field of received packet.
*/


UTF_TEST_CASE_MAC(tgfTunnelMplsPushAndEgressQosMapping)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMplsPushAndEgrQosCfgAdd();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test that checking egress interface based exp2exp mapping config:
    Ethernet packet enter the device , and bridged to egress eport.
    the egress eport will add MPLS tunnel start.
    the EXP of the TS will do remap EXP to EXP in specific 'QOS Mapping Table Select'
    configure VLANs,
    set a X-over-MPLS Tunnel Start Entry;
    set egress QoS settings that mapped exp of tunnel start label to new value;
    Verify exp field of received packet.
*/


UTF_TEST_CASE_MAC(tgfTunnelStartMplsEgressQosMappingPassengerEthernet)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelStartMplsEgressQosMappingPassengerEthernet();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test that checking 'outer vlan tag' modifications on TS packets.
    the test:
    Ethernet packet enter the device , and bridged to egress eport.
    the egress eport will add MPLS tunnel start.
    the EPCL will match this packet and will apply modifyVid = 'outer tag' and modifyUp = 'outer tag'
    expected behavior:
    1. if TS have vlan tag    --> it apply on this tag (regardless to tags in passenger)
    2. if TS have no vlan tag --> no modification done (regardless to tags in passenger)

    NOTE: the TS is pointed from the egress eport.(this is reason that test applicable only to E_ARCH)
*/


UTF_TEST_CASE_MAC(tgfTunnelStartMplsModifyOuterVlanTagPassengerEthernet)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelStartMplsModifyOuterVlanTagPassengerEthernet();
}


/*----------------------------------------------------------------------------*/
/*
    3.2.4 MPLS LSR Pop 1 label
    3.2.5 MPLS LSR Pop 2 label
*/
UTF_TEST_CASE_MAC(tgfTunnelMplsLsrPop1Label)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    prvTgfTunnelMplsPop(PRV_TGF_TUNNEL_MPLS_LSR_POP_1_LABEL_E);

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelMplsLsrPop2Label)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }
    prvTgfTunnelMplsPop(PRV_TGF_TUNNEL_MPLS_LSR_POP_2_LABEL_E);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    3.2.6 MPLS LSR Swap
    3.2.7 MPLS LSR Pop-Swap
*/
UTF_TEST_CASE_MAC(tgfTunnelMplsLsrSwap)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMplsPush(PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E);

    prvTgfPclRestore();
}
UTF_TEST_CASE_MAC(tgfTunnelMplsLsrPopSwap)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMplsPush(PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E);

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    MPLS LSR Pop 2 or 3 labels and check TTL Assignment in Parallel Lookup mode
*/
UTF_TEST_CASE_MAC(tgfTunnelMplsLsrPopLabelTtlParallelLookup)
{
    GT_U32      notAppFamilyBmp;
    GT_BOOL     copyTtlExpFromTunnelHeaderArr[PRV_TGF_MAX_LOOKUPS_NUM_CNS];
    GT_BOOL     enableDecrementTtlArr[PRV_TGF_MAX_LOOKUPS_NUM_CNS];
    GT_U8       lookupNum;
    GT_U8       testCounter=0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    copyTtlExpFromTunnelHeaderArr[0]=GT_TRUE;
    enableDecrementTtlArr[0]=GT_TRUE;

    for(lookupNum = 1; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        copyTtlExpFromTunnelHeaderArr[lookupNum]=GT_FALSE;
        enableDecrementTtlArr[lookupNum]=GT_FALSE;
    }

    prvTgfTunnelMplsPopTtlParallelLookup(copyTtlExpFromTunnelHeaderArr,enableDecrementTtlArr,GT_FALSE,GT_FALSE);
    testCounter++;

    while (testCounter<PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS)
    {
        copyTtlExpFromTunnelHeaderArr[testCounter-1]=GT_FALSE;
        copyTtlExpFromTunnelHeaderArr[testCounter]=GT_TRUE;

        enableDecrementTtlArr[testCounter-1]=GT_FALSE;
        enableDecrementTtlArr[testCounter]=GT_TRUE;

        if (testCounter==(PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1))
        {
            if (testCounter==(PRV_TGF_MAX_LOOKUPS_NUM_CNS-1))
            {
                prvTgfTunnelMplsPopTtlParallelLookup(copyTtlExpFromTunnelHeaderArr, enableDecrementTtlArr, GT_TRUE, GT_TRUE);
            }
            else
            {
                prvTgfTunnelMplsPopTtlParallelLookup(copyTtlExpFromTunnelHeaderArr, enableDecrementTtlArr, GT_FALSE, GT_TRUE);
            }
        }
        else
        {
            prvTgfTunnelMplsPopTtlParallelLookup(copyTtlExpFromTunnelHeaderArr,enableDecrementTtlArr,GT_FALSE,GT_FALSE);
        }
        testCounter++;
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of UP marking in MIM tunnel start:
    configure VLAN, FDB entries;
    configure MIM TS entry (up Mark Mode from QoS);
    configure Cos profile entry and set port QoS attributes (modify UP);
    send traffic to match TS entry;
    verify MIM tunneled traffic (with UP from QoS) on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartUpMarkingMode)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* skip test if VPLS mode enabled for xCat and xCat3 devices */
    if (prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT_E | UTF_XCAT3_E, CPSS-6202);
    }

    /* Set Base configuration */
    prvTgfTunnelStartUpMarkingModeConfigSet();

    /* Generate traffic */
    prvTgfTunnelStartUpMarkingModeTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartUpMarkingModeConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of the globally set MIM ether type:
    configure VLAN, FDB entries;
    configure MIM TS entry (up Mark Mode from Entry);
    send traffic to match TS entry;
    verify MIM tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartMimEtherType)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* skip test if VPLS mode enabled for xCat and xCat3 devices */
    if (prvUtfIsVplsModeUsed(prvTgfDevNum))
    {
        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT_E | UTF_XCAT3_E, CPSS-6202);
    }

    /* Set Base configuration */
    prvTgfTunnelStartMimEtherTypeConfigSet();

    /* Generate traffic */
    prvTgfTunnelStartMimEtherTypeTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartMimEtherTypeConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of I-DEI marking in MIM tunnel start:
    configure VLAN, FDB entries;
    configure MIM TS entry (up Mark Mode from entry);
    configure Cos profile entry and set port QoS attributes (modify UP);
    map DP GREEN to Drop Eligibility Indicator bit;
    send traffic with specific DP to match TS entry;
    verify MIM tunneled traffic (with I-DEI from DP-to-CFI) on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartIdeiMarkingMode)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelStartIdeiMarkingModeConfigSet();

    /* Generate traffic */
    prvTgfTunnelStartIdeiMarkingModeTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartIdeiMarkingModeConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/*
    4.1.4 Test functionality of MIM MAC SA Assignment TODO
*/

/*----------------------------------------------------------------------------*/
/*
    4.2.1 Test functionality of MIM Ether Type TODO
*/

/*----------------------------------------------------------------------------*/
/*
    4.2.2 Test functionality of MIM Mac2Me Restriction TODO
*/

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of MIM TTI key MAC DA or MAC SA:
    configure VLAN, FDB entries;
    configure MIM TTI key lookup SA MAC mode;
    configure TTI rule for MIM key;
    send MIM tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermUseMacSaInMimTtiLookup)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermUseMacSaInMimTtiLookupConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermUseMacSaInMimTtiLookupTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermUseMacSaInMimTtiLookupConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of masked fields in the MIM TTI key:
    configure VLAN, FDB entries;
    configure MIM TTI key lookup DA MAC mode;
    configure TTI rule for MIM key (with masked MAC);
    send MIM tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermMaskMacInMimTtiLookup)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermMaskMacInMimTtiLookupConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermMaskMacInMimTtiLookupTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermMaskMacInMimTtiLookupConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of Ethernet TTI key type:
    configure VLAN, FDB entries;
    configure TTI rule with Ether key;
    send tunneled traffic to match TTI rule;
    verify to get no traffic -- dropped;
    invalidate TTI rule;
    send tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
    validate TTI rule;
    send tunneled traffic to match TTI rule;
    verify to get no traffic -- dropped;
    send tunneled traffic to NOT match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherType)
{
/*
    1. Set TTI Ether Type Key Base configuration
    2. Generate traffic -- check traffic is blocked
    3. Invalidate Rule
    4. Generate traffic -- check traffic is not blocked
    5. Validate rule
    6. Generate traffic -- check traffic is blocked
    7. Generate traffic on different MAC DA -- check traffic is not blocked
    8. Restore configuration
*/

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Ether Type Key Base configuration */
    prvTgfTunnelTermEtherTypeBaseConfigurationSet();

    /* Generate traffic -- check no traffic */
    prvTgfTunnelTermEtherTypeTrafficGenerateExpectNoTraffic();

    /* invalidate TTI rules */
    prvTgfTunnelTermEtherTypeRuleValidStatusSet(GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermEtherTypeTrafficGenerate();

    /* validate TTI rules */
    prvTgfTunnelTermEtherTypeRuleValidStatusSet(GT_TRUE);

   /* Generate traffic -- check no traffic */
    prvTgfTunnelTermEtherTypeTrafficGenerateExpectNoTraffic();

    /* Generate traffic on different MAC DA -- check traffic is not blocked*/
    prvTgfTunnelTermEtherTypeDifferentTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermEtherTypeConfigurationRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of the Tag1 VLAN command field in TTI action entry:
    configure VLAN, FDB entries;
    configure Ethernet TTI rule to match traffic with specific VLANs and DA;
    configure TTI action to modify tags;
    send double tagged IPv4 traffic to match TTI rule;
    verify double tagged traffic with updated tags on nextHop port.
    for E-ARCH devices:
    send double tagged Ethernet traffic to match TTI rule;
    verify double tagged traffic with updated tags on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermTag1CmdField)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermTag1VlanCommandFieldConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermTag1VlanCommandFieldTrafficGenerate(PRV_TGF_TTI_KEY_IPV4_E);

    /* Restore configuration */
    prvTgfTunnelTermTag1VlanCommandFieldConfigRestore();

    prvTgfPclRestore();

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Set Base configuration with UDBs, according to bug CPSS-4230 */
        prvTgfTunnelTermTag1VlanCommandUdbConfigurationSet();

        /* Generate traffic */
        prvTgfTunnelTermTag1VlanCommandFieldTrafficGenerate(PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E);

        /* Restore configuration */
        prvTgfTunnelTermTag1VlanCommandUdbConfigRestore();

        prvTgfPclRestore();
    }
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of trust EXP QoS field in TTI action entry:
    configure VLAN, FDB entries;
    configure MPLS TTI rule to match MPLS traffic with 2 labels;
    configure TTI action to terminate tunnel and set trust EXP field to true;
    configure EXPtoQoS mapping entry with EXP matching EXP of outer MPLS label;
    send QinQ traffic to match TTI rule;
    verify traffic with updated EXP QoS field on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermExpQosField)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermTrustExpQosFieldConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermTrustExpQosFieldTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermTrustExpQosFieldConfigRestore();

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelTermExpQosField_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfTunnelTermExpQosField));
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of trust EXP QoS field in TTI action entry:
    configure VLAN, FDB entries;
    configure port default QoS profile;
    configure MPLS TTI rule to match MPLS traffic with 3 labels;
    configure TTI action to pop 2 MPLS command;
    configure EXP to QoS mapping entry;
    send QinQ traffic to match TTI rule;
    verify traffic with 1 MPLS label and updated EXP QoS field on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelExpQosFieldNonTerm)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    /* Changing QOS params is done only in Extended TTI Action -
       we do not support this action in utf tests.
       Also this test use ruleAction.passengerPacketType = PRV_TGF_TTI_PASSENGER_MPLS_E
       that is not supported in PUMA */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTrustExpQosFieldNonTermConfigSet();

    /* Generate traffic */
    prvTgfTunnelTrustExpQosFieldNonTermTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTrustExpQosFieldNonTermConfigRestore();

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelExpQosFieldNonTerm_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfTunnelExpQosFieldNonTerm));
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of trust EXP QoS field in TTI action entry:
    configure VLAN, FDB entries;
    configure port default QoS profile;
    configure MPLS TTI rule to match MPLS traffic with 3 labels;
    configure TTI action to pop 3 MPLS command;
    configure EXP to QoS mapping entry;
    send QinQ traffic to match TTI rule;
    verify traffic with 0 MPLS label and updated EXP QoS field on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelExpQosFieldNonTermPop3)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Changing QOS params is done only in Extended TTI Action -
       we do not support this action in utf tests.
       Also this test use ruleAction.passengerPacketType = PRV_TGF_TTI_PASSENGER_MPLS_E
       that is not supported in PUMA */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTrustExpQosFieldNonTermPop3ConfigSet();

    /* Generate traffic */
    prvTgfTunnelTrustExpQosFieldNonTermPop3TrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTrustExpQosFieldNonTermConfigRestore();

    prvTgfPclRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of QoS profile field in TTI action entry:
    configure VLAN, FDB entries;
    configure port default QoS profile;
    configure IPv4 TTI rule to match IPv4 tunneled packets;
    configure TTI action not to trust any QoS fields;
    configure EXP to QoS mapping entry;
    send IPv4 over IPv4 traffic to match TTI rule;
    verify IPv4 traffic with 1 MPLS label and updated QoS fields on nextHop port;
    and QoS fields matching EXP field in last MPLS label that was not popped.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermTrustAnyQosField)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    /* Changing QOS params is done only in Extended TTI Action -
       we do not support this action in utf tests */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);


    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermTrustAnyQosFieldConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermTrustAnyQosFieldTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermTrustAnyQosFieldConfigRestore();

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelTermTrustAnyQosField_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfTunnelTermTrustAnyQosField));
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of Trust DSCP QoS Profile Field:
    configure VLAN, FDB entries;
    configure port default QoS profile;
    configure IPv4 TTI rule to match IPv4 tunneled packets;
    configure TTI action not to to trust DSCP;
    configure DSCP to QoS entry with DSCP matching DSCP of passenger packet;
    send IPv4 over IPv4 traffic to match TTI rule;
    verify IPv4 traffic with updated DSCP on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermTrustDscpQosField)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    /* Changing QOS params is done only in Extended TTI Action -
       we do not support this action in utf tests */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);


    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermTrustDscpQosFieldConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermTrustDscpQosFieldTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermTrustDscpQosFieldConfigRestore();

    prvTgfPclRestore();
}

/*
    this test removed because:

    test fail because :
    The simulation supports <trustQosMappingTableIndex> in the TTI action table , but the CPSS not.
    This cause to tests that do 'trust L2/L3' with TunnelTerminated to not be OK
    to the 'loop on trustQosMappingTableIndex'

UTF_TEST_CASE_MAC(tgfTunnelTermTrustDscpQosField_qosMappingTableIterator)
{
    * test with qos mapping table iterator  *
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfTunnelTermTrustDscpQosField));
}
*/

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of Trust UP QoS Profile Field:
    configure VLAN, FDB entries;
    configure port default QoS profile;
    configure IPv4 TTI rule to match IPv4 tunneled packets;
    configure TTI action not to to trust UP;
    configure UP to QoS entry with UP matching UP of passenger packet;
    send IPv4 over IPv4 traffic to match TTI rule;
    verify IPv4 traffic with updated UP on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermTrustUpQosField)
{
/*
    1. Set Base configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    /* Changing QOS params is done only in Extended TTI Action -
       we do not support this action in utf tests */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);


    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermTrustUpQosFieldConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermTrustUpQosFieldTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermTrustUpQosFieldConfigRestore();

    prvTgfPclRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelTermTrustUpQosField_qosMappingTableIterator)
{
    /* test with qos mapping table iterator  */
    qosMappingTableIterator(UTF_TEST_NAME_MAC(tgfTunnelTermTrustUpQosField));
}

/*----------------------------------------------------------------------------*/
/*
    4.4.7 Test functionality of Other TTI Action Type 2 Fields TODO
*/

/*----------------------------------------------------------------------------*/
/*
    4.5.1 Test functionality of MPLS Illegal TTL Exception TODO
*/

/*----------------------------------------------------------------------------*/
/*
    4.5.2 Test functionality of MPLS Unsupported Exception TODO
*/

/*----------------------------------------------------------------------------*/

static void  globalPort2LocalPortConf(IN GT_U32    testNum)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration3
    4. Generate traffic
    5. Restore configuration
*/

    int i,j;
    int nextHopeIndex = 3;
    GT_U32 *ttiMultiPortGroupLocalPort2Array; /* pointer to array */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck() ||
       !PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* choose port list accoring to the mode */
    ttiMultiPortGroupLocalPort2Array = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ?
                                    prvTgfTtiMultiPortGroupLocalPort2_XLG_Array :
                                    prvTgfTtiMultiPortGroupLocalPort2Array;
    SAVE_ORIG_PORTS_MAC;

    /******************** test for ports 2 18 34 *********************/
    prvTgfPortsNum = (GT_U8)(PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.numOfPortGroups);
    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
        COPY_ARRAY_MAC(prvTgfPortsArray,GT_U32,ttiMultiPortGroupLocalPort2Array,prvTgfPortsNum);
        prvTgfPortsArray[nextHopeIndex] = PRV_TGF_NEXTHOPE_PORT_NUM_CNS;
    }

    utfGeneralStateMessageSave(0,"part 1 of test ");

    /* Set Base configuration */
    prvTgfTunnelTermGlobalPort2LocalPortBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelTermGlobalPort2LocalPortRouteConfigurationSet();

    for (i=0; i < (prvTgfPortsNum - 1); i++)
    {
        /* Set TTI configuration */
        prvTgfTunnelTermGlobalPort2LocalPortTtiConfigurationSet(testNum,i);

        /* Generate traffic */
        for (j=0; j < (prvTgfPortsNum - 1); j++)
        {
            if(j == nextHopeIndex)
            {
                continue;
            }
            prvTgfTunnelTermGlobalPort2LocalPortTrafficGenerate(testNum,i,j);
        }
    }

    /* Restore configuration */
    prvTgfTunnelTermGlobalPort2LocalPortConfigurationRestore();

    utfGeneralStateMessageSave(0,"part 2 of test ");

    /******************** test for ports 18 34 50*********************/
    COPY_ARRAY_MAC(prvTgfPortsArray,GT_U32,&ttiMultiPortGroupLocalPort2Array[1],prvTgfPortsNum-1);
    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
        prvTgfPortsArray[prvTgfPortsNum-1] = ttiMultiPortGroupLocalPort2Array[0];
        prvTgfPortsArray[nextHopeIndex] = PRV_TGF_NEXTHOPE_PORT_NUM_CNS;
    }

    /* Set Base configuration */
    prvTgfTunnelTermGlobalPort2LocalPortBaseConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelTermGlobalPort2LocalPortRouteConfigurationSet();

    for (i=1; i < prvTgfPortsNum; i++)
    {
        /* Set TTI configuration */
        prvTgfTunnelTermGlobalPort2LocalPortTtiConfigurationSet(testNum,i);

        /* Generate traffic */
        for (j=1; j < prvTgfPortsNum; j++)
        {
            if(j == (nextHopeIndex + 1))
            {
                continue;
            }

            prvTgfTunnelTermGlobalPort2LocalPortTrafficGenerate(testNum,i,j);
        }
    }

    /* Restore configuration */
    prvTgfTunnelTermGlobalPort2LocalPortConfigurationRestore();

    RESTORE_ORIG_PORTS_MAC;

    prvTgfPclRestore();
}
/*
    4.6.1 Test tunnel termination IPv4 over IPv4 to All Port Groups
*/
UTF_TEST_CASE_MAC(tgfTunnelTermGlobalPort2LocalPortConf1)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_PUMA_E);

    globalPort2LocalPortConf(1);/* 'test 1' */
}

/*----------------------------------------------------------------------------*/
/*
    4.6.2 Test tunnel termination IPv4 over IPv4 to a single Port Group
*/
UTF_TEST_CASE_MAC(tgfTunnelTermGlobalPort2LocalPortConf2)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_PUMA_E);

    globalPort2LocalPortConf(2); /* 'test 2' */
}

/*----------------------------------------------------------------------------*/
/*
    4.6.3 Test tunnel termination IPv4 over IPv4 to specific Port Groups
*/
UTF_TEST_CASE_MAC(tgfTunnelTermGlobalPort2LocalPortConf3)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_PUMA_E);

    globalPort2LocalPortConf(3); /* 'test 3' */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination ethernet over IPv4 redirect to egress:
    configure VLAN, FDB entries;
    configure IPv4 TTI rule to match IPv4 tunneled packets;
    configure TTI action to redirect to egress;
    send Ethernet over IPv4 traffic to match TTI rule;
    verify Ethernet traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverIpv4RedirectToEgress)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    /* check xcat and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* verify this is not xCat A1 / xCat A2 */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) && (GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressBridgeConfigSet();

    /* Set Base Inlif configuration */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressInilifConfigurationSet();

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressTtiConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination ethernet over IPv4 redirect to egress ePort:
    configure VLAN, FDB entries;
    map ePort to physical port;
    configure IPv4 TTI rule to match IPv4 tunneled packets;
    configure TTI action to redirect to egress;
    send Ethernet over IPv4 traffic to match TTI rule;
    verify Ethernet traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverIpv4RedirectToEgressEport)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* verify this is not xCat A1 / xCat A2 */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) && (GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportBridgeConfigSet();

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTtiConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination ethernet over IPv4 GRE redirect to egress:
    configure VLAN with non tagged ports, FDB entries;
    configure GRE ethernet type 1 and 2;
    configure IPv4 TTI rule to match IPv4 tunneled packets;
    configure TTI action to redirect to egress;
    send Ethernet over IPv4 GRE traffic to match TTI rule;
    verify non tagged Ethernet traffic on nextHop port;
    configure VLAN with tagged ports;
    send Ethernet over IPv4 GRE traffic to match TTI rule;
    verify tagged Ethernet traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverIpv4GreRedirectToEgress)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    /* check xcat and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* verify this is not xCat A1 / xCat A2 */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) && (GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Build Packet */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet(0);

    /* Set Bridge configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBridgeConfigSet(GT_FALSE);

    /* Set Base Inlif configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressInilifConfigurationSet();

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet(0, GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate(0);

    /* Change all ports to be tagged */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBridgeConfigSet(GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate(0);

    /* Restore configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Scenario description:
         Packet received as IP-Ethernet-over-IPv4-GRE tunnel
         Packet is TT
         Passenger Ethernet packet MAC DA is Router MAC address
         Triggers IPv4 UC router, Nexthop entry has TS Pointer, and TS Type is IP
         TS entry is IPv4-GRE
         Packet is egressed as IPv4-over-GRE-IPv4
*/
UTF_TEST_CASE_MAC(tgfTunnelIpEtherOverIpv4Gre)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

     /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Build Packet */
    prvTgfTunnelIpEtherOverIpv4GreBuildPacketSet();

    /* Set Bridge configuration */
    prvTgfTunnelIpEtherOverIpv4GreBridgeConfigSet();

    /* Set TTI configuration */
    prvTgfTunnelIpEtherOverIpv4GreTtiConfigSet();

    /* Set Tunnel Start configuration */
    prvTgfTunnelIpEtherOverIpv4GreIpv4TunnelStartConfigurationSet();

     /* Set Route configuration */
    prvTgfTunnelIpEtherOverIpv4GreLttRouteConfigurationSet(prvUtfVrfId);

    /* Generate traffic */
    prvTgfTunnelIpEtherOverIpv4GreTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelIpEtherOverIpv4GreConfigurationRestore(prvUtfVrfId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination ethernet over IPv4 GRE redirect to egress:
    configure VLAN with tagged ports, FDB entries;
    configure GRE ethernet type 1 and 2;
    configure IPv4 TTI rule to match IPv4 tunneled packets;
    configure TTI action to redirect to egress;
    send Ethernet over IPv4 GRE traffic to match TTI rule;
    verify tagged Ethernet traffic on nextHop port;
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverIpv4GreRedirectToEgressTtHeaderLength)
{
/*
    AUTODOC: use existing basic test tgfTunnelTermEtherOverIpv4GreRedirectToEgress
    part 1:
        define in tti action:
        -   ttHeaderLength = 24; which is the size of the IPv4 GRE header.
        tunnel terminate according to tt header length and redirect the packet to egress port.
    part 2:
        define in tti action:
        -   ttHeaderLength = 24 + 4; which is the size of the IPv4 GRE header
                            + 4 zero bytes --> check parsing of non generic key.
        tunnel terminate according to tt header length and redirect the packet to egress port.
    part 3:
        define in tti action:
        -   ttHeaderLength = 24 + 6; which is the size of the IPv4 GRE header
                            + 6 zero bytes --> check parsing of non generic key;
                            check also length not devided in 4.
        tunnel terminate according to tt header length and redirect the packet to egress port.
    part 4:
        define in tti action:
        -   ttHeaderLength = 24 + 6; which is the size of the IPv4 GRE header
                            + 6 zero bytes --> check parsing of non generic key;
                            check also length not devided in 4.
        - passengerParsingOfTransitNonMplsTransitTunnelEnable = GT_TRUE;
                    If enabled, the packet parsing is based on passenger packet,
                    where the passenger packet type is defined in the TTI Action
                    <Tunnel Passenger Type> and tunnel header length is defined in the
                    TTI Action <Tunnel-termination Header Length>
        - tunnelTerminate = GT_FALSE; This configuration is relevant only
                    if TTI Action <Tunnel Terminate> = Disabled
        - ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E
        transit according to tt header length and redirect the packet to egress port;
        no tunnel termination.
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Build Packet */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet(0);

    /* Set Bridge configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBridgeConfigSet(GT_TRUE);

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet(24, GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate(0);

    /* Build Packet */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet(1);

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet(24 + 4, GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate(1);

    /* Build Packet */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet(2);

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet(24 + 6, GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate(2);

    /* Build Packet */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressBuildPacketSet(3);

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTtiConfigSet(24 + 6, GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressTrafficGenerate(3);

    /* Restore configuration */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination ethernet over IPv4 no redirect + bridging:
    configure VLAN with non tagged ports, FDB entries;
    configure GRE ethernet type 0;
    configure IPv4 TTI rule to match IPv4 tunneled packets and forward action;
    send Ethernet over IPv4 GRE traffic to match TTI rule;
    verify tagged passenger traffic is forwarded on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverIpv4NoRedirectBridging)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    /* check xcat and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* verify this is not xCat A1 / xCat A2 */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) && (GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingBridgeConfigSet();

    /* Set Base Inlif configuration */
    prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingInilifConfigSet();

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingTtiConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermEtherOverIpv4NoRedirectBridgingConfigurationRestore();
}

static GT_BOOL retainCrc = GT_FALSE;

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start ethernet over IPv4 redirect to egress:
    configure VLAN, FDB entries;
    configure X_OVER_GRE_IPV4 Tunnel Start entry;
    configure PCL rule to redirect tunneled traffic;
    send Ethernet traffic to match TS entry and PCL rule;
    verify to get Ethernet over IPv4 tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(internal_tgfTunnelStartEtherOverIpv4GreRedirectToEgress)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    /* check xcat and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* verify this is not xCat A1 / xCat A2 */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) && (GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSet();

    /* Set Tunnel configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSet(retainCrc);

    /* Set Pcl configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSet();

    /* Generate traffic */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerate(retainCrc);

    /* Restore configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgress)
{
    retainCrc = GT_FALSE;
    UTF_TEST_CALL_MAC(internal_tgfTunnelStartEtherOverIpv4GreRedirectToEgress);
}

UTF_TEST_CASE_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgress_DSATag)
{
    GT_U32      notAppFamilyBmp;
    /* Sip5 only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    retainCrc = GT_FALSE;
    /* enable substitution of VLAN tag with DSA tag cascade egress port */
    tgfTunnelStartTestAsDsaTagged(TGF_DSA_2_WORD_TYPE_E);
    UTF_TEST_CALL_MAC(internal_tgfTunnelStartEtherOverIpv4GreRedirectToEgress);

    /* enable substitution of VLAN tag with eDSA tag cascade egress port */
    tgfTunnelStartTestAsDsaTagged(TGF_DSA_4_WORD_TYPE_E);
    UTF_TEST_CALL_MAC(internal_tgfTunnelStartEtherOverIpv4GreRedirectToEgress);
}


UTF_TEST_CASE_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgress_retainCRC)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    retainCrc = GT_TRUE;
    UTF_TEST_CALL_MAC(internal_tgfTunnelStartEtherOverIpv4GreRedirectToEgress);
}

static GT_BOOL useDscpRemapOnTs = GT_FALSE;
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start ethernet over IPv4 redirect to egress ePort:
    configure VLAN, FDB entries;
    configure X_OVER_GRE_IPV4 Tunnel Start entry;
    configure PCL rule to redirect tunneled traffic;
    set ePort mapping configuration;
    send Ethernet traffic to match TS entry and PCL rule;
    verify to get Ethernet over IPv4 tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgressEport)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportBridgeConfigSet(useDscpRemapOnTs);

    /* Set Tunnel configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTunnelConfigSet();

    /* Set Pcl configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportPclConfigSet();

    /* Generate traffic */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressEportConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test based on tgfTunnelStartEtherOverIpv4GreRedirectToEgressEport
    meaning that ethernet packet is directed to TS of ipv4.

    this test add next :
    since the TS ipv4 define the DSCP that will be in the packet (TS_DSCP)
    on egress port set next config :
      Set <QOS Mapping Table Select> = 5
      Set <Enable Egress DSCP Mapping> = 1
    Set in table 5 of "QOS Mapping Table Select" mapping from DSCP TS_DSCP to DSCP NEW_DSCP

    --> check that packet egress with DSCP = NEW_DSCP and not TS_DSCP

*/
UTF_TEST_CASE_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgressEport_egressQosDscpRemark)
{
    useDscpRemapOnTs = GT_TRUE;
    UTF_TEST_CALL_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgressEport);
    useDscpRemapOnTs = GT_FALSE;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start ethernet over IPv4 redirect to egress (with offset):
    configure VLAN, FDB entries;
    configure length offset per egress port;
    configure X_OVER_GRE_IPV4 Tunnel Start entry;
    configure PCL rule to redirect tunneled traffic;
    send Ethernet traffic to match TS entry and PCL rule;
    verify to get Ether over IPv4 traffic with updated TotalLength on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgressOffset)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    /* check xcat and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* Changing total length is not supported in puma */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* verify this is not xCat A1 / xCat A2 */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) && (GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSetOffset();

    /* Set Tunnel configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSetOffset();

    /* Set Pcl configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSetOffset();

    /* Generate traffic */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerateOffset();

    /* Restore configuration */
    prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestoreOffset();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ethernet lookup and redirect to egress:
    configure VLAN, FDB entries;
    configure TTI lookup and MAC mode;
    configure IPv4 TTI rule to redirect traffic with specific DA;
    send Ethernet over IPv4 traffic to match TTI rule;
    verify Ethernet over IPv4 traffic redirected on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfEtherRedirectToEgress)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    /* check xcat and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfEtherRedirectToEgressBridgeConfigSet();

    /* Set Base Inlif configuration */
    prvTgfEtherRedirectToEgressInilifConfigSet();

    /* Set TTI configuration */
    prvTgfEtherRedirectToEgressTtiConfigSet();

    /* Generate traffic */
    prvTgfEtherRedirectToEgressTrafficGenerate();

    /* Restore configuration */
    prvTgfEtherRedirectToEgressConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ethernet lookup and redirect to egress with TS:
    configure VLAN, FDB entries;
    configure TTI lookup and MAC mode;
    configure IPv4 TTI rule to redirect traffic with specific DA;
    configure IPV4_OVER_IPV4 Tunnel Start entry;
    send Ethernet over IPv4 traffic to match TTI and TS rules;
    verify tunneled traffic redirected on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfEtherRedirectToEgressWithTS)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Set Tunnel configuration
    5. Generate traffic
    6. Restore configuration
*/

    /* check xcat and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E );

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* in xCat PBR mode there is no Tunnel Start*/
    if (prvUtfIsPbrModeUsed() &&
        ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* verify this is not xCat A1 / xCat A2 */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) && (GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfEtherRedirectToEgressWithTsBridgeConfigSet();

    /* Set Base Inlif configuration */
    prvTgfEtherRedirectToEgressWithTsInilifConfigSet();

    /* Set TTI configuration */
    prvTgfEtherRedirectToEgressWithTsTtiConfigSet();

    /* Set Tunnel configuration */
    prvTgfEtherRedirectToEgressWithTsTunnelConfigurationSet();

    /* Generate traffic */
    prvTgfEtherRedirectToEgressWithTsTrafficGenerate();

    /* Restore configuration */
    prvTgfEtherRedirectToEgressWithTsConfigurationRestore();


}

/* trill uts */
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test known unicast ethernet packet sent to TRILL tunnel:
    configure VLAN, FDB entries;
    configure ePort attributes and mapping;
    configure TRILL Tunnel Start entry;
    send Ethernet traffic to match TS entry;
    verify tunneled trill traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForward)
{
/*
    AUTODOC:
    Test Goal:
        Test known unicast ethernet packet sent to TRILL tunnel.
    Test scenario:
        Match native ethernet packet in the FDB.
        The FDB forward the packet to ePort.
        The ePort is configured to add tunnel start
        Tunnel start is configured with TRILL tunnel start.
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    CPSS_TBD_BOOKMARK_EARCH

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set configuration */
    prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardConfigSet();

    /* Generate traffic */
    prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForwardConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Unknown UC ethernet packet sent to TRILL and to local native ethernet ports:
    configure VLAN, FDB entries;
    map eVIDX to a pointer of linked-list of L2MLL entries;
    configure L2MLL entry with target VIDX that is multi-target physical port group;
    configure VIDX membership with set of local native Ethernet ports;
    configure additional L2MLL entry for each adjacency in Rbid tree;
    each L2MLL assigns a target ePort that is mapped to a TRILL interface;
    send Ethernet traffic to match TS entry;
    verify tunneled trill traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartTrillUnknownUcFlooding)
{
/*
    AUTODOC:
    Test Goal:
        Test Unknown unicast ethernet packet sent to TRILL tunnel and to local native ethernet ports.
    Test scenario:
        - Native Ethernet packet is processed by the Bridge engine. The MAC DA is unknown in the FDB: the packet is assigned a target
          eVIDX from the eVLAN table <Flood eVIDX>. The eVIDX is mapped to a pointer of linked-list of L2MLL entries.
        - One L2MLL entry is configured with a target VIDX that is a multi-target physical port group. The VIDX membership contains
          the set of local native Ethernet ports. The packet is transmitted as native Ethernet on each port in the VIDX group.
          As in normal bridging, the packet is filtered on the physical source port on which the packet was received.
        - Additional L2MLL entry is configured for each adjacency in Rbid distribution tree. Each L2MLL assigns a target ePort that is
          mapped to a TRILL interface. For each such L2MLL entry, the native Ethernet packet is encapsulated with a TRILL header and
          outer MAC header, and is transmitted on its egress port to the next hop RBridge in the distribution tree
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    CPSS_TBD_BOOKMARK_EARCH
    SKIP_TEST_MAC

    #if 0 /*trill tests are not operational*/
    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");
    /* Set configuration */
    prvTgfTunnelStartTrillUnknownUcFloodingConfigSet();

    /* Generate traffic */
    prvTgfTunnelStartTrillUnknownUcFloodingTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartTrillUnknownUcFloodingConfigRestore();
    #endif /*0*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test TRILL Known UC Transit Packet:
    configure VLAN, FDB entries;
    configure trill engine enable on transmit port;
    configure TRILL TTI rule to match all UC TRILL transit packets;
    send tunneled Trill traffic to match TTI rule;
    verify tunneled trill traffic with updated DA on nextHop port.
*/
UTF_TEST_CASE_MAC(prvTgfTunnelTrillKnownUcTransit)
{
    GT_U32      notAppFamilyBmp;
/*
    AUTODOC:
    Test Goal:
        Test TRILL Known UC Transit Packet.
    Test scenario:
        TRILL Transit packets are matched by the TTI and are routed without termination to ePort.
        The ePort forces TS only to replace the incoming outer MAC header.
*/

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    CPSS_TBD_BOOKMARK_EARCH
    SKIP_TEST_MAC

    #if 0 /*trill tests are not operational*/
    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");

    /* Set configuration */
    prvTgfTunnelTrillKnownUcTransitConfigSet();

    /* Generate traffic */
    prvTgfTunnelTrillKnownUcTransitTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTrillKnownUcTransmitConfigRestore();
    #endif /*0*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test TRILL Known UC Transit Packet:
    configure VLAN, FDB entries;
    configure trill engine enable on transmit port;
    configure TRILL TTI rule to match all TRILL UC-to-me packets;
    configure MAC to ME table;
    configure entry in TRILL Adjacency Check dedicated TCAM;
    send tunneled Trill traffic to match TTI rule;
    verify Ethernet traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(prvTgfTunnelTrillKnownUcDataToTti)
{
    GT_U32      notAppFamilyBmp;

/*
    AUTODOC:
    Test Goal:
        Test TRILL Known UC Data Frame Forwarding Behavior.
    Test scenario:
        1. TRILL UC data frame Identification
        2. IS-IS Adjacency Check
        3. check If Trill.HopCount == 0
        4. Decapsulate and bridge based on inner MAC header, send native packet to port
*/


    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    CPSS_TBD_BOOKMARK_EARCH
    SKIP_TEST_MAC

    #if 0 /*trill tests are not operational*/
    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");
    /* Set configuration */
    prvTgfTunnelTrillKnownUcDataToTtiConfigSet();

    /* Generate traffic */
    prvTgfTunnelTrillKnownUcDataToTtiAllTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTrillKnownUcDataToTtiConfigRestore();
    #endif /*0*/
}

UTF_TEST_CASE_MAC(tgfTunnelTrillKnownUcDataToTti_fromCascadePortDsaTagForward)
{
    GT_U32      notAppFamilyBmp;
/*
    AUTODOC:
    Test Goal:
        Test Sorce Eport assignment Flow.
    Test scenario:
        1. Define:
                -   ByPass Bridge == 0
                    Port n <Centralized Chassis Mode> == 1
                -   Port n < TRILL engine enable> == 0
           Transmit DSA packet with an 8B extended DSA tag in FORWARD format and check:
                -   The packet is processed by the Bridge engine. MAC DA is a known in the FDB:, than the Bridge FDB DA entry assigns a target ePort.
                -   SA learning is according to Default Source ePort assignment for Packets received on CC enabled port with DSA<SrcIsTrunk> = 0.

        2. Define:
                -   ByPass Bridge == 1
                -   Port n <Centralized Chassis Mode> == 0
           Transmit DSA packet with an 8B extended DSA tag in FORWARD format and check:
                -   The packet is assigned a target ePort from the 'DSA info'.
                -   SA learning is according to the 'DSA info' from 'remote device'.

        3. Run test tgfTunnelTrillKnownUcDataToTti which Defines:
                -   Port n < TRILL engine enable> == 1
           Transmit TRILL packet and check:
                -   The packet is assigned a target ePort from tti action.
                -   SA learning is according to source ePort assignmen from tti action.
*/

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    CPSS_TBD_BOOKMARK_EARCH
    SKIP_TEST_MAC

    #if 0 /*trill tests are not operational*/
    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");

    /* state that we run in DSA tag mode - bypass bridge*/
    prvTgfTunnelTrillKnownUcDataToTtiConfigurationDsaTagEnableSet(GT_TRUE/*use DSA*/,GT_FALSE/*bypass bridge*/);

    /* run the test - prvTgfTunnelTrillKnownUcDataToTti */
    UTF_TEST_CALL_MAC(prvTgfTunnelTrillKnownUcDataToTti);

    /* state that we run in DSA tag mode - bypass bridge*/
    prvTgfTunnelTrillKnownUcDataToTtiConfigurationDsaTagEnableSet(GT_TRUE/*use DSA*/,GT_TRUE/*bypass bridge*/);

    /* run the test - prvTgfTunnelTrillKnownUcDataToTti */
    UTF_TEST_CALL_MAC(prvTgfTunnelTrillKnownUcDataToTti);

    /* restore values */
    prvTgfTunnelTrillKnownUcDataToTtiConfigurationDsaTagEnableSet(GT_FALSE/*use DSA*/,GT_FALSE/*bypass bridge*/);
    #endif /*0*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test All-Rbridges MC TRILL tunnel packet functionality:
    configure VLAN, FDB entries;
    configure TTI rule to match TRILL Transit packets;
    configure TTI action to assign a target eVIDX;
    map eVIDX to pointer of linked-list of L2MLL entries;
    configure LL of L2MLL to forward multi-destination TRILL transit traffic;
    configure L2MLL entry for each adjacency in Rbid distribution tree;
    each L2MLL assigns a target ePort that is mapped to a TRILL interface;
    configure TTI rule to match TRILL to-me packets;
    configure I-RBID lookup to assign src ePort that associated with MAC SA;
    maps eVIDX to pointer of linked-list of L2MLL entries;
    configure L2MLL entry with a trg VIDX that is multi-target phy port group;
    send tunneled Trill traffic to match TTI rules;
    verify tunneled Trill traffic with updated MAC and hopCount on nextHop port.
*/
UTF_TEST_CASE_MAC(prvTgfTunnelTermTrillUnknownUcTransitAndTermination)
{
/*
    AUTODOC:
    Test Goal:
        Test All-Rbridges multicast packet sent to TRILL tunnel and to local native ethernet ports.
    Test scenario:
        - Multi-destination TRILL packets are internally duplicated into two separate and independent packet instances.
          The first instance (instance #0) is treated as a multi-destination TRILL "Transit" packet, and the second instance
          (instance #1) is treated as a TRILL "to-ME" packet.
        - TRILL Transit packets are matched by the TTI and assigned a target eVIDX by the TTI action. The eVIDX is mapped
          to a pointer of linked-list of L2MLL entries. The linked list of L2MLL entries used for forward multi-destination
          TRILL transit traffic; L2MLL entry is configured for each adjacency in Rbid distribution tree. Each L2MLL assigns a target ePort
          that is mapped to a TRILL interface. For each such L2MLL entry, the TRILL header <Hop Count> is decremented by 1 and the outer
          MAC header is stripped and rebuilt based on the target ePort <Tunnel Pointer>. Then it is transmitted on its egress port to
          the next hop RBridge in the distribution tree.
        - TRILL to-me packets are matched by the TTI, decapsulated and the inner Ethernet packet is processed by the Bridge engine.
          The inner MAC SA is learned by the Bridge. The I-RBID lookup assigns the source ePort that is associated with MAC SA.
          The passenger MAC DA is unknown in the FDB: the packet is assigned a target eVIDX from the eVLAN table <Flood eVIDX>.
          The eVIDX is mapped to a pointer of linked-list of L2MLL entries. L2MLL entry is configured with a target VIDX that
          is a multi-target physical port group. The VIDX membership contains the set of local native Ethernet ports. The packet is transmitted
          as native Ethernet on each port in the VIDX group. As in normal bridging, the packet is filtered on the physical source port
          on which the packet was received.
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    CPSS_TBD_BOOKMARK_EARCH
    SKIP_TEST_MAC

    #if 0 /*trill tests are not operational*/
    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");
    /* Set configuration */
    prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermTrillUnknownUcTransitAndTerminationTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTermTrillUnknownUcTransitAndTerminationConfigRestore();
    #endif /*0*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test known unicast ethernet packet sent to ECMP TRILL tunnel:
    configure VLAN, FDB entries;
    configure ePorts attributes and mapping;
    each secondary ePort is mapped to a different physical port;
    configure 3 TRILL TS entries for each ePorts with different nextHop DA;
    configure L2ECMP engine to map global ePort to the set of target ePort paths;
    send traffic to match TS entries;
    verify tunneled Trill traffic with updated MAC;
    configure ePorts attributes and mapping;
    each secondary ePorts are mapped to the same physical port;
    send traffic to match TS entries;
    verify tunneled Trill traffic with updated MAC.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartTrillKnownUcEcmp)
{
    GT_U32      notAppFamilyBmp;

/*
    AUTODOC:
    Test Goal:
        Test known unicast ethernet packet sent to ECMP TRILL tunnel.
    Test scenario:
        - Native Ethernet packet is processed by the Bridge engine. The Bridge FDB DA entry assigns the target global ePort for the L2ECMP group.
          The L2ECMP engine maps the global ePort to the set of target ePort paths. The packet hash value is then used to select one of the target paths to be used by this packet.
          Each of the ePorts in the L2ECMP group assigns a different <Tunnel-start pointer>. Each Tunnel-start entry has a different next hop MAC DA but has the same TRILL header fields
         (TRILL header is independent of the patch taken).
        - In test 1: Each secondary ePort is mapped to a different physical port.
        - In test 2: 2 secondary ePorts are mapped to the same physical port.
*/

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    CPSS_TBD_BOOKMARK_EARCH
    SKIP_TEST_MAC

    #if 0 /*trill tests are not operational*/
    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");

    /* Set configuration */
    prvTgfTunneStartlTrillKnownUcEcmpConfigSet();

    /* Generate traffic */
    prvTgfTunneStartlTrillKnownUcEcmpTrafficGenerate();

    /* Set configuration */
    prvTgfTunneStartlTrillKnownUcEcmpConfig2Set();

    /* Generate traffic */
    prvTgfTunneStartlTrillKnownUcEcmpTraffic2Generate();

    /* Restore configuration */
    prvTgfTunneStartlTrillKnownUcEcmpRestore();
    #endif /*0*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test known MC ethernet packet sent to TRILL tunnel and to local native ethernet ports:
    configure VLAN, FDB entries;
    MAC DA is a registered Multicast;
    configure L2MLL MLL table partitioning mode;
    create L2 MLL LTT entries;
    configure eVidx mapping to pointer of linked-list of L2MLL entries with value > 4K;
    configure L2MLL entry with trg VIDX that is a multi-target physical port group;
    VIDX contains the set of local native Ethernet ports;
    send traffic as native Ethernet on each port in the VIDX group;
    verify traffic is filtered on the physical src port on which was received;
    configure L2MLL entry for each adjacency in distribution tree;
    each L2MLL assigns a trg ePort that is mapped to a TRILL interface;
    send traffic as native Ethernet packet;
    verify tunneled TRILL traffic and outer MAC header on egress port
    to nextHop RBridge in the distribution tree.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartTrillKnownMc)
{
/*
    AUTODOC:
    Test Goal:
        Test known MULTICAST ethernet packet sent to to TRILL tunnel and to local native ethernet ports.
    Test scenario:
        - Native Ethernet packet is processed by the Bridge engine. MAC DA is a registered Multicast, than the Bridge FDB DA entry assigns a target eVIDX. The eVIDX is mapped to
          a pointer of linked-list of L2MLL entries with value > 4K (indicating L2MLL duplication list).
        - One L2MLL entry is configured with a target VIDX that is a multi-target physical port group. The VIDX membership contains the set of local native Ethernet ports.
          The packet is transmitted as native Ethernet on each port in the VIDX group. As in normal bridging, the packet is filtered on the physical source port on which the packet was received.
        - Additional L2MLL entry is configured for each adjacency in distribution tree. Each L2MLL assigns a target ePort that is mapped to a TRILL interface. For each such L2MLL entry,
          the native Ethernet packet is encapsulated with a TRILL header and outer MAC header, and is transmitted on its egress port to the next hop RBridge in the distribution tree.

*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    CPSS_TBD_BOOKMARK_EARCH
    SKIP_TEST_MAC

    #if 0 /*trill tests are not operational*/
    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(1,0,"trill tests are not operational \n");

    /* Set configuration */
    prvTgfTunnelStartTrillKnownMcConfigSet();

    /* Generate traffic */
    prvTgfTunnelStartTrillKnownMcTrafficGenerate();

    /* Restore configuration */
    prvTgfTunneStartlTrillKnownMcRestore();
    #endif /*0*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test all TRILL exceptions:
    configure VLAN, FDB entries;
    configure TTI rule to match all UC TRILL transit packets;
    used any index greater than that used for Egress RBridge rules;
    set ePort attributes and mapping configuration;
    1. TRILL Adjacency Exception Checking:
    there are 256 entries in the Adjacency Check TCAM;
    if Adjacency TCAM lookup does not find a matching entry;
    this indicates the packet was received from invalid adjacency and one
    of the respective exception commands is assigned to the packet.
    2. TRILL Packet General Exception Checking;
    TRILL engine parses outer MAC header, TRILL header, and inner MAC header
    and performs various packet header checks;
    3. TRILL RBID Lookup Exception Checking;
    RBID Lookup validates TRILL header I-RBID and E-RBID, and assigns
    source ePort and target ePort accordingly.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermTrillExceptionsCheck)
{
/*
    AUTODOC:
    Test Goal:
        Test all TRILL exceptions by sending known MULTICAST ethernet packet and known UNICAST ethernet packet to to TRILL tunnel.
    Test scenario:
        - TRILL Adjacency Exception Checking: There are 256 entries in the Adjacency Check TCAM. If Adjacency TCAM lookup does not find a matching entry;
          this indicates the packet was received from an invalid adjacency and one of the respective exception commands is assigned to the packet.
        - TRILL Packet General Exception Checking: The TRILL engine parses the outer MAC header, the TRILL header, and the inner MAC header, and performs various packet header checks.
        - TRILL RBID Lookup Exception Checking: The RBID Lookup validates the TRILL header I-RBID and E-RBID, and assigns a source ePort and target ePort accordingly.
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    CPSS_TBD_BOOKMARK_EARCH

    /* Skip test on GM simulation. Need to fix JIRA CPSS-6588 */
    if(GT_TRUE == prvUtfIsGmCompilation())
        SKIP_TEST_MAC;

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set configuration */
    prvTgfTunnelTrillTtiExceptionsCheckConfigSet();

    /* Generate traffic */
    prvTgfTunnelTrillTtiExceptionsCheckTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelTrillTtiExceptionsCheckConfigRestore();
}
/* trill uts */

/*----------------------------------------------------------------------------*/
/* AUTODOC: Tunnel Termination of Eth-o-IPv4/GRE With MACSec
            Goals:
            - The purpose of this test is to verify the correct tunnel removal of an Eth-o-IPv4/GRE tunneled packet.
            - Checking the artificial decrement of the Total Length field in the IPv4/GRE Header of the tunnel with a constant value.
            Scenario:
            - take basic test of TTI Eth-o-IPv4 GRE and define:
            - Enable IpTunnelTotalLengthDeductionEnable for Transmit port.
            - Set IpTotalLengthDeductionValue = 24 bytes
            - Send Ethernet-over-IPv4 GRE packet with 'total length' field in IPv4 GRE header + IpTotalLengthDeductionValue
            Expected results:
            - the packet reached Recive port (no IPv4 TTI Header Error)

            Tunnel Termination of Eth-o-IPv4/GRE with IPv4 TTI Header Error
            Goals:
            - The purpose of this test is to verify The IPv4 TTI Exception check in case of wrong total length in the IP tunnel header
            Scenario:
            - take basic test of TTI Eth-o-IPv4 GRE and define:
            - Disable IpTunnelTotalLengthDeductionEnable for Transmit port.
            - Set IpTotalLengthDeductionValue = 24 bytes
            - Send Ethernet-over-IPv4 GRE packet with 'total length' field in IPv4 GRE header + IpTotalLengthDeductionValue.
            Expected results:
            - Get exception - the packets will be trap to cpu
             IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) - MACSec Value to deduce <= MAC layer packet byte count
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverIpv4GreMacSecRedirectToEgress)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* check if the device supports the tti */
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* set configuration for MACSec */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressMacSecConfigSet(GT_TRUE);

    /* run test tgfTunnelTermEtherOverIpv4GreRedirectToEgress */
    UTF_TEST_CALL_MAC(tgfTunnelTermEtherOverIpv4GreRedirectToEgress);

    /* remove confiuration for MACSec */
    prvTgfTunnelTermEtherOverIpv4GreRedirectToEgressMacSecConfigSet(GT_FALSE);

    /* run test tgfTunnelTermEtherOverIpv4GreRedirectToEgress */
    UTF_TEST_CALL_MAC(tgfTunnelTermEtherOverIpv4GreRedirectToEgress);
}




/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start Generic IPv6  with UDP header, 16 template bytes,IPv4 passenger:
    configure VLAN, FDB entries;
    configure TS entry with GRE ethType;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartGenericIpv4UdpTemplate)
{
    prvTgfTunnelStartGenericIpv4TemplateTest(PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E,0/*don't care in this case*/);
    utfExtraTestEnded();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start Generic IPv6  with UDP header, 16 template bytes,IPv4 passenger:
    configure VLAN, FDB entries;
    configure TS entry with GRE ethType;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartGenericIpv4GenericIpHeaderTemplate)
{
    /*This test is for BobK,Aldrin and BC3*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~ (UTF_FALCON_E|UTF_ALDRIN2_E|UTF_BOBCAT3_E|UTF_ALDRIN_E|UTF_CAELUM_E)));

    prvTgfTunnelStartGenericIpv4TemplateTest(PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E,0x8F);
    utfExtraTestEnded();
}




/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start Generic IPv6  with UDP header, 16 template bytes,IPv4 passenger:
    configure VLAN, FDB entries;
    configure TS entry with GRE ethType;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartGenericIpv6UdpTemplate)
{
    prvTgfTunnelStartGenericIpv6TemplateTest(PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E,0/*don't care in this case*/);
    utfExtraTestEnded();


}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel start Generic IPv6  with UDP header, 16 template bytes,IPv4 passenger:
    configure VLAN, FDB entries;
    configure TS entry with GRE ethType;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartGenericIpv6GenericIpHeaderTemplate)
{
    /*This test is for BobK,Aldrin and BC3*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~ (UTF_FALCON_E|UTF_ALDRIN2_E|UTF_BOBCAT3_E|UTF_ALDRIN_E|UTF_CAELUM_E)));

    prvTgfTunnelStartGenericIpv6TemplateTest(PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E,0x9F);
    utfExtraTestEnded();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination IPv4 over IPv4 GRE with GRE extensions:
    configure VLAN, FDB entries;
    configure TTI rule;
    enable GRE extensions, set the IPv4 GRE extensions command to hard drop;
    send tunneled traffic to match TTI rule;
    verify passenger traffic is dropped;
    disable GRE extensions;
    send tunneled traffic to match TTI rule;
    verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermGreExtensions)
{
    GT_U32 prvUtfVrfId = 0;
    GT_U32 notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    /* basic test */
    prvTgfTunnelTermIpv4overGreIpv4BaseConfigurationSet(prvUtfVrfId, 0);

    /* Set Route configuration */
    prvTgfTunnelTermIpv4overGreIpv4RouteConfigurationSet();

    /* Set TTI configuration */
    /* basic test */
    prvTgfTunnelTermIpv4overGreIpv4TtiConfigurationSet(0);

    /* Disable GRE exceptions, and since we send GRE header causing exception we expect no drops */
    prvTgfTunnelTermIpv4overGreIpv4GreExtensionsSet(GT_FALSE, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic - check that traffic is passed*/
    prvTgfTunnelTermIpv4overGreIpv4TrafficGenerate();

    /* Enable GRE exceptions, and since we send GRE header causing exception we expect HARD DROP */
    prvTgfTunnelTermIpv4overGreIpv4GreExtensionsSet(GT_TRUE, CPSS_PACKET_CMD_DROP_HARD_E);

    /* Generate traffic - check that traffic is dropped*/
    prvTgfTunnelTermIpv4overGreIpv4TrafficGenerateExpectNoTraffic();

    /* Restore configuration */
    prvTgfTunnelTermIpv4overGreIpv4ConfigurationRestore(GT_FALSE);

    prvTgfPclRestore();
}

/*do we test packets that ingress with DSA tag*/
static GT_BOOL ingressIsCascadePort = GT_FALSE;
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination Ethernet over Mpls:
    1. configure VLAN, FDB entries;
       configure TTI rule;
    2. configure TTI 'old key' Mpls;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
    3. configure TTI UDB key Mpls other;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverMplsRedirectToEgress)
{
/*

    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Build Packet */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet();

    /* Set Base configuration */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet(); /* basic test */

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressOldKeyTtiConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_TRUE);

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) && (ingressIsCascadePort == GT_FALSE))
    {
        /* Enable MPLS lookup only if there is a MAC to Me entry */
        prvTgfTunnelTermEthernetOverMplsMacToMeEnableSet(GT_TRUE);

        /* Generate traffic - expect no traffic */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_FALSE);

        /* Add MAC to Me entry */
        prvTgfTunnelTermEthernetOverMplsMacToMeSet();

        /* Generate traffic */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_TRUE);

        /* Disable MPLS lookup only if there is a MAC to Me entry */
        prvTgfTunnelTermEthernetOverMplsMacToMeEnableSet(GT_FALSE);

        /* Remove MAC to Me entry */
        prvTgfTunnelTermEthernetOverMplsMacToMeDel();

        /* Generate traffic */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_TRUE);
    }

    /* Restore configuration */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressConfigurationRestore(GT_FALSE);

    prvTgfPclRestore();

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Build Packet */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet();

        /* Set Base configuration */
         prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet();

        /* Set TTI configuration */
         prvTgfTunnelTermEtherOverMplsRedirectToEgressTtiConfigSet();

        /* Generate traffic */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_TRUE);

        /* Restore configuration */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressConfigurationRestore(GT_TRUE);

        prvTgfPclRestore();
    }
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination Ethernet over Mpls:
    1. configure VLAN;
       configure TTI rule;
    2. configure TTI UDB key MPLS and check METADATA field numberOfMplsLabels;
       send tunneled traffic to match TTI rule;
       verify Ethernet packet (without MPLS header) on egress port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverMplsMplsLabelsRedirectToEgress)
{
/*

    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      notAppFamilyBmp;
    GT_U32      ii;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if((GT_FALSE == prvTgfBrgTtiCheck()) || (GT_FALSE == PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* number of MPLS labels in the tunnel header is '1' */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressNumOfMplsLabels(1);

    /* Build Packet */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet();

    /* Set Base configuration */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet();

    /* Set TTI configuration */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressUdbKeyTtiConfigSet();

    /* Generate traffic */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_TRUE);

    for (ii=2; ii<=4; ii++)
    {
        /* number of MPLS labels in the tunnel header is 'ii' */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressNumOfMplsLabels(ii);

        /* Set TTI configuration */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressUdbKeyTtiConfigSet();

        /* Build Packet */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet();

        /* Generate traffic */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_TRUE);
    }

    /* Restore configuration */
    prvTgfTunnelTermEtherOverMplsRedirectToEgressConfigurationRestore(GT_TRUE);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination Ethernet over Mpls from cascade port.
    see test tgfTunnelTermEtherOverMplsRedirectToEgress + ingress with 'Extended DSA tag'
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverMplsRedirectToEgress_fromCascadePort)
{
    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    ingressIsCascadePort = GT_TRUE;
    prvTgfTunnelTermEtherOverMplsRedirectToEgressDsaUse(ingressIsCascadePort);

    UTF_TEST_CALL_MAC(tgfTunnelTermEtherOverMplsRedirectToEgress);

    ingressIsCascadePort = GT_FALSE;
    prvTgfTunnelTermEtherOverMplsRedirectToEgressDsaUse(ingressIsCascadePort);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of I-SID and B-DA assign mode in MIM tunnel start:
    configure VLAN, FDB entries;
    configure TTI rule to forward packet on egress direction
    configure MIM TS entry (I-SID mode from entry and B-DA mode from entry);
    send tagget traffic to match TS entry;
    verify MIM tunneled traffic (with I-SID and B-DA from tunnel start entry)
    on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartMimISidAndBdaIssignFromTsEntry)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode = PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E;
    PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode = PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;


    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfTunnelStartMimISidAndBdaSet(mimISidAssignMode, mimBDaAssignMode);

    /* Generate traffic */
    prvTgfTunnelStartMimISidAndBdaTrafficGenerate(mimISidAssignMode, mimBDaAssignMode);

    /* Restore configuration */
    prvTgfTunnelStartMimISidAndBdaConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of I-SID and B-DA assign mode in MIM tunnel start:
    configure VLAN, FDB entries;
    configure TTI rule to forward packet on egress direction
    configure MIM TS entry (I-SID mode from eVLAN Servise ID entry and
    B-DA mode from tunnel start entry);
    send tagget traffic to match TS entry;
    verify MIM tunneled traffic (with I-SID from eVLAN service ID and B-DA from
    tunnel start entry) on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartMimISidIssignFromEVlanServiceId)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode = PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E;
    PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode = PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;


    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfTunnelStartMimISidAndBdaSet(mimISidAssignMode, mimBDaAssignMode);

    /* Generate traffic */
    prvTgfTunnelStartMimISidAndBdaTrafficGenerate(mimISidAssignMode, mimBDaAssignMode);

    /* Restore configuration */
    prvTgfTunnelStartMimISidAndBdaConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of I-SID and B-DA assign mode in MIM tunnel start:
    configure VLAN, FDB entries;
    configure TTI rule to forward packet on egress direction
    configure MIM TS entry (I-SID mode from tunnel start entry and B-DA mode
    from eVLAN service ID entry);
    send tagget traffic to match TS entry;
    verify MIM tunneled traffic (with I-SID from tunnel start entry and MacDA
    created from tunnel start BDAMac and eVlan Servise Id entry) on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelStartMimBdaIssignFromTsEntryAndVlan)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode = PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E;
    PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode = PRV_TGF_CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E;


    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfTunnelStartMimISidAndBdaSet(mimISidAssignMode, mimBDaAssignMode);

    /* Generate traffic */
    prvTgfTunnelStartMimISidAndBdaTrafficGenerate(mimISidAssignMode, mimBDaAssignMode);

    /* Restore configuration */
    prvTgfTunnelStartMimISidAndBdaConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel termination Ethernet over Mpls with configured UDB:
       configure TTI UDB key Mpls;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverMplsUdbRedirectToEgress)
{
/*

    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/

    GT_U32      notAppFamilyBmp;
    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }


    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* Build Packet */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet();

        /* Set Base configuration */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet();

        /* Set TTI configuration */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigSet();

        /* Generate traffic */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate(GT_TRUE);

        /* Restore configuration */
        prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigurationRestore();

        prvTgfPclRestore();
    }
}

/* AUTODOC: Test tunnel termination IPv4 over IPv4 - enable/disable TTI lookup
            for IPv4 multicast:
    1. configure VLAN, FDB entries;
       configure TTI rule;
       enable TTI lookup for IPv4 multicast;
    2. configure TTI 'old key' IPv4;
       send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
    3. disable TTI lookup for IPv4 multicast;
       send tunneled traffic to match TTI rule;
       verify no traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4McEnableSet)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Enable TTI lookup for IPv4 multicast
    5. Generate traffic
    6. Disable TTI lookup for IPv4 multicast
    7. Generate traffic
    8. Restore configuration
*/

    GT_U32      prvUtfVrfId = 0;
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet(prvUtfVrfId, 0, GT_FALSE);

    /* Set Route configuration */
    prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet();

    /* Set TTI configuration */
    prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet(0, GT_FALSE);

    /* Enable TTI lookup for IPv4 multicast */
    prvTgfTtiIpv4McEnableSet(prvTgfDevNum, GT_TRUE);

    /* Generate traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Disable TTI lookup for IPv4 multicast */
    prvTgfTtiIpv4McEnableSet(prvTgfDevNum, GT_FALSE);

    /* Generate traffic -- check no traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic(GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    /* Restore configuration */
    prvTgfTunnelTermIpv4overIpv4ConfigurationRestore(GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS);

    prvTgfPclRestore();
}

/* AUTODOC: Test tunnel termination for Ethernet over MPLS with PW control word
    1. configure VLAN, FDB entries;
    2. configure TTI rule;
    3. configure TTI 'old key' MPLS;
    4. send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverMplsPwLabel)
{
/*
    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    tgfTunnelTermEtherOverMplsPwLabelBridgeConfigSet(); /* basic test */

    /* Set TTI configuration */
    tgfTunnelTermEtherOverMplsPwLabelTtiConfigSet();

    /* Generate traffic - expect forwarding */
    tgfTunnelTermEtherOverMplsPwLabelTrafficGenerate(0, GT_TRUE, GT_FALSE);

    /* Set the command for illegal control word to FORWARD */
    tgfTunnelTermEtherOverMplsPwLabelExceptionCommandSet(PRV_TGF_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic with illegal control word - expect forwarding */
    tgfTunnelTermEtherOverMplsPwLabelTrafficGenerate(1, GT_TRUE, GT_FALSE);

    /* Set the command for illegal control word to DROP_HARD */
    tgfTunnelTermEtherOverMplsPwLabelExceptionCommandSet(PRV_TGF_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E, CPSS_PACKET_CMD_DROP_HARD_E);

    /* Generate traffic with illegal control word - expect dropping */
    tgfTunnelTermEtherOverMplsPwLabelTrafficGenerate(1, GT_FALSE, GT_FALSE);

    /* Set the command for illegal control word to TRAP */
    tgfTunnelTermEtherOverMplsPwLabelExceptionCommandSet(PRV_TGF_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E, CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Generate traffic with illegal control word - expect trapping */
    tgfTunnelTermEtherOverMplsPwLabelTrafficGenerate(1, GT_FALSE, GT_TRUE);

    /* Set the base CPU code to 205 */
    tgfTunnelTermEtherOverMplsPwLabelCpuCodeBaseSet(205);

    /* Generate traffic with illegal control word - expect trapping */
    tgfTunnelTermEtherOverMplsPwLabelTrafficGenerate(1, GT_FALSE, GT_TRUE);

    /* Restore configuration */
    tgfTunnelTermEtherOverMplsPwLabelConfigurationRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test tunnel termination for Ethernet over MPLS with PW control word (Parallel lookup)
    1. configure VLAN, FDB entries;
    2. configure TTI rule;
    3. configure TTI 'old key' MPLS;
    4. send tunneled traffic to match TTI rule;
       verify passenger traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermEtherOverMplsPwLabelParallelLookup)
{
/*
    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_BOOL     cwBasedPwArray[PRV_TGF_MAX_LOOKUPS_NUM_CNS];
    GT_BOOL     applyNonDataCwCommandArray[PRV_TGF_MAX_LOOKUPS_NUM_CNS];
    GT_U8       lookupNum;
    GT_U8       testCounter=0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        cwBasedPwArray[lookupNum]=GT_TRUE;
        applyNonDataCwCommandArray[lookupNum]=GT_TRUE;
    }

    tgfTunnelTermEtherOverMplsPwLabelParallelLookup(cwBasedPwArray,applyNonDataCwCommandArray,GT_FALSE);
    testCounter++;

    while (testCounter<PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS)
    {
        lookupNum--;
        cwBasedPwArray[lookupNum]=GT_FALSE;
        applyNonDataCwCommandArray[lookupNum]=GT_FALSE;

        if (testCounter==(PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1))
        {
            tgfTunnelTermEtherOverMplsPwLabelParallelLookup(cwBasedPwArray,applyNonDataCwCommandArray,GT_TRUE);
        }
        else
        {
            tgfTunnelTermEtherOverMplsPwLabelParallelLookup(cwBasedPwArray,applyNonDataCwCommandArray,GT_FALSE);
        }
        testCounter++;
    }
}

/* AUTODOC: Test tunnel termination with parallel lookup
    1. configure VLAN, FDB entries;
    2. configure TTI rules and action;
    3. send tunneled traffic to match TTI rule;
       verify passenger traffic on target port.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermParallelLookup)
{
/*
    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId = 0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports are not remote ports
     * (test is using feature not supported on DSA port) */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   0,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }


    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    /* basic test */
    prvTgfTunnelTermParallelLookupBaseConfigurationSet(prvUtfVrfId);

    /* Set Route configuration */
    prvTgfTunnelTermParallelLookupRouteConfigurationSet();

    /* Set TTI configuration */
    prvTgfTunnelTermParallelLookupTtiConfigurationSet();

    /* All lookups are enabled, so all lookup actions are applied */
    /* Generate traffic - check no traffic */
    prvTgfTunnelTermParallelLookupTrafficGenerateExpectNoTraffic(GT_FALSE);

    /* Disable Lookup 1 in lookup 0 action */
    prvTgfTunnelTermParallelLookupEnableLookup(0, GT_FALSE);

    /* Lookup 0 action is applied */
    /* Generate traffic */
    prvTgfTunnelTermParallelLookupTrafficGenerate(GT_FALSE);

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        /* Enable Lookup 1 in lookup 0 action */
        prvTgfTunnelTermParallelLookupEnableLookup(0, GT_TRUE);

        /* Disable Lookup 2 in lookup 1 action */
        prvTgfTunnelTermParallelLookupEnableLookup(1, GT_FALSE);

        /* Lookup 0 and 1 actions are applied */
        /* Generate traffic */
        prvTgfTunnelTermParallelLookupTrafficGenerate(GT_TRUE);

        /* Enable Lookup 2 in lookup 1 action */
        prvTgfTunnelTermParallelLookupEnableLookup(1, GT_TRUE);

        /* Disable Lookup 3 in lookup 2 action */
        prvTgfTunnelTermParallelLookupEnableLookup(2, GT_FALSE);

        /* Lookup 0, 1 and 2 actions are applied */
        /* Generate traffic */
        prvTgfTunnelTermParallelLookupTrafficGenerateExpectNoTraffic(GT_TRUE);

        /* Disable Lookup 1 in lookup 0 action */
        prvTgfTunnelTermParallelLookupEnableLookup(0, GT_FALSE);
    }

    /* Disable parallel lookup */
    prvTgfTunnelTermParallelLookupEnable(GT_FALSE);

    /* Generate traffic */
    prvTgfTunnelTermParallelLookupTrafficGenerate(GT_FALSE);

    /* SIP_5_20 the 'single TCAM mode' was removed from the device */
    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* Enable Lookup 1 in lookup 0 action */
        prvTgfTunnelTermParallelLookupEnableLookup(0, GT_TRUE);

        /* Generate traffic */
        prvTgfTunnelTermParallelLookupTrafficGenerate(GT_FALSE);
    }

    /* Restore configuration */
    prvTgfTunnelTermParallelLookupConfigurationRestore();

    prvTgfPclRestore();
}

/* AUTODOC: Test tunnel termination with parallel lookup
    1. Configure all TTI rules with NO_REDIRECT command;
    2. Send traffic and check that no packet received;
    3. Configure all TTI rules one by one with different Redirect commands;
    4. Send packets and check traffic on expected ports;
    Note: Only one Redirect command should be enabled in the rule at a time
*/
UTF_TEST_CASE_MAC(tgfTunnelTermParallelLookupRedirect)
{
/*
    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set Base configuration */
    prvTgfTunnelTermParallelLookupBaseConfigurationSet(0);

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        /* Set Routing configuration */
        prvTgfTunnelTermParallelLookupRedirectRouteConfigurationSet();
    }

    /* Set TTI configuration for rule 0 in lookup 0 with NO_REDIRECT command */
    prvTgfTunnelTermParallelLookupRedirectTtiRule0ConfigurationSet(PRV_TGF_TTI_NO_REDIRECT_E);

    /* Set TTI configuration for rule 1 in lookup 1 with NO_REDIRECT command */
    prvTgfTunnelTermParallelLookupRedirectTtiRule1ConfigurationSet(PRV_TGF_TTI_NO_REDIRECT_E);

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        /* Set TTI configuration for rule 2 in lookup 2 with NO_REDIRECT command */
        prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet(PRV_TGF_TTI_NO_REDIRECT_E, GT_FALSE);

        /* Set TTI configuration for rule 3 in lookup 3 with NO_REDIRECT command */
        prvTgfTunnelTermParallelLookupRedirectTtiRule3ConfigurationSet(CPSS_PACKET_CMD_FORWARD_E);
    }

    /* Generate traffic - check no traffic */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic(GT_TRUE, GT_TRUE);

    /* Set TTI configuration for rule 0 in lookup 0 with REDIRECT_TO_EGRESS command */
    prvTgfTunnelTermParallelLookupRedirectTtiRule0ConfigurationSet(PRV_TGF_TTI_REDIRECT_TO_EGRESS_E);

    /* Generate traffic - check traffic on NEXTHOPE port */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE_PORT_IDX_CNS, GT_TRUE, GT_TRUE);

    /* Set TTI configuration for rule 0 in lookup 0 with NO_REDIRECT command */
    prvTgfTunnelTermParallelLookupRedirectTtiRule0ConfigurationSet(PRV_TGF_TTI_NO_REDIRECT_E);

    /* Set TTI configuration for rule 1 in lookup 1 with REDIRECT_TO_EGRESS command */
    prvTgfTunnelTermParallelLookupRedirectTtiRule1ConfigurationSet(PRV_TGF_TTI_REDIRECT_TO_EGRESS_E);

    /* Generate traffic - check traffic on NEXTHOPE1 port */
    prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE1_PORT_IDX_CNS, GT_TRUE, GT_TRUE);

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        /* Set TTI configuration for rule 2 in lookup 2 with REDIRECT_TO_ROUTER_LOOKUP command */
        prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet(PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E, GT_FALSE);

        /* Generate traffic - check traffic on NEXTHOPE1 port */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE1_PORT_IDX_CNS, GT_TRUE, GT_TRUE);

        /* Set TTI configuration for rule 1 in lookup 1 with NO_REDIRECT command */
        prvTgfTunnelTermParallelLookupRedirectTtiRule1ConfigurationSet(PRV_TGF_TTI_NO_REDIRECT_E);

        /* Set TTI configuration for rule 2 in lookup 2 with REDIRECT_TO_ROUTER_LOOKUP command */
        prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet(PRV_TGF_TTI_REDIRECT_TO_ROUTER_LOOKUP_E, GT_TRUE);

        /* Generate traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerate(PRV_TGF_NEXTHOPE2_PORT_IDX_CNS, GT_TRUE, GT_TRUE);

        /* Set TTI configuration for rule 2 in lookup 2 with NO_REDIRECT command */
        prvTgfTunnelTermParallelLookupRedirectTtiRule2ConfigurationSet(PRV_TGF_TTI_NO_REDIRECT_E, GT_FALSE);

        /* Set TTI configuration for rule 3 in lookup 3 with DROP command */
        prvTgfTunnelTermParallelLookupRedirectTtiRule3ConfigurationSet(CPSS_PACKET_CMD_DROP_HARD_E);

        /* Generate traffic - check no traffic */
        prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic(GT_TRUE, GT_TRUE);
    }

    /* Restore CNC configurations */
    prvTgfTunnelTermParallelLookupCncRestore();

    /* Restore configuration */
    prvTgfTunnelTermParallelLookupConfigurationRestore();

}

/* AUTODOC: Test tunnel termination - CPU codes for exception and masking of IP header errors.
    tested APIs: cpssDxChTtiExceptionCpuCodeSet, cpssDxChIpHeaderErrorMaskSet
    1. configure VLAN, FDB entries;
    2. configure TTI rule and set CPU code for CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
    3. send tunneled traffic and verify passenger traffic.
    4. Mask IPv4 SIP=DIP header error
    5. send tunneled traffic and verify passenger traffic.
    6. Unmask IPv4 SIP=DIP header error
    7. send tunneled traffic and verify passenger traffic.
*/
UTF_TEST_CASE_MAC(tgfTunnelTermExceptionCpuCode)
{
/*
    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_U32      prvUtfVrfId;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelTermExceptionCpuCodeBaseConfigurationSet(prvUtfVrfId);

    /* Set Route configuration */
    prvTgfTunnelTermExceptionCpuCodeRouteConfigurationSet();

    /* Set TTI configuration */
    prvTgfTunnelTermExceptionCpuCodeTtiConfigurationSet();

    /* Generate traffic with IPv4 SIP address error */
    prvTgfTunnelTermExceptionCpuCodeTrafficGenerate(0);

    /* Generate traffic with IPv4 header error */
    prvTgfTunnelTermExceptionCpuCodeTrafficGenerate(1);

    /* Mask IPv4 SIP=DIP header error */
    prvTgfTunnelTermExceptionCpuCodeMaskHeaderErrorType(PRV_TGF_IP_HEADER_ERROR_SIP_DIP_ENT, GT_TRUE);

    /* Generate traffic with IPv4 SIP=DIP header error */
    prvTgfTunnelTermExceptionCpuCodeTrafficGenerate(2);

    /* Unmask IPv4 SIP=DIP header error */
    prvTgfTunnelTermExceptionCpuCodeMaskHeaderErrorType(PRV_TGF_IP_HEADER_ERROR_SIP_DIP_ENT, GT_FALSE);

    /* Generate traffic with IPv4 SIP=DIP header error */
    prvTgfTunnelTermExceptionCpuCodeTrafficGenerate(1);

    /* Restore configuration */
    prvTgfTunnelTermExceptionCpuCodeConfigurationRestore();

    prvTgfPclRestore();
}

/*  AUTODOC: Test functionality of TTI Ingress Mirroring:

    AUTODOC: add TTI rule 0 with mirroring enabled and index [0] with action: redirect to egress
    AUTODOC: set analyzer interface configuration with index [0]
    AUTODOC: send packet and expect TTI match and packet mirrored
    -------------------------------------------------------------------------
    AUTODOC: set TTI rule 0 with mirroring disabled
    AUTODOC: send packet and expect TTI match and packet not mirrored
    -------------------------------------------------------------------------
    AUTODOC: add TTI rule 1 with mirroring enabled index [0] with action: redirect to egress
    AUTODOC: set TTI rule 0 with mirroring disabled
    AUTODOC: send packet and expect TTI0 and TT1 match and packet not mirrored
    AUTODOC: Restore configuration
*/
UTF_TEST_CASE_MAC(tgfTunnelTermMirrorToIngressAnalyzer)
{
/*
    1. Set TTI Mirror to Ingress Analyzer Base configuration
    2. Generate traffic -- check traffic is mirrored
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermMirrorToIngressAnalyzer();

}

/* AUTODOC: Test functionality of TTI set mac2me:
    1. configure TTI rule with Ether key;
    2. send tunneled traffic to match TTI rule;
    3. restore configuration;
*/
UTF_TEST_CASE_MAC(tgfTunnelTermSetMac2Me)
{
/*
    1. Set TTI Base configuration
    2. Generate traffic -- check
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermSetMac2MeTest();

    tgfTunnelTermSetMac2MeTestRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test functionality of L3 offset - 2 key type for LLC Non SNAP:
    configure VLAN, FDB entries;
    configure TTI rule with L3 offset - 2 key;
    send tunneled traffic to match TTI rule;
    verify to get no traffic -- dropped;
*/
UTF_TEST_CASE_MAC(tgfTunnelTermLlcNonSnapType)
{
/*
    1. Set TTI Ether Type Key Base configuration
    2. Generate traffic -- check traffic is blocked
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Set TTI Ether Type Key Base configuration */
    prvTgfTunnelTermEtherTypeOtherBaseConfigurationSet();

    /* Generate traffic -- check no traffic */
    prvTgfTunnelTermEtherTypeOtherTrafficGenerateExpectNoTraffic();

    /* Restore configuration */
    prvTgfTunnelTermEtherTypeOtherConfigRestore();
}

/* AUTODOC: Test functionality of TTI set iPclUdbConfigTableIndex:
    1. configure Pcl to drop traffic with UDE0 packet type;
    2. send traffic and check that it is dropped;
    3. setup tti rule to modify packet type to be UDE6;
    4. send traffic and check that pcl not match it (no drop);
    5. check dual lookup: setup second tti rule to modify packet type to be UDE0;
    6. send traffic and check that it is dropped by pcl rule;
    7. restore configuration;
*/
UTF_TEST_CASE_MAC(tgfTunnelTermSetIPclUdbConfigTableIndex)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermSetIPclUdbConfigTableIndexTest();
}

#if 0
/* AUTODOC: Check TCAM Lookup Order
    1. Configure basic TTI rule with IPv4 key;
    2. Configure TTI0 rules with <continueToNextTtiLookup> Disabled
    3. Generate traffic and check TTI0 rules to match
    4. Configure TTI0 and TTI1 rules with <continueToNextTtiLookup> Enabled
    5. Generate traffic and check TTI1 rules to match
    6. Restore configuration
*/
UTF_TEST_CASE_MAC(tgfTunnelTcamCheckLookupOrder)
{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set TTI0 Configuration
    4. Generate traffic
    5. Set TTI1 Configuration
    6. Generate traffic
    7. Restore configuration
*/

    GT_U32      notAppFamilyBmp;
    GT_U32      step = 128 /* send every 128th packet to reduce time consumption */;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Build ipv4 packet */
    tgfTunnelTcamCheckLookupOrderBuildPacket();

    /* Configure VLAN 5 with ports [0, 8], VLAN 6 with ports [18, 23] */
    tgfTunnelTcamCheckLookupOrderBridgeConfigSet();

    /* Configure basic TTI rule*/
    tgfTunnelTcamCheckLookupOrderBasicRuleConfigurationSet();

    /* Disable 2nd TTI lookup */
    tgfTunnelTcamCheckLookupOrderEnableTti1Lookup(GT_FALSE);

    /* Configure rules for TTI0. 5120 - range of TTI0  */
    tgfTunnelTcamCheckLookupOrderConfigurationSet(5120, step);

    /* Start sending packets to match all TTI0 rules*/
    tgfTunnelTcamCheckLookupOrderStartTraffic(5120, step);

    /* Enable 2nd TTI lookup */
    tgfTunnelTcamCheckLookupOrderEnableTti1Lookup(GT_TRUE);

    /* Configure rules for TTI0 and TTI1. 6144 - range of TTI1 */
    tgfTunnelTcamCheckLookupOrderConfigurationSet(6144, step);

    /* Start sending packets to match TTI1 rules */
    tgfTunnelTcamCheckLookupOrderStartTraffic(6144, step);

    /* Restore configuration */
    tgfTunnelTcamCheckLookupOrderConfigurationRestore();
}
#endif

/* AUTODOC: Test tunnel termination trigger requirements for MPLS traffic

       AUTODOC: create VLAN 5 with untagged ports [0,1]
       AUTODOC: create VLAN 6 with untagged ports [2,3]
       AUTODOC: set the TTI Rule Action for all the packets
       AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with action:
       AUTODOC:   cmd REDIRECT, port 36

       AUTODOC: set packet etherType 0x8847, legal mpls
       AUTODOC: send packet and expect TTI match, redirect to port 36

       AUTODOC: set packet etherType 0x8848, legal mpls
       AUTODOC: send packet and expect TTI match, redirect to port 36

       AUTODOC: set packet etherType 0x8849, illegal etherType
       AUTODOC: send packet and expect no TTI match, no redirect to port 36

       AUTODOC: save original mpls packet etherType (for config restore)
       AUTODOC: set packet etherType 0x8849 (illegal) as legal mpls ethertype
       AUTODOC: send packet and expect TTI match, redirect to port 36
*/
UTF_TEST_CASE_MAC(tgfTunnelTermTrigReqMpls)
{
/*
    1. Build Packet
    2. Set Base configuration
    3. Set TTI configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermTrigReqMpls();
}


/* AUTODOC: Test tunnel termination source id modifying and mask check,
                supported up to four lookups

       AUTODOC: create VLAN 5 with untagged ports [0,1]
       AUTODOC: create VLAN 6 with untagged ports [2,3]

       AUTODOC: set the TTI Rule Action for all the packets
       AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with
       AUTODOC: TTI action: modify srcId, modify vid to 6, redirect to port 36

       AUTODOC: Enable tcam segment mode (for parallel lookup)
       AUTODOC: Add TTI rules (2 or 4 according to the device type)
       AUTODOC: Each TTI rule modifies source Id with unique source Id mask

       AUTODOC: set the ePcl Rule for the packets matching final tti srcId
       AUTODOC: ePcl rule action: drop packets with tti src id value

       AUTODOC: send packet and expect TTI match, and no traffic on port 36 (dropped by ePcl)

       AUTODOC: set tti src id mask, so tti will apply another srcId, so ePcl will not match
       AUTODOC: send packet and expect TTI match, and traffic on port 36 (no ePcl match, no drop)

       AUTODOC: modify tti src id mask for first lookup, so tti will apply another srcId, so ePcl will not match
       AUTODOC: send packet and expect TTI match, and traffic on port 36 (no ePcl match, no drop)

       AUTODOC: test configuration restore (for all the lookups)
*/
UTF_TEST_CASE_MAC(tgfTunnelTermSourceIdModify)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermSourceIdModify();
}



/* AUTODOC: Test tunnel termination source Eport Assignment
                supported up to four lookups

       AUTODOC: create VLAN 5 with untagged ports [0]
       AUTODOC: create VLAN 6 with untagged ports [1,2,3]

       AUTODOC: for all lookups do
       AUTODOC: set the TTI Rule Action for all the packets
       AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with
       AUTODOC: TTI action: ePort assignment, modify srcId, modify vid to 6,
                redirect to port 18 lookup 0,2 and port 36 lookup 1,3

       AUTODOC: Enable tcam segment mode (for parallel lookup)
       AUTODOC: Add TTI rules (2 or 4 according to the device type)
       AUTODOC: Each TTI rule modifies source Id with unique source Id mask,
                and sourceEport with unique sourceEport mask

       AUTODOC: send packet and expect TTI match, and traffic on port 36
                (according to lookup 1 for dual lookup or lookup 3 for quad lookup)
       AUTODOC: Check the sourceEport assignment: by checking an entry was
                learened in the FDB table with the assigned new sourceEport.

       AUTODOC: send number of packets as number of lookup,
                each time disable the sourceEportAssignment on the last lookup done,
                and each time check that the entry in the FDB table was learned with
                a different source ePort

       AUTODOC: test configuration restore (for all the lookups)
*/
UTF_TEST_CASE_MAC(tgfTunnelTermSourceEportAssignment)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;
    GT_BOOL     sourceEPortAssignmentEnableArray[PRV_TGF_MAX_LOOKUPS_NUM_CNS];
    GT_U8       lookupNumSourceEportExpected;
    GT_U8       lookupNum;
    GT_U8       testCounter=0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        sourceEPortAssignmentEnableArray[lookupNum]=GT_TRUE;
    }

    /* expect the eport to be as defined in the last valid lookup */
    lookupNumSourceEportExpected = (lookupNum-1);
    tgfTunnelTermSourceEportAssignment(sourceEPortAssignmentEnableArray,lookupNumSourceEportExpected,GT_FALSE);
    testCounter++;

    while (lookupNumSourceEportExpected!=0)
    {
        sourceEPortAssignmentEnableArray[lookupNumSourceEportExpected] = GT_FALSE;
        lookupNumSourceEportExpected--;
        if (testCounter==(PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1))
        {
            tgfTunnelTermSourceEportAssignment(sourceEPortAssignmentEnableArray, lookupNumSourceEportExpected,GT_TRUE);
        }
        else
        {
            tgfTunnelTermSourceEportAssignment(sourceEPortAssignmentEnableArray, lookupNumSourceEportExpected,GT_FALSE);
        }
        testCounter++;
    }
}

/* AUTODOC: Test tunnel termination cnc, supported up to four lookups

       AUTODOC: create VLAN 5 with untagged ports [0,1]
       AUTODOC: create VLAN 6 with untagged ports [2,3]

       AUTODOC: set the TTI Rule Action for all the packets
       AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with
       AUTODOC: TTI action: modify srcId, modify vid to 6, redirect to port 36

       AUTODOC: Add TTI rules (2 or 4 according to the device type)
       AUTODOC: Each TTI rule binds to cnc counter index

       AUTODOC: set Cnc configuration for all cnc indexes

       AUTODOC: send packet and expect TTI match, and traffic on port 36

       AUTODOC: check cnc counters for every lookup

       AUTODOC: test configuration restore (for all the lookups)
*/
UTF_TEST_CASE_MAC(tgfTunnelTermCncParallelLookup)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelTermCncParallelLookup();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel IPV6. Test covers following features:
            1) Configurable extension
            2) Tti non HBH error (exception) command
            3) Is L4 valid logic (for ipv6 configurable extension)

    AUTODOC: build IPv6 Packet  with extension header value 0x1

    AUTODOC: create VLAN 5 with untagged ports [0,1]
    AUTODOC: create VLAN 6 with untagged ports [2,3]

    AUTODOC: add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with
    AUTODOC: TTI action: modify vid to 6, redirect to port 36
    AUTODOC: set the TTI Rule Pattern and Mask to match packets with <isL4Valid> is not set
    AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21
    AUTODOC: set the TTI Rule Action for matched packet (not l4 valid)
    AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_E at the port 0
    AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key PRV_TGF_TTI_KEY_UDB_IPV6_E
    AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_UDB_IPV6_E on port 0 VLAN 5 with action: redirect to egress
    -----------------------------------------------------------
    AUTODOC: send packet and expect no TTI match, and no traffic on port 36

    -----------------------------------------------------------
    AUTODOC: save current configured extended header (for configuration restore)
    AUTODOC: save ipv6 tti non HBH error command (for configuration restore)
    AUTODOC: set configurable extended header with id 0, ext value 0x1 (same as in packet)
    AUTODOC: set ipv6 tti non HBH error command CPSS_PACKET_CMD_FORWARD_E (prevent packet drop)
    AUTODOC: send packet and expect TTI match (not L4 valid), and traffic on port 36

    -----------------------------------------------------------
    AUTODOC: test configuration restore
    AUTODOC: restore ipv6 tti configurable extension value
    AUTODOC: restore ipv6 tti non HBH error command
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv6ExtensionHeaderSet)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermIpv6ExtensionHeaderSet();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel mpls pw ttl expiry vccv exception.
                Test covers following features:
                1) Vccv exception enable
                2) Vccv exception command
                3) Pw label ttl values 0/1/2

    AUTODOC: build mpls packet with s-bit 0x1, ttl 0x1
    AUTODOC: create VLAN 5 with untagged ports [0,1]
    AUTODOC: create VLAN 6 with untagged ports [2,3]
    AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_MPLS_E on port 0 VLAN 5 with action: redirect to egress
    -----------------------------------------------------------
    AUTODOC: send packet and expect TTI match, and no traffic on port 36, dropped (exception)
    -------------------------------------------------------------------------
    AUTODOC: change ttl field in pw label to 0x2
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0x2, no exception)
    -------------------------------------------------------------------------
    AUTODOC: change ttl field in pw label to 0
    AUTODOC: set the CPSS_PACKET_CMD_FORWARD_E command for expiry_vccv_error exception
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0, exception forward)
    -------------------------------------------------------------------------
    AUTODOC: set the CPSS_PACKET_CMD_DROP_HARD_E command for expiry_vccv_error exception
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0, exception drop)
    AUTODOC: restore vccv command
    AUTODOC: test configuration restore
*/

UTF_TEST_CASE_MAC(prvTgfTunnelPwTtlExpiryVccvException)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelPwTtlExpiryVccvException();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel mpls pw ttl expiry vccv exception with Flow Label Exist
            in parallel lookup
                Test covers following features:
                1) Vccv exception enable with FL exist
                2) Vccv exception command
                3) Pw label ttl values 0/1/2 with FL exist

    AUTODOC: build mpls packet with PW s-bit 0x1, ttl 0x1, and FL
    AUTODOC: create VLAN 5 with untagged ports [0,1]
    AUTODOC: create VLAN 6 with untagged ports [2,3]
    AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_MPLS_E on port 0 VLAN 5 with action: redirect to egress
    -----------------------------------------------------------
    AUTODOC: send packet and expect TTI match, and no traffic on port 36, dropped (exception)
    -------------------------------------------------------------------------
    AUTODOC: change ttl field in pw label to 0x2
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0x2, no exception)
    -------------------------------------------------------------------------
    AUTODOC: change ttl field in pw label to 0
    AUTODOC: set the CPSS_PACKET_CMD_FORWARD_E command for expiry_vccv_error exception
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0, exception forward)
    -------------------------------------------------------------------------
    AUTODOC: set the CPSS_PACKET_CMD_DROP_HARD_E command for expiry_vccv_error exception
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (ttl value 0, exception drop)
    AUTODOC: restore vccv command
    AUTODOC: test configuration restore
*/

UTF_TEST_CASE_MAC(prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup)
{
    GT_U32      notAppFamilyBmp;
    GT_BOOL     ttlExpiryVccvEnable[PRV_TGF_MAX_LOOKUPS_NUM_CNS];
    GT_BOOL     pwe3FlowLabelExist[PRV_TGF_MAX_LOOKUPS_NUM_CNS];
    GT_U8       lookupNum;
    GT_U8       testCounter=0;

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        ttlExpiryVccvEnable[lookupNum]=GT_TRUE;
        pwe3FlowLabelExist[lookupNum]=GT_TRUE;
    }

    prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup(ttlExpiryVccvEnable,pwe3FlowLabelExist,GT_FALSE);
    testCounter++;

    while (testCounter<PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS)
    {
        lookupNum--;
        ttlExpiryVccvEnable[lookupNum]=GT_FALSE;
        pwe3FlowLabelExist[lookupNum]=GT_FALSE;

        if (testCounter==(PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1))
        {
            prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup(ttlExpiryVccvEnable,pwe3FlowLabelExist,GT_TRUE);
        }
        else
        {
            prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup(ttlExpiryVccvEnable,pwe3FlowLabelExist,GT_FALSE);
        }
        testCounter++;
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel mpls pw cw based ETree.
                Test covers following features:
                1) pwCwBasedETreeEnable
                2) <L> bit values 0/1

    AUTODOC: build mpls packet with <L> bit 1
    AUTODOC: create VLAN 5 with untagged ports [0,1]
    AUTODOC: create VLAN 6 with untagged ports [2,3]
    -------------------------------------------------------------------------
    AUTODOC: set the TTI Rule Action for all the packets
    AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with
    AUTODOC: TTI action: TT <enable>, cwBasedPw <enable>, modify vid to 6, redirect to port 36
    -------------------------------------------------------------------------
    AUTODOC: set the ePcl Rule for the packets matching srcId
    AUTODOC: ePcl rule action: drop packets with src id value
    -------------------------------------------------------------------------
    AUTODOC: TTI action: set pwCwBasedETreeEnable <enabled>
    AUTODOC: send packet and expect TTI match, and no traffic on port 36 (dropped by ePcl)
    -------------------------------------------------------------------------
    AUTODOC: TTI action: set pwCwBasedETreeEnable <disabled>
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (no PCL match, packet forward)
    -------------------------------------------------------------------------
    AUTODOC: TTI action: set pwCwBasedETreeEnable <enabled>
    AUTODOC: set packet's <L> bit to 0
    AUTODOC: send packet and expect TTI match, and traffic on port 36 (no PCL match, packet forward)
    -------------------------------------------------------------------------
    AUTODOC: TTI0 action: set pwCwBasedETreeEnable <disabled>, enable second TTI lookup
    AUTODOC: TTI1 action: set pwCwBasedETreeEnable <enabled>
    AUTODOC: set packet's <L> bit to 1
    AUTODOC: send packet and expect TTI0 match and TTI1 match and no traffic on port 36 (dropped by ePcl)
    -------------------------------------------------------------------------
    AUTODOC: test configuration restore
*/
UTF_TEST_CASE_MAC(tgfTunnelPwCwBasedETree)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelPwCwBasedETree();
}

/* AUTODOC: Test functionality of TTI OAM mpls ChannelToOamOpcode
*/
UTF_TEST_CASE_MAC(tgfTunnelTermMapMplsChannelToOamOpcode)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);


    CPSS_TBD_BOOKMARK_EARCH /* need sip5 oam alignment: channel type to opcode mapping from tti */
    SKIP_TEST_MAC
#if 0
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermMapMplsChannelToOamOpcodeTest();
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prfTgfTunnelDualLookupNestedVLan
                Test covers following features:
                1) nestedVlanEnable

    AUTODOC: build ethetnet packet with vid 5

    AUTODOC: create VLAN 5 with untagged ports [0,1]
    AUTODOC: create VLAN 6 with untagged ports [2,3]
    AUTODOC: add TTI rule 0 with PRV_TGF_TTI_KEY_ETH_E with nestedVlan enabled and action: redirect to egress
    ------------------------------------------------------------
    AUTODOC: send packet and expect TTI match and VLAN tag present
    ------------------------------------------------------------
    AUTODOC: change TTI rule 0 nestedVlan to disabled
    AUTODOC: send packet and expect TTI match, and no VLAN tag
    ------------------------------------------------------------
    AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_ETH_E with nestedVlan enabled and action: redirect to egress
    AUTODOC: send packet and expect TTI match, and VLAN tag present
    ------------------------------------------------------------
    AUTODOC: test configuration restore

*/
UTF_TEST_CASE_MAC(prfTgfTunnelDualLookupNestedVLan)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prfTgfTunnelDualLookupNestedVLan();
}

/* AUTODOC: Test functionality of TTI iPclOverrideConfigTableIndex:

    0. set base configuration:

        ini tti rules lookup 0/1 base tcam indexes
        enable TTI lookup for port 0, key TTI_KEY_ETH
        create VLAN 6 with untagged port [3]
        set the EtherType 0x8888 to identify UDE
        init ipcl lookup CPSS_PCL_LOOKUP_NUMBER_0_E for port 0
        init ipcl lookup CPSS_PCL_LOOKUP_NUMBER_1_E for port 0
        init ipcl lookup CPSS_PCL_LOOKUP_NUMBER_2_E for port 0

    1. configure iPcl to drop traffic:

        set the iPCL Rules:
             rule 1:  match vlan id 1 - set vlan id 2
             rule 2:  match vlan id 2 - set vlan id 3
             rule 3:  match vlan id 3 - drop the packet

    2. send traffic and check that it is dropped:

        CPSS_PCL_LOOKUP_NUMBER_0_E matches rule 1: match vlan id 1 - set vlan id 2
        CPSS_PCL_LOOKUP_NUMBER_1_E matches rule 2: match vlan id 2 - set vlan id 3
        CPSS_PCL_LOOKUP_NUMBER_2_E matches rule 3: match vlan id 3 - drop the packet

        ---> The packet dropped by pcl lookup 2.

    3. setup tti rule to not modify ingress pcl config index:

        first rule - using default values
        add TTI rule with: match all the packets

        ttiAction2.pcl1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        ttiAction2.continueToNextTtiLookup         = GT_TRUE;

    4. send traffic and check that it is dropped - no pcl config changes:

        CPSS_PCL_LOOKUP_NUMBER_0_E matches rule 1: match vlan id 1 - set vlan id 2
        CPSS_PCL_LOOKUP_NUMBER_1_E matches rule 2: match vlan id 2 - set vlan id 3
        CPSS_PCL_LOOKUP_NUMBER_2_E config index was not changed by second tti rule
        CPSS_PCL_LOOKUP_NUMBER_2_E matches rule 3: match vlan id 3 - drop the packet

        ---> The packet dropped by pcl lookup 2.

    5. check dual lookup: setup second tti rule to modify pcl config index:

        second rule - override values
        add TTI rule with: match all the packets
        ttiAction2.pcl1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        ttiAction2.continueToNextTtiLookup         = GT_FALSE;

    6. send traffic and check that it is not dropped by pcl rule:

        CPSS_PCL_LOOKUP_NUMBER_0_E matches rule 1: match vlan id 1 - set vlan id 2
        CPSS_PCL_LOOKUP_NUMBER_1_E matches rule 2: match vlan id 2 - set vlan id 3
        CPSS_PCL_LOOKUP_NUMBER_2_E config index was changed by second tti rule
        CPSS_PCL_LOOKUP_NUMBER_2_E disabled - so no drop the packet

        ---> The packet forwarded with modified vid 3 to egress port

    7. restore configuration

*/
UTF_TEST_CASE_MAC(tgfTunnelTermSetIPclOverrideConfigTableIndex)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermSetIPclOverrideConfigTableIndexTest();
}


/* AUTODOC: Test functionality of Tunnel Start Passenger Type
    Description: This field is taken from the entry with <Redirect Command> set
                    to Redirect to egress interface.
                 The goal is to set empty tunnel start entry and two actions
                 with different tunnel start passenger type that will change
                 appropriate bytes in packet. Changing their Redirect cmd
                 we'll check if the expected field was taken.

    AUTODOC: build ethernet packet with vid 5
    AUTODOC: create VLAN 5 with untagged ports [0,1]
    AUTODOC: create VLAN 6 with untagged ports [2,3]
    -----------------------------------------------------------
    AUTODOC: set TTI0 rule with redirect command Redirect to Egress and
             ts passenger type ETHERNET
             It will change packet byte 17 to '0x50'
    AUTODOC: set TTI1 rule with redirect command NO_REDIRECT and
             ts passenger type OTHER
             It will change packet byte 17 to '0x40' or to '0x1c' on GM
    AUTODOC: send packet and expect TTI match and TTI0 TSPT taken
    -----------------------------------------------------------
    AUTODOC: change TTI0 Cmd to NO_REDIRECT
    AUTODOC: change TTI1 Cmd to REDIRECT_TO_EGRESS
    AUTODOC: send packet and expect TTI match and TTI1 Tunnel Start Passenger Type taken
    -----------------------------------------------------------
    AUTODOC: change TTI0 Cmd to REDIRECT_TO_EGRESS
    AUTODOC: send packet and expect TTI match and TTI1 Tunnel Start Passenger Type taken
    -----------------------------------------------------------
    AUTODOC: test configuration restore
*/

UTF_TEST_CASE_MAC(prvTgfTunnelTermTSPassengerTypeDualLookup)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelTermTSPassengerTypeDualLookup();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel IPV6. Test covers tti ipv6 header error exceptions,
             (exceptions commands and cpu codes)
            1) Header error exception:
                a) packet header version
                b) packet SIP == DIP
            2) SIP address exception
                a) SIP == FF:: (multicast)
                a) SIP ==  ::1 (loopback)

    ------------------------------------------------------------
    build correct IPv6 Packet

    create VLAN 5 with untagged ports [0,1]
    create VLAN 6 with untagged ports [2,3]

    set the TTI Rule Pattern and Mask to match all packets
    add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with
    TTI action: modify vid to 6, bypass ingress, redirect to port 36

    ------------------------------------------------------------
    send packet and expect TTI match, and traffic on port 36

    ---------------------------------------------------------------------------------
    Following checks performed according scenario below:

    1) modify packet, ipv6 header version to 7 (wrong version)
    2) modify packet, set packet SIP = DIP
    3) modify packet, set packet SIP = FF:: (multicast sip)
    4) modify packet, set packet SIP =  ::1 (loopback sip)

    Checks logic:
        save exception configuration
        ------------------
            set ipv6 tti header error command to CPSS_PACKET_CMD_FORWARD_E
            send packet and expect ipv6 header exception
            expect packet forward and no trap to cpu
        ------------------
            set ipv6 tti header error command to CPSS_PACKET_CMD_DROP_HARD_E
            send packet and expect ipv6 header exception
            expect packet drop and no trap to cpu
        ------------------
            set ipv6 tti header error command to CPSS_PACKET_CMD_TRAP_TO_CPU_E
            set exception CPU code CPSS_NET_IP_HDR_ERROR_E
            send packet and expect ipv6 header exception
            expect packet trap to cpu with configured cpu code
        ------------------
        restore exception configuration

    ---------------------------------------------------------------------------------

    restore test configuration
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv6HeaderExceptions)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermIpv6HeaderExceptions();
}

/* AUTODOC: Test functionality of Dual Lookup: Policy Based Routing Pointer
    Description:
                 1. This field is taken from the same Action
                 from which the <Redirect Command> Policy-based Route is assigned.

                 2. Redirect Command
                    Enabled if this field is enabled in either TTI0 or TTI1.
                    If TTI1 enables the same <Redirect Command> as TTI0, then TTI1 overrides TTI0.

                 The goal is to check these conditions by sending a packet with
                 configured TTI valid/invalid LTT pointer and expect to
                 receive or not to receive traffic respectively.

    -----------------------------------------------------------
    AUTODOC: build TT ipv4-over-ipv4 packet
    AUTODOC: create VLAN 5 with untagged ports [0,1]
    AUTODOC: create VLAN 6 with untagged ports [2,3]
    AUTODOC: configure IP routing, LPM and LTT entries
    -----------------------------------------------------------
    AUTODOC: set TTI0 rule with redirect command NO_REDIRECT and invalid routerLttPtr + 1
    AUTODOC: set TTI1 rule with redirect command REDIRECT_TO_ROUTER and valid routerLttPtr
    AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and traffic on port
    -----------------------------------------------------------
    AUTODOC: set TTI1 routerLttPtr to invalid (routerLttPtr + 1)
    AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and no traffic on port
    -----------------------------------------------------------
    AUTODOC: set TTI0 redirect Cmd to REDIRECT_TO_ROUTER and valid routerLttPtr
    AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and no traffic on port
    -----------------------------------------------------------
    AUTODOC: set TTI0 redirect Cmd to REDIRECT_TO_ROUTER invalid routerLttPtr + 1
    AUTODOC: set TTI1 redirect Cmd to REDIRECT_TO_ROUTER and valid routerLttPtr
    AUTODOC: send packet and expect TTI match, TTI1 routerLttPtr taken and receive traffic on port
    -----------------------------------------------------------
    AUTODOC: test configuration restore
*/

UTF_TEST_CASE_MAC(prvTgfTunnelTermPbrDualLookup)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelTermPbrDualLookup();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    tgfTunnelTermIpv6PclId: test covers tti ipv6 metadata pclId logic for ipv6 udb key types

    Following ipv6 udb TTI key types checked according scenario below:
        PRV_TGF_TTI_KEY_UDB_IPV6_E
        PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E
        PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E

    ======= Build correct IPv6 Packet =======

    ======= Set vlans configuration =======
        create VLAN 5 with untagged ports [0,1]
        create VLAN 6 with untagged ports [2,3]

    ======= Set TTI configuration =======
        add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with
        TTI action: modify vid to 6, redirect to port 36
        set the TTI Rule Pattern and Mask to match packets with <pclId> is PRV_TGF_PCL_ID_CNS
        set UDB byte for the key keyType: anchor Metadata, offset 22
        set the TTI Rule Action for matched packet (pclId)
        enable the TTI lookup for keyType at the port 0
        set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType
        add TTI rule 1 with keyType on port 0 VLAN 5 with action: redirect to egress

    ======= Check default pclId tti no match =======
        send packet and expect no TTI match, and no traffic on port 36

    ======= Check configured pclId == tti rule pcl id, expect match =======
        save original pclId for restore
        set pclId for the TTI key keyType
        send packet and expect TTI match, and traffic on port 36

    ======= Check configured pclId != tti rule pcl id, expect no match =======
        set wrong pclId for the TTI key keyType
        send packet and expect no TTI match, and no traffic on port 36
        restore original pclId

    ======= Check configured pclId == tti rule pcl id =======
    ======= configured pclId keyType != tti rule pcl keyType, expect no match =======
        calculate keyType other than tested, but still one of ipv6 udb key types
        save original pclId for restore
        set correct pclId for ipv6 udb tti key other than in the TTI rule
        send packet and expect no TTI match, and no traffic on port 36
        restore original pclId for other key type

    ======= Check configured pclId == tti rule pcl id =======
    ======= configured pclId keyType == tti rule pcl keyType, expect match =======
        save original pclId for restore
        set correct pclId for ipv6 udb tti key same as in the TTI rule
        send packet and expect TTI match, and traffic on port 36
        restore original pclId

    test configuration restore
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv6PclId)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermIpv6PclId();
}

/* AUTODOC: Test functionality of TTI Dual Lookup: eVlan Precedence
            Test covers following fields:
                 tag0VlanPrecedence
                 tag0VlanCmd

    ------------------------------------------------------------
    AUTODOC: build ethernet packet with vid 5
    AUTODOC: create VLAN 5 with tagged ports [0,1]
    AUTODOC: create VLAN 6 with tagged ports [2,3]
    ------------------------------------------------------------
    AUTODOC: CASE 1:
    AUTODOC: set TTI0 rule with vlan precedence SOFT
    AUTODOC: set TTI1 rule with vlan precedence SOFT
    AUTODOC: set PCL rule with modify vlan to '0x06'
    AUTODOC: send packet, expect packet vlan changed by IPCL to '0x06' and a packet on port
    ------------------------------------------------------------
    AUTODOC: CASE 2:
    AUTODOC: set TTI0 rule vlan cmd to MODIFY
    AUTODOC: set TTI1 rule vlan cmd to MODIFY
    AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port
    ------------------------------------------------------------
    AUTODOC: CASE 3:
    AUTODOC: set TTI0 rule vlan precedence to HARD and Vlan Cmd to DO_NOT_MODIFY
    AUTODOC: set TTI1 rule Vlan Cmd to DO_NOT_MODIFY
    AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port
    ------------------------------------------------------------
    AUTODOC: CASE 4:
    AUTODOC: set TTI0 rule vlan precedence to SOFT
    AUTODOC: set TTI1 rule vlan precedence to HARD
    AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port
    ------------------------------------------------------------
    AUTODOC: CASE 5:
    AUTODOC: set TTI0 rule vlan precedence to HARD and Vlan Cmd to MODIFY_ALL
    AUTODOC: set TTI1 rule vlan precedence to SOFT and Vlan Cmd to MODIFY_ALL
    AUTODOC: send packet, expect packet vlan not changed by IPCL and no traffic on port
    ------------------------------------------------------------
    AUTODOC: CASE 6:
    AUTODOC: set TTI0 rule vlan precedence to SOFT
    AUTODOC: set TTI1 rule vlan precedence to HARD
    AUTODOC: send packet, expect packet vlan not changed by IPCL and no traffic on port
    ------------------------------------------------------------
    AUTODOC: CASE 7: Dual Lookup for Vlan cmd
    AUTODOC: set TTI0 rule vlan precedence to SOFT with Vlan MODIFY
    AUTODOC: set TTI1 rule vlan precedence to HARD with vlan DO_NOT_MODIFY
    AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port
    ------------------------------------------------------------

    AUTODOC: test configuration restore
*/

UTF_TEST_CASE_MAC(prvTgfTunnelTermEVlanPrecDualLookup)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelTermEVlanPrecDualLookup();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTunnelTermIpv6LocalDevSourceIsTrunk:
    test covers tti ipv6 metadata local dev source is trunk logic for ipv6 udb key types

    Following ipv6 udb TTI key types checked according scenario below:
        PRV_TGF_TTI_KEY_UDB_IPV6_E
        PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E
        PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E

    ======= Build correct IPv6 Packet =======

    ======= Set vlans configuration =======
        create VLAN 5 with untagged ports [0,1]
        create VLAN 6 with untagged ports [2,3]

    ======= Set TTI configuration =======
        add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with
        TTI action: modify vid to 6, redirect to port 36
        set the TTI Rule Pattern and Mask to match packets with local dev source is trunk
        set UDB byte for the key keyType: anchor Metadata, offset 22
        set the TTI Rule Action for matched packet
        enable the TTI lookup for keyType at the port 0
        set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType
        add TTI rule 1 with keyType on port 0 VLAN 5 with action: redirect to egress

    ======= Check default local dev source is trunk no match =======
        send packet and expect no TTI match, and no traffic on port 36

    ======= Check configured port member of trunk 7, expect match =======
        create TRUNK 7 with port 0 member (test send port)
        send packet and expect TTI match, and traffic on port 36

    test configuration restore
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv6LocalDevSourceIsTrunk)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermIpv6LocalDevSourceIsTrunk();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTunnelTermIpv6L3DataWord0FirstNibble:
    test covers tti ipv6 metadata L3 Data Word0 First Nibble logic for ipv6 udb key types

    Following ipv6 udb TTI key types checked according scenario below:
        PRV_TGF_TTI_KEY_UDB_IPV6_E
        PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E
        PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E

    ======= Build correct IPv6 Packet =======

    ======= Set vlans configuration =======
        create VLAN 5 with untagged ports [0,1]
        create VLAN 6 with untagged ports [2,3]

    ======= Set TTI configuration =======
        add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with
        TTI action: modify vid to 6, redirect to port 36
        set the TTI Rule Pattern and Mask to match packets with L3 Data Word0 First Nibble value 0xF
        set UDB byte for the key keyType: anchor Metadata, offset 21
        set the TTI Rule Action for matched packet
        enable the TTI lookup for keyType at the port 0
        set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType
        add TTI rule 1 with keyType on port 0 VLAN 5 with action: redirect to egress

    ======= Check default value of L3 Data Word0 First Nibble 0xF - match =======
        send packet and expect TTI match, and traffic on port 36

    ======= Check modified value of L3 Data Word0 First Nibble 0, expect no match =======
        send packet and expect no TTI match, and no traffic on port 36

    test configuration restore
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv6L3DataWord0FirstNibble)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermIpv6L3DataWord0FirstNibble();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId:
    test covers tti metadata local dev source ePort/trunkId for ipv4 and ipv6 keys

    Following udb TTI key types checked according scenario below:
        PRV_TGF_TTI_KEY_UDB_IPV6_E
        PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E
        PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E
        PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E
        PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E
        PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E
        PRV_TGF_TTI_KEY_UDB_IPV4_FRAGMENT_E

    ======= Build correct IPv6 Packet =======

    ======= Set vlans configuration =======
        create VLAN 5 with untagged ports [0,1]
        create VLAN 6 with untagged ports [2,3]

    ======= Set TTI configuration =======
        add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with
        TTI action: modify vid to 6, redirect to port 36
        set the TTI Rule Pattern and Mask to match packets with local dev source ePort 18
        set UDB byte for the key keyType: anchor Metadata, offset 26
        set the TTI Rule Action for matched packet
        enable the TTI lookup for keyType at the port 0
        set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType
        add TTI rule 1 with keyType on port 0 VLAN 5 with action: redirect to egress

    ======= Send packet from port 0, expect no TTI match =======
        send packet and expect no TTI match, and no traffic on port 36

    ======= Send packet from port 18, expect TTI match =======
        send packet and expect TTI match, and traffic on port 36

    test configuration restore
*/
UTF_TEST_CASE_MAC(tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfTunnelTermTtiDsaMetadata:
    Description:
        Test covers tti metadata dsa tag fields:
            SrcPort/SRCePortPORT/TrunkId
            SrcDev
            Source ID
            Source Is Trunk

    ======= build ethernet dsa-tagged packet=======

    ======= Set vlans configuration =======
        create VLAN 5 with tagged ports [0,1]
        create VLAN 6 with tagged ports [2,3]

    ======= Set TTI configuration =======
        set TTI rule not to match metadata SrcPort

    ===== CASE 1: Send packet with no TTI match. Show the packet is received on port ====
        send packet, expect TTI no match, and packet received on port

    ===== CASE 2: Test of DSA SrcPort/SRCePortPORT/TrunkId ====
        set TTI rule to match metadata SrcPort
        send packet, expect TTI match, and packet dropped

    ===== CASE 3: Test of DSA Tag SrcDev ====
        set TTI rule to match metadata SrcDev
        send packet, expect TTI match, and packet dropped

    ===== CASE 4: Test of DSA Source ID ====
        set TTI rule to match metadata SourceId
        send packet, expect TTI match, and packet dropped

    ===== CASE 5: Test Of DSA Tga Source Is Trunk ====
        set TTI rule to match metadata SourceIsTrunk
        send packet, expect TTI match, and packet dropped

    ===== test configuration restore =====
*/
UTF_TEST_CASE_MAC(prvTgfTunnelTermTtiDsaMetadata)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelTermTtiDsaMetadata();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfTunnelTermTtiDsaMetadata:
    Description:
        Test covers tti metadata ARP packet fields:
            SIP
            DIP

    ======= build ARP Packet =======

    ======= Set vlans configuration =======
        create VLAN 5 with untagged ports [0,1]
        create VLAN 6 with untagged ports [2,3]

    ======= Set TTI configuration =======
        add TTI rule with command HARD DROP and Pattern with invalid udbArray

    ===== CASE 0: Send packet with no TTI match. Show the packet is received on port ====

    ===== CASE 1-4: Send packet. Expect TTI match, metadata match for packet's SIP ====
        Expect packet dropped

    ===== CASE 5-8: Send packet. Expect TTI match, metadata match for packet's DIP ====
        Expect packet dropped

    ===== test configuration restore =====
*/
UTF_TEST_CASE_MAC(prvTgfTunnelTermTtiIpv4ArpUdb)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelTermTtiIpv4ArpUdb();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfTunnelTermTtiFullDb:
    Description:
        Test sets key size for UDB key to [10B / 20B / 30B] and fills TCAM
        with rules of size [10B / 20B / 30B] accordingly
    Note:
        Test work time is about 20 minutes on WM and hours on GM

    ======= build ethernet Packet =======

    ======= Set vlans configuration =======
        create VLAN 5 with tagged ports [0,1]
        create VLAN 6 with tagged ports [2,3]

    ====== CASE 1: Starting with rule size 10 ======
    ====== CASE 2: Starting with rule size 20 ======
    ====== CASE 3: Starting with rule size 30 ======
        Set TTI configuration:
            add TTI rules with rule size for key size 10B/20B/30B
        Send packets. Expect TTI match and traffic on port

    ===== test configuration restore =====
*/

UTF_TEST_CASE_MAC(prvTgfTunnelTermTtiFullDb)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    /* skip long test */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* skip this test for all families under Golden Model (due to many packets) */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelTermTtiFullDb();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfTunnelStartMplsVpws:
    Description:
        Test set TTI rules to match the incoming packets (tagged and untagged packets).
        Configure TTI to redirect packet to ePort.
        Set ePort to physical port configuration.
        Configure MPLS Tunnel Start on the egress ePort.
        Check packet arrive to the egress port with the MPLS label attached.
*/

UTF_TEST_CASE_MAC(csTunnelStartMplsVpws)
{
#ifdef CHX_FAMILY
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    csTunnelStartMplsVpwsRoutingTest();
#endif
}

/*----------------------------------------------------------------------------*/
    /* AUTODOC: test1: tunnel start unknown uc flooding */
    /* AUTODOC: MPLS configuration of device with 1 port in ETHERNET domain, and 3 ports in MPLS domain. */
    /* AUTODOC: test check 'flooding' from ethernet port to the other 3 mpls ports.(TS to the MPLS domain) */
    /* AUTODOC: ePort A is port to Ethernet domain */
    /* AUTODOC: eport B is port to MPLS domain with 1 label */
    /* AUTODOC: eport C is port to MPLS domain with 2 labels */
    /* AUTODOC: eport D is port to MPLS domain with 3 labels and PW label + FLOW label + CW label */

UTF_TEST_CASE_MAC(cstTunnelStartMplsUnknownUcFlooding)
{
/*
    #### CST code
    Set configuration

    ### TGF code - to test traffic
    Generate traffic
    Restore configuration
*/

#ifdef CHX_FAMILY

    /* verify this is bobCat2 */
    if ((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is not for PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    cstTunnelStartMplsUnknownUcFloodingTest();

#endif /* CHX_FAMILY */
}

/*----------------------------------------------------------------------------*/
    /* AUTODOC: test2: transit tunnel termination of known uc (2 labels manipulations)*/
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain.*/
    /* AUTODOC: test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)*/
    /* AUTODOC: ePort A is port to MPLS domain with 2 labels */
    /* AUTODOC: eport B is port to MPLS domain with 2 labels. Label 1 is swapped */
    /* AUTODOC: eport C is port to MPLS domain with 2 labels. Label 1 is popped & new label is pushed. */

    /* AUTODOC: test3: transit tunnel termination of known uc (2 labels manipulations)*/
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain.*/
    /* AUTODOC: test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)*/
    /* AUTODOC: ePort B is port to MPLS domain with 2 labels */
    /* AUTODOC: eport A is port to MPLS domain with 1 label. Label 1 is pop_swap */
    /* AUTODOC: eport D is port to MPLS domain with 1 labels. Label 1 is popped & swap */

UTF_TEST_CASE_MAC(cstTunnelTerminationTransitMplsKnownUc2Labels)
{
/*
    #### CST code
    Set configuration

    ### TGF code - to test traffic
    Generate traffic
    Restore configuration
*/

#ifdef CHX_FAMILY

    /* verify this is bobCat2 */
    if ((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is not for PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    cstTunnelTerminationTransitMplsKnownUc2LabelsTest();

#endif /* CHX_FAMILY */
}

/*----------------------------------------------------------------------------*/
    /* AUTODOC: test4: transit tunnel termination of known uc (3 labels manipulations) */
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain. */
    /* AUTODOC: test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain) */
    /* AUTODOC: ePort A is port to MPLS domain with 3 labels */
    /* AUTODOC: eport B is port to MPLS domain with 1 label. Labels 1,2 are pop2. */
    /* AUTODOC: eport C is port to MPLS domain with 1 label. Label 1 is pop1 & label 2 is pop1. */
    /* AUTODOC: eport D is port to MPLS domain with 1 label. Labels 1,2 are pop2 & label 2 is swapped. */

    /* AUTODOC: test5: transit tunnel termination of known uc (3 labels manipulations) */
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain. */
    /* AUTODOC: test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain) */
    /* AUTODOC: ePort B is port to MPLS domain with 3 labels */
    /* AUTODOC: eport C is port to MPLS domain with 2 label. Label 1 is pop1. */
    /* AUTODOC: eport D is port to MPLS domain with 2 label. Label 1,2 are pop2 & new label 2 is push. */
    /* AUTODOC: eport A is port to MPLS domain with 2 label. Label 1 is pop1 & label 2 is pop_swap. */


UTF_TEST_CASE_MAC(cstTunnelTerminationTransitMplsKnownUc3Labels)
{
/*
    #### CST code
    Set configuration

    ### TGF code - to test traffic
    Generate traffic
    Restore configuration
*/

#ifdef CHX_FAMILY

    /* verify this is bobCat2 */
    if ((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is not for PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    cstTunnelTerminationTransitMplsKnownUc3LabelsTest();

#endif /* CHX_FAMILY */
}

/*----------------------------------------------------------------------------*/
    /* AUTODOC: test6: transit tunnel termination of known uc (4 labels manipulations)*/
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain.*/
    /* AUTODOC: test check 4 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)*/
    /* AUTODOC: ePort A is port to MPLS domain with 4 labels.  */
    /* AUTODOC: eport B is port to MPLS domain with 1 label. Labels 1,2,3 are pop3 */
    /* AUTODOC: eport C is port to MPLS domain with 1 label. Label 1,2 are pop2 & label 3 is pop1 */
    /* AUTODOC: eport D is port to MPLS domain with 1 label. Label 1,2,3 are pop3 & swapped */


UTF_TEST_CASE_MAC(cstTunnelTerminationTransitMplsKnownUc4Labels)
{
/*
    #### CST code
    Set configuration

    ### TGF code - to test traffic
    Generate traffic
    Restore configuration
*/

#ifdef CHX_FAMILY

    /* verify this is bobCat2 */
    if ((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is not for PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    cstTunnelTerminationTransitMplsKnownUc4LabelsTest();

#endif /* CHX_FAMILY */
}



/*----------------------------------------------------------------------------*/
/* AUTODOC:  CST
    Test tunnel start IPv4 over GRE IPv4:
    configure VLAN, FDB entries;
    configure TS entry with GRE ethType;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(cstTunnelStartIpv4overGreIpv4)
{
/*
    #### CST code
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration

    ### TGF code - to test traffic
    4. Generate traffic
    5. Restore configuration
*/

#ifdef CHX_FAMILY

    GT_STATUS rc;

    /* verify this is bobCat2 */
    if ((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is not for PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }


    /* Configuration set */
    rc = cstTunnelStartIpv4overGreIpv4ConfigSet();

    if(GT_OK != rc)
    {
        cpssOsPrintf("can't set test configuration");
    }
    else
    {
        /* Generate traffic */
        prvTgfTunnelStartIpv4overGreIpv4TrafficGenerate();
    }

    /* Restore configuration */
    prvTgfTunnelStartIpv4overGreIpv4ConfigurationRestore(0);

    prvTgfPclRestore();


#endif /* CHX_FAMILY */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:  CST
    Test tunnel term VrfId
    configure VLAN, TTI,
    configure TS entry with GRE ethType;
    send traffic to match TS entry;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(cstTunnelTermVrfId)
{
/*
    #### CST code
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration

    ### TGF code - to test traffic
    4. Generate traffic
    5. Restore configuration
*/

#ifdef CHX_FAMILY

    GT_STATUS rc;

    /* verify this is bobCat2 */
    if ((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is not for PBR mode */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }


    /* Configuration set */
    rc = cstTunnelTermVrfIdConfigSet();

    if(GT_OK != rc)
    {
        cpssOsPrintf("can't set test configuration");
    }
    else
    {
        /* Generate traffic */
        prvTgfTunnelStartIpv4overGreIpv4TrafficGenerate();
    }

    rc = cstTunnelTermVrfIdConfigRestore();
    if(GT_OK != rc)
    {
        cpssOsPrintf("can't restore test configuration");
    }

#endif /* CHX_FAMILY */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfTunnelCapwapTest:

    Set configuration:

        create VLAN 10 with tagged ports [0,1]
        add FDB entry with MAC 01:02:03:34:02 , VLAN 10, port 58
        enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E at the port 0
        set the TTI Rule Pattern, Mask and Action for Mpls TCAM location
        Set a tunnel start entry
        add TS entry 8 with:
          tunnelType = GENERIC_IPV4
          vlanId=6, ttl=33
          DA=88:77:11:11:55:66, srcIp=2.2.2.2, dstIp=1.1.1.3
        configure MAC SA to global MAC SA table into index 7
        configure index to global MAC SA table in eport attribute table
        set the egress eport to use TS for the ethernet passenger
        configure destination e-port properties
        get default Generic IPv4 UDP Template packet

    -------------------------
    Generating Traffic
    Check captured egress traffic on port

    -------------------------
    Restore Configuration
*/

UTF_TEST_CASE_MAC(prvTgfTunnelCapwapTest)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelCapwapTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfTunnelIpv6CapwapTest:

    Set configuration:

        create VLAN 10 with tagged ports [0,1]
        add FDB entry with MAC 01:02:03:34:02 , VLAN 10, port 58
        enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E at the port 0
        set the TTI Rule Pattern, Mask and Action for Mpls TCAM location
        Set a tunnel start entry
        add TS entry 8 with:
          tunnelType = GENERIC_IPV4
          vlanId=6, ttl=33
          DA=88:77:11:11:55:66, srcIp={0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212},
          ,dstIp={0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}
        configure MAC SA to global MAC SA table into index 7
        configure index to global MAC SA table in eport attribute table
        set the egress eport to use TS for the ethernet passenger
        configure destination e-port properties
        get default Generic IPv6 UDP Template packet

    -------------------------
    Generating Traffic
    Check captured egress traffic on port

    -------------------------
    Restore Configuration
*/

UTF_TEST_CASE_MAC(prvTgfTunnelIpv6CapwapTest)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelIpv6CapwapTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTunnelMplsControlWord
    Set configuration:

        create VLAN 10 with tagged ports [0,1]
        add FDB entry with MAC 01:02:03:34:02 , VLAN 10, port 58
        enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E at the port 0
        set the TTI Rule Pattern, Mask and Action for Mpls TCAM location
        Set a tunnel start entry
        add TS entry 8 with:
          tunnelType = GENERIC_IPV4
          vlanId=6, ttl=33
          DA=88:77:11:11:55:66, srcIp=2.2.2.2, dstIp=1.1.1.3
          controlWordIndex = 1
        configure MAC SA to global MAC SA table into index 7
        configure index to global MAC SA table in eport attribute table
        set the egress eport to use TS for the ethernet passenger
        configure destination e-port properties
        get default Generic IPv4 UDP Template packet

    -------------------------
    Generating Traffic
    Check if captured egress traffic on port received with MPLS control word

    -------------------------
    Restore Configuration
*/
UTF_TEST_CASE_MAC(tgfTunnelMplsControlWord)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelMplsControlWord();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfTunnelIpv6MplsControlWord:

    Set configuration:

        create VLAN 10 with tagged ports [0,1]
        add FDB entry with MAC 01:02:03:34:02 , VLAN 10, port 58
        enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E at the port 0
        set the TTI Rule Pattern, Mask and Action for Mpls TCAM location
        Set a tunnel start entry
        add TS entry 8 with:
          tunnelType = GENERIC_IPV6
          vlanId=6, ttl=33
          DA=88:77:11:11:55:66, srcIp={0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212},
          ,dstIp={0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}
          controlWordIndex = 1
        configure MAC SA to global MAC SA table into index 7
        configure index to global MAC SA table in eport attribute table
        set the egress eport to use TS for the ethernet passenger
        configure destination e-port properties
        get default Generic IPv6 UDP Template packet

    -------------------------
    Generating Traffic
    Check if captured egress traffic on port has MPLS control word

    -------------------------
    Restore Configuration
*/
UTF_TEST_CASE_MAC(tgfTunnelIpv6MplsControlWord)
{
    /* Supported by eArch devices */
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    prvTgfTunnelIpv6MplsControlWordTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test tunnel IP ECN resolving:
    configure VLAN, FDB entries;
    configure TS entry with GRE ethType;
    configure TTI rule with tunnel-termination action;
    configure ECN mormal mode;
    send traffic;
    verify tunneled traffic on nextHop port;
    configure ECN compatibility mode;
    verify tunneled traffic on nextHop port.
*/
UTF_TEST_CASE_MAC(tgfTunnelEcn)
{
/*
    1. Set Base configuration
    2. Set Tunnel configuration
    2. Set Route configuration
    3. Set TTI configuration
    4. Generate traffic with enabled ECN normal mode
    5. Generate traffic with enabled ECN compatibility mode
    5. Restore configuration
*/
    GT_U32      prvUtfVrfId = 0;
    GT_U32      notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set Base configuration */
    prvTgfTunnelIpv4EcnBaseConfigurationSet(prvUtfVrfId, 0);

    /* Set Tunnel configuration */
    prvTgfTunnelIpv4EcnIpv4TunnelConfigurationSet();

    /* Set Route configuration */
    prvTgfTunnelIpv4EcnRouteConfigurationSet();

    /* Set TTI configuration */
    prvTgfTunnelIpv4EcnTtiConfigurationSet(0);

    /* Test normal ECN mode */
    prvTgfTunnelIpv4EcnTest(PRV_TGF_TUNNEL_START_ECN_NORMAL_MODE_E);

    /* Test compatibility ECN mode */
    prvTgfTunnelIpv4EcnTest(PRV_TGF_TUNNEL_START_ECN_COMPATIBILITY_MODE_E);

    prvTgfTunnelIpv4EcnConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test SIP6 TTI metadata fields:
    configure VLAN;
    configure 2 TTI rules with trap to cpu action;
    send traffic with two extension headers and UDP protocol;
    expect matching rule with condition where IPvX protocol 0x11;
    verify CPU code;
    send traffic with next header value 0x01;
    expect matching rule with condition where IPvX protocol 0x01;
    verify CPU code;
    restore configuration;
*/
UTF_TEST_CASE_MAC(tgfTunnelTermSip6Metadata)
{
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E) ;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTunnelTermSip6Metadata();
}

/*
 * Configuration of tgfTunnel suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfTunnel)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelCapwapTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelIpv6CapwapTest)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartGenericIpv4UdpTemplate)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartGenericIpv4GenericIpHeaderTemplate)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartGenericIpv6UdpTemplate)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartGenericIpv6GenericIpHeaderTemplate)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv6overIpv4)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4overIpv4)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4overIpv4EArch)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4overIpv4RedirectToLTT)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4overIpv4Invalidate)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4overIpv4Trap)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTunnelHeaderStart)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4overIpv4SinglePortGroup)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrPop1Label)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrPop2Label)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrPopLabelTtlParallelLookup)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermMimBasic)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermMimPassengerOuterTag0or1)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherType)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermUseMacSaInMimTtiLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermMaskMacInMimTtiLookup)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTag1CmdField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermExpQosField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelExpQosFieldNonTerm)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelExpQosFieldNonTermPop3)


    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrustAnyQosField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrustDscpQosField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrustUpQosField)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrustAnyQosField_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermExpQosField_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelExpQosFieldNonTerm_qosMappingTableIterator)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrustAnyQosField_qosMappingTableIterator)
/*    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrustDscpQosField_qosMappingTableIterator)*/
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrustUpQosField_qosMappingTableIterator)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermGlobalPort2LocalPortConf1)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermGlobalPort2LocalPortConf2)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermGlobalPort2LocalPortConf3)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverIpv4RedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverIpv4RedirectToEgressEport)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgress_DSATag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgress_retainCRC)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgressEport)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgressEport_egressQosDscpRemark)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartEtherOverIpv4GreRedirectToEgressOffset)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverIpv4GreRedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverIpv4NoRedirectBridging)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelIpEtherOverIpv4Gre)

    UTF_SUIT_DECLARE_TEST_MAC(tgfEtherRedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfEtherRedirectToEgressWithTS)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartIpv4overGreIpv4)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartIpv6overIpv4)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelEcn)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartUpMarkingMode)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartMimEtherType)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartIdeiMarkingMode)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartMimISidAndBdaIssignFromTsEntry)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartMimISidIssignFromEVlanServiceId)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartMimBdaIssignFromTsEntryAndVlan)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartGeneric)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMimStartFromTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMimStartFromPcl)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrPush1Label)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrPush2Label)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrPush3Label)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsPushAndEgressQosMapping)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartMplsEgressQosMappingPassengerEthernet)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartMplsModifyOuterVlanTagPassengerEthernet)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrSwap)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsLsrPopSwap)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTrillNativeEthernetToTunnelStartUsingFdbForward)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTrillKnownUcTransit)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTrillKnownUcDataToTti)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTrillKnownUcDataToTti_fromCascadePortDsaTagForward)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartTrillUnknownUcFlooding)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTermTrillUnknownUcTransitAndTermination)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartTrillKnownUcEcmp)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelStartTrillKnownMc)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrillExceptionsCheck)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverIpv4GreMacSecRedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverIpv4GreRedirectToEgressTtHeaderLength)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverMplsRedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverMplsRedirectToEgress_fromCascadePort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverMplsMplsLabelsRedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermGreExtensions)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4McEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverMplsUdbRedirectToEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverMplsPwLabel)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermEtherOverMplsPwLabelParallelLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermParallelLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermParallelLookupRedirect)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermExceptionCpuCode)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermMirrorToIngressAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermSetMac2Me)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermLlcNonSnapType)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermSetIPclUdbConfigTableIndex)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermTrigReqMpls)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermSourceIdModify)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermSourceEportAssignment)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermCncParallelLookup)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv6ExtensionHeaderSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelPwTtlExpiryVccvException)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelPwCwBasedETree)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermMapMplsChannelToOamOpcode)
    UTF_SUIT_DECLARE_TEST_MAC(prfTgfTunnelDualLookupNestedVLan)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermSetIPclOverrideConfigTableIndex)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTermTSPassengerTypeDualLookup)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv6HeaderExceptions)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermSip6Metadata)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTermPbrDualLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv6PclId)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTermEVlanPrecDualLookup)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv6LocalDevSourceIsTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv6L3DataWord0FirstNibble)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTermTtiDsaMetadata)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTermTtiIpv4ArpUdb)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTunnelTermTtiFullDb)
    UTF_SUIT_DECLARE_TEST_MAC(cstTunnelStartIpv4overGreIpv4)
    UTF_SUIT_DECLARE_TEST_MAC(cstTunnelTermVrfId)
    UTF_SUIT_DECLARE_TEST_MAC(csTunnelStartMplsVpws)


    UTF_SUIT_DECLARE_TEST_MAC(cstTunnelStartMplsUnknownUcFlooding)
    UTF_SUIT_DECLARE_TEST_MAC(cstTunnelTerminationTransitMplsKnownUc2Labels)
    UTF_SUIT_DECLARE_TEST_MAC(cstTunnelTerminationTransitMplsKnownUc3Labels)
    UTF_SUIT_DECLARE_TEST_MAC(cstTunnelTerminationTransitMplsKnownUc4Labels)

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelMplsControlWord)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelIpv6MplsControlWord)


#if 0
    /* tgfTunnelTcamCheckLookupOrder temporary commented out
        due to the issues with test configuration restore:
        TO FIX tcam rules invalidation */

    UTF_SUIT_DECLARE_TEST_MAC(tgfTunnelTcamCheckLookupOrder)
#endif

UTF_SUIT_END_TESTS_MAC(tgfTunnel)


