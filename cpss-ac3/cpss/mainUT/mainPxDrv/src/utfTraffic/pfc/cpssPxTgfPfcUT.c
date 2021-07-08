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
* @file cpssPxTgfPfcUT.c
*
* @brief Functional tests for PFC subsystem of PX
*
* @version   1
********************************************************************************
*/
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <utfTraffic/common/cpssPxTgfCommon.h>

#include <cpss/px/cpssPxTypes.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/port/cpssPxPortPfc.h>
#include <cpss/px/port/cpssPxPortTxDebug.h>

#include <gtOs/gtOsMem.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utfTraffic/common/cpssPxTgfCommon.h>

/* Configuration mode */
#define  PRV_TGF_PFC_CONFIGURATION_MODE_SET_CNS       0
#define  PRV_TGF_PFC_CONFIGURATION_MODE_RESTORE_CNS   1


/* user priority */
#define  PRV_TGF_TRAFFIC_USER_PRIORITY_CNS             7

/* count of transmitted frames */
#define  PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS    1500

/* MAC DA used on tests */
static GT_U8 prvTgfPfcMacDa[6] = {
    0x00, 0x01, 0x02, 0x03, 0xAA, 0x05
};

/* struct for backup configuration before test start */
static struct
{
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     srcPacketTypeFormat;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     dstPacketTypeFormat;

} prvTgfPfcBackup;


/**
* @internal prvCpssPxTgfPortPfcTrafficGenerate function
* @endinternal
*
* @brief   set DST Port Map Table entry = receive port (portNum +1)
*         block referenced TxQ for receive port
*         transmit burstCount buffers to referenced quueue on portNum
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] burstCount               - number of buffers to send
*                                       none
*/
GT_VOID prvCpssPxTgfPortPfcTrafficGenerate
(
    GT_SW_DEV_NUM           devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    GT_U32                  tcQueue,
    GT_U32                  burstCount
)
{
    GT_STATUS   rc;
    GT_U8       frame[256];
    GT_U32      frameLength;
    GT_PHYSICAL_PORT_NUM    rcvPortNum;
    GT_PHYSICAL_PORT_NUM    sendPortNum;
    TGF_PACKET_STC              ingressPacketInfo;
    TGF_PACKET_PART_STC         ingressFullPart[1];
    TGF_PACKET_PAYLOAD_STC      ingressPayloadPart;
    CPSS_PX_PORTS_BMP   targetPortsBmp;
    CPSS_PX_COS_FORMAT_ENTRY_STC    cosFormatEntry;

    /* set transmit and receive ports */
    sendPortNum = prvTgfPxPortsArray[portNum];
    rcvPortNum  = prvTgfPxPortsArray[portNum + 1];

    /* create frame */
    rc = cpssPxTgfEthSimpleFrameWithRandomPayloadCreate(
            prvTgfPfcMacDa,  NULL,         /* MAC DA, MAC SA  */
            GT_FALSE, 0, 0, 0,  0x5555,         /* VLAN, EtherType */
            64,                                 /* payload length  */
            frame, &frameLength);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxTgfEthSimpleFrameWithRandomPayloadCreate");

    cpssOsMemSet(&cosFormatEntry, 0, sizeof(cosFormatEntry));

    cosFormatEntry.cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
    cosFormatEntry.cosAttributes.trafficClass   = tcQueue;
    cosFormatEntry.cosAttributes.dropPrecedence = CPSS_DP_GREEN_E;
    cosFormatEntry.cosAttributes.userPriority = PRV_TGF_TRAFFIC_USER_PRIORITY_CNS;

    rc = cpssPxCosFormatEntrySet(devNum, sendPortNum, &cosFormatEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntrySet");

    /* clear MAC counters for receive port */
    rc = cpssPxTgfEthCountersReset(devNum, rcvPortNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxTgfEthCountersReset");

    targetPortsBmp = 1 << rcvPortNum;
    /* set DST Port Map Table entry for target bitmap */
    rc =  cpssPxIngressPortRedirectSet(devNum, rcvPortNum, targetPortsBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapEntrySet");

    /* block referenced TxQ  for receive port */
    rc = cpssPxPortTxDebugQueueTxEnableSet(devNum, rcvPortNum,tcQueue, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");


    /* transmit frames to send port */
    ingressPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    ingressPacketInfo.numOfParts = 1;
    ingressPacketInfo.partsArray = &ingressFullPart[0];

    ingressFullPart[0].type = TGF_PACKET_PART_PAYLOAD_E;
    ingressFullPart[0].partPtr = &ingressPayloadPart;

    ingressPayloadPart.dataLength = frameLength;
    ingressPayloadPart.dataPtr    = frame;
    rc = tgfTrafficGeneratorPxTransmitPacketAndCheckResult(devNum,sendPortNum,&ingressPacketInfo,burstCount ,0,NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPxTransmitPacketAndCheckResult");
}

/**
* @internal prvCpssPxTgfPortPfcConfigurationBackup function
* @endinternal
*
* @brief   Backup configuration for "One Port" test.
*/
static GT_VOID prvCpssPxTgfPortPfcConfigurationBackup
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum

)
{
    GT_STATUS   rc;


    /* Backup Packet Type Source Format entry */
    rc = cpssPxIngressPortMapPacketTypeFormatEntryGet (devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                                  (portNum + 1), &(prvTgfPfcBackup.srcPacketTypeFormat));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet1");

    /* Backup Packet Type Destination Format entry */
    rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                                      (portNum + 1),&(prvTgfPfcBackup.dstPacketTypeFormat));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssPxIngressPortMapPacketTypeFormatEntryGet2");

    /* Backup CoS format entry configuration */
    rc = cpssPxCosFormatEntryGet(devNum, portNum,
            &(prvTgfPfcBackup.cosFormatEntry));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntryGet");


}

/**
* @internal prvCpssPxTgfPortPfcConfigurationSetOrRestore function
* @endinternal
*
* @brief   Restore/Set configuration for "One Port" test.
*
* @param[in] mode                     - configuration mode
*                                       None.
*/
static GT_VOID prvCpssPxTgfPortPfcConfigurationSetOrRestore
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    IN  GT_U32                                 mode
)
{
    GT_STATUS   rc;

    /*
        for RESTORE mode
    */
    if (mode == PRV_TGF_PFC_CONFIGURATION_MODE_RESTORE_CNS)
    {
        /* Restore from backup packet type format */
        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                                (portNum + 1),  &(prvTgfPfcBackup.srcPacketTypeFormat));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapPacketTypeFormatEntrySet");


            /* Set/Restore Packet Type Destination Format entry */
        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                                                          CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                                                          (portNum + 1),  &(prvTgfPfcBackup.dstPacketTypeFormat));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxIngressPortMapPacketTypeFormatEntrySet");

        /* Restore from backup CoS format entry */
        rc = cpssPxCosFormatEntrySet(devNum, portNum,
                &(prvTgfPfcBackup.cosFormatEntry));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxCosFormatEntrySet");
        /* Set/Restore DST Port Map Table entry for target bitmap */
    }
    /*
        for SET mode
    */
    else
    {
        /*
            BACKUP CONFIGURATION
        */
        prvCpssPxTgfPortPfcConfigurationBackup(devNum, portNum);

    }


}


/**
* @internal prvCpssPxTgfPortPfcTrafficTestResult function
* @endinternal
*
* @brief   Checks that
*         PFC counter value is as expected
*         PFC triggering messages was sent
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] expectedPfcCounter       - PFC counter expected value
*                                       none
*/
GT_VOID prvCpssPxTgfPortPfcTrafficTestResult
(
    GT_SW_DEV_NUM           devNum,
    GT_PHYSICAL_PORT_NUM    portNum,
    GT_U32                  tcQueue,
    GT_U32                  expectedPfcCounter
)
{
    GT_STATUS   rc;
    GT_U32      pfcCounterNum;
    GT_U32      pfcCountValue;
    GT_U64      pfcTrigCntrValue;
    GT_U32      expectedpfcTrigCntr, receivedValue;

    pfcCounterNum = portNum;

    if (cpssDeviceRunCheck_onEmulator())
    {
        cpssOsTimerWkAfter(50);
    }
   /* get count of allocated buffers */
    rc = cpssPxPortPfcCounterGet(devNum, tcQueue, pfcCounterNum,&pfcCountValue);
    pfcCountValue = (pfcCountValue & 0x1fffff);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssPxPortPfcCounterGet");
    /* verify number of buffers/packets */
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedPfcCounter, pfcCountValue,"got other count of buffers than expected");

    cpssPxPortMacCounterGet (devNum, portNum,  CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E, &pfcTrigCntrValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortMacCounterGet");

    receivedValue = pfcTrigCntrValue.l[0];

    /* expected counters value*/
    expectedpfcTrigCntr = 1;
    /* check counters */
    UTF_VERIFY_EQUAL3_STRING_MAC(expectedpfcTrigCntr, receivedValue,
            "Got other counter value than expected on port %d:PFC MAC counter:  %d expected: %d\n",
            portNum, receivedValue, expectedpfcTrigCntr);
    /* release TxQ for receive port */
    rc = cpssPxPortTxDebugQueueTxEnableSet(devNum, (portNum + 1),tcQueue, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxDebugQueueTxEnableSet");
    prvCpssPxTgfPortPfcConfigurationSetOrRestore(devNum, portNum, PRV_TGF_PFC_CONFIGURATION_MODE_RESTORE_CNS);
    cpssOsTimerWkAfter(50);

}

/**
* @internal prvCpssPxTgfPortPfcTriggeringSet function
* @endinternal
*
* @brief   Configure PFC triggering parameters
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      pfcCounterNum    - PFC counter (APPLICABLE RANGES: 0..127)
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcProfileCfgPtr         - pointer to PFC Profile configurations
*                                       none
*/
GT_VOID prvCpssPxTgfPortPfcTriggeringSet
(
    IN  GT_SW_DEV_NUM                          devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    IN  GT_U32                                 profileIndex,
    IN  GT_U32                                 tcQueue,
    IN CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_STATUS st;
    CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode;

    /* no remapping - default settings are used pfcCounterNum = portNum*/

    prvCpssPxTgfPortPfcConfigurationSetOrRestore(devNum, portNum, PRV_TGF_PFC_CONFIGURATION_MODE_SET_CNS);

    pfcCountMode = CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E;
    /* call device specific API to enable PFC triggering*/
    st = cpssPxPortPfcCountingModeSet(devNum,pfcCountMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"ERROR of cpssPxPortPfcCountingModeSet: %d", st);



    st = cpssPxPortPfcProfileIndexSet(devNum, portNum, profileIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"ERROR of cpssPxPortPfcProfileIndexSet: %d", st);

    st = cpssPxPortPfcProfileQueueConfigSet(devNum, profileIndex, tcQueue, pfcProfileCfgPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR of cpssPxPortPfcProfileQueueConfigSet: %d", st);

}


/**
* @internal prvTgfPortPfcTriggeringTest function
* @endinternal
*
* @brief   Sets PFC triggering parameters - no DBA mode
*         set DST Port Map Table entry = receive port (portNum +1)
*         block referenced TxQ for receive port
*         transmit burstCount buffers to referenced quueue on portNum
*         Checks that
*         PFC counter value is as expected
*         PFC triggering messages was sent
*/
GT_VOID prvTgfPortPfcTriggeringTest
(
    IN  GT_SW_DEV_NUM                          devNum
)
{
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U32  profileIndex, tcQueue, burstCount;
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     pfcProfileCfg;

    if(!cpssDeviceRunCheck_onEmulator())
    {
        pfcProfileCfg.xonThreshold  = 10;
        pfcProfileCfg.xoffThreshold = 0x1FF;
        pfcProfileCfg.xonAlpha      = 0;
        pfcProfileCfg.xoffAlpha     = 0;

        burstCount = 0x200;
    }
    else
    {
        pfcProfileCfg.xonThreshold  = 1;
        pfcProfileCfg.xoffThreshold = 9;
        pfcProfileCfg.xonAlpha      = 0;
        pfcProfileCfg.xoffAlpha     = 0;

        burstCount = 10;
    }

    for (portNum = 0; portNum < 16; (portNum = portNum + 2)) 
    {
        for (profileIndex = 0; profileIndex < 8; (profileIndex = profileIndex + 4)) 
        {
            for (tcQueue = 0; tcQueue < 8; (tcQueue = tcQueue + 4)) 
            {
                if(cpssDeviceRunCheck_onEmulator())
                {
                    cpssOsTimerWkAfter(50);
                }

                prvCpssPxTgfPortPfcTriggeringSet(devNum, portNum, profileIndex, tcQueue, &pfcProfileCfg); 
                prvCpssPxTgfPortPfcTrafficGenerate(devNum, portNum, tcQueue, burstCount); 
                prvCpssPxTgfPortPfcTrafficTestResult(devNum, portNum, tcQueue, burstCount);
            }
        }
    }
}

/**
* @internal prvTgfPortPfcDbaTriggeringTest function
* @endinternal
*
* @brief   Sets PFC triggering parameters - with DBA
*         set DST Port Map Table entry = receive port (portNum +1)
*         block referenced TxQ for receive port
*         transmit burstCount buffers to referenced quueue on portNum
*         Checks that
*         PFC counter value is as expected
*         PFC triggering messages was sent
*/
GT_VOID prvTgfPortPfcDbaTriggeringTest
(
   IN GT_SW_DEV_NUM    devNum
)
{
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U32  profileIndex, tcQueue, burstCount;
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     pfcProfileCfg;
    GT_STATUS st;

    pfcProfileCfg.xonThreshold  = 10;
    pfcProfileCfg.xoffThreshold = 10;
    pfcProfileCfg.xonAlpha      = 0;
    pfcProfileCfg.xoffAlpha     = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;
    burstCount = 900;

    st =  cpssPxPortPfcDbaAvailableBuffersSet(devNum, 1000);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"ERROR of cpssPxPortPfcDbaAvailableBuffersSet: %d", st);
    for (portNum = 0; portNum < 16; (portNum = portNum + 2))
    {
        for (profileIndex = 0; profileIndex < 8; (profileIndex = profileIndex +4) )
        {
            for (tcQueue = 0; tcQueue < 8; (tcQueue = tcQueue + 4))
            {
                prvCpssPxTgfPortPfcTriggeringSet(devNum, portNum, profileIndex, tcQueue, &pfcProfileCfg);
                prvCpssPxTgfPortPfcTrafficGenerate(devNum, portNum, tcQueue, burstCount);
                prvCpssPxTgfPortPfcTrafficTestResult(devNum, portNum,tcQueue, burstCount);
            }

        }
    }
    /*retore */
    st =  cpssPxPortPfcDbaAvailableBuffersSet(devNum, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"ERROR of cpssPxPortPfcDbaAvailableBuffersSet: %d", st);


}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxTgfPortPfcTriggering)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U8    devNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        prvTgfPortPfcTriggeringTest (devNum);
    }
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxTgfPortPfcDbaTriggering)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    GT_U8       devNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        prvTgfPortPfcDbaTriggeringTest (devNum);
    }

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssTgfCpssPxPfc suite
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfPxPfc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPortPfcTriggering)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPortPfcDbaTriggering)

UTF_SUIT_END_TESTS_MAC(cpssPxTgfPxPfc)

