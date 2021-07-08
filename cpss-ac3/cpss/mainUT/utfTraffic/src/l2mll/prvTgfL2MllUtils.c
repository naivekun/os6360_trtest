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
* @file prvTgfL2MllUtils.c
*
* @brief Utilities for L2 MLL tests
*
* @version   8
********************************************************************************
*/

#include <l2mll/prvTgfL2MllUtils.h>
#include <common/tgfIpGen.h>

/* variable to save and restore global configurations */

static GT_BOOL eVidxForAllLookup;
static GT_U32  maxVidxIndexDef;
static GT_BOOL ttlTrapEnable;
static CPSS_NET_RX_CPU_CODE_ENT ttlCpuCode;

/**
* @internal prvTgfL2MllUtilsGlobalConfigurationGetAndSave function
* @endinternal
*
* @brief   Get related L2 MLL global configuration and save it for later restore.
*/
GT_VOID prvTgfL2MllUtilsGlobalConfigurationGetAndSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* save eVidx for all lookup enabling state */
    rc = prvTgfL2MllLookupForAllEvidxEnableGet(prvTgfDevNum, &eVidxForAllLookup);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableGet");

    /* save maximal VIDX value that refers to a port distribution list(VIDX) */
    rc = prvTgfL2MllLookupMaxVidxIndexGet(prvTgfDevNum, &maxVidxIndexDef);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupMaxVidxIndexGet: maxVidxIndexDef %d",
                                 maxVidxIndexDef);

    /* save TTL exception configuration */
    rc = prvTgfL2MllTtlExceptionConfigurationGet(prvTgfDevNum, &ttlTrapEnable, &ttlCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllTtlExceptionConfigurationGet");
}

/**
* @internal prvTgfL2MllUtilsGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore related L2 MLL global configuration previously saved.
*/
GT_VOID prvTgfL2MllUtilsGlobalConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* restore eVidx for all lookup enabling state */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, eVidxForAllLookup);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 eVidxForAllLookup);

    /* restore maximal VIDX value that refers to a port distribution list(VIDX) */
    rc = prvTgfL2MllLookupMaxVidxIndexSet(prvTgfDevNum, maxVidxIndexDef);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupMaxVidxIndexSet: maxVidxIndexDef %d",
                                 maxVidxIndexDef);

    /* restore TTL exception configuration */
    rc = prvTgfL2MllTtlExceptionConfigurationSet(prvTgfDevNum, ttlTrapEnable, ttlCpuCode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfL2MllTtlExceptionConfigurationSet: enable %d, cpuCode %d",
                                 ttlTrapEnable, ttlCpuCode);
}

/**
* @internal prvTgfL2MllUtilsMllAndLttEntriesClear function
* @endinternal
*
* @brief   Clear L2 MLL & LTT entries.
*
* @param[in] lttIndexArr              - Array of LTT indexes to clear.
* @param[in] numOfLttIndexes          - Number of valid entries in <lttIndexArr>.
* @param[in] mllIndexArr              - Array of MLL indexes to clear.
* @param[in] numOfMllIndexes          - Number of valid entries in <mllIndexArr>.
*                                       None
*/
GT_VOID prvTgfL2MllUtilsMllAndLttEntriesClear
(
    IN GT_U32 *lttIndexArr,
    IN GT_U32  numOfLttIndexes,
    IN GT_U32 *mllIndexArr,
    IN GT_U32  numOfMllIndexes
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;
    GT_U32  i;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    /* clear L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    for ( i = 0 ; i < numOfLttIndexes ; i++)
    {
        rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, lttIndexArr[i], &lttEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index %d",
                                     lttIndexArr[i]);
    }

    /* clear L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    for ( i = 0 ; i < numOfMllIndexes ; i++)
    {
        rc = prvTgfL2MllPairWrite(prvTgfDevNum, mllIndexArr[i], mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index %d",
                                     mllIndexArr[i]);
    }
}

/**
* @internal prvTgfL2MllUtilsResetAllEthernetCounters function
* @endinternal
*
* @brief   Clear all test ports Ethernet counters
*/
GT_VOID prvTgfL2MllUtilsResetAllEthernetCounters
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfL2MllUtilsPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*/
GT_VOID prvTgfL2MllUtilsPacketSend
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr,
    IN GT_U32          prvTgfBurstCount,
    IN GT_U32          prvTgfSendPortIndex
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIndex]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfL2MllLConfigReset function
* @endinternal
*
* @brief   Restore configuration for the MLL and the LTT table according to the info.
*
* @param[in] testMllDbArr[]           - array of 'orig' DB MLL and LTT info
* @param[in] testMllDb_restoreInfoArr[] - array of actual indexes of MLL and LTT entries
*                                       None
*/
GT_VOID prvTgfL2MllLConfigReset
(
    IN L2_MLL_ENTRY_SHORT_INFO_STC testMllDbArr[],
    IN GT_U32                      testMllDb_restoreInfoArr[]
)
{
    GT_U32                         ii;/*iterator*/
    GT_U32                         currentMllIndex;/*(current)index into the L2MLL table */
    GT_U32                         lttIndex;/*index into the MLL LTT table*/

    for(ii = 0; testMllDbArr[ii].mllIndexAndLttIndex != NOT_VALID_ENTRY_CNS ; ii++)
    {
        if(testMllDbArr[ii].mllIndexAndLttIndex != AUTO_CALC_INDEX_CNS)
        {
            lttIndex = TEST_LTT_INDEX_GET(testMllDb_restoreInfoArr[ii]);
            /* AUTODOC: clear LTT MLL entry */
            prvTgfL2MllUtilsMllAndLttEntriesClear(&lttIndex, 1, NULL, 0);
        }

        currentMllIndex = TEST_MLL_INDEX_GET(testMllDb_restoreInfoArr[ii]);

        /* AUTODOC: clear L2 MLL entry */
        prvTgfL2MllUtilsMllAndLttEntriesClear(NULL, 0,&currentMllIndex, 1);
    }
}

/**
* @internal prvTgfL2MllLConfigSet function
* @endinternal
*
* @brief   Set configuration for the MLL and the LTT table according to the info.
*
* @param[in] testMllDbArr[]           - array of 'orig' DB MLL and LTT info
* @param[in] setMode                  - specific test mode how to use the MLL indexes
*
* @param[out] testMllDb_restoreInfoArr[] - array of actual indexes of MLL and LTT entries
*                                       None
*/
GT_VOID prvTgfL2MllLConfigSet
(
    IN L2_MLL_ENTRY_SHORT_INFO_STC  testMllDbArr[],
    OUT GT_U32                      testMllDb_restoreInfoArr[],
    IN MLL_INDEX_TEST_MODE_ENT      setMode
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                lttEntry;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT            mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC                     mllPairEntry;
    GT_U32                                      numOfValidMllInPair;
    GT_U32                         mllsNumber;
    GT_U32                         ii, j;/*iterator*/
    GT_U32                         relativeIndex;/* index relative to 'mllsNumber' */
    GT_U32                         currentMllIndex=0;/*(current)index into the L2MLL table */
    GT_U32                         nextMllIndex;/*(next)index into the L2MLL table */
    GT_U32                         lttIndex;/*index into the MLL LTT table*/

    /* AUTODOC: set LTT and L2 MLL entries needed by the test */

    /* reset L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));
    /* reset L2 MLL entry */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    /* get number of MLLs pairs */
    mllsNumber = prvTgfL2MllNumberOfMllsGet();

    currentMllIndex = nextMllIndex = 0;

    for(ii = 0; testMllDbArr[ii].mllIndexAndLttIndex != NOT_VALID_ENTRY_CNS ; )
    {
        if((testMllDbArr[ii].mllIndexAndLttIndex != NOT_VALID_ENTRY_CNS)&&
           (testMllDbArr[ii+1].mllIndexAndLttIndex != NOT_VALID_ENTRY_CNS))
        {
            mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
            numOfValidMllInPair=2;
        }
        else
        {
            mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;
            numOfValidMllInPair=1;
        }

        /* set a mllPair: firstMllIndex and secondMllIndex */
        for(j=0;j<numOfValidMllInPair;j++)
        {
            if(testMllDbArr[ii+j].mllIndexAndLttIndex == AUTO_CALC_INDEX_CNS)
            {
                currentMllIndex = nextMllIndex;/* value saved from the previous entry set */
                PRV_UTF_LOG1_MAC("index to mll table[0x%x] : pointed from previous mll pointer \n",
                    currentMllIndex);

                testMllDb_restoreInfoArr[ii+j] = currentMllIndex;
            }
            else
            {
                /* ltt entry should be set only once foe each pair */
                if(j==0)
                {
                    /* this is indication to set index according to this test logic */
                    relativeIndex = TEST_MLL_INDEX_GET(testMllDbArr[ii+j].mllIndexAndLttIndex);
                    currentMllIndex = relativeIndex;

                    PRV_UTF_LOG1_MAC("index to mll table[0x%x] : starts new link list \n",
                        currentMllIndex);

                    lttEntry.mllPointer = currentMllIndex;
                    lttEntry.entrySelector = 0;
                    lttIndex = TEST_LTT_INDEX_GET(testMllDbArr[ii+j].mllIndexAndLttIndex);

                    PRV_UTF_LOG1_MAC("index to LTT mll table[0x%x] \n",
                        lttIndex);
                    /* AUTODOC: set the MLL LTT entry that point to this MLL */
                    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, lttIndex, &lttEntry);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet_noWa");

                    testMllDb_restoreInfoArr[ii+j] = testMllDbArr[ii+j].mllIndexAndLttIndex;
                }
                else
                {
                    currentMllIndex = TEST_MLL_INDEX_GET(testMllDbArr[ii+j].mllIndexAndLttIndex);
                    PRV_UTF_LOG1_MAC("index to mll pair mll table[0x%x] : \n", currentMllIndex);

                    testMllDb_restoreInfoArr[ii+j] = currentMllIndex;
                }
            }


            if(j==0)
            {
                mllPairEntry.firstMllNode.last = testMllDbArr[ii+j].last;
                mllPairEntry.firstMllNode.egressInterface.type = testMllDbArr[ii+j].egressInterfaceType;

                switch (mllPairEntry.firstMllNode.egressInterface.type)
                {
                case CPSS_INTERFACE_PORT_E:
                    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[testMllDbArr[ii+j].egressInterfaceValue];
                    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    mllPairEntry.firstMllNode.egressInterface.trunkId = (GT_TRUNK_ID)testMllDbArr[ii+j].egressInterfaceValue;
                    break;
                case CPSS_INTERFACE_VIDX_E:
                    mllPairEntry.firstMllNode.egressInterface.vidx = (GT_U16)testMllDbArr[ii+j].egressInterfaceValue;
                    break;
                default:
                    break;
                }
            }
            else
            {
                mllPairEntry.secondMllNode.last = testMllDbArr[ii+j].last;
                mllPairEntry.secondMllNode.egressInterface.type = testMllDbArr[ii+j].egressInterfaceType;

                switch (mllPairEntry.secondMllNode.egressInterface.type)
                {
                case CPSS_INTERFACE_PORT_E:
                    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[testMllDbArr[ii+j].egressInterfaceValue];
                    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    mllPairEntry.secondMllNode.egressInterface.trunkId = (GT_TRUNK_ID)testMllDbArr[ii+j].egressInterfaceValue;
                    break;
                case CPSS_INTERFACE_VIDX_E:
                    mllPairEntry.secondMllNode.egressInterface.vidx = (GT_U16)testMllDbArr[ii+j].egressInterfaceValue;
                    break;
                default:
                    break;
                }
            }

            relativeIndex = TEST_MLL_INDEX_GET(testMllDbArr[ii+j].nextMllPointer);

            if(setMode == MLL_INDEX_TEST_MODE_REFERENCE_FROM_END_OF_TALBE_E)
            {
                /* the pointer to next MLL pointer is taken relative from the end of the table */
                nextMllIndex = mllsNumber - relativeIndex;
            }
            else
            {
                nextMllIndex = relativeIndex;
            }

            if(((j==0)&&(mllPairEntry.firstMllNode.last == GT_TRUE))||
               ((j==1)&&(mllPairEntry.secondMllNode.last == GT_TRUE)))
            {
                mllPairEntry.nextPointer = 0;
                mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
            }
            else
            {
                mllPairEntry.nextPointer = nextMllIndex;
                mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
            }
        }

        PRV_UTF_LOG1_MAC("nextPointer[0x%x] \n",
                mllPairEntry.nextPointer);

        /* AUTODOC: insert on all MLLs */
        rc = prvTgfL2MllPairWrite(prvTgfDevNum, currentMllIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite");

        ii=ii+2;
    }
}


