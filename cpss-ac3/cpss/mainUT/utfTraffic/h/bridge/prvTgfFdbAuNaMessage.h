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
* @file prvTgfFdbAuNaMessage.h
*
* @brief Check NA message.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbAuNaMessage_H
#define __prvTgfFdbAuNaMessage_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum{
    TESTED_FIELD_IN_NA_MSG_SP_UNKNOWN_E,/*<spUnknown>*/

    TESTED_FIELD_IN_NA_MSG___LAST___E
}TESTED_FIELD_IN_NA_MSG_ENT;

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
);

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
);

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
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbAuNaMessage_H */



