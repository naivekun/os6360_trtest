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
* @file cpssDxChDiagPacketGeneratorUT.c
*
* @brief Unit tests for cpssDxChDiagPacketGenerator
*
* @version   6
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagPacketGenerator.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>

/* Valid port num value used for testing */
#define DIAG_VALID_PORTNUM_CNS       0
/* Valid port num value used for testing with errata */
#define DIAG_VALID_PORTNUM_ERRATUM_CNS  1

/* Check valid port for errata */
#define PRV_DIAG_PACKET_GEN_ERRATA_PORT_VALUE_CHECK_MAC(dev, port, portTypeOptions) \
            if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E))  \
            {                                                                                                   \
                if(portTypeOptions == PRV_CPSS_GE_PORT_GE_ONLY_E)                                               \
                {                                                                                               \
                    if ((port % 4) == 0)                                                                        \
                    {                                                                                           \
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);                           \
                        continue;                                                                               \
                    }                                                                                           \
                    else                                                                                        \
                    {                                                                                           \
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);                                      \
                    }                                                                                           \
                }                                                                                               \
                else                                                                                            \
                {                                                                                               \
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);                               \
                    continue;                                                                                   \
                }                                                                                               \
            }                                                                                                   \
            else                                                                                                \
            {                                                                                                   \
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);                                              \
            }

/* Skip NA interface mode ports */
#define PRV_DIAG_PACKET_GEN_SKIP_NA_INTERFACE_MODE_PORTS_MAC(dev, port) \
        {                                                                                                       \
            CPSS_PORT_INTERFACE_MODE_ENT ifMode;                                                                \
            GT_STATUS rc;                                                                                       \
            rc = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);                                              \
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChPortInterfaceModeGet: %d, %d",                     \
                                         dev, port);                                                            \
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)                                                         \
            {                                                                                                   \
                continue;                                                                                       \
            }                                                                                                   \
            if(prvCpssDxChPortRemotePortCheck(dev, port))                                                       \
            {                                                                                                   \
                continue;                                                                                       \
            }                                                                                                   \
        }

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPacketGeneratorConnectSet
(
    IN GT_U8                                 devNum,
    IN GT_PHYSICAL_PORT_NUM                  portNum,
    IN GT_BOOL                               connect,
    IN CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChDiagPacketGeneratorConnectSet) */
GT_VOID cpssDxChDiagPacketGeneratorConnectSetUT(GT_VOID)
{
    /*
        ITERATE_DEVICES_PHY_PORT (Lion2, Bobcat2, Caelum, Bobcat3)
        1.1.1. Call with connect[GT_TRUE],
                         configPtr{
                            macDa[00:00:00:00:00:01],
                            macDaIncrementEnable[GT_FALSE],
                            macSa[00:00:00:00:00:88],
                            vlanTagEnable[GT_TRUE],
                            vpt[0, 3, 7],
                            cfi[0, 1, 1],
                            vid[1, 1024, 4095],
                            etherType[0, 0xAA55, 0xFFFF],
                            payloadType[CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E,
                                        CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E,
                                        CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E],
                            cyclicPatternArr[8][0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55],
                            packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E,
                                             CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E,
                                             CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
                            packetLength[20, 1000, 16383],
                            undersizeEnable[GT_FALSE, GT_TRUE, GT_FALSE]
                            transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E,
                                         CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E,
                                         CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                            packetCount[8191, 10, 1],
                            packetCountMultiplier[CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E],
                            ipg[0, 1024, 0xFFFF] }.
        Expected: GT_OK.
        1.1.2. Call cpssDxChDiagPacketGeneratorConnectGet.
        Expected: GT_OK and the same values as was set.
        1.1.3. Call with connect[GT_TRUE](is relevant),
            configPtr->vlanTagEnable[GT_TRUE](is relevant),
            out of range configPtr->vpt[8] and other valid params.
        Expected: NOT GT_OK.
        1.1.4. Call with connect[GT_TRUE](is relevant),
            configPtr->vlanTagEnable[GT_FALSE](not relevant),
            out of range configPtr->vpt[8] and other valid params.
        Expected: GT_OK.
        1.1.5. Call with connect[GT_FALSE](not relevant),
            configPtr->vlanTagEnable[GT_TRUE](is relevant),
            out of range configPtr->vpt[8] and other valid params.
        Expected: GT_OK.
        1.1.6. Call with connect[GT_TRUE](is relevant),
            configPtr->vlanTagEnable[GT_TRUE](is relevant),
            out of range configPtr->cfi[2] and other valid params.
        Expected: NOT GT_OK.
        1.1.7. Call with connect[GT_TRUE](is relevant),
            configPtr->vlanTagEnable[GT_FALSE](not relevant),
            out of range configPtr->cfi[2] and other valid params.
        Expected: GT_OK.
        1.1.8. Call with connect[GT_FALSE](not relevant),
            configPtr->vlanTagEnable[GT_TRUE](is relevant),
            out of range configPtr->cfi[2] and other valid params.
        Expected: GT_OK.
        1.1.9. Call with connect[GT_TRUE](is relevant),
            configPtr->vlanTagEnable[GT_TRUE](is relevant),
            out of range configPtr->vid[4096] and other valid params.
        Expected: NOT GT_OK.
        1.1.10. Call with connect[GT_TRUE](is relevant),
            configPtr->vlanTagEnable[GT_FALSE](not relevant),
            out of range configPtr->vid[4096] and other valid params.
        Expected: GT_OK.
        1.1.11. Call with connect[GT_FALSE](not relevant),
            configPtr->vlanTagEnable[GT_TRUE](is relevant),
            out of range configPtr->vid[4096] and other valid params.
        Expected: GT_OK.
        1.1.12. Call with connect[GT_TRUE](is relevant),
            out of range enum value configPtr->payloadType and other valid params.
        Expected: GT_BAD_PARAM.
        1.1.13. Call with connect[GT_FALSE](not relevant),
            out of range enum value configPtr->payloadType and other valid params.
        Expected: GT_OK.
        1.1.14. Call with connect[GT_TRUE](is relevant),
            out of range enum value configPtr->packetLengthType and other valid params.
        Expected: GT_BAD_PARAM.
        1.1.15. Call with connect[GT_FALSE](not relevant),
            out of range enum value configPtr->packetLengthType and other valid params.
        Expected: GT_OK.
        1.1.16. Call with connect[GT_TRUE](is relevant),
            configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E](is relevant),
            out of range configPtr->packetLength[16384] and other valid params.
        Expected: GT_BAD_PARAM.
        1.1.17. Call with connect[GT_TRUE](is relevant),
            configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E](not relevant),
            out of range configPtr->packetLength[16384] and other valid params.
        Expected: GT_OK.
        1.1.18. Call with connect[GT_FALSE](not relevant),
            configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E](is relevant),
            out of range configPtr->packetLength[16384] and other valid params.
        Expected: GT_OK.
        1.1.19. Call with connect[GT_TRUE](is relevant),
            out of range enum value configPtr->transmitMode and other valid params.
        Expected: GT_BAD_PARAM.
        1.1.20. Call with connect[GT_FALSE](not relevant),
            out of range enum value configPtr->transmitMode and other valid params.
        Expected: GT_OK.
        1.1.21. Call with connect[GT_TRUE](is relevant),
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
            out of range configPtr->packetCount[8192] and other valid params.
        Expected: GT_BAD_PARAM.
        1.1.22. Call with connect[GT_TRUE](is relevant),
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E](not relevant),
            out of range configPtr->packetCount[8192] and other valid params.
        Expected: GT_OK.
        1.1.23. Call with connect[GT_FALSE](not relevant),
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
            out of range configPtr->packetCount[8192] and other valid params.
        Expected: GT_OK.
        1.1.24. Call with connect[GT_TRUE](is relevant),
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
            out of range enum values configPtr->packetCountMultiplier and other valid params.
        Expected: GT_BAD_PARAM.
        1.1.25. Call with connect[GT_TRUE](is relevant),
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E](not relevant),
            out of range enum values configPtr->packetCountMultiplier and other valid params.
        Expected: GT_OK.
        1.1.26. Call with connect[GT_FALSE](not relevant),
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
            out of range enum values configPtr->packetCountMultiplier and other valid params.
        Expected: GT_OK.
        1.1.27. Call with connect[GT_TRUE](is relevant),
            out of range configPtr[NULL] and other valid values.
        Expected: GT_BAD_PTR.
        1.1.28. Call with connect[GT_FALSE](not relevant),
            out of range configPtr[NULL] and other valid values.
        Expected: GT_OK.
    */

    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_PHYSICAL_PORT_NUM                    port        = 0;
    GT_BOOL                                 connect     = GT_FALSE;
    GT_BOOL                                 connectRet  = GT_FALSE;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC    config      = {{{0}}};
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC    configRet   = {{{0}}};
    GT_BOOL                                 isEqual     = GT_FALSE;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT          portTypeOptions;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            PRV_DIAG_PACKET_GEN_SKIP_NA_INTERFACE_MODE_PORTS_MAC(dev, port);

/*
            st = cpssDxChPortInterfaceModeSet(dev, port, CPSS_PORT_INTERFACE_MODE_SGMII_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChPortSpeedSet(dev, port, CPSS_PORT_SPEED_100_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
*/
            /*
                1.1.1. Call with connect[GT_TRUE],
                                 configPtr{
                                    macDa[00:00:00:00:00:01],
                                    macDaIncrementEnable[GT_FALSE],
                                    macSa[00:00:00:00:00:88],
                                    vlanTagEnable[GT_TRUE],
                                    vpt[0, 3, 7],
                                    cfi[0, 1, 1],
                                    vid[1, 1024, 4095],
                                    etherType[0, 0xAA55, 0xFFFF],
                                    payloadType[CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E,
                                                CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E,
                                                CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E],
                                    cyclicPatternArr[8][0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55],
                                    packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E,
                                                     CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E,
                                                     CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
                                    packetLength[20, 1000, 16383],
                                    undersizeEnable[GT_FALSE, GT_TRUE, GT_FALSE]
                                    transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E,
                                                 CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E,
                                                 CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                                    packetCount[8191, 10, 1],
                                    packetCountMultiplier[CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E],
                                    ipg[0, 1024, 0xFFFF] }.
                Expected: GT_OK.
            */

            /* iterate with vid[1] */
            connect = GT_TRUE;
            config.macDa.arEther[0] = 0x01;
            config.macDaIncrementEnable = GT_FALSE;
            config.macSa.arEther[0] = 0x88;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 0;
            config.cfi = 0;
            config.vid = 1;
            config.etherType = 0;
            config.payloadType = CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E;
            config.cyclicPatternArr[0] = 0x55;
            config.cyclicPatternArr[1] = 0xAA;
            config.cyclicPatternArr[2] = 0xFF;
            config.cyclicPatternArr[3] = 0xA5;
            config.cyclicPatternArr[4] = 0x00;
            config.cyclicPatternArr[5] = 0xFF;
            config.cyclicPatternArr[6] = 0xA5;
            config.cyclicPatternArr[7] = 0x00;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 20;
            config.undersizeEnable = GT_FALSE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_CONTINUES_E;
            config.ipg = 0;

            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            PRV_DIAG_PACKET_GEN_ERRATA_PORT_VALUE_CHECK_MAC(dev,port,portTypeOptions);

            /*
                1.1.2. Call cpssDxChDiagPacketGeneratorConnectGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChDiagPacketGeneratorConnectGet(dev, port, &connectRet, &configRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(connect, connectRet,
                                             "got another connectRet: %d", connectRet);

                /* verify macDa */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.macDa.arEther,
                                             (GT_VOID*) &configRet.macDa.arEther,
                                             sizeof(config.macDa.arEther))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                             "get another macDa than was set");

                UTF_VERIFY_EQUAL1_STRING_MAC(config.macDaIncrementEnable,
                                             configRet.macDaIncrementEnable,
                                             "got another configRet.macDaIncrementEnable: %d",
                                             configRet.macDaIncrementEnable);

                /* verify macSa */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.macSa.arEther,
                                             (GT_VOID*) &configRet.macSa.arEther,
                                             sizeof(config.macSa.arEther))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                             "get another macSa than was set");

                UTF_VERIFY_EQUAL1_STRING_MAC(config.vlanTagEnable,
                                             configRet.vlanTagEnable,
                                             "got another configRet.vlanTagEnable: %d",
                                             configRet.vlanTagEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.vpt,
                                             configRet.vpt,
                                             "got another configRet.vpt: %d",
                                             configRet.vpt);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cfi,
                                             configRet.cfi,
                                             "got another configRet.cfi: %d",
                                             configRet.cfi);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.vid,
                                             configRet.vid,
                                             "got another configRet.vid: %d",
                                             configRet.vid);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.etherType,
                                             configRet.etherType,
                                             "got another configRet.etherType: %d",
                                             configRet.etherType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.payloadType,
                                             configRet.payloadType,
                                             "got another configRet.payloadType: %d",
                                             configRet.payloadType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[0],
                                             configRet.cyclicPatternArr[0],
                                             "got another configRet.cyclicPatternArr[0]: %d",
                                             configRet.cyclicPatternArr[0]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[1],
                                             configRet.cyclicPatternArr[1],
                                             "got another configRet.cyclicPatternArr[1]: %d",
                                             configRet.cyclicPatternArr[1]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[2],
                                             configRet.cyclicPatternArr[2],
                                             "got another configRet.cyclicPatternArr[2]: %d",
                                             configRet.cyclicPatternArr[2]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[3],
                                             configRet.cyclicPatternArr[3],
                                             "got another configRet.cyclicPatternArr[3]: %d",
                                             configRet.cyclicPatternArr[3]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[4],
                                             configRet.cyclicPatternArr[4],
                                             "got another configRet.cyclicPatternArr[4]: %d",
                                             configRet.cyclicPatternArr[4]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[5],
                                             configRet.cyclicPatternArr[5],
                                             "got another configRet.cyclicPatternArr[5]: %d",
                                             configRet.cyclicPatternArr[5]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[6],
                                             configRet.cyclicPatternArr[6],
                                             "got another configRet.cyclicPatternArr[6]: %d",
                                             configRet.cyclicPatternArr[6]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cyclicPatternArr[7],
                                             configRet.cyclicPatternArr[7],
                                             "got another configRet.cyclicPatternArr[7]: %d",
                                             configRet.cyclicPatternArr[7]);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.packetLengthType,
                                             configRet.packetLengthType,
                                             "got another configRet.packetLengthType: %d",
                                             configRet.packetLengthType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.packetLength,
                                             configRet.packetLength,
                                             "got another configRet.packetLength: %d",
                                             configRet.packetLength);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.transmitMode,
                                             configRet.transmitMode,
                                             "got another configRet.transmitMode: %d",
                                             configRet.transmitMode);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.ipg,
                                             configRet.ipg,
                                             "got another configRet.ipg: %d",
                                             configRet.ipg);
            }

            /* iterate with vid[1024] */
            config.vpt = 3;
            config.cfi = 1;
            config.vid = 1024;
            config.etherType = 0xAA55;
            config.payloadType = CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;
            config.packetLength = 1000;
            config.undersizeEnable = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 10;
            config.packetCountMultiplier = CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_512_E;
            config.ipg = 1024;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChDiagPacketGeneratorConnectGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChDiagPacketGeneratorConnectGet(dev, port, &connectRet, &configRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(connect, connectRet,
                                             "got another connectRet: %d", connectRet);

                /* verify macDa */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.macDa.arEther,
                                             (GT_VOID*) &configRet.macDa.arEther,
                                             sizeof(config.macDa.arEther))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                             "get another macDa than was set");

                UTF_VERIFY_EQUAL1_STRING_MAC(config.macDaIncrementEnable,
                                             configRet.macDaIncrementEnable,
                                             "got another configRet.macDaIncrementEnable: %d",
                                             configRet.macDaIncrementEnable);

                /* verify macSa */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.macSa.arEther,
                                             (GT_VOID*) &configRet.macSa.arEther,
                                             sizeof(config.macSa.arEther))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                             "get another macSa than was set");

                UTF_VERIFY_EQUAL1_STRING_MAC(config.vlanTagEnable,
                                             configRet.vlanTagEnable,
                                             "got another configRet.vlanTagEnable: %d",
                                             configRet.vlanTagEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.vpt,
                                             configRet.vpt,
                                             "got another configRet.vpt: %d",
                                             configRet.vpt);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cfi,
                                             configRet.cfi,
                                             "got another configRet.cfi: %d",
                                             configRet.cfi);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.vid,
                                             configRet.vid,
                                             "got another configRet.vid: %d",
                                             configRet.vid);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.etherType,
                                             configRet.etherType,
                                             "got another configRet.etherType: %d",
                                             configRet.etherType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.payloadType,
                                             configRet.payloadType,
                                             "got another configRet.payloadType: %d",
                                             configRet.payloadType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.packetLengthType,
                                             configRet.packetLengthType,
                                             "got another configRet.packetLengthType: %d",
                                             configRet.packetLengthType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.undersizeEnable,
                                             configRet.undersizeEnable,
                                             "got another configRet.undersizeEnable: %d",
                                             configRet.undersizeEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.transmitMode,
                                             configRet.transmitMode,
                                             "got another configRet.transmitMode: %d",
                                             configRet.transmitMode);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.ipg,
                                             configRet.ipg,
                                             "got another configRet.ipg: %d",
                                             configRet.ipg);
            }

            /* iterate with vid[4095] */
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 7;
            config.cfi = 1;
            config.vid = 4095;
            config.etherType = 0xFFFF;
            config.payloadType = CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.undersizeEnable = GT_FALSE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 1;
            config.ipg = 0xFFFF;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssDxChDiagPacketGeneratorConnectGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChDiagPacketGeneratorConnectGet(dev, port, &connectRet, &configRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(connect, connectRet,
                                             "got another connectRet: %d", connectRet);

                /* verify macDa */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.macDa.arEther,
                                             (GT_VOID*) &configRet.macDa.arEther,
                                             sizeof(config.macDa.arEther))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                             "get another macDa than was set");

                UTF_VERIFY_EQUAL1_STRING_MAC(config.macDaIncrementEnable,
                                             configRet.macDaIncrementEnable,
                                             "got another configRet.macDaIncrementEnable: %d",
                                             configRet.macDaIncrementEnable);

                /* verify macSa */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.macSa.arEther,
                                             (GT_VOID*) &configRet.macSa.arEther,
                                             sizeof(config.macSa.arEther))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                             "get another macSa than was set");

                UTF_VERIFY_EQUAL1_STRING_MAC(config.vlanTagEnable,
                                             configRet.vlanTagEnable,
                                             "got another configRet.vlanTagEnable: %d",
                                             configRet.vlanTagEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.vpt,
                                             configRet.vpt,
                                             "got another configRet.vpt: %d",
                                             configRet.vpt);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.cfi,
                                             configRet.cfi,
                                             "got another configRet.cfi: %d",
                                             configRet.cfi);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.vid,
                                             configRet.vid,
                                             "got another configRet.vid: %d",
                                             configRet.vid);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.etherType,
                                             configRet.etherType,
                                             "got another configRet.etherType: %d",
                                             configRet.etherType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.payloadType,
                                             configRet.payloadType,
                                             "got another configRet.payloadType: %d",
                                             configRet.payloadType);
                /* verify cyclicPatternArr */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &config.cyclicPatternArr,
                                             (GT_VOID*) &configRet.cyclicPatternArr,
                                             sizeof(config.cyclicPatternArr))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                             "get another cyclicPatternArr than was set");

                UTF_VERIFY_EQUAL1_STRING_MAC(config.packetLengthType,
                                             configRet.packetLengthType,
                                             "got another configRet.packetLengthType: %d",
                                             configRet.packetLengthType);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.undersizeEnable,
                                             configRet.undersizeEnable,
                                             "got another configRet.undersizeEnable: %d",
                                             configRet.undersizeEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.transmitMode,
                                             configRet.transmitMode,
                                             "got another configRet.transmitMode: %d",
                                             configRet.transmitMode);
                UTF_VERIFY_EQUAL1_STRING_MAC(config.ipg,
                                             configRet.ipg,
                                             "got another configRet.ipg: %d",
                                             configRet.ipg);
            }

            /*
                1.1.3. Call with connect[GT_TRUE](is relevant),
                    configPtr->vlanTagEnable[GT_TRUE](is relevant),
                    out of range configPtr->vpt[8] and other valid params.
                Expected: NOT GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 8;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 7;

            /*
                1.1.4. Call with connect[GT_TRUE](is relevant),
                    configPtr->vlanTagEnable[GT_FALSE](not relevant),
                    out of range configPtr->vpt[8] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_FALSE;
            config.vpt = 8;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 7;

            /*
                1.1.5. Call with connect[GT_FALSE](not relevant),
                    configPtr->vlanTagEnable[GT_TRUE](is relevant),
                    out of range configPtr->vpt[8] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 8;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 7;

            /*
                1.1.6. Call with connect[GT_TRUE](is relevant),
                    configPtr->vlanTagEnable[GT_TRUE](is relevant),
                    out of range configPtr->cfi[2] and other valid params.
                Expected: GT_OUT_OF_RANGE.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 2;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 1;

            /*
                1.1.7. Call with connect[GT_TRUE](is relevant),
                    configPtr->vlanTagEnable[GT_FALSE](not relevant),
                    out of range configPtr->cfi[2] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_FALSE;
            config.cfi = 2;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 1;

            /*
                1.1.8. Call with connect[GT_FALSE](not relevant),
                    configPtr->vlanTagEnable[GT_TRUE](is relevant),
                    out of range configPtr->cfi[2] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 2;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 1;

            /*
                1.1.9. Call with connect[GT_TRUE](is relevant),
                    configPtr->vlanTagEnable[GT_TRUE](is relevant),
                    out of range configPtr->vid[4096] and other valid params.
                Expected: NOT GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4096;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4095;

            /*
                1.1.10. Call with connect[GT_TRUE](is relevant),
                    configPtr->vlanTagEnable[GT_FALSE](not relevant),
                    out of range configPtr->vid[4096] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_FALSE;
            config.vid = 4096;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4095;

            /*
                1.1.11. Call with connect[GT_FALSE](not relevant),
                    configPtr->vlanTagEnable[GT_TRUE](is relevant),
                    out of range configPtr->vid[4096] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4096;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4095;

            /*
                1.1.12. Call with connect[GT_TRUE](is relevant),
                    out of range enum value configPtr->payloadType and other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPacketGeneratorConnectSet
                                (dev, port, connect, &config),
                                config.payloadType);

            /*
                1.1.13. Call with connect[GT_FALSE](not relevant),
                    out of range enum value configPtr->payloadType and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.payloadType = -1;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.payloadType = CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E;

            /*
                1.1.14. Call with connect[GT_TRUE](is relevant),
                    out of range enum value configPtr->packetLengthType and other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPacketGeneratorConnectSet
                                (dev, port, connect, &config),
                                config.packetLengthType);

            /*
                1.1.15. Call with connect[GT_FALSE](not relevant),
                    out of range enum value configPtr->packetLengthType and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.packetLengthType = -1;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;

            /*
                1.1.16. Call with connect[GT_TRUE](is relevant),
                    configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E](is relevant),
                    out of range configPtr->packetLength[16384] and other valid params.
                Expected: NOT GT_OK.
            */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16384;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16383;

            /*
                1.1.17. Call with connect[GT_TRUE](is relevant),
                    configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E](not relevant),
                    out of range configPtr->packetLength[16384] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;
            config.packetLength = 16384;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) || IS_XCAT3_DEV_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16383;

            /*
                1.1.18. Call with connect[GT_FALSE](not relevant),
                    configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E](is relevant),
                    out of range configPtr->packetLength[16384] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16384;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16383;

            /*
                1.1.19. Call with connect[GT_TRUE](is relevant),
                    out of range enum value configPtr->transmitMode and other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPacketGeneratorConnectSet
                                (dev, port, connect, &config),
                                config.transmitMode);


            /*
                1.1.20. Call with connect[GT_FALSE](not relevant),
                    out of range enum value configPtr->transmitMode and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.transmitMode = -1;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;

            /*
                1.1.21. Call with connect[GT_TRUE](is relevant),
                    configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
                    out of range configPtr->packetCount[8192] and other valid params.
                Expected: NOT GT_OK.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 8192;
            config.packetLength = 20;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 1;
            config.packetLength = 16383;

            /*
                1.1.22. Call with connect[GT_TRUE](is relevant),
                    configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E](not relevant),
                    out of range configPtr->packetCount[8192] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_CONTINUES_E;
            config.packetCount = 8192;
            config.packetLength = 20;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 1;
            config.packetLength = 16383;

            /*
                1.1.23. Call with connect[GT_FALSE](not relevant),
                    configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
                    out of range configPtr->packetCount[8192] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 8192;
            config.packetLength = 20;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 1;
            config.packetLength = 16383;

            /*
                1.1.24. Call with connect[GT_TRUE](is relevant),
                    configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
                    out of range enum values configPtr->packetCountMultiplier and other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPacketGeneratorConnectSet
                                (dev, port, connect, &config),
                                config.packetCountMultiplier);

            /*
                1.1.25. Call with connect[GT_TRUE](is relevant),
                    configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E](not relevant),
                    out of range enum values configPtr->packetCountMultiplier and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_CONTINUES_E;
            config.packetCountMultiplier = -1;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCountMultiplier = CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E;

            /*
                1.1.26. Call with connect[GT_FALSE](not relevant),
                    configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E](is relevant),
                    out of range enum values configPtr->packetCountMultiplier and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCountMultiplier = -1;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCountMultiplier = CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E;

            /*
                1.1.27. Call with connect[GT_TRUE](is relevant),
                    out of range configPtr[NULL] and other valid values.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /*
                1.1.28. Call with connect[GT_FALSE](not relevant),
                    out of range configPtr[NULL] and other valid values.
                Expected: GT_OK.
            */
            connect = GT_FALSE;

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
        }

         st = prvUtfNextMacPortReset(&port, dev);
         UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

         /*
             1.2. For all active devices go over all non available physical ports.
         */
         while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
         {
             /* 1.2.1. Call function for each non-active port */
             st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
             UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
         }

         /*
             1.3. For active device check that function returns GT_BAD_PARAM
             for out of bound value for port number.
         */
         port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
         st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
         UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

         /*
             1.4. For active device check that function returns GT_BAD_PARAM
                 for CPU port number.
         */
         port = CPSS_CPU_PORT_NUM_CNS;
         st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
         UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
     }

     /* restore value */
     port   = DIAG_VALID_PORTNUM_CNS;

     /* 2. For not-active devices and devices from non-applicable family */
     /* check that function returns GT_BAD_PARAM.                        */

     /* prepare device iterator */
     PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                            UTF_CH2_E | UTF_CH3_E |
                                            UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E );

     /* go over all non active devices */
     while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
     {
         if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E))
         {
             port = DIAG_VALID_PORTNUM_ERRATUM_CNS;
         }
         st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
     }

     /* 3. Call function with out of bound value for device id.*/
     dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

     st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, connect, &config);
     UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPacketGeneratorConnectGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT GT_BOOL                              *connectPtr,
    OUT CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChDiagPacketGeneratorConnectGet) */
GT_VOID cpssDxChDiagPacketGeneratorConnectGetUT(GT_VOID)
{
    /*
        ITERATE_DEVICES_PHY_PORT (Lion2, Bobcat2, Caelum, Bobcat3)
        1.1.1. Call with non-null connectPtr, non-null configPtr.
        Expected: GT_OK.
        1.1.2. Call with out of range connectPtr[NULL], non-null configPtr.
        Expected: GT_BAD_PTR.
        1.1.3. Call with non-null connectPtr, out of range configPtr[NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_PHYSICAL_PORT_NUM                    port        = 0;
    GT_BOOL                                 connect     = GT_FALSE;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC    config      = {{{0}}};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            PRV_DIAG_PACKET_GEN_SKIP_NA_INTERFACE_MODE_PORTS_MAC(dev, port);
            /*
                1.1.1. Call with non-null connectPtr, non-null configPtr.
                Expected: GT_OK.
            */
            st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                                       &connect, &config);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /*
                1.1.2. Call with out of range connectPtr[NULL], non-null configPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                                       NULL, &config);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


            /*
                1.1.3. Call with non-null connectPtr, out of range configPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                                       &connect, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
       }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                                       &connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                                   &connect, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                                   &connect, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* restore value */
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E))
        {
            port = DIAG_VALID_PORTNUM_ERRATUM_CNS;
        }
        st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                                   &connect, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPacketGeneratorConnectGet(dev, port,
                                               &connect, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPacketGeneratorTransmitEnable
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChDiagPacketGeneratorTransmitEnable) */
GT_VOID cpssDxChDiagPacketGeneratorTransmitEnableUT(GT_VOID)
{
    /*
        ITERATE_DEVICES_PHY_PORT (Lion2, Bobcat2, Caelum, Bobcat3)
        1.1.1. Connect port to packet generator.
        Expected: GT_OK.
        1.1.2. Call with enable[GT_FALSE, GT_TRUE].
        Expected: GT_OK.
        1.1.3. Disconnect port from packet generator.
        Expected: GT_OK.
        1.1.2. Call with enable[GT_FALSE].
        Expected: GT_BAD_STATE.
    */

    GT_STATUS               st          = GT_OK;
    GT_U8                   dev         = 0;
    GT_PHYSICAL_PORT_NUM    port        = 0;
    GT_BOOL                 enable      = GT_FALSE;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC    config      = {{{0}}};
    config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            PRV_DIAG_PACKET_GEN_SKIP_NA_INTERFACE_MODE_PORTS_MAC(dev, port);

            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /*
                1.1.1. Connect port to packet generator.
                Expected: GT_OK.
            */

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, GT_TRUE, &config);
            PRV_DIAG_PACKET_GEN_ERRATA_PORT_VALUE_CHECK_MAC(dev,port,portTypeOptions);

            /*
                1.1.2. Call with enable[GT_FALSE, GT_TRUE].
                Expected: GT_OK.
            */

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;
            st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with enable[GT_TRUE] */
            enable = GT_TRUE;
            st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.3. Connect port to packet generator.
                Expected: GT_OK.
            */

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, GT_FALSE, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /*
                1.1.4. Call with enable[GT_FALSE, GT_TRUE].
                Expected: GT_BAD_STATE.
            */

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;
            st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
        }

        /* restore value */
        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* restore value */
    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E))
        {
            port = DIAG_VALID_PORTNUM_ERRATUM_CNS;
        }
        st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChDiagPacketGeneratorBurstTransmitStatusGet) */
GT_VOID cpssDxChDiagPacketGeneratorBurstTransmitStatusGetUT(GT_VOID)
{
    /*
        ITERATE_DEVICES_PHY_PORT (Lion2, Bobcat2, Caelum, Bobcat3)
        1.1.1. Connect port to packet generator.
        Expected: GT_OK.
        1.1.2. Call with non-null burstTransmitDonePtr.
        Expected: GT_OK.
        1.1.3. Call with out of range burstTransmitDonePtr[NULL].
        Expected: GT_BAD_PTR.
        1.1.4. Disconnect port from packet generator.
        Expected: GT_OK.
        1.1.5. Call with non-null burstTransmitDonePtr.
        Expected: GT_BAD_STATE.
    */

    GT_STATUS               st                  = GT_OK;
    GT_U8                   dev                 = 0;
    GT_PHYSICAL_PORT_NUM    port                = 0;
    GT_BOOL                 burstTransmitDone   = GT_FALSE;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC    config      = {{{0}}};
    config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            PRV_DIAG_PACKET_GEN_SKIP_NA_INTERFACE_MODE_PORTS_MAC(dev, port);

            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Connect port to packet generator.
                Expected: GT_OK.
            */

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, GT_TRUE, &config);
            PRV_DIAG_PACKET_GEN_ERRATA_PORT_VALUE_CHECK_MAC(dev,port,portTypeOptions);

            /*
                1.1.2. Call with non-null burstTransmitDonePtr.
                Expected: GT_OK.
            */

            st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.3. Call with out of range burstTransmitDonePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.4. Disconnect port from packet generator.
                Expected: GT_OK.
            */

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, GT_FALSE, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.5. Call with non-null burstTransmitDonePtr.
                Expected: GT_BAD_STATE.
            */

            st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_FALCON_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        if (PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E))
        {
            port = DIAG_VALID_PORTNUM_ERRATUM_CNS;
        }
        st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/**
* @internal cpssDxChDiagPacketGeneratorCheckDataTransmissionUT function
* @endinternal
*
* @brief   Check that sent packet using Packet Generator come to right port.
*/
GT_VOID cpssDxChDiagPacketGeneratorCheckDataTransmissionUT(GT_VOID)
{
    /*
        ITERATE_DEVICES
        1  Prepare device to test: save link states, force up all applicable
           ports and reset counters
        2  ITERATE PORTS
        2.1  Check link state
        2.2  Configure packet generator and transmit packet
        2.3  Wait for packet
        2.4  Disconnect packet generator from port and disable transmission
        2.5  Try to find sent packet and print information about that
        3  Restore port link states
    */
    GT_U8                                 dev = 0;
#ifndef ASIC_SIMULATION
    GT_STATUS                             rc = GT_OK;
    GT_STATUS                             st = GT_OK;
    GT_PHYSICAL_PORT_NUM                  port = 0;
    GT_PHYSICAL_PORT_NUM                  i = 0;               /* used for go over all applicable ports */
    GT_U32                                j;
    CPSS_PORT_MAC_COUNTER_SET_STC         portCntrs;           /* port counters */
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT        portTypeOptions;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC  config = {{{0}}};
    GT_BOOL                               portState;
    GT_U8                                 devPortStateMap[32]; /* save port states (up to 256 ports)   */
                                                               /* before test start for current device */
    CPSS_INTERFACE_INFO_STC               portInterface;       /* used for link up */
#endif /* ASIC_SIMULATION */


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E |
                                           UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* configure packet that we will try to send */
    config.macDa.arEther[0] = 0x01;
    config.macDaIncrementEnable = GT_FALSE;
    config.macSa.arEther[0] = 0x88;
    config.vlanTagEnable = GT_FALSE;
    config.vpt = 0;
    config.cfi = 0;
    config.vid = 1;
    config.etherType = 0x0800;
    config.payloadType = CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E;
    config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;
    config.packetLength = 1000;
    config.packetCount = 1;
    config.packetCountMultiplier = CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E;
    config.undersizeEnable = GT_TRUE;
    config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
    config.ipg = 1024;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*
        Go over all active devices
    */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        PRV_UTF_LOG1_MAC("Go over all applicable ports for device %i\n", dev);

        /*
            1  Prepare device to test: save link states, force up all applicable ports and reset counters
        */
        /* prepare to go over all applicable ports */
        st = prvUtfNextMacPortReset(&i, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* configure portInterface for tgfTrafficGenerator... functions */
        portInterface.type             = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum = dev;

        /* save port link states as unchanged */
        for (j = 0; j < 32; j++)
        {
            devPortStateMap[j] = 0;
        }

        /* go over all ports */
        while (GT_OK == prvUtfNextMacPortGet(&i, GT_TRUE))
        {
            PRV_DIAG_PACKET_GEN_SKIP_NA_INTERFACE_MODE_PORTS_MAC(dev, i);

            portInterface.devPort.portNum = i;

            /* force up link if needed */
            rc = cpssDxChPortLinkStatusGet(dev, i, &portState);
            if ((rc == GT_OK) && (portState == GT_FALSE))
            {
                tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_TRUE);

                /* save port link state as changed */
                if (i <= 255) {
                    devPortStateMap[i >> 3] |= 0x01 << (i & 0x07);
                }
            }

            /* reset counters */
            tgfTrafficGeneratorPortCountersEthReset(&portInterface);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* prepare to go over all applicable ports for current device */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            2  Go over all applicable ports
        */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            PRV_DIAG_PACKET_GEN_SKIP_NA_INTERFACE_MODE_PORTS_MAC(dev, port);

            PRV_UTF_LOG2_MAC("Device %i port %2i:  ", dev, port);

            /*
                2.1  Check link state
            */
            rc = cpssDxChPortLinkStatusGet(dev, port, &portState);
            if (portState == GT_FALSE)
            {
                PRV_UTF_LOG0_MAC("[skip]\n   INFO: Could not force link up\n");
                continue;
            }


            /*
                2.2  Configure packet generator and transmit packet
            */
            st = prvUtfPortTypeOptionsGet(dev, port, &portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, GT_TRUE, &config);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            PRV_DIAG_PACKET_GEN_ERRATA_PORT_VALUE_CHECK_MAC(dev, port, portTypeOptions);

            st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /*
                2.3  Wait for packet
            */
            cpssOsTimerWkAfter(10);


            /*
                2.4  Disconnect packet generator from port and disable transmission
            */
            st = cpssDxChDiagPacketGeneratorConnectSet(dev, port, GT_FALSE, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssDxChDiagPacketGeneratorTransmitEnable(dev, port, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);


            /*
                2.5  Try to find sent packet and print information about that
            */
            /* read counters; clear on read registers */
            rc = prvTgfReadPortCountersEth(dev, port, GT_FALSE, &portCntrs);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            /* if packet did not come to corrent port - try to find it elsewhere on current device */
            if (portCntrs.goodPktsSent.l[0] == 0)
            {
                /* prepare to go over all applicable ports */
                st = prvUtfNextMacPortReset(&i, dev);
                if (st == GT_OK)
                {
                    /* go over all ports on current device */
                    while (((st != GT_OK) || (portCntrs.goodPktsSent.l[0] == 0))
                           && (GT_OK == prvUtfNextMacPortGet(&i, GT_TRUE)))
                    {
                        /* read counters for "i" port on "dev" device; clear on read registers */
                        st = prvTgfReadPortCountersEth(dev, i, GT_FALSE, &portCntrs);
                    }

                    if ((st == GT_OK) && (portCntrs.goodPktsSent.l[0] != 0))
                    {
                        PRV_UTF_LOG2_MAC("[fail]\n   ERROR: Packet sent to port %i instead of port %i\n", i, port);
                    }
                    else
                    {
                        PRV_UTF_LOG0_MAC("[fail]\n   ERROR: Packet has not been sent to any port\n");
                    }
                }
                else
                {
                    PRV_UTF_LOG0_MAC("[fail]\n   ERROR: Could not reset port\n");
                }

                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_FAIL);
            }
            else
            {
                PRV_UTF_LOG0_MAC("[ ok ]\n");
            }
        }

        /*
            3  Restore port link states
        */
        for (j = 0; j < 256; j++)
        {
            if (devPortStateMap[j >> 3] & (0x01 << (j & 0x07)))
            {
                portInterface.devPort.portNum = j;
                tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
            }
        }
    }
#endif /* ASIC_SIMULATION */
}


/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChDiagPacketGenerator suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChDiagPacketGenerator)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPacketGeneratorConnectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPacketGeneratorConnectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPacketGeneratorTransmitEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPacketGeneratorBurstTransmitStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPacketGeneratorCheckDataTransmission)
UTF_SUIT_END_TESTS_MAC(cpssDxChDiagPacketGenerator)


