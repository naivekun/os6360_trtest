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
* @file tgfNstGen.h
*
* @brief Generic API for Nst
*
* @version   1
********************************************************************************
*/
#ifndef __tgfNstGenh
#define __tgfNstGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/


/**
* @enum PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT
 *
 * @brief Enumeration for L2 or/and L3 traffic type
*/
typedef enum{

    /** @brief Port isolation for L2
     *  packets (L2 Port Isolation table)
     */
    PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E,

    /** @brief Port isolation for L3
     *  packets (L3 Port Isolation table)
     */
    PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E

} PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT;

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/
/**
* @internal prvTgfNstPortIsolationEnableSet function
* @endinternal
*
* @brief   Function enables/disables the port isolation feature.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - port isolation feature enable/disable
*                                      GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfNstPortIsolationEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enable
);

/**
* @internal prvTgfNstPortIsolationTableEntrySet function
* @endinternal
*
* @brief   Function sets port isolation table entry.
*         Each entry represent single source port/device or trunk.
*         Each entry holds bitmap of all local device ports (and CPU port), where
*         for each local port there is a bit marking. If it's a member of source
*         interface (if outgoing traffic from this source interface is allowed to
*         go out at this specific local port).
* @param[in] devNum                   - device number
* @param[in] trafficType              - packets traffic type - L2 or L3
* @param[in] srcInterface             - table index is calculated from source interface.
*                                      Only portDev and Trunk are supported.
* @param[in] cpuPortMember            - port isolation for CPU Port
*                                      GT_TRUE - member
*                                      GT_FALSE - not member
* @param[in] localPortsMembersPtr     - (pointer to) port bitmap to be written to the
*                                      L2/L3 PI table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, srcInterface or
*                                       localPortsMembersPtr
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Traffic is separated for L2 and L3 (means that application may
*       block L2 traffic while allowing L3).
*
*/
GT_STATUS prvTgfNstPortIsolationTableEntrySet
(
    IN GT_U8                                        devNum,
    IN PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT  trafficType,
    IN CPSS_INTERFACE_INFO_STC                      srcInterface,
    IN GT_BOOL                                      cpuPortMember,                      
    IN CPSS_PORTS_BMP_STC                           *localPortsMembersPtr
);

/******************************************************************************\
 *                       API for default settings                             *
\******************************************************************************/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfNstGenh */


