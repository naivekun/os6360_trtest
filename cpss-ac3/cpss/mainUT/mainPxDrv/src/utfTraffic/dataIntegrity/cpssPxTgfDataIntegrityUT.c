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
* @file cpssPxTgfDataIntegrityUT.c
*
* @brief Enhanced UTs for CPSS Px Data Integrity
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>

#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <appDemo/userExit/px/appDemoPxDiagDataIntegrityAux.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>

/******************************************************************************\
 *                             Define section                                 *
\******************************************************************************/
#define PX_JUMBO_FRAME_SIZE_CNS        (10240) /* 10K */
#define PX_SEND_PACKET_SIZE_CNS        (10212)
#define PX_PACKET_BUFFER_SIZE_CNS      (256)
#define PX_BUFFERS_PER_JUMBO_FRAME_CNS (PX_JUMBO_FRAME_SIZE_CNS / PX_PACKET_BUFFER_SIZE_CNS)
#define PX_WAIT_TIMEOUT_CNS            (10000)
#define PX_SENDER_PORT                 prvPxTgfPortsArray[1]

extern GT_BOOL tgfPrvIntendedError;
/* packet for transmitting */
static TGF_PACKET_STC   *prvTgfPacketInfoPtr = NULL;

/* number of frames */
static GT_U32            prvTgfBurstCount    = 0;

/* number of VFDs */
static GT_U32            prvTgfNumVfd        = 0;

/* sleep time in milliseconds  */
#define hugeBurstSleepTime  50

#define RESET_VALUE_CNS 0
static CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memoriesArr[] =
{
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_DESC_IDDB_MEM_E,
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_INTERNAL_DESC_IDDB_MEM_E,
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_DATA_BASE_E,
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_DESCRIPTOR_MEM_E,
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_LL_LINK_LIST_E,
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RXDMA_IBUF_HDR_IDDB_MEM_E,
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E
};

/* Event counter and semaphore signaling we have all needed events */
static GT_U32          errEventCntr = 0;
static CPSS_OS_SIG_SEM eventMppmCame = CPSS_OS_SEMB_EMPTY_E;
static GT_U32          evErrorCtr = 0;
static CPSS_OS_SIG_SEM eventRamErrors = CPSS_OS_SEMB_EMPTY_E;

extern PX_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC *pxDataIntegrityEventIncrementFunc;

/* Callback function prototype for event counting */
typedef GT_STATUS PX_DATA_INTEGRITY_EVENT_CB_FUNC(
    IN GT_U8                                  devNum,
    IN CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
);

/* total nuber of buffers in the device */
#define DEV_BUFFERS_NUM_MAC(devNum)       16*_1M /(8 * PX_PACKET_BUFFER_SIZE_CNS))

/* how many packets need to be sent to utilize all buffers?
   We use Jumbo packets (10K).Each Jumbo allocates 10K/256 buffers.
*/
#define PX_NUM_PACKETS_TO_SEND_MAC(devNum) (16*_1M /(8 * PX_PACKET_BUFFER_SIZE_CNS)) / PX_BUFFERS_PER_JUMBO_FRAME_CNS




/* Restore Values */
static GT_U32          savedMruPort0;
static GT_U32          savedMruPort1;

static PX_DATA_INTEGRITY_EVENT_CB_FUNC *savedDataIntegrityErrorCb;

/*
 * Packet Information
 */
/* L2 part of unknown UC packet */
static TGF_PACKET_L2_STC prvTgfUcPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}  /* srcMac */
};

static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

static GT_U8 prvTgfPayloadDataArr[PX_SEND_PACKET_SIZE_CNS] =
{
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};

static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),   /* dataLength */
    prvTgfPayloadDataArr            /* dataPtr */
};

static TGF_PACKET_PART_STC prvTgfUcPcktArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfUcPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_IPV4_E,     &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

#define PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    sizeof(prvTgfPayloadDataArr)

#define PACKET_CRC_LEN_CNS (PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

static TGF_PACKET_STC prvTgfGlobalPacketInfo = {
    PACKET_LEN_CNS,
    sizeof(prvTgfUcPcktArray) / sizeof(TGF_PACKET_PART_STC),
    prvTgfUcPcktArray
};

static GT_BOOL           pxDiTraceEnable = GT_TRUE;

static GT_U32   prvPxTgfPortsArray[4] = {0,1,2,3};

static CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT currentMemType;

/**
* @internal prvTgfSetTxSetupEth function
* @endinternal
*
* @brief   Setting transmit parameters
*
* @param[in] devNum                   - the CPU device number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfSetTxSetupEth
(
    IN GT_U8                devNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
)
{
    GT_STATUS rc = GT_OK;

    /* set the CPU device to send traffic */
    rc = tgfTrafficGeneratorCpuDeviceSet(devNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficGeneratorCpuDeviceSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* bind parameters into local variables */
    prvTgfPacketInfoPtr = packetInfoPtr;
    prvTgfBurstCount    = burstCount;
    prvTgfNumVfd        = numVfd;

    return (GT_ALREADY_EXIST == rc) ? GT_OK : rc;
}



/**
* @internal static prvTgfStartTransmitingEth function
* @endinternal
*
* @brief   Transmit packet
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
static GT_STATUS prvTgfStartTransmitingEth
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS                rc         = GT_OK;
    GT_U32                   burstCount = prvTgfBurstCount;

    /* clear portInterface */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* fill portInterface struct */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = devNum;
    portInterface.devPort.portNum = portNum;

    /* set port in loopback mode */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: tgfTrafficGeneratorPortLoopbackModeEnableSet FAILED, rc = [%d] , port[%d]", rc , portNum);

        return rc;
    }

    /* transmit packet */
    rc = tgfTrafficGeneratorPortTxEthTransmit(&portInterface,
                                              prvTgfPacketInfoPtr,
                                              burstCount,
                                              prvTgfNumVfd,
                                              NULL,/* array of VFDs */
                                              RESET_VALUE_CNS,
                                              RESET_VALUE_CNS,
                                              0, /* number of packets in burst that will be printed */
                                              GT_TRUE);


    /* disable loopback mode on port */
    (void) tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficGeneratorPortTxEthTransmit FAILED, rc = [%d]", rc);
    }

    return rc;
}
/**
* @internal mppmErrorHandler function
* @endinternal
*
* @brief   Callback called in case of data integrity error in MPPM.
*
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS mppmErrorHandler
(
    IN GT_U8                                  dev,
    IN CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{

    GT_U32 expectedNumOfEvents;


    expectedNumOfEvents = PX_NUM_PACKETS_TO_SEND_MAC(dev);

    /* Print debug info */
    if (pxDiTraceEnable)
    {
        PRV_UTF_LOG0_MAC("====== PX DATA INTEGRITY MPPM ERROR EVENT======\n");
        PRV_UTF_LOG1_MAC("Device:      %d\n", dev);
        PRV_UTF_LOG1_MAC("Event type:  %d\n", eventPtr->eventsType);
        PRV_UTF_LOG1_MAC("Failed row: %d\n",
            eventPtr->location.ramEntryInfo.ramRow);
        PRV_UTF_LOG1_MAC("RAM memType: %d\n",
            eventPtr->location.ramEntryInfo.memType);

        PRV_UTF_LOG1_MAC("isMppmInfoValid:  %d\n",
            eventPtr->location.isMppmInfoValid);
        if (eventPtr->location.isMppmInfoValid)
        {
            PRV_UTF_LOG1_MAC("MPPM ID:  %d\n",
                eventPtr->location.mppmMemLocation.mppmId);
            PRV_UTF_LOG1_MAC("Bank ID:  %d\n",
                eventPtr->location.mppmMemLocation.bankId);
        }

        /* each packet generates both single and multiple ECC errors */
    }
    if((dev == prvTgfDevNum) &&
       (eventPtr->location.isMppmInfoValid == GT_TRUE) &&
       (eventPtr->location.ramEntryInfo.memType ==
            CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E)
      )
    {
        errEventCntr++;
        if(errEventCntr == expectedNumOfEvents)
        {
            cpssOsSigSemSignal(eventMppmCame);
        }
    }

    return GT_OK;
}
/**
* @internal prvTgfDiagMppmEccProtectionConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPxDiagMppmEccProtectionConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: Get default MRU for receiving port */
    rc = cpssPxPortMruGet(prvTgfDevNum,prvPxTgfPortsArray[0], &savedMruPort0);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "prvTgfPortMruGet: "
    "port=[%d]", prvPxTgfPortsArray[0]);

    /* AUTODOC: set MRU value 10240 for receiving port */
    rc = cpssPxPortMruSet(prvTgfDevNum,prvPxTgfPortsArray[0], PX_JUMBO_FRAME_SIZE_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet: "
        "port=[%d], MRU=[%d]", prvPxTgfPortsArray[0], PX_JUMBO_FRAME_SIZE_CNS);

    /* AUTODOC: Get default MRU for sending port */
    rc = cpssPxPortMruGet(prvTgfDevNum, PX_SENDER_PORT, &savedMruPort1);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "prvTgfPortMruGet: "
    "port=[%d]", PX_SENDER_PORT);

    /* AUTODOC: set MRU value 10240 for sending port */
    rc = cpssPxPortMruSet(prvTgfDevNum, prvPxTgfPortsArray[1], PX_JUMBO_FRAME_SIZE_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet: "
        "port=[%d], MRU=[%d]", prvPxTgfPortsArray[1], PX_JUMBO_FRAME_SIZE_CNS);

    /* AUTODOC: Create semaphore to wait all MPPM errors found */
    rc = cpssOsSigSemBinCreate("eventMppmCame", CPSS_OS_SEMB_EMPTY_E,
        &eventMppmCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    /* AUTODOC: Save DI error callback and set own */
    savedDataIntegrityErrorCb = pxDataIntegrityEventIncrementFunc;
    pxDataIntegrityEventIncrementFunc = mppmErrorHandler;


}

/**
* @internal prvTgfDiagMppmEccProtectionTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPxDiagMppmEccProtectionTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    GT_U32                    ii;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    /* AUTODOC: Reset counters
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");*/

    /* AUTODOC: Setup memory type to test (MPPM) */
    memoryLocation.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
    memoryLocation.info.ramEntryInfo.memType =
        CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;

    /* AUTODOC: reset number of MPPM events */
    errEventCntr = 0;

    rc = cpssPxDiagDataIntegrityEventMaskSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
        CPSS_EVENT_UNMASK_E
    );
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssPxDiagDataIntegrityEventMaskSet: "
        "dev=[%d],  eccMode=[%d], unmaskEn=[%d]",
        prvTgfDevNum,CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,CPSS_EVENT_UNMASK_E);

    /* Send packets*/
    for(ii = PX_NUM_PACKETS_TO_SEND_MAC(prvTgfDevNum); ii > 0; ii--)
    {
        tgfPrvIntendedError = GT_TRUE;
        /* Enable error injection */
        rc = cpssPxDiagDataIntegrityErrorInjectionConfigSet(prvTgfDevNum,&memoryLocation,
                                                            CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"cpssPxDiagDataIntegrityErrorInjectionConfigGet: "
                                     "dev=[%d],  injectMode=[%d], injectEn=[%d]",
                                     prvTgfDevNum,CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,GT_TRUE);
        /* Send */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfGlobalPacketInfo,1, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
            prvTgfDevNum);

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, PX_SENDER_PORT);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: "
            "%d, %d\n", prvTgfDevNum, PX_SENDER_PORT);
        /* provide time for events handling */
        cpssOsTimerWkAfter(10);
    }

    /*  Disable error injection */
    rc = cpssPxDiagDataIntegrityErrorInjectionConfigSet(
        prvTgfDevNum,&memoryLocation,CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssPxDiagDataIntegrityErrorInjectionConfigGet: "
        "dev=[%d],  injectMode=[%d], injectEn=[%d]",prvTgfDevNum,CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,GT_FALSE);
    tgfPrvIntendedError = GT_FALSE;
    /* Wait for error events */
    rc = cpssOsSigSemWait(eventMppmCame, PX_WAIT_TIMEOUT_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssOsSigSemWait. Came %d MPPM error events of %d",
        errEventCntr,(PX_NUM_PACKETS_TO_SEND_MAC(prvTgfDevNum)));
}

/**
* @internal prvTgfDiagMppmEccProtectionRestore function
* @endinternal
*
* @brief   Restore saved configuration
*/
GT_VOID prvTgfPxDiagMppmEccProtectionRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Restore DI error callback */
    pxDataIntegrityEventIncrementFunc = savedDataIntegrityErrorCb;

    /* AUTODOC: Delete semaphore */
    rc = cpssOsSigSemDelete(eventMppmCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemDelete");

    /* AUTODOC: restore MRU values for ports*/
    rc = cpssPxPortMruSet(prvTgfDevNum,prvPxTgfPortsArray[0], savedMruPort0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssPxPortMruSet: "
        "port=[%d], MRU=[%d]", prvPxTgfPortsArray[0], savedMruPort0);
    rc = cpssPxPortMruSet(prvTgfDevNum,PX_SENDER_PORT, savedMruPort1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssPxPortMruSet: "
        "port=[%d], MRU=[%d]", PX_SENDER_PORT, savedMruPort1);

}
/*----------------------------------------------------------------------------*/
/* AUTODOC: test for buffer memory (MPPM) ECC protection check

    - Create semaphore to wait signal after MPPM errors found
    - Save Data Integrity error callback and set test one that sends “signaling” semaphore after getting interrupt from the tested memory
    - Reset counters
    - Setup memory type to test - MPPM
    - enable jumbo frames;
    - Setup memory type to test - MPPM
    - Reset number of MPPM events
    - Enable MPPM interrupt
    - Enable error injection for MPPM
    - Send packets
    - Wait for semaphore signal :
      Test passes if the signal is received
      Test fails if the signal wasn’t received
       Restore the configuration

 */
UTF_TEST_CASE_MAC(prvTgfPxDiagMppmEccProtection)
{

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfPxDiagMppmEccProtectionConfigurationSet();
    prvTgfPxDiagMppmEccProtectionTrafficGenerate();
    prvTgfPxDiagMppmEccProtectionRestore();
#endif /* ASIC_SIMULATION */
}
/**
* @internal pxDataIntegrityErrorHandler function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS pxDataIntegrityErrorHandler
(
 IN GT_U8                                     dev,
 IN CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
 )
{
    /* Print debug info */
    PRV_UTF_LOG0_MAC("======PX DATA INTEGRITY Internal Memory ERROR EVENT======\n");
    PRV_UTF_LOG1_MAC("Device     : %d \n", dev);
    PRV_UTF_LOG1_MAC("Event type : %d\n", eventPtr->eventsType);
    PRV_UTF_LOG5_MAC("Memory     : %d {%d, %d, %d}, line # %d\n ",
                     eventPtr->location.ramEntryInfo.memType,
                     eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId,
                     eventPtr->location.ramEntryInfo.ramRow);

    if((dev == prvTgfDevNum) &&(eventPtr->location.ramEntryInfo.memType == currentMemType))
    {
            cpssOsSigSemSignal(eventRamErrors);

    }
    return GT_OK;
}

/**
* @internal pxDataIntegrityInjectError function
* @endinternal
*
* @brief   Inject error in specified memory or disable injection.
*
* @param[in] memType                  - memory type
* @param[in] enable                   - if GT_TRUE --  error injection. In case of GT_FALSE --
*                                      disable error injection.
*                                       None
*/
static GT_VOID pxDataIntegrityInjectError
(
    IN CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    IN GT_BOOL enable
)
{
    GT_STATUS                            rc;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    memoryLocation.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
    memoryLocation.info.ramEntryInfo.memType = memType;

    rc = cpssPxDiagDataIntegrityErrorInjectionConfigSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        enable
    );
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "cpssPxDiagDataIntegrityErrorInjectionConfigGet: dev=%d, injectEn=%d",
        prvTgfDevNum, enable );
}

/**
* @internal pxDataIntegrityInterruptConfig function
* @endinternal
*
* @brief   Mask or unmask interrupt
*
* @param[in] memType                  - memory type
*                                      enable  - if GT_TRUE -- enable error injection. In case of GT_FALSE --
*                                      disable error injection.
*/
static GT_VOID pxDataIntegrityInterruptConfig
(
    IN CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType,
    IN CPSS_EVENT_MASK_SET_ENT                    operation
)
{
    GT_STATUS                            rc;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC   memoryLocation;

    memoryLocation.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
    memoryLocation.info.ramEntryInfo.memType = memType;

    rc = cpssPxDiagDataIntegrityEventMaskSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
        operation
    );
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "cpssPxDiagDataIntegrityEventMaskSet: "
        "dev=[%d], "
        "mode=[CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E], "
        "operation=[%d]",
        prvTgfDevNum,
        operation
    );
}


/**
* @internal prvPxTgfDiagDataIntegrityErrorInjectionConfigSet
*           function
* @endinternal
*
* @brief   Sets configuration
*/
GT_VOID prvPxTgfDiagDataIntegrityErrorInjectionConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Create semaphore to wait all DI errors found */
    rc = cpssOsSigSemBinCreate("eventRamErrors", CPSS_OS_SEMB_EMPTY_E,
        &eventRamErrors);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    /* AUTODOC: Save DI error callback and set own */
    savedDataIntegrityErrorCb = pxDataIntegrityEventIncrementFunc;
    pxDataIntegrityEventIncrementFunc = pxDataIntegrityErrorHandler;

}

/**
* @internal
*           prvPxTgfDiagDataIntegrityErrorInjectionTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvPxTgfDiagDataIntegrityErrorInjectionTrafficGenerate
(
    GT_U32 index
)
{
    GT_STATUS                 rc;
    GT_U32                    jj;


    currentMemType = memoriesArr[index];
    PRV_UTF_LOG1_MAC("index = %d\n", index);
    /* AUTODOC: Unmask event */
    pxDataIntegrityInterruptConfig(currentMemType, CPSS_EVENT_UNMASK_E);


    for(jj = 0; jj < 4; jj++)
    {
        tgfPrvIntendedError = GT_TRUE;
        /* AUTODOC: Inject error */
        pxDataIntegrityInjectError(currentMemType, GT_TRUE);

                /* Send */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfGlobalPacketInfo,1, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
            prvTgfDevNum);

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, PX_SENDER_PORT);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: "
            "%d, %d\n", prvTgfDevNum, PX_SENDER_PORT);
        /* provide time for events handling */
        cpssOsTimerWkAfter(20);
}

    /* AUTODOC: Wait for events */
    rc = cpssOsSigSemWait(eventRamErrors, PX_WAIT_TIMEOUT_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Not all events came.We have %d, but expected ..",
        errEventCntr);

    if(rc != GT_OK)
    {
        cpssOsPrintf("Not all events came.We have %d, but expected ..\n",
            evErrorCtr /*memEntryPtr->expectedErrors*/);
        /* Send signal to free semaphore */
        cpssOsSigSemSignal(eventRamErrors);
        return;
    }
    else
    {
        /* AUTODOC: Maybe some unexpected signals will come */
        cpssOsTimerWkAfter(1000);
    }

    tgfPrvIntendedError = GT_FALSE;

    evErrorCtr = 0;
    pxDataIntegrityInjectError(currentMemType, GT_FALSE);

    /* AUTODOC: mask event */
    pxDataIntegrityInterruptConfig(currentMemType, CPSS_EVENT_MASK_E);
}
/**
* @internal prvPxTgfDiagDataIntegrityErrorInjectionRestore
*           function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvPxTgfDiagDataIntegrityErrorInjectionRestore
(
    GT_VOID
)
{
    GT_STATUS                     rc;


    /* AUTODOC: Restore data integrity error handler */
    pxDataIntegrityEventIncrementFunc = savedDataIntegrityErrorCb;

    /* AUTODOC: Delete semaphore */
    rc = cpssOsSigSemDelete(eventRamErrors);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemDelete");

}
/* AUTODOC: Test Data Integrity Events for different RAMs:
    enable RAM error injection;
    unmask RAM error interrupts;
    send traffic;
    verify  that events with proper information were received;
*/
UTF_TEST_CASE_MAC(prvTgfPxDiagDataIntegrityErrorInjection)
{

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    GT_U32 index;

    prvPxTgfDiagDataIntegrityErrorInjectionConfigSet();
    for (index = 0; memoriesArr[index] != CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E;  index++)
    {
        prvPxTgfDiagDataIntegrityErrorInjectionTrafficGenerate(index);

    }
    prvPxTgfDiagDataIntegrityErrorInjectionRestore();
#endif /* ASIC_SIMULATION */
}

/*
 * Configuration of tgfDataIntegrityDiag suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfDataIntegrity)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxDiagMppmEccProtection)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxDiagDataIntegrityErrorInjection)

UTF_SUIT_END_TESTS_MAC(cpssPxTgfDataIntegrity)

