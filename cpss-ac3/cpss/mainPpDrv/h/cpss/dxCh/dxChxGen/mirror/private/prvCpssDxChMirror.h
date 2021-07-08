/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChMirror.h
*
* @brief Common private mirror declarations.
*
* @version   6
********************************************************************************
*/
#ifndef __prvCpssDxChMirrorh
#define __prvCpssDxChMirrorh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>

#define PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_HW_VAL_MAC(_val, _mode)       \
    switch(_mode)                                                           \
    {                                                                       \
    case CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E:                \
    _val = 0;                                                       \
    break;                                                          \
    case CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E:                \
    _val = 1;                                                       \
    break;                                                          \
    case CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E:                \
    _val = 2;                                                       \
    break;                                                          \
    case CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E: \
    _val = 3;                                                       \
    break;                                                          \
    default:                                                            \
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                            \
}

#define PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_SW_VAL_MAC(_val, _mode)       \
    switch(_val)                                                           \
    {                                                                       \
    case 0:                \
    _mode = CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E;                                                       \
    break;                                                          \
    case 1:                \
    _mode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;                                                       \
    break;                                                          \
    case 2:                \
    _mode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E;                                                       \
    break;                                                          \
    case 3: \
    _mode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E;                                                       \
    break;                                                          \
    default:                                                            \
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                            \
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChMirrorh */

