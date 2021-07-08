/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxDiagPacketGeneratorUT.c
*
* @brief Unit tests for cpssPxDiagPacketGenerator
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/diag/cpssPxDiagPacketGenerator.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>
#include <utfTraffic/common/cpssPxTgfCommon.h>


/* Valid port num value used for testing */
#define DIAG_VALID_PORTNUM_CNS       0


/**
* @internal prvCpssPxDiagPacketGeneratorCompareConfig function
* @endinternal
*
* @brief   Compare 2 packet generator configurations: from
*         cpssPxDiagPacketGeneratorConnectSet() and from
*         cpssPxDiagPacketGeneratorConnectGet().
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] set_isConnected          - connect value from setter
* @param[in] get_isConnected          - connect value from getter
* @param[in] set_config               - packet generator config from setter
* @param[in] get_config               - packet generator config from getter
*
* @param[out] compareStatus            - GT_OK, if configuration equal. Otherwise - GT_FAIL
*                                      COMMENTS:
*                                      None.
*/
static GT_VOID prvCpssPxDiagPacketGeneratorCompareConfig
(
    IN  GT_BOOL                             set_isConnected,
    IN  GT_BOOL                             get_isConnected,
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *set_config,
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *get_config,
    OUT GT_STATUS                           *compareStatus
)
{
    GT_U32   i;
    GT_BOOL  isEqual;

    *compareStatus = GT_FAIL;

    /* verify connect state */
    UTF_VERIFY_EQUAL1_STRING_MAC(set_isConnected, get_isConnected,
                                 "got another connect: %d", get_isConnected);

    if (set_isConnected == GT_TRUE)
    {
        /* MAC DA */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(set_config->macDa.arEther),
                                     (GT_VOID*) &(get_config->macDa.arEther),
                                     sizeof(set_config->macDa.arEther))) ? GT_TRUE : GT_FALSE;

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                     "got another macDa than was set");

        /* MAC DA increment */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->macDaIncrementEnable,
                                     get_config->macDaIncrementEnable,
                                     "got another macDaIncrementEnable: %d",
                                     get_config->macDaIncrementEnable);

        /* MAC DA increment limit */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->macDaIncrementLimit,
                                     get_config->macDaIncrementLimit,
                                     "got another macDaIncrementEnable: %d",
                                     get_config->macDaIncrementLimit);

        /* MAC SA */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &(set_config->macSa.arEther),
                                     (GT_VOID*) &(get_config->macSa.arEther),
                                     sizeof(set_config->macSa.arEther))) ? GT_TRUE : GT_FALSE;

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqual,
                                     "got another macSa than was set");

        /* Ethernet type */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->etherType,
                                     get_config->etherType,
                                     "got another etherType: %d",
                                     get_config->etherType);

        /* VLAN tag */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->vlanTagEnable,
                                     get_config->vlanTagEnable,
                                     "got another vlanTagEnable: %d",
                                     get_config->vlanTagEnable);

        if (set_config->vlanTagEnable)
        {
            /* vpt */
            UTF_VERIFY_EQUAL1_STRING_MAC(set_config->vpt,
                                         get_config->vpt,
                                         "got another vpt: %d",
                                         get_config->vpt);
            /* cfi */
            UTF_VERIFY_EQUAL1_STRING_MAC(set_config->cfi,
                                         get_config->cfi,
                                         "got another cfi: %d",
                                         get_config->cfi);
            /* vid */
            UTF_VERIFY_EQUAL1_STRING_MAC(set_config->vid,
                                         get_config->vid,
                                         "got another vid: %d",
                                         get_config->vid);
        }

        /* Packet length */
        UTF_VERIFY_EQUAL1_STRING_MAC(get_config->packetLength,
                                     set_config->packetLength,
                                     "got another packetLength: %d",
                                     set_config->packetLength);

        /* Packet payload type */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->payloadType,
                                     get_config->payloadType,
                                     "got another payloadType: %d",
                                     get_config->payloadType);

        /* Packet length type */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->packetLengthType,
                                     get_config->packetLengthType,
                                     "got another packetLengthType: %d",
                                     get_config->packetLengthType);

        /* Undersized packets transmission */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->undersizeEnable,
                                     get_config->undersizeEnable,
                                     "got another undersizeEnable: %d",
                                     get_config->undersizeEnable);

        /* Packet transmittion mode */
        UTF_VERIFY_EQUAL1_STRING_MAC(get_config->transmitMode,
                                     set_config->transmitMode,
                                     "got another transmitMode: %d",
                                     set_config->transmitMode);

        if (set_config->transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
        {
            /* Packet count */
            UTF_VERIFY_EQUAL1_STRING_MAC(set_config->packetCount,
                                         get_config->packetCount,
                                         "got another packetCount: %d",
                                         get_config->packetCount);

            /* Packet multiplier */
            UTF_VERIFY_EQUAL1_STRING_MAC(set_config->packetCountMultiplier,
                                         get_config->packetCountMultiplier,
                                         "got another packetCountMultiplier: %d",
                                         get_config->packetCountMultiplier);
        }

        /* Inter frame gap */
        UTF_VERIFY_EQUAL1_STRING_MAC(set_config->ifg,
                                     get_config->ifg,
                                     "got another ifg: %d",
                                     get_config->ifg);

        /* Interface size */
        if (set_config->interfaceSize != CPSS_DIAG_PG_IF_SIZE_DEFAULT_E)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(set_config->interfaceSize,
                                         get_config->interfaceSize,
                                         "got another interfaceSize: %d",
                                         get_config->interfaceSize);
        }

        /* Data pattern */
        for (i = 0;
             i < (sizeof(set_config->cyclicPatternArr) / sizeof(set_config->cyclicPatternArr[0]));
             i++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(set_config->cyclicPatternArr[i],
                                         get_config->cyclicPatternArr[i],
                                         "got another cyclicPatternArr[%d]: %d",
                                         i, get_config->cyclicPatternArr[i]);
        }
    }

    *compareStatus = GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagPacketGeneratorConnectSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             connect,
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssPxDiagPacketGeneratorConnectSet) */
GT_VOID cpssPxDiagPacketGeneratorConnectSetUT(GT_VOID)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call with connect[GT_TRUE],
                 configPtr {
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
                    cyclicPatternArr
                        [0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                         0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                         0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                         0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                         0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                         0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                         0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                         0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55],
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
                    ifg[0, 1024, 0xFFFF]
                 }.
           Expected: GT_OK.
    1.1.2. Call cpssPxDiagPacketGeneratorConnectGet.
           Expected: GT_OK and the same values as was set.
    1.1.3. Call with connect[GT_TRUE],
           configPtr->vlanTagEnable[GT_TRUE],
           out of range configPtr->vpt[8] and other valid params.
           Expected: GT_OUT_OF_RANGE.
    1.1.4. Call with connect[GT_TRUE],
           configPtr->vlanTagEnable[GT_FALSE],
           out of range configPtr->vpt[8] and other valid params.
           Expected: GT_OK.
    1.1.5. Call with connect[GT_FALSE],
           configPtr->vlanTagEnable[GT_TRUE],
           out of range configPtr->vpt[8] and other valid params.
           Expected: GT_OK.
    1.1.6. Call with connect[GT_TRUE],
           configPtr->vlanTagEnable[GT_TRUE],
           out of range configPtr->cfi[2] and other valid params.
           Expected: GT_OUT_OF_RANGE.
    1.1.7. Call with connect[GT_TRUE],
           configPtr->vlanTagEnable[GT_FALSE],
           out of range configPtr->cfi[2] and other valid params.
           Expected: GT_OK.
    1.1.8. Call with connect[GT_FALSE],
           configPtr->vlanTagEnable[GT_TRUE],
           out of range configPtr->cfi[2] and other valid params.
           Expected: GT_OK.
    1.1.9. Call with connect[GT_TRUE],
           configPtr->vlanTagEnable[GT_TRUE],
           out of range configPtr->vid[4096] and other valid params.
           Expected: GT_OUT_OF_RANGE.
    1.1.10. Call with connect[GT_TRUE],
            configPtr->vlanTagEnable[GT_FALSE],
            out of range configPtr->vid[4096] and other valid params.
            Expected: GT_OK.
    1.1.11. Call with connect[GT_FALSE],
            configPtr->vlanTagEnable[GT_TRUE],
            out of range configPtr->vid[4096] and other valid params.
            Expected: GT_OK.
    1.1.12. Call with connect[GT_TRUE],
            out of range enum value configPtr->payloadType and other valid params.
            Expected: GT_BAD_PARAM.
    1.1.13. Call with connect[GT_FALSE],
            out of range enum value configPtr->payloadType and other valid params.
            Expected: GT_OK.
    1.1.14. Call with connect[GT_TRUE],
            out of range enum value configPtr->packetLengthType and other valid params.
            Expected: GT_BAD_PARAM.
    1.1.15. Call with connect[GT_FALSE],
            out of range enum value configPtr->packetLengthType and other valid params.
            Expected: GT_OK.
    1.1.16. Call with connect[GT_TRUE],
            configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
            out of range configPtr->packetLength[16384] and other valid params.
            Expected: GT_BAD_PARAM.
    1.1.17. Call with connect[GT_TRUE],
            configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E],
            out of range configPtr->packetLength[16384] and other valid params.
            Expected: GT_OK.
    1.1.18. Call with connect[GT_FALSE],
            configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
            out of range configPtr->packetLength[16384] and other valid params.
            Expected: GT_OK.
    1.1.19. Call with connect[GT_TRUE],
            out of range enum value configPtr->transmitMode and other valid params.
            Expected: GT_BAD_PARAM.
    1.1.20. Call with connect[GT_FALSE],
            out of range enum value configPtr->transmitMode and other valid params.
            Expected: GT_OK.
    1.1.21. Call with connect[GT_TRUE],
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
            out of range configPtr->packetCount[8192] and other valid params.
            Expected: GT_BAD_PARAM.
    1.1.22. Call with connect[GT_TRUE],
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E],
            out of range configPtr->packetCount[8192] and other valid params.
            Expected: GT_OK.
    1.1.23. Call with connect[GT_FALSE],
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
            out of range configPtr->packetCount[8192] and other valid params.
            Expected: GT_OK.
    1.1.24. Call with connect[GT_TRUE],
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
            out of range enum values configPtr->packetCountMultiplier and other valid params.
            Expected: GT_BAD_PARAM.
    1.1.25. Call with connect[GT_TRUE],
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E],
            out of range enum values configPtr->packetCountMultiplier and other valid params.
            Expected: GT_OK.
    1.1.26. Call with connect[GT_FALSE],
            configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
            out of range enum values configPtr->packetCountMultiplier and other valid params.
            Expected: GT_OK.
    1.1.27. Call with connect[GT_TRUE],
            out of range configPtr[NULL] and other valid values.
            Expected: GT_BAD_PTR.
    1.1.28. Call with connect[GT_FALSE],
            out of range configPtr[NULL] and other valid values.
            Expected: GT_OK.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For all active devices check function for out of bound value for port number.
         Expected: GT_BAD_PARAM.
    1.4. For all active devices check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st          = GT_OK;
    GT_U32                              i           = 0;
    GT_U8                               dev         = 0;
    GT_PHYSICAL_PORT_NUM                port        = 0;
    GT_BOOL                             connect     = GT_FALSE;
    GT_BOOL                             connectRet  = GT_FALSE;
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  config      = {{{0}}};
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  configRet   = {{{0}}};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with connect[GT_TRUE],
                             configPtr {
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
                                cyclicPatternArr
                                    [0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                                     0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                                     0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                                     0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                                     0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                                     0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                                     0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55
                                     0x00:0xA5:0xFF:0x00:0xA5:0xFF:0xAA:0x55],
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
                                ifg[0, 1024, 0xFFFF]
                             }.
                Expected: GT_OK.
            */

            /*
                1.1.1. Call cpssPxDiagPacketGeneratorConnectSet with:
                           macDaIncrementEnable[GT_FALSE],
                           vid[1],
                           vpt[0],
                           cfi[0],
                           etherType[0x0000],
                           payloadType[CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E],
                           packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
                           packetLength[20]
                           transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E],
                           ifg[0].
                Expected: GT_OK.
            */
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

            for (i = 0; i < 8; i++)
            {
                config.cyclicPatternArr[8 * i]     = 0x55;
                config.cyclicPatternArr[8 * i + 1] = 0xAA;
                config.cyclicPatternArr[8 * i + 2] = 0xFF;
                config.cyclicPatternArr[8 * i + 3] = 0xA5;
                config.cyclicPatternArr[8 * i + 4] = 0x00;
                config.cyclicPatternArr[8 * i + 5] = 0xFF;
                config.cyclicPatternArr[8 * i + 6] = 0xA5;
                config.cyclicPatternArr[8 * i + 7] = 0x00;
            }

            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 20;
            config.undersizeEnable = GT_FALSE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_CONTINUES_E;
            config.ifg = 0;


            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxDiagPacketGeneratorConnectGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connectRet, &configRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (GT_OK == st)
            {
                prvCpssPxDiagPacketGeneratorCompareConfig(connect, connectRet,
                                                          &config, &configRet, &st);
                if (st != GT_OK)
                {
                    return;
                }
            }

            /*
                1.1.1. Call cpssPxDiagPacketGeneratorConnectSet with:
                           vid[1024],
                           vpt[3],
                           cfi[0],
                           etherType[0xAA55],
                           payloadType[CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E],
                           packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E],
                           transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                           packetCount[10],
                           packetCountMultiplier[CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_512_E],
                           ifg[1024].
                Expected: GT_OK.
            */
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
            config.ifg = 1024;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxDiagPacketGeneratorConnectGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connectRet, &configRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (st == GT_OK)
            {
                prvCpssPxDiagPacketGeneratorCompareConfig(connect, connectRet,
                                                          &config, &configRet, &st);
                if (st != GT_OK)
                {
                    return;
                }
            }

            /*
                1.1.1. Call cpssPxDiagPacketGeneratorConnectSet with:
                           vid[4095],
                           vpt[7],
                           cfi[1],
                           etherType[0xFFFF],
                           payloadType[CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E],
                           packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
                           transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                           packetCount[1],
                           packetCountMultiplier[CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_512_E],
                           ifg[0xFFFF].
                Expected: GT_OK.
            */
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
            config.ifg = 0xFFFF;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxDiagPacketGeneratorConnectGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connectRet, &configRet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if (GT_OK == st)
            {
                prvCpssPxDiagPacketGeneratorCompareConfig(connect, connectRet,
                                                          &config, &configRet, &st);
                if (st != GT_OK)
                {
                    return;
                }
            }

            /*
                1.1.3. Call with connect[GT_TRUE],
                       configPtr->vlanTagEnable[GT_TRUE],
                       out of range configPtr->vpt[8] and other valid params.
                Expected: GT_OUT_OF_RANGE.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 8;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 7;

            /*
                1.1.4. Call with connect[GT_TRUE],
                       configPtr->vlanTagEnable[GT_FALSE],
                       out of range configPtr->vpt[8] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_FALSE;
            config.vpt = 8;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 7;

            /*
                1.1.5. Call with connect[GT_FALSE],
                       configPtr->vlanTagEnable[GT_TRUE],
                       out of range configPtr->vpt[8] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 8;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vpt = 7;

            /*
                1.1.6. Call with connect[GT_TRUE],
                       configPtr->vlanTagEnable[GT_TRUE],
                       out of range configPtr->cfi[2] and other valid params.
                Expected: GT_OUT_OF_RANGE.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 2;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 1;

            /*
                1.1.7. Call with connect[GT_TRUE],
                       configPtr->vlanTagEnable[GT_FALSE],
                       out of range configPtr->cfi[2] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_FALSE;
            config.cfi = 2;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 1;

            /*
                1.1.8. Call with connect[GT_FALSE],
                       configPtr->vlanTagEnable[GT_TRUE],
                       out of range configPtr->cfi[2] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 2;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.cfi = 1;

            /*
                1.1.9. Call with connect[GT_TRUE],
                       configPtr->vlanTagEnable[GT_TRUE],
                       out of range configPtr->vid[4096] and other valid params.
                Expected: GT_OUT_OF_RANGE.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4096;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4095;

            /*
                1.1.10. Call with connect[GT_TRUE],
                        configPtr->vlanTagEnable[GT_FALSE],
                        out of range configPtr->vid[4096] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_FALSE;
            config.vid = 4096;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4095;

            /*
                1.1.11. Call with connect[GT_FALSE],
                        configPtr->vlanTagEnable[GT_TRUE],
                        out of range configPtr->vid[4096] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4096;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.vlanTagEnable = GT_TRUE;
            config.vid = 4095;

            /*
                1.1.12. Call with connect[GT_TRUE],
                        out of range enum value configPtr->payloadType and other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssPxDiagPacketGeneratorConnectSet(dev, port,
                                                                    connect, &config),
                                config.payloadType);

            /*
                1.1.13. Call with connect[GT_FALSE],
                        out of range enum value configPtr->payloadType and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.payloadType = -1;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.payloadType = CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E;

            /*
                1.1.14. Call with connect[GT_TRUE],
                        out of range enum value configPtr->packetLengthType and other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssPxDiagPacketGeneratorConnectSet(dev, port,
                                                                    connect, &config),
                                config.packetLengthType);

            /*
                1.1.15. Call with connect[GT_FALSE],
                        out of range enum value configPtr->packetLengthType and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.packetLengthType = -1;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;

            /*
                1.1.16. Call with connect[GT_TRUE],
                        configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
                        out of range configPtr->packetLength[16384] and other valid params.
                Expected: GT_OUT_OF_RANGE.
            */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16384;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16383;

            /*
                1.1.17. Call with connect[GT_TRUE],
                        configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E],
                        out of range configPtr->packetLength[16384] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;
            config.packetLength = 16384;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16383;

            /*
                1.1.18. Call with connect[GT_FALSE],
                        configPtr->packetLengthType[CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E],
                        out of range configPtr->packetLength[16384] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16384;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E;
            config.packetLength = 16383;

            /*
                1.1.19. Call with connect[GT_TRUE],
                        out of range enum value configPtr->transmitMode and other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            UTF_ENUMS_CHECK_MAC(cpssPxDiagPacketGeneratorConnectSet(dev, port,
                                                                    connect, &config),
                                config.transmitMode);

            /*
                1.1.20. Call with connect[GT_FALSE],
                        out of range enum value configPtr->transmitMode and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.transmitMode = -1;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;

            /*
                1.1.21. Call with connect[GT_TRUE],
                        configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                        out of range configPtr->packetCount[8192] and other valid params.
                Expected: GT_OUT_OF_RANGE.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 8192;
            config.packetLength = 20;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 1;
            config.packetLength = 16383;

            /*
                1.1.22. Call with connect[GT_TRUE],
                        configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E],
                        out of range configPtr->packetCount[8192] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_CONTINUES_E;
            config.packetCount = 8192;
            config.packetLength = 20;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 1;
            config.packetLength = 16383;

            /*
                1.1.23. Call with connect[GT_FALSE],
                        configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                        out of range configPtr->packetCount[8192] and other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 8192;
            config.packetLength = 20;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCount = 1;
            config.packetLength = 16383;

            /*
                1.1.24. Call with connect[GT_TRUE],
                        configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                        out of range enum values configPtr->packetCountMultiplier and
                        other valid params.
                Expected: GT_BAD_PARAM.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            UTF_ENUMS_CHECK_MAC(cpssPxDiagPacketGeneratorConnectSet(dev, port,
                                                                    connect, &config),
                                config.packetCountMultiplier);

            /*
                1.1.25. Call with connect[GT_TRUE],
                        configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_CONTINUES_E],
                        out of range enum values configPtr->packetCountMultiplier and
                        other valid params.
                Expected: GT_OK.
            */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_CONTINUES_E;
            config.packetCountMultiplier = -1;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCountMultiplier = CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E;

            /*
                1.1.26. Call with connect[GT_FALSE],
                        configPtr->transmitMode[CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E],
                        out of range enum values configPtr->packetCountMultiplier and
                        other valid params.
                Expected: GT_OK.
            */
            connect = GT_FALSE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCountMultiplier = -1;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
            config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
            config.packetCountMultiplier = CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E;

            /*
                1.1.27. Call with connect[GT_TRUE],
                        out of range configPtr[NULL] and other valid values.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /*
                1.1.28. Call with connect[GT_FALSE],
                        out of range configPtr[NULL] and other valid values.
                Expected: GT_OK.
            */
            connect = GT_FALSE;

            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* restore values */
            connect = GT_TRUE;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                 for CPU port number.
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* restore value */
    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagPacketGeneratorConnectSet(dev, port, connect, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagPacketGeneratorConnectGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *connectPtr,
    OUT CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssPxDiagPacketGeneratorConnectGet) */
GT_VOID cpssPxDiagPacketGeneratorConnectGetUT(GT_VOID)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Call with non-null connectPtr, non-null configPtr.
           Expected: GT_OK.
    1.1.2. Call with null connectPtr, non-null configPtr.
           Expected: GT_BAD_PTR.
    1.1.3. Call with non-null connectPtr, null configPtr.
           Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For active devices check function for out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For active devices check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st      = GT_OK;
    GT_U8                               dev     = 0;
    GT_PHYSICAL_PORT_NUM                port    = 0;
    GT_BOOL                             connect = GT_FALSE;
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  config  = {{{0}}};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null connectPtr, non-null configPtr.
                Expected: GT_OK.
            */
            st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connect, &config);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with null connectPtr, non-null configPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxDiagPacketGeneratorConnectGet(dev, port, NULL, &config);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.3. Call with non-null connectPtr, null configPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connect, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connect, &config);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connect, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                 for CPU port number.
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connect, &config);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* restore value */
    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connect, &config);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagPacketGeneratorConnectGet(dev, port, &connect, &config);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagPacketGeneratorTransmitEnable
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
*/
/* UTF_TEST_CASE_MAC(cpssPxDiagPacketGeneratorTransmitEnable) */
GT_VOID cpssPxDiagPacketGeneratorTransmitEnableUT(GT_VOID)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
    1.1.1. Connect port to packet generator.
           Expected: GT_OK.
    1.1.2. Call with enable[GT_FALSE].
           Expected: GT_OK.
    1.1.3. Call with enable[GT_TRUE].
           Expected: GT_OK.
    1.1.4. Connect port to packet generator.
           Expected: GT_OK.
    1.1.5. Call with enable[GT_FALSE].
           Expected: GT_BAD_STATE.
    1.1.6. Call with enable[GT_TRUE].
           Expected: GT_BAD_STATE.
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For active devices check function for out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For active devices check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st      = GT_OK;
    GT_U8                               dev     = 0;
    GT_PHYSICAL_PORT_NUM                port    = 0;
    GT_BOOL                             enable  = GT_FALSE;
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  config  = {{{0}}};

    config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Connect port to packet generator.
                Expected: GT_OK.
            */
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, GT_TRUE, &config);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with enable[GT_FALSE].
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.3. Call with enable[GT_TRUE].
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.4. Connect port to packet generator.
                Expected: GT_OK.
            */
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, GT_FALSE, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.5. Call with enable[GT_FALSE].
                Expected: GT_BAD_STATE.
            */
            enable = GT_FALSE;

            st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);

            /*
                1.1.6. Call with enable[GT_TRUE].
                Expected: GT_BAD_STATE.
            */
            enable = GT_TRUE;

            st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
        }

        /* restore value */
        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                 for CPU port number.
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* restore value */
    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
)
*/
/* UTF_TEST_CASE_MAC(cpssPxDiagPacketGeneratorBurstTransmitStatusGet) */
GT_VOID cpssPxDiagPacketGeneratorBurstTransmitStatusGetUT(GT_VOID)
{
/*
    1. Go over all active devices.
    1.1. For all active devices go over all available physical ports.
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
    1.2. For all active devices go over all non available physical ports.
         Expected: GT_BAD_PARAM.
    1.3. For active devices check function for out of bound value of port number.
         Expected: GT_BAD_PARAM.
    1.4. For active devices check function for CPU port number.
         Expected: GT_BAD_PARAM.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value for device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st                  = GT_OK;
    GT_U8                               dev                 = 0;
    GT_PHYSICAL_PORT_NUM                port                = 0;
    GT_BOOL                             burstTransmitDone   = GT_FALSE;
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  config              = {{{0}}};

    config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Connect port to packet generator.
                Expected: GT_OK.
            */
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, GT_TRUE, &config);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with non-null burstTransmitDonePtr.
                Expected: GT_OK.
            */
            st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.3. Call with out of range burstTransmitDonePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.4. Disconnect port from packet generator.
                Expected: GT_OK.
            */
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, GT_FALSE, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.5. Call with non-null burstTransmitDonePtr.
                Expected: GT_BAD_STATE.
            */
            st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports.
        */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_BAD_PARAM
                 for CPU port number.
        */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /*    check that function returns GT_NOT_APPLICABLE_DEVICE.         */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port, &burstTransmitDone);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/**
* @internal cpssPxDiagPacketGeneratorCheckDataTransmissionUT function
* @endinternal
*
* @brief   Check that sent packet using Packet Generator come to right port.
*/
GT_VOID cpssPxDiagPacketGeneratorCheckDataTransmissionUT(GT_VOID)
{
/*
    1. Go over all active devices
    1.1. Go over all applicable ports.
    1.1.1. Prepare port to test: save link state, force link up if needed,
           reset counters.
    1.1.2. Check link state - skip if could not force link up port
    1.1.3. Configure packet generator and transmit packet
    1.1.4. Wait for packet
    1.1.5. Disconnect packet generator from port and disable transmission
    1.1.6. Print information about sent packet
    1.1.7. Restore port state
*/
    GT_U8                               dev = 0;
    GT_STATUS                           st = GT_OK;
    GT_PHYSICAL_PORT_NUM                port = 0;
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  config = {{{0}}};
    GT_BOOL                             portState;
    GT_BOOL                             isPortUpped;
    GT_BOOL                             isBurstTransmitDone;
    CPSS_INTERFACE_INFO_STC             portInterface;       /* used for link up */
    GT_U32                              timeout;
    CPSS_PX_TGF_PORT_MAC_COUNTERS_STC   portCounters;

    /* configure packet that we will try to send */
    config.macDa.arEther[0] = 0x01;
    config.macDaIncrementEnable = GT_FALSE;
    config.macSa.arEther[0] = 0x88;
    config.vlanTagEnable = GT_FALSE;
    config.etherType = 0x0800;
    config.payloadType = CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E;
    config.packetLengthType = CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E;
    config.packetCount = 1;
    config.packetCountMultiplier = CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E;
    config.undersizeEnable = GT_FALSE;
    config.transmitMode = CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E;
    config.interfaceSize = CPSS_DIAG_PG_IF_SIZE_DEFAULT_E;
    config.ifg = 1024;

    cpssOsMemSet(&portCounters, 0, sizeof(portCounters));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    if(config.ifg == 1024)/* always TRUE !!! but we allow ASIC_SIMULATION to compile this code ! without warning */
    {
        /* asic simulation not support this UT --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC
    }


    /* clear capturing RxPcktTable */
    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    /*
        1. Go over all active devices
    */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC

        /* configure portInterface for tgfTrafficGenerator... functions */
        portInterface.type             = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum = dev;

        /* prepare to go over all applicable ports for current device */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. Go over all applicable ports */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Prepare port to test: save link state, force link up
                       if needed, reset counters.
            */
            portInterface.devPort.portNum = port;

            /* force up link if needed */
            st = cpssPxPortLinkStatusGet(dev, port, &portState);
            if ((st == GT_OK) && (portState == GT_FALSE))
            {
                tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_TRUE);
                cpssOsTimerWkAfter(10);
            }

            /* reset counters */
            tgfTrafficGeneratorPortCountersEthReset(&portInterface);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /*
                1.1.2. Check link state - skip if could not force link up port
            */
            st = cpssPxPortLinkStatusGet(dev, port, &isPortUpped);
            if ((st != GT_OK) || (!isPortUpped))
            {
                continue;
            }

            /*
                1.1.3. Configure packet generator and transmit packet
            */
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, GT_TRUE, &config);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.4. Wait for packet
            */
            timeout = 10;
            isBurstTransmitDone = GT_FALSE;
            do
            {
                timeout--;

                st = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(dev, port,
                        &isBurstTransmitDone);
                if(timeout && ((st != GT_OK) || (!isBurstTransmitDone)))
                {
                    /*allow time to process*/
                    cpssOsTimerWkAfter(100);
                }
            }
            while (timeout && (st == GT_OK) && (!isBurstTransmitDone));

            if(timeout == 0 && (isBurstTransmitDone == GT_FALSE))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_TIMEOUT, "Timeout finished: waiting for processing the 'Burst Transmit' !!!");
            }

            /*
                1.1.5. Disconnect packet generator from port and disable transmission
            */
            st = cpssPxDiagPacketGeneratorConnectSet(dev, port, GT_FALSE, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssPxDiagPacketGeneratorTransmitEnable(dev, port, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);

            /*
                1.1.6. Print information about sent packet
            */
            /* read counters; clear on read registers       */
            st = cpssPxTgfEthCountersRead(dev, port, &portCounters);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssPxTgfEthCountersRead");

            /* if packet do not come - generate test fail */
            if (portCounters.goodPktsSent == 0)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, GT_FAIL, dev, port);
            }

            /*
                1.1.7. Restore port state
            */
            if (portState == GT_FALSE)
            {
                tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
            }
        }
    }
}

/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxDiagPacketGenerator suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxDiagPacketGenerator)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagPacketGeneratorConnectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagPacketGeneratorConnectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagPacketGeneratorTransmitEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagPacketGeneratorBurstTransmitStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagPacketGeneratorCheckDataTransmission)
UTF_SUIT_END_TESTS_MAC(cpssPxDiagPacketGenerator)

