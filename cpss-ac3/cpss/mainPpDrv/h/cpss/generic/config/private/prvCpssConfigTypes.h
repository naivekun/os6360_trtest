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
* @file prvCpssConfigTypes.h
* @version   53
********************************************************************************
*/
#ifndef __prvCpssConfigTypesh
#define __prvCpssConfigTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

/* include the FDB types for the mac Addr Query Info */
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
/* get the port(s) info */
#include <cpss/common/port/private/prvCpssPortTypes.h>
/* get trunk info */
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfMii.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
/* log related defines and prototypes */
#include <cpss/generic/log/prvCpssLog.h>


/* Check for device's port existence and skip if port does not exist */
#define PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(_dev, _port) \
    if(!PRV_CPSS_PHY_PORT_IS_EXIST_MAC(_dev, _port)) \
        continue

/**
* @struct PRV_CPSS_FAMILY_INFO_STC
 *
 * @brief A structure to hold common PP Family needed in CPSS
*/
typedef struct{

    /** info of trunks common to all PP of a specific family */
    PRV_CPSS_FAMILY_TRUNK_INFO_STC trunkInfo;

    /** @brief number of references by devices that associated with this family
     *  needed to support the ability to release this DB
     */
    GT_U32 numReferences;

} PRV_CPSS_FAMILY_INFO_STC;

/* array of pointers to the valid families
(index to array is family type - one of CPSS_PP_FAMILY_TYPE_ENT) */
extern PRV_CPSS_FAMILY_INFO_STC* prvCpssFamilyInfoArray[CPSS_PP_FAMILY_LAST_E];


#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /*__prvCpssConfigTypesh*/

