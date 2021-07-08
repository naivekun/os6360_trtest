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
* @file prvTgfPolicerMirroring.c
*
* @brief Policer Mirroring packet cmd tests
*
* @version
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfMirror.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>
#include <policer/prvTgfPolicerMirroring.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>

/* analyzer port number */
#define PRV_TGF_ANALYZER_PORT_IDX_CNS     3

/* amount of sent packets */
#define PRV_TGF_PACKET_NUM_CNS            1

/* analyzer index */
#define PRV_TGF_ANALYZER_INDEX_CNS        2

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/* port index 1 */
#define PRV_TGF_PORT_IDX_1_CNS            1

/* port index 0 */
#define PRV_TGF_SEND_PORT_INDEX_CNS       0

/* interface of an analyzer bound to an rx-port */
PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC     orig_tgfPortInf;
GT_BOOL                                   orig_enable;
GT_U32                                    orig_index;

/* save for restore stage metering enable */
static GT_BOOL saveStageMeterEnable;

/* save for restore port metering enable */
static GT_BOOL savePortMeterEnable;

/* save for restore meter mode: port or flow */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT saveMeterMode;

/* saved policer configuration */
static PRV_TGF_POLICER_ENTRY_STC            saveMeterEntry;

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/*Reset the port counters */
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfPolicerMirroringGlobalCfgSet function
* @endinternal
*
* @brief  Configure FDB and Policer
*/
GT_VOID prvTgfPolicerMirroringCfgSet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
)
{
    GT_STATUS                         rc;
    GT_U32                            plrMru, port_idx;    
    PRV_TGF_POLICER_ENTRY_STC         meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;    

    rc = prvTgfFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbDefCfgSet");        

    /* AUTODOC: Enable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_TRUE);

    /* AUTODOC: Save stage metering enable for restore */
    rc = prvTgfPolicerMeteringEnableGet(prvTgfDevNum,
                                        stage,
                                        &saveStageMeterEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveStageMeterEnable);

    /* AUTODOC: Enable stage for metering */
    rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                  stage, GT_TRUE);

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        port_idx = PRV_TGF_PORT_IDX_1_CNS;
    else
        port_idx = PRV_TGF_SEND_PORT_INDEX_CNS;

    /* AUTODOC: Save port metering enable for restore */
    rc = prvTgfPolicerPortMeteringEnableGet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[port_idx],
                                            &savePortMeterEnable);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerPortMeteringEnableGet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[port_idx],
                                 savePortMeterEnable);

    /* AUTODOC: Enable port for metering */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[port_idx],
                                            GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[port_idx],
                                 GT_TRUE);

    /* AUTODOC: Save metering mode for restore */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum,
                                        stage,
                                        &saveMeterMode);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveMeterMode);

    /* AUTODOC: Set metering mode for "port" mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    rc = cpssDxChPolicerMeteringAnalyzerIndexGet(prvTgfDevNum,stage,CPSS_DP_RED_E,&orig_enable,&orig_index); 
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "cpssDxChPolicerMeteringAnalyzerIndexGet: %d, %d",
                                  prvTgfDevNum,
                                  PRV_TGF_ANALYZER_INDEX_CNS);
    rc = cpssDxChPolicerMeteringAnalyzerIndexSet(prvTgfDevNum,stage,CPSS_DP_RED_E,GT_TRUE,PRV_TGF_ANALYZER_INDEX_CNS); 
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "cpssDxChPolicerMeteringAnalyzerIndexSet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_ANALYZER_INDEX_CNS);
    
    /* AUTODOC: Save Policer entry */
    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                               stage,
                               prvTgfPortsArray[port_idx],
                               &saveMeterEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerEntryGet: %d", prvTgfDevNum);

    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.policerEnable = GT_TRUE;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET1_E;

    /* AUTODOC: Metering according to L2 */
    meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

    meterEntry.modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyTc = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.remarkMode = PRV_TGF_POLICER_REMARK_MODE_L3_E;
    meterEntry.redMirrorToAnalyzerEnable = GT_TRUE;    
    
    rc = prvTgfPolicerMruGet(prvTgfDevNum, stage, &plrMru);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruGet: %d, %d, %d",
                                 prvTgfDevNum, stage, plrMru);    
    meterEntry.tbParams.srTcmParams.cir = 0;
    meterEntry.tbParams.srTcmParams.cbs = plrMru;    
    meterEntry.tbParams.srTcmParams.ebs = plrMru;    
    rc = prvTgfPolicerEntrySet(stage,
                               prvTgfPortsArray[port_idx],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                 prvTgfDevNum, stage);

}

/**
* @internal prvTgfPolicerMirroringTrafficGenerate function
* @endinternal
*
* @brief  Configure to send packet and check traffic sent from analyzer
*/
GT_VOID prvTgfPolicerMirroringTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
)
{
    GT_STATUS                       rc  = GT_OK;        
    CPSS_INTERFACE_INFO_STC         portInterface;            
    GT_U8                           dev      = 0;
    GT_U8                           queue    = 0;
    GT_BOOL                         getFirst = GT_TRUE;
    GT_U32                          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                          packetActualLength = 0;
    TGF_NET_DSA_STC                 rxParam;
    static GT_U8                    packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC tgfPortInf;    
    
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");           
    
    tgfPortInf.interface.type             = CPSS_INTERFACE_PORT_E;
    tgfPortInf.interface.devPort.hwDevNum = prvTgfDevNum;
    tgfPortInf.interface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, PRV_TGF_ANALYZER_INDEX_CNS, &orig_tgfPortInf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d",
                                 prvTgfDevNum); 

    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_INDEX_CNS, &tgfPortInf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d",
                                 PRV_TGF_ANALYZER_INDEX_CNS); 

    /* Start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: enable capture on AnalyzerPort */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);

    /* AUTODOC: Generate traffic */

    rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, &prvTgfPacketInfo,
            prvTgfBurstCount, 0 , NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: Send packet to port 1 */
    rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth");    
        
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = cpssOsMemCmp(&prvTgfPacketL2Part.daMac, &(packetBuff[0]), sizeof(TGF_MAC_ADDR)); /* compare dstMac address */   
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");    

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfPolicerMirroringCfgRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
)
{
    GT_STATUS                         rc,rc1 = GT_OK;
    GT_U32                            port_idx; 
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;    

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        port_idx = PRV_TGF_PORT_IDX_1_CNS;
    else
        port_idx = PRV_TGF_SEND_PORT_INDEX_CNS;
    
    /* AUTODOC: Restore metering entry */
    rc = prvTgfPolicerEntrySet(stage,
                               prvTgfPortsArray[port_idx],
                               &saveMeterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                 stage,
                                 prvTgfPortsArray[port_idx]);    

    /* AUTODOC: Restore stage metering enable */
    rc = prvTgfPolicerMeteringEnableSet(stage,saveStageMeterEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 saveStageMeterEnable);

    /* AUTODOC: Restore port metering enable */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[port_idx],
                                            savePortMeterEnable);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[port_idx],
                                 savePortMeterEnable);

    /* AUTODOC: Restore metering mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,saveMeterMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 saveMeterMode);

    /* AUTODOC: Disable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_FALSE);

    rc = cpssDxChPolicerMeteringAnalyzerIndexSet(prvTgfDevNum,stage,CPSS_DP_RED_E,orig_enable,orig_index); 
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "cpssDxChPolicerMeteringAnalyzerIndexSet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_ANALYZER_INDEX_CNS);
    
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ANALYZER_INDEX_CNS, &orig_tgfPortInf);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d",
                                 PRV_TGF_ANALYZER_INDEX_CNS);

    rc = prvTgfFdbDefCfgRestore();
    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}
