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
* @file prvTgfTimersUpload.c
*
* @brief Timers upload tests for IPFIX
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfIpfixGen.h>

#include <ipfix/prvTgfTimersUpload.h>

/**
* @internal prvTgfIpfixTimersUploadAbsoluteTest function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadAbsoluteTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     uploadDone;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadSet;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadGet;
    PRV_TGF_IPFIX_TIMER_STC                 timerGet;

    uploadSet.uploadMode = PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E;
    uploadSet.timer.nanoSecondTimer = 0;
    uploadSet.timer.secondTimer.l[0] = 0;
    uploadSet.timer.secondTimer.l[1] = 0;

    rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    uploadSet.timer.secondTimer.l[0] = 0xFFFFFFFF - 2;
    uploadSet.timer.secondTimer.l[1] = 0xFFFFFFFF;

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }

    uploadSet.timer.secondTimer.l[0] = 100;
    uploadSet.timer.secondTimer.l[1] = 0;

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);
    }

    rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                 "Upload mode different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                 "Upload nanoSecond different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[0],
                                 "Upload second.l[0] different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[1],
                                 "Upload second.l[1] different then expected");

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF - 2, uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(100, uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    rc = prvTgfIpfixTimestampUploadTrigger(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadTrigger: %d",
                                 prvTgfDevNum);

    cpssOsTimerWkAfter(10);

    rc = prvTgfIpfixTimestampUploadStatusGet(prvTgfDevNum, &uploadDone);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadStatusGet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, uploadDone,
                                 "Upload was not finished as expected");

    cpssOsTimerWkAfter(5000);

    /* get IPFIX timers */

    rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, &timerGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    if( timerGet.secondTimer.l[0] < 4 ||
        timerGet.secondTimer.l[0] > 6 )
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(5, timerGet.secondTimer.l[0],
                                     "ipfix second.l[0] different then expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, timerGet.secondTimer.l[1],
                                 "ipfix second.l[1] different then expected");

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, &timerGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        if( timerGet.secondTimer.l[0] < 1 ||
            timerGet.secondTimer.l[0] > 3 )
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(2, timerGet.secondTimer.l[0],
                                         "ipfix second.l[0] different then expected");
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(0, timerGet.secondTimer.l[1],
                                     "ipfix second.l[1] different then expected");
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &timerGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        if( timerGet.secondTimer.l[0] < 104 ||
            timerGet.secondTimer.l[0] > 106 )
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(105, timerGet.secondTimer.l[0],
                                         "ipfix second.l[0] different then expected");
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(0, timerGet.secondTimer.l[1],
                                     "ipfix second.l[1] different then expected");
    }
}

/**
* @internal prvTgfIpfixTimersUploadIncrementalTest function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadIncrementalTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     uploadDone;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadSet;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadGet;
    PRV_TGF_IPFIX_TIMER_STC                 timerGet,timerGet1;

    uploadSet.uploadMode = PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E;
    uploadSet.timer.nanoSecondTimer = 0;
    uploadSet.timer.secondTimer.l[0] = 0;
    uploadSet.timer.secondTimer.l[1] = 0;

    rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    uploadSet.timer.secondTimer.l[0] = 0xFFFFFFFF;
    uploadSet.timer.secondTimer.l[1] = 0xFFFFFFFF;

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }

    uploadSet.timer.secondTimer.l[0] = 100;
    uploadSet.timer.secondTimer.l[1] = 0;

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);
    }

    rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                 "Upload mode different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                 "Upload nanoSecond different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[0],
                                 "Upload second.l[0] different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[1],
                                 "Upload second.l[1] different then expected");

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(100, uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    rc = prvTgfIpfixTimestampUploadTrigger(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadTrigger: %d",
                                 prvTgfDevNum);

    cpssOsTimerWkAfter(10);

    rc = prvTgfIpfixTimestampUploadStatusGet(prvTgfDevNum, &uploadDone);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadStatusGet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, uploadDone,
                                 "Upload was not finished as expected");

    cpssOsTimerWkAfter(4000);

    /* get IPFIX timers */

    rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, &timerGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, &timerGet1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[0], timerGet1.secondTimer.l[0]+1,
                                     "ipfix second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[1], timerGet1.secondTimer.l[1],
                                     "ipfix second.l[1] different then expected");

    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &timerGet1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[0] + 100, timerGet1.secondTimer.l[0],
                                     "ipfix second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[1], timerGet1.secondTimer.l[1],
                                     "ipfix second.l[1] different then expected");
    }
}

/**
* @internal prvTgfIpfixTimersUploadRestoreByClear function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadRestoreByClear
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     uploadDone;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadSet;

    uploadSet.uploadMode = PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E;
    uploadSet.timer.nanoSecondTimer = 0;
    uploadSet.timer.secondTimer.l[0] = 0;
    uploadSet.timer.secondTimer.l[1] = 0;

    rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);
    }

    rc = prvTgfIpfixTimestampUploadTrigger(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadTrigger: %d",
                                 prvTgfDevNum);

    cpssOsTimerWkAfter(10);

    rc = prvTgfIpfixTimestampUploadStatusGet(prvTgfDevNum, &uploadDone);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadStatusGet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, uploadDone,
                                 "Upload was not finished as expected");
}


