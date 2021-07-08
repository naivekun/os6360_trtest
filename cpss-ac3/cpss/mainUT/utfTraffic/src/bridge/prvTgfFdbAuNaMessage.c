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
* @file prvTgfFdbAuNaMessage.c
*
* @brief Check NA message
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>

#include <bridge/prvTgfFdbAuNaMessage.h>
#include <appDemo/userExit/userEventHandler.h>
#include <bridge/prvTgfFdbAuNaMessageExtFormat.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_2_CNS            2

#define INGRESS_PORT_INDEX_CNS    3

static TESTED_FIELD_IN_NA_MSG_ENT my_testedField = TESTED_FIELD_IN_NA_MSG___LAST___E;
/* expected value of the field */
static GT_U32   my_extraValue = 0;

#define NUM_MSG_TO_GET_CNS  1
static CPSS_MAC_UPDATE_MSG_EXT_STC     my_auMessagesArr[1+NUM_MSG_TO_GET_CNS];
/* indication to get new NA from CPSS , or to keep using NA in my_auMessagesArr[0]
    GT_FALSE - to get new NA from CPSS into my_auMessagesArr[0]
    GT_TRUE - to keep using NA in my_auMessagesArr[0]
*/
static GT_BOOL  my_auMessages_useExistingMsg = GT_FALSE;
/* indication that new NA is expected in the CPU */
static GT_BOOL  my_auMessages_newExpected = GT_TRUE;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
static GT_ETHERADDR macForShadow = {PRV_TGF_FDB_NA_MSG_EXT_FMT_MAC_SA_CNS};
static PRV_TGF_MAC_ENTRY_KEY_STC     my_macForShadow;
/******************************* Inner functions ******************************/

/**
* @internal prvTgfFdbAuNaMessageConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         disable AppDemo from NA processing.
* @param[in] testedField              - one of TESTED_FIELD_IN_NA_MSG_ENT
*                                       None
*/
GT_VOID prvTgfFdbAuNaMessageConfigurationSet
(
    IN TESTED_FIELD_IN_NA_MSG_ENT testedField
)
{
    GT_STATUS                   rc;

    my_testedField = testedField;

    UTF_VERIFY_EQUAL2_STRING_MAC(1, ((my_testedField < TESTED_FIELD_IN_NA_MSG___LAST___E) ? 1:0),
        "testedField: [%d] out of range[%d]",
        my_testedField,
        (TESTED_FIELD_IN_NA_MSG___LAST___E-1));


    /* Disable learning in appDemo and allow accumulation of AU messages by the test */
    rc = appDemoAllowProcessingOfAuqMessages(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");


    /* add ports to vlan 2 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

}

/**
* @internal checkAuMessage function
* @endinternal
*
* @brief   send packet and check NA message.
*/
static GT_VOID checkAuMessage
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    GT_U32                          actNumOfAu;
    CPSS_MAC_UPDATE_MSG_EXT_STC     *auMessagesPtr;
    GT_U32                          numOfAu = NUM_MSG_TO_GET_CNS; /* each time only one message
                                                    is expected */

    if(my_auMessages_newExpected == GT_FALSE)
    {
        actNumOfAu = 1;

        /* get AU messages */
        rc = prvTgfBrgFdbAuMsgBlockGet(prvTgfDevNum, &actNumOfAu, &my_auMessagesArr[NUM_MSG_TO_GET_CNS]);
        if (rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "prvTgfBrgFdbAuMsgBlockGet: %d", prvTgfDevNum);
        }
        if (rc != GT_NO_MORE)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbAuMsgBlockGet: %d", prvTgfDevNum);
        }

        UTF_VERIFY_EQUAL1_PARAM_MAC(0, actNumOfAu , "Not expecting new NA in the CPU \n");

        return;
    }

    if(my_auMessages_useExistingMsg == GT_FALSE)
    {
        actNumOfAu = numOfAu;

        /* get AU messages */
        rc = prvTgfBrgFdbAuMsgBlockGet(prvTgfDevNum, &actNumOfAu, my_auMessagesArr);
        if (rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "prvTgfBrgFdbAuMsgBlockGet: %d", prvTgfDevNum);
        }
        if (rc != GT_NO_MORE)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbAuMsgBlockGet: %d", prvTgfDevNum);
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(numOfAu, actNumOfAu, "The number of received messages is wrong: %d, %d",
                                     actNumOfAu, numOfAu);

        if (actNumOfAu != numOfAu)
        {
            return;
        }

    }
    else
    {
        /* keep using my_auMessagesArr[] */
    }

    auMessagesPtr = &my_auMessagesArr[0];

    /* Check the received message */

    if(my_testedField == TESTED_FIELD_IN_NA_MSG_SP_UNKNOWN_E)
    {
        /* check field */
        UTF_VERIFY_EQUAL0_STRING_MAC(my_extraValue, auMessagesPtr->macEntry.spUnknown,
            "The spUnknown of the AU message is not as expected.");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_NOT_IMPLEMENTED,
            "GT_NOT_IMPLEMENTED");
    }

}


/**
* @internal my_test function
* @endinternal
*
* @brief   Send packet with unknown macDA.
*         Generate traffic:
*         Send from port[3] packet:
*         Success Criteria:
*         The AU NA message is updated correctly. according to testedField
*/
static GT_VOID my_test
(
    GT_VOID
)
{
    GT_U32  ingressPort = prvTgfPortsArray[INGRESS_PORT_INDEX_CNS];

    if(my_auMessages_useExistingMsg == GT_FALSE)
    {
        /* AUTODOC: send packet on Port(i) */
        prvTgfFdbAuNaMessageExtFormatPacketSend(GT_FALSE, ingressPort);
        /* give time for device/simulation for packet processing */
        cpssOsTimerWkAfter(10);
    }

    checkAuMessage();
}

/**
* @internal my_test_spUnknown function
* @endinternal
*
* @brief   test spUnknow
*/
static GT_VOID my_test_spUnknown
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    GT_U32  ingressPort = prvTgfPortsArray[INGRESS_PORT_INDEX_CNS];
    GT_BOOL orig_value_bool = GT_FALSE;
    /* JIRA : MT-294 : The design decides whether to delete an FDB entry based
        on age bit in case of storm prevention entry */
    GT_BOOL flush_sp_require_2_delete_pass = PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) ? GT_FALSE : GT_TRUE;

    my_macForShadow.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    my_macForShadow.key.macVlan.vlanId = PRV_TGF_VLANID_2_CNS;
    my_macForShadow.key.macVlan.macAddr = macForShadow;

    rc = cpssDxChBrgFdbNaStormPreventGet(prvTgfDevNum,ingressPort,&orig_value_bool);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbNaStormPreventGet: device [%d] port[%d]", prvTgfDevNum,ingressPort);

    PRV_UTF_LOG0_MAC("\n");
    PRV_UTF_LOG1_MAC("enable SP on ingress port[%d] \n", ingressPort);

    /* enable SP on ingress port */
    rc = cpssDxChBrgFdbNaStormPreventSet(prvTgfDevNum,
            ingressPort,
            GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbNaStormPreventSet: device [%d] port[%d]", prvTgfDevNum,ingressPort);

    PRV_UTF_LOG0_MAC("send packet - check NA hold <spUnknown> == GT_TRUE \n");
    my_extraValue = 1;
    my_test();

    PRV_UTF_LOG0_MAC("resend packet - check that new packet will NOT generate new NA (due to SP) \n");
    /* check that new packet will NOT generate new NA */
    my_auMessages_newExpected = GT_FALSE;
    my_test();

    PRV_UTF_LOG0_MAC("flush FDB \n");
    /* flush FDB dynamic */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: device [%d] (dynamic only)", prvTgfDevNum);

    if(prvTgfFdbShadowUsed == GT_FALSE)
    {
        GT_U32  numOfValid; /* Number of valid entries in the FDB */
        GT_U32  numOfSkip;  /* Number of entries with skip bit set */
        GT_U32  numOfSp;    /* Number of entries with SP bit set */

        if(flush_sp_require_2_delete_pass == GT_TRUE)
        {
            rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,&numOfSp,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            PRV_UTF_LOG0_MAC("check that first flush FDB action did NOT deleted SP entry \n");
            UTF_VERIFY_EQUAL0_STRING_MAC(1, (numOfValid-numOfSkip),"expected valid non-skipped entry");
            UTF_VERIFY_EQUAL0_STRING_MAC(1, numOfSp,"expected SP entry");

            PRV_UTF_LOG0_MAC("flush FDB (second time !) due to wrong behavior of the device \n");
            /* the SP entries treats the 'delete action' as 'aging action' !!!
                so the first flush caused 'ageBit from 1 to 0'
                and this second flush will actually delete it
            */
            /* flush FDB dynamic (second time) */
            rc = prvTgfBrgFdbFlush(GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: device [%d] (dynamic only)  (second time) ", prvTgfDevNum);
        }

        rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,&numOfSp,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        PRV_UTF_LOG0_MAC("check that flush FDB action deleted SP entry \n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, (numOfValid-numOfSkip),"expected no valid entry");
    }
    else
    {
        /* No entry is in the FDB shadow ... but still it exists in the device */
        /* need to explicitly delete it */
        prvTgfBrgFdbMacEntryDelete(&my_macForShadow);
    }

    PRV_UTF_LOG1_MAC("disable SP on ingress port[%d] \n", ingressPort);
    /* disable SP on ingress port */
    rc = cpssDxChBrgFdbNaStormPreventSet(prvTgfDevNum,
            ingressPort,
            GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbNaStormPreventSet: device [%d] port[%d]", prvTgfDevNum,ingressPort);

    PRV_UTF_LOG0_MAC("send packet - check NA hold <spUnknown> == GT_FALSE \n");
    my_auMessages_newExpected = GT_TRUE;
    my_extraValue = 0;
    my_test();

    PRV_UTF_LOG0_MAC("resend packet - check that new NA (because no SP) \n");
    /* check that new packet will generate new NA (because no auto-learn and no controlled learn)*/
    my_test();

    PRV_UTF_LOG1_MAC("restore SP on ingress port[%d] \n", ingressPort);
    /* restore SP on ingress port */
    rc = cpssDxChBrgFdbNaStormPreventSet(prvTgfDevNum,
            ingressPort,
            orig_value_bool);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbNaStormPreventSet: device [%d] port[%d]", prvTgfDevNum,ingressPort);

    if (prvTgfFdbShadowUsed != GT_FALSE)
    {
        /* No entry is in the FDB shadow ... but still it exists in the device */
        /* need to explicitly delete it */
        prvTgfBrgFdbMacEntryDelete(&my_macForShadow);
    }
}

/**
* @internal prvTgfFdbAuNaMessageTrafficGenerate function
* @endinternal
*
* @brief   Send packet with unknown macDA.
*         Generate traffic:
*         Send from port[3] packet:
*         Success Criteria:
*         The AU NA message is updated correctly. according to testedField
*/
GT_VOID prvTgfFdbAuNaMessageTrafficGenerate
(
    GT_VOID
)
{
    if(my_testedField == TESTED_FIELD_IN_NA_MSG_SP_UNKNOWN_E)
    {
        my_test_spUnknown();
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_NOT_IMPLEMENTED,
            "GT_NOT_IMPLEMENTED");
    }

}


/**
* @internal prvTgfFdbAuNaMessageConfigurationRestore function
* @endinternal
*
* @brief   restore test configuration:
*         enable AppDemo to NA processing.
*/
GT_VOID prvTgfFdbAuNaMessageConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                       rc;


    my_auMessages_useExistingMsg = GT_FALSE;

    /* Enable learning in appDemo */
    rc = appDemoAllowProcessingOfAuqMessages(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "appDemoAllowProcessingOfAuqMessages");

    /* flush FDB dynamic */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: device [%d] (dynamic only)", prvTgfDevNum);

    if(prvTgfFdbShadowUsed)
    {
        /* No entry is in the FDB shadow ... but still it exists in the device */
        /* need to explicitly delete it */
        prvTgfBrgFdbMacEntryDelete(&my_macForShadow);
    }

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

}


