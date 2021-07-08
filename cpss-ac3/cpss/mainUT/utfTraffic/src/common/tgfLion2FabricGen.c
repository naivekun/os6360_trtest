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
* @file tgfLion2FabricGen.c
*
* @brief API for Lion2 Fabric for connecting BC2 devices
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfLion2FabricGen.h>

#ifdef CHX_FAMILY
#include <appDemo/sysHwConfig/gtAppDemoLion2FabricForBc2.h>
#endif /* CHX_FAMILY */

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfLion2FabricInit function
* @endinternal
*
* @brief   The function initializes DB and writes PCL rule for TO_CPU packets
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] cpuLinkPortNum           - number of Lion2 port linked to BC2 devices with CPU port
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvTgfLion2FabricInit
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_PORT_NUM        cpuLinkPortNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    /* call device specific API */
    rc = appDemoLion2FabricInit(
        lion2FabricDevNum, cpuLinkPortNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: appDemoLion2FabricInit FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lion2FabricDevNum);
    TGF_PARAM_NOT_USED(cpuLinkPortNum);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfLion2FabricForwardAddBc2Device function
* @endinternal
*
* @brief   The function writes PCL rules for UC packets with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be forwarded to the given target device.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] hwDevId                  - HW id of BC2 target device
* @param[in] linkPortNum              - number of Lion2 port linked to BC2 target device
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvTgfLion2FabricForwardAddBc2Device
(
    IN  GT_U8           lion2FabricDevNum,
    IN  GT_U32          hwDevId,
    IN  GT_PORT_NUM     linkPortNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    /* call device specific API */
    rc = appDemoLion2FabricForwardAddBc2Device(
        lion2FabricDevNum, hwDevId, linkPortNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: appDemoLion2FabricForwardAddBc2Device FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lion2FabricDevNum);
    TGF_PARAM_NOT_USED(hwDevId);
    TGF_PARAM_NOT_USED(linkPortNum);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfLion2FabricForwardAddBc2Vidx function
* @endinternal
*
* @brief   The function writes PCL rules for MC packets with the given target VIDX
*         with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2Vidx                  - target VIDX (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvTgfLion2FabricForwardAddBc2Vidx
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2Vidx,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    /* call device specific API */
    rc = appDemoLion2FabricForwardAddBc2Vidx(
        lion2FabricDevNum, bc2Vidx, targetPortsBmpPtr);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: appDemoLion2FabricForwardAddBc2Vidx FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lion2FabricDevNum);
    TGF_PARAM_NOT_USED(bc2Vidx);
    TGF_PARAM_NOT_USED(targetPortsBmpPtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfLion2FabricForwardAddBc2Vlan function
* @endinternal
*
* @brief   The function writes PCL rules for BC packets with the given target VID
*         with commands FORWARD, FROM_CPU
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2VlanId                - target eVID (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvTgfLion2FabricForwardAddBc2Vlan
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2VlanId,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    /* call device specific API */
    rc = appDemoLion2FabricForwardAddBc2Vlan(
        lion2FabricDevNum, bc2VlanId, targetPortsBmpPtr);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: appDemoLion2FabricForwardAddBc2Vlan FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lion2FabricDevNum);
    TGF_PARAM_NOT_USED(bc2VlanId);
    TGF_PARAM_NOT_USED(targetPortsBmpPtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfLion2FabricCleanUp function
* @endinternal
*
* @brief   The function restores Lion2 device from all Lion2Fabric configurations.
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvTgfLion2FabricCleanUp
(
    IN GT_U8              lion2FabricDevNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    /* call device specific API */
    rc = appDemoLion2FabricCleanUp(
        lion2FabricDevNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: appDemoLion2FabricCleanUp FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lion2FabricDevNum);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfLion2FabricConfigureQos function
* @endinternal
*
* @brief   The function configure Lion2Fabric for TC/DP support inside Lion2 device.
*         The eDSA Tag should remain unchanged.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] toCpuTc                  - TC for all TO_CPU packets
* @param[in] toCpuDp                  - DP for all TO_CPU packets
* @param[in] toAnalyzerTc             - TC for all TO_ANALYZER packets
* @param[in] toAnalyzerDp             - DP for all TO_ANALYZER packets
* @param[in] fromCpuDp                - DP for all FROM_CPU packets
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvTgfLion2FabricConfigureQos
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_U32             toCpuTc,
    IN CPSS_DP_LEVEL_ENT  toCpuDp,
    IN GT_U32             toAnalyzerTc,
    IN CPSS_DP_LEVEL_ENT  toAnalyzerDp,
    IN CPSS_DP_LEVEL_ENT  fromCpuDp
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    /* call device specific API */
    rc = appDemoLion2FabricConfigureQos(
        lion2FabricDevNum, toCpuTc, toCpuDp,
        toAnalyzerTc, toAnalyzerDp, fromCpuDp);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: appDemoLion2FabricConfigureQos FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lion2FabricDevNum);
    TGF_PARAM_NOT_USED(toCpuTc);
    TGF_PARAM_NOT_USED(toCpuDp);
    TGF_PARAM_NOT_USED(toAnalyzerTc);
    TGF_PARAM_NOT_USED(toAnalyzerDp);
    TGF_PARAM_NOT_USED(fromCpuDp);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/******************************************************************************\
 *                       API for default settings                             *
\******************************************************************************/


