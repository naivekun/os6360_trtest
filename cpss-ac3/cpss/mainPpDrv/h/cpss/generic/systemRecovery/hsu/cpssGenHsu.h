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
* @file cpssGenHsu.h
*
* @brief Includes generic HSU definitions.
*
*
* @version   7
********************************************************************************
*/

#ifndef __cpssGenHsuh
#define __cpssGenHsuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/* indicates single iteration export/import */
#define CPSS_HSU_SINGLE_ITERATION_CNS                       0xFFFFFFFF

/**
* @enum CPSS_HSU_DATA_TYPE_ENT
 *
 * @brief Different data types that can be exported and imported.
*/
typedef enum{

    /** TCAM manager HSU data type */
    CPSS_HSU_DATA_TYPE_TCAM_MANAGER_E

} CPSS_HSU_DATA_TYPE_ENT;

/**
* @internal cpssHsuEventHandleUpdate function
* @endinternal
*
* @brief   This function replace old event handle created after regular init
*         to new event handle that application got during HSU.
*
* @note   APPLICABLE DEVICES:      ALL
* @param[in] oldEvReqHndl             - old event  handle created after regular init.
* @param[in] newEvReqHndl             - new event handle  created during HSU.
*
* @retval GT_OK                    - on success.
*/
GT_STATUS cpssHsuEventHandleUpdate
(
    IN     GT_U32 oldEvReqHndl,
    IN    GT_U32  newEvReqHndl
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenHsuh */


