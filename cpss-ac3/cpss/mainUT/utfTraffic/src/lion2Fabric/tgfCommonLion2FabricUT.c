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
* @file tgfCommonLion2FabricUT.c
*
* @brief Enhanced UTs for App Demo Lion2Fabric
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <common/tgfLion2FabricGen.h>
#include <lion2Fabric/prvTgfLion2Fabric.h>


/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfLion2FabricToDev
   - Init with CPU Device Link.
   - Include Ingress Port configuring it as Device link or as VIDX link
   - Configure Egress Port as given (by hwId) Device link
   - Configure QoS parameters
   - Send Packets and Check Egress ports (by Mac Counters) and Tc/Dp (by Egress Counters):
        - Forwarded Packet to Port on Given Device
        - From CPU Packet to Port on Given Device
        - To Analyzer Packet to Port on Given Device
   - Clean Up.
*/
UTF_TEST_CASE_MAC(tgfLion2FabricToDev)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~ UTF_LION2_E));

    prvTgfLion2FabricToDev();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfLion2FabricToVidx
   - Init with CPU Device Link.
   - Include Ingress Port configuring it as Device link or as VIDX link
   - Configure Egress Ports as given VIDX
   - Configure QoS parameters
   - Send Packets and Check Egress ports (by Mac Counters) and Tc/Dp (by Egress Counters):
        - Forwarded Packet to given VIDX
        - From CPU Packet to given VIDX
        - To Analyzer Packet given VIDX
   - Clean Up.
*/
UTF_TEST_CASE_MAC(tgfLion2FabricToVidx)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~ UTF_LION2_E));

    tgfLion2FabricToVidx();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfLion2FabricToVlan
   - Init with CPU Device Link.
   - Include Ingress Port configuring it as Device link or as VIDX link
   - Configure Egress Ports as given VLAN
   - Configure QoS parameters
   - Send Packets and Check Egress ports (by Mac Counters) and Tc/Dp (by Egress Counters):
        - Forwarded Packet to given VLAN (VIDX==0xFFF)
        - From CPU Packet to given VLAN (VIDX==0xFFF)
   - Clean Up.
*/
UTF_TEST_CASE_MAC(tgfLion2FabricToVlan)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~ UTF_LION2_E));

    tgfLion2FabricToVlan();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: tgfLion2FabricToCpu
   - Init with CPU Device Link.
   - Include Ingress Port configuring it as Device link or as VIDX link
   - Configure QoS parameters
   - Send Packet and Check Egress ports (by Mac Counters) and Tc/Dp (by Egress Counters):
        - To CPU Packet.
   - Clean Up.
*/
UTF_TEST_CASE_MAC(tgfLion2FabricToCpu)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (GT_U32)(~ UTF_LION2_E));

    tgfLion2FabricToCpu();
}

/*
 * Configuration of tgfTrunk suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfLion2Fabric)
    UTF_SUIT_DECLARE_TEST_MAC(tgfLion2FabricToDev)
    UTF_SUIT_DECLARE_TEST_MAC(tgfLion2FabricToVidx)
    UTF_SUIT_DECLARE_TEST_MAC(tgfLion2FabricToVlan)
    UTF_SUIT_DECLARE_TEST_MAC(tgfLion2FabricToCpu)
UTF_SUIT_END_TESTS_MAC(tgfLion2Fabric)


