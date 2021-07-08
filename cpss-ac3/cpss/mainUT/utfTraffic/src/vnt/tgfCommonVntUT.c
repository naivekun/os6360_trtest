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
* @file tgfCommonVntUT.c
*
* @brief Enhanced UTs for CPSS Vnt
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <vnt/prvTgfVnt.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 7.1 - Oam Port Pdu Trap Enable:
    configure VLAN, FDB entries;
    enable TRAP_TO_CPU of 802.3ah LLC protocol;
    send OAM control traffic;
    verify no Tx traffic on ports and verify traffic on CPU.
*/
UTF_TEST_CASE_MAC(tgfVntOamPortPduTrapEnable)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* Set configuration */
    prvTgfVntOamPortPduTrapSet();

    /* Generate traffic */
    prvTgfVntOamPortPduTrapTrafficGenerate();

    /* Restore configuration */
    prvTgfVntOamPortPduTrapRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 7.2 - Vnt Loopback Mode Configuration:
    configure VLAN, FDB entries;
    enable 802.3ah Loopback mode and TRAP_TO_CPU of 802.3ah LLC protocol;
    send OAM control traffic;
    verify no Tx traffic on loopback port and verify traffic on other ports;
    send BC traffic;
    verify traffic on loopback port and verify no traffic on other ports.
*/
UTF_TEST_CASE_MAC(tgfVntLoopbackConfiguration)
{
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("Test crashes GM\n");

    /* Set configuration */
    prvTgfVntLoopbackModeConfSet();

    /* Generate traffic */
    prvTgfVntLoopbackTrafficConfGenerate();

    /* Restore configuration */
    prvTgfVntLoopbackConfRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 7.3 - Vnt Cfm EtherType Identification:
    configure VLAN, FDB entries;
    configure PCL policy, UDB fields;
    add PCL rule to match EtherType and specific UDB field;
    configure EtherType to identify CFM PDUs;
    send BC traffic with no EtherType;
    verify Tx traffic on all ports;
    send BC traffic with configured EtherType;
    verify no Tx traffic on all ports, only on CPU.
*/
UTF_TEST_CASE_MAC(tgfVntCfmEtherTypeIdentification)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    CPSS_TBD_BOOKMARK/* UTF_XCAT2_E should be removed after PCL xCat2 update */

    /* Set configuration */
    prvTgfVntCfmEtherTypeIdentificationSet();

    /* Generate traffic */
    prvTgfVntCfmEtherTypeIdentificationTrafficGenerate();

    /* Restore configuration */
    prvTgfVntCfmEtherTypeIdentificationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 7.4 - Vnt Cfm Opcode Identification:
    configure VLAN, FDB entries;
    configure PCL policy, UDB fields;
    add PCL rule (forward) to match EtherType and specific UDB field;
    configure EtherType to identify CFM PDUs and set CFM LBR opcode;
    send CFM traffic with configured EtherType;
    verify Tx traffic forwarded due to PCL rule with updated Opcode.
*/
UTF_TEST_CASE_MAC(tgfVntCfmOpcodeIdentification)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    CPSS_TBD_BOOKMARK/* UTF_XCAT2_E should be removed after PCL xCat2 update */

    /* Set configuration */
    prvTgfVntCfmOpcodeIdentificationSet();

    /* Generate traffic */
    prvTgfVntCfmOpcodeIdentificationTrafficGenerate();

    /* Restore configuration */
    prvTgfVntCfmOpcodeIdentificationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 7.5 - Vnt Cfm Opcode Change:
    configure VLAN entry;
    configure TTI rule to match EtherType;
    configure EtherType to identify CFM PDUs and set CFM LBR opcode;
    send CFM traffic with configured EtherType;
    verify Tx traffic forwarded due to TTI rule with updated Opcode.
    change CFM opcode change mode.
    verify Tx traffic forwarded due to TTI rule with modified Opcode.
*/
UTF_TEST_CASE_MAC(tgfVntCfmOpcodeChange)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT2_E);
    CPSS_TBD_BOOKMARK/* UTF_XCAT2_E should be removed after PCL xCat2 update */

    /* Set configuration */
    prvTgfVntCfmOpcodeChangeSet();

    /* Generate traffic */
    prvTgfVntCfmOpcodeChangeTrafficGenerate();

    /* Restore configuration */
    prvTgfVntCfmOpcodeChangeRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test 7.6 - Unidirectional transmit mode:
    configure VLAN, FDB entries;
    enable Force Link Down and unidirectional transmit on specific port;
    send BC traffic;
    verify Tx traffic on all ports (including link down port).
*/
UTF_TEST_CASE_MAC(tgfVntOamPortUnidirectionalTransmitMode)
{
    /* Set configuration */
    prvTgfVntOamPortUnidirectionalTransmitModeSet();

    /* Generate traffic */
    prvTgfVntOamPortUnidirectionalTransmitModeTrafficGenerate();

    /* Restore configuration */
    prvTgfVntOamPortUnidirectionalTransmitModeRestore();
}


/*
 * Configuration of tgfVnt suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfVnt)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVntOamPortPduTrapEnable)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVntLoopbackConfiguration)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVntCfmEtherTypeIdentification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVntCfmOpcodeIdentification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVntCfmOpcodeChange)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVntOamPortUnidirectionalTransmitMode)
UTF_SUIT_END_TESTS_MAC(tgfVnt)


