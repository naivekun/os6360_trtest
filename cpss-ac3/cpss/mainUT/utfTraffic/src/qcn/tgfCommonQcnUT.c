/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file tgfCommonQcnUT.c
*
* @brief Enhanced UTs for CPSS Qcn
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <port/prvTgfPortMacCountersCapture.h>
#include <port/prvTgfPortTxResourceHistogram.h>
#include <port/prvTgfPortTx.h>
#include <port/prvTgfPortIfModeSpeed.h>
#include <port/tgfPortFlowControlPacketsCounter.h>
#include <port/prvTgfPortMruCheck.h>
#include <port/tgfPortFwdToLb.h>
#include <qcn/prvTgfPortTxQcnDba.h>

#include <trafficEngine/tgfTrafficTable.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfQcnGen.h>

extern GT_VOID tgfPortTxSip6QcnQueueResourcesConfigure(GT_BOOL configSet, GT_BOOL ingressPktHdr, GT_U32 resource, 
                                                       GT_U32 poolNum, PRV_TGF_PORT_CN_LIMIT_ENT queuePortLimits);

extern GT_VOID tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate(GT_BOOL ingressPktHdr);

/*----------------------------------------------------------------------------*/
/* AUTODOC: TC1: QCN message generation with ingress packet header and Src Info
Configuration.
1. Enable QCN
2. Set the CNM Triggering State Variables
3. Set the CNM EtherType
4. Set the CN profile configuration for TxQ (7) associated with receive port (1)
5. Set CN sample interval entry
6. Set QueueStatusMode to send CNM to a configurable target (CPU port other than 80)
7. Configure VLAN for port (0) and port(1)
8. Bind receive port (1) to Drop Profile 7.
9. Set appendPacket = TRUE, keepQcnSrcInfo = TRUE and enable CNM for untagged frames

Generate Traffic
1. Clear MAC counters
2. Configure COS profile 1000 with Drop Precedence and Traffic Class (7) of ingress frames. Assign the default COS profile 1000 to receive port 1.
3. Block TxQ (7) for receive port (1).
4. Transmit 2 frames (with MAC DA 00:01:02:03:AA:01) to port 0.

Expected
QCN packet on CPU port 80 with ingress packet header and QCN Src Info
*/
UTF_TEST_CASE_MAC(tgfPortTxQcnQueueResourcesWithIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    tgfPortTxQcnQueueResourcesConfigure(GT_TRUE,GT_TRUE);
    tgfPortTxQcnPortResourcesTrafficGenerate(GT_TRUE);
    tgfPortTxQcnQueueResourcesConfigure(GT_FALSE,GT_FALSE);
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: TC2: QCN message generation without ingress packet header and Src Info

Expected
QCN packet on CPU port 80 without ingress packet header and QCN Src Info
*/
UTF_TEST_CASE_MAC(tgfPortTxQcnQueueResourcesWithoutIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    tgfPortTxQcnQueueResourcesConfigure(GT_TRUE,GT_FALSE);
    tgfPortTxQcnPortResourcesTrafficGenerate(GT_FALSE);
    tgfPortTxQcnQueueResourcesConfigure(GT_FALSE,GT_FALSE);
#endif
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: QCN message generation with ingress packet header and Src Info
Configuration.
1. Enable DBA mode for QCN
2. Enable QCN
3. Set the amount of available buffers for QCN DBA
4. Set the CNM Triggering State Variables
5. Set the CNM EtherType
6. Set the CN profile configuration for TxQ (7) associated with receive port (1)
7. Set CN sample interval entry
8. Set QueueStatusMode to send CNM to a configurable target (CPU port other than 80)
9. Configure VLAN for port (0) and port(1)
10. Bind receive port (1) to Drop Profile 7
11. Set appendPacket = TRUE, keepQcnSrcInfo = TRUE and enable CNM for untagged frames

Generate Traffic
1. Clear MAC counters
2. Configure COS profile 1000 with Drop Precedence and Traffic Class (7) of ingress frames. Assign the default COS profile 1000 to receive port 1.
3. Block TxQ (7) for receive port (1).
4. Transmit 2 frames (with MAC DA 00:01:02:03:AA:01) to port 0.

Expected
QCN packet on CPU port 80 with ingress packet header and Src Info
*/
UTF_TEST_CASE_MAC(tgfPortTxQcnDbaQueueResources)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    tgfPortTxQcnDbaQueueResourcesConfigure(GT_TRUE);
    tgfPortTxQcnDbaPortResourcesTrafficGenerate(GT_TRUE);
    tgfPortTxQcnDbaQueueResourcesConfigure(GT_FALSE);
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: QCN on Falcon without ingress packet header in global free buffer resource 
            mode using Queue/Port profile limits.
Configuration.
1. Set the QCN DBA resource mode to use global free buffers.
2. Enable QCN.
3. Set the ether type for QCN frames.
4. Set the number of global available buffers for QCN DBA.
5. Set the congested q/priority bits in CNM header.
6. Set the CNM enable state for CN untagged frames.
7. Set the QCN trigger threshold.
8. Set the Fb calculation configuration.
9. Set the CNM target assignment mode to QueueStatusMode to send CNM to a configurable target CPU port (81).
10. Set the receiver's port (1) drop profile (7).
11. Set the CNM generation config and excluding the ingress packet header.
12. Set CN Profile Cfg for Drop Profile 7 for all TCs with higher threshold
    and exclude TC used on test (15) which is configured with lower threshold.
13. Configure VLAN (1) for sender port (0) and receiver port(1).
14. Set default QoS profile for sender port (1).
15. Set Trust mode and COS entry for sender port (1).
16. Set the CNM termination status for sender port (1) and CPU receiver port (81).

Generate Traffic
1. Clear MAC counters
2. Configure QCN Queue profile for all values of alpha for drop profile 7 and traffic class 15.
3. Get the expected amount of available buffers for all applicable values of alpha.
4. Block TxQ (15) for receive port (1).
5. Transmit 2 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
6. Enable TxQ (15) for receive port (1).

Expected
QCN packet on CPU port 81 without ingress packet header
*/
UTF_TEST_CASE_MAC(tgfPortTxSip6QcnQueueGlobalResourcesWithoutIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U32 iter = 0;
    PRV_TGF_PORT_CN_LIMIT_ENT queuePortLimit;
    for(iter = 0; iter < 2; iter ++)
    {
        queuePortLimit = (iter == 0) ? PRV_TGF_PORT_CN_QUEUE_LIMIT_E : PRV_TGF_PORT_CN_PORT_LIMIT_E;

        tgfPortTxSip6QcnQueueResourcesConfigure(GT_TRUE/*configSet*/,GT_FALSE/*ingressPktHdr*/,
                                                CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E/*resource*/,
                                                0/*poolNum*/,queuePortLimit/*queuePortLimits*/);
        tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate(GT_FALSE);
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_FALSE,GT_FALSE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E,
                                                0,queuePortLimit);
    }
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Same as TC1 except that the QCN message includes the ingress header too i.e.
Step 11 above should be modifies as 
Set the CNM generation config and include the ingress packet header.

Expected
QCN packet on CPU port 81 with ingress packet header
*/
UTF_TEST_CASE_MAC(tgfPortTxSip6QcnQueueGlobalResourcesWithIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U32 iter = 0;
    PRV_TGF_PORT_CN_LIMIT_ENT queuePortLimit;
    for(iter = 0; iter < 2; iter ++)
    {
        queuePortLimit = (iter == 0) ? PRV_TGF_PORT_CN_QUEUE_LIMIT_E : PRV_TGF_PORT_CN_PORT_LIMIT_E;
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_TRUE,GT_TRUE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E,
                                                0,queuePortLimit);
        tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate(GT_FALSE);
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_FALSE,GT_TRUE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E,
                                                0,queuePortLimit);
    }
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: QCN on Falcon without ingress packet header in pool free buffer resource 
      mode using pool 0 and Queue profile limits.
Configuration: same as TC1 except that the resource used is pool free buffers and pool
0 i.e. Step 1 should configure pool free buffers resource mode using pool 0.

Expected
QCN packet on CPU port 81 with ingress packet header
*/
UTF_TEST_CASE_MAC(tgfPortTxSip6QcnQueuePool0ResourcesWithoutIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U32 iter = 0;
    PRV_TGF_PORT_CN_LIMIT_ENT queuePortLimit;
    for(iter = 0; iter < 2; iter ++)
    {
        queuePortLimit = (iter == 0) ? PRV_TGF_PORT_CN_QUEUE_LIMIT_E : PRV_TGF_PORT_CN_PORT_LIMIT_E;
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_TRUE,GT_FALSE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                0,queuePortLimit);
        tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate(GT_FALSE);
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_FALSE,GT_FALSE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                0,queuePortLimit);
    }
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: QCN on Falcon without ingress packet header in pool free buffer resource 
      mode using pool 1 and Queue profile limits.
Configuration same as TC1 except that the resource used is pool free buffers and pool
1 i.e. Step 1 should configure pool free buffers resource mode using pool 1.

Expected
QCN packet on CPU port 81 with ingress packet header
*/
UTF_TEST_CASE_MAC(tgfPortTxSip6QcnQueuePool1ResourcesWithoutIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U32 iter = 0;
    PRV_TGF_PORT_CN_LIMIT_ENT queuePortLimit;
    for(iter = 0; iter < 2; iter ++)
    {
        queuePortLimit = (iter == 0) ? PRV_TGF_PORT_CN_QUEUE_LIMIT_E : PRV_TGF_PORT_CN_PORT_LIMIT_E;
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_TRUE,GT_FALSE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                1,queuePortLimit);
        tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate(GT_FALSE);
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_FALSE,GT_FALSE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                1,queuePortLimit);
    }
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: QCN on Falcon with ingress packet header in pool free buffer resource 
      mode using pool 0 and Queue profile limits.

Configuration Same as TC3 except that the QCN message includes the ingress header.

Expected
QCN packet on CPU port 81 with ingress packet header
*/
UTF_TEST_CASE_MAC(tgfPortTxSip6QcnQueuePool0ResourcesWithIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U32 iter = 0;
    PRV_TGF_PORT_CN_LIMIT_ENT queuePortLimit;
    for(iter = 0; iter < 2; iter ++)
    {
        queuePortLimit = (iter == 0) ? PRV_TGF_PORT_CN_QUEUE_LIMIT_E : PRV_TGF_PORT_CN_PORT_LIMIT_E;
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_TRUE,GT_TRUE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                0,queuePortLimit);
        tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate(GT_FALSE);
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_FALSE,GT_TRUE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                0,queuePortLimit);
    }
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: QCN on Falcon without ingress packet header in pool free buffer resource 
      mode using pool 1 and Queue profile limits.
Configuration same as TC1 except that the resource used is pool free buffers and pool
1 i.e. Step 1 should configure pool free buffers resource mode using pool 1.

Expected
QCN packet on CPU port 81 with ingress packet header
*/
UTF_TEST_CASE_MAC(tgfPortTxSip6QcnQueuePool1ResourcesWithIngressHdr)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U32 iter = 0;
    PRV_TGF_PORT_CN_LIMIT_ENT queuePortLimit;
    for(iter = 0; iter < 2; iter ++)
    {
        queuePortLimit = (iter == 0) ? PRV_TGF_PORT_CN_QUEUE_LIMIT_E : PRV_TGF_PORT_CN_PORT_LIMIT_E;
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_TRUE,GT_TRUE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                1,queuePortLimit);
        tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate(GT_FALSE);
        tgfPortTxSip6QcnQueueResourcesConfigure(GT_FALSE,GT_TRUE,CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E,
                                                1,queuePortLimit);
    }
#endif
}

/*
 * Configuration of tgfPort suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfQcn)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxQcnQueueResourcesWithIngressHdr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxQcnQueueResourcesWithoutIngressHdr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxQcnDbaQueueResources)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxSip6QcnQueueGlobalResourcesWithoutIngressHdr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxSip6QcnQueueGlobalResourcesWithIngressHdr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxSip6QcnQueuePool0ResourcesWithoutIngressHdr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxSip6QcnQueuePool1ResourcesWithoutIngressHdr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxSip6QcnQueuePool0ResourcesWithIngressHdr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxSip6QcnQueuePool1ResourcesWithIngressHdr)
UTF_SUIT_END_TESTS_MAC(tgfQcn)

