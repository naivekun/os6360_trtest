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
* @file prvTgfPortGroupAlarms.h
*
* @brief IPFIX Alarms per port group test for IPFIX declarations
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPortGroupAlarmsh
#define __prvTgfPortGroupAlarmsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpfixPortGroupAlarmsTestInit function
* @endinternal
*
* @brief   IPFIX per port group alarms manipulation test configuration set.
*/
GT_VOID prvTgfIpfixPortGroupAlarmsTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixPortGroupAlarmsTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixPortGroupAlarmsTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixPortGroupAlarmsTestRestore function
* @endinternal
*
* @brief   IPFIX per port group alarms manipulation test configuration restore.
*/
GT_VOID prvTgfIpfixPortGroupAlarmsTestRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortGroupAlarmsh */

