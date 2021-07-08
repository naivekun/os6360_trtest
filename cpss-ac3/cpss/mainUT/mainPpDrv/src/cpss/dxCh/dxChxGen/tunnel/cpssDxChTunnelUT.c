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
* @file cpssDxChTunnelUT.c
*
* @brief Unit tests for cpssDxChTunnel, that provides
* CPSS tunnel implementation.
*
* @version   65
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define TUNNEL_VALID_PHY_PORT_CNS  0

/* maximum index */
#define MAX_INDEX_CNS 14

/* maximum index for XCAT C0 and XCAT3 devices*/
#define MAX_INDEX_FOR_VPLS_CNS 6

/* maximum index for control word inxed in Generic IP Profile Table */
#define MAX_GENERIC_IP_PROFILE_TABLE_CONTROL_WORD_INDEX_CNS 7

/* maximum value for service-ID circular shift size in Generic IP Profile Table */
#define MAX_GENERIC_IP_PROFILE_SERVICE_ID_CIRCULAR_SIFT_SIZE_CNS 23

/* maximum value for the Tunnel-start MAC DA mode in Generic IP Profile Table */
#define MAX_GENERIC_IP_PROFILE_MAC_DA_MODE_CNS 24

/* maximum value for the Tunnel-start desination IP mode in Generic IP Profile Table */
#define MAX_GENERIC_IP_PROFILE_DIP_MODE_CNS 24

/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

static GT_STATUS prvCompareTunnelStartConfigStructs
(
    IN GT_U8                              devNum,
    IN GT_CHAR                            *name,
    IN CPSS_TUNNEL_TYPE_ENT               tunnelType,
    IN CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *setConfigPtr,
    IN CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *getConfigPtr
);

/* max index Control Word for XCAT C0 and XCAT3 or SIP5 */
#define MAX_INDEX_CONTROL_WORD(_dev)  \
     ((PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(_dev) == GT_TRUE)) ? 6 : 14

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              routerArpTunnelStartLineIndex,
    IN  CPSS_TUNNEL_TYPE_ENT                tunnelType,
    IN  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   *configPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEntrySet)
{
    GT_STATUS                         st         = GT_OK;

    GT_U8                             dev;
    GT_U32                            lineIndex  = 0;
    CPSS_TUNNEL_TYPE_ENT              tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
    CPSS_TUNNEL_TYPE_ENT              tunnelTypeGet = CPSS_TUNNEL_X_OVER_IPV4_E;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT config, configTmp;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT configGet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        /* Call with tunnelType [CPSS_TUNNEL_X_OVER_IPV4_E] */
        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        else
            tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

        config.ipv4Cfg.tagEnable    = GT_TRUE;
        config.ipv4Cfg.vlanId       = 100;
        config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up           = 0;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp         = 0;
        config.ipv4Cfg.cfi          = 0;
        config.ipv4Cfg.retainCRC    = GT_FALSE;

        config.ipv4Cfg.macDa.arEther[0] = 0x0;
        config.ipv4Cfg.macDa.arEther[1] = 0x1A;
        config.ipv4Cfg.macDa.arEther[2] = 0xFF;
        config.ipv4Cfg.macDa.arEther[3] = 0xFF;
        config.ipv4Cfg.macDa.arEther[4] = 0xFF;
        config.ipv4Cfg.macDa.arEther[5] = 0xFF;

        config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
        config.ipv4Cfg.ttl              = 0;
        config.ipv4Cfg.autoTunnel       = GT_TRUE;
        config.ipv4Cfg.autoTunnelOffset = 1;

        config.ipv4Cfg.destIp.arIP[0] = 10;
        config.ipv4Cfg.destIp.arIP[1] = 15;
        config.ipv4Cfg.destIp.arIP[2] = 1;
        config.ipv4Cfg.destIp.arIP[3] = 250;

        config.ipv4Cfg.srcIp.arIP[0] = 10;
        config.ipv4Cfg.srcIp.arIP[1] = 15;
        config.ipv4Cfg.srcIp.arIP[2] = 1;
        config.ipv4Cfg.srcIp.arIP[3] = 255;

        config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
        config.ipv4Cfg.profileIndex           = 3;
        config.ipv4Cfg.greProtocolForEthernet = 0x0800;
        config.ipv4Cfg.greFlagsAndVersion     = 0xFFFF;
        config.ipv4Cfg.udpDstPort             = 0;
        config.ipv4Cfg.udpSrcPort             = 0;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);
        }

        /*  1.1. */
        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        else
            tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;

        config.ipv4Cfg.tagEnable    = GT_TRUE;
        config.ipv4Cfg.vlanId       = 4095;
        config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up           = 3;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp         = 63;
        config.ipv4Cfg.cfi          = 1;
        config.ipv4Cfg.retainCRC    = GT_TRUE;

        config.ipv4Cfg.macDa.arEther[0] = 0xFF;
        config.ipv4Cfg.macDa.arEther[1] = 0xFF;
        config.ipv4Cfg.macDa.arEther[2] = 0xFF;
        config.ipv4Cfg.macDa.arEther[3] = 0xFF;
        config.ipv4Cfg.macDa.arEther[4] = 0x01;
        config.ipv4Cfg.macDa.arEther[5] = 0x10;

        config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
        config.ipv4Cfg.ttl              = 255;
        config.ipv4Cfg.autoTunnel       = GT_FALSE;
        config.ipv4Cfg.autoTunnelOffset = 0;

        config.ipv4Cfg.destIp.arIP[0] = 255;
        config.ipv4Cfg.destIp.arIP[1] = 255;
        config.ipv4Cfg.destIp.arIP[2] = 255;
        config.ipv4Cfg.destIp.arIP[3] = 255;

        config.ipv4Cfg.srcIp.arIP[0] = 255;
        config.ipv4Cfg.srcIp.arIP[1] = 255;
        config.ipv4Cfg.srcIp.arIP[2] = 255;
        config.ipv4Cfg.srcIp.arIP[3] = 255;

        config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
        config.ipv4Cfg.profileIndex           = 3;
        config.ipv4Cfg.greProtocolForEthernet = 0xFFFF;
        config.ipv4Cfg.greFlagsAndVersion     = 0x0101;
        config.ipv4Cfg.udpDstPort             = 0;
        config.ipv4Cfg.udpSrcPort             = 0;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);
        }

        if((PRV_CPSS_SIP_5_15_CHECK_MAC(dev)==GT_TRUE)||
        (PRV_CPSS_SIP_5_16_CHECK_MAC(dev)==GT_TRUE)||
         (PRV_CPSS_SIP_5_20_CHECK_MAC(dev)==GT_TRUE))
    {
         /*  1.1. */
        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 8;

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        else
            tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

        config.ipv4Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
        config.ipv4Cfg.ipProtocol = 0x34;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);
        }
     }

        /*  1.1. */
        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        else
            tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
        config.ipv4Cfg.tagEnable    = GT_FALSE;
        config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up           = 3;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp         = 32;
        config.ipv4Cfg.cfi          = 1;
        config.ipv4Cfg.retainCRC    = GT_TRUE;

        config.ipv4Cfg.macDa.arEther[0] = 0xFF;
        config.ipv4Cfg.macDa.arEther[1] = 0xFF;
        config.ipv4Cfg.macDa.arEther[2] = 0xFF;
        config.ipv4Cfg.macDa.arEther[3] = 0xFF;
        config.ipv4Cfg.macDa.arEther[4] = 0x01;
        config.ipv4Cfg.macDa.arEther[5] = 0x10;

        config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
        config.ipv4Cfg.ttl              = 128;
        config.ipv4Cfg.autoTunnel       = GT_FALSE;
        config.ipv4Cfg.autoTunnelOffset = 0;

        config.ipv4Cfg.destIp.arIP[0] = 255;
        config.ipv4Cfg.destIp.arIP[1] = 255;
        config.ipv4Cfg.destIp.arIP[2] = 255;
        config.ipv4Cfg.destIp.arIP[3] = 255;

        config.ipv4Cfg.srcIp.arIP[0] = 255;
        config.ipv4Cfg.srcIp.arIP[1] = 255;
        config.ipv4Cfg.srcIp.arIP[2] = 255;
        config.ipv4Cfg.srcIp.arIP[3] = 255;

        config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        config.ipv4Cfg.profileIndex           = 7;
        config.ipv4Cfg.greProtocolForEthernet = 0;
        config.ipv4Cfg.greFlagsAndVersion     = 0;
        config.ipv4Cfg.udpDstPort             = 0xFFFF;
        config.ipv4Cfg.udpSrcPort             = 0xFFFE;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);
        }

        /* 1.2. */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        else
            tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }

        /*  1.3. */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

            config.ipv4Cfg.retainCRC            = GT_TRUE;
            config.ipv4Cfg.ipHeaderProtocol     = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
            /* profile index 7 for legacy support(empty profile: no extension data is added in tunnel header)*/
            config.ipv4Cfg.profileIndex         = 7;
            /*If the passenger is IPv4, the GRE protocol is fixed: 0x0800*/
            config.ipv4Cfg.greProtocolForEthernet   = 0x0800;
            config.ipv4Cfg.greFlagsAndVersion       = 0;
            /*not relevant*/
            config.ipv4Cfg.udpDstPort               = 0;
            config.ipv4Cfg.udpSrcPort               = 0;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            /*  1.3.1. */
            config.ipv4Cfg.tagEnable  = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv4Cfg.ipHeaderProtocol);

            /*  1.3.2. */
            config.ipv4Cfg.profileIndex = 8;
            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->ipv4Cfg.tagEnable = %d, configPtr->ipv4Cfg.profileIndex = %d",
                            dev, config.ipv4Cfg.tagEnable, config.ipv4Cfg.profileIndex);
        }

        /*  1.4.  */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
            config.ipv4Cfg.profileIndex = 7;
        }
        else
        {
            tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
        }

        config.ipv4Cfg.destIp.arIP[0] = 255;
        config.ipv4Cfg.destIp.arIP[1] = 255;
        config.ipv4Cfg.destIp.arIP[2] = 255;
        config.ipv4Cfg.destIp.arIP[3] = 255;

        config.ipv4Cfg.srcIp.arIP[0] = 255;
        config.ipv4Cfg.srcIp.arIP[1] = 255;
        config.ipv4Cfg.srcIp.arIP[2] = 255;
        config.ipv4Cfg.srcIp.arIP[3] = 251;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }


        /*  1.5.  */
        configTmp = config;
        tunnelType = CPSS_TUNNEL_TRILL_E;
        config.trillCfg.tagEnable = GT_FALSE;
        config.trillCfg.trillHeader.version = 0;
        config.trillCfg.trillHeader.opLength = 0;
        config.trillCfg.trillHeader.hopCount = 0;
        config.trillCfg.trillHeader.eRbid = 2;
        config.trillCfg.trillHeader.iRbid = 1;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }

        /* restore valid configuration */
        config = configTmp;

        /*  1.6.  */
        tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
        config.ipv4Cfg.destIp.arIP[0] = 10;
        config.ipv4Cfg.destIp.arIP[1] = 15;
        config.ipv4Cfg.destIp.arIP[2] = 1;
        config.ipv4Cfg.destIp.arIP[3] = 250;

        config.ipv4Cfg.srcIp.arIP[0] = 10;
        config.ipv4Cfg.srcIp.arIP[1] = 15;
        config.ipv4Cfg.srcIp.arIP[2] = 1;
        config.ipv4Cfg.srcIp.arIP[3] = 255;

        config.ipv4Cfg.tagEnable = GT_TRUE;
        config.ipv4Cfg.vlanId    = 4096;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
            "%d, configPtr->ipv4Cfg.tagEnable = %d, configPtr->ipv4Cfg.vlanId = %d",
                        dev, config.ipv4Cfg.tagEnable, config.ipv4Cfg.vlanId);

         /* Call with tunnelType [CPSS_TUNNEL_GENERIC_IPV4_E] */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
            "%d, configPtr->ipv4Cfg.tagEnable = %d, configPtr->ipv4Cfg.vlanId = %d",
                        dev, config.ipv4Cfg.tagEnable, config.ipv4Cfg.vlanId);
        }

        /* restore */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        else
            tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
        config.ipv4Cfg.vlanId = 100;

        /*  1.7.  */
        config.ipv4Cfg.vlanId     = 100;
        config.ipv4Cfg.tagEnable  = GT_TRUE;

        if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv4Cfg.upMarkMode);
        }

        /*  1.8.  */
        config.ipv4Cfg.tagEnable  = GT_TRUE;
        config.ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up         = 8;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
            "%d, configPtr->ipv4Cfg.tagEnable = %d, ->upMarkMode = %d, ->up = %d",
            dev, config.ipv4Cfg.tagEnable, config.ipv4Cfg.upMarkMode, config.ipv4Cfg.up);

        config.ipv4Cfg.up = 0;

        /*  1.9.  */
        config.ipv4Cfg.tagEnable    = GT_TRUE;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp         = 64;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
            "%d, configPtr->ipv4Cfg.tagEnable = %d, ->dscpMarkMode = %d, ->dscp = %d",
            dev, config.ipv4Cfg.tagEnable, config.ipv4Cfg.dscpMarkMode, config.ipv4Cfg.dscp);

        config.ipv4Cfg.dscp = 0;

        /*  1.10.  */
        config.ipv4Cfg.ttl = 256;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv4Cfg.ttl = %d",
                                         dev, config.ipv4Cfg.ttl);

        config.ipv4Cfg.ttl = 0;

        /*  1.11.  */
        config.ipv4Cfg.autoTunnel       = GT_TRUE;
        config.ipv4Cfg.autoTunnelOffset = 16;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
            "%d, tunnelType = %d, configPtr->ipv4Cfg.autoTunnelOffset = %d, ->autoTunnel = %d",
            dev, tunnelType, config.ipv4Cfg.autoTunnelOffset, config.ipv4Cfg.autoTunnel);


        config.ipv4Cfg.autoTunnelOffset = 1;

        /*  1.12.  */
        /* Call with tunnelType [CPSS_TUNNEL_GENERIC_IPV4_E] */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);

            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv4Cfg.ipHeaderProtocol);
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /*  1.13. */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.vlanId       = 100;
            config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.up           = 0;
            config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.dscp         = 0;
            config.ipv4Cfg.cfi          = 0;
            config.ipv4Cfg.retainCRC    = GT_FALSE;

            config.ipv4Cfg.macDa.arEther[0] = 0x0;
            config.ipv4Cfg.macDa.arEther[1] = 0x1A;
            config.ipv4Cfg.macDa.arEther[2] = 0xFF;
            config.ipv4Cfg.macDa.arEther[3] = 0xFF;
            config.ipv4Cfg.macDa.arEther[4] = 0xFF;
            config.ipv4Cfg.macDa.arEther[5] = 0xFF;

            config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
            config.ipv4Cfg.ttl              = 0;
            config.ipv4Cfg.autoTunnel       = GT_TRUE;
            config.ipv4Cfg.autoTunnelOffset = 1;

            config.ipv4Cfg.destIp.arIP[0] = 10;
            config.ipv4Cfg.destIp.arIP[1] = 15;
            config.ipv4Cfg.destIp.arIP[2] = 1;
            config.ipv4Cfg.destIp.arIP[3] = 250;

            config.ipv4Cfg.srcIp.arIP[0] = 10;
            config.ipv4Cfg.srcIp.arIP[1] = 15;
            config.ipv4Cfg.srcIp.arIP[2] = 1;
            config.ipv4Cfg.srcIp.arIP[3] = 255;

            config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
            config.ipv4Cfg.profileIndex           = 3;
            config.ipv4Cfg.greProtocolForEthernet = 0x0800;
            config.ipv4Cfg.greFlagsAndVersion     = 0xFFFF;
            config.ipv4Cfg.udpDstPort             = 0;
            config.ipv4Cfg.udpSrcPort             = 0;


            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);


            /*  1.14. */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.vlanId       = 4095;
            config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.up           = 3;
            config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.dscp         = 63;
            config.ipv4Cfg.cfi          = 1;
            config.ipv4Cfg.retainCRC    = GT_TRUE;

            config.ipv4Cfg.macDa.arEther[0] = 0xFF;
            config.ipv4Cfg.macDa.arEther[1] = 0xFF;
            config.ipv4Cfg.macDa.arEther[2] = 0xFF;
            config.ipv4Cfg.macDa.arEther[3] = 0xFF;
            config.ipv4Cfg.macDa.arEther[4] = 0x01;
            config.ipv4Cfg.macDa.arEther[5] = 0x10;

            config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
            config.ipv4Cfg.ttl              = 255;
            config.ipv4Cfg.autoTunnel       = GT_FALSE;
            config.ipv4Cfg.autoTunnelOffset = 0;

            config.ipv4Cfg.destIp.arIP[0] = 255;
            config.ipv4Cfg.destIp.arIP[1] = 255;
            config.ipv4Cfg.destIp.arIP[2] = 255;
            config.ipv4Cfg.destIp.arIP[3] = 255;

            config.ipv4Cfg.srcIp.arIP[0] = 255;
            config.ipv4Cfg.srcIp.arIP[1] = 255;
            config.ipv4Cfg.srcIp.arIP[2] = 255;
            config.ipv4Cfg.srcIp.arIP[3] = 255;

            config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
            config.ipv4Cfg.profileIndex           = 3;
            config.ipv4Cfg.greProtocolForEthernet = 0xFFFF;
            config.ipv4Cfg.greFlagsAndVersion     = 0x0101;
            config.ipv4Cfg.udpDstPort             = 0;
            config.ipv4Cfg.udpSrcPort             = 0;


            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);


            /*  1.15. */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

            config.ipv4Cfg.tagEnable    = GT_FALSE;
            config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.up           = 3;
            config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.dscp         = 32;
            config.ipv4Cfg.cfi          = 1;
            config.ipv4Cfg.retainCRC    = GT_TRUE;

            config.ipv4Cfg.macDa.arEther[0] = 0xFF;
            config.ipv4Cfg.macDa.arEther[1] = 0xFF;
            config.ipv4Cfg.macDa.arEther[2] = 0xFF;
            config.ipv4Cfg.macDa.arEther[3] = 0xFF;
            config.ipv4Cfg.macDa.arEther[4] = 0x01;
            config.ipv4Cfg.macDa.arEther[5] = 0x10;

            config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
            config.ipv4Cfg.ttl              = 128;
            config.ipv4Cfg.autoTunnel       = GT_FALSE;
            config.ipv4Cfg.autoTunnelOffset = 0;

            config.ipv4Cfg.destIp.arIP[0] = 255;
            config.ipv4Cfg.destIp.arIP[1] = 255;
            config.ipv4Cfg.destIp.arIP[2] = 255;
            config.ipv4Cfg.destIp.arIP[3] = 255;

            config.ipv4Cfg.srcIp.arIP[0] = 255;
            config.ipv4Cfg.srcIp.arIP[1] = 255;
            config.ipv4Cfg.srcIp.arIP[2] = 255;
            config.ipv4Cfg.srcIp.arIP[3] = 255;

            config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
            config.ipv4Cfg.profileIndex           = 7;
            config.ipv4Cfg.greProtocolForEthernet = 0;
            config.ipv4Cfg.greFlagsAndVersion     = 0;
            config.ipv4Cfg.udpDstPort             = 0xFFFF;
            config.ipv4Cfg.udpSrcPort             = 0xFFFE;


            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            /*  1.16.  */
            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.vlanId       = 4096;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.vlanId       = 0;

            /*  1.17.  */
            config.ipv4Cfg.tagEnable    = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv4Cfg.upMarkMode);

            /*  1.18.  */
            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.up           = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.up       = 0;

            /*  1.19.  */
            config.ipv4Cfg.tagEnable    = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv4Cfg.dscpMarkMode);

            /*  1.20.  */
            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.dscp         = 64;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.dscp       = 0;

            /*  1.21.  */
            config.ipv4Cfg.cfi         = 2;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.cfi         = 0;

            /*  1.22.  */
            config.ipv4Cfg.ttl         = 256;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.ttl         = 0;

            /*  1.23.  */
            config.ipv4Cfg.autoTunnel        = GT_TRUE;
            config.ipv4Cfg.autoTunnelOffset  = 13;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.autoTunnelOffset         = 0;

            /*  1.24.  */
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv4Cfg.ipHeaderProtocol);

            /*  1.25.  */
            config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
            config.ipv4Cfg.profileIndex  = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.profileIndex         = 0;

            /*  1.26.  */
            config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
            config.ipv4Cfg.greProtocolForEthernet  = 0x10000;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.greProtocolForEthernet         = 0;

            /*  1.27.  */
            config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
            config.ipv4Cfg.greFlagsAndVersion  = 0x10000;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.greFlagsAndVersion         = 0;

            /*  1.28.  */
            config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
            config.ipv4Cfg.udpDstPort  = 0x10000;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.udpDstPort         = 0;

            /*  1.29.  */
            config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
            config.ipv4Cfg.udpSrcPort  = 0x10000;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            config.ipv4Cfg.udpSrcPort         = 0;
        }

        /*  1.30.  */
        lineIndex  = 10;
        cpssOsBzero((GT_VOID*)&config, sizeof(config));
        tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

        config.mplsCfg.tagEnable  = GT_TRUE;
        config.mplsCfg.vlanId     = 100;
        config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.up         = 7;
        config.mplsCfg.cfi        = 0;

        config.mplsCfg.macDa.arEther[0] = 0x0;
        config.mplsCfg.macDa.arEther[1] = 0x1A;
        config.mplsCfg.macDa.arEther[2] = 0xFF;
        config.mplsCfg.macDa.arEther[3] = 0xFF;
        config.mplsCfg.macDa.arEther[4] = 0xFF;
        config.mplsCfg.macDa.arEther[5] = 0xFF;

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            config.mplsCfg.numLabels    = 3;
        }else
        {
            config.mplsCfg.numLabels    = 2;
        }

        config.mplsCfg.ttl          = 10;
        config.mplsCfg.label1       = 100;
        config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp1         = 7;
        config.mplsCfg.label2       = 200;
        config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp2         = 7;

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            config.mplsCfg.label3       = 300;
            config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.exp3         = 7;
            config.mplsCfg.setSBit      = GT_TRUE;
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
            {
                config.mplsCfg.pushEliAndElAfterLabel1 = GT_TRUE;
                config.mplsCfg.pushEliAndElAfterLabel2 = GT_TRUE;
                config.mplsCfg.pushEliAndElAfterLabel3 = GT_TRUE;
            }
        }

        config.mplsCfg.retainCRC    = GT_FALSE;
        config.mplsCfg.controlWordEnable = GT_FALSE;
        config.mplsCfg.controlWordIndex = 1;
        config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);
        }

        /*  1.31.  */
        lineIndex  = 10;
        tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

        config.mplsCfg.tagEnable  = GT_TRUE;
        config.mplsCfg.vlanId     = 4095;
        config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.up         = 7;
        config.mplsCfg.cfi        = 0;

        config.mplsCfg.macDa.arEther[0] = 0x0;
        config.mplsCfg.macDa.arEther[1] = 0x1A;
        config.mplsCfg.macDa.arEther[2] = 0xFF;
        config.mplsCfg.macDa.arEther[3] = 0xFF;
        config.mplsCfg.macDa.arEther[4] = 0xFF;
        config.mplsCfg.macDa.arEther[5] = 0xFF;

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            config.mplsCfg.numLabels    = 3;
        }else
        {
            config.mplsCfg.numLabels    = 2;
        }

        config.mplsCfg.ttl          = 255;
        config.mplsCfg.label1       = 1048575;
        config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mplsCfg.exp1         = 2;
        config.mplsCfg.label2       = 1048575;
        config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mplsCfg.exp2         = 3;

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            config.mplsCfg.label3       = 1048575;
            config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mplsCfg.exp3         = 1;
            config.mplsCfg.setSBit      = GT_TRUE;
        }

        config.mplsCfg.retainCRC    = GT_FALSE;
        config.mplsCfg.controlWordEnable = GT_TRUE;

        config.mplsCfg.controlWordIndex = MAX_INDEX_CONTROL_WORD(dev);

        config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);
        }


        /*  1.32.  */
        lineIndex  = 10;
        tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

        config.mplsCfg.tagEnable  = GT_TRUE;
        config.mplsCfg.vlanId     = 4095;
        config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.up         = 7;
        config.mplsCfg.cfi        = 1;

        config.mplsCfg.macDa.arEther[0] = 0x0;
        config.mplsCfg.macDa.arEther[1] = 0x1A;
        config.mplsCfg.macDa.arEther[2] = 0xFF;
        config.mplsCfg.macDa.arEther[3] = 0x15;
        config.mplsCfg.macDa.arEther[4] = 0x3F;
        config.mplsCfg.macDa.arEther[5] = 0xF3;

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            config.mplsCfg.numLabels    = 3;
        }else
        {
            config.mplsCfg.numLabels    = 2;
        }

        config.mplsCfg.ttl          = 255;
        config.mplsCfg.label1       = 524287;
        config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mplsCfg.exp1         = 5;
        config.mplsCfg.label2       = 1048575;
        config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp2         = 1;

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            config.mplsCfg.label3       = 1048575;
            config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mplsCfg.exp3         = 1;
            config.mplsCfg.setSBit      = GT_TRUE;
        }

        if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
           PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
        {
            config.mplsCfg.retainCRC    = GT_TRUE;
        }else
        {
            config.mplsCfg.retainCRC    = GT_FALSE;
        }
        config.mplsCfg.controlWordEnable = GT_TRUE;
        config.mplsCfg.controlWordIndex = 1;
        config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);
        }

        /*  1.33.  */
        config.mplsCfg.tagEnable = GT_TRUE;
        config.mplsCfg.vlanId    = 4096;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->mplsCfg.tagEnable = %d, configPtr->mplsCfg.vlanId = %d",
                                     dev, config.mplsCfg.tagEnable, config.mplsCfg.vlanId);

        config.mplsCfg.vlanId = 100;

        /*  1.34.  */
        config.mplsCfg.tagEnable  = GT_TRUE;

        if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.mplsCfg.upMarkMode);
        }

        /*  1.35.  */
        config.mplsCfg.tagEnable  = GT_TRUE;
        config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.up         = 8;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->mplsCfg.tagEnable = %d, ->upMarkMode = %d, ->up = %d",
                dev, config.mplsCfg.tagEnable, config.mplsCfg.upMarkMode, config.mplsCfg.up);

        config.mplsCfg.up = 0;

        /*
            1.36. Call with out of range configPtr->mplsCfg {numLabels[0, 5]}
                            and other parameters from 1.19.
            Expected: BAD_PARAM.
        */
        /* Call with configPtr->mplsCfg.numLabels [0] */
        config.mplsCfg.numLabels = 0;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.numLabels = %d",
                                         dev, config.mplsCfg.numLabels);

        /* Call with configPtr->mplsCfg.numLabels [5] */
        config.mplsCfg.numLabels = 5;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.numLabels = %d",
                                         dev, config.mplsCfg.numLabels);

        /*  1.37.  */
        config.mplsCfg.numLabels = 2;
        config.mplsCfg.ttl       = 256;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.ttl = %d",
                                         dev, config.mplsCfg.ttl);

        config.mplsCfg.ttl = 10;

        /*  1.38.  */
        config.mplsCfg.label1 = 1048576;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->mplsCfg.numLabels = %d, configPtr->mplsCfg.label1 = %d",
                                     dev, config.mplsCfg.numLabels, config.mplsCfg.label1);

        /*  1.39.  */
        config.mplsCfg.numLabels = 1;
        config.mplsCfg.label1    = 1048576;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->mplsCfg.numLabels = %d, configPtr->mplsCfg.label1 = %d",
                                     dev, config.mplsCfg.numLabels, config.mplsCfg.label1);

        /*  1.40.  */
        config.mplsCfg.numLabels    = 2;
        config.mplsCfg.label1       = 100;

        if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.mplsCfg.exp1MarkMode);
        }

        /*  1.41.  */
        config.mplsCfg.numLabels    = 2;
        config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp1         = 8;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
        "%d, configPtr->mplsCfg.numLabels = %d, configPtr->mplsCfg.exp1MarkMode = %d, configPtr->mplsCfg.exp1 = %d",
             dev, config.mplsCfg.numLabels, config.mplsCfg.exp1MarkMode, config.mplsCfg.exp1);

        /*  1.42.  */
        config.mplsCfg.numLabels = 1;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->mplsCfg.numLabels = %d, configPtr->mplsCfg.exp1 = %d",
                          dev, config.mplsCfg.numLabels, config.mplsCfg.exp1);

        /*  1.43.  */
        config.mplsCfg.numLabels = 2;
        config.mplsCfg.label2    = 1048576;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.label2 = %d",
                                         dev, config.mplsCfg.label2);

        config.mplsCfg.label2 = 200;

        /*  1.44.  */
        if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.mplsCfg.exp2MarkMode);
        }

        /*  1.45.  */
        config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.exp2         = 8;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.exp2 = %d",
                                         dev, config.mplsCfg.exp2);

        if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            /*  1.46.  */
            config.mplsCfg.numLabels = 3;
            config.mplsCfg.label3    = 1048576;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.label2 = %d",
                                             dev, config.mplsCfg.label2);

            config.mplsCfg.label3 = 300;

            /*  1.47.  */
            if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                    (dev, lineIndex, tunnelType, &config),
                                    config.mplsCfg.exp3MarkMode);
            }

            /*  1.48.  */
            config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.exp3         = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.exp2 = %d",
                                             dev, config.mplsCfg.exp2);
        }

        if((PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE) || (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE))
        {
            config.mplsCfg.exp1         = 7;
            config.mplsCfg.exp2         = 7;
            config.mplsCfg.exp3         = 7;

            /*  1.49. */
            config.mplsCfg.controlWordEnable = GT_TRUE;
            config.mplsCfg.controlWordIndex = 0;
            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.controlWordIndex = %d",
                                             dev, config.mplsCfg.controlWordIndex);

            /*   1.50.  */

            config.mplsCfg.controlWordIndex = MAX_INDEX_CONTROL_WORD(dev);

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.controlWordIndex = %d",
                                             dev, config.mplsCfg.controlWordIndex);

            /*   1.51.  */
            config.mplsCfg.controlWordIndex = (((PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)) ? 7 : 15);

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.controlWordIndex = %d",
                                             dev, config.mplsCfg.controlWordIndex);

            /*   1.52.  */
            config.mplsCfg.controlWordEnable = GT_FALSE;
            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.controlWordEnable = %d",
                                             dev, config.mplsCfg.controlWordEnable);
        }

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) {

            /*   1.53.  */
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.mplsCfg.mplsEthertypeSelect);

            config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

            /*   1.54.  */
            /* Call with tunnelType [CPSS_TUNNEL_GENERIC_IPV6_E - CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E] */
            tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

            config.ipv6Cfg.tagEnable    = GT_TRUE;
            config.ipv6Cfg.vlanId       = 100;
            config.ipv6Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.up           = 0;
            config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.dscp         = 0;
            config.ipv6Cfg.retainCRC    = GT_TRUE;

            config.ipv6Cfg.macDa.arEther[0] = 0x0;
            config.ipv6Cfg.macDa.arEther[1] = 0x1A;
            config.ipv6Cfg.macDa.arEther[2] = 0xFF;
            config.ipv6Cfg.macDa.arEther[3] = 0xFF;
            config.ipv6Cfg.macDa.arEther[4] = 0xFF;
            config.ipv6Cfg.macDa.arEther[5] = 0xFF;

            config.ipv6Cfg.ttl              = 0;

            config.ipv6Cfg.destIp.arIP[0] = 10;
            config.ipv6Cfg.destIp.arIP[1] = 15;
            config.ipv6Cfg.destIp.arIP[2] = 1;
            config.ipv4Cfg.destIp.arIP[3] = 250;

            config.ipv6Cfg.srcIp.arIP[0] = 10;
            config.ipv6Cfg.srcIp.arIP[1] = 15;
            config.ipv6Cfg.srcIp.arIP[2] = 1;
            config.ipv6Cfg.srcIp.arIP[3] = 255;

            config.ipv6Cfg.ipHeaderProtocol     = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
            /* profile index 7 for legacy support (empty profile: no extension data is added in tunnel header)*/
            config.ipv6Cfg.profileIndex         = 1;
            /*If the passenger is IPv4, the GRE protocol is fixed: 0x0800*/
            config.ipv6Cfg.greProtocolForEthernet   = 0x0800;
            config.ipv6Cfg.greFlagsAndVersion       = 0;
            /*not relevant*/
            config.ipv6Cfg.udpDstPort               = 0;
            config.ipv6Cfg.udpSrcPort               = 0;
            config.ipv6Cfg.flowLabelMode = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            /*   1.55.  */
            tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

            config.ipv6Cfg.tagEnable    = GT_TRUE;
            config.ipv6Cfg.vlanId       = 4095;
            config.ipv6Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.up           = 7;
            config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.dscp         = 63;
            config.ipv6Cfg.retainCRC    = GT_FALSE;

            config.ipv6Cfg.macDa.arEther[0] = 0xFF;
            config.ipv6Cfg.macDa.arEther[1] = 0xFF;
            config.ipv6Cfg.macDa.arEther[2] = 0xFF;
            config.ipv6Cfg.macDa.arEther[3] = 0xFF;
            config.ipv6Cfg.macDa.arEther[4] = 0xFF;
            config.ipv6Cfg.macDa.arEther[5] = 0xFF;

            config.ipv6Cfg.ttl              = 255;

            config.ipv6Cfg.destIp.u32Ip[0] = 0xF3F1FF1F;
            config.ipv6Cfg.destIp.u32Ip[1] = 0xFAF1FFEF;
            config.ipv6Cfg.destIp.u32Ip[2] = 0xFFFFFFFF;
            config.ipv6Cfg.destIp.u32Ip[3] = 0xF3F5FF1F;

            config.ipv6Cfg.srcIp.u32Ip[0] = 0xF3FFFFFF;
            config.ipv6Cfg.srcIp.u32Ip[1] = 0xFFF6F4FF;
            config.ipv6Cfg.srcIp.u32Ip[2] = 0xF1FFF5FF;
            config.ipv6Cfg.srcIp.u32Ip[3] = 0xF1F6AFF7;

            config.ipv6Cfg.ipHeaderProtocol     = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
            /* profile index 7 for legacy support (empty profile: no extension data is added in tunnel header)*/
            config.ipv6Cfg.profileIndex         = 3;
            /*If the passenger is IPv4, the GRE protocol is fixed: 0x0800*/
            config.ipv6Cfg.greProtocolForEthernet   = 0x0800;
            config.ipv6Cfg.greFlagsAndVersion       = 0xFFFF;


            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            /*   1.56.  */
            tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

            config.ipv6Cfg.tagEnable    = GT_TRUE;
            config.ipv6Cfg.vlanId       = 2048;
            config.ipv6Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.up           = 3;
            config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.dscp         = 32;
            config.ipv6Cfg.retainCRC    = GT_FALSE;

            config.ipv6Cfg.macDa.arEther[0] = 0xFF;
            config.ipv6Cfg.macDa.arEther[1] = 0xFF;
            config.ipv6Cfg.macDa.arEther[2] = 0x3F;
            config.ipv6Cfg.macDa.arEther[3] = 0xAF;
            config.ipv6Cfg.macDa.arEther[4] = 0xF1;
            config.ipv6Cfg.macDa.arEther[5] = 0x1F;

            config.ipv6Cfg.ttl              = 128;

            config.ipv6Cfg.destIp.u32Ip[0] = 0xFFFFFFFF;
            config.ipv6Cfg.destIp.u32Ip[1] = 0xFFFFFFFF;
            config.ipv6Cfg.destIp.u32Ip[2] = 0xFFFFFFFF;
            config.ipv6Cfg.destIp.u32Ip[3] = 0xFFFFFFFF;

            config.ipv6Cfg.srcIp.u32Ip[0] = 0xFFFFFFFF;
            config.ipv6Cfg.srcIp.u32Ip[1] = 0xFFFFFFFF;
            config.ipv6Cfg.srcIp.u32Ip[2] = 0xFFFFFFFF;
            config.ipv6Cfg.srcIp.u32Ip[3] = 0xFFFFFFFF;

            config.ipv6Cfg.ipHeaderProtocol     = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
            /* profile index 7 for legacy support (empty profile: no extension data is added in tunnel header)*/
            config.ipv6Cfg.profileIndex         = 7;

            config.ipv6Cfg.greProtocolForEthernet   = 0x0800;
            config.ipv6Cfg.greFlagsAndVersion       = 0xFFFF;

            config.ipv6Cfg.udpDstPort               = 0xFFFF;
            config.ipv6Cfg.udpSrcPort               = 0xFFFF;
            config.ipv6Cfg.flowLabelMode = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            if((PRV_CPSS_SIP_5_15_CHECK_MAC(dev)== GT_TRUE)||
                (PRV_CPSS_SIP_5_16_CHECK_MAC(dev)== GT_TRUE)||
                    PRV_CPSS_SIP_5_20_CHECK_MAC(dev)== GT_TRUE)
            {
                 /*   1.57.  */
                lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 8;

                tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

                config.ipv6Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
                config.ipv6Cfg.ipProtocol = 0x48;

                st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
                if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                    cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                    st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                    /* compare values */
                    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV6_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                                     dev, lineIndex, tunnelType);
                    }else
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                                     dev, lineIndex, tunnelType);
                    }

                    st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                                 dev, lineIndex, tunnelType);
                }
             }

            /*   1.57.  */
            tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

            config.ipv6Cfg.tagEnable = GT_TRUE;
            config.ipv6Cfg.vlanId    = 4096;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->ipv4Cfg.tagEnable = %d, configPtr->ipv4Cfg.vlanId = %d",
                            dev, config.ipv6Cfg.tagEnable, config.ipv6Cfg.vlanId);

            /*   1.58.  */
            config.ipv6Cfg.vlanId     = 100;
            config.ipv6Cfg.tagEnable  = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv6Cfg.upMarkMode);

            /*   1.59.  */
            config.ipv6Cfg.tagEnable  = GT_TRUE;
            config.ipv6Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.up         = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->ipv4Cfg.tagEnable = %d, ->upMarkMode = %d, ->up = %d",
                dev, config.ipv6Cfg.tagEnable, config.ipv6Cfg.upMarkMode, config.ipv6Cfg.up);

            config.ipv6Cfg.up = 0;

            /*   1.60.  */
            config.ipv6Cfg.tagEnable    = GT_TRUE;
            config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv6Cfg.dscp         = 64;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
                "%d, configPtr->ipv6Cfg.tagEnable = %d, ->dscpMarkMode = %d, ->dscp = %d",
                dev, config.ipv6Cfg.tagEnable, config.ipv6Cfg.dscpMarkMode, config.ipv6Cfg.dscp);

            config.ipv6Cfg.dscp = 0;

            /*   1.61.  */
            config.ipv6Cfg.ttl = 256;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv6Cfg.ttl = %d",
                                             dev, config.ipv6Cfg.ttl);

            config.ipv6Cfg.ttl = 0;

            /*   1.62.  */
            config.ipv6Cfg.profileIndex = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv6Cfg.profileIndex = %d",
                                             dev, config.ipv6Cfg.profileIndex);

            config.ipv6Cfg.profileIndex = 7;

            /*   1.63.  */
            config.ipv6Cfg.tagEnable  = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv6Cfg.ipHeaderProtocol);

            /*   1.64.  */
            config.ipv6Cfg.tagEnable  = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.ipv6Cfg.flowLabelMode);
        }

        /*   1.65.  */
        tunnelType = CPSS_TUNNEL_MAC_IN_MAC_E;
        lineIndex = 20;

        config.mimCfg.tagEnable = GT_TRUE;
        config.mimCfg.vlanId = 4095;
        config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mimCfg.up = 7;

        config.mimCfg.macDa.arEther[0] = 0xAF;
        config.mimCfg.macDa.arEther[1] = 0xBF;
        config.mimCfg.macDa.arEther[2] = 0xCF;
        config.mimCfg.macDa.arEther[3] = 0xDF;
        config.mimCfg.macDa.arEther[4] = 0xEF;
        config.mimCfg.macDa.arEther[5] = 0xFF;

        config.mimCfg.retainCrc = GT_FALSE;
        config.mimCfg.iSid = 0xFFFFFF;
        config.mimCfg.iUp = 7;
        config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mimCfg.iDp = 1;
        config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mimCfg.iTagReserved = 15;
        config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E;
        config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;


         if((PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum) == GT_TRUE)||
            (PRV_CPSS_SIP_5_16_CHECK_MAC(prvTgfDevNum) == GT_TRUE)||
            (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) == GT_TRUE))
        {
            config.mimCfg.bSaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E;
        }
        else
        {
            config.mimCfg.bSaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E;
        }

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

        }else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }

        /* 1.66.   */
        config.mimCfg.tagEnable = GT_FALSE;
        config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mimCfg.up = 3;

        config.mimCfg.macDa.arEther[0] = 0;
        config.mimCfg.macDa.arEther[1] = 0;
        config.mimCfg.macDa.arEther[2] = 0;
        config.mimCfg.macDa.arEther[3] = 0;
        config.mimCfg.macDa.arEther[4] = 0;
        config.mimCfg.macDa.arEther[5] = 0;

        config.mimCfg.retainCrc = GT_TRUE;
        config.mimCfg.iSid = 0x1FFFFF;
        config.mimCfg.iUp = 3;
        config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mimCfg.iDp = 1;
        config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mimCfg.iTagReserved = 7;
        config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E;
        config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E &&
           PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) == GT_FALSE)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

        }else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }

        /* 1.67.   */
        config.mimCfg.tagEnable = GT_TRUE;
        config.mimCfg.vlanId = 2048;
        config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mimCfg.up = 1;

        config.mimCfg.macDa.arEther[0] = 0xFF;
        config.mimCfg.macDa.arEther[1] = 0xFF;
        config.mimCfg.macDa.arEther[2] = 0xFF;
        config.mimCfg.macDa.arEther[3] = 0xFF;
        config.mimCfg.macDa.arEther[4] = 0xFF;
        config.mimCfg.macDa.arEther[5] = 0xFF;

        config.mimCfg.retainCrc = GT_TRUE;
        config.mimCfg.iSid = 0x1F1FF1;
        config.mimCfg.iUp = 0;
        config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mimCfg.iDp = 1;
        config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mimCfg.iTagReserved = 7;
        config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E;
        config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E &&
           PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) == GT_FALSE)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

        }else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }

        /* 1.68.   */
        config.mimCfg.tagEnable = GT_TRUE;
        config.mimCfg.vlanId = 1;
        config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mimCfg.up = 0;

        config.mimCfg.macDa.arEther[0] = 0;
        config.mimCfg.macDa.arEther[1] = 0;
        config.mimCfg.macDa.arEther[2] = 0xAD;
        config.mimCfg.macDa.arEther[3] = 0xFF;
        config.mimCfg.macDa.arEther[4] = 0xEE;
        config.mimCfg.macDa.arEther[5] = 0xFF;

        config.mimCfg.retainCrc = GT_TRUE;
        config.mimCfg.iSid = 0;
        config.mimCfg.iUp = 0;
        config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mimCfg.iDp = 0;
        config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
        config.mimCfg.iTagReserved = 0;
        config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E;
        config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E &&
           PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) == GT_FALSE)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

            cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

            st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelTypeGet, &configGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

            /* compare values */
            UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

            st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                         dev, lineIndex, tunnelType);

        }else
        {
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
        }


        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            /* 1.69.   */
            config.mimCfg.tagEnable = GT_TRUE;
            config.mimCfg.vlanId = 4096;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            config.mimCfg.vlanId = 0;

            /* 1.70.   */
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.mimCfg.upMarkMode);

            /* 1.71.   */
            config.mimCfg.up = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            config.mimCfg.up = 0;

            /* 1.72.   */
            config.mimCfg.iSid = 0x1000000;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            config.mimCfg.iSid = 0;

            /* 1.73.   */
            config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.iUp = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            config.mimCfg.iUp = 0;

            /* 1.74.   */
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.mimCfg.iUpMarkMode);

            /* 1.75.   */
            config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.iDp = 8;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            config.mimCfg.iDp = 0;

            /* 1.76.   */
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                config.mimCfg.iDpMarkMode);

            /* 1.77.   */
            config.mimCfg.iTagReserved = 16;

            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            config.mimCfg.iTagReserved = 0;

            if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* 1.78.   */
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                    (dev, lineIndex, tunnelType, &config),
                                    config.mimCfg.iSidAssignMode);

                /* 1.79.   */
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                    (dev, lineIndex, tunnelType, &config),
                                    config.mimCfg.bDaAssignMode);
            }

        }

        /*  restore valid parameters  */
        lineIndex  = 10;
        tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

        config.ipv4Cfg.tagEnable    = GT_TRUE;
        config.ipv4Cfg.vlanId       = 100;
        config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up           = 0;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp         = 0;
        config.ipv4Cfg.cfi          = 0;

        config.ipv4Cfg.macDa.arEther[0] = 0x0;
        config.ipv4Cfg.macDa.arEther[1] = 0x1A;
        config.ipv4Cfg.macDa.arEther[2] = 0xFF;
        config.ipv4Cfg.macDa.arEther[3] = 0xFF;
        config.ipv4Cfg.macDa.arEther[4] = 0xFF;
        config.ipv4Cfg.macDa.arEther[5] = 0xFF;

        config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
        config.ipv4Cfg.ttl              = 0;
        config.ipv4Cfg.autoTunnel       = GT_TRUE;
        config.ipv4Cfg.autoTunnelOffset = 1;

        config.ipv4Cfg.destIp.arIP[0] = 10;
        config.ipv4Cfg.destIp.arIP[1] = 15;
        config.ipv4Cfg.destIp.arIP[2] = 1;
        config.ipv4Cfg.destIp.arIP[3] = 250;

        config.ipv4Cfg.srcIp.arIP[0] = 10;
        config.ipv4Cfg.srcIp.arIP[1] = 15;
        config.ipv4Cfg.srcIp.arIP[2] = 1;
        config.ipv4Cfg.srcIp.arIP[3] = 255;

        /*  1.80.  */
        if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                (dev, lineIndex, tunnelType, &config),
                                tunnelType);
        }

        /*  1.81.  */
        /* Call with tunnelType [CPSS_TUNNEL_X_OVER_MPLS_E] */
        tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tunnelType = %d",
                                         dev, tunnelType);

        /*  1.82.  */
        tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, NULL);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, configPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configPtr = NULL", dev);
        }

        /*  1.83.  */
        /* Call with TTL mode 3 */
        tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;
        config.mplsCfg.tagEnable = GT_FALSE;
        config.mplsCfg.vlanId = 0;
        config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E;
        config.mplsCfg.ttl = 0;
        config.mplsCfg.numLabels = 1;
        config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.mplsCfg.cfi = 0;
        config.mplsCfg.controlWordEnable = GT_FALSE;
        config.mplsCfg.pushEliAndElAfterLabel1 = GT_FALSE;
        config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;
        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E", dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E", dev);
            config.mplsCfg.ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
            st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E", dev);
        }
    }

    /* restore valid parameters */
    lineIndex  = 10;
    tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

    config.ipv4Cfg.tagEnable    = GT_TRUE;
    config.ipv4Cfg.vlanId       = 100;
    config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.up           = 0;
    config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.dscp         = 0;
    config.ipv4Cfg.cfi          = 0;

    config.ipv4Cfg.macDa.arEther[0] = 0x0;
    config.ipv4Cfg.macDa.arEther[1] = 0x1A;
    config.ipv4Cfg.macDa.arEther[2] = 0xFF;
    config.ipv4Cfg.macDa.arEther[3] = 0xFF;
    config.ipv4Cfg.macDa.arEther[4] = 0xFF;
    config.ipv4Cfg.macDa.arEther[5] = 0xFF;

    config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
    config.ipv4Cfg.ttl              = 0;
    config.ipv4Cfg.autoTunnel       = GT_TRUE;
    config.ipv4Cfg.autoTunnelOffset = 1;

    config.ipv4Cfg.destIp.arIP[0] = 10;
    config.ipv4Cfg.destIp.arIP[1] = 15;
    config.ipv4Cfg.destIp.arIP[2] = 1;
    config.ipv4Cfg.destIp.arIP[3] = 250;

    config.ipv4Cfg.srcIp.arIP[0] = 10;
    config.ipv4Cfg.srcIp.arIP[1] = 15;
    config.ipv4Cfg.srcIp.arIP[2] = 1;
    config.ipv4Cfg.srcIp.arIP[3] = 255;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartEntrySet(dev, lineIndex, tunnelType, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEntryGet)
{
/*
    ITERATE_DEVICES (APPLICABLE DEVICES: DxCh2; DxCh3; xCat; xCat3; Lion)
    1.1. Call with routerArpTunnelStartLineIndex[max - 1]
                   non-null tunnelTypePtr
                   and non-null configPtr.
    Expected: GT_OK.
    1.2. Call with out of range routerArpTunnelStartLineIndex [max],
                   non-null tunnelTypePtr
                   and non-null configPtr.
    Expected: GT_BAD_PARAM.
    1.3. Call with routerArpTunnelStartLineIndex[max - 1],
                   tunnelTypePtr[NULL]
                   and non-null configPtr.
    Expected: GT_BAD_PTR.
    1.4. Call with routerArpTunnelStartLineIndex[max - 1],
                   non-null tunnelTypePtr and configPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                         st        = GT_OK;

    GT_U8                             dev;
    GT_U32                            lineIndex = 0;
    CPSS_TUNNEL_TYPE_ENT              tunnelType;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT config;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with routerArpTunnelStartLineIndex[max - 1]
                           non-null tunnelTypePtr
                           and non-null configPtr.
            Expected: GT_OK.
        */
        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

        st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);
        }

        /*
            1.2. Call with out of range routerArpTunnelStartLineIndex [max],
                           non-null tunnelTypePtr
                           and non-null configPtr.
            Expected: GT_BAD_PARAM.
        */
        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart;

        st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelType, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, lineIndex);

        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

        /*
            1.3. Call with routerArpTunnelStartLineIndex[max - 1],
                           tunnelTypePtr[NULL]
                           and non-null configPtr.
            Expected: GT_BAD_PTR.
        */
        lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

        st = cpssDxChTunnelStartEntryGet(dev, lineIndex, NULL, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tunnelTypePtr = NULL", dev);
        }

        /*
            1.4. Call with routerArpTunnelStartLineIndex[max - 1],
                           non-null tunnelTypePtr and configPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelType, NULL);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configPtr = NULL", dev);
        }
    }

    lineIndex  = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelType, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartEntryGet(dev, lineIndex, &tunnelType, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTaggingSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChEthernetOverMplsTunnelStartTaggingSet)
{
/*
    ITERATE_DEVICES (APPLICABLE DEVICES: DxCh2; DxCh3; xCat; xCat3; Lion)
    1.1. Call with enable [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChEthernetOverMplsTunnelStartTaggingGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_BOOL   state    = GT_TRUE;
    GT_BOOL   stateGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE and GT_FALSE].
            Expected: GT_OK.
        */

        /* Call function with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChEthernetOverMplsTunnelStartTaggingSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /* Call function with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChEthernetOverMplsTunnelStartTaggingSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChEthernetOverMplsTunnelStartTaggingGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChEthernetOverMplsTunnelStartTaggingGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChEthernetOverMplsTunnelStartTaggingGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChEthernetOverMplsTunnelStartTaggingSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChEthernetOverMplsTunnelStartTaggingSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTaggingGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChEthernetOverMplsTunnelStartTaggingGet)
{
/*
    ITERATE_DEVICES (APPLICABLE DEVICES: DxCh2; DxCh3; xCat; xCat3; Lion)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_BOOL   state    = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChEthernetOverMplsTunnelStartTaggingGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChEthernetOverMplsTunnelStartTaggingGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChEthernetOverMplsTunnelStartTaggingGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChEthernetOverMplsTunnelStartTaggingGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTagModeSet

(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT tagMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChEthernetOverMplsTunnelStartTagModeSet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with tagMode [CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E /
                            CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E]
    Expected: GT_OK.
    1.2. Call cpssDxChEthernetOverMplsTunnelStartTagModeGet with not NULL tagModePtr.
    Expected: GT_OK and the same tagModePtr as was set.
    1.3. Call with tagMode [wrong enum values]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT tagMode    =
                            CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E;
    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT tagModeGet =
                            CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tagMode [CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E /
                                    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E]
            Expected: GT_OK.
        */
        /* iterate with CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E */
        tagMode = CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E;

        st = cpssDxChEthernetOverMplsTunnelStartTagModeSet(dev, tagMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tagMode);

        /*
            1.2. Call cpssDxChEthernetOverMplsTunnelStartTagModeGet with not NULL tagModePtr.
            Expected: GT_OK and the same tagModePtr as was set.
        */
        st = cpssDxChEthernetOverMplsTunnelStartTagModeGet(dev, &tagModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChEthernetOverMplsTunnelStartTagModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(tagMode, tagModeGet,
                    "get another tagMode than was set: %d", dev);

        /* iterate with CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E */
        tagMode = CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E;

        st = cpssDxChEthernetOverMplsTunnelStartTagModeSet(dev, tagMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, tagMode);

        /*
            1.2. Call cpssDxChEthernetOverMplsTunnelStartTagModeGet with not NULL tagModePtr.
            Expected: GT_OK and the same tagModePtr as was set.
        */
        st = cpssDxChEthernetOverMplsTunnelStartTagModeGet(dev, &tagModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChEthernetOverMplsTunnelStartTagModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(tagMode, tagModeGet,
                    "get another tagMode than was set: %d", dev);

        /*
            1.3. Call with tagMode [wrong enum values]
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChEthernetOverMplsTunnelStartTagModeSet
                            (dev, tagMode),
                            tagMode);
    }

    tagMode = CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChEthernetOverMplsTunnelStartTagModeSet(dev, tagMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChEthernetOverMplsTunnelStartTagModeSet(dev, tagMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChEthernetOverMplsTunnelStartTagModeGet

(
    IN  GT_U8                                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT *tagModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChEthernetOverMplsTunnelStartTagModeGet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with not NULL tagModePtr.
    Expected: GT_OK.
    1.2. Call tagModePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT tagMode =
                    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL tagModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChEthernetOverMplsTunnelStartTagModeGet(dev, &tagMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call tagModePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChEthernetOverMplsTunnelStartTagModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tagModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChEthernetOverMplsTunnelStartTagModeGet(dev, &tagMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChEthernetOverMplsTunnelStartTagModeGet(dev, &tagMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartPassengerVlanTranslationEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPassengerVlanTranslationEnableSet)
{
/*
    ITERATE_DEVICES (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with enable [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChTunnelStartPassengerVlanTranslationEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL   enable    = GT_TRUE;
    GT_BOOL   enableGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE and GT_FALSE].
            Expected: GT_OK.
        */

        /* Call function with enable [GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChTunnelStartPassengerVlanTranslationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChTunnelStartPassengerVlanTranslationEnableGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChTunnelStartPassengerVlanTranslationEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChTunnelStartPassengerVlanTranslationEnableGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /* Call function with enable [GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChTunnelStartPassengerVlanTranslationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChTunnelStartPassengerVlanTranslationEnableGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChTunnelStartPassengerVlanTranslationEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChTunnelStartPassengerVlanTranslationEnableGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPassengerVlanTranslationEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartPassengerVlanTranslationEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartPassengerVlanTranslationEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPassengerVlanTranslationEnableGet)
{
/*
    ITERATE_DEVICES (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U8     dev;

    GT_BOOL   enable = GT_TRUE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTunnelStartPassengerVlanTranslationEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTunnelStartPassengerVlanTranslationEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPassengerVlanTranslationEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartPassengerVlanTranslationEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               ttl
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelTtlSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with ttl [0 / 100/ 255].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartMplsPwLabelTtlGet with non NULL ttlPtr.
    Expected: GT_OK and the same ttl.
    1.1.3. Call with out of range ttl [256].
    Expected: BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_U32      ttl    = 0;
    GT_U32      ttlGet = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with ttl [0 / 100/ 255].
                Expected: GT_OK.
            */
            /* call with ttl = 0 */
            ttl = 0;

            st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ttl);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, ttl);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelTtlGet with non NULL ttlPtr.
                Expected: GT_OK and the same ttl.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttlGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelTtlGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(ttl, ttlGet,
                           "get another ttl than was set: %d, %d", dev, port);
            }

            /* call with ttl = 100 */
            ttl = 100;

            st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ttl);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, ttl);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelTtlGet with non NULL ttlPtr.
                Expected: GT_OK and the same ttl.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttlGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelTtlGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(ttl, ttlGet,
                           "get another ttl than was set: %d, %d", dev, port);
            }

            /* call with ttl = 255 */
            ttl = 255;

            st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ttl);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, ttl);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelTtlGet with non NULL ttlPtr.
                Expected: GT_OK and the same ttl.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttlGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelTtlGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(ttl, ttlGet,
                           "get another ttl than was set: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with out of range ttl [256].
                Expected: BAD_PARAM.
            */
            ttl = 256;

            st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, ttl);
        }

        ttl = 0;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ttl);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    ttl  = 0;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelTtlSet(dev, port, ttl);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelTtlGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *ttlPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelTtlGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL ttlPtr.
    Expected: GT_OK.
    1.1.2. Call with ttlPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_U32      ttl = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL ttlPtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttl);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, ttl);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, ttl);
            }

            /*
                1.1.2. Call with ttlPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ttlPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttl);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttl);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelTtlGet(dev, port, &ttl);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelPushEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelPushEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartMplsPwLabelPushEnableGet with non NULL enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChTunnelStartMplsPwLabelPushEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelPushEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelPushEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChTunnelStartMplsPwLabelPushEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelPushEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelPushEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelPushEnableSet(dev, port, enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelPushEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelPushEnableSet(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    enable = GT_FALSE;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelPushEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelPushEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelPushEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelPushEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st                           = GT_OK;
    GT_U8       dev                          = 0;
    GT_PORT_NUM port                         = 0;
    GT_BOOL     enable                         = 0;
    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelPushEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsFlowLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsFlowLabelEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartMplsFlowLabelEnableGet with non NULL enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChTunnelStartMplsFlowLabelEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsFlowLabelEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsFlowLabelEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChTunnelStartMplsFlowLabelEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsFlowLabelEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsFlowLabelEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsFlowLabelEnableSet(dev, port, enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsFlowLabelEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsFlowLabelEnableSet(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    enable = GT_FALSE;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsFlowLabelEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsFlowLabelEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsFlowLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsFlowLabelEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st                           = GT_OK;
    GT_U8       dev                          = 0;
    GT_PORT_NUM port                         = 0;
    GT_BOOL     enable                         = 0;
    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsFlowLabelEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsFlowLabelTtlSet
(
    IN GT_U8                devNum,
    IN GT_U8                ttl
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsFlowLabelTtlSet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1. Call with ttl = 0,
    Expected: GT_OK
    1.2. Call with ttl = 33,
    Expected: GT_OK
    1.2. Call with ttl = 63,
    Expected: GT_OK
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_U32      notAppFamilyBmp;
    GT_U8       ttl = 0;
    GT_U8       getTtl = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with ttl = 0 */
        ttl = 0;
        /* 1.1.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsFlowLabelTtlSet(dev, ttl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ttl);

        /* 1.1.2. Check the configured value */
        st = cpssDxChTunnelStartMplsFlowLabelTtlGet(dev, &getTtl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ttl, getTtl,
                                     "get another value than was set: device %d",
                                     dev);

        /* 1.2. Call with ttl = 33 */
        ttl = 0;
        /* 1.2.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsFlowLabelTtlSet(dev, ttl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ttl);

        /* 1.2.2. Check the configured value */
        st = cpssDxChTunnelStartMplsFlowLabelTtlGet(dev, &getTtl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ttl, getTtl,
                                     "get another value than was set: device %d",
                                     dev);

        /* 1.3. Call with ttl = 33 */
        ttl = 63;
        /* 1.3.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsFlowLabelTtlSet(dev, ttl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ttl);

        /* 1.3.2. Check the configured value */
        st = cpssDxChTunnelStartMplsFlowLabelTtlGet(dev, &getTtl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ttl, getTtl,
                                     "get another value than was set: device %d",
                                     dev);
    }

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsFlowLabelTtlSet(dev, ttl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, ttl);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsFlowLabelTtlGet
(
    IN  GT_U8           devNum,
    OUT GT_U8           *ttlPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsFlowLabelTtlGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1. Call cpssDxChTunnelStartMplsFlowLabelTtlGet,
    Expected: GT_OK
    1.2. Call cpssDxChTunnelStartMplsFlowLabelTtlGet with valuePtr = NULL,
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_U32      notAppFamilyBmp;
    GT_U8       ttl;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsFlowLabelTtlGet(dev, &ttl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. check that the function returns GT_BAD_PTR for value = NULL */
        st = cpssDxChTunnelStartMplsFlowLabelTtlGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsFlowLabelTtlGet(dev, &ttl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               label
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with label [0 / 100/ 0xFFFFF].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartMplsPwLabelGet with non NULL labelPtr.
    Expected: GT_OK and the same label.
    1.1.3. Call with out of range label [0x100000].
    Expected: BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;
    GT_U32      label    = 0;
    GT_U32      labelGet = 0;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with label [0 / 100/ 0xFFFFF].
                Expected: GT_OK.
            */
            /* call with label = 0 */
            label = 0;

            st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, label);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, label);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelGet with non NULL labelPtr.
                Expected: GT_OK and the same label.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &labelGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(label, labelGet,
                           "get another label than was set: %d, %d", dev, port);
            }

            /* call with label = 100 */
            label = 100;

            st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, label);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, label);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelGet with non NULL labelPtr.
                Expected: GT_OK and the same label.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &labelGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(label, labelGet,
                           "get another label than was set: %d, %d", dev, port);
            }

            /* call with label = 0xFFFFF */
            label = 0xFFFFF;

            st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, label);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, label);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelGet with non NULL labelPtr.
                Expected: GT_OK and the same label.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &labelGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(label, labelGet,
                           "get another label than was set: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with out of range label [0x100000].
                Expected: BAD_PARAM.
            */
            label = 0x100000;

            st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, label);
        }

        label = 0;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    label = 0;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelSet(dev, port, label);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *labelPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL labelPtr.
    Expected: GT_OK.
    1.1.2. Call with labelPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;
    GT_U32      label = 0;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL labelPtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &label);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call with labelPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, labelPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &label);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &label);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &label);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &label);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelGet(dev, port, &label);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelExpSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               exp
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelExpSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with exp [0 / 5 / 7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartMplsPwLabelExpGet with non NULL expPtr.
    Expected: GT_OK and the same exp.
    1.1.3. Call with out of range exp [8].
    Expected: BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_U32      exp    = 0;
    GT_U32      expGet = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with exp [0 / 5 / 7].
                Expected: GT_OK.
            */
            /* call with exp = 0 */
            exp = 0;

            st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, exp);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, exp);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelExpGet with non NULL expPtr.
                Expected: GT_OK and the same exp.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &expGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelExpGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(exp, expGet,
                           "get another exp than was set: %d, %d", dev, port);
            }

            /* call with exp = 5 */
            exp = 5;

            st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, exp);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, exp);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelExpGet with non NULL expPtr.
                Expected: GT_OK and the same exp.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &expGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelExpGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(exp, expGet,
                           "get another exp than was set: %d, %d", dev, port);
            }

            /* call with exp = 7 */
            exp = 7;

            st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, exp);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, exp);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPwLabelExpGet with non NULL expPtr.
                Expected: GT_OK and the same exp.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &expGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPwLabelExpGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(exp, expGet,
                           "get another exp than was set: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with out of range exp [8].
                Expected: GT_BAD_PARAM.
            */
            exp = 8;

            st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, exp);
        }

        exp = 0;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    exp = 0;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelExpSet(dev, port, exp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwLabelExpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *expPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwLabelExpGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL expPtr.
    Expected: GT_OK.
    1.1.2. Call with expPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_U32      exp = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL expPtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &exp);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call with expPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, expPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &exp);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &exp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &exp);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &exp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPwLabelExpGet(dev, port, &exp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Caelum, Bobcat3)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet with non NULL enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(dev, port, enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    enable = GT_FALSE;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st                           = GT_OK;
    GT_U8       dev                          = 0;
    GT_PORT_NUM port                         = 0;
    GT_BOOL     enable                         = 0;
    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E);;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Caelum, Bobcat3)
    1.1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet with non NULL enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */
            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(dev, port, enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet with non NULL enablePtr.
                Expected: GT_OK and the same enable.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(dev, port, enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(dev, port, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    enable = GT_FALSE;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st                           = GT_OK;
    GT_U8       dev                          = 0;
    GT_PORT_NUM port                         = 0;
    GT_BOOL     enable                         = 0;
    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enable);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartHeaderTpidSelectSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  GT_U32               tpidEntryIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartHeaderTpidSelectSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with tpidEntryIndex [0 / 5 / 7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartHeaderTpidSelectGet with non NULL tpidEntryIndexPtr.
    Expected: GT_OK and the same tpidEntryIndex.
    1.1.3. Call with out of range tpidEntryIndex [8].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    GT_U32      tpidEntryIndex    = 0;
    GT_U32      tpidEntryIndexGet = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with tpidEntryIndex [0 / 5 / 7].
                Expected: GT_OK.
            */
            /* call with tpidEntryIndex = 0 */
            tpidEntryIndex = 0;

            st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tpidEntryIndex);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, tpidEntryIndex);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartHeaderTpidSelectGet with non NULL tpidEntryIndexPtr.
                Expected: GT_OK and the same tpidEntryIndex.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndexGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartHeaderTpidSelectGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
                           "get another tpidEntryIndex than was set: %d, %d", dev, port);
            }

            /* call with tpidEntryIndex = 5 */
            tpidEntryIndex = 5;

            st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tpidEntryIndex);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, tpidEntryIndex);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartHeaderTpidSelectGet with non NULL tpidEntryIndexPtr.
                Expected: GT_OK and the same tpidEntryIndex.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndexGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartHeaderTpidSelectGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
                           "get another tpidEntryIndex than was set: %d, %d", dev, port);
            }

            /* call with tpidEntryIndex = 7 */
            tpidEntryIndex = 7;

            st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, tpidEntryIndex);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port, tpidEntryIndex);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartHeaderTpidSelectGet with non NULL tpidEntryIndexPtr.
                Expected: GT_OK and the same tpidEntryIndex.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndexGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartHeaderTpidSelectGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
                           "get another tpidEntryIndex than was set: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with out of range tpidEntryIndex [8].
                Expected: GT_OUT_OF_RANGE.
            */
            tpidEntryIndex = 8;

            st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, tpidEntryIndex);
        }

        tpidEntryIndex = 0;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    tpidEntryIndex = 0;
    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartHeaderTpidSelectSet(dev, port, tpidEntryIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartHeaderTpidSelectGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    OUT GT_U32               *tpidEntryIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartHeaderTpidSelectGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL tpidEntryIndexPtr.
    Expected: GT_OK.
    1.1.2. Call with tpidEntryIndexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st                           = GT_OK;
    GT_U8       dev                          = 0;
    GT_PORT_NUM port                         = 0;
    GT_U32      tpidEntryIndex         = 0;
    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL tpidEntryIndexPtr.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndex);
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }

            /*
                1.1.2. Call with tpidEntryIndexPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tpidEntryIndexPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndex);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndex);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not active devices check that function returns GT_NOT_APPLICABLE_DEVICE.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartHeaderTpidSelectGet(dev, port, &tpidEntryIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPortGroupEntrySet)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId   = 0;

    GT_BOOL                             isEqual       = GT_FALSE;
    GT_U32                              lineIndex     = 0;
    GT_U32                              configSize    = 0;
    CPSS_TUNNEL_TYPE_ENT                tunnelType    = CPSS_TUNNEL_X_OVER_IPV4_E;
    CPSS_TUNNEL_TYPE_ENT                tunnelTypeGet = CPSS_TUNNEL_X_OVER_IPV4_E;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   config, configTmp;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   configGet;


    cpssOsBzero((GT_VOID*) &config, sizeof(config));
    cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*  1.1.1. */
            cpssOsBzero((GT_VOID*) &config, sizeof(config));
            cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

            /* Call with tunnelType [CPSS_TUNNEL_X_OVER_IPV4_E] */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

            if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
            }
            else
            {
                tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
            }

            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.vlanId       = 100;
            config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.up           = 0;
            config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.dscp         = 0;
            config.ipv4Cfg.cfi          = 0;

            config.ipv4Cfg.macDa.arEther[0] = 0x0;
            config.ipv4Cfg.macDa.arEther[1] = 0x1A;
            config.ipv4Cfg.macDa.arEther[2] = 0xFF;
            config.ipv4Cfg.macDa.arEther[3] = 0xFF;
            config.ipv4Cfg.macDa.arEther[4] = 0xFF;
            config.ipv4Cfg.macDa.arEther[5] = 0xFF;

            config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
            config.ipv4Cfg.ttl              = 0;
            config.ipv4Cfg.autoTunnel       = GT_TRUE;
            config.ipv4Cfg.autoTunnelOffset = 1;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
            {
                config.ipv4Cfg.profileIndex = 7;
            }
            else
            {
                config.ipv4Cfg.profileIndex = 0;
            }
            config.ipv4Cfg.destIp.arIP[0] = 10;
            config.ipv4Cfg.destIp.arIP[1] = 15;
            config.ipv4Cfg.destIp.arIP[2] = 1;
            config.ipv4Cfg.destIp.arIP[3] = 250;

            config.ipv4Cfg.srcIp.arIP[0] = 10;
            config.ipv4Cfg.srcIp.arIP[1] = 15;
            config.ipv4Cfg.srcIp.arIP[2] = 1;
            config.ipv4Cfg.srcIp.arIP[3] = 255;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }

            /*  1.1.2. */
            st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                      &tunnelTypeGet, &configGet);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChTunnelStartPortGroupEntryGet: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChTunnelStartPortGroupEntryGet: %d", dev);

                /* validation values */
                UTF_VERIFY_EQUAL1_STRING_MAC(tunnelType, tunnelTypeGet,
                           "got another tunnelType then was set: %d", dev);

                /* calc size of IpV4 config */
                /* CFI bit relevant for DxCh3; xCat; xCat3; Lion */
                configSize = sizeof(config.ipv4Cfg) - sizeof(config.ipv4Cfg.cfi) *
                     (CPSS_PP_FAMILY_CHEETAH2_E == PRV_CPSS_PP_MAC(dev)->devFamily);

                /* check if received config is equal to original */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipv4Cfg,
                                             (GT_VOID*) &configGet.ipv4Cfg,
                                             configSize)) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                           "got another config then was set: %d", dev);
            }

            /* Call with tunnelType [CPSS_TUNNEL_X_OVER_GRE_IPV4_E] */
            if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
            }
            else
            {
                tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
            }

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }

            /*  1.1.3. */
            st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                      &tunnelTypeGet, &configGet);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
            {
                if (configGet.ipv4Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E)
                {
                    configGet.ipv4Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
                }
                if (configGet.ipv4Cfg.greProtocolForEthernet == 0x6558)
                {
                    configGet.ipv4Cfg.greProtocolForEthernet = 0;
                }
            }

            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTunnelStartPortGroupEntryGet: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTunnelStartPortGroupEntryGet: %d", dev);

                /* validation values */
                UTF_VERIFY_EQUAL1_STRING_MAC(tunnelType, tunnelTypeGet,
                           "got another tunnelType then was set: %d", dev);

                /* calc size of IpV4 config */
                /* CFI bit relevant for DxCh3; xCat; xCat3; Lion */
                configSize = sizeof(config.ipv4Cfg) - sizeof(config.ipv4Cfg.cfi) *
                     (CPSS_PP_FAMILY_CHEETAH2_E == PRV_CPSS_PP_MAC(dev)->devFamily);

                /* check if received config is equal to original */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.ipv4Cfg,
                                             (GT_VOID*) &configGet.ipv4Cfg,
                                             configSize)) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                           "got another config then was set: %d", dev);
            }

            /*  1.1.4. */
            if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
            else
                tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
            config.ipv4Cfg.destIp.arIP[0] = 255;
            config.ipv4Cfg.destIp.arIP[1] = 255;
            config.ipv4Cfg.destIp.arIP[2] = 255;
            config.ipv4Cfg.destIp.arIP[3] = 255;

            config.ipv4Cfg.srcIp.arIP[0] = 255;
            config.ipv4Cfg.srcIp.arIP[1] = 255;
            config.ipv4Cfg.srcIp.arIP[2] = 255;
            config.ipv4Cfg.srcIp.arIP[3] = 251;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }

            /*  1.1.5. */
            config.ipv4Cfg.destIp.arIP[0] = 10;
            config.ipv4Cfg.destIp.arIP[1] = 15;
            config.ipv4Cfg.destIp.arIP[2] = 1;
            config.ipv4Cfg.destIp.arIP[3] = 250;

            config.ipv4Cfg.srcIp.arIP[0] = 10;
            config.ipv4Cfg.srcIp.arIP[1] = 15;
            config.ipv4Cfg.srcIp.arIP[2] = 1;
            config.ipv4Cfg.srcIp.arIP[3] = 255;

            config.ipv4Cfg.tagEnable = GT_TRUE;
            config.ipv4Cfg.vlanId    = 4096;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv4Cfg.vlanId = %d",
                                         dev, config.ipv4Cfg.vlanId);

            config.ipv4Cfg.vlanId = 100;

            /*  1.1.6. */
            config.ipv4Cfg.vlanId    = 100;
            config.ipv4Cfg.tagEnable = GT_TRUE;

            if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.ipv4Cfg.upMarkMode);
            }

            /*  1.1.7. */
            config.ipv4Cfg.tagEnable  = GT_TRUE;
            config.ipv4Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.up         = 8;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                            "%d, ->up = %d", dev, config.ipv4Cfg.up);

            config.ipv4Cfg.up = 0;

            /*  1.1.8. */
            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.dscp         = 64;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                            "%d, ->dscp = %d", dev, config.ipv4Cfg.dscp);

            config.ipv4Cfg.dscp = 0;

            /*  1.1.9. */
            config.ipv4Cfg.ttl = 256;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv4Cfg.ttl = %d",
                                             dev, config.ipv4Cfg.ttl);

            config.ipv4Cfg.ttl = 0;

            /*  1.1.10. */
            config.ipv4Cfg.autoTunnel       = GT_TRUE;
            config.ipv4Cfg.autoTunnelOffset = 16;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, ->ipv4Cfg.autoTunnelOffset = %d",
                                             dev, config.ipv4Cfg.autoTunnelOffset);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /*  1.1.11. */
                lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

                tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

                config.ipv4Cfg.tagEnable    = GT_TRUE;
                config.ipv4Cfg.vlanId       = 100;
                config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.up           = 0;
                config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.dscp         = 0;
                config.ipv4Cfg.cfi          = 0;
                config.ipv4Cfg.retainCRC    = GT_FALSE;

                config.ipv4Cfg.macDa.arEther[0] = 0x0;
                config.ipv4Cfg.macDa.arEther[1] = 0x1A;
                config.ipv4Cfg.macDa.arEther[2] = 0xFF;
                config.ipv4Cfg.macDa.arEther[3] = 0xFF;
                config.ipv4Cfg.macDa.arEther[4] = 0xFF;
                config.ipv4Cfg.macDa.arEther[5] = 0xFF;

                config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
                config.ipv4Cfg.ttl              = 0;
                config.ipv4Cfg.autoTunnel       = GT_TRUE;
                config.ipv4Cfg.autoTunnelOffset = 1;

                config.ipv4Cfg.destIp.arIP[0] = 10;
                config.ipv4Cfg.destIp.arIP[1] = 15;
                config.ipv4Cfg.destIp.arIP[2] = 1;
                config.ipv4Cfg.destIp.arIP[3] = 250;

                config.ipv4Cfg.srcIp.arIP[0] = 10;
                config.ipv4Cfg.srcIp.arIP[1] = 15;
                config.ipv4Cfg.srcIp.arIP[2] = 1;
                config.ipv4Cfg.srcIp.arIP[3] = 255;

                config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                config.ipv4Cfg.profileIndex           = 3;
                config.ipv4Cfg.greProtocolForEthernet = 0x0800;
                config.ipv4Cfg.greFlagsAndVersion     = 0xFFFF;
                config.ipv4Cfg.udpDstPort             = 0;
                config.ipv4Cfg.udpSrcPort             = 0;


                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);


                /*  1.1.12.  */
                lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

                tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

                config.ipv4Cfg.tagEnable    = GT_TRUE;
                config.ipv4Cfg.vlanId       = 4095;
                config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.up           = 3;
                config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.dscp         = 63;
                config.ipv4Cfg.cfi          = 1;
                config.ipv4Cfg.retainCRC    = GT_TRUE;

                config.ipv4Cfg.macDa.arEther[0] = 0xFF;
                config.ipv4Cfg.macDa.arEther[1] = 0xFF;
                config.ipv4Cfg.macDa.arEther[2] = 0xFF;
                config.ipv4Cfg.macDa.arEther[3] = 0xFF;
                config.ipv4Cfg.macDa.arEther[4] = 0x01;
                config.ipv4Cfg.macDa.arEther[5] = 0x10;

                config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
                config.ipv4Cfg.ttl              = 255;
                config.ipv4Cfg.autoTunnel       = GT_FALSE;
                config.ipv4Cfg.autoTunnelOffset = 0;

                config.ipv4Cfg.destIp.arIP[0] = 255;
                config.ipv4Cfg.destIp.arIP[1] = 255;
                config.ipv4Cfg.destIp.arIP[2] = 255;
                config.ipv4Cfg.destIp.arIP[3] = 255;

                config.ipv4Cfg.srcIp.arIP[0] = 255;
                config.ipv4Cfg.srcIp.arIP[1] = 255;
                config.ipv4Cfg.srcIp.arIP[2] = 255;
                config.ipv4Cfg.srcIp.arIP[3] = 255;

                config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                config.ipv4Cfg.profileIndex           = 3;
                config.ipv4Cfg.greProtocolForEthernet = 0xFFFF;
                config.ipv4Cfg.greFlagsAndVersion     = 0x0101;
                config.ipv4Cfg.udpDstPort             = 0;
                config.ipv4Cfg.udpSrcPort             = 0;


                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);


                /*  1.1.13. */
                lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

                tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

                config.ipv4Cfg.tagEnable    = GT_FALSE;
                config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.up           = 3;
                config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.dscp         = 32;
                config.ipv4Cfg.cfi          = 1;
                config.ipv4Cfg.retainCRC    = GT_TRUE;

                config.ipv4Cfg.macDa.arEther[0] = 0xFF;
                config.ipv4Cfg.macDa.arEther[1] = 0xFF;
                config.ipv4Cfg.macDa.arEther[2] = 0xFF;
                config.ipv4Cfg.macDa.arEther[3] = 0xFF;
                config.ipv4Cfg.macDa.arEther[4] = 0x01;
                config.ipv4Cfg.macDa.arEther[5] = 0x10;

                config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
                config.ipv4Cfg.ttl              = 128;
                config.ipv4Cfg.autoTunnel       = GT_FALSE;
                config.ipv4Cfg.autoTunnelOffset = 0;

                config.ipv4Cfg.destIp.arIP[0] = 255;
                config.ipv4Cfg.destIp.arIP[1] = 255;
                config.ipv4Cfg.destIp.arIP[2] = 255;
                config.ipv4Cfg.destIp.arIP[3] = 255;

                config.ipv4Cfg.srcIp.arIP[0] = 255;
                config.ipv4Cfg.srcIp.arIP[1] = 255;
                config.ipv4Cfg.srcIp.arIP[2] = 255;
                config.ipv4Cfg.srcIp.arIP[3] = 255;

                config.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
                config.ipv4Cfg.profileIndex           = 7;
                config.ipv4Cfg.greProtocolForEthernet = 0;
                config.ipv4Cfg.greFlagsAndVersion     = 0;
                config.ipv4Cfg.udpDstPort             = 0xFFFF;
                config.ipv4Cfg.udpSrcPort             = 0xFFFE;


                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_TUNNEL_GENERIC_IPV4_E, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                /*  1.1.14.  */
                config.ipv4Cfg.tagEnable    = GT_TRUE;
                config.ipv4Cfg.vlanId       = 4096;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.vlanId       = 0;

                /*  1.1.15.  */
                config.ipv4Cfg.tagEnable    = GT_TRUE;
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.ipv4Cfg.upMarkMode);

                /*  1.1.16.  */
                config.ipv4Cfg.tagEnable    = GT_TRUE;
                config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.up           = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.up       = 0;

                /*  1.1.17.  */
                config.ipv4Cfg.tagEnable    = GT_TRUE;
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.ipv4Cfg.dscpMarkMode);

                /*  1.1.18.  */
                config.ipv4Cfg.tagEnable    = GT_TRUE;
                config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv4Cfg.dscp         = 64;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.dscp       = 0;

                /*  1.1.19.  */
                config.ipv4Cfg.cfi         = 2;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.cfi         = 0;

                /*  1.1.20.  */
                config.ipv4Cfg.ttl         = 256;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.ttl         = 0;

                /*  1.1.21.  */
                config.ipv4Cfg.autoTunnel        = GT_TRUE;
                config.ipv4Cfg.autoTunnelOffset  = 13;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.autoTunnelOffset         = 0;

                /*  1.1.22.  */
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEntrySet
                                    (dev, lineIndex, tunnelType, &config),
                                    config.ipv4Cfg.ipHeaderProtocol);

                /*  1.1.23.  */
                config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
                config.ipv4Cfg.profileIndex  = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.profileIndex         = 0;

                /*  1.1.24.  */
                config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                config.ipv4Cfg.greProtocolForEthernet  = 0x10000;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.greProtocolForEthernet         = 0;

                /*  1.1.25.  */
                config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                config.ipv4Cfg.greFlagsAndVersion  = 0x10000;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.greFlagsAndVersion         = 0;

                /*  1.1.26.  */
                config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
                config.ipv4Cfg.udpDstPort  = 0x10000;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.udpDstPort         = 0;

                /*  1.1.27.  */
                config.ipv4Cfg.ipHeaderProtocol        = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
                config.ipv4Cfg.udpSrcPort  = 0x10000;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                config.ipv4Cfg.udpSrcPort         = 0;
            }

            /*  1.1.28.  */
            cpssOsBzero((GT_VOID*) &config, sizeof(config));
            cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

            lineIndex  = 10;
            tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

            config.mplsCfg.tagEnable  = GT_TRUE;
            config.mplsCfg.vlanId     = 100;
            config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.up         = 7;
            config.mplsCfg.cfi        = 0;

            config.mplsCfg.macDa.arEther[0] = 0x0;
            config.mplsCfg.macDa.arEther[1] = 0x1A;
            config.mplsCfg.macDa.arEther[2] = 0xFF;
            config.mplsCfg.macDa.arEther[3] = 0xFF;
            config.mplsCfg.macDa.arEther[4] = 0xFF;
            config.mplsCfg.macDa.arEther[5] = 0xFF;


            if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                config.mplsCfg.numLabels    = 3;
            }else
            {
                config.mplsCfg.numLabels    = 2;
            }

            config.mplsCfg.ttl          = 10;
            config.mplsCfg.label1       = 100;
            config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.exp1         = 7;
            config.mplsCfg.label2       = 200;
            config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.exp2         = 7;
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
            {
                config.mplsCfg.pushEliAndElAfterLabel1 = GT_TRUE;
                config.mplsCfg.pushEliAndElAfterLabel2 = GT_TRUE;
                config.mplsCfg.pushEliAndElAfterLabel3 = GT_TRUE;
            }

            if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                config.mplsCfg.label3       = 300;
                config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.mplsCfg.exp3         = 7;
                config.mplsCfg.setSBit      = GT_TRUE;
            }

            config.mplsCfg.retainCRC    = GT_FALSE;
            config.mplsCfg.controlWordEnable = GT_FALSE;
            config.mplsCfg.controlWordIndex = 1;
            config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }

            /*  1.1.29.  */
            lineIndex  = 10;
            tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

            config.mplsCfg.tagEnable  = GT_TRUE;
            config.mplsCfg.vlanId     = 4095;
            config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.up         = 7;
            config.mplsCfg.cfi        = 0;

            config.mplsCfg.macDa.arEther[0] = 0x0;
            config.mplsCfg.macDa.arEther[1] = 0x1A;
            config.mplsCfg.macDa.arEther[2] = 0xFF;
            config.mplsCfg.macDa.arEther[3] = 0xFF;
            config.mplsCfg.macDa.arEther[4] = 0xFF;
            config.mplsCfg.macDa.arEther[5] = 0xFF;

            if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                config.mplsCfg.numLabels    = 3;
            }else
            {
                config.mplsCfg.numLabels    = 2;
            }

            config.mplsCfg.ttl          = 255;
            config.mplsCfg.label1       = 1048575;
            config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mplsCfg.exp1         = 2;
            config.mplsCfg.label2       = 1048575;
            config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mplsCfg.exp2         = 3;

            if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                config.mplsCfg.label3       = 1048575;
                config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
                config.mplsCfg.exp3         = 1;
                config.mplsCfg.setSBit      = GT_TRUE;
            }

            config.mplsCfg.retainCRC    = GT_FALSE;
            config.mplsCfg.controlWordEnable = GT_TRUE;

            config.mplsCfg.controlWordIndex = MAX_INDEX_CONTROL_WORD(dev);

            config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }

            /*  1.1.30.  */
            lineIndex  = 10;
            tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

            config.mplsCfg.tagEnable  = GT_TRUE;
            config.mplsCfg.vlanId     = 4095;
            config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.up         = 7;
            config.mplsCfg.cfi        = 1;

            config.mplsCfg.macDa.arEther[0] = 0x0;
            config.mplsCfg.macDa.arEther[1] = 0x1A;
            config.mplsCfg.macDa.arEther[2] = 0xFF;
            config.mplsCfg.macDa.arEther[3] = 0x15;
            config.mplsCfg.macDa.arEther[4] = 0x3F;
            config.mplsCfg.macDa.arEther[5] = 0xF3;

            if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                config.mplsCfg.numLabels    = 3;
            }else
            {
                config.mplsCfg.numLabels    = 2;
            }

            config.mplsCfg.ttl          = 255;
            config.mplsCfg.label1       = 524287;
            config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mplsCfg.exp1         = 5;
            config.mplsCfg.label2       = 1048575;
            config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.exp2         = 1;

            if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                config.mplsCfg.label3       = 1048575;
                config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
                config.mplsCfg.exp3         = 1;
                config.mplsCfg.setSBit      = GT_TRUE;
            }

            if(GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,
               PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E))
            {
                config.mplsCfg.retainCRC    = GT_TRUE;
            }else
            {
                config.mplsCfg.retainCRC    = GT_FALSE;
            }
            config.mplsCfg.controlWordEnable = GT_TRUE;
            config.mplsCfg.controlWordIndex = 1;
            config.mplsCfg.mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);
            }

            /*  1.1.31.  */
            configTmp = config;

            cpssOsBzero((GT_VOID*) &config, sizeof(config));
            cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

            tunnelType = CPSS_TUNNEL_TRILL_E;
            config.trillCfg.tagEnable = GT_FALSE;
            config.trillCfg.trillHeader.version = 0;
            config.trillCfg.trillHeader.opLength = 0;
            config.trillCfg.trillHeader.hopCount = 0;
            config.trillCfg.trillHeader.eRbid = 2;
            config.trillCfg.trillHeader.iRbid = 1;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }

            /* restore valid configuration */
            config = configTmp;
            tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E;

            /*  1.1.32.  */
            config.mplsCfg.tagEnable = GT_TRUE;
            config.mplsCfg.vlanId    = 4096;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, ->mplsCfg.vlanId = %d",
                                         dev, config.mplsCfg.vlanId);

            config.mplsCfg.vlanId = 100;

            /*  1.1.33.  */
            config.mplsCfg.tagEnable  = GT_TRUE;

            if (!prvUtfIsPbrModeUsed())
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.mplsCfg.upMarkMode);
            }

            /*  1.1.34.  */
            config.mplsCfg.tagEnable  = GT_TRUE;
            config.mplsCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.up         = 8;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ->up = %d", dev, config.mplsCfg.up);

            config.mplsCfg.up = 0;

            /*  1.1.35.  */
            /* Call with configPtr->mplsCfg.numLabels [0] */
            config.mplsCfg.numLabels = 0;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.numLabels = %d",
                                             dev, config.mplsCfg.numLabels);

            /* Call with configPtr->mplsCfg.numLabels [5] */
            config.mplsCfg.numLabels = 5;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.numLabels = %d",
                                             dev, config.mplsCfg.numLabels);

            config.mplsCfg.numLabels = 2;

            /*  1.1.36.  */
            config.mplsCfg.ttl = 256;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.ttl = %d",
                                             dev, config.mplsCfg.ttl);

            config.mplsCfg.ttl = 10;

            /*  1.1.37.  */
            config.mplsCfg.label1 = 1048576;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.label1 = %d",
                                             dev, config.mplsCfg.label1);

            config.mplsCfg.label1 = 0;

            /*  1.1.39.  */
            config.mplsCfg.numLabels    = 2;
            config.mplsCfg.label1       = 100;

            if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex,tunnelType, &config),
                                    config.mplsCfg.exp1MarkMode);
            }

            /*  1.1.40.  */
            config.mplsCfg.numLabels    = 2;
            config.mplsCfg.exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.exp1         = 8;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                "%d, ->mplsCfg.exp1 = %d", dev, config.mplsCfg.exp1);

            config.mplsCfg.exp1 = 0;

            /*  1.1.41.  */
            config.mplsCfg.numLabels = 2;
            config.mplsCfg.label2    = 1048576;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.label2 = %d",
                                             dev, config.mplsCfg.label2);

            config.mplsCfg.label2 = 200;

            /*  1.1.42.  */
            if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.mplsCfg.exp2MarkMode);
            }

            /*  1.1.43.  */
            config.mplsCfg.exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mplsCfg.exp2         = 8;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, configPtr->mplsCfg.exp2 = %d",
                                             dev, config.mplsCfg.exp2);

            config.mplsCfg.exp2 = 0;

            /* run tests with values relevant for xCat and above devices only */
            if (PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                /*  1.1.44.  */
                config.mplsCfg.numLabels = 3;
                config.mplsCfg.label3    = 1048576;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.label2 = %d",
                                                 dev, config.mplsCfg.label2);

                config.mplsCfg.label3 = 300;

                /*  1.1.44.  */
                if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
                {
                    UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                        (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                        config.mplsCfg.exp3MarkMode);
                }

                /*  1.1.45.  */
                config.mplsCfg.exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.mplsCfg.exp3         = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->mplsCfg.exp2 = %d",
                                                 dev, config.mplsCfg.exp2);
            }

            /*  1.1.46.  */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart;
            tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

            cpssOsBzero((GT_VOID*) &config, sizeof(config));
            cpssOsBzero((GT_VOID*) &configGet, sizeof(configGet));

            config.ipv4Cfg.tagEnable    = GT_TRUE;
            config.ipv4Cfg.vlanId       = 100;
            config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.up           = 0;
            config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.ipv4Cfg.dscp         = 0;
            config.ipv4Cfg.cfi          = 0;

            config.ipv4Cfg.macDa.arEther[0] = 0x0;
            config.ipv4Cfg.macDa.arEther[1] = 0x1A;
            config.ipv4Cfg.macDa.arEther[2] = 0xFF;
            config.ipv4Cfg.macDa.arEther[3] = 0xFF;
            config.ipv4Cfg.macDa.arEther[4] = 0xFF;
            config.ipv4Cfg.macDa.arEther[5] = 0xFF;

            config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
            config.ipv4Cfg.ttl              = 0;
            config.ipv4Cfg.autoTunnel       = GT_TRUE;
            config.ipv4Cfg.autoTunnelOffset = 1;

            config.ipv4Cfg.destIp.arIP[0] = 10;
            config.ipv4Cfg.destIp.arIP[1] = 15;
            config.ipv4Cfg.destIp.arIP[2] = 1;
            config.ipv4Cfg.destIp.arIP[3] = 250;

            config.ipv4Cfg.srcIp.arIP[0] = 10;
            config.ipv4Cfg.srcIp.arIP[1] = 15;
            config.ipv4Cfg.srcIp.arIP[2] = 1;
            config.ipv4Cfg.srcIp.arIP[3] = 255;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, lineIndex);


            /*  1.1.47.  */
            tunnelType = CPSS_TUNNEL_MAC_IN_MAC_E;
            lineIndex = 20;

            config.mimCfg.tagEnable = GT_TRUE;
            config.mimCfg.vlanId = 4095;
            config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.up = 7;

            config.mimCfg.macDa.arEther[0] = 0xAF;
            config.mimCfg.macDa.arEther[1] = 0xBF;
            config.mimCfg.macDa.arEther[2] = 0xCF;
            config.mimCfg.macDa.arEther[3] = 0xDF;
            config.mimCfg.macDa.arEther[4] = 0xEF;
            config.mimCfg.macDa.arEther[5] = 0xFF;

            config.mimCfg.retainCrc = GT_FALSE;
            config.mimCfg.iSid = 0xFFFFFF;
            config.mimCfg.iUp = 7;
            config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.iDp = 1;
            config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.iTagReserved = 15;
            config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E;
            config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
            if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

            }else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }

            /*  1.1.48.  */
            config.mimCfg.tagEnable = GT_FALSE;
            config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mimCfg.up = 3;

            config.mimCfg.macDa.arEther[0] = 0;
            config.mimCfg.macDa.arEther[1] = 0;
            config.mimCfg.macDa.arEther[2] = 0;
            config.mimCfg.macDa.arEther[3] = 0;
            config.mimCfg.macDa.arEther[4] = 0;
            config.mimCfg.macDa.arEther[5] = 0;

            config.mimCfg.retainCrc = GT_TRUE;
            config.mimCfg.iSid = 0x1FFFFF;
            config.mimCfg.iUp = 3;
            config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mimCfg.iDp = 1;
            config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mimCfg.iTagReserved = 7;
            config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E;
            config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
            if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E &&
               PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) == GT_FALSE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

            }else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }

            /*  1.1.49.  */
            config.mimCfg.tagEnable = GT_TRUE;
            config.mimCfg.vlanId = 2048;
            config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.up = 1;

            config.mimCfg.macDa.arEther[0] = 0xFF;
            config.mimCfg.macDa.arEther[1] = 0xFF;
            config.mimCfg.macDa.arEther[2] = 0xFF;
            config.mimCfg.macDa.arEther[3] = 0xFF;
            config.mimCfg.macDa.arEther[4] = 0xFF;
            config.mimCfg.macDa.arEther[5] = 0xFF;

            config.mimCfg.retainCrc = GT_TRUE;
            config.mimCfg.iSid = 0x1F1FF1;
            config.mimCfg.iUp = 0;
            config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mimCfg.iDp = 1;
            config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.iTagReserved = 7;
            config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E;
            config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
            if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E &&
               PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) == GT_FALSE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

            }else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }

            /*  1.1.50.  */
            config.mimCfg.tagEnable = GT_TRUE;
            config.mimCfg.vlanId = 1;
            config.mimCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.up = 0;

            config.mimCfg.macDa.arEther[0] = 0;
            config.mimCfg.macDa.arEther[1] = 0;
            config.mimCfg.macDa.arEther[2] = 0xAD;
            config.mimCfg.macDa.arEther[3] = 0xFF;
            config.mimCfg.macDa.arEther[4] = 0xEE;
            config.mimCfg.macDa.arEther[5] = 0xFF;

            config.mimCfg.retainCrc = GT_TRUE;
            config.mimCfg.iSid = 0;
            config.mimCfg.iUp = 0;
            config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            config.mimCfg.iDp = 0;
            config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
            config.mimCfg.iTagReserved = 0;
            config.mimCfg.iSidAssignMode = CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E;
            config.mimCfg.bDaAssignMode = CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
            if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E &&
               PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) == GT_FALSE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

            }else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
            }


            if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                /*  1.1.51.  */
                config.mimCfg.tagEnable = GT_TRUE;
                config.mimCfg.vlanId = 4096;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
                config.mimCfg.vlanId = 0;

                /*  1.1.52.  */
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp,
                                     lineIndex, tunnelType, &config),
                                    config.mimCfg.upMarkMode);

                /*  1.1.53.  */
                config.mimCfg.up = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
                config.mimCfg.up = 0;

                /*  1.1.54.  */
                config.mimCfg.iSid = 0x1000000;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
                config.mimCfg.iSid = 0;

                /*  1.1.55.  */
                config.mimCfg.iUpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.mimCfg.iUp = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
                config.mimCfg.iUp = 0;

                /*  1.1.56.  */
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp,
                                     lineIndex, tunnelType, &config),
                                    config.mimCfg.iUpMarkMode);

                /*  1.1.57.  */
                config.mimCfg.iDpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.mimCfg.iDp = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
                config.mimCfg.iDp = 0;

                /*  1.1.58.  */
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp,
                                     lineIndex, tunnelType, &config),
                                    config.mimCfg.iDpMarkMode);

                /*  1.1.59.  */
                config.mimCfg.iTagReserved = 16;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);
                config.mimCfg.iTagReserved = 0;

                if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                {
                    /*  1.1.60.  */
                    UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp,
                                         lineIndex, tunnelType, &config),
                                        config.mimCfg.iSidAssignMode);

                    /*  1.1.61.  */
                    UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp,
                                         lineIndex, tunnelType, &config),
                                        config.mimCfg.bDaAssignMode);
                }
            }

            if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) {

                /*  1.1.62.  */
                /* Call with tunnelType [CPSS_TUNNEL_GENERIC_IPV6_E - CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E] */
                tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

                config.ipv6Cfg.tagEnable    = GT_TRUE;
                config.ipv6Cfg.vlanId       = 100;
                config.ipv6Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.up           = 0;
                config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.dscp         = 0;
                config.ipv6Cfg.retainCRC    = GT_TRUE;

                config.ipv6Cfg.macDa.arEther[0] = 0x0;
                config.ipv6Cfg.macDa.arEther[1] = 0x1A;
                config.ipv6Cfg.macDa.arEther[2] = 0xFF;
                config.ipv6Cfg.macDa.arEther[3] = 0xFF;
                config.ipv6Cfg.macDa.arEther[4] = 0xFF;
                config.ipv6Cfg.macDa.arEther[5] = 0xFF;

                config.ipv6Cfg.ttl              = 0;

                config.ipv6Cfg.destIp.arIP[0] = 10;
                config.ipv6Cfg.destIp.arIP[1] = 15;
                config.ipv6Cfg.destIp.arIP[2] = 1;
                config.ipv4Cfg.destIp.arIP[3] = 250;

                config.ipv6Cfg.srcIp.arIP[0] = 10;
                config.ipv6Cfg.srcIp.arIP[1] = 15;
                config.ipv6Cfg.srcIp.arIP[2] = 1;
                config.ipv6Cfg.srcIp.arIP[3] = 255;

                config.ipv6Cfg.ipHeaderProtocol     = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                /* profile index 7 for legacy support (empty profile: no extension data is added in tunnel header)*/
                config.ipv6Cfg.profileIndex         = 1;
                /*If the passenger is IPv4, the GRE protocol is fixed: 0x0800*/
                config.ipv6Cfg.greProtocolForEthernet   = 0x0800;
                config.ipv6Cfg.greFlagsAndVersion       = 0;
                /*not relevant*/
                config.ipv6Cfg.udpDstPort               = 0;
                config.ipv6Cfg.udpSrcPort               = 0;
                config.ipv6Cfg.flowLabelMode = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                /*  1.1.63.  */
                tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

                config.ipv6Cfg.tagEnable    = GT_TRUE;
                config.ipv6Cfg.vlanId       = 4095;
                config.ipv6Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.up           = 7;
                config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.dscp         = 63;
                config.ipv6Cfg.retainCRC    = GT_FALSE;

                config.ipv6Cfg.macDa.arEther[0] = 0xFF;
                config.ipv6Cfg.macDa.arEther[1] = 0xFF;
                config.ipv6Cfg.macDa.arEther[2] = 0xFF;
                config.ipv6Cfg.macDa.arEther[3] = 0xFF;
                config.ipv6Cfg.macDa.arEther[4] = 0xFF;
                config.ipv6Cfg.macDa.arEther[5] = 0xFF;

                config.ipv6Cfg.ttl              = 255;

                config.ipv6Cfg.destIp.u32Ip[0] = 0xF3F1FF1F;
                config.ipv6Cfg.destIp.u32Ip[1] = 0xFAF1FFEF;
                config.ipv6Cfg.destIp.u32Ip[2] = 0xFFFFFFFF;
                config.ipv6Cfg.destIp.u32Ip[3] = 0xF3F5FF1F;

                config.ipv6Cfg.srcIp.u32Ip[0] = 0xF3FFFFFF;
                config.ipv6Cfg.srcIp.u32Ip[1] = 0xFFF6F4FF;
                config.ipv6Cfg.srcIp.u32Ip[2] = 0xF1FFF5FF;
                config.ipv6Cfg.srcIp.u32Ip[3] = 0xF1F6AFF7;

                config.ipv6Cfg.ipHeaderProtocol     = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                /* profile index 7 for legacy support (empty profile: no extension data is added in tunnel header)*/
                config.ipv6Cfg.profileIndex         = 3;
                /*If the passenger is IPv4, the GRE protocol is fixed: 0x0800*/
                config.ipv6Cfg.greProtocolForEthernet   = 0x0800;
                config.ipv6Cfg.greFlagsAndVersion       = 0xFFFF;


                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                /*  1.1.64.  */
                tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

                config.ipv6Cfg.tagEnable    = GT_TRUE;
                config.ipv6Cfg.vlanId       = 2048;
                config.ipv6Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.up           = 3;
                config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.dscp         = 32;
                config.ipv6Cfg.retainCRC    = GT_FALSE;

                config.ipv6Cfg.macDa.arEther[0] = 0xFF;
                config.ipv6Cfg.macDa.arEther[1] = 0xFF;
                config.ipv6Cfg.macDa.arEther[2] = 0x3F;
                config.ipv6Cfg.macDa.arEther[3] = 0xAF;
                config.ipv6Cfg.macDa.arEther[4] = 0xF1;
                config.ipv6Cfg.macDa.arEther[5] = 0x1F;

                config.ipv6Cfg.ttl              = 128;

                config.ipv6Cfg.destIp.u32Ip[0] = 0xFFFFFFFF;
                config.ipv6Cfg.destIp.u32Ip[1] = 0xFFFFFFFF;
                config.ipv6Cfg.destIp.u32Ip[2] = 0xFFFFFFFF;
                config.ipv6Cfg.destIp.u32Ip[3] = 0xFFFFFFFF;

                config.ipv6Cfg.srcIp.u32Ip[0] = 0xFFFFFFFF;
                config.ipv6Cfg.srcIp.u32Ip[1] = 0xFFFFFFFF;
                config.ipv6Cfg.srcIp.u32Ip[2] = 0xFFFFFFFF;
                config.ipv6Cfg.srcIp.u32Ip[3] = 0xFFFFFFFF;

                config.ipv6Cfg.ipHeaderProtocol     = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
                /* profile index 7 for legacy support (empty profile: no extension data is added in tunnel header)*/
                config.ipv6Cfg.profileIndex         = 7;

                config.ipv6Cfg.greProtocolForEthernet   = 0x0800;
                config.ipv6Cfg.greFlagsAndVersion       = 0xFFFF;

                config.ipv6Cfg.udpDstPort               = 0xFFFF;
                config.ipv6Cfg.udpSrcPort               = 0xFFFF;
                config.ipv6Cfg.flowLabelMode = CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                cpssOsBzero((GT_VOID*)&configGet, sizeof(configGet));

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);

                /* compare values */
                UTF_VERIFY_EQUAL3_STRING_MAC(tunnelType, tunnelTypeGet, "tunnelType != tunnelTypeGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                st = prvCompareTunnelStartConfigStructs(dev, "config", tunnelType, &config, &configGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "config != configGet: %d, %d, %d",
                                             dev, lineIndex, tunnelType);

                /*  1.1.65.  */
                tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

                config.ipv6Cfg.tagEnable = GT_TRUE;
                config.ipv6Cfg.vlanId    = 4096;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
                    "%d, configPtr->ipv4Cfg.tagEnable = %d, configPtr->ipv4Cfg.vlanId = %d",
                                dev, config.ipv6Cfg.tagEnable, config.ipv6Cfg.vlanId);

                /*  1.1.66.  */
                config.ipv6Cfg.vlanId     = 100;
                config.ipv6Cfg.tagEnable  = GT_TRUE;

                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.ipv6Cfg.upMarkMode);

                /*  1.1.67.  */
                config.ipv6Cfg.tagEnable  = GT_TRUE;
                config.ipv6Cfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.up         = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
                    "%d, configPtr->ipv4Cfg.tagEnable = %d, ->upMarkMode = %d, ->up = %d",
                    dev, config.ipv6Cfg.tagEnable, config.ipv6Cfg.upMarkMode, config.ipv6Cfg.up);

                config.ipv6Cfg.up = 0;

                /*  1.1.68.  */
                config.ipv6Cfg.tagEnable    = GT_TRUE;
                config.ipv6Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.ipv6Cfg.dscp         = 64;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
                    "%d, configPtr->ipv6Cfg.tagEnable = %d, ->dscpMarkMode = %d, ->dscp = %d",
                    dev, config.ipv6Cfg.tagEnable, config.ipv6Cfg.dscpMarkMode, config.ipv6Cfg.dscp);

                config.ipv6Cfg.dscp = 0;

                /*  1.1.69.  */
                config.ipv6Cfg.ttl = 256;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv6Cfg.ttl = %d",
                                                 dev, config.ipv6Cfg.ttl);

                config.ipv6Cfg.ttl = 0;

                /*  1.1.70.  */
                config.ipv6Cfg.profileIndex = 8;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, configPtr->ipv6Cfg.profileIndex = %d",
                                                 dev, config.ipv6Cfg.profileIndex);

                config.ipv6Cfg.profileIndex = 7;

                /*  1.1.71.  */
                config.ipv6Cfg.tagEnable  = GT_TRUE;

                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.ipv6Cfg.ipHeaderProtocol);

                /*  1.1.72.  */
                config.ipv6Cfg.tagEnable  = GT_TRUE;

                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    config.ipv6Cfg.flowLabelMode);
            }

            if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* Call with Generic Tunnel Start type */
                tunnelType = CPSS_TUNNEL_GENERIC_E;
                config.genCfg.upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                config.genCfg.up = 0;
                config.genCfg.tagEnable = GT_FALSE;
                config.genCfg.vlanId = 9;
                config.genCfg.genericType = CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E;
                config.genCfg.retainCrc = GT_FALSE;
                config.genCfg.macDa.arEther[0] = 0x00;
                config.genCfg.macDa.arEther[1] = 0x1A;
                config.genCfg.macDa.arEther[2] = 0xFF;
                config.genCfg.macDa.arEther[3] = 0xFF;
                config.genCfg.macDa.arEther[4] = 0xFF;
                config.genCfg.macDa.arEther[5] = 0xFF;
                config.genCfg.etherType = 0x0889;
                config.genCfg.data[0] = 0x33;
                config.genCfg.data[1] = 0x35;
                config.genCfg.data[4] = 0x45;
                config.genCfg.data[12] = 0x55;
                config.genCfg.data[35] = 0x88;

                st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex, tunnelType, &config);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex, &tunnelTypeGet, &configGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, lineIndex, tunnelType);

                UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_TUNNEL_GENERIC_E, tunnelTypeGet,
                            "got another tunnelType then was set: %d", dev);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.genCfg,
                                        (GT_VOID*) &configGet.genCfg,
                                        sizeof(config.genCfg))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                            "got another config then was set: %d", dev);
            }

            /*  1.1.73.  */
            lineIndex  = 10;

            if (!prvUtfIsPbrModeUsed() || (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupEntrySet
                                    (dev, portGroupsBmp, lineIndex, tunnelType, &config),
                                    tunnelType);
            }

            /*  1.1.74.  */

            /*  1.1.75.  */
            tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, NULL);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, configPtr = NULL", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configPtr = NULL", dev);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore valid parameters */
        lineIndex  = 10;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
            config.ipv4Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
        }
        else
        {
            tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
        }

        config.ipv4Cfg.tagEnable    = GT_TRUE;
        config.ipv4Cfg.vlanId       = 100;
        config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.up           = 0;
        config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        config.ipv4Cfg.dscp         = 0;
        config.ipv4Cfg.cfi          = 0;

        config.ipv4Cfg.macDa.arEther[0] = 0x0;
        config.ipv4Cfg.macDa.arEther[1] = 0x1A;
        config.ipv4Cfg.macDa.arEther[2] = 0xFF;
        config.ipv4Cfg.macDa.arEther[3] = 0xFF;
        config.ipv4Cfg.macDa.arEther[4] = 0xFF;
        config.ipv4Cfg.macDa.arEther[5] = 0xFF;

        config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
        config.ipv4Cfg.ttl              = 0;
        config.ipv4Cfg.autoTunnel       = GT_TRUE;
        config.ipv4Cfg.autoTunnelOffset = 1;

        config.ipv4Cfg.destIp.arIP[0] = 10;
        config.ipv4Cfg.destIp.arIP[1] = 15;
        config.ipv4Cfg.destIp.arIP[2] = 1;
        config.ipv4Cfg.destIp.arIP[3] = 250;

        config.ipv4Cfg.srcIp.arIP[0] = 10;
        config.ipv4Cfg.srcIp.arIP[1] = 15;
        config.ipv4Cfg.srcIp.arIP[2] = 1;
        config.ipv4Cfg.srcIp.arIP[3] = 255;


        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                      tunnelType, &config);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                  tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
        }
    }

    portGroupsBmp = 1;

    lineIndex  = 10;
    tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;

    config.ipv4Cfg.tagEnable    = GT_TRUE;
    config.ipv4Cfg.vlanId       = 100;
    config.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.up           = 0;
    config.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    config.ipv4Cfg.dscp         = 0;
    config.ipv4Cfg.cfi          = 0;

    config.ipv4Cfg.macDa.arEther[0] = 0x0;
    config.ipv4Cfg.macDa.arEther[1] = 0x1A;
    config.ipv4Cfg.macDa.arEther[2] = 0xFF;
    config.ipv4Cfg.macDa.arEther[3] = 0xFF;
    config.ipv4Cfg.macDa.arEther[4] = 0xFF;
    config.ipv4Cfg.macDa.arEther[5] = 0xFF;

    config.ipv4Cfg.dontFragmentFlag = GT_TRUE;
    config.ipv4Cfg.ttl              = 0;
    config.ipv4Cfg.autoTunnel       = GT_TRUE;
    config.ipv4Cfg.autoTunnelOffset = 1;

    config.ipv4Cfg.destIp.arIP[0] = 10;
    config.ipv4Cfg.destIp.arIP[1] = 15;
    config.ipv4Cfg.destIp.arIP[2] = 1;
    config.ipv4Cfg.destIp.arIP[3] = 250;

    config.ipv4Cfg.srcIp.arIP[0] = 10;
    config.ipv4Cfg.srcIp.arIP[1] = 15;
    config.ipv4Cfg.srcIp.arIP[2] = 1;
    config.ipv4Cfg.srcIp.arIP[3] = 255;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                                  tunnelType, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartPortGroupEntrySet(dev, portGroupsBmp, lineIndex,
                                              tunnelType, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartPortGroupEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN   GT_U32                             routerArpTunnelStartLineIndex,
    OUT  CPSS_TUNNEL_TYPE_ENT               *tunnelTypePtr,
    OUT  CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *configPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPortGroupEntryGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(APPLICABLE DEVICES: DxCh2; DxCh3; xCat; xCat3; Lion)
    1.1.1. Call with routerArpTunnelStartLineIndex[max - 1]
                     non-null tunnelTypePtr
                     and non-null configPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range routerArpTunnelStartLineIndex [max],
                     non-null tunnelTypePtr
                     and non-null configPtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with routerArpTunnelStartLineIndex[max - 1],
                     tunnelTypePtr[NULL]
                     and non-null configPtr.
    Expected: GT_BAD_PTR.
    1.1.4. Call with routerArpTunnelStartLineIndex[max - 1],
                     non-null tunnelTypePtr
                     and configPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId   = 0;

    GT_U32                              lineIndex  = 0;
    CPSS_TUNNEL_TYPE_ENT                tunnelType = CPSS_TUNNEL_X_OVER_IPV4_E;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   config;

    cpssOsBzero((GT_VOID*) &config, sizeof(config));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with routerArpTunnelStartLineIndex[max - 1]
                                 non-null tunnelTypePtr
                                 and non-null configPtr.
                Expected: GT_OK.
            */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

            st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                      &tunnelType, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, lineIndex);
            }

            /*
                1.1.2. Call with out of range routerArpTunnelStartLineIndex [max],
                                 non-null tunnelTypePtr
                                 and non-null configPtr.
                Expected: GT_BAD_PARAM.
            */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart;

            st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                      &tunnelType, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, lineIndex);

            /*
                1.1.3. Call with routerArpTunnelStartLineIndex[max - 1],
                                 tunnelTypePtr[NULL]
                                 and non-null configPtr.
                Expected: GT_BAD_PTR.
            */
            lineIndex  = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;

            st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                      NULL, &config);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, tunnelTypePtr = NULL", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tunnelTypePtr = NULL", dev);
            }

            /*
                1.1.4. Call with routerArpTunnelStartLineIndex[max - 1],
                                 non-null tunnelTypePtr
                                 and configPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                      &tunnelType, NULL);
            if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, configPtr = NULL", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, configPtr = NULL", dev);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                      &tunnelType, &config);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                  &tunnelType, &config);
        if (prvUtfIsPbrModeUsed() && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
        }
    }

    portGroupsBmp = 1;

    lineIndex = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices return GT_OK.
       for non multi core the API ignores the portGroupsBmp  */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                                  &tunnelType, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartPortGroupEntryGet(dev, portGroupsBmp, lineIndex,
                                              &tunnelType, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet
(
    IN  GT_U8   dev,
    OUT GT_U32  *additionToLengthPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet)
{
/*
    ITERATE_DEVICES(DxChXcat)
    1.1. Call with not null additionToLengthPtr.
    Expected: GT_OK.
    1.2. Call api with wrong additionToLengthPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_U32  additionToLength;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null additionToLengthPtr.
            Expected: GT_OK.
        */
        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(dev, &additionToLength);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call api with wrong additionToLengthPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(dev, NULL);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, additionToLengthPtr = NULL", dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(dev, &additionToLength);
        if (GT_TRUE == PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(dev, &additionToLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet
(
    IN  GT_U8  dev,
    IN  GT_U32 additionToLength
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet)
{
/*
    ITERATE_DEVICES(DxChXcat)
    1.1. Call with additionToLength[0 / BIT_3 / BIT_6 - 1],
    Expected: GT_OK.
    1.2. Call cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong additionToLength [BIT_6 + 1].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS st = GT_OK;

    GT_U8  dev;
    GT_U32 additionToLength = 0;
    GT_U32 additionToLengthGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with additionToLength[0 / BIT_3 / BIT_6 - 1],
            Expected: GT_OK.
        */
        /* call with additionToLength[0] */
        additionToLength = 0;

        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(dev, additionToLength);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(dev, &additionToLengthGet);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(additionToLength, additionToLengthGet,
                       "got another additionToLength then was set: %d", dev);
        }

        /* call with additionToLength[BIT_3] */
        additionToLength = BIT_3;

        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(dev, additionToLength);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* call get func again, 1.2. */
        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(dev, &additionToLengthGet);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(additionToLength, additionToLengthGet,
                       "got another additionToLength then was set: %d", dev);
        }

        /* call with additionToLength[BIT_6 - 1] */
        additionToLength = BIT_6 - 1;

        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(dev, additionToLength);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* call get func again, 1.2. */
        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet(dev, &additionToLengthGet);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(additionToLength, additionToLengthGet,
                       "got another additionToLength then was set: %d", dev);
        }

        /*
            1.3. Call api with wrong additionToLength [BIT_6 + 1].
            Expected: GT_OUT_OF_RANGE.
        */
        additionToLength = BIT_6 + 1;

        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(dev, additionToLength);

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        }

        additionToLength = 0;
    }

    /* restore correct values */
    additionToLength = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(dev, additionToLength);
        if (GT_TRUE == PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet(dev, additionToLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet
(
    IN  GT_U8    dev,
    IN  GT_U8    port,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(DxChXcat)
    1.1.1. Call with not null enable pointer.
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8    dev;
    GT_PHYSICAL_PORT_NUM port   = TUNNEL_VALID_PHY_PORT_CNS;
    GT_BOOL  enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not null enable pointer.
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                         port, &enable);
            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                         port, NULL);
            if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
            else
            {
                if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                    GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                           "%d, enablePtr = NULL", dev);
                }
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                         port, &enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                     port, &enable);
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                     port, &enable);
        if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                    GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
            }
        }
    }

    /* restore correct values */
    port = TUNNEL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                     port, &enable);
        if (GT_TRUE == PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                     port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet
(
    IN  GT_U8   dev,
    IN  GT_U8   port,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS(DxChXcat)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_PHYSICAL_PORT_NUM  port = 0;
    GT_BOOL enable = GT_FALSE;
    GT_BOOL enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            /* call with enable[GT_TRUE] */
            enable = GT_TRUE;

            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(dev,
                                         port, enable);
            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            /*
                1.1.2. Call cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                         port, &enableGet);
            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
            }

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(dev,
                                         port, enable);
            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            /* call get func again, 1.1.2. */
            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet(dev,
                                         port, &enableGet);
            if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
            else
            {
                if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                    GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet: %d ", dev);

                    /* Verifying values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                               "got another enable then was set: %d", dev);
                }
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(dev,
                                         port, enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(dev,
                                     port, enable);
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
            GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(dev,
                                     port, enable);
        if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                    GT_TRUE != PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
            }
        }
    }

    /* restore correct values */
    port = TUNNEL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(dev,
                                     port, enable);
        if (GT_TRUE == PRV_CPSS_DXCH_XCAT_A3_ONLY_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet(dev,
                                     port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwControlWordSet
(
    IN  GT_U8           devNum,
    IN  GT_U8           index,
    IN  GT_U32          value
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwControlWordSet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3, xCat, xCat3)
    1.1. Call with index[0],
    Expected: GT_OK
    1.2. Call with index[14/6],
    Expected: GT_OK
    1.3. Call with index[15/7],
    Expected: GT_BAD_PARAM
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_U32      index, indexMax;
    GT_U32      value, getValue;
    GT_STATUS   expectedSt = GT_OK;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /* 1.1. check that the function returns GT_OK */
        index = 0;
        value = 0xA5A5A5A5;
        st = cpssDxChTunnelStartMplsPwControlWordSet(dev, index, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expectedSt, st, dev, index, value);

        /* 1.2 Check the configured value */
        st = cpssDxChTunnelStartMplsPwControlWordGet(dev, index, &getValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, index);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(value, getValue,
                                     "get another value than was set: device %d, index %d",
                                     dev, index);
        }

        /* 1.3. check that the function returns GT_OK */
        index = indexMax = MAX_INDEX_CONTROL_WORD(dev);
        value = 0xC3C3C3C3;
        st = cpssDxChTunnelStartMplsPwControlWordSet(dev, index, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(expectedSt, st, dev, index, value);

        /* 1.4 Check the configured value */
        st = cpssDxChTunnelStartMplsPwControlWordGet(dev, index, &getValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, index);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(value, getValue,
                                     "get another value than was set: device %d, index %d",
                                     dev, index);
        }

        /* 1.5. check that the function returns GT_BAD_PARAM for illegal index */
        index = indexMax+1;
        st = cpssDxChTunnelStartMplsPwControlWordSet(dev, index, value);
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_BAD_PARAM;
        }
        UTF_VERIFY_EQUAL3_PARAM_MAC(expectedSt, st, dev, index, value);

    }

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        index = value = 0;
        st = cpssDxChTunnelStartMplsPwControlWordSet(dev, index, value);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index, value);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwControlWordGet
(
    IN  GT_U8           devNum,
    IN  GT_U8           index,
    IN  GT_U32          *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwControlWordGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1. Call with index[0],
    Expected: GT_OK
    1.2. Call with index[14/6],
    Expected: GT_OK
    1.3. Call with index[15/7],
    Expected: GT_BAD_PARAM
    1.4. Call with valuePtr = NULL,
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_U32      index, indexMax;
    GT_U32      value;
    GT_STATUS   expectedSt = GT_OK;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /* 1.1. check that the function returns GT_OK */
        index = 0;
        st = cpssDxChTunnelStartMplsPwControlWordGet(dev, index, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, index);

        /* 1.2. check that the function returns GT_OK */
        index = indexMax = MAX_INDEX_CONTROL_WORD(dev);
        st = cpssDxChTunnelStartMplsPwControlWordGet(dev, index, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, index);

        /* 1.3. check that the function returns GT_BAD_PARAM for illegal index */
        index = indexMax+1;
        st = cpssDxChTunnelStartMplsPwControlWordGet(dev, index, &value);
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_BAD_PARAM;
        }
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, index);


        /* 1.4. check that the function returns GT_BAD_PTR for value = NULL */
        st = cpssDxChTunnelStartMplsPwControlWordGet(dev, index, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
    }

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        index = value = 0;
        st = cpssDxChTunnelStartMplsPwControlWordGet(dev, index, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwETreeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_U8           index,
    IN  GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwETreeEnableSet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1. Call with index = 0 and enable = GT_TRUE,
    Expected: GT_OK
    1.2. Call with index = 0 and enable = GT_FALSE,
    Expected: GT_OK
    1.3. Call with index = 14 and enable = GT_TRUE,
    Expected: GT_OK
    1.4. Call with index = 14 and enable = GT_FALSE,
    Expected: GT_OK
    1.5. Call with index = 15,
    Expected: GT_BAD_PARAM
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_U32      notAppFamilyBmp;
    GT_U32      index;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     getEnable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with index = 0 and enable = GT_TRUE */
        index = 0;
        enable = GT_TRUE;
        /* 1.1.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsPwETreeEnableSet(dev, index, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, enable);

        /* 1.1.2. Check the configured value */
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &getEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, getEnable,
                                     "get another value than was set: device %d, index %d",
                                     dev, index);

        /* 1.2. Call with index = 0 and enable = GT_FALSE */
        enable = GT_FALSE;
        /* 1.2.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsPwETreeEnableSet(dev, index, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, enable);

        /* 1.2.2. Check the configured value */
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &getEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, getEnable,
                                     "get another value than was set: device %d, index %d",
                                     dev, index);

        /* 1.3. Call with index = 14 and enable = GT_TRUE */
        index = 14;
        enable = GT_TRUE;
        /* 1.3.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsPwETreeEnableSet(dev, index, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, enable);

        /* 1.3.2. Check the configured value */
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &getEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, getEnable,
                                     "get another value than was set: device %d, index %d",
                                     dev, index);

        /* 1.4. Call with index = 14 and enable = GT_FALSE */
        enable = GT_FALSE;
        /* 1.4.1. check that the function returns GT_OK */
        st = cpssDxChTunnelStartMplsPwETreeEnableSet(dev, index, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, enable);

        /* 1.4.2. Check the configured value */
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &getEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, getEnable,
                                     "get another value than was set: device %d, index %d",
                                     dev, index);

        /* 1.5. Call with index = 15 and check that the function returns GT_BAD_PARAM */
        index = 15;
        st = cpssDxChTunnelStartMplsPwETreeEnableSet(dev, index, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index, enable);
    }

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        index = 0;
        st = cpssDxChTunnelStartMplsPwETreeEnableSet(dev, index, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index, enable);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartMplsPwETreeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartMplsPwETreeEnableGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1. Call with index[0],
    Expected: GT_OK
    1.2. Call with index[14],
    Expected: GT_OK
    1.3. Call with index[15],
    Expected: GT_BAD_PARAM
    1.4. Call with valuePtr = NULL,
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_U32      notAppFamilyBmp;
    GT_U32      index;
    GT_BOOL     value;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. check that the function returns GT_OK */
        index = 0;
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* 1.2. check that the function returns GT_OK */
        index = 14;
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* 1.3. check that the function returns GT_BAD_PARAM for illegal index */
        index = 15;
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, index);

        /* 1.4. check that the function returns GT_BAD_PTR for value = NULL */
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, index);
    }

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        index = value = 0;
        st = cpssDxChTunnelStartMplsPwETreeEnableGet(dev, index, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, index);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartGenProfileTableEntrySet)
{

    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_U32                              profileIndex; /*(APPLICABLE RANGES: 0...7)*/
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC   profileDataPtr;

    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&profileDataPtr, sizeof(profileDataPtr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1 Call with profileIndex [0]
                            profileDataPtr.templateDataSize [CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E],
                            profileDataPtr.templateDataBitsCfg [0],
                            profileDataPtr.hashShiftLeftBitsNumber [0]
                            profileDataPtr.udpSrcPortMode [CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E].
            Expected: GT_OK.
        */
        profileIndex = 0;

        profileDataPtr.templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E;
        profileDataPtr.templateDataBitsCfg[0] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E;
        profileDataPtr.hashShiftLeftBitsNumber = 0;
        profileDataPtr.udpSrcPortMode = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;
        profileDataPtr.serviceIdCircularShiftSize = 0;
        profileDataPtr.macDaMode = 0;
        profileDataPtr.dipMode = 0;
        profileDataPtr.controlWordIndex = 0;

        st = cpssDxChTunnelStartGenProfileTableEntrySet (dev, profileIndex, &profileDataPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /*
            1.2 check out of range profileIndex.
                            Call with profileIndex [wrong value], other parameters same as in 1.1
            Expected: NON GT_OK.
        */
        profileIndex=8;
        st = cpssDxChTunnelStartGenProfileTableEntrySet (dev, profileIndex, &profileDataPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /*
            1.3 check out of range profileIndex.hashShiftLeftBitsNumber
                            Call with profileIndex.hashShiftLeftBitsNumber [wrong value], other parameters same as in 1.1
            Expected: NON GT_OK.
        */
        profileIndex = 0;
        profileDataPtr.hashShiftLeftBitsNumber = 8;
        st = cpssDxChTunnelStartGenProfileTableEntrySet (dev, profileIndex, &profileDataPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);


        /*
            1.4 Call withwrong enum values profileDataPtr.templateDataSize}
                            and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        profileDataPtr.hashShiftLeftBitsNumber = 0;
        UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartGenProfileTableEntrySet
                            (dev, profileIndex, &profileDataPtr),
                            profileDataPtr.templateDataSize);

        /*
            1.5 Call with wrong enum values profileDataPtr.udpSrcPortMode}
                            and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartGenProfileTableEntrySet
                            (dev, profileIndex, &profileDataPtr),
                            profileDataPtr.udpSrcPortMode);

        if (PRV_CPSS_SIP_5_15_CHECK_MAC(dev) == GT_TRUE)
        {
            /*
            1.6 Call with wrong value profileDataPtr.serviceIdCircularShiftSize
                            and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
            */
            profileDataPtr.serviceIdCircularShiftSize = MAX_GENERIC_IP_PROFILE_SERVICE_ID_CIRCULAR_SIFT_SIZE_CNS + 1;
            st = cpssDxChTunnelStartGenProfileTableEntrySet (dev, profileIndex, &profileDataPtr);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

            /*
                1.7 Call with wrong value profileDataPtr.genericTsMacDaMode
                                and other parameters from 1.1.
                Expected: GT_BAD_PARAM.
            */
            profileDataPtr.serviceIdCircularShiftSize = 0;
            profileDataPtr.macDaMode = MAX_GENERIC_IP_PROFILE_MAC_DA_MODE_CNS + 1;
            st = cpssDxChTunnelStartGenProfileTableEntrySet (dev, profileIndex, &profileDataPtr);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

            /*
                1.8 Call with wrong value profileDataPtr.genericTsProfileDipMode
                                and other parameters from 1.1.
                Expected: GT_BAD_PARAM.
            */
            profileDataPtr.macDaMode = 0;
            profileDataPtr.dipMode = MAX_GENERIC_IP_PROFILE_DIP_MODE_CNS + 1;
            st = cpssDxChTunnelStartGenProfileTableEntrySet (dev, profileIndex, &profileDataPtr);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

            /*
                1.9 Call with wrong value profileDataPtr.genericTsMacDaMode
                                and other parameters from 1.1.
                Expected: GT_BAD_PARAM.
            */
            profileDataPtr.dipMode = 0;
            profileDataPtr.controlWordIndex = MAX_GENERIC_IP_PROFILE_TABLE_CONTROL_WORD_INDEX_CNS + 1;
            st = cpssDxChTunnelStartGenProfileTableEntrySet (dev, profileIndex, &profileDataPtr);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    profileIndex = 7;
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartGenProfileTableEntrySet(dev, profileIndex, &profileDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartGenProfileTableEntrySet(dev, profileIndex, &profileDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartGenProfileTableEntryGet)
{

    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_U32                              profileIndex; /*(APPLICABLE RANGES: 0...7)*/
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC   profileDataPtr;

    GT_U32      notAppFamilyBmp        = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*) &profileDataPtr, sizeof(profileDataPtr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1 Check with legal parameters. Call with profileIndex [0].
            Expected: GT_OK
        */
        profileIndex = 0;

        st = cpssDxChTunnelStartGenProfileTableEntryGet (dev, profileIndex, &profileDataPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /*
            1.2 check out of range profileIndex.
                            Call with profileIndex [wrong value], other parameters same as in 1.1
            Expected: NON GT_OK.
        */
        profileIndex=8;
        st = cpssDxChTunnelStartGenProfileTableEntryGet (dev, profileIndex, &profileDataPtr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /*
            1.4 Call with profileDataPtr [NULL], other parameters same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTunnelStartGenProfileTableEntryGet (dev, profileIndex, NULL);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, "%d, profileDataPtr = NULL", dev);
    }

     /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    profileIndex = 7;
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartGenProfileTableEntryGet(dev, profileIndex, &profileDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartGenProfileTableEntryGet(dev, profileIndex, &profileDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEgessVlanTableServiceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U16               vlanId,
    OUT GT_U32               *vlanServiceIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEgessVlanTableServiceIdGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1. Call with vlanId[0, 1024, 8191] and
        valid value of vlanServiceIdPtr[NON-NULL].
    Expected: GT_OK
    1.2. Call with out of range vlanId[8192] and
        other  values the same as 1.1.
    Expected: NON GT_OK
    1.3. Call with out of range vlanServiceIdPtr[NULL] and
        other values same as 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st              = GT_OK;
    GT_U8           dev             = 0;
    GT_U32          notAppFamilyBmp;
    GT_U16          vlanId          = 0;
    GT_U32          vlanServiceId   = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId[0] and
                valid value of vlanServiceIdPtr[NON-NULL].
            Expected: GT_OK
        */
        vlanId = 0;
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with vlanId[1024] and
                valid value of vlanServiceIdPtr[NON-NULL].
            Expected: GT_OK
        */
        vlanId = 1024;
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with vlanId[8191] and
                valid value of vlanServiceIdPtr[NON-NULL].
            Expected: GT_OK
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range vlanId[8192] and
                other  values the same as 1.1.
            Expected: NON GT_OK
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore valid value */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1;

        /*
            1.3. Call with out of range vlanServiceIdPtr[NULL] and
                other values same as 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* restore valid values */
    vlanId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                       &vlanServiceId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEgessVlanTableServiceIdSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32               vlanServiceId
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEgessVlanTableServiceIdSet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1. Call with vlanId[0, 1024, 8191] and
        valid value of vlanServiceId[0, 0xAAAA, 0xFFFFFF].
    Expected: GT_OK
    1.2. Call cpssDxChTunnelStartEgessVlanTableServiceIdGet
       with the same parameters.
    Expected: GT_OK and the same values as set.
    1.3. Call with out of range vlanId[8192] and
        other  values the same as 1.1.
    Expected: NON GT_OK
    1.4. Call with out of range vlanServiceId[0x1000000] and
        other values same as 1.1.
    Expected: NON GT_OK.
*/
    GT_STATUS       st              = GT_OK;
    GT_U8           dev             = 0;
    GT_U32          notAppFamilyBmp;
    GT_U16          vlanId          = 0;
    GT_U32          vlanServiceId   = 0;
    GT_U32          vlanServiceIdRet   = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId[0] and
                valid value of vlanServiceId[0].
            Expected: GT_OK
        */
        vlanId = 0;
        vlanServiceId = 0;
        st = cpssDxChTunnelStartEgessVlanTableServiceIdSet(dev, vlanId,
                                                           vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call cpssDxChTunnelStartEgessVlanTableServiceIdGet
               with the same parameters.
            Expected: GT_OK and the same values as set.
        */
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceIdRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(vlanServiceId, vlanServiceIdRet,
                                     "got another values than was set: %d",
                                     dev);

        /*
            1.1. Call with vlanId[1024] and
                valid value of vlanServiceId[0xAAAA].
            Expected: GT_OK
        */
        vlanId = 1024;
        vlanServiceId = 0xAAAA;
        st = cpssDxChTunnelStartEgessVlanTableServiceIdSet(dev, vlanId,
                                                           vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call cpssDxChTunnelStartEgessVlanTableServiceIdGet
               with the same parameters.
            Expected: GT_OK and the same values as set.
        */
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceIdRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(vlanServiceId, vlanServiceIdRet,
                                     "got another values than was set: %d",
                                     dev);

        /*
            1.1. Call with vlanId[8191] and
                valid value of vlanServiceId[0xFFFFFF].
            Expected: GT_OK
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1;
        vlanServiceId = 0xFFFFFF;
        st = cpssDxChTunnelStartEgessVlanTableServiceIdSet(dev, vlanId,
                                                           vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. Call cpssDxChTunnelStartEgessVlanTableServiceIdGet
               with the same parameters.
            Expected: GT_OK and the same values as set.
        */
        st = cpssDxChTunnelStartEgessVlanTableServiceIdGet(dev, vlanId,
                                                           &vlanServiceIdRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(vlanServiceId, vlanServiceIdRet,
                                     "got another values than was set: %d",
                                     dev);


        /*
            1.3. Call with out of range vlanId[8192] and
                other  values the same as 1.1.
            Expected: NON GT_OK
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChTunnelStartEgessVlanTableServiceIdSet(dev, vlanId,
                                                           vlanServiceId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore valid value */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1;

        /*
            1.4. Call with out of range vlanServiceId[0x1000000] and
                other values same as 1.1.
            Expected: NON GT_OK.
        */
        vlanServiceId = 0x1000000;

        st = cpssDxChTunnelStartEgessVlanTableServiceIdSet(dev, vlanId,
                                                           vlanServiceId);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore valid value */
        vlanServiceId = 0xFFFFFF;
    }

    /* restore valid values */
    vlanId = 0;
    vlanServiceId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEgessVlanTableServiceIdSet(dev, vlanId,
                                                           vlanServiceId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTunnelStartEgessVlanTableServiceIdSet(dev, vlanId,
                                                       vlanServiceId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEntryExtensionGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *tsExtensionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEntryExtensionGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with valid value of tsExtensionPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range value of tsExtensionPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st                  = GT_OK;
    GT_U8           dev                 = 0;
    GT_PORT_NUM     port                = 0;
    GT_U32          notAppFamilyBmp;
    GT_U32          tsExtension         = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with valid value of tsExtensionPtr[NON-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtension);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with out of range value of tsExtensionPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartEntryExtensionGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtension);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtension);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtension);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtension);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtension);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEntryExtensionSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               tsExtension
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEntryExtensionSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with tsExtension [0, 0xAAAA, 0xFFFF].
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartEntryExtensionGet.
    Expected: GT_OK and the same values as set.
*/
    GT_STATUS       st                  = GT_OK;
    GT_U8           dev                 = 0;
    GT_PORT_NUM     port                = 0;
    GT_U32          notAppFamilyBmp;
    GT_U32          tsExtension         = 0;
    GT_U32          tsExtensionRet      = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with tsExtension [0, 0xAAAA, 0xFFFF].
                Expected: GT_OK.
            */
            tsExtension = 0;

            st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChTunnelStartEntryExtensionGet.
                Expected: GT_OK and the same values as set.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtensionRet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartEntryExtensionGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tsExtension, tsExtensionRet,
                                             "got another value: %d, %d",
                                             dev, port);
            }

            /*
                1.1.1. Call with tsExtension [0xAAAA].
                Expected: GT_OK.
            */
            tsExtension = 0xAAAA;

            st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChTunnelStartEntryExtensionGet.
                Expected: GT_OK and the same values as set.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtensionRet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartEntryExtensionGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tsExtension, tsExtensionRet,
                                             "got another value: %d, %d",
                                             dev, port);
            }

            /*
                1.1.1. Call with tsExtension [0xFFFF].
                Expected: GT_OK.
            */
            tsExtension = 0xFFFF;

            st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChTunnelStartEntryExtensionGet.
                Expected: GT_OK and the same values as set.
            */
            if (GT_OK == st)
            {
                st = cpssDxChTunnelStartEntryExtensionGet(dev, port, &tsExtensionRet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChTunnelStartEntryExtensionGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tsExtension, tsExtensionRet,
                                             "got another value: %d, %d",
                                             dev, port);
            }
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartEntryExtensionSet(dev, port, tsExtension);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartPortGroupGenProfileTableEntryGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    OUT CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC       *profileDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPortGroupGenProfileTableEntryGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with profileIndex[0, 3, 7] and
        valid value of profileDataPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range profileIndex[8] and
        other parameters the same as 1.1.1.
    Expected: NOT GT_OK.
    1.1.3. Call with invalid value of profileDataPtr[NULL] and
        other parameters the same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                                   st                  = GT_OK;
    GT_U8                                       dev                 = 0;
    GT_U32                                      notAppFamilyBmp     = 0;
    GT_U32                                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP                          portGroupsBmp       = 1;
    GT_U32                                      profileIndex        = 0;
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC   profileData;

    /* ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsBzero((GT_VOID*)&profileData, sizeof(profileData));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with profileIndex[0, 3, 7] and
                    valid value of profileDataPtr[NON-NULL].
                Expected: GT_OK.
            */
            profileIndex = 0;

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with profileIndex[0, 3, 7] and
                    valid value of profileDataPtr[NON-NULL].
                Expected: GT_OK.
            */
            profileIndex = 3;

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.1. Call with profileIndex[0, 3, 7] and
                    valid value of profileDataPtr[NON-NULL].
                Expected: GT_OK.
            */
            profileIndex = 7;

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with out of range profileIndex[8] and
                    other parameters the same as 1.1.1.
                Expected: NOT GT_OK.
            */
            profileIndex = 8;

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore valid value */
            profileIndex = 7;

            /*
                1.1.3. Call with invalid value of profileDataPtr[NULL] and
                    other parameters the same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                    portGroupsBmp, profileIndex,
                                                    &profileData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartPortGroupGenProfileTableEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           profileIndex,
    IN CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC        *profileDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartPortGroupGenProfileTableEntrySet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with profileIndex[0],
        profileData{
          templateDataSize [CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E],
          templateDataBitsCfg [0],
          hashShiftLeftBitsNumber [0]
          udpSrcPortMode
           [CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E]
        }.
    Expected: GT_OK.
    1.1.2. Call cpssDxChTunnelStartPortGroupGenProfileTableEntryGet.
    Expected: GT_OK and the same values as set.
    1.1.3. Call with out of range profileIndex[8] and
        other parameters the same as 1.1.1.
    Expected: NOT GT_OK.
    1.1.4. Call with out of range profileData.hashShiftLeftBitsNumber[8] and
        other parameters the same as 1.1.1.
    Expected: NOT GT_OK.
    1.1.5. Call with out of range enum value profileData.templateDataSizer
        and other parameters the same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with out of range enum value profileData.udpSrcPortMode
        and other parameters the same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.7. Call with out of range profileDataPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                   st                  = GT_OK;
    GT_U8                                       dev                 = 0;
    GT_U32                                      notAppFamilyBmp     = 0;
    GT_U32                                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP                          portGroupsBmp       = 1;
    GT_U32                                      profileIndex        = 0;
    GT_BOOL                                     isEqual             =GT_FALSE;
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC   profileData;
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC   profileDataRet;

    /* ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with profileIndex[0],
                    profileData{
                      templateDataSize
                           [CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E],
                      templateDataBitsCfg [0],
                      hashShiftLeftBitsNumber [0]
                      udpSrcPortMode
                           [CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E]}.
                Expected: GT_OK.
            */
            profileIndex = 0;

            cpssOsBzero((GT_VOID*) &profileData, sizeof(profileData));
            cpssOsBzero((GT_VOID*) &profileDataRet, sizeof(profileDataRet));
            profileData.templateDataSize =
                    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E;
            profileData.templateDataBitsCfg[0] = 0;
            profileData.hashShiftLeftBitsNumber = 0;
            profileData.udpSrcPortMode =
                    CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChTunnelStartPortGroupGenProfileTableEntryGet.
                Expected: GT_OK and the same values as set.
            */
            cpssOsBzero((GT_VOID*) &profileDataRet, sizeof(profileDataRet));

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntryGet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileDataRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&profileData,
                                         (GT_VOID*)&profileDataRet,
                                         sizeof (profileData))) ?
                                                             GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                         "get another values than was set: %d",
                                         dev);

            /*
                1.1.3. Call with out of range profileIndex[8] and
                    other parameters the same as 1.1.1.
                Expected: NOT GT_OK.
            */
            profileIndex = 8;

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore valid value */
            profileIndex = 0;

            /*
                1.1.4. Call with out of range profileData.hashShiftLeftBitsNumber[8] and
                    other parameters the same as 1.1.1.
                Expected: NOT GT_OK.
            */
            profileData.hashShiftLeftBitsNumber = 8;

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore valid value */
            profileData.hashShiftLeftBitsNumber = 0;

            /*
                1.1.5. Call with out of range enum value profileData.templateDataSize
                    and other parameters the same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupGenProfileTableEntrySet
                                (dev, portGroupsBmp, profileIndex, &profileData),
                                profileData.templateDataSize);

            /*
                1.1.6. Call with out of range enum value profileData.udpSrcPortMode
                    and other parameters the same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartPortGroupGenProfileTableEntrySet
                                (dev, portGroupsBmp, profileIndex, &profileData),
                                profileData.udpSrcPortMode);

            /*
                1.1.7. Call with out of range profileDataPtr[NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                                   portGroupsBmp, profileIndex,
                                                   &profileData);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                               portGroupsBmp, profileIndex,
                                               &profileData);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore valid values */
    profileIndex = 0;

    cpssOsBzero((GT_VOID*) &profileData, sizeof(profileData));
    profileData.templateDataSize =
            CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E;
    profileData.templateDataBitsCfg[0] = 0;
    profileData.hashShiftLeftBitsNumber = 0;
    profileData.udpSrcPortMode =
            CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                               portGroupsBmp, profileIndex,
                                               &profileData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChTunnelStartPortGroupGenProfileTableEntrySet(dev,
                                           portGroupsBmp, profileIndex,
                                           &profileData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEcnModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEcnModeSet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with ecnMode [CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E /
                            CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E]
    Expected: GT_OK.
    1.2. Call cpssDxChTunnelStartEcnModeGet with not NULL ecnModePtr.
    Expected: GT_OK and the same ecnModePtr as was set.
    1.3. Call with ecnMode [wrong enum values]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT ecnMode =
                            CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E;
    CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT ecnModeGet =
                            CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with ecnMode [CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E /
                                    CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E]
            Expected: GT_OK.
        */
        /* iterate with CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E */
        ecnMode = CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E;

        st = cpssDxChTunnelStartEcnModeSet(dev, ecnMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecnMode);

        /*
            1.2. Call cpssDxChTunnelStartEcnModeGet with not NULL ecnModePtr.
            Expected: GT_OK and the same ecnModePtr as was set.
        */
        st = cpssDxChTunnelStartEcnModeGet(dev, &ecnModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChTunnelStartEcnModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecnMode, ecnModeGet,
                    "get another ECN mode than was set: %d", dev);

        /* iterate with CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E */
        ecnMode = CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E;

        st = cpssDxChTunnelStartEcnModeSet(dev, ecnMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ecnMode);

        /*
            1.2. Call cpssDxChTunnelStartEcnModeGet with not NULL ecnModePtr.
            Expected: GT_OK and the same ecnModePtr as was set.
        */
        st = cpssDxChTunnelStartEcnModeGet(dev, &ecnModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChEthernetOverMplsTunnelStartTagModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ecnMode, ecnModeGet,
                    "get another ECN mode than was set: %d", dev);

        /*
            1.3. Call with ecnMode [wrong enum values]
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTunnelStartEcnModeSet
                            (dev, ecnMode),
                            ecnMode);
    }

    ecnMode = CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEcnModeSet(dev, ecnMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartEcnModeSet(dev, ecnMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTunnelStartEcnModeGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTunnelStartEcnModeGet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: DxCh3; xCat; xCat3; Lion)
    1.1. Call with not NULL ecnModePtr.
    Expected: GT_OK.
    1.2. Call ecnModePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT ecnMode =
                    CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL tagModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTunnelStartEcnModeGet(dev, &ecnMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call tagModePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChTunnelStartEcnModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, ecnModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTunnelStartEcnModeGet(dev, &ecnMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTunnelStartEcnModeGet(dev, &ecnMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTunnel suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTunnel)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChEthernetOverMplsTunnelStartTaggingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChEthernetOverMplsTunnelStartTaggingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChEthernetOverMplsTunnelStartTagModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChEthernetOverMplsTunnelStartTagModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPassengerVlanTranslationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPassengerVlanTranslationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelTtlSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelTtlGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsFlowLabelEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsFlowLabelEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsFlowLabelTtlSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsFlowLabelTtlGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelPushEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelPushEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelExpSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwLabelExpGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPushSourceBasedLabelEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartHeaderTpidSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartHeaderTpidSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartGenProfileTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartGenProfileTableEntryGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPortGroupEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPortGroupEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartIpTunnelTotalLengthOffsetGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartIpTunnelTotalLengthOffsetSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPortIpTunnelTotalLengthOffsetEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwControlWordSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwControlWordGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwETreeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartMplsPwETreeEnableGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEgessVlanTableServiceIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEgessVlanTableServiceIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEntryExtensionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEntryExtensionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPortGroupGenProfileTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartPortGroupGenProfileTableEntrySet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEcnModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTunnelStartEcnModeGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChTunnel)

/**
* @internal prvCompareTunnelStartConfigStructs function
* @endinternal
*
* @brief   This routine set compares 2 Action structures.
*
* @param[in] devNum                   - device number
* @param[in] name                     -  of tested sctucts objects
*                                      actionType        - type of the action to use
*                                      firstPtr          - (pointer to) first struct
*                                      secondPtr         - (pointer to) second struct
*/
static GT_STATUS prvCompareTunnelStartConfigStructs
(
    IN GT_U8                              devNum,
    IN GT_CHAR                            *name,
    IN CPSS_TUNNEL_TYPE_ENT               tunnelType,
    IN CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *setConfigPtr,
    IN CPSS_DXCH_TUNNEL_START_CONFIG_UNT  *getConfigPtr
)
{
    GT_STATUS st = GT_OK;
    GT_BOOL isEqual = GT_FALSE;


    switch(tunnelType)
        {

        case CPSS_TUNNEL_X_OVER_IPV4_E:
        case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        case CPSS_TUNNEL_GENERIC_IPV4_E:

            if(setConfigPtr->ipv4Cfg.tagEnable != getConfigPtr->ipv4Cfg.tagEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.tagEnable than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.tagEnable,
                                 getConfigPtr->ipv4Cfg.tagEnable);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->ipv4Cfg.tagEnable) &&
               (setConfigPtr->ipv4Cfg.vlanId != getConfigPtr->ipv4Cfg.vlanId))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.vlanId than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.vlanId,
                                 getConfigPtr->ipv4Cfg.vlanId);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->ipv4Cfg.tagEnable) &&
               (setConfigPtr->ipv4Cfg.upMarkMode != getConfigPtr->ipv4Cfg.upMarkMode))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.tagEnable than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.upMarkMode,
                                 getConfigPtr->ipv4Cfg.upMarkMode);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->ipv4Cfg.tagEnable) &&
               (CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->ipv4Cfg.upMarkMode) &&
               (setConfigPtr->ipv4Cfg.up != getConfigPtr->ipv4Cfg.up))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.up than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.up,
                                 getConfigPtr->ipv4Cfg.up);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv4Cfg.dscpMarkMode != getConfigPtr->ipv4Cfg.dscpMarkMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.dscpMarkMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.dscpMarkMode,
                                 getConfigPtr->ipv4Cfg.dscpMarkMode);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->ipv4Cfg.dscpMarkMode) &&
               (setConfigPtr->ipv4Cfg.dscp != getConfigPtr->ipv4Cfg.dscp))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.dscp than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.dscp,
                                 getConfigPtr->ipv4Cfg.dscp);
                st = GT_FAIL;
            }


            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&setConfigPtr->ipv4Cfg.macDa, (GT_VOID*)&getConfigPtr->ipv4Cfg.macDa,
                       (sizeof (setConfigPtr->ipv4Cfg.macDa.arEther[0])*6))) ? GT_TRUE : GT_FALSE;
            if(GT_FALSE == isEqual)
            {
                PRV_UTF_LOG1_MAC("get another %s.ipv4Cfg.macDa than was set\n",
                                 name);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv4Cfg.dontFragmentFlag != getConfigPtr->ipv4Cfg.dontFragmentFlag)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.dontFragmentFlag than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.dontFragmentFlag,
                                 getConfigPtr->ipv4Cfg.dontFragmentFlag);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv4Cfg.ttl != getConfigPtr->ipv4Cfg.ttl)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.ttl than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.ttl,
                                 getConfigPtr->ipv4Cfg.ttl);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv4Cfg.autoTunnel != getConfigPtr->ipv4Cfg.autoTunnel)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.autoTunnel than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.autoTunnel,
                                 getConfigPtr->ipv4Cfg.autoTunnel);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->ipv4Cfg.autoTunnel) &&
               (setConfigPtr->ipv4Cfg.autoTunnelOffset != getConfigPtr->ipv4Cfg.autoTunnelOffset))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.autoTunnelOffset than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.autoTunnelOffset,
                                 getConfigPtr->ipv4Cfg.autoTunnelOffset);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv4Cfg.destIp.u32Ip != getConfigPtr->ipv4Cfg.destIp.u32Ip)
            {
                PRV_UTF_LOG1_MAC("get another %s.ipv4Cfg.destIp than was set",
                                 name);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv4Cfg.srcIp.u32Ip != getConfigPtr->ipv4Cfg.srcIp.u32Ip)
            {
                PRV_UTF_LOG1_MAC("get another %s.ipv4Cfg.srcIp than was set",
                                 name);
                st = GT_FAIL;
            }

            if((PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum)) &&
               (setConfigPtr->ipv4Cfg.cfi != getConfigPtr->ipv4Cfg.cfi) &&
               (GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.cfi than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv4Cfg.cfi,
                                 getConfigPtr->ipv4Cfg.cfi);
                st = GT_FAIL;
            }

            if((GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)) &&
               (CPSS_TUNNEL_GENERIC_IPV4_E == tunnelType))
            {
                if(setConfigPtr->ipv4Cfg.retainCRC != getConfigPtr->ipv4Cfg.retainCRC)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.retainCRC than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->ipv4Cfg.retainCRC,
                                     getConfigPtr->ipv4Cfg.retainCRC);
                    st = GT_FAIL;
                }

                if(setConfigPtr->ipv4Cfg.ipHeaderProtocol != getConfigPtr->ipv4Cfg.ipHeaderProtocol)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.ipHeaderProtocol than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->ipv4Cfg.ipHeaderProtocol,
                                     getConfigPtr->ipv4Cfg.ipHeaderProtocol);
                    st = GT_FAIL;
                }

                if ((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)==GT_TRUE)||
                    (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum)==GT_TRUE)||
                        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)==GT_TRUE)
                {
                    if(setConfigPtr->ipv4Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E)
                    {
                        if(setConfigPtr->ipv4Cfg.ipProtocol != getConfigPtr->ipv4Cfg.ipProtocol)
                        {
                            PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.ipProtocol than was set, expected = %d, received = %d\n",
                                             name, setConfigPtr->ipv4Cfg.ipProtocol,
                                             getConfigPtr->ipv4Cfg.ipProtocol);
                            st = GT_FAIL;
                        }
                    }
                }

                if(setConfigPtr->ipv4Cfg.profileIndex != getConfigPtr->ipv4Cfg.profileIndex)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.profileIndex than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->ipv4Cfg.profileIndex,
                                     getConfigPtr->ipv4Cfg.profileIndex);
                    st = GT_FAIL;
                }

                if((setConfigPtr->ipv4Cfg.greProtocolForEthernet != getConfigPtr->ipv4Cfg.greProtocolForEthernet) &&
                   (CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E == setConfigPtr->ipv4Cfg.ipHeaderProtocol))
                {
                    PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.greProtocolForEthernet than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->ipv4Cfg.greProtocolForEthernet,
                                     getConfigPtr->ipv4Cfg.greProtocolForEthernet);
                    st = GT_FAIL;
                }

                if((setConfigPtr->ipv4Cfg.greFlagsAndVersion != getConfigPtr->ipv4Cfg.greFlagsAndVersion) &&
                   (CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E == setConfigPtr->ipv4Cfg.ipHeaderProtocol))
                {
                    PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.greFlagsAndVersion than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->ipv4Cfg.greFlagsAndVersion,
                                     getConfigPtr->ipv4Cfg.greFlagsAndVersion);
                    st = GT_FAIL;
                }

                if((setConfigPtr->ipv4Cfg.udpDstPort != getConfigPtr->ipv4Cfg.udpDstPort) &&
                   (CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E == setConfigPtr->ipv4Cfg.ipHeaderProtocol))
                {
                    PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.udpDstPort than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->ipv4Cfg.udpDstPort,
                                     getConfigPtr->ipv4Cfg.udpDstPort);
                    st = GT_FAIL;
                }

                if((setConfigPtr->ipv4Cfg.udpSrcPort != getConfigPtr->ipv4Cfg.udpSrcPort) &&
                   (CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E == setConfigPtr->ipv4Cfg.ipHeaderProtocol))
                {
                    PRV_UTF_LOG3_MAC("get another %s.ipv4Cfg.udpSrcPort than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->ipv4Cfg.udpSrcPort,
                                     getConfigPtr->ipv4Cfg.udpSrcPort);
                    st = GT_FAIL;
                }
            }

            break;

        case CPSS_TUNNEL_X_OVER_MPLS_E:

            if(setConfigPtr->mplsCfg.tagEnable   != getConfigPtr->mplsCfg.tagEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.tagEnable than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.tagEnable,
                                 getConfigPtr->mplsCfg.tagEnable);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->mplsCfg.tagEnable) &&
               (setConfigPtr->mplsCfg.vlanId != getConfigPtr->mplsCfg.vlanId))
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.vlanId than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.vlanId,
                                 getConfigPtr->mplsCfg.vlanId);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->mplsCfg.tagEnable) &&
               (setConfigPtr->mplsCfg.upMarkMode != getConfigPtr->mplsCfg.upMarkMode))
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.upMarkMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.upMarkMode,
                                 getConfigPtr->mplsCfg.upMarkMode);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->mplsCfg.tagEnable) &&
               (setConfigPtr->mplsCfg.up != getConfigPtr->mplsCfg.up))
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.up than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.up,
                                 getConfigPtr->mplsCfg.up);
                st = GT_FAIL;
            }

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&setConfigPtr->mplsCfg.macDa, (GT_VOID*)&getConfigPtr->mplsCfg.macDa,
                       (sizeof (setConfigPtr->mplsCfg.macDa.arEther[0])*6))) ? GT_TRUE : GT_FALSE;
            if(GT_FALSE == isEqual)
            {
                PRV_UTF_LOG1_MAC("get another %s.mplsCfg.macDa than was set\n",
                                 name);
                st = GT_FAIL;
            }

            if(setConfigPtr->mplsCfg.numLabels != getConfigPtr->mplsCfg.numLabels)
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.numLabels than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.numLabels,
                                 getConfigPtr->mplsCfg.numLabels);
                st = GT_FAIL;
            }

            if(setConfigPtr->mplsCfg.ttl != getConfigPtr->mplsCfg.ttl)
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.ttl than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.ttl,
                                 getConfigPtr->mplsCfg.ttl);
                st = GT_FAIL;
            }

            if ((setConfigPtr->mplsCfg.ttl == 0) &&
                 PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum) &&
                 setConfigPtr->mplsCfg.ttlMode != getConfigPtr->mplsCfg.ttlMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.ttlMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.ttlMode,
                                 getConfigPtr->mplsCfg.ttlMode);
                st = GT_FAIL;
            }

            if( setConfigPtr->mplsCfg.label1 != getConfigPtr->mplsCfg.label1)
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.label1 than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.label1,
                                 getConfigPtr->mplsCfg.label1);
                st = GT_FAIL;
            }

            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                if (setConfigPtr->mplsCfg.pushEliAndElAfterLabel1 != getConfigPtr->mplsCfg.pushEliAndElAfterLabel1)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.pushEliAndElAfterLabel1 than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.pushEliAndElAfterLabel1,
                                     getConfigPtr->mplsCfg.pushEliAndElAfterLabel1);
                    st = GT_FAIL;
                }
            }

            if( setConfigPtr->mplsCfg.exp1MarkMode != getConfigPtr->mplsCfg.exp1MarkMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.exp1MarkMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.exp1MarkMode,
                                 getConfigPtr->mplsCfg.exp1MarkMode);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->mplsCfg.exp1MarkMode) &&
                (setConfigPtr->mplsCfg.exp1 != getConfigPtr->mplsCfg.exp1))
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.exp1 than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.exp1,
                                 getConfigPtr->mplsCfg.exp1);
                st = GT_FAIL;
            }

            if(setConfigPtr->mplsCfg.numLabels >= 2)
            {
                if( setConfigPtr->mplsCfg.label2 != getConfigPtr->mplsCfg.label2)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.label2 than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.label2,
                                     getConfigPtr->mplsCfg.label2);
                    st = GT_FAIL;
                }

                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    if (setConfigPtr->mplsCfg.pushEliAndElAfterLabel2 != getConfigPtr->mplsCfg.pushEliAndElAfterLabel2)
                    {
                        PRV_UTF_LOG3_MAC("get another %s.mplsCfg.pushEliAndElAfterLabel2 than was set, expected = %d, received = %d\n",
                                         name, setConfigPtr->mplsCfg.pushEliAndElAfterLabel2,
                                         getConfigPtr->mplsCfg.pushEliAndElAfterLabel2);
                        st = GT_FAIL;
                    }
                }

                if( setConfigPtr->mplsCfg.exp2MarkMode != getConfigPtr->mplsCfg.exp2MarkMode)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.exp2MarkMode than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.exp2MarkMode,
                                     getConfigPtr->mplsCfg.exp2MarkMode);
                    st = GT_FAIL;
                }

                if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->mplsCfg.exp2MarkMode) &&
                    (setConfigPtr->mplsCfg.exp2 != getConfigPtr->mplsCfg.exp2))
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.exp2 than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.exp2,
                                     getConfigPtr->mplsCfg.exp2);
                    st = GT_FAIL;
                }
            }

            if( (setConfigPtr->mplsCfg.numLabels >= 3) &&
                PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
            {
                if( setConfigPtr->mplsCfg.label3 != getConfigPtr->mplsCfg.label3)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.label3 than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.label3,
                                     getConfigPtr->mplsCfg.label3);
                    st = GT_FAIL;
                }

                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    if (setConfigPtr->mplsCfg.pushEliAndElAfterLabel3 != getConfigPtr->mplsCfg.pushEliAndElAfterLabel3)
                    {
                        PRV_UTF_LOG3_MAC("get another %s.mplsCfg.pushEliAndElAfterLabel3 than was set, expected = %d, received = %d\n",
                                         name, setConfigPtr->mplsCfg.pushEliAndElAfterLabel3,
                                         getConfigPtr->mplsCfg.pushEliAndElAfterLabel3);
                        st = GT_FAIL;
                    }
                }

                if( setConfigPtr->mplsCfg.exp3MarkMode != getConfigPtr->mplsCfg.exp3MarkMode)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.exp3MarkMode than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.exp3MarkMode,
                                     getConfigPtr->mplsCfg.exp3MarkMode);
                    st = GT_FAIL;
                }

                if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->mplsCfg.exp3MarkMode) &&
                    (setConfigPtr->mplsCfg.exp3 != getConfigPtr->mplsCfg.exp3))
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.exp3 than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.exp3,
                                     getConfigPtr->mplsCfg.exp3);
                    st = GT_FAIL;
                }
            }

            if(setConfigPtr->mplsCfg.retainCRC != getConfigPtr->mplsCfg.retainCRC)
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.retainCRC than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.retainCRC,
                                 getConfigPtr->mplsCfg.retainCRC);
                st = GT_FAIL;
            }

            if( (setConfigPtr->mplsCfg.setSBit != getConfigPtr->mplsCfg.setSBit) &&
                PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.setSBit than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.setSBit,
                                 getConfigPtr->mplsCfg.setSBit);
                st = GT_FAIL;
            }

            if( (setConfigPtr->mplsCfg.cfi != getConfigPtr->mplsCfg.cfi) &&
                PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) &&
                (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE))
            {
                PRV_UTF_LOG3_MAC("get another %s.mplsCfg.cfi than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mplsCfg.cfi,
                                 getConfigPtr->mplsCfg.cfi);
                st = GT_FAIL;
            }


            if((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)||
                (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE))
            {
                if(setConfigPtr->mplsCfg.controlWordEnable   != getConfigPtr->mplsCfg.controlWordEnable)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.controlWordEnable than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.controlWordEnable,
                                     getConfigPtr->mplsCfg.controlWordEnable);
                    st = GT_FAIL;
                }

                if((setConfigPtr->mplsCfg.controlWordIndex   != getConfigPtr->mplsCfg.controlWordIndex) &&
                    (GT_TRUE == setConfigPtr->mplsCfg.controlWordEnable))
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.controlWordIndex than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.controlWordIndex,
                                     getConfigPtr->mplsCfg.controlWordIndex);
                    st = GT_FAIL;
                }
            }

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {

                if(setConfigPtr->mplsCfg.mplsEthertypeSelect   != getConfigPtr->mplsCfg.mplsEthertypeSelect)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mplsCfg.mplsEthertypeSelect than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mplsCfg.mplsEthertypeSelect,
                                     getConfigPtr->mplsCfg.mplsEthertypeSelect);
                    st = GT_FAIL;
                }
            }

            break;
        case CPSS_TUNNEL_MAC_IN_MAC_E:

            if(setConfigPtr->mimCfg.tagEnable   != getConfigPtr->mimCfg.tagEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.tagEnable than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.tagEnable,
                                 getConfigPtr->mimCfg.tagEnable);
                st = GT_FAIL;
            }

            if((setConfigPtr->mimCfg.vlanId   != getConfigPtr->mimCfg.vlanId) &&
                (GT_TRUE == setConfigPtr->mimCfg.tagEnable))
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.vlanId than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.vlanId,
                                 getConfigPtr->mimCfg.vlanId);
                st = GT_FAIL;
            }

            if((setConfigPtr->mimCfg.upMarkMode   != getConfigPtr->mimCfg.upMarkMode) &&
                (GT_TRUE == setConfigPtr->mimCfg.tagEnable))
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.upMarkMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.upMarkMode,
                                 getConfigPtr->mimCfg.upMarkMode);
                st = GT_FAIL;
            }

            if((setConfigPtr->mimCfg.up   != getConfigPtr->mimCfg.up) &&
                (GT_TRUE == setConfigPtr->mimCfg.tagEnable))
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.up than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.up,
                                 getConfigPtr->mimCfg.up);
                st = GT_FAIL;
            }

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&setConfigPtr->mimCfg.macDa, (GT_VOID*)&getConfigPtr->mimCfg.macDa,
                       (sizeof (setConfigPtr->mimCfg.macDa.arEther[0])*6))) ? GT_TRUE : GT_FALSE;
            if(GT_FALSE == isEqual)
            {
                PRV_UTF_LOG1_MAC("get another %s.mimCfg.macDa than was set\n",
                                 name);
                st = GT_FAIL;
            }

            if(setConfigPtr->mimCfg.retainCrc   != getConfigPtr->mimCfg.retainCrc)
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.retainCrc than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.retainCrc,
                                 getConfigPtr->mimCfg.retainCrc);
                st = GT_FAIL;
            }

            if((setConfigPtr->mimCfg.iSidAssignMode == CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E) &&
               (setConfigPtr->mimCfg.iSid   != getConfigPtr->mimCfg.iSid))
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.iSid than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.iSid,
                                 getConfigPtr->mimCfg.iSid);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->mimCfg.iUpMarkMode) &&
                (setConfigPtr->mimCfg.iUp   != getConfigPtr->mimCfg.iUp))
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.iUp than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.iUp,
                                 getConfigPtr->mimCfg.iUp);
                st = GT_FAIL;
            }

            if(setConfigPtr->mimCfg.iUpMarkMode   != getConfigPtr->mimCfg.iUpMarkMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.iUpMarkMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.iUpMarkMode,
                                 getConfigPtr->mimCfg.iUpMarkMode);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->mimCfg.iDpMarkMode) &&
                (setConfigPtr->mimCfg.iDp   != getConfigPtr->mimCfg.iDp))
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.iDp than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.iDp,
                                 getConfigPtr->mimCfg.iDp);
                st = GT_FAIL;
            }

            if(setConfigPtr->mimCfg.iTagReserved   != getConfigPtr->mimCfg.iTagReserved)
            {
                PRV_UTF_LOG3_MAC("get another %s.mimCfg.iTagReserved than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->mimCfg.iTagReserved,
                                 getConfigPtr->mimCfg.iTagReserved);
                st = GT_FAIL;
            }

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                if(setConfigPtr->mimCfg.iSidAssignMode   != getConfigPtr->mimCfg.iSidAssignMode)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mimCfg.iSidAssignMode than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mimCfg.iSidAssignMode,
                                     getConfigPtr->mimCfg.iSidAssignMode);
                    st = GT_FAIL;
                }

                if(setConfigPtr->mimCfg.bDaAssignMode   != getConfigPtr->mimCfg.bDaAssignMode)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mimCfg.bDaAssignMode than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mimCfg.bDaAssignMode,
                                     getConfigPtr->mimCfg.bDaAssignMode);
                    st = GT_FAIL;
                }

                if(setConfigPtr->mimCfg.bSaAssignMode   != getConfigPtr->mimCfg.bSaAssignMode)
                {
                    PRV_UTF_LOG3_MAC("get another %s.mimCfg.bSaAssignMode than was set, expected = %d, received = %d\n",
                                     name, setConfigPtr->mimCfg.bSaAssignMode,
                                     getConfigPtr->mimCfg.bSaAssignMode);
                    st = GT_FAIL;
                }
            }

            break;

        case CPSS_TUNNEL_GENERIC_IPV6_E:

            if(setConfigPtr->ipv6Cfg.tagEnable != getConfigPtr->ipv6Cfg.tagEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.tagEnable than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.tagEnable,
                                 getConfigPtr->ipv6Cfg.tagEnable);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->ipv6Cfg.tagEnable) &&
                (setConfigPtr->ipv6Cfg.vlanId != getConfigPtr->ipv6Cfg.vlanId))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.vlanId than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.vlanId,
                                 getConfigPtr->ipv6Cfg.vlanId);
                st = GT_FAIL;
            }

            if((GT_TRUE == setConfigPtr->ipv6Cfg.tagEnable) &&
                (setConfigPtr->ipv6Cfg.upMarkMode != getConfigPtr->ipv6Cfg.upMarkMode))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.upMarkMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.upMarkMode,
                                 getConfigPtr->ipv6Cfg.upMarkMode);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->ipv6Cfg.upMarkMode) &&
                (setConfigPtr->ipv6Cfg.up != getConfigPtr->ipv6Cfg.up))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.up than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.up,
                                 getConfigPtr->ipv6Cfg.up);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv6Cfg.dscpMarkMode != getConfigPtr->ipv6Cfg.dscpMarkMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.dscpMarkMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.dscpMarkMode,
                                 getConfigPtr->ipv6Cfg.dscpMarkMode);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->ipv6Cfg.dscpMarkMode) &&
                (setConfigPtr->ipv6Cfg.dscp != getConfigPtr->ipv6Cfg.dscp))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.dscp than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.dscp,
                                 getConfigPtr->ipv6Cfg.dscp);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E == setConfigPtr->ipv6Cfg.dscpMarkMode) &&
                (setConfigPtr->ipv6Cfg.dscp != getConfigPtr->ipv6Cfg.dscp))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.dscp than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.dscp,
                                 getConfigPtr->ipv6Cfg.dscp);
                st = GT_FAIL;
            }

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&setConfigPtr->ipv6Cfg.macDa, (GT_VOID*)&getConfigPtr->ipv6Cfg.macDa,
                       (sizeof (setConfigPtr->ipv6Cfg.macDa.arEther[0])*6))) ? GT_TRUE : GT_FALSE;
            if(GT_FALSE == isEqual)
            {
                PRV_UTF_LOG1_MAC("get another %s.ipv6Cfg.macDa than was set\n",
                                 name);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv6Cfg.ttl != getConfigPtr->ipv6Cfg.ttl)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.ttl than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.ttl,
                                 getConfigPtr->ipv6Cfg.ttl);
                st = GT_FAIL;
            }

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&setConfigPtr->ipv6Cfg.destIp.u32Ip, (GT_VOID*)&getConfigPtr->ipv6Cfg.destIp.u32Ip,
                       (sizeof (setConfigPtr->ipv6Cfg.destIp.u32Ip[0])*4))) ? GT_TRUE : GT_FALSE;
            if(GT_FALSE == isEqual)
            {
                PRV_UTF_LOG1_MAC("get another %s.ipv6Cfg.destIp than was set\n",
                                 name);
                st = GT_FAIL;
            }

            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&setConfigPtr->ipv6Cfg.srcIp.u32Ip, (GT_VOID*)&getConfigPtr->ipv6Cfg.srcIp.u32Ip,
                       (sizeof (setConfigPtr->ipv6Cfg.srcIp.u32Ip[0])*4))) ? GT_TRUE : GT_FALSE;
            if(GT_FALSE == isEqual)
            {
                PRV_UTF_LOG1_MAC("get another %s.ipv6Cfg.srcIp than was set\n",
                                 name);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv6Cfg.retainCRC != getConfigPtr->ipv6Cfg.retainCRC)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.retainCRC than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.retainCRC,
                                 getConfigPtr->ipv6Cfg.retainCRC);
                st = GT_FAIL;
            }

            if(setConfigPtr->ipv6Cfg.profileIndex != getConfigPtr->ipv6Cfg.profileIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.profileIndex than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.profileIndex,
                                 getConfigPtr->ipv6Cfg.profileIndex);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E == setConfigPtr->ipv6Cfg.ipHeaderProtocol) &&
                (setConfigPtr->ipv6Cfg.greProtocolForEthernet != getConfigPtr->ipv6Cfg.greProtocolForEthernet))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.greProtocolForEthernet than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.greProtocolForEthernet,
                                 getConfigPtr->ipv6Cfg.greProtocolForEthernet);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E == setConfigPtr->ipv6Cfg.ipHeaderProtocol) &&
                (setConfigPtr->ipv6Cfg.udpDstPort != getConfigPtr->ipv6Cfg.udpDstPort))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.udpDstPort than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.udpDstPort,
                                 getConfigPtr->ipv6Cfg.udpDstPort);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E == setConfigPtr->ipv6Cfg.ipHeaderProtocol) &&
                (setConfigPtr->ipv6Cfg.greFlagsAndVersion != getConfigPtr->ipv6Cfg.greFlagsAndVersion))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.greFlagsAndVersion than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.greFlagsAndVersion,
                                 getConfigPtr->ipv6Cfg.greFlagsAndVersion);
                st = GT_FAIL;
            }

            if((CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E == setConfigPtr->ipv6Cfg.ipHeaderProtocol) &&
                (setConfigPtr->ipv6Cfg.udpSrcPort != getConfigPtr->ipv6Cfg.udpSrcPort))
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.udpSrsPort than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.udpSrcPort,
                                 getConfigPtr->ipv6Cfg.udpSrcPort);
                st = GT_FAIL;
            }

            if ((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)==GT_TRUE)||
                    (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum)==GT_TRUE)||
                        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)==GT_TRUE)
                {
                    if(setConfigPtr->ipv6Cfg.ipHeaderProtocol == CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E)
                    {
                        if(setConfigPtr->ipv6Cfg.ipProtocol != getConfigPtr->ipv6Cfg.ipProtocol)
                        {
                            PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.ipProtocol than was set, expected = %d, received = %d\n",
                                             name, setConfigPtr->ipv4Cfg.ipProtocol,
                                             getConfigPtr->ipv4Cfg.ipProtocol);
                            st = GT_FAIL;
                        }
                    }
                }

            if(setConfigPtr->ipv6Cfg.flowLabelMode != getConfigPtr->ipv6Cfg.flowLabelMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.ipv6Cfg.flowLabelMode than was set, expected = %d, received = %d\n",
                                 name, setConfigPtr->ipv6Cfg.flowLabelMode,
                                 getConfigPtr->ipv6Cfg.flowLabelMode);
                st = GT_FAIL;
            }


            break;

        default:
            break;
        }

  return st;

}

