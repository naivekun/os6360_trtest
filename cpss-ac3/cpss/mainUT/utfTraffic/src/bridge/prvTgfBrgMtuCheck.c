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
* @file prvTgfBrgMtuCheck.c
*
* @brief Bridge ePort Profile MTU check - basic
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <bridge/prvTgfBrgMtuCheck.h>

/**********************************************************************
 * typedef: PRV_TGF_MTU_RESTORE_CONFIG
 *
 * Description: The MTU specific parameters for restore the default cfg.
 *
 * Fields:
 *   dftMtuSize           - MTU size value per physical port.
 *   dftProfilePort       - MTU profile index per port.
 *   dftMtuCheckEnable    - target ePort MTU check feature enabled.
 *   dftByteCountMode     - default byte count mode.
 *   dftExceptionCommand  - default command.
 *   dftExceptionCpuCode  - CPU code set.
 *********************************************************************/
static struct
{
    GT_U32                                      dftMtuSize;
    GT_U32                                      dftProfilePort;
    GT_BOOL                                     dftMtuCheckEnable;
    PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT     dftByteCountMode;
    CPSS_PACKET_CMD_ENT                         dftExceptionCommand;
    CPSS_NET_RX_CPU_CODE_ENT                    dftExceptionCpuCode;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* MTU size per profile */
#define PRV_TGF_MTU_SIZE_PER_PROFILE_CNS            80

#define PRV_TGF_VLANID_1_CNS                        1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS                   0
/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS                2

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS                2

/* CPU code to be set in test*/
#define PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS          CPSS_NET_CONTROL_E

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* checking the result packets */

/* expected number of Rx packets on ports - for DROP command */
static GT_U8 prvTgfPacketsCountRxArrDrop[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{   /*0 18  36 54 */
    {1, 0, 1, 0}, /*pkt#0*/
    {1, 0, 0, 0}  /*pkt#1*/
};
/* expected number of Tx packets on ports */
static GT_U8 prvTgfPacketsCountTxArrDrop[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 1, 0},
    {1, 0, 0, 0}
};

/* expected number of Rx packets on ports - for Forward command */
static GT_U8 prvTgfPacketsCountRxArrForward[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{   /*0 8  18 23 */
    {1, 0, 1, 0}, /*na*/
    {1, 0, 1, 0}
};
/* expected number of Tx packets on ports */
static GT_U8 prvTgfPacketsCountTxArrForward[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 1, 0}, /*na*/
    {1, 0, 1, 0}
};

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

static GT_U8 prvTgfPayloadData1Arr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, /*64*/
    0x41, 0x42, /*66 length*/
};
static GT_U8 prvTgfPayloadData2Arr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, /*64*/
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, /*96*/
};

/* PAYLOAD part first packet */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart1 =
{
    sizeof(prvTgfPayloadData1Arr),                     /* dataLength */
    prvTgfPayloadData1Arr                              /* dataPtr */
};

/* PAYLOAD part second packet */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart2 =
{
    sizeof(prvTgfPayloadData2Arr),                     /* dataLength */
    prvTgfPayloadData2Arr                              /* dataPtr */
};

/* PARTS of first tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPart1Array[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart1}
};

/* PARTS of second tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPart2Array[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart2}
};

/* LENGTH of first packet =512 */
#define PRV_TGF_TAGGED_PACKET1_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadData1Arr)

/* LENGTH of second packet =513 */
#define PRV_TGF_TAGGED_PACKET2_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadData2Arr)

/* First PACKET to send - size 512 */
static TGF_PACKET_STC prvTgfTaggedPacketInfo1 =
{
    PRV_TGF_TAGGED_PACKET1_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTaggedPacketPart1Array) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPart1Array                                        /* partsArray */
};

/* Second PACKET to send - size 513 */
static TGF_PACKET_STC prvTgfTaggedPacketInfo2 =
{
    PRV_TGF_TAGGED_PACKET2_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTaggedPacketPart2Array) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPart2Array                                        /* partsArray */
};

/* send packet info array */
static TGF_PACKET_STC* prvTgfPacketInfoArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    &prvTgfTaggedPacketInfo1, &prvTgfTaggedPacketInfo2
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgGenMtuCheckConfigSet function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..3)
*                                       None
*/
GT_VOID prvTgfBrgGenMtuCheckConfigSet
(
    IN GT_U32   profileId
)
{
    GT_STATUS   rc = GT_OK;
    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: save default MTU configuration for restore */
    rc = prvTgfBrgGenMtuConfigGet(&prvTgfRestoreCfg.dftMtuCheckEnable,
                                  &prvTgfRestoreCfg.dftByteCountMode,
                                  &prvTgfRestoreCfg.dftExceptionCommand,
                                  &prvTgfRestoreCfg.dftExceptionCpuCode);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuConfigGet: %d %d %d %d",
                                 prvTgfRestoreCfg.dftMtuCheckEnable,
                                 prvTgfRestoreCfg.dftByteCountMode,
                                 prvTgfRestoreCfg.dftExceptionCommand,
                                 prvTgfRestoreCfg.dftExceptionCpuCode);

    rc = prvTgfBrgGenMtuProfileGet(profileId,
                                   &prvTgfRestoreCfg.dftMtuSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuProfileGet: %d %d",
                                 profileId,
                                 prvTgfRestoreCfg.dftMtuSize);


    rc = prvTgfBrgGenMtuPortProfileIdxGet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                          &prvTgfRestoreCfg.dftProfilePort);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuPortProfileIdxGet: %d %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 prvTgfRestoreCfg.dftProfilePort);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_VLANID_1_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: for MTU profile index <I> [0..3] set MTU size 512 */
    rc = prvTgfBrgGenMtuProfileSet(profileId, PRV_TGF_MTU_SIZE_PER_PROFILE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuProfileSet");

    /* AUTODOC: for target ePort 2, set profile <I> */
    rc = prvTgfBrgGenMtuPortProfileIdxSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], profileId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuPortProfileIdxSet");

    /* AUTODOC: for profileId 0 -> set the CPU port Default profileId to 1*/
    if (profileId == 0) {
        rc = prvTgfBrgGenMtuProfileSet(profileId+1, PRV_TGF_MTU_SIZE_PER_PROFILE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuProfileSet");

        rc = prvTgfBrgGenMtuPortProfileIdxSet(prvTgfDevNum,
                                              CPSS_CPU_PORT_NUM_CNS, profileId+1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuPortProfileIdxSet");
    }

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* the 'FROM_CPU' will send to 'eport' (not to physical port) ,
          so it will access the E2PHY and will get 'default' MRUIndex = 0 ...
          that will cause HARD DROP */

        rc = prvTgfBrgGenMtuProfileSet((profileId+2)%4, 1518);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuProfileSet");

        /* so we need to modify the mruIndex on the 'sender' to not be like the tested mruIndex */
        rc = prvTgfBrgGenMtuPortProfileIdxSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], (profileId+2)%4);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuPortProfileIdxSet");
    }

}

/**
* @internal prvTgfBrgGenMtuCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgGenMtuCheckTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portIter = 0;
    GT_U32                      sendIter = 0;
    GT_U32                      packetLen = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U8                       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                      packetActualLength = 0;
    GT_U8                       devNum;
    GT_U8                       queue;
    TGF_NET_DSA_STC             rxParam;

    /* AUTODOC: enable the target ePort MTU check feature and set the parameters:*/
    /* AUTODOC:        enable feature */
    /* AUTODOC:        set the byte count mode to be L3_mode */
    /* AUTODOC:        set exception command to HARD_DROP packet  */
    /* AUTODOC:        set exception CPU code to MTU_EXCEPTION_CPU_CODE */
    rc = prvTgfBrgGenMtuConfigSet(GT_TRUE,
                                  PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E,
                                  CPSS_PACKET_CMD_DROP_HARD_E,
                                  PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuConfigSet");

        /* AUTODOC: GENERATE TRAFFIC#1: */
        for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
        {
            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

            /* reset counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                     prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                             sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* AUTODOC: send 2 packets on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11 */
            /* AUTODOC:   payload sizes: */
            /* AUTODOC:     packet#1 = [512] bytes */
            /* AUTODOC:     packet#2 = [513] bytes */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic#1: */
            /* AUTODOC:   packet#1 (size=512) - received on port 36 */
            /* AUTODOC:   packet#2 (size=513) - dropped */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
                /* check ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxArrDrop[sendIter][portIter],
                                            prvTgfPacketsCountTxArrDrop[sendIter][portIter],
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }

            /* clear VFD array */
            cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        }

    /* AUTODOC: enable the target ePort MTU check feature and set the parameters:*/
    /* AUTODOC:        enable feature */
    /* AUTODOC:        set the byte count mode to be L2_mode */
    /* AUTODOC:        set exception command to FORWARD packet  */
    /* AUTODOC:        set exception CPU code to CPSS_NET_CONTROL_E */
    rc = prvTgfBrgGenMtuConfigSet(GT_TRUE,
                                  PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E,
                                  CPSS_PACKET_CMD_FORWARD_E,
                                  PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuConfigSet");

        /* AUTODOC: GENERATE TRAFFIC#1: */
        for (sendIter = 1; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
        {
            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

            /* reset counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                     prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                             sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* AUTODOC: send 2 packets on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11 */
            /* AUTODOC:   payload sizes: */
            /* AUTODOC:     packet#1 = [512] bytes */
            /* AUTODOC:     packet#2 = [513] bytes */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic#1: */
            /* AUTODOC:   packet#1 (size=512) - received on port 36 */
            /* AUTODOC:   packet#2 (size=513) - forwarded */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
                /* check ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxArrForward[sendIter][portIter],
                                            prvTgfPacketsCountTxArrForward[sendIter][portIter],
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }

            /* clear VFD array */
            cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        }

    /* AUTODOC: enable the target ePort MTU check feature and set the parameters:*/
    /* AUTODOC:        enable feature */
    /* AUTODOC:        set the byte count mode to be L2_mode */
    /* AUTODOC:        set exception command to MIRROR_TO_CPU packet  */
    /* AUTODOC:        set exception CPU code to CPSS_NET_CONTROL_E */
    rc = prvTgfBrgGenMtuConfigSet(GT_TRUE,
                                  PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E,
                                  CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                  PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuConfigSet");

        /* AUTODOC: GENERATE TRAFFIC#1: */
        for (sendIter = 1; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
        {
            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

            /* reset counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                     prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                             sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* AUTODOC: send 2 packets on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11 */
            /* AUTODOC:   payload sizes: */
            /* AUTODOC:     packet#1 = [512] bytes */
            /* AUTODOC:     packet#2 = [513] bytes */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic#1: */
            /* AUTODOC:   packet#1 (size=512) - received on port 36 */
            /* AUTODOC:   packet#2 (size=513) - forwarded */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
                /* check ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxArrForward[sendIter][portIter],
                                            prvTgfPacketsCountTxArrForward[sendIter][portIter],
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }

            /* AUTODOC:   packet#2 received in CPU */
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE, packetBuff,
                                               &buffLen, &packetActualLength,
                                               &devNum, &queue, &rxParam);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");
            /* AUTODOC:   field CPU code is   PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS */
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of CPUcode in rxParams: %d, &d\n",
                                         rxParam.cpuCode, PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS );

            /* clear VFD array */
            cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));
        }

    /* AUTODOC: enable the target ePort MTU check feature and set the parameters:*/
    /* AUTODOC:        enable feature */
    /* AUTODOC:        set the byte count mode to be L2_mode */
    /* AUTODOC:        set exception command to TRAP_TO_CPU packet  */
    /* AUTODOC:        set exception CPU code to CPSS_NET_CONTROL_E */
    rc = prvTgfBrgGenMtuConfigSet(GT_TRUE,
                                  PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E,
                                  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                  PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuConfigSet");

        /* AUTODOC: GENERATE TRAFFIC#1: */
        for (sendIter = 1; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
        {
            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

            /* reset counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                     prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                             sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* AUTODOC: send 2 packets on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11 */
            /* AUTODOC:   payload sizes: */
            /* AUTODOC:     packet#1 = [512] bytes */
            /* AUTODOC:     packet#2 = [513] bytes */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic#1: */
            /* AUTODOC:   packet#1 (size=512) - received on port 36 */
            /* AUTODOC:   packet#2 (size=513) - drop */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
                /* check ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxArrDrop[sendIter][portIter],
                                            prvTgfPacketsCountTxArrDrop[sendIter][portIter],
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
            /* AUTODOC:   packet#2 received in CPU */
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE, packetBuff,
                                               &buffLen, &packetActualLength,
                                               &devNum, &queue, &rxParam);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");
            /* AUTODOC:   field CPU code is   PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS */
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of CPUcode in rxParams: %d, &d\n",
                                         rxParam.cpuCode, PRV_TGF_MTU_EXCEPTION_CPU_CODE_CNS );

            /* clear VFD array */
            cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        }

}

/**
* @internal prvTgfBrgGenMtuCheckConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..3)
*                                       None
*/
GT_VOID prvTgfBrgGenMtuCheckConfigRestore
(
    IN GT_U32   profileId
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* AUTODOC: restore default feature enable and bytecount */
    /* AUTODOC: restore exception command and exception CPU command */
    rc = prvTgfBrgGenMtuConfigSet(prvTgfRestoreCfg.dftMtuCheckEnable,
                                  prvTgfRestoreCfg.dftByteCountMode,
                                  prvTgfRestoreCfg.dftExceptionCommand,
                                  prvTgfRestoreCfg.dftExceptionCpuCode);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuConfigSet: %d %d %d %d",
                                 prvTgfRestoreCfg.dftMtuCheckEnable,
                                 prvTgfRestoreCfg.dftByteCountMode,
                                 prvTgfRestoreCfg.dftExceptionCommand,
                                 prvTgfRestoreCfg.dftExceptionCpuCode);

    /* AUTODOC: restore default MTU size per profile */
    rc = prvTgfBrgGenMtuProfileSet(profileId,
                                   prvTgfRestoreCfg.dftMtuSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuProfileSet: %d %d",
                                 profileId,
                                 prvTgfRestoreCfg.dftMtuSize);

    /* AUTODOC: restore profile per port configuration*/
    rc = prvTgfBrgGenMtuPortProfileIdxSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                          prvTgfRestoreCfg.dftProfilePort);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuPortProfileIdxSet: %d %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 prvTgfRestoreCfg.dftProfilePort);


    /* AUTODOC: for profileId 0 -> restore the CPU port Default profileId to 0 */
    if (profileId == 0) {
        rc = prvTgfBrgGenMtuProfileSet(profileId+1, prvTgfRestoreCfg.dftMtuSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuProfileSet");

        rc = prvTgfBrgGenMtuPortProfileIdxSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuPortProfileIdxSet");
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);


    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* the 'FROM_CPU' will send to 'eport' (not to physical port) ,
          so it will access the E2PHY and will get 'default' MRUIndex = 0 ...
          that will cause HARD DROP */

        rc = prvTgfBrgGenMtuProfileSet((profileId+2)%4, prvTgfRestoreCfg.dftMtuSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuProfileSet");

        /* restore to 0 */
        rc = prvTgfBrgGenMtuPortProfileIdxSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenMtuPortProfileIdxSet");
    }

}


