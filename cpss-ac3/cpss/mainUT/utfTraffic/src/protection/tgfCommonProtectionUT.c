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
* @file tgfCommonProtectionUT.c
*
* @brief Enhanced UTs for CPSS Protection Switching
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <protection/prvTgfProtection.h>

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Protection Switching in 1:1 architecture
    - Enable protection switching
    - Enable TX protection switching for the target eport
    - Map between the target eport to the LOC table
    - Set the LOC table status bit of the source eport to 0
    - Send traffic
    - Verify that the traffic is sent over the working path
    - Set the LOC table status bit of the source eport to 1
    - Send traffic
    - Verify that the traffic is sent over the protection path
    - Disable protection switching
    - Send traffic
    - Verify that the traffic is sent over the working path
*/
UTF_TEST_CASE_MAC(tgfProtectionTxOneToOne)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Bridge configurations */
    prvTgfProtectionTxBridgeConfigurationSet(GT_TRUE);

    /* Enable protection switching */
    prvTgfProtectionEnableConfigSet(GT_TRUE, GT_TRUE);

    /* Protection switching 1:1 TX configurations */
    prvTgfProtectionOneToOneTxConfigurationSet();

    /* Set the LOC table status bit of the source eport to 0 */
    prvTgfProtectionLocStatusConfigSet(CPSS_DIRECTION_EGRESS_E,
                                       PRV_TGF_PROTECTION_LOC_NOT_DETECTED_E,
                                       GT_TRUE);

    /* Send traffic and verify that the traffic is sent over the working path */
    prvTgfProtectionTxOneToOneTrafficGenerate(GT_TRUE);

    /* Set the LOC table status bit of the source eport to 1 */
    prvTgfProtectionLocStatusConfigSet(CPSS_DIRECTION_EGRESS_E,
                                       PRV_TGF_PROTECTION_LOC_DETECTED_E,
                                       GT_FALSE);

    /* Send traffic and verify that the traffic is sent over the protection path */
    prvTgfProtectionTxOneToOneTrafficGenerate(GT_FALSE);

    /* Disable protection switching */
    prvTgfProtectionEnableConfigSet(GT_FALSE, GT_FALSE);

    /* Send traffic and verify that the traffic is sent over the working path */
    prvTgfProtectionTxOneToOneTrafficGenerate(GT_TRUE);

    /* Restore configuration */
    prvTgfProtectionTxOneToOneRestoreConfiguration();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Protection Switching in 1+1 architecture
    - Enable protection switching
    - Set MAC entry - destination is global ePort
    - Set global ePort configuration
    - Enable multi-target port mapping
    - Set the multi-target port value and mask
    - Set the multi-target port base
    - Set the port to VIDX base
    - Configure MLL LTT entry
    - Configure L2 MLL entry with 1+1 protection enabled
*/
UTF_TEST_CASE_MAC(tgfProtectionTxOnePlusOne)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Bridge configurations */
    prvTgfProtectionTxBridgeConfigurationSet(GT_FALSE);

    /* Enable protection switching */
    prvTgfProtectionEnableConfigSet(GT_TRUE, GT_TRUE);

    /* Protection switching 1+1 TX Configurations */
    prvTgfProtectionOnePlusOneTxConfigurationSet();

    /* Send traffic on the MLL entry. Verify that the traffic is sent over the
       2 ports */
    prvTgfProtectionTxOnePlusOneTrafficGenerate();

    /* Restore configuration */
    prvTgfProtectionTxOnePlusOneRestoreConfiguration();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Protection Switching in 1+1 architecture
    - Enable protection switching
    - Configure 2 TTI actions - one for working path and one for protection path
    - Set packet command for RX protection to hard drop
    - Map between source ePort in the TTI action to Protection LOC table
    - Set the LOC table status bit of the target eport to 0
    - Send traffic to the working port and to the protection port
    - Verify that the traffic is accepted on the working port and dropped on the protection port
    - Set the LOC table status bit of the target eport to 1
    - Send traffic to the working port and to the protection port
    - Verify that the traffic is accepted on the protection port and dropped on the working port
    - Set the LOC table status bit of the target eport to 0
    - Send traffic to the working port and to the protection port
    - Verify that the traffic is accepted on the working port and dropped on the protection port
    - Set packet command for protection switching RX exception to trap and set the RX exception CPU code
    - Send traffic to the working port and to the protection port
    - Verify that the traffic is accepted on the working port and trapped on the protection port with correct CPU code
*/
UTF_TEST_CASE_MAC(tgfProtectionRx)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
                                        UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Bridge configurations */
    prvTgfProtectionRxBridgeConfigurationSet();

    /* Enable protection switching */
    prvTgfProtectionEnableConfigSet(GT_TRUE, GT_TRUE);

    /* Protection RX configurations */
    prvTgfProtectionRxConfigurationSet();

    /* Set the LOC table status bit of the target eport to 0 */
    prvTgfProtectionLocStatusConfigSet(CPSS_DIRECTION_INGRESS_E,
                                       PRV_TGF_PROTECTION_LOC_NOT_DETECTED_E,
                                       GT_TRUE);

    /* Send traffic to the working port and to the protection port.
       Verify that the traffic is accepted on the working port and dropped on
       the protection port */
    prvTgfProtectionRxTrafficGenerate(CPSS_PACKET_CMD_FORWARD_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);

    /* Set the LOC table status bit of the target eport to 1 */
    prvTgfProtectionLocStatusConfigSet(CPSS_DIRECTION_INGRESS_E,
                                       PRV_TGF_PROTECTION_LOC_DETECTED_E,
                                       GT_FALSE);

    /* Send traffic to the working port and to the protection port.
       Verify that the traffic is dropped on the working port and accepted on
       the protection port */
    prvTgfProtectionRxTrafficGenerate(CPSS_PACKET_CMD_DROP_HARD_E,
                                      CPSS_PACKET_CMD_FORWARD_E);

    /* Set the LOC table status bit of the target eport to 0 */
    prvTgfProtectionLocStatusConfigSet(CPSS_DIRECTION_INGRESS_E,
                                       PRV_TGF_PROTECTION_LOC_NOT_DETECTED_E,
                                       GT_FALSE);

    /* Send traffic to the working port and to the protection port.
       Verify that the traffic is accepted on the working port and dropped on
       the protection port */
    prvTgfProtectionRxTrafficGenerate(CPSS_PACKET_CMD_FORWARD_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);

    /* Set packet command for protection switching RX exception to trap and set
       the RX exception CPU/drop code */
    prvTgfProtectionRxExceptionConfigSet(CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                         CPSS_NET_CONTROL_E);

    /* Send traffic to the working port and to the protection port.
       Verify that the traffic is accepted on the working port and trapped on
       the protection port with correct CPU code */
    prvTgfProtectionRxTrafficGenerate(CPSS_PACKET_CMD_FORWARD_E,
                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Restore configuration */
    prvTgfProtectionRxRestoreConfiguration();
}

/*
 * Configuration of tgfProtection suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfProtection)
    UTF_SUIT_DECLARE_TEST_MAC(tgfProtectionTxOneToOne)
    UTF_SUIT_DECLARE_TEST_MAC(tgfProtectionTxOnePlusOne)
    UTF_SUIT_DECLARE_TEST_MAC(tgfProtectionRx)
UTF_SUIT_END_TESTS_MAC(tgfProtection)


