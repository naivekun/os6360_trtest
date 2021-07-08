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
* @file cpssPxTgfCncUT.c
*
* @brief Functional tests for CNC subsystem of PX
*
* @version   1
********************************************************************************
*/
/* includes */
#include <cpss/px/cpssPxTypes.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <cpss/px/cnc/cpssPxCnc.h>
#include <gtOs/gtOsMem.h>

#define PRV_TGF_PACKET_NUM 5
#define PRV_TGF_SEND_PORT_NUM 0
#define PRV_TGF_BLOCK_NUM 0
#define PRV_TGF_RESET_BYTE_COUNT 1024
#define PRV_TGF_RESET_PACKET_COUNT 512
#define PRV_TGF_RESET1_BYTE_COUNT0  0xFFFFFFFF
#define PRV_TGF_RESET1_BYTE_COUNT1  0x0000001F
#define PRV_TGF_RESET1_PACKET_COUNT 0x03FFFFFF

#define PRV_TGF_MEMBERS_NUM_MAC(_x) (sizeof(_x) / sizeof(_x[0]))

#define PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS   1024

typedef struct
{
    GT_U32 index;
    GT_U32 packets;
    GT_U32 bytes;
} PRV_TGF_COUNTER_VALUES_STC;

static GT_U8 prvTgfFrameArr[] = {
      0x00, 0x00, 0x11, 0x12, 0x13, 0x14, /*mac da 00:0x00:0x01:0x02:0x03:0x04*/
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /*mac sa 00:00:00:00:00:01 */
      0x55, 0x55,                         /*ethertype                */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

static TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC prvTgfEgrPortsArr[] = {{0, 0, NULL}};

/**
* @internal cpssPxTgfPxCncClrByReadConfig function
* @endinternal
*
* @brief   Configure test cpssPxTgfPxCncClrByRead
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID cpssPxTgfPxCncClrByReadConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS st;
    static GT_BOOL clientEnableRestore;
    static GT_BOOL clearOnReadRestore;
    static GT_U64  rangesRestore;
    static CPSS_PX_CNC_COUNTER_STC clearValueRestore;
    GT_U64  ranges;
    CPSS_PX_CNC_COUNTER_STC clearValue;

    if(GT_TRUE == config)
    {
        /* AUTODOC: Save egress-processing client enabled state */
        st = cpssPxCncBlockClientEnableGet(dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E, &clientEnableRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E);

        /* AUTODOC: Enable egress-processing client */
        st = cpssPxCncBlockClientEnableSet(dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E, GT_TRUE);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E, GT_TRUE);

        /* AUTODOC: Save clear by read enable state */
        st = cpssPxCncCounterClearByReadEnableGet(dev, &clearOnReadRestore);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* AUTODOC: Enable clear by read */
        st = cpssPxCncCounterClearByReadEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, GT_TRUE);

        /* AUTODOC: Save ranges enabled state for the block */
        st = cpssPxCncBlockClientRangesGet(dev, PRV_TGF_BLOCK_NUM,
            &rangesRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);

        /* AUTODOC: Enable counter range 0..1K-1 for the block */
        ranges.l[0] = 0x1;
        ranges.l[1] = 0x0;
        st = cpssPxCncBlockClientRangesSet(dev, PRV_TGF_BLOCK_NUM, ranges);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);

        /* AUTODOC: Save clear by read value */
        st = cpssPxCncCounterClearByReadValueGet(dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &clearValueRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);

        /* AUTODOC: Set clear by read value */
        clearValue.byteCount.l[0] = PRV_TGF_RESET_BYTE_COUNT;
        clearValue.byteCount.l[1] = 0;
        clearValue.packetCount.l[0] = PRV_TGF_RESET_PACKET_COUNT;
        clearValue.packetCount.l[1] = 0;
        st = cpssPxCncCounterClearByReadValueSet(dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &clearValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
    }
    else
    {
        /* AUTODOC: Restore egress-processing enabled state */
        st = cpssPxCncBlockClientEnableSet(dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E, clientEnableRestore);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E, clientEnableRestore);

        /* AUTODOC: Restore clear by read enabled state */
        st = cpssPxCncCounterClearByReadEnableSet(dev, clearOnReadRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, clearOnReadRestore);

        /* AUTODOC: Restore ranges */
        st = cpssPxCncBlockClientRangesSet(dev, PRV_TGF_BLOCK_NUM,
            rangesRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);

        /* AUTODOC: Restore clear by read value */
        st = cpssPxCncCounterClearByReadValueSet(dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &clearValueRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
    }
}

/**
* @internal cpssPxTgfPxCncClrByReadExecute function
* @endinternal
*
* @brief   Execute test cpssPxTgfPxCncClrByRead for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID cpssPxTgfPxCncClrByReadExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      timeout;
    GT_U32      inProcess;
    GT_U32      count;
    CPSS_PX_CNC_COUNTER_STC counters[PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS];

    /* AUTODOC: Dump counters */
    st = cpssPxCncBlockUploadTrigger(dev, PRV_TGF_BLOCK_NUM);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);
    timeout = 10;
    while(1)
    {
        st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if ((inProcess & (0x1 << PRV_TGF_BLOCK_NUM)) == 0)
        {
            break;
        }
        cpssOsTimerWkAfter(100);
        if(!(timeout--))
        {
            PRV_UTF_LOG0_MAC("Timeout finished\n");
        }
    }

    count = PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS;
    st = cpssPxCncUploadedBlockGet(dev, &count,
        CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, counters);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS, count);

    /* AUTODOC: Dump counters. Read reset values */
    st = cpssPxCncBlockUploadTrigger(dev, PRV_TGF_BLOCK_NUM);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);
    timeout = 10;
    while(1)
    {
        st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if ((inProcess & (0x1 << PRV_TGF_BLOCK_NUM)) == 0)
        {
            break;
        }
        cpssOsTimerWkAfter(100);
        if(!(timeout--))
        {
            PRV_UTF_LOG0_MAC("Timeout finished\n");
        }
    }

    count = PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS;
    st = cpssPxCncUploadedBlockGet(dev, &count,
        CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, counters);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS, count);

    /* AUTODOC: Compare counters with reset value */
    for(ii = 0; ii < PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS; ii++)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(0, counters[ii].packetCount.l[1], ii);
        UTF_VERIFY_EQUAL1_PARAM_MAC(PRV_TGF_RESET_PACKET_COUNT,
            counters[ii].packetCount.l[0], ii);
        UTF_VERIFY_EQUAL1_PARAM_MAC(0, counters[ii].byteCount.l[1], ii);
        UTF_VERIFY_EQUAL1_PARAM_MAC(PRV_TGF_RESET_BYTE_COUNT,
            counters[ii].byteCount.l[0], ii);
    }
}

UTF_TEST_CASE_MAC(cpssPxTgfPxCncClrByRead)
{
    GT_U8   dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        cpssPxTgfPxCncClrByReadConfig(dev, GT_TRUE);

        /* AUTODOC: Execute the test */
        cpssPxTgfPxCncClrByReadExecute(dev);

        /* AUTODOC: Restore configuration */
        cpssPxTgfPxCncClrByReadConfig(dev, GT_FALSE);
    }

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/**
* @internal cpssPxTgfPxCncWrapAroundConfig function
* @endinternal
*
* @brief   Configure test cpssPxTgfPxCncWrapAround
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID cpssPxTgfPxCncWrapAroundConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
        GT_STATUS st;
    static GT_BOOL clientEnableRestore;
    static GT_BOOL clearOnReadRestore;
    static GT_BOOL wrapEnRestore;
    static GT_U64  rangesRestore;
    static CPSS_PX_CNC_COUNTER_STC clearValueRestore;
    static CPSS_PX_CNC_COUNTER_FORMAT_ENT formatRestore;
    GT_U64  ranges;
    CPSS_PX_CNC_COUNTER_STC clearValue;

    if(GT_TRUE == config)
    {
        /* AUTODOC: Save egress-processing client enabled state */
        st = cpssPxCncBlockClientEnableGet(dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E, &clientEnableRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E);

        /* AUTODOC: Enable egress-processing client */
        st = cpssPxCncBlockClientEnableSet(dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E, GT_TRUE);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E, GT_TRUE);

        /* AUTODOC: Save clear by read enable state */
        st = cpssPxCncCounterClearByReadEnableGet(dev, &clearOnReadRestore);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* AUTODOC: Enable clear by read */
        st = cpssPxCncCounterClearByReadEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, GT_TRUE);

        /* AUTODOC: Save ranges enabled state for the block */
        st = cpssPxCncBlockClientRangesGet(dev, PRV_TGF_BLOCK_NUM,
            &rangesRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);

        /* AUTODOC: Enable counter range 0..1K-1 for the block */
        ranges.l[0] = 0x1;
        ranges.l[1] = 0x0;
        st = cpssPxCncBlockClientRangesSet(dev, PRV_TGF_BLOCK_NUM, ranges);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);

        st = cpssPxCncCounterClearByReadValueGet(dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E, &clearValueRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E);

        /* AUTODOC: Set clear by read value */
        clearValue.byteCount.l[0]   = PRV_TGF_RESET1_BYTE_COUNT0;
        clearValue.byteCount.l[1]   = PRV_TGF_RESET1_BYTE_COUNT1;
        clearValue.packetCount.l[0] = PRV_TGF_RESET1_PACKET_COUNT;
        clearValue.packetCount.l[1] = 0;
        st = cpssPxCncCounterClearByReadValueSet(dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E, &clearValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E);

        /* AUTODOC: Save counter format for the block */
        st = cpssPxCncCounterFormatGet(dev, PRV_TGF_BLOCK_NUM, &formatRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);

        /* AUTODOC: Set counter format */
        st = cpssPxCncCounterFormatSet(dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E);

        /* AUTODOC: Save wraparound enabled state */
        st = cpssPxCncCounterWraparoundEnableGet(dev, &wrapEnRestore);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* AUTODOC: Enable wraparound */
        st = cpssPxCncCounterWraparoundEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, GT_TRUE);
    }
    else
    {
        /* AUTODOC: Restore egress-processing enabled state */
        st = cpssPxCncBlockClientEnableSet(dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E, clientEnableRestore);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E, clientEnableRestore);

        /* AUTODOC: Restore clear by read enabled state */
        st = cpssPxCncCounterClearByReadEnableSet(dev, clearOnReadRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, clearOnReadRestore);

        /* AUTODOC: Restore ranges */
        st = cpssPxCncBlockClientRangesSet(dev, PRV_TGF_BLOCK_NUM,
            rangesRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);

        /* AUTODOC: Restore clear by read value */
        st = cpssPxCncCounterClearByReadValueSet(dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E, &clearValueRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E);

        /* AUTODOC: Restore counting format */
        st = cpssPxCncCounterFormatSet(dev, PRV_TGF_BLOCK_NUM, formatRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM,
            formatRestore);

        /* AUTODOC: Retore wraparound enabled state */
        st = cpssPxCncCounterWraparoundEnableSet(dev, wrapEnRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, wrapEnRestore);
    }
}

/**
* @internal cpssPxTgfPxCncWrapAroundExecute function
* @endinternal
*
* @brief   Execute test cpssPxTgfPxCncWrapAround for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID cpssPxTgfPxCncWrapAroundExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      timeout;
    GT_U32      inProcess;
    GT_U32      count;
    CPSS_PX_CNC_COUNTER_STC counters[PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS];
    GT_U32      indexesArr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    GT_U32      indexCount;

    /* AUTODOC: Dump counters. Reset values will be set */
    st = cpssPxCncBlockUploadTrigger(dev, PRV_TGF_BLOCK_NUM);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, PRV_TGF_BLOCK_NUM);
    timeout = 10;
    while(1)
    {
        st = cpssPxCncBlockUploadInProcessGet(dev, &inProcess);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if ((inProcess & (0x1 << PRV_TGF_BLOCK_NUM)) == 0)
        {
            break;
        }
        cpssOsTimerWkAfter(100);
        if(!(timeout--))
        {
            PRV_UTF_LOG0_MAC("Timeout finished\n");
        }
    }

    count = PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS;
    st = cpssPxCncUploadedBlockGet(dev, &count,
        CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E, counters);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PRV_TGF_CNC_BLOCK_MAX_ENTRIES_CNS, count);

    indexCount = 8;
    st = cpssPxCncCounterWraparoundIndexesGet(dev, PRV_TGF_BLOCK_NUM,
        &indexCount, indexesArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, indexCount);

    /* AUTODOC: Send packets */
    for(ii = 0; ii < PRV_TGF_PACKET_NUM; ii++)
    {
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
            PRV_TGF_SEND_PORT_NUM, prvTgfFrameArr, sizeof(prvTgfFrameArr),
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfEgrPortsArr), prvTgfEgrPortsArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* AUTODOC: Check wraparound state */
    indexCount = 8;
    st = cpssPxCncCounterWraparoundIndexesGet(dev, PRV_TGF_BLOCK_NUM,
        &indexCount, indexesArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(2, indexCount);
    UTF_VERIFY_EQUAL0_PARAM_MAC(16, indexesArr[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0,  indexesArr[1]);

    /* AUTODOC: Check wraparound state */
    indexCount = 8;
    st = cpssPxCncCounterWraparoundIndexesGet(dev, PRV_TGF_BLOCK_NUM,
        &indexCount, indexesArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, indexCount);
}

UTF_TEST_CASE_MAC(cpssPxTgfPxCncWrapAround)
{
    GT_U8   dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        cpssPxTgfPxCncWrapAroundConfig(dev, GT_TRUE);

        /* AUTODOC: Execute the test */
        cpssPxTgfPxCncWrapAroundExecute(dev);

        /* AUTODOC: Restore configuration */
        cpssPxTgfPxCncWrapAroundConfig(dev, GT_FALSE);
    }

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTgfCpssPxTgfCnc suite
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfPxCnc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPxCncClrByRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPxCncWrapAround)
UTF_SUIT_END_TESTS_MAC(cpssPxTgfPxCnc)

