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
* @file prvTgfNetIfFromCpuToVidxExcludedPort.c
*
* @brief Send a FROM_CPU packet to VIDX with excluded port and ensure the
* port doesn't receive the packet
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>


#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfNetIfGen.h>
#include <netIf/prvTgfNetIfFromCpuToVidxExcludedPort.h>

#define TX_QUEUE_CNS 5

/* vidx. Will be initialized later. */
GT_U16 prvTgfVidx;

/* vlan untagged packet */
static GT_U8 prvTgfPacketBuf[64] = {
    /* MAC DA */ 0x00, 0x00, 0x00, 0x00, 0x34, 0x02,
    /* MAC SA */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x22,
    /* etherType(payload size) */ 0x40,
    /* payload */ 0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA
};

/* pointer to buffer in pool of packet buffers */
static GT_U8 *prvTgfPooBufPtr = NULL;

/* memory to store saved prvTgfPortsArray */
static GT_U8 prvTgfSavedPorts[sizeof(prvTgfPortsArray)];

/**
* @internal prvTgfNetIfFromCpuToVidxConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
static GT_VOID prvTgfNetIfFromCpuToVidxConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    i;
    GT_U32    portsArrLen = sizeof(prvTgfPortsArray)/sizeof(prvTgfPortsArray[0]);


    /* AUTODOC: store original prvTgfPortsArray.*/
    cpssOsMemCpy(prvTgfSavedPorts, prvTgfPortsArray, sizeof(prvTgfPortsArray));

    /* AUTODOC: (SIP5) ensure prvTgfPortsArray[0.. prvTgfPortsNum-1] has
       port number >= BIT_6 to check FROM_CPU eDSA tag field excludedInterface
       physically stored in two separate peaces: 0..5 bits, 6..7 bits */
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* look for first port number >= BIT_6 */
        for (i=0; (i < portsArrLen && prvTgfPortsArray[i] < BIT_6); i++);

        if (i < portsArrLen &&
            i >= prvTgfPortsNum &&
            prvTgfPortsArray[i] < UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(prvTgfDevNum))
        {
            /* A valid port number >= BIT_6 is present in prvTgfPortsArray but is
             * out of first prvTgfPortsNum items. Replace item [0] with it. */
            prvTgfPortsArray[0] = prvTgfPortsArray[i];
        }
    }

    /* AUTODOC: create pool of packet buffers  */
    rc = prvTgfTrafficGeneratorPoolInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrafficGeneratorPoolInit");

    /* AUTODOC: allocate packet buffer from the pool and initialize it */
    prvTgfPooBufPtr = prvTgfTrafficGeneratorPoolGetBuffer();
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, (GT_UINTPTR)prvTgfPooBufPtr,
                                     "prvTgfTrafficGeneratorPoolGetBuffer");
    cpssOsMemCpy(prvTgfPooBufPtr, prvTgfPacketBuf, sizeof(prvTgfPacketBuf) );

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* AUTODOC: initialize vidx with random value 1..max */
    prvTgfVidx  = (GT_U16)(1 + cpssOsRand() %
        (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.vidxNum - 1));

    /* AUTODOC: create VIDX with all ports  */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx,
                               prvTgfPortsArray,
                               NULL,
                               prvTgfPortsNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet");
}

/**
* @internal prvTgfNetIfFromCpuToVidxConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
static GT_VOID prvTgfNetIfFromCpuToVidxConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: invalidate VIDX  */
    rc = prvTgfBrgVidxEntrySet(prvTgfVidx, NULL, NULL, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet");

    /* AUTODOC: free allocated packet buffer */
    if (prvTgfPooBufPtr)
    {
        rc = prvTgfTrafficGeneratorPoolFreeBuffer(prvTgfPooBufPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "prvTgfTrafficGeneratorPoolFreeBuffer");
    }

    /* AUTODOC: restore original prvTgfPortsArray.*/
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPorts, sizeof(prvTgfPortsArray));

    /* AUTODOC: reset MAC counters on all tested ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

}


/**
* @internal prvTgfNetIfFromCpuToVidxTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check the results
*/
static GT_VOID prvTgfNetIfFromCpuToVidxTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    i;
    GT_U32    excludedIter;
    GT_U32    expTxCnt;       /* number of expected egress packets */

    GT_U8     *buffersArr[1];
    GT_U32    buffersLenArr[1];

    PRV_TGF_NET_TX_PARAMS_STC txParams;
    CPSS_INTERFACE_INFO_STC   *excludedInfPtr;
    TGF_DSA_DSA_FROM_CPU_STC *dsaFromCpuPtr;
    GT_HW_DEV_NUM            hwDevNum;
    cpssOsMemSet(&txParams, 0, sizeof(txParams));

    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* AUTODOC: prepare tx parameters: FROM_CPU packet, destination is VIDX */
    txParams.packetIsTagged     = GT_FALSE;
    txParams.sdmaInfo.txQueue   = 5; /* arbitrary value 0..7*/
    txParams.sdmaInfo.recalcCrc = 1;

    txParams.dsaParam.dsaType    = TGF_DSA_4_WORD_TYPE_E;
    txParams.dsaParam.dsaCommand = TGF_DSA_CMD_FROM_CPU_E;
    dsaFromCpuPtr = &txParams.dsaParam.dsaInfo.fromCpu;
    dsaFromCpuPtr->srcHwDev                                = prvTgfDevNum;
    dsaFromCpuPtr->dstInterface.type                       = CPSS_INTERFACE_VIDX_E;
    dsaFromCpuPtr->dstInterface.vidx                       = prvTgfVidx;
    /* dsaFromCpuPtr->dstInterface.vlanId                 = 1; */
    dsaFromCpuPtr->extDestInfo.multiDest.mirrorToAllCPUs   = GT_TRUE;
    dsaFromCpuPtr->extDestInfo.multiDest.srcIsTagged       = GT_FALSE;
    dsaFromCpuPtr->extDestInfo.multiDest.excludeInterface  = GT_TRUE;
    txParams.dsaParam.commonParams.vid                     = 1;
    /* excluded interface will be initialized later */
    excludedInfPtr = &dsaFromCpuPtr->extDestInfo.multiDest.excludedInterface;


    buffersArr[0]    = prvTgfPooBufPtr;
    buffersLenArr[0] = sizeof(prvTgfPacketBuf) / sizeof(prvTgfPacketBuf[0]);

    /* AUTODOC: check two cases: exclude physical port, exclude ePort */
    excludedInfPtr->devPort.hwDevNum = hwDevNum;
    dsaFromCpuPtr->extDestInfo.multiDest.excludedIsPhyPort = GT_TRUE;

    for (excludedIter = 0; excludedIter < prvTgfPortsNum; excludedIter++)
    {
        /* AUTODOC: update FROM_CPU DSA tag to exclude every port (in course) */
        excludedInfPtr->devPort.portNum  = prvTgfPortsArray[excludedIter];


        /* AUTODOC: reset MAC counters on all tested ports */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset",
                                     excludedIter);

        cpssOsPrintf("******************** Exclude Phy Port %d **********\n",
                     excludedInfPtr->devPort.portNum);

        /* AUTODOC: send the packet */
        rc = prvTgfNetIfSdmaTxPacketSend(prvTgfDevNum,
                                         &txParams,
                                         buffersArr, buffersLenArr,
                                         1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfNetIfSdmaTxPacketSend",
                                     excludedIter);

        cpssOsTimerWkAfter(50);
        /* AUTODOC: check MAC counters. Should be incremented every port
           counters except excluded one. */
        for (i =0; i < prvTgfPortsNum; i++)
        {
            expTxCnt = (i == excludedIter ? 0: 1);
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[i],
                                        0, expTxCnt, 0, 1);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                         "excluded port[%02d], expected port[%02d] is%02d\n",
                                         prvTgfPortsArray[excludedIter],
                                         prvTgfPortsArray[i],
                                         expTxCnt);
        }
    }
}

/**
* @internal prvTgfNetIfFromCpuToVidxExcludedPort function
* @endinternal
*
* @brief   Test to check FROM_CPU eDSA "excluded port" feature.
*         Send a FROM_CPU packet to VIDX with excluded port and ensure the
*         port doesn't receive the packet
*/
GT_VOID prvTgfNetIfFromCpuToVidxExcludedPort
(
    GT_VOID
)
{
    prvTgfNetIfFromCpuToVidxConfigurationSet();
    prvTgfNetIfFromCpuToVidxTrafficGenerate();
    prvTgfNetIfFromCpuToVidxConfigurationRestore();
}
