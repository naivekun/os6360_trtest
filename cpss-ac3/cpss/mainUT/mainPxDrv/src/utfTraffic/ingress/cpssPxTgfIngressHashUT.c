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
* @file cpssPxTgfIngressHashUT.c
*
* @brief Ingress hash calculation tests with traffic for the Pipe devices.
*
* @version   1
********************************************************************************
*/
/* includes */
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>

#define PACKET_NUM_DEFAULT                   32
static GT_U32   packetNum = PACKET_NUM_DEFAULT;

#define SEND_PORT_NUM                        0
#define SEND_PORT_NUM_ANOTHER                5
#define LAG_PORT1_NUM                        1
#define LAG_PORT2_NUM                        2
#define LAG_PORT3_NUM                        3
#define LAG_PORT4_NUM                        4
#define PACKET_NUM                           packetNum
#define PRV_TGF_MEMBERS_NUM_MAC(_x) (sizeof(_x) / sizeof(_x[0]))

#define PRV_TGF_FROM_PORT0_PACKET_TYPE       0
#define PRV_TGF_FROM_PORT5_PACKET_TYPE       5

#define PRV_TGF_DOWNSTREAM_UC_PACKET_TYPE       10
#define PRV_TGF_DOWNSTREAM_MC_PACKET_TYPE       20
#define PRV_TGF_ETAG_TPID                       0x893f

static GT_U8 prvTgfFrame1Arr[] = {
    /*0x00*/0x00, 0x00, 0x01, 0x02, 0x03, 0x04,/*mac da 00:00:01:02:03:04*/
    /*0x06*/0x00, 0x00, 0x00, 0x00, 0x00, 0x01,/*mac sa 00:00:00:00:00:01 */
    /*0x0C*/0x55, 0x55,                        /*ethertype                */
    /*0x0E*/0x00, 0x01,                        /*payload*/
    /*0x10*/0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    /*0x18*/0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
    /*0x20*/0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    /*0x28*/0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    /*0x30*/0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    /*0x38*/0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    /*0x40*/0x00, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    /*0x48*/0x3A, 0x3B, 0x3C, 0x3D, 0x04, 0x3F};

static CPSS_PX_PORTS_BMP prvTgfUniformPortEntries[] =
    {0x1FF1, 0x1FE9, 0x1FE5, 0x1FE3};

static CPSS_PX_PORTS_BMP prvTgfDfltPortEntry[] = { 0x11FFF };

static CPSS_PX_INGRESS_HASH_UDBP_STC prvTgfUdbps[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] =
{
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  0, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 0*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  2, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 1*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  4, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 2*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  6, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 3*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  8, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 4*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 10, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 5*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 12, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 6*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 14, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 7*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  0, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 8*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  2, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 9*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  4, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /*10*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  5, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /*11*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  5, {GT_TRUE,  GT_FALSE, GT_FALSE, GT_FALSE}},/*12*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  5, {GT_FALSE, GT_TRUE,  GT_FALSE, GT_FALSE}},/*13*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  5, {GT_FALSE, GT_FALSE, GT_TRUE,  GT_FALSE}},/*14*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  5, {GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE}}, /*15*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  4, {GT_FALSE, GT_FALSE, GT_TRUE,  GT_FALSE}},/*16*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  5, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /*17*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  4, {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE}},/*18*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  8, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /*19*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E, 10, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}  /*20*/
};

static CPSS_PX_INGRESS_HASH_UDBP_STC prvTgfUdbpsA1[2] =
{
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,  0x3F, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 0*/
    {CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,  0x3F, {GT_TRUE,  GT_TRUE,  GT_TRUE,  GT_TRUE}}, /* 1*/
};

typedef struct
{
    GT_BOOL enableKeyEntry0;
    GT_BOOL enableKeyEntry5;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC keyDataPort0;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC keyMaskPort0;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC keyDataPort5;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC keyMaskPort5;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormatDstPort0;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormatDstPort5;
    CPSS_PX_PORTS_BMP dstMapEntry;
} PRV_TGF_RESTORE_VALUES_STC;

static PRV_TGF_RESTORE_VALUES_STC prvTgfRestore;

extern GT_VOID prvTgfPxIngressSaveConfig
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          enable
);

extern GT_VOID prvTgfPxIngressRestoreConfig
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          enable
);

static void packetNumDefaultSet(GT_BOOL usingNonPRNG)
{
    if(!cpssDeviceRunCheck_onEmulator())
    {
        packetNum = PACKET_NUM_DEFAULT;
        return;
    }

    if(usingNonPRNG == GT_TRUE)
    {
        packetNum = 4;
    }
    else
    {
        packetNum = 8;
    }

}


/**
* @internal prvTgfPxIngressHashDesignatedPortTableUniformFill function
* @endinternal
*
* @brief   Fill designated port table that designated ports were uniformely
*         distributed among table entries
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashDesignatedPortTableUniformFill
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32            entryNum,
    IN CPSS_PX_PORTS_BMP entryArr[]
)
{
    GT_U32              ii;
    GT_U32              portIdx;
    GT_STATUS           st;
    GT_U32              maxEntries;

    maxEntries =  (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum)) ? 
        CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS : 
        CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS/2;

    for(ii = 0; ii < maxEntries; ii++)
    {
        portIdx = ii % entryNum;
        st = cpssPxIngressHashDesignatedPortsEntrySet(devNum, ii, entryArr[portIdx]);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, ii, portIdx,
            entryArr[portIdx]);
    }
}

/**
* @internal prvTgfPxIngressHashDesignatedPortTableUniformFill_A1 function
* @endinternal
*
* @brief   Fill designated port table per packet type - UC/MC,
*         where designated ports are uniformely distributed among table entries.
* @param[in] devNum                   - the device number
* @param[in] packetTypeUc             - packet type UC
*                                      GT_TRUE  - packet type ic UC
*                                      GT_FALSE - packet type ic MC
* @param[in] designatedPortsArr[]
*                                      - array of designated ports entries
* @param[in] designatedPortsArrEntries
*                                      - number of entries in array of designated ports entries
*                                       None
*/
static GT_VOID prvTgfPxIngressHashDesignatedPortTableUniformFill_A1
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_BOOL           packetTypeUc,
    IN CPSS_PX_PORTS_BMP designatedPortsArr[],
    IN GT_U32            designatedPortsArrEntries
)
{
    GT_U32              ii, fromIndex;
    GT_U32              portIdx;
    GT_STATUS           st;
    GT_U32              maxEntries;

    maxEntries = CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS;

    if (packetTypeUc)
    {
        fromIndex = 0;
        maxEntries /= 2;
    }
    else
    {
        fromIndex = maxEntries/2;
    }

    for (ii = fromIndex; ii < maxEntries; ii++) 
    {
        portIdx = ii % designatedPortsArrEntries;
        st = cpssPxIngressHashDesignatedPortsEntrySet(devNum, ii, designatedPortsArr[portIdx]);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, ii, portIdx,
            designatedPortsArr[portIdx]);
    }
}

/**
* @internal prvTgfPxIncrementLAGCounters function
* @endinternal
*
* @brief   Increment egress counters.
*
* @param[in] devNum                   - the device number
* @param[in,out] egrCountersArr[4]        - array for LAG counters
* @param[in,out] egrCountersArr[4]
*                                       None
*/
static GT_VOID prvTgfPxIncrementLAGCounters
(
    IN      GT_SW_DEV_NUM     devNum,
    INOUT   GT_U32            egrCountersArr[4]
)
{
    GT_U32                          ii;
    GT_STATUS                       st;
    CPSS_PX_PORT_MAC_COUNTER_ENT    counterType;
    CPSS_PX_PORT_MAC_COUNTERS_STC   portCounters;

    counterType = CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E;

    for(ii = 1; ii < 12; ii++)
    {
        st = cpssPxPortMacCountersOnPortGet(devNum, ii, &portCounters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ii);
        if((ii >= LAG_PORT1_NUM) && (ii <= LAG_PORT4_NUM))
        {
            egrCountersArr[ii - 1] += portCounters.mibCounter[counterType].l[0];
        }
        else
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, portCounters.mibCounter[counterType].l[0]);
        }
    }
}

/**
* @internal prvTgfPxIngressHashLAGConfig function
* @endinternal
*
* @brief   Configure LAG
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashLAGConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS                                           st;
    GT_U32                                              indexConstant;
    GT_U32                                              entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC          keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC          keyMask;
    GT_BOOL                                             bypassLagDesignatedBitmap;

    /* AUTODOC: enable packet type key table entry for ports 0 and 5 */
    st = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,
        PRV_TGF_FROM_PORT0_PACKET_TYPE, &prvTgfRestore.enableKeyEntry0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    st = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,
        PRV_TGF_FROM_PORT5_PACKET_TYPE, &prvTgfRestore.enableKeyEntry5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,
        PRV_TGF_FROM_PORT0_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,
        PRV_TGF_FROM_PORT5_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* AUTODOC: packet key will always consist from ingress port only */
    st = cpssPxIngressPacketTypeKeyEntryGet(devNum, PRV_TGF_FROM_PORT0_PACKET_TYPE,
        &prvTgfRestore.keyDataPort0, &prvTgfRestore.keyMaskPort0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    st = cpssPxIngressPacketTypeKeyEntryGet(devNum, PRV_TGF_FROM_PORT0_PACKET_TYPE,
        &prvTgfRestore.keyDataPort5, &prvTgfRestore.keyMaskPort5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    keyData.profileIndex = SEND_PORT_NUM;
    keyMask.profileIndex = 0x7F;

    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
        PRV_TGF_FROM_PORT0_PACKET_TYPE, &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    keyData.profileIndex = SEND_PORT_NUM_ANOTHER;
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
        PRV_TGF_FROM_PORT5_PACKET_TYPE, &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* AUTODOC: configure packet type format entries for ports 0 and 5 */
    st = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        PRV_TGF_FROM_PORT0_PACKET_TYPE,
        &prvTgfRestore.packetTypeFormatDstPort0);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    st = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        PRV_TGF_FROM_PORT5_PACKET_TYPE,
        &prvTgfRestore.packetTypeFormatDstPort5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    indexConstant = 1;
    packetTypeFormat.indexConst = indexConstant;
    packetTypeFormat.indexMax = 0x500;
    packetTypeFormat.bitFieldArr[1].byteOffset = 4;
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;
    packetTypeFormat.bitFieldArr[0].byteOffset = 5;
    packetTypeFormat.bitFieldArr[0].startBit = 0;
    packetTypeFormat.bitFieldArr[0].numBits = 8;

    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        PRV_TGF_FROM_PORT0_PACKET_TYPE,
        &packetTypeFormat);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        PRV_TGF_FROM_PORT5_PACKET_TYPE,
        &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* AUTODOC: packets in the test will always select entry 0x305 of
       destination port table */
    entryIndex = 0x305;
    st = cpssPxIngressPortMapEntryGet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        entryIndex, &prvTgfRestore.dstMapEntry, &bypassLagDesignatedBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* AUTODOC: select ports of the LAG group */
    st = cpssPxIngressPortMapEntrySet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        entryIndex,
        (1 << LAG_PORT1_NUM) | (1 << LAG_PORT2_NUM) |
        (1 << LAG_PORT3_NUM) | (1 << LAG_PORT4_NUM), GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressHashLAGRestore function
* @endinternal
*
* @brief   Configure LAG
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashLAGRestore
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS                                           st;
    GT_U32                                              entryIndex;

    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,
        PRV_TGF_FROM_PORT0_PACKET_TYPE, prvTgfRestore.enableKeyEntry0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,
        PRV_TGF_FROM_PORT5_PACKET_TYPE, prvTgfRestore.enableKeyEntry5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
        PRV_TGF_FROM_PORT0_PACKET_TYPE, &prvTgfRestore.keyDataPort0,
        &prvTgfRestore.keyMaskPort0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
        PRV_TGF_FROM_PORT5_PACKET_TYPE, &prvTgfRestore.keyDataPort5,
        &prvTgfRestore.keyMaskPort5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        PRV_TGF_FROM_PORT0_PACKET_TYPE,
        &prvTgfRestore.packetTypeFormatDstPort0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        PRV_TGF_FROM_PORT5_PACKET_TYPE,
        &prvTgfRestore.packetTypeFormatDstPort5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Set packet type destination port map entry to cascade port  */
    entryIndex = 0x305;
    st = cpssPxIngressPortMapEntrySet(devNum,
        CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
        entryIndex, prvTgfRestore.dstMapEntry, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressHashPrngIndexingConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashPrngIndexing
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashPrngIndexingConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                   st;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexModeRestore;

    if(GT_TRUE == config)
    {
        /* AUTODOC: Fill designated table with uniform port distribution */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfUniformPortEntries),
            prvTgfUniformPortEntries);

        /* AUTODOC: LAG group of ports: 1,2,3,4 */
        prvTgfPxIngressHashLAGConfig(dev);

        /* AUTODOC: Configure random indexing */
        st = cpssPxIngressHashPortIndexModeGet(dev, SEND_PORT_NUM,
            &indexModeRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E);
    }
    else
    {
        /* AUTODOC: Restore */
        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            indexModeRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            indexModeRestore);

        prvTgfPxIngressHashLAGRestore(dev);

        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry);
    }
}

/**
* @internal prvTgfPxIngressHashPrngIndexingExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashPrngIndexing for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashPrngIndexingExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      egrCounters[4];
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC egrPortsArr[] =
        {{ LAG_PORT1_NUM, 0, NULL }, { LAG_PORT2_NUM, 0, NULL },
    { LAG_PORT3_NUM, 0, NULL }, { LAG_PORT4_NUM, 0, NULL }};

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    /* AUTODOC: Send packets */
    for(ii = 0; ii < PACKET_NUM; ii++)
    {
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
            SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
            PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        prvTgfPxIncrementLAGCounters(dev, egrCounters);
    }

    /* AUTODOC: Expect some amount of egressed packets on each LAG port */
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[3]);
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashPrngIndexing)
{
    GT_U8       dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_FALSE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        prvTgfPxIngressHashPrngIndexingConfig(dev, GT_TRUE);

        /* AUTODOC: Execute the test */
        prvTgfPxIngressHashPrngIndexingExecute(dev);

        /* AUTODOC: Restore configuration */
        prvTgfPxIngressHashPrngIndexingConfig(dev, GT_FALSE);
    }

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/**
* @internal prvTgfPxIngressHashCrc32IndexingConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashCrc32Indexing
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashCrc32IndexingConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                   st;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexModeRestore;
    static CPSS_PX_INGRESS_HASH_MODE_ENT               hashModeRestore;
    static GT_U32 bitOffsetsRestore[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    static CPSS_PX_INGRESS_HASH_UDBP_STC
        udbpRestore[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    static GT_BOOL sourcePortEnableRestore;

    if(GT_TRUE == config)
    {
        /* AUTODOC: Fill designated table with uniform port distribution */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfUniformPortEntries),
            prvTgfUniformPortEntries);

        /* AUTODOC: LAG group of ports: 1,2,3,4 */
        prvTgfPxIngressHashLAGConfig(dev);

        /* AUTODOC: Configure CRC32 indexing */
        st = cpssPxIngressHashPortIndexModeGet(dev, SEND_PORT_NUM,
            &indexModeRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);

        st = cpssPxIngressHashPacketTypeEntryGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore,
            &sourcePortEnableRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, prvTgfUdbps, GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, GT_FALSE);

        st = cpssPxIngressHashPacketTypeHashModeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            &hashModeRestore, bitOffsetsRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_CRC32_E, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_CRC32_E);
    }
    else
    {
        /* AUTODOC: Restore */
        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            indexModeRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            indexModeRestore);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            hashModeRestore, bitOffsetsRestore);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
                CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
                hashModeRestore);
        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore,
            sourcePortEnableRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        prvTgfPxIngressHashLAGRestore(dev);

        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry);
    }
}

/**
* @internal prvTgfPxIngressHashCrc32IndexingExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashCrc32Indexing for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashCrc32IndexingExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS   st;
    GT_U32      ii, kk;
    GT_U32      egrCounters[2][4];
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC egrPortsArr[] =
        {{ LAG_PORT1_NUM, 0, NULL }, { LAG_PORT2_NUM, 0, NULL },
         { LAG_PORT3_NUM, 0, NULL }, { LAG_PORT4_NUM, 0, NULL }};
    GT_U32      seeds[] = {0xFEEDBABE, 0xDEADBEEF};
    GT_U8       originalByte;

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    /* AUTODOC: Two different seeds must produce egress on two different ports*/
    for(kk = 0; kk < 2; kk++)
    {
        st = cpssPxIngressHashSeedSet(dev, seeds[kk]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, seeds[kk]);

        for(ii = 0; ii < PACKET_NUM / 2; ii++)
        {
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
                SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
                PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            prvTgfPxIncrementLAGCounters(dev, egrCounters[kk]);
        }
    }

    /* AUTODOC: Expect that two different seeds must produce egress on two
       different ports*/
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[0][3]);

    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[1][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][3]);

    /* AUTODOC: Do the test with payload change */
    originalByte = prvTgfFrame1Arr[14];
    prvTgfFrame1Arr[14] = 0x00;
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    for(kk = 0; kk < 2; kk++)
    {
        for(ii = 0; ii < PACKET_NUM / 2; ii++)
        {
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
                SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
                PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            prvTgfPxIncrementLAGCounters(dev, egrCounters[kk]);
        }
        /* Change byte 14 in payload */
        prvTgfFrame1Arr[14] = 0xAA;
    }
    prvTgfFrame1Arr[14] = originalByte;
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[0][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][3]);

    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[1][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][3]);
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashCrc32Indexing)
{
    GT_U8       dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        prvTgfPxIngressHashCrc32IndexingConfig(dev, GT_TRUE);

        /* AUTODOC: Execute the test */
        prvTgfPxIngressHashCrc32IndexingExecute(dev);

        /* AUTODOC: Restore configuration */
        prvTgfPxIngressHashCrc32IndexingConfig(dev, GT_FALSE);
    }

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/**
* @internal prvTgfPxIngressHashSingleDesignatedConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashSingleDesignated
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashSingleDesignatedConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                   st;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexModeRestore;

    if(GT_TRUE == config)
    {
        /* AUTODOC: Fill designated table with uniform port distribution */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfUniformPortEntries),
            prvTgfUniformPortEntries);

        /* AUTODOC: LAG group of ports: 1,2,3,4 */
        prvTgfPxIngressHashLAGConfig(dev);

        /* AUTODOC: Configure random indexing */
        st = cpssPxIngressHashPortIndexModeGet(dev, SEND_PORT_NUM,
            &indexModeRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E);
    }
    else
    {
        /* AUTODOC: Restore */
        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            indexModeRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            indexModeRestore);

        prvTgfPxIngressHashLAGRestore(dev);

        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry);
    }
}

/**
* @internal prvTgfPxIngressHashSingleDesignatedExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashSingleDesignated for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashSingleDesignatedExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      egrCounters[4];
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC egrPortsArr[] =
        {{ LAG_PORT1_NUM, 0, NULL }, { LAG_PORT2_NUM, 0, NULL },
         { LAG_PORT3_NUM, 0, NULL }, { LAG_PORT4_NUM, 0, NULL }};
    CPSS_PX_PORTS_BMP portEntries[] = {0x1FE3};

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    /* AUTODOC: Send packets */
    for(ii = 0; ii < PACKET_NUM; ii++)
    {
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
            SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
            PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        prvTgfPxIncrementLAGCounters(dev, egrCounters);
    }

    /* AUTODOC: Expect that some amount of packets egressed from each
       LAG port */
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, egrCounters[3]);

    /* AUTODOC: Perform configuration. Every entry in designated port table
       points to port 1. Entries selected randomly */
    prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
        PRV_TGF_MEMBERS_NUM_MAC(portEntries), portEntries);

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    for(ii = 0; ii < PACKET_NUM; ii++)
    {
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
            SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
            PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        prvTgfPxIncrementLAGCounters(dev, egrCounters);
    }

    /* AUTODOC: Expect that the traffic egressed from a single LAG port --
       port1 */
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3]);
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashSingleDesignated)
{
    GT_U8       dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_FALSE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Perform configuration. Designated ports distributed
           uniformly across entries. Entries selected randomly */
        prvTgfPxIngressHashSingleDesignatedConfig(dev, GT_TRUE);

        /* AUTODOC: Execute the test */
        prvTgfPxIngressHashSingleDesignatedExecute(dev);

        /* AUTODOC: Restore configuration */
        prvTgfPxIngressHashSingleDesignatedConfig(dev, GT_FALSE);

        tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
    }
}

/**
* @internal prvTgfPxIngressHashSelectedBitsConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashSelectedBits
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashSelectedBitsConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                   st;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexModeRestore;
    static CPSS_PX_INGRESS_HASH_MODE_ENT               hashModeRestore;
    static CPSS_PX_INGRESS_HASH_MODE_ENT               hashModeIpv4Restore;
    static GT_U32  bitOffsetsRestore[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    static GT_U32  bitOffsetsIpv4Restore[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    static CPSS_PX_INGRESS_HASH_UDBP_STC udbpRestore[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    static CPSS_PX_INGRESS_HASH_UDBP_STC udbpIpv4Restore[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    
    static GT_BOOL sourcePortEnableRestore;
    static GT_BOOL sourcePortEnableIpv4Restore;
    GT_U32  offsets[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] =
        {0, 0, 0, 0, 0, 0, 0, 0}; /* Hash consists of zeros */
    GT_U32  offsetsIpv4[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] =
        {0, 1, 2, 3, 4, 5, 6, 17};

    if(GT_TRUE == config)
    {
        /* AUTODOC: Fill designated table with uniform port distribution */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfUniformPortEntries),
            prvTgfUniformPortEntries);

        /* AUTODOC: LAG group of ports: 1,2,3,4 */
        prvTgfPxIngressHashLAGConfig(dev);

        /* AUTODOC: Configure selected bits hash mode. */
        st = cpssPxIngressHashPortIndexModeGet(dev, SEND_PORT_NUM,
            &indexModeRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);

        st = cpssPxIngressHashPacketTypeEntryGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore,
            &sourcePortEnableRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, prvTgfUdbps, GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, GT_FALSE);

        st = cpssPxIngressHashPacketTypeEntryGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, udbpIpv4Restore,
            &sourcePortEnableIpv4Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E);

        st = cpssPxIngressHashPacketTypeHashModeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, &hashModeRestore,
            bitOffsetsRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E,
            offsets);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E);

        st = cpssPxIngressHashPacketTypeHashModeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,
            &hashModeIpv4Restore,
            bitOffsetsIpv4Restore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E,
            offsetsIpv4);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E);
    }
    else
    {
        /* AUTODOC: Restore */
        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            indexModeRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            indexModeRestore);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, udbpIpv4Restore,
            sourcePortEnableIpv4Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,
            sourcePortEnableIpv4Restore);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore,
            sourcePortEnableRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, sourcePortEnableRestore);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, hashModeRestore,
            bitOffsetsRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            hashModeRestore);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,
            hashModeIpv4Restore, bitOffsetsIpv4Restore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,
            hashModeIpv4Restore);

        prvTgfPxIngressHashLAGRestore(dev);
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry);
    }
}

/**
* @internal prvTgfPxIngressHashSelectedBitsExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashSelectedBits for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashSelectedBitsExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      egrCounters[4];
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC egrPortsArr[] =
        {{ LAG_PORT1_NUM, 0, NULL }, { LAG_PORT2_NUM, 0, NULL },
         { LAG_PORT3_NUM, 0, NULL }, { LAG_PORT4_NUM, 0, NULL }};
    CPSS_PX_INGRESS_HASH_UDBP_STC restoreVal;

    /* Test list:
        1. Send multiple packets and expect that selected port number will be
           the same because of the same selected bits.
        2. Check if anchors in UDBP config are working. Different anchor means
           different offseth within frame. Expect different egress ports
           selected.
        3. Check if offset in UDBP config is working. Different offset means
           different data in hash key. Different data means different egress
           port.
        4. Check if mask in UDBP config is workong. Different mask means
           different data extracted from selected bytepair. This means different
           egress port.
    */

    /* AUTODOC: Send packets. Constant egress port */
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    for(ii = 0; ii < PACKET_NUM; ii++)
    {
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
            SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
            PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        prvTgfPxIncrementLAGCounters(dev, egrCounters);
    }

    /* AUTODOC: Expect first entry selected in LAG everytime  */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM, egrCounters[3]);

    /* AUTODOC: Etherype = IPv4 */
    prvTgfFrame1Arr[12] = 0x08;
    prvTgfFrame1Arr[13] = 0x00;

    /* AUTODOC: Backup UDBPs*/
    restoreVal = prvTgfUdbps[0];

    /* AUTODOC: Check UDBP Anchor parameter. Output ports must be different */
    if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
    {
        /* AUTODOC: Set 0th UDBP, to be with offset 63 from L2 */
        prvTgfUdbps[0] = prvTgfUdbpsA1[0];
    }
    else
    {
        /* AUTODOC: Set 0th UDBP, to be with offset 5 from L2 */
        prvTgfUdbps[0] = prvTgfUdbps[11];
    }
    st = cpssPxIngressHashPacketTypeEntrySet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, prvTgfUdbps,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, GT_FALSE);

    /* AUTODOC: Send packets */
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
        SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
        PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    prvTgfPxIncrementLAGCounters(dev, egrCounters);

    /* AUTODOC: Expect first entry selected in LAG everytime  */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(1, egrCounters[3]);

    if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
    {
        /* AUTODOC: Set 0th UDBP, to be with offset 63 from L3 */
        prvTgfUdbps[0] = prvTgfUdbpsA1[1];
    }
    else
    {
        /* AUTODOC: Set 0th UDBP, to be with offset 5 from L3 */
        prvTgfUdbps[0] = prvTgfUdbps[17];
    }
    st = cpssPxIngressHashPacketTypeEntrySet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, prvTgfUdbps,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, GT_FALSE);

    /* AUTODOC: Send packets */
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
        SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
        PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    prvTgfPxIncrementLAGCounters(dev, egrCounters);

    /* AUTODOC: Expect last entry selected in LAG everytime  */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(1, egrCounters[3]);

    /* AUTODOC: Check UDBP Offset parameter. Output ports must be different */
    /* AUTODOC: Set 0th UDBP, to be with offset 4 */
    prvTgfUdbps[0] = prvTgfUdbps[2];
    st = cpssPxIngressHashPacketTypeEntrySet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, prvTgfUdbps,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, GT_FALSE);

    /* AUTODOC: Send packets */
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
        SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
        PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    prvTgfPxIncrementLAGCounters(dev, egrCounters);

    /* AUTODOC: Expect last entry selected in LAG everytime  */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(1, egrCounters[3]);

    /* AUTODOC: Set 0th UDBP, to be with offset 5 and all parameters as
       previous */
    prvTgfUdbps[0] = prvTgfUdbps[11];
    st = cpssPxIngressHashPacketTypeEntrySet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, prvTgfUdbps,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, GT_FALSE);

    /* AUTODOC: Send packets */
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
        SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
        PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    prvTgfPxIncrementLAGCounters(dev, egrCounters);

    /* AUTODOC: Expect first entry selected in LAG everytime  */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(1, egrCounters[3]);

    /* AUTODOC: Check UDBP Mask parameter. Output ports must be different */
    /* AUTODOC: Set 0th UDBP, to be with offset 4 and Mask 1111 */
    prvTgfUdbps[0] = prvTgfUdbps[10];
    st = cpssPxIngressHashPacketTypeEntrySet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, prvTgfUdbps,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, GT_FALSE);

    /* AUTODOC: Send packets */
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
        SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
        PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    prvTgfPxIncrementLAGCounters(dev, egrCounters);

    /* AUTODOC: Expect last entry selected in LAG everytime  */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(1, egrCounters[3]);

    /* AUTODOC: Set 0th UDBP, to be with offset 4 and Mask 0000 */
    prvTgfUdbps[0] = prvTgfUdbps[18];
    st = cpssPxIngressHashPacketTypeEntrySet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, prvTgfUdbps,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E, GT_FALSE);

    /* AUTODOC: Send packets */
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
        SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
        PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    prvTgfPxIncrementLAGCounters(dev, egrCounters);

    /* AUTODOC: Expect first entry selected in LAG everytime  */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(1, egrCounters[3]);

    /* AUTODOC: Restore UDBPs*/
    prvTgfUdbps[0] = restoreVal;

    /* AUTODOC: Restore Etherype */
    prvTgfFrame1Arr[12] = 0x55;
    prvTgfFrame1Arr[13] = 0x55;
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashSelectedBits)
{
    GT_U8       dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        prvTgfPxIngressHashSelectedBitsConfig(dev, GT_TRUE);

        /* AUTODOC: Execute the test */
        prvTgfPxIngressHashSelectedBitsExecute(dev);

        /* AUTODOC: Restore */
        prvTgfPxIngressHashSelectedBitsConfig(dev, GT_FALSE);
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/**
* @internal prvTgfPxIngressHashKeyPortNumberConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashKeyPortNumber
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashKeyPortNumberConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                   st;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT indexModeRestore0;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT indexModeRestore5;
    static CPSS_PX_INGRESS_HASH_MODE_ENT            hashModeRestore;
    static GT_U32 bitOffsetsRestore[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    static CPSS_PX_INGRESS_HASH_UDBP_STC udbpRestore[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    static GT_BOOL sourcePortEnableRestore;
    GT_U32      offsets[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] =
        {339, 338, 337, 336, 0, 0, 0, 0}; /* Hash depends on src port */
    static GT_BOOL udeValid;
    static GT_U16 udeEthertype;

    if(GT_TRUE == config)
    {
        /* AUTODOC: Fill designated table with uniform port distribution */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfUniformPortEntries),
            prvTgfUniformPortEntries);

        /* AUTODOC: LAG group of ports: 1,2,3,4 */
        prvTgfPxIngressHashLAGConfig(dev);

        /* AUTODOC: Set up hash indexing for ports 0 and 5 */
        st = cpssPxIngressHashPortIndexModeGet(dev, SEND_PORT_NUM,
            &indexModeRestore0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);

        st = cpssPxIngressHashPortIndexModeGet(dev, SEND_PORT_NUM,
            &indexModeRestore5);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM_ANOTHER,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM_ANOTHER,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);

        /* AUTODOC: Set up hash key configuration */
        st = cpssPxIngressHashPacketTypeEntryGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore,
            &sourcePortEnableRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, prvTgfUdbps, GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, GT_FALSE);

        /* AUTODOC: Configure selected bits */
        st = cpssPxIngressHashPacketTypeHashModeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, &hashModeRestore,
            bitOffsetsRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E,
            offsets);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E);

        st = cpssPxIngressHashUdeEthertypeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, &udeEthertype, &udeValid);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E);
    }
    else
    {
        /* AUTODOC: Restore */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfUniformPortEntries),
            prvTgfUniformPortEntries);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            indexModeRestore0);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            indexModeRestore0);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            indexModeRestore5);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM_ANOTHER,
            indexModeRestore5);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore,
            sourcePortEnableRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            sourcePortEnableRestore);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, hashModeRestore,
            bitOffsetsRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, hashModeRestore);

        st = cpssPxIngressHashUdeEthertypeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, udeEthertype, udeValid);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, udeEthertype, udeValid);

        prvTgfPxIngressHashLAGRestore(dev);

        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry);
    }
}

/**
* @internal prvTgfPxIngressHashKeyPortNumberExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashKeyPortNumber for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashKeyPortNumberExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS                           st;
    GT_U32                              ii, jj;
    GT_U32                              egrCounters[4];
    CPSS_PX_PORT_MAC_COUNTERS_STC       portCounters;
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC egrPortsArr[] =
        {{ LAG_PORT1_NUM, 0, NULL }, { LAG_PORT2_NUM, 0, NULL },
         { LAG_PORT3_NUM, 0, NULL }, { LAG_PORT4_NUM, 0, NULL }};

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    for(ii = 0; ii < PACKET_NUM / 2; ii++)
    {
        if (ii < PACKET_NUM / 4)
        {
            /* AUTODOC: Send packets from port 0 */
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
                SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
                PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            /* AUTODOC: Send packets from port 5 */
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
                SEND_PORT_NUM_ANOTHER, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
                PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* AUTODOC: Update counters */
        for(jj = 0; jj < 12; jj++)
        {
            st = cpssPxPortMacCountersOnPortGet(dev, jj, &portCounters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, jj);
            if((jj >= LAG_PORT1_NUM) && (jj <= LAG_PORT4_NUM))
            {
                egrCounters[jj - 1] += portCounters.mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0];
            }
            else if ((SEND_PORT_NUM_ANOTHER != jj) && (SEND_PORT_NUM != jj))
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(0, portCounters.mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0]);
            }
        }
    }

    /* AUTODOC: Expect that all packets will egress from a single LAG port */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[3]);

    /* AUTODOC: Configure hash key with ingress port */
    st = cpssPxIngressHashPacketTypeEntrySet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, prvTgfUdbps, GT_TRUE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, GT_TRUE);

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    for(ii = 0; ii < PACKET_NUM / 2; ii++)
    {
        if (ii < PACKET_NUM / 4)
        {
            /* AUTODOC: Send packets from port 0 */
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
                SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
                PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            /* AUTODOC: Send packets from port 5 */
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
                SEND_PORT_NUM_ANOTHER, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
                PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* AUTODOC: Update counters */
        for(jj = 0; jj < 12; jj++)
        {
            st = cpssPxPortMacCountersOnPortGet(dev, jj, &portCounters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, jj);
            if((jj >= LAG_PORT1_NUM) && (jj <= LAG_PORT4_NUM))
            {
                egrCounters[jj - 1] += portCounters.mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0];
            }
            else if ((SEND_PORT_NUM_ANOTHER != jj) && (SEND_PORT_NUM != jj))
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(0, portCounters.mibCounter[CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E].l[0]);
            }
        }
    }

    /* AUTODOC: Expect that packets will egress from two different LAG
       members depending on ingress port */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 4, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 4, egrCounters[3]);
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashKeyPortNumber)
{
    GT_U8       dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure. Hash key without ingress port */
        prvTgfPxIngressHashKeyPortNumberConfig(dev, GT_TRUE);

        /* AUTODOC: Execute the test */
        prvTgfPxIngressHashKeyPortNumberExecute(dev);

        /* AUTODOC: Restore */
        prvTgfPxIngressHashKeyPortNumberConfig(dev, GT_FALSE);
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/**
* @internal prvTgfPxIngressHashUdeConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashUde
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashUdeConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                       st;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT indexModeRestore;
    static CPSS_PX_INGRESS_HASH_MODE_ENT            hashModeEthRestore;
    static CPSS_PX_INGRESS_HASH_MODE_ENT            hashModeUdeRestore;
    static GT_U32 bitOffsetsEthRestore[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    static GT_U32 bitOffsetsUdeRestore[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    static CPSS_PX_INGRESS_HASH_UDBP_STC udbpEthRestore[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    static GT_BOOL sourcePortEnableEthRestore;
    static CPSS_PX_INGRESS_HASH_UDBP_STC udbpUdeRestore[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    static GT_BOOL sourcePortEnableUdeRestore;
    GT_U32      offsetsEth[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] =
        {0, 0, 0, 0, 0, 0, 0, 0};
    GT_U32      offsetsUde[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] =
        {103, 102, 101, 100, 99, 98, 97, 96};

    if(GT_TRUE == config)
    {
        /* AUTODOC: Fill designated table with uniform port distribution */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfUniformPortEntries),
            prvTgfUniformPortEntries);

        /* AUTODOC: LAG group of ports: 1,2,3,4 */
        prvTgfPxIngressHashLAGConfig(dev);

        /* AUTODOC: Configure selected-bits hash for packet types UDE1 and
           Ethernet. Selected bits for them are different */
        st = cpssPxIngressHashPortIndexModeGet(dev, SEND_PORT_NUM,
            &indexModeRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM);

        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);

        st = cpssPxIngressHashPacketTypeHashModeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, &hashModeEthRestore,
            bitOffsetsEthRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E, offsetsEth);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E);

        st = cpssPxIngressHashPacketTypeHashModeGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, &hashModeUdeRestore,
            bitOffsetsUdeRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E, offsetsUde);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E,
            CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E);

        /* AUTODOC: Configure the same hash key configurations for UDE1 and
           Ethernet packet types */
        st = cpssPxIngressHashPacketTypeEntryGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpEthRestore,
            &sourcePortEnableEthRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, prvTgfUdbps, GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, GT_FALSE);

        st = cpssPxIngressHashPacketTypeEntryGet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, udbpUdeRestore,
            &sourcePortEnableUdeRestore);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, prvTgfUdbps, GT_FALSE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, GT_FALSE);
    }
    else
    {
        /* AUTODOC: Restore */
        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM,
            indexModeRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM,
            indexModeRestore);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, hashModeEthRestore,
            bitOffsetsEthRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, hashModeEthRestore);

        st = cpssPxIngressHashPacketTypeHashModeSet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, hashModeUdeRestore,
            bitOffsetsUdeRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, hashModeUdeRestore);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpEthRestore,
            sourcePortEnableEthRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,
            sourcePortEnableEthRestore);

        st = cpssPxIngressHashPacketTypeEntrySet(dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, udbpUdeRestore,
            sourcePortEnableUdeRestore);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,
            CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E,
            sourcePortEnableUdeRestore);

        prvTgfPxIngressHashLAGRestore(dev);

        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry);
    }
}

/**
* @internal prvTgfPxIngressHashUdeExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashUde for given device
*
* @param[in] dev                      - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashUdeExecute
(
    IN  GT_SW_DEV_NUM   dev
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      egrCounters[4];
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC egrPortsArr[] =
        {{ LAG_PORT1_NUM, 0, NULL }, { LAG_PORT2_NUM, 0, NULL },
         { LAG_PORT3_NUM, 0, NULL }, { LAG_PORT4_NUM, 0, NULL }};

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    /* AUTODOC: Send packets without configured UDE1 */
    for(ii = 0; ii < PACKET_NUM / 2; ii++)
    {
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
            SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
            PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        prvTgfPxIncrementLAGCounters(dev, egrCounters);
    }

    /* AUTODOC: Expect packets will egress on port 4 */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[3]);

    /* AUTODOC: Configure UDE1 */
    st = cpssPxIngressHashUdeEthertypeSet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, 0x5555, GT_TRUE);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, 0x5555, GT_TRUE);

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));

    /* AUTODOC: Send packets with configured UDE1 */
    for(ii = 0; ii < PACKET_NUM / 2; ii++)
    {
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimple(dev,
            SEND_PORT_NUM, prvTgfFrame1Arr, sizeof(prvTgfFrame1Arr),
            PRV_TGF_MEMBERS_NUM_MAC(egrPortsArr), egrPortsArr);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        prvTgfPxIncrementLAGCounters(dev, egrCounters);
    }

    /* AUTODOC: Expect packets will egress on port 2. Because we have
       different configuration of hash bits for given ethertype */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3]);


    /* AUTODOC: unset Configure UDE1 */
    st = cpssPxIngressHashUdeEthertypeSet(dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, 0x0000, GT_FALSE);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
        CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E, 0x0000, GT_FALSE);

}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashUde)
{
    GT_U8       dev = 0;

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_TRUE);

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        prvTgfPxIngressHashUdeConfig(dev, GT_TRUE);

        /* AUTODOC: Execute */
        prvTgfPxIngressHashUdeExecute(dev);

        /* AUTODOC: Restore */
        prvTgfPxIngressHashUdeConfig(dev, GT_FALSE);
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/**
* @internal prvTgfPxIngressUplinkPortDownUcConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream single destination packets
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownUcConfig
(
    IN GT_SW_DEV_NUM    devNum
)
{
    GT_STATUS st;
    GT_U32 entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;

    /* Enable packet type key for MC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, PRV_TGF_DOWNSTREAM_UC_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = SEND_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = PRV_TGF_ETAG_TPID;
    keyMask.etherType = 0xFFFF;
    keyData.udbPairsArr[0].udb[0] = 0x00;
    keyMask.udbPairsArr[0].udb[0] = 0xF0;

    /* Set packet type key for UC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            PRV_TGF_DOWNSTREAM_UC_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* UC packet type destination port map */
    packetTypeFormat.indexConst = 0;
    packetTypeFormat.indexMax = _4K - 1;

    packetTypeFormat.bitFieldArr[2].byteOffset = 16;
    packetTypeFormat.bitFieldArr[2].startBit = 4;
    packetTypeFormat.bitFieldArr[2].numBits = 2;    /* 0x0 */
    packetTypeFormat.bitFieldArr[1].byteOffset = 16;
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;    /* 0x0 */
    packetTypeFormat.bitFieldArr[0].byteOffset = 17;
    packetTypeFormat.bitFieldArr[0].startBit = 0;
    packetTypeFormat.bitFieldArr[0].numBits = 8;    /* 0x11/0x12 */

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            PRV_TGF_DOWNSTREAM_UC_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = 0x11;

    PRV_TGF_LOG1_MAC("DST entryIndex = 0x%x \n",entryIndex);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            (1 << LAG_PORT1_NUM), GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = 0x12;

    PRV_TGF_LOG1_MAC("DST entryIndex = 0x%x \n",entryIndex);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            (1 << LAG_PORT2_NUM), GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Set LAG table number for UC packet type */
    st = cpssPxIngressHashPacketTypeLagTableNumberSet(devNum, 
                                                      PRV_TGF_DOWNSTREAM_UC_PACKET_TYPE, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
}

/**
* @internal prvTgfPxIngressUplinkPortDownMcConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream multi destination packets
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownMcConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    GT_U32 entryIndex;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     packetTypeFormat;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;

    /* Enable packet type key for MC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, PRV_TGF_DOWNSTREAM_MC_PACKET_TYPE, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    keyData.profileIndex = SEND_PORT_NUM;
    keyMask.profileIndex = 0x7F;
    keyData.etherType = PRV_TGF_ETAG_TPID;
    keyMask.etherType = 0xFFFF;

    /* Set packet type key for MC downstream traffic */
    st = cpssPxIngressPacketTypeKeyEntrySet(devNum,
                                            PRV_TGF_DOWNSTREAM_MC_PACKET_TYPE,
                                            &keyData, &keyMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* MC packet type destination port map */
    packetTypeFormat.indexConst = 0;
    packetTypeFormat.indexMax = _8K - 1;

    packetTypeFormat.bitFieldArr[2].byteOffset = 16;
    packetTypeFormat.bitFieldArr[2].startBit = 4;
    packetTypeFormat.bitFieldArr[2].numBits = 2;    /* 0x1 */
    packetTypeFormat.bitFieldArr[1].byteOffset = 16;
    packetTypeFormat.bitFieldArr[1].startBit = 0;
    packetTypeFormat.bitFieldArr[1].numBits = 4;    /* 0x0 */
    packetTypeFormat.bitFieldArr[0].byteOffset = 17;
    packetTypeFormat.bitFieldArr[0].startBit = 0;
    packetTypeFormat.bitFieldArr[0].numBits = 8;    /* 0x1 */

    /* Set packet type destination format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            PRV_TGF_DOWNSTREAM_MC_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = 0x1001;

    PRV_TGF_LOG1_MAC("DST entryIndex = 0x%x \n",entryIndex);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            (1 << LAG_PORT1_NUM) | (1 << LAG_PORT2_NUM), GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = 0x1002;

    PRV_TGF_LOG1_MAC("DST entryIndex = 0x%x \n",entryIndex);

    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                            entryIndex,
                                            (1 << LAG_PORT3_NUM), GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* MC packet type source port map */
    packetTypeFormat.indexConst = 0x123;
    packetTypeFormat.indexMax = _4K - 1;

    /* Set packet type source format entry for MC downstream traffic */
    st = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                            PRV_TGF_DOWNSTREAM_MC_PACKET_TYPE,
                                            &packetTypeFormat);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    entryIndex = 0x123;

    PRV_TGF_LOG1_MAC("SRC entryIndex = 0x%x \n",entryIndex);
    st = cpssPxIngressPortMapEntrySet(devNum,
                                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                            entryIndex,
                                            (~(1 << SEND_PORT_NUM) & 0x1FFFF), GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* Set LAG table number for MC packet type */
    st = cpssPxIngressHashPacketTypeLagTableNumberSet(devNum, 
                                                      PRV_TGF_DOWNSTREAM_MC_PACKET_TYPE, 1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

}

/**
* @internal prvTgfPxIngressUplinkPortDownStreamConfig function
* @endinternal
*
* @brief   Up-link port configuration for downstream traffic
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressUplinkPortDownStreamConfig
(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS st;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    /* Set profile for uplink port */
    portKey.srcPortProfile = SEND_PORT_NUM;
    /* Set UDBP0 */
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    /* E-Tag <GRP>/<E-CID>*/
    portKey.portUdbPairArr[0].udbByteOffset = 16;

    st = cpssPxIngressPortPacketTypeKeySet(devNum, SEND_PORT_NUM, &portKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    /* AUTODOC: Set UC downstream configuration */
    prvTgfPxIngressUplinkPortDownUcConfig(devNum);
    /* AUTODOC: Set MC downstream configuration */
    prvTgfPxIngressUplinkPortDownMcConfig(devNum);
}

/**
* @internal prvTgfPxIngressHashConfig function
* @endinternal
*
* @brief   Save/restore ingress hash configurations
*
* @param[in] devNum                   - the device number
* @param[in] saveFlag                 - GT_TRUE   - save configurations
*                                      GT_FALSE  - restore configurations
*                                       None
*/
static GT_VOID prvTgfPxIngressHashConfig
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          saveFlag
)
{
    GT_STATUS st;
    static CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT    indexModeRestore;
    static CPSS_PX_INGRESS_HASH_MODE_ENT               hashModeRestore;
    static GT_U32 bitOffsetsRestore[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    static CPSS_PX_INGRESS_HASH_UDBP_STC
        udbpRestore[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    static GT_BOOL sourcePortEnableRestore;
    static CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT     lagTableModeRestore;

    if (saveFlag == GT_TRUE)
    {
        prvTgfPxIngressSaveConfig(devNum, GT_TRUE);

        st = cpssPxIngressHashPortIndexModeGet(devNum, SEND_PORT_NUM, 
                                               &indexModeRestore); 
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, SEND_PORT_NUM); 


        st = cpssPxIngressHashPacketTypeEntryGet(devNum, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore, 
                                    &sourcePortEnableRestore); 
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E); 


        st = cpssPxIngressHashPacketTypeHashModeGet(devNum, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 
                                    &hashModeRestore, bitOffsetsRestore); 
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E); 

        st = cpssPxIngressHashLagTableModeGet(devNum, &lagTableModeRestore);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum); 
    }
    else
    {
        st = cpssPxIngressHashLagTableModeSet(devNum, lagTableModeRestore);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum); 

        st = cpssPxIngressHashPacketTypeHashModeSet(devNum,
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 
                                    hashModeRestore, bitOffsetsRestore); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 
                                    hashModeRestore); 
        st = cpssPxIngressHashPacketTypeEntrySet(devNum, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, udbpRestore, 
                                    sourcePortEnableRestore); 
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E); 

        st = cpssPxIngressHashPortIndexModeSet(devNum, SEND_PORT_NUM, 
                                    indexModeRestore); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, SEND_PORT_NUM, 
                                    indexModeRestore); 

        prvTgfPxIngressRestoreConfig(devNum, GT_TRUE);
    }
}

/**
* @internal prvTgfPxIngressHashLagConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashLagUc/prvTgfPxIngressHashLagMc
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashLagConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                   st;
    static CPSS_PX_PORTS_BMP prvTgfPortUcEntries[] =
        {0x1FE5, 0x1FE3};
    static CPSS_PX_PORTS_BMP prvTgfPortMcEntries[] =
        {0x11FE1, 0x11FE1, 0x1FE5, 0x1FE3};

    if(GT_TRUE == config)
    {
        /* Save ingress hash configurations */
        prvTgfPxIngressHashConfig(dev, GT_TRUE);

        /* AUTODOC: Fill designated table with uniform port distribution for UC traffic */
        prvTgfPxIngressHashDesignatedPortTableUniformFill_A1(dev, GT_TRUE,
            prvTgfPortUcEntries, PRV_TGF_MEMBERS_NUM_MAC(prvTgfPortUcEntries));

        /* AUTODOC: Fill designated table with uniform port distribution for MC traffic */
        prvTgfPxIngressHashDesignatedPortTableUniformFill_A1(dev, GT_FALSE,
            prvTgfPortMcEntries, PRV_TGF_MEMBERS_NUM_MAC(prvTgfPortMcEntries));

        /* AUTODOC: Setup common ingress configuration for downstream traffic */
        prvTgfPxIngressUplinkPortDownStreamConfig(dev);

        /* AUTODOC: Configure CRC32 indexing */
        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM, 
                                               CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM, 
                                    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E); 
        
        st = cpssPxIngressHashPacketTypeEntrySet(dev, 
                                                 CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, prvTgfUdbps, GT_FALSE); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, GT_FALSE); 
        
        st = cpssPxIngressHashPacketTypeHashModeSet(dev, 
                                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 
                                                    CPSS_PX_INGRESS_HASH_MODE_CRC32_E, NULL); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 
                                    CPSS_PX_INGRESS_HASH_MODE_CRC32_E); 
    }
    else
    {
        /* AUTODOC: Restore configuration */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev, 
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry); 

        /* Restore ingress hash configurations */
        prvTgfPxIngressHashConfig(dev, GT_FALSE);
    }
}

/**
* @internal prvTgfPxIngressHashLagLegacyConfig function
* @endinternal
*
* @brief   Configure test prvTgfPxIngressHashLagLegacy
*
* @param[in] dev                      - the device number
* @param[in] config                   - GT_TRUE - configure
*                                      GT_FALSE - restore
*                                       None
*/
static GT_VOID prvTgfPxIngressHashLagLegacyConfig
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_BOOL         config
)
{
    GT_STATUS                                   st;

    static CPSS_PX_PORTS_BMP prvTgfPortEmptyLagEntries[] = {0x1FE1};

    if(GT_TRUE == config)
    {
        /* Save ingress hash configurations */
        prvTgfPxIngressHashConfig(dev, GT_TRUE);

        /* AUTODOC: Fill all entries[0..255] of designated table with no LAG ports */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev,
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfPortEmptyLagEntries), prvTgfPortEmptyLagEntries);

        /* AUTODOC: Setup common ingress configuration for downstream traffic */
        prvTgfPxIngressUplinkPortDownStreamConfig(dev);

        /* AUTODOC: Configure CRC32 indexing */
        st = cpssPxIngressHashPortIndexModeSet(dev, SEND_PORT_NUM, 
                                    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, SEND_PORT_NUM, 
                                    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E); 
        
        st = cpssPxIngressHashPacketTypeEntrySet(dev, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, prvTgfUdbps, GT_FALSE); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, GT_FALSE); 
        
        st = cpssPxIngressHashPacketTypeHashModeSet(dev, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 
                                    CPSS_PX_INGRESS_HASH_MODE_CRC32_E, NULL); 
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, 
                                    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E, 
                                    CPSS_PX_INGRESS_HASH_MODE_CRC32_E); 

        /* AUTODOC: Configure LAG table to single mode */
        st = cpssPxIngressHashLagTableModeSet(dev,
                                    CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, 
                                    CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E); 

    }
    else
    {
        /* AUTODOC: Restore configuration */
        prvTgfPxIngressHashDesignatedPortTableUniformFill(dev, 
            PRV_TGF_MEMBERS_NUM_MAC(prvTgfDfltPortEntry), prvTgfDfltPortEntry); 

        /* Restore ingress hash configurations */
        prvTgfPxIngressHashConfig(dev, GT_FALSE);
    }
}

#define E_PCP_CNS                   0
#define E_DEI_CNS                   0
#define DUMMY_INGRESS_E_CID_BASE    0x123
#define DUMMY_INGRESS_E_CID_GRP     0x1
#define DUMMY_INGRESS_ECID_1_CNS    0x1
#define DUMMY_INGRESS_ECID_11_CNS   0x11
#define DUMMY_INGRESS_ECID_12_CNS   0x12
#define DUMMY_INGRESS_E_CID_EXT     0xab
#define DUMMY_E_CID_EXT             0xcd

static GT_U8    egressEtagUc11[8] = {
/*byte 0*/    (GT_U8)(PRV_TGF_ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(PRV_TGF_ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (DUMMY_INGRESS_ECID_11_CNS >> 8)),
/*byte 5*/    (GT_U8)(DUMMY_INGRESS_ECID_11_CNS & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static GT_U8    egressEtagUc12[8] = {
/*byte 0*/    (GT_U8)(PRV_TGF_ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(PRV_TGF_ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (DUMMY_INGRESS_ECID_12_CNS >> 8)),
/*byte 5*/    (GT_U8)(DUMMY_INGRESS_ECID_12_CNS & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static GT_U8    egressEtagMc1001[8] = {
/*byte 0*/    (GT_U8)(PRV_TGF_ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(PRV_TGF_ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (DUMMY_INGRESS_E_CID_GRP/*GRP*/ << 4) |  (DUMMY_INGRESS_ECID_1_CNS >> 8)),
/*byte 5*/    (GT_U8)(DUMMY_INGRESS_ECID_1_CNS & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

/**
* @internal prvTgfPxIngressHashLagUcExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashLagUc for given device
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashLagUcExecute
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   st;
    GT_U32      egrCounters[4];
    GT_U32      numOfEgressPortsInfo;
    GT_U32      seeds[] = {0xFEED, 0xBABE, 0xDEAD, 0xBEEF};
    GT_U32      ii;

    static  TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC egressPortsArr[4];
    static TGF_BYTES_INFO_LIST_STC    ingressFrame;

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    cpssOsMemSet(egressPortsArr, 0, sizeof(egressPortsArr));
    cpssOsMemSet(&ingressFrame, 0, sizeof(ingressFrame));

    egressPortsArr[0].portNum = LAG_PORT1_NUM;

    /**************************/
    /* prepare ingress packet */
    /**************************/
    ingressFrame.numOfParts = 4;
    ingressFrame.partsInfo[0].numOfBytes   = 12;/*mac DA,SA*/
    ingressFrame.partsInfo[0].partBytesPtr = prvTgfFrame1Arr;
    ingressFrame.partsInfo[1].numOfBytes   = 8;   /*E-Tag*/
    ingressFrame.partsInfo[1].partBytesPtr = egressEtagUc11;
    ingressFrame.partsInfo[2].numOfBytes   = 0;   /*skip*/
    ingressFrame.partsInfo[2].partBytesPtr = NULL;/*skip*/
    ingressFrame.partsInfo[3].numOfBytes   = sizeof(prvTgfFrame1Arr) - ingressFrame.partsInfo[0].numOfBytes;
    ingressFrame.partsInfo[3].partBytesPtr = &prvTgfFrame1Arr[ingressFrame.partsInfo[0].numOfBytes];


    /* AUTODOC: Set hash seed to enforce egress on port2 */
    st = cpssPxIngressHashSeedSet(devNum, seeds[3]); 
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, seeds[3]); 
    
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters)); 
    
    /* AUTODOC: Send UC packet with E-CID[0x11] */
    for (ii = 0; ii < PACKET_NUM; ii++) 
    {
        numOfEgressPortsInfo = 1; 
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum, 
                                                                 SEND_PORT_NUM, 
                                                                 &ingressFrame, 
                                                                 numOfEgressPortsInfo, 
                                                                 egressPortsArr); 
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum); 
        
        prvTgfPxIncrementLAGCounters(devNum, egrCounters);
    }
    
    /* AUTODOC: Expect egress on one port1 */
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM, egrCounters[0]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3]); 


    /* AUTODOC: Set E-Tag with E-CID[0x12] */
    ingressFrame.partsInfo[1].partBytesPtr = egressEtagUc12; 
    egressPortsArr[0].portNum = LAG_PORT2_NUM;

    /* AUTODOC: Set hash seed to enforce egress on port1 */
    st = cpssPxIngressHashSeedSet(devNum, seeds[1]); 
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, seeds[1]); 
    
    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters)); 
    
    /* AUTODOC: Send UC packet with E-CID[0x12] */
    for (ii = 0; ii < PACKET_NUM; ii++) 
    {
        numOfEgressPortsInfo = 1; 
        st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum, 
                                                                 SEND_PORT_NUM, 
                                                                 &ingressFrame, 
                                                                 numOfEgressPortsInfo, 
                                                                 egressPortsArr); 
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum); 
        
        prvTgfPxIncrementLAGCounters(devNum, egrCounters);
    }
    
    /* AUTODOC: Expect egress on one port2 */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM, egrCounters[1]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3]); 
}

/**
* @internal prvTgfPxIngressHashLagMcExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashLagMc for given device
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashLagMcExecute
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   st;
    GT_U32      ii, kk;
    GT_U32      egrCounters[4][4];
    GT_U32      numOfEgressPortsInfo;
    GT_U32      seeds[] = {0xFEED, 0xBABE, 0xDEAD, 0xBEEF};

    static  TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC egressPortsArr[4];
    static TGF_BYTES_INFO_LIST_STC    ingressFrame;

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    cpssOsMemSet(egressPortsArr, 0, sizeof(egressPortsArr));
    cpssOsMemSet(&ingressFrame, 0, sizeof(ingressFrame));

    egressPortsArr[0].portNum = LAG_PORT1_NUM;
    egressPortsArr[1].portNum = LAG_PORT2_NUM;

    /**************************/
    /* prepare ingress packet */
    /**************************/
    ingressFrame.numOfParts = 4;
    ingressFrame.partsInfo[0].numOfBytes   = 12;/*mac DA,SA*/
    ingressFrame.partsInfo[0].partBytesPtr = prvTgfFrame1Arr;
    ingressFrame.partsInfo[1].numOfBytes   = 8;   /*E-Tag*/
    ingressFrame.partsInfo[1].partBytesPtr = egressEtagMc1001;
    ingressFrame.partsInfo[2].numOfBytes   = 0;   /*skip*/
    ingressFrame.partsInfo[2].partBytesPtr = NULL;/*skip*/
    ingressFrame.partsInfo[3].numOfBytes   = sizeof(prvTgfFrame1Arr) - ingressFrame.partsInfo[0].numOfBytes;
    ingressFrame.partsInfo[3].partBytesPtr = &prvTgfFrame1Arr[ingressFrame.partsInfo[0].numOfBytes];


    /* AUTODOC: Two different seeds must produce egress on two different ports*/
    for (kk = 0; kk < 4; kk++) 
    {
        st = cpssPxIngressHashSeedSet(devNum, seeds[kk]); 
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, seeds[kk]); 
        
        for (ii = 0; ii < PACKET_NUM / 2; ii++) 
        {
            numOfEgressPortsInfo = 2; 
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum, 
                                                                     SEND_PORT_NUM, 
                                                                     &ingressFrame, 
                                                                     numOfEgressPortsInfo, 
                                                                     egressPortsArr); 
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum); 
            
            prvTgfPxIncrementLAGCounters(devNum, egrCounters[kk]);
        }
    }
    
    /* AUTODOC: Expect that two different seeds must produce egress on two
       different ports*/
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][0]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][1]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][2]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][3]); 
    
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[1][0]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][1]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][2]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][3]); 
    
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][0]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][1]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][2]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][3]); 
    
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][0]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[3][1]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][2]); 
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][3]); 
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashLagUc)
{
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Skip test for revision A0 */
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(prvTgfDevNum) == 0)
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_TRUE);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        prvTgfPxIngressHashLagConfig(dev, GT_TRUE);

        /* AUTODOC: Execute */
        prvTgfPxIngressHashLagUcExecute(dev); 

        /* AUTODOC: Restore */
        prvTgfPxIngressHashLagConfig(dev, GT_FALSE);
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashLagMc)
{
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Skip test for revision A0 */
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(prvTgfDevNum) == 0)
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_TRUE);

    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        prvTgfPxIngressHashLagConfig(dev, GT_TRUE);

        /* AUTODOC: Execute */
        prvTgfPxIngressHashLagMcExecute(dev); 

        /* AUTODOC: Restore */
        prvTgfPxIngressHashLagConfig(dev, GT_FALSE);
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}


/**
* @internal prvTgfPxIngressHashLagLegacyExecute function
* @endinternal
*
* @brief   Execute test prvTgfPxIngressHashLagLegacy for given device
*
* @param[in] devNum                   - the device number
*                                       None
*/
static GT_VOID prvTgfPxIngressHashLagLegacyExecute
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   st;
    GT_U32      ii, kk;
    GT_U32      egrCounters[4][4];
    GT_U32      numOfEgressPortsInfo;
    CPSS_PX_PORTS_BMP prvTgfPortMcEntries[] =
        {0x1FF1, 0x1FE9, 0x1FE5, 0x1FE3};
    GT_U32      seeds[] = {0xBEAF, 0xBABE, 0xDEADBEAF, 0xFEEDBABE};

    static  TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC egressPortsArr[4];
    static TGF_BYTES_INFO_LIST_STC    ingressFrame;

    cpssOsMemSet(egrCounters, 0, sizeof(egrCounters));
    cpssOsMemSet(egressPortsArr, 0, sizeof(egressPortsArr));
    cpssOsMemSet(&ingressFrame, 0, sizeof(ingressFrame));

    egressPortsArr[0].portNum = LAG_PORT1_NUM;
    egressPortsArr[1].portNum = LAG_PORT2_NUM;

    /**************************/
    /* prepare ingress packet */
    /**************************/
    ingressFrame.numOfParts = 4;
    ingressFrame.partsInfo[0].numOfBytes   = 12;/*mac DA,SA*/
    ingressFrame.partsInfo[0].partBytesPtr = prvTgfFrame1Arr;
    ingressFrame.partsInfo[1].numOfBytes   = 8;   /*E-Tag*/
    ingressFrame.partsInfo[1].partBytesPtr = egressEtagMc1001;
    ingressFrame.partsInfo[2].numOfBytes   = 0;   /*skip*/
    ingressFrame.partsInfo[2].partBytesPtr = NULL;/*skip*/
    ingressFrame.partsInfo[3].numOfBytes   = sizeof(prvTgfFrame1Arr) - ingressFrame.partsInfo[0].numOfBytes;
    ingressFrame.partsInfo[3].partBytesPtr = &prvTgfFrame1Arr[ingressFrame.partsInfo[0].numOfBytes];


    /* AUTODOC: Two different seeds must produce egress on two different ports*/
    for (kk = 0; kk < 4; kk++) 
    {
        st = cpssPxIngressHashSeedSet(devNum, seeds[kk]); 
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, seeds[kk]); 
        
        for (ii = 0; ii < PACKET_NUM / 2; ii++) 
        {
            numOfEgressPortsInfo = 2; 
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum, 
                                                                     SEND_PORT_NUM, 
                                                                     &ingressFrame, 
                                                                     numOfEgressPortsInfo, 
                                                                     egressPortsArr); 
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum); 
            
            prvTgfPxIncrementLAGCounters(devNum, egrCounters[kk]);
        }
    }
    
    /* AUTODOC: Expect no traffic on egress ports */
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][3]);
    
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][3]);
    
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][3]);
    
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][3]);

    /* AUTODOC: Fill upper entries[128..255] of designated table with uniform port distribution for MC traffic */
    prvTgfPxIngressHashDesignatedPortTableUniformFill_A1(devNum, GT_FALSE,
        prvTgfPortMcEntries, PRV_TGF_MEMBERS_NUM_MAC(prvTgfPortMcEntries));

    /* AUTODOC: Two different seeds must produce egress on two different ports*/
    for (kk = 0; kk < 4; kk++) 
    {
        st = cpssPxIngressHashSeedSet(devNum, seeds[kk]); 
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, seeds[kk]); 

        for (ii = 0; ii < PACKET_NUM / 2; ii++) 
        {
            numOfEgressPortsInfo = 2; 
            st = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum, 
                                                                     SEND_PORT_NUM, 
                                                                     &ingressFrame, 
                                                                     numOfEgressPortsInfo, 
                                                                     egressPortsArr); 
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum); 

            prvTgfPxIncrementLAGCounters(devNum, egrCounters[kk]);
        }
    }

    /* AUTODOC: Expect that two different seeds must produce egress on two
       different ports*/
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[0][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[0][3]);

    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[1][3]);

    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(PACKET_NUM / 2, egrCounters[2][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[2][3]);

    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][0]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][1]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][2]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, egrCounters[3][3]);
}

UTF_TEST_CASE_MAC(prvTgfPxIngressHashLagLegacy)
{
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Skip test for revision A0 */
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(prvTgfDevNum) == 0)
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_TRUE);
    packetNumDefaultSet(GT_TRUE);


    /* AUTODOC: Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* AUTODOC: Configure */
        prvTgfPxIngressHashLagLegacyConfig(dev, GT_TRUE);

        /* AUTODOC: Execute */
        prvTgfPxIngressHashLagLegacyExecute(dev); 

        /* AUTODOC: Restore */
        prvTgfPxIngressHashLagLegacyConfig(dev, GT_FALSE); 
    }
    tgfTrafficGeneratorPxIgnoreCounterCheckSet(GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxTgfIngressHash suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfIngressHash)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashPrngIndexing)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashCrc32Indexing)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashSingleDesignated)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashSelectedBits)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashKeyPortNumber)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashUde)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashLagUc)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashLagMc)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxIngressHashLagLegacy)
UTF_SUIT_END_TESTS_MAC(cpssPxTgfIngressHash)


