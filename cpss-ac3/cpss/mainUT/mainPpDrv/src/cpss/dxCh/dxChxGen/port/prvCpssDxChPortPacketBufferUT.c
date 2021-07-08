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
* @file prvCpssDxChPortPacketBufferUT.c
 *
 * @brief Unit tests for CPSS internal functions, that provides
 * Packet Buffer unit management. Tests not check wrong parameter values
 * rejecting because tested functions are private. Only Get after Set tests.
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

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPacketBuffer.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortPacketBuffer.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortPacketBufferInternal.h>

#define GPC_MAX_CHANNELS 8
typedef struct
{
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT speed[GPC_MAX_CHANNELS];
} GPC_READ_CFG_STC;

static GT_STATUS utilPacketBufferReadGpcCfgGet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tileIndex,
    IN  GT_U32           gpcIndex,
    OUT GPC_READ_CFG_STC *gpcReadCfgPtr
)
{
    GT_STATUS rc;
    GT_U32    channelIndex;

    for (channelIndex = 0; (channelIndex < GPC_MAX_CHANNELS); channelIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcChannelSpeedGet(
            devNum, tileIndex, gpcIndex, channelIndex,
            &(gpcReadCfgPtr->speed[channelIndex]));
        if (rc != GT_OK) return rc;
    }
    return GT_OK;
}

static GT_STATUS utilPacketBufferReadGpcCfgClear
(
    IN  GT_U8            devNum,
    IN  GT_U32           tileIndex,
    IN  GT_U32           gpcIndex
)
{
    GT_STATUS rc;
    GT_U32    channelIndex;

    /* cleanup to avoid virtual serdes overlapping */
    for (channelIndex = 0; (channelIndex < GPC_MAX_CHANNELS); channelIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet(
            devNum, tileIndex, gpcIndex, channelIndex,
            PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_NONE_E);
        if (rc != GT_OK) return rc;
    }
    return GT_OK;
}

static GT_STATUS utilPacketBufferReadGpcCfgRestore
(
    IN  GT_U8            devNum,
    IN  GT_U32           tileIndex,
    IN  GT_U32           gpcIndex,
    IN  GPC_READ_CFG_STC *gpcReadCfgPtr
)
{
    GT_STATUS rc;
    GT_U32    channelIndex;

    /* cleanup to avoid virtual serdes overlapping */
    rc = utilPacketBufferReadGpcCfgClear(
        devNum, tileIndex, gpcIndex);
    if (rc != GT_OK) return rc;
    /* set given speeds */
    for (channelIndex = 0; (channelIndex < GPC_MAX_CHANNELS); channelIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet(
            devNum, tileIndex, gpcIndex, channelIndex,
            gpcReadCfgPtr->speed[channelIndex]);
        if (rc != GT_OK) return rc;
    }
    return GT_OK;
}

static int utilPacketBufferReadGpcCfgDebugPrint = 0;
void utilPacketBufferReadGpcCfgDebugPrintSet(int v)
{
    utilPacketBufferReadGpcCfgDebugPrint = v;
}

static GT_STATUS utilPacketBufferReadGpcCfgPrint
(
    IN  GT_U8            devNum,
    IN  GT_U32           tileIndex,
    IN  GT_U32           gpcIndex
)
{
    GT_STATUS rc;
    GT_U32    channelIndex;
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT speed;
    GT_U32    speedGb;

    cpssOsPrintf("Channels, not-zero-speeds_gb: ");
    for (channelIndex = 0; (channelIndex < GPC_MAX_CHANNELS); channelIndex++)
    {
        rc = prvCpssDxChPortPacketBufferInternalGpcChannelSpeedGet(
            devNum, tileIndex, gpcIndex, channelIndex, &speed);
        if (rc != GT_OK) return rc;
        rc = prvCpssDxChPortPacketBufferInternalChannelSpeedTo25GUnits(
            speed, &speedGb);
        if (rc != GT_OK) return rc;
        if (speedGb != 0)
        {
            cpssOsPrintf("{%d, %dG} ", channelIndex, speedGb);
        }
    }
    cpssOsPrintf("\n");
    if (utilPacketBufferReadGpcCfgDebugPrint)
    {
        prvCpssDxChPortPacketBufferGpcSpeedsPrint(devNum, tileIndex, gpcIndex);
    }
    return GT_OK;
}

typedef struct
{
    GT_U32 channel; /* 0xFF - mark of end of array */
    GT_U32 speedInGb;
} PORT_SPEED_GB;

static GT_STATUS utilPacketBufferReadGpcSpeedGbArrSet
(
    IN  GT_U8            devNum,
    IN  GT_U32           tileIndex,
    IN  GT_U32           gpcIndex,
    IN  PORT_SPEED_GB    *portSpeedGbArrPtr
)
{
    PRV_CPSS_DXCH_PORT_PACKET_BUFFER_CHANNEL_SPEED_ENT speed;
    PORT_SPEED_GB *portSpeedGbPtr;
    GT_STATUS rc;

    for (portSpeedGbPtr = portSpeedGbArrPtr;
          (portSpeedGbPtr->channel < GPC_MAX_CHANNELS); portSpeedGbPtr++)
    {
        rc = prvCpssDxChPortPacketBufferInternalChannel25GUnitsToSpeed(
            portSpeedGbPtr->speedInGb, &speed);
        if (rc != GT_OK) return rc;
        rc = prvCpssDxChPortPacketBufferInternalGpcChannelSpeedSet(
            devNum, tileIndex, gpcIndex,
            portSpeedGbPtr->channel, speed);
        if (rc != GT_OK) return rc;
    }
    return GT_OK;
}

static void utilPacketBufferReadGpcSpeedTest
(
    IN  GT_U8            devNum,
    IN  GT_U32           tileIndex,
    IN  GT_U32           gpcIndex
)
{
    GT_STATUS rc;
    GT_U32 i;

    static PORT_SPEED_GB portSpeedGbArr_400[] = {{0, 400}, {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_2x_200[] = {{0, 200}, {4, 200}, {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_4x_100[] =
        {{0, 100}, {2, 100}, {4, 100}, {6, 100}, {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_8x_50[] =
        {{0, 50}, {1, 50}, {2, 50}, {3, 50},
        {4, 50}, {5, 50}, {6, 50}, {7, 50},
        {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_8x_25[] =
        {{0, 25}, {1, 25}, {2, 25}, {3, 25},
        {4, 25}, {5, 25}, {6, 25}, {7, 25},
        {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_force_4x_100[] =
        {{1, 0}, {0, 100}, {3, 0}, {2, 100},
        {5, 0}, {4, 100}, {7, 0}, {6, 100},
        {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_force_400[] =
        {{2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {0, 400}, {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_4X50_200[] =
        {{0, 50}, {1, 50}, {2, 50}, {3, 50}, {4, 200}, {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_100_50_25_100_100[] =
        {{1, 0}, {0, 100}, {2, 50}, {3, 25}, {4, 100}, {6, 100}, {0xFF}};

    static PORT_SPEED_GB *portSpeedGbPtrArr[] =
    {
        portSpeedGbArr_400
        ,portSpeedGbArr_2x_200
        ,portSpeedGbArr_4x_100
        ,portSpeedGbArr_8x_50
        ,portSpeedGbArr_8x_25
        ,portSpeedGbArr_8x_50
        ,portSpeedGbArr_force_4x_100
        ,portSpeedGbArr_force_400
        ,portSpeedGbArr_4X50_200
        ,portSpeedGbArr_100_50_25_100_100
    };
    static GT_U32 portSpeedGbPtrArrSize =
        (sizeof(portSpeedGbPtrArr) / sizeof(portSpeedGbPtrArr[0]));

    static PORT_SPEED_GB portSpeedGbArr_wrong0[] = {{1, 25}, {0, 100}, {0xFF}};
    static PORT_SPEED_GB portSpeedGbArr_wrong1[] = {{0, 100}, {1, 25}, {0xFF}};


    rc = utilPacketBufferReadGpcCfgClear(
        devNum, tileIndex, gpcIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

    for (i = 0; (i < portSpeedGbPtrArrSize); i++)
    {
        rc = utilPacketBufferReadGpcSpeedGbArrSet(
            devNum, tileIndex, gpcIndex, portSpeedGbPtrArr[i]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        rc = utilPacketBufferReadGpcCfgPrint(devNum, tileIndex, gpcIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
    }

    /* check rejectin wrong configuration */
    rc = utilPacketBufferReadGpcCfgClear(
        devNum, tileIndex, gpcIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
    rc = utilPacketBufferReadGpcSpeedGbArrSet(
        devNum, tileIndex, gpcIndex, portSpeedGbArr_wrong0);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
    rc = utilPacketBufferReadGpcSpeedGbArrSet(
        devNum, tileIndex, gpcIndex, portSpeedGbArr_wrong1);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(prvCpssDxChPortPacketBufferSpeedCfg)
{
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GPC_READ_CFG_STC gpcReadCfg;

    /* Falcon only */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E |
        UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* additional ckeck */
        if (! PRV_CPSS_SIP_6_CHECK_MAC(dev)) continue;

        /* save found configuration */
        st =  utilPacketBufferReadGpcCfgGet(
            dev, 0/*tileIndex*/, 0/*gpcIndex*/, &gpcReadCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* test can be interrupted by return from the middle       */
        /* the original speed configuration will be restored below */
        utilPacketBufferReadGpcSpeedTest(dev, 0/*tileIndex*/, 0/*gpcIndex*/);

        /* restore found configuration */
        st =  utilPacketBufferReadGpcCfgRestore(
            dev, 0/*tileIndex*/, 0/*gpcIndex*/, &gpcReadCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of prvCpssDxChPortPacketBuffer suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(prvCpssDxChPortPacketBuffer)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPortPacketBufferSpeedCfg)
UTF_SUIT_END_TESTS_MAC(prvCpssDxChPortPacketBuffer)


