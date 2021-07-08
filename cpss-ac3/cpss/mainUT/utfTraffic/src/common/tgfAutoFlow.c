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
* @file tgfAutoFlow.c
*
* @brief Generic API for enhanced UT AutoFlow.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMath.h>


#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

#include <common/tgfAutoFlow.h>

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/**
* @struct PRV_TGF_AF_TRAFFIC_STC
 *
 * @brief The traffic identifiers of the auto flow.
 * (R)- if not set, randomized in creation.
 * (RR) - if not set, randomized before every flow.
*/
typedef struct{

    /** L2/IPv4/IPv6   (R) */
    PRV_TGF_AF_TRAFFIC_TYPE_ENT trafficType;

    /** number of packets in every burst   (RR) */
    GT_U32 burstCount;

    /** @brief MAC DA      (RR)
     *  daMAcValid      - is user's preset valid
     */
    TGF_MAC_ADDR daMac;

    /** MAC SA      (RR) */
    TGF_MAC_ADDR saMac;

    /** UC, MC or BC, only if DA is not preset  (RR) */
    PRV_TGF_AF_ADDRESS_TYPE_ENT addressType;

    /** untagged/tagged/double tagged */
    PRV_TGF_AF_VLAN_TAG_TYPE_ENT vlanTagType;

    /** vlan tag 0 struct */
    TGF_PACKET_VLAN_TAG_STC vlanTag0;

    /** vlan tag 1 struct */
    TGF_PACKET_VLAN_TAG_STC vlanTag1;

    /** packets EtherType struct */
    TGF_PACKET_ETHERTYPE_STC etherTypeStc;

    /** IPv4 header struct */
    TGF_PACKET_IPV4_STC ipv4Header;

    /** IPv6 header struct */
    TGF_PACKET_IPV6_STC ipv6Header;

    /** @brief packet size   (RR)
     *  pazketSizeValid   - is user's preset valid
     */
    GT_U32 packetSize;

    /** data structure  (RR) */
    PRV_TGF_AF_DATA_STC dataStc;

    /** is user's preset valid */
    GT_BOOL trafficTypeValid;

    /** is user's preset valid */
    GT_BOOL burstCountValid;

    GT_BOOL daMacValid;

    /** is user's preset valid */
    GT_BOOL saMacValid;

    /** is user's preset valid */
    GT_BOOL addressTypeValid;

    /** is user's preset valid */
    GT_BOOL vlanTagTypeValid;

    /** is user's preset valid */
    GT_BOOL vlanTag0Valid;

    /** is user's preset valid */
    GT_BOOL vlanTag1Valid;

    /** is user's preset valid */
    GT_BOOL etherTypeStcValid;

    /** is user's preset valid */
    GT_BOOL ipv4HeaderValid;

    /** is user's preset valid */
    GT_BOOL ipv6HeaderValid;

    GT_BOOL packetSizeValid;

    /** @brief is user's preset valid
     *  Comment:
     */
    GT_BOOL dataStcValid;

} PRV_TGF_AF_TRAFFIC_STC;

/**
* @struct PRV_TGF_AF_BRIDGE_STC
 *
 * @brief The bridge identifiers of the auto flow.
 * (R)- if not set, randomized in creation.
 * (RR) - if not set, randomized before every flow.
*/
typedef struct{

    /** the ingress vlan id (R) */
    GT_U16 ingressVlan;

    /** @brief the ingress port index (RR)
     *  ingressPortIdxVlan   - is user's preset valid
     */
    GT_U32 ingressPortIdx;

    /** ingress vlan tag 0 pid */
    TGF_ETHER_TYPE ingTag0Pid;

    /** ingress vlan tag 1 pid */
    TGF_ETHER_TYPE ingTag1Pid;

    /** egress vlan tag 0 pid */
    TGF_ETHER_TYPE egTag0Pid;

    /** egress vlan tag 1 pid */
    TGF_ETHER_TYPE egTag1Pid;

    /** egress tagging */
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT egTagCmd;

    /** is user's preset valid */
    GT_BOOL ingressVlanValid;

    GT_BOOL ingressPortIdxValid;

    /** is user's preset valid */
    GT_BOOL ingTag0PidValid;

    /** is user's preset valid */
    GT_BOOL ingTag1PidValid;

    /** is user's preset valid */
    GT_BOOL egTag0PidValid;

    /** is user's preset valid */
    GT_BOOL egTag1PidValid;

    /** @brief is user's preset valid
     *  Comment:
     */
    GT_BOOL egTagCmdValid;

} PRV_TGF_AF_BRIDGE_STC;



static PRV_TGF_AF_TRAFFIC_STC *trafficAf=NULL;
static PRV_TGF_AF_BRIDGE_STC  *bridgeAf=NULL;

static GT_U32   prvTgfAfSeed=0;
static GT_BOOL  prvTgfAfSeedValid=GT_FALSE;
/*static variables to save user settings (prevent changes after creation)*/
static GT_U16   prvTgfAfDefVlanId = 0;
static GT_U16   prvTgfAfVlanId = 1;
static GT_U32   prvTgfAfPacketPartsNum=0;
static GT_U32   prvTgfAfV4=0;
static GT_U32   prvTgfAfV6=0;
static GT_U32   prvTgfAfVlanTags=0;
static GT_U32   prvTgfAfHeadersSize=0;
static PRV_TGF_AF_TRAFFIC_TYPE_ENT prvTgfAfTrafficType=PRV_TGF_AF_TRAFFIC_TYPE_L2_E;

static GT_U16   prvTgfAfIngTpidEnt0=0x8100;
static GT_U16   prvTgfAfIngTpidEnt1=0x8100;
static GT_U16   prvTgfAfEgTpidEnt0=0x8100;
static GT_U16   prvTgfAfEgTpidEnt1=0x8100;
static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT    
           prvTgfAfEgTagCmd = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;

/*static variables to save data from device for restoring*/
static GT_U16   prvTgfAfDefIngTpidEnt0=0x8100;
static GT_U16   prvTgfAfDefIngTpidEnt1=0x8100;
static GT_U16   prvTgfAfDefEgTpidEnt0=0x8100;
static GT_U16   prvTgfAfDefEgTpidEnt1=0x8100;
static GT_U32   prvTgfAfDefIngTpidTag0Bmp=255;
static GT_U32   prvTgfAfDefIngTpidTag1Bmp=255;
static GT_U32   prvTgfAfDefEgTpidTag0Idx=0;
static GT_U32   prvTgfAfDefEgTpidTag1Idx=0;


static TGF_PACKET_PAYLOAD_STC   *prvTgfAfPacketPayloadPartPtr=NULL;
static TGF_PACKET_PART_STC      *prvTgfAfPacketPartArray=NULL;
static TGF_PACKET_L2_STC        *prvTgfAfPacketL2PartPtr=NULL;
static TGF_PACKET_VLAN_TAG_STC  *prvTgfAfPacketTag0PartPtr=NULL;
static TGF_PACKET_VLAN_TAG_STC  *prvTgfAfPacketTag1PartPtr=NULL;
static TGF_PACKET_ETHERTYPE_STC *prvTgfAfPacketEtherTypePartPtr=NULL;
static TGF_PACKET_IPV4_STC      *prvTgfAfPacketIpV4PartPtr=NULL;
static TGF_PACKET_IPV6_STC      *prvTgfAfPacketIpV6PartPtr=NULL;
static TGF_PACKET_STC           *prvTgfAfPacketInfoPtr = NULL;

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/
/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/
/**
* @internal tgfAutoFlowTrafficPreset function
* @endinternal
*
* @brief   The function presets basic traffic settings for the auto test creation.
*
* @param[in] trafficParam             - parameter for setting
* @param[in] valuePtr                 - (pointer to) value for the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowTrafficPreset
(
    IN  PRV_TGF_AF_TRAFFIC_PARAM_ENT                    trafficParam,
    IN  PRV_TGF_AF_TRAFFIC_UNT                         *valuePtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          idx;
    /*AUTODOC: preset traffic values*/
    switch(trafficParam)
    {
        case PRV_TGF_AF_TRAFFIC_PARAM_TRAFFIC_TYPE_E:
            if (valuePtr->trafficType >= PRV_TGF_AF_TRAFFIC_TYPE_L2_E && valuePtr->trafficType <= PRV_TGF_AF_TRAFFIC_TYPE_IP6_E)
            {
                trafficAf->trafficType = valuePtr->trafficType;
                trafficAf->trafficTypeValid = GT_TRUE;
            }
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_BURST_COUNT_E:
            trafficAf->burstCount = valuePtr->burstCount;
            trafficAf->burstCountValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_MAC_DA_E:
            for(idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
            {
                trafficAf->daMac[idx] = valuePtr->daMac[idx];
            }
            trafficAf->daMacValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_MAC_SA_E:
            for(idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
            {
                trafficAf->saMac[idx] = valuePtr->saMac[idx];
            }
            trafficAf->saMacValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_ADDRESS_TYPE_E:
            if (valuePtr->addressType >= PRV_TGF_AF_ADDRESS_TYPE_UC_E && valuePtr->addressType <= PRV_TGF_AF_ADDRESS_TYPE_BC_E)
            {
                trafficAf->addressType = valuePtr->addressType;
                trafficAf->addressTypeValid = GT_TRUE;
            }
                break;
        case PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_TYPE_E:
            trafficAf->vlanTagType = valuePtr->vlanTagType;
            trafficAf->vlanTagTypeValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_0_E:
            trafficAf->vlanTag0 = valuePtr->vlanTag0;
            trafficAf->vlanTag0Valid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_1_E:
            trafficAf->vlanTag1 = valuePtr->vlanTag1;
            trafficAf->vlanTag1Valid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_ETHER_TYPE_E:
            trafficAf->etherTypeStc = valuePtr->etherTypeStc;
            trafficAf->etherTypeStcValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_IPV4_HEADER_E:
            trafficAf->ipv4Header = valuePtr->ipv4Header;
            trafficAf->ipv4HeaderValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_IPV6_HEADER_E:
            trafficAf->ipv6Header = valuePtr->ipv6Header;
            trafficAf->ipv6HeaderValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_PACKET_SIZE_E:
            trafficAf->packetSize = valuePtr->packetSize;
            trafficAf->packetSizeValid = GT_TRUE;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_DATA_STC_E:
            trafficAf->dataStc.dataPtr = valuePtr->dataStc.dataPtr;
            trafficAf->dataStc.dataLen = valuePtr->dataStc.dataLen;
            trafficAf->dataStcValid = GT_TRUE;
            break;
        default:
            rc = GT_BAD_PARAM;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfAutoFlowTrafficPreset: %d ",
                                 prvTgfDevNum); 
}

/**
* @internal tgfAutoFlowTrafficGet function
* @endinternal
*
* @brief   The function returns basic traffic settings for the auto test creation.
*         If the parameter has not been set earlier,
*         the recieved value will not necessarily be the one used in the test
*         - randomization might occur while creating the packet.
*         for more information - look in the description of the parameters.
* @param[in] trafficParam             - parameter to get
*
* @param[out] valuePtr                 - (pointer to) value of the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowTrafficGet
(
    IN  PRV_TGF_AF_TRAFFIC_PARAM_ENT                  trafficParam,
    OUT PRV_TGF_AF_TRAFFIC_UNT                       *valuePtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          idx;
    /*AUTODOC: get traffic values*/
    switch(trafficParam)
    {
        case PRV_TGF_AF_TRAFFIC_PARAM_TRAFFIC_TYPE_E:
            if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->trafficType = trafficAf->trafficType;
            }
            else 
            {
                valuePtr->trafficType = prvTgfAfTrafficType;
            }
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_BURST_COUNT_E:
            valuePtr->burstCount = trafficAf->burstCount;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_MAC_DA_E:
            for(idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
            {
                valuePtr->daMac[idx] = trafficAf->daMac[idx];
            }
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_MAC_SA_E:
            for(idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
            {
                valuePtr->saMac[idx] = trafficAf->saMac[idx];
            }
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_ADDRESS_TYPE_E:
            valuePtr->addressType = trafficAf->addressType;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_TYPE_E:
            if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->vlanTagType = trafficAf->vlanTagType;
            }
            else 
            {
                 if (prvTgfAfVlanTags == 0)
                 {
                    valuePtr->vlanTagType = PRV_TGF_AF_VLAN_TAG_TYPE_UNTAGGED_E;
                 }
                 else if (prvTgfAfVlanTags == 1)
                 {
                     valuePtr->vlanTagType = PRV_TGF_AF_VLAN_TAG_TYPE_TAGGED_E;
                 }
                 else if (prvTgfAfVlanTags == 2)
                 {
                     valuePtr->vlanTagType = PRV_TGF_AF_VLAN_TAG_TYPE_DOUBLE_TAGGED_E;
                 }
             }
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_0_E:
            valuePtr->vlanTag0 = trafficAf->vlanTag0;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_1_E:
            valuePtr->vlanTag1 = trafficAf->vlanTag1;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_ETHER_TYPE_E:
            if (prvTgfAfPacketInfoPtr != NULL)
            {
                 if (prvTgfAfTrafficType == PRV_TGF_AF_TRAFFIC_TYPE_IP4_E)
                 {
                    trafficAf->etherTypeStc.etherType = TGF_ETHERTYPE_0800_IPV4_TAG_CNS;
                 }
                 else if (prvTgfAfTrafficType == PRV_TGF_AF_TRAFFIC_TYPE_IP6_E)
                 {
                    trafficAf->etherTypeStc.etherType = TGF_ETHERTYPE_86DD_IPV6_TAG_CNS;
                 }
            }
            valuePtr->etherTypeStc = trafficAf->etherTypeStc;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_IPV4_HEADER_E:
            valuePtr->ipv4Header = trafficAf->ipv4Header;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_IPV6_HEADER_E:
            valuePtr->ipv6Header = trafficAf->ipv6Header;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_PACKET_SIZE_E:
            valuePtr->packetSize = trafficAf->packetSize;
            break;
        case PRV_TGF_AF_TRAFFIC_PARAM_DATA_STC_E:
            valuePtr->dataStc.dataPtr = trafficAf->dataStc.dataPtr;
            valuePtr->dataStc.dataLen = trafficAf->dataStc.dataLen;
            break;
        default:
            rc = GT_BAD_PARAM;
    }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfAutoFlowTrafficGet: %d ",
                                 prvTgfDevNum);
}

/**
* @internal tgfAutoFlowBridgePreset function
* @endinternal
*
* @brief   The function presets basic bridge settings for the auto test creation.
*
* @param[in] bridgeParam              - parameter for setting
* @param[in] value                    -  for the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowBridgePreset
(
    IN  PRV_TGF_AF_BRIDGE_PARAM_ENT                    bridgeParam,
    IN  PRV_TGF_AF_BRIDGE_UNT                          value
)
{
    GT_STATUS       rc=GT_OK;
    /*AUTODOC: preset bridge values*/
    switch(bridgeParam)
    {
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_VLAN_E:
            if (value.ingressVlan > 0 && value.ingressVlan < 4096)
            {
                bridgeAf->ingressVlan = value.ingressVlan;
                bridgeAf->ingressVlanValid = GT_TRUE;
            }
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_PORT_IDX_E:
            if (value.ingressPortIdx >= 0 && value.ingressPortIdx < PRV_TGF_PORTS_NUM_CNS)
            {
                bridgeAf->ingressPortIdx = value.ingressPortIdx;
                bridgeAf->ingressPortIdxValid = GT_TRUE;
            }
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_0_PID_E:
            bridgeAf->ingTag0Pid = value.ingTag0Pid;
            bridgeAf->ingTag0PidValid = GT_TRUE;
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_1_PID_E:
            bridgeAf->ingTag1Pid = value.ingTag1Pid;
            bridgeAf->ingTag1PidValid = GT_TRUE;
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAG_0_PID_E:
            bridgeAf->egTag0Pid = value.egTag0Pid;
            bridgeAf->egTag0PidValid = GT_TRUE;
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAG_1_PID_E:
            bridgeAf->egTag1Pid = value.egTag1Pid;
            bridgeAf->egTag1PidValid = GT_TRUE;
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAGGING_CMD_E:
            if (value.egTagCmd >= PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E &&
                 value.egTagCmd <= PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E)
            {
                bridgeAf->egTagCmd = value.egTagCmd;
                bridgeAf->egTagCmdValid = GT_TRUE;
            } 
            break;
        default:
            rc = GT_BAD_PARAM;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfAutoFlowBridgePreset: %d ",
                                 prvTgfDevNum);
}
    
/**
* @internal tgfAutoFlowBridgeGet function
* @endinternal
*
* @brief   The function returns basic bridge settings for the auto test creation.
*         If the parameter has not been set earlier,
*         the recieved value will not necessarily be the one used in the test
*         - randomization might occur while creating the test.
*         for more information - look in the description of the parameters.
* @param[in] bridgeParam              - parameter to get
*
* @param[out] valuePtr                 - (pointer to) value of the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowBridgeGet
(
    IN  PRV_TGF_AF_BRIDGE_PARAM_ENT                    bridgeParam,
    OUT  PRV_TGF_AF_BRIDGE_UNT                        *valuePtr
)
{
    GT_STATUS       rc = GT_OK;
    /*AUTODOC: get bridge values*/
    switch(bridgeParam)
    {
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_VLAN_E:  
            if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->ingressVlan = bridgeAf->ingressVlan;
            }
            else 
            {
                valuePtr->ingressVlan = prvTgfAfVlanId;
            }
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_PORT_IDX_E:
            valuePtr->ingressPortIdx = bridgeAf->ingressPortIdx;
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_0_PID_E:
             if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->ingTag0Pid = bridgeAf->ingTag0Pid;
            }
            else 
            {
                valuePtr->ingTag0Pid = prvTgfAfIngTpidEnt0;
            }
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_1_PID_E:
            if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->ingTag1Pid = bridgeAf->ingTag1Pid;
            }
            else 
            {
                valuePtr->ingTag0Pid = prvTgfAfIngTpidEnt1;
            }
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAG_0_PID_E:
             if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->egTag0Pid = bridgeAf->egTag0Pid;
            }
            else 
            {
                valuePtr->egTag0Pid = prvTgfAfEgTpidEnt0;
            }
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAG_1_PID_E:
            if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->egTag1Pid = bridgeAf->egTag1Pid;
            }
            else 
            {
                valuePtr->egTag1Pid = prvTgfAfEgTpidEnt1;
            }
            break;
        case PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAGGING_CMD_E:
            if (prvTgfAfPacketInfoPtr == NULL)
            {
                valuePtr->egTagCmd = bridgeAf->egTagCmd;
            }
            else 
            {
                valuePtr->egTagCmd = prvTgfAfEgTagCmd;
            }
            break;
        default:
            rc = GT_BAD_PARAM;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfAutoFlowBridgeGet: %d ",
                                 prvTgfDevNum);
}

/**
* @internal tgfAutoFlowRandSeed function
* @endinternal
*
* @brief   The function presets the seed,
*         to be used in the rand process.
*/
GT_VOID tgfAutoFlowRandSeed
(
    IN GT_U32   seed
)
{
    if (prvTgfAfPacketInfoPtr == NULL)
    {
        prvTgfAfSeed = seed;
        prvTgfAfSeedValid = GT_TRUE;
    }
}

/**
* @internal tgfAutoFlowCreate function
* @endinternal
*
* @brief   The function configures the switch & creates the packets to be sent,
*         according to preset, and randomization of undetermined parameters.
*/
GT_VOID tgfAutoFlowCreate
(
    IN GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          portIdx = 0;
    GT_U32          trafficTypeIdx;

    if (prvTgfAfSeedValid == GT_FALSE)
    {
        prvTgfAfSeed = cpssOsRand();
    }
    cpssOsSrand(prvTgfAfSeed);
    PRV_UTF_LOG1_MAC(" tgfAutoFlowCreate seed = [%d] \n", prvTgfAfSeed);
    
    /*randomiaze ingress VLAN ID*/
    if(bridgeAf->ingressVlanValid != GT_TRUE)
    {
        bridgeAf->ingressVlan = (GT_U16)(2 + (cpssOsRand() % PRV_TGF_AF_MAX_VLANS_CNS));
    }
    prvTgfAfVlanId = bridgeAf->ingressVlan;
    /* AUTODOC: SETUP CONFIGURATION: */
    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx], 
                                 &prvTgfAfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);

    /* get default Tpid ingress bitmaps and entries 0,1 ingress and egress directions */ 
    

    rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,                                  /*ingress tag 0 bmp*/
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE0_E,
                                         &prvTgfAfDefIngTpidTag0Bmp);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);

    rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,                                  /*ingress tag 1 bmp*/
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         &prvTgfAfDefIngTpidTag1Bmp);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);

    rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,                                    /*egress tag 0 idx*/
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE0_E,
                                         &prvTgfAfDefEgTpidTag0Idx);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidGet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);

    rc = prvTgfBrgVlanPortEgressTpidGet(prvTgfDevNum,                                     /*egress tag 1 idx*/
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         &prvTgfAfDefEgTpidTag1Idx);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidGet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);


   rc =  prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,0,&prvTgfAfDefIngTpidEnt0);   /*ingress TPID entry 0*/
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                 prvTgfDevNum);

   rc =  prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,1,&prvTgfAfDefIngTpidEnt1);   /*ingress TPID entry 1*/
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                 prvTgfDevNum);

   rc =  prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_EGRESS_E,0,&prvTgfAfDefEgTpidEnt0);     /*egress TPID entry 0*/
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                 prvTgfDevNum);

   rc =  prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_EGRESS_E,1,&prvTgfAfDefEgTpidEnt1);     /*egress TPID entry 1*/
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                 prvTgfDevNum);

   /* set the TPID definitions*/
   prvTgfAfIngTpidEnt0  =   bridgeAf->ingTag0Pid;
   prvTgfAfIngTpidEnt1  =   bridgeAf->ingTag1Pid;
   prvTgfAfEgTpidEnt0   =   bridgeAf->egTag0Pid;
   prvTgfAfEgTpidEnt1   =   bridgeAf->egTag1Pid;

   rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,0,prvTgfAfIngTpidEnt0);
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                 prvTgfDevNum);
   rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,1,prvTgfAfIngTpidEnt1);
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                 prvTgfDevNum);
   rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,0,prvTgfAfEgTpidEnt0);
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                 prvTgfDevNum);
   rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,1,prvTgfAfEgTpidEnt1);
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                 prvTgfDevNum);

   for(portIdx = 0 ; portIdx < PRV_TGF_PORTS_NUM_CNS ; portIdx++)
   {
       rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE0_E,
                                         1);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);
       rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         2);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);
       rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE0_E,
                                         0);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);
       rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                         prvTgfPortsArray[portIdx],
                                         CPSS_VLAN_ETHERTYPE1_E,
                                         1);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx]);
   }
    /* AUTODOC: create (random or preset) VLAN with all ports according to tagging command*/
    prvTgfAfEgTagCmd = bridgeAf->egTagCmd;
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(prvTgfAfVlanId,prvTgfAfEgTagCmd);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                               "ERROR of prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d, vlanId %d",
                                 prvTgfDevNum,prvTgfAfVlanId);

    /* set pvid for all ports, enables randomization of ingress port later. */
    for(portIdx = 0 ; portIdx < PRV_TGF_PORTS_NUM_CNS ; portIdx++)
    {
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, 
                                 prvTgfPortsArray[portIdx], 
                                 prvTgfAfVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d , %d port index",
                                 prvTgfDevNum,portIdx);
    }

   if(trafficAf->trafficTypeValid != GT_TRUE)
    {
        trafficTypeIdx = (cpssOsRand() % 3);
        switch (trafficTypeIdx)
        {
            case 1:
                trafficAf->trafficType = PRV_TGF_AF_TRAFFIC_TYPE_IP4_E;
                break;
            case 2:
                trafficAf->trafficType = PRV_TGF_AF_TRAFFIC_TYPE_IP6_E;
                break;
            default:
                trafficAf->trafficType = PRV_TGF_AF_TRAFFIC_TYPE_L2_E;
        }
    }
   
    prvTgfAfTrafficType = trafficAf->trafficType;
   
   /* AUTODOC: allocate relevant parts according to packet type */
   prvTgfAfPacketPayloadPartPtr = 
       (TGF_PACKET_PAYLOAD_STC *)cpssOsMalloc(sizeof(TGF_PACKET_PAYLOAD_STC ));
   prvTgfAfPacketL2PartPtr = 
       (TGF_PACKET_L2_STC *)cpssOsMalloc(sizeof(TGF_PACKET_L2_STC));
   prvTgfAfPacketInfoPtr= 
       (TGF_PACKET_STC *)cpssOsMalloc(sizeof(TGF_PACKET_STC));
  
   switch (trafficAf->vlanTagType)
   {
        case PRV_TGF_AF_VLAN_TAG_TYPE_UNTAGGED_E:
            prvTgfAfVlanTags = 0;
            break;
        case PRV_TGF_AF_VLAN_TAG_TYPE_TAGGED_E:
            prvTgfAfVlanTags = 1;
            break;
        case PRV_TGF_AF_VLAN_TAG_TYPE_DOUBLE_TAGGED_E:
            prvTgfAfVlanTags = 2;
            break;
        default:
            rc = GT_BAD_PARAM;
   }
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfAutoFlowCreate: %d ",
                                 prvTgfDevNum);
   switch (prvTgfAfTrafficType)
   {
       case PRV_TGF_AF_TRAFFIC_TYPE_L2_E:
            prvTgfAfV4 = 0;
            prvTgfAfV6 = 0;
           break;
       case PRV_TGF_AF_TRAFFIC_TYPE_IP4_E:
            prvTgfAfV4 = 1;
            prvTgfAfV6 = 0;
           break;
       case PRV_TGF_AF_TRAFFIC_TYPE_IP6_E:
            prvTgfAfV6 = 1;
            prvTgfAfV4 = 0;
           break;
       default:
            rc = GT_BAD_PARAM;
   }
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfAutoFlowCreate: %d ",
                                 prvTgfDevNum);
   

   /*headers size*/
   prvTgfAfHeadersSize = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS * prvTgfAfVlanTags
                    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS * prvTgfAfV4
                    + TGF_IPV6_HEADER_SIZE_CNS * prvTgfAfV6;

   prvTgfAfPacketPartsNum =  1     /*L2*/
                 +prvTgfAfVlanTags /*vlan tags 0/1/2 */
                 +1                /*EtherType*/
                 +prvTgfAfV4       /*header if IPv4*/
                 +prvTgfAfV6       /*header if IPv6*/
                 +1;               /*payload*/
   if (prvTgfAfPacketPartArray != NULL)
   {
       cpssOsFree(prvTgfAfPacketPartArray);
   }
   prvTgfAfPacketPartArray = 
       (TGF_PACKET_PART_STC *)cpssOsMalloc(sizeof(TGF_PACKET_PART_STC)*prvTgfAfPacketPartsNum);

}
/**
* @internal tgfAutoFlowTxSetup function
* @endinternal
*
* @brief   The function sets the transmission of a single flow.
*/
GT_VOID tgfAutoFlowTxSetup
(
    IN GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      idx;
    GT_U32      dataSize;
    GT_U32      flag;
    
    /* AUTODOC: randomize flow data and settings */

    /* randomize ingress port */
    if ( bridgeAf->ingressPortIdxValid != GT_TRUE )
    {
        bridgeAf->ingressPortIdx = (cpssOsRand() % PRV_TGF_PORTS_NUM_CNS);
    }
    /* randomize packets number*/
    if ( trafficAf->burstCountValid != GT_TRUE )
    {
        trafficAf->burstCount = 1 + (cpssOsRand() % PRV_TGF_AF_MAX_BURST_COUNT_CNS);
    }
    /* randomize MAC da */
    if ( trafficAf->daMacValid != GT_TRUE )
    {
        /* random UC/MC/BC */
        if (trafficAf->addressTypeValid != GT_TRUE)
        {
            trafficAf->daMac[0] = (GT_U8)(cpssOsRand() % 3);
        }
        else 
        {
            trafficAf->daMac[0] = (GT_U8)trafficAf->addressType;
        }
        if (trafficAf->daMac[0] == 2)
        {
            for(idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
            {
                trafficAf->daMac[idx] = 0xff;
            }
        }
        /* random destination address */
        else
        {
            for(idx = 1 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
            {
                trafficAf->daMac[idx] = (GT_U8)(cpssOsRand() % 256);
            }
        }
    }
   
    /* randomize MAC sa */
    if ( trafficAf->saMacValid != GT_TRUE )
    {
        trafficAf->saMac[0] = 0x00;
        for(idx = 1 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
        {
            trafficAf->saMac[idx] = (GT_U8)(cpssOsRand() % 256);
        }
    }
    /* if da == sa change da to MC */
    for (flag = 0, idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++)
    {
        if(trafficAf->daMac[idx] == trafficAf->saMac[idx])
        {
            flag++;
        }
        else 
        {
           idx = PRV_TGF_AF_MAC_ADDR_LEN_CNS;
        }
    }
    if(flag == 6)
    {
        trafficAf->daMac[0] = 0x01;
    }
    for (idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS ; idx++) 
    {
        prvTgfAfPacketL2PartPtr->daMac[idx] = trafficAf->daMac[idx];
        prvTgfAfPacketL2PartPtr->saMac[idx] = trafficAf->saMac[idx];
    }
    /* set vlan tag 0 struct */
    if (prvTgfAfVlanTags > 0) 
    {
        /* vid = ingress vlan, if not preset */
        if (trafficAf->vlanTag0Valid != GT_TRUE)
        {
            trafficAf->vlanTag0.etherType = prvTgfAfIngTpidEnt0;
            trafficAf->vlanTag0.vid = prvTgfAfVlanId;
        }
        prvTgfAfPacketTag0PartPtr = &(trafficAf->vlanTag0);
    }
    /* set vlan tag 1 struct */
    if (prvTgfAfVlanTags == 2) 
    {
        /* vid is random, if not preset */
        if (trafficAf->vlanTag1Valid != GT_TRUE)
        {
            trafficAf->vlanTag1.vid = (GT_U16)( 2 +(cpssOsRand() % PRV_TGF_AF_MAX_VLANS_CNS));
            trafficAf->vlanTag1.etherType = prvTgfAfIngTpidEnt1;
        }
        prvTgfAfPacketTag1PartPtr = &(trafficAf->vlanTag1);
    }
    /* ethertype randomization */
    if (prvTgfAfTrafficType == PRV_TGF_AF_TRAFFIC_TYPE_L2_E)
    {
    
        if (trafficAf->etherTypeStcValid != GT_TRUE)
        {
            trafficAf->etherTypeStc.etherType = (TGF_ETHER_TYPE)
                        (cpssOsRand() % PRV_TGF_AF_MAX_GTU16_CNS);
            switch (trafficAf->etherTypeStc.etherType)
            {
                case TGF_ETHERTYPE_8100_VLAN_TAG_CNS    :
                case TGF_ETHERTYPE_9100_TAG_CNS         :
                case TGF_ETHERTYPE_8847_MPLS_TAG_CNS    :
                case TGF_ETHERTYPE_0800_IPV4_TAG_CNS    :
                case TGF_ETHERTYPE_86DD_IPV6_TAG_CNS    :
                case TGF_ETHERTYPE_0806_ARP_TAG_CNS     :
                case TGF_ETHERTYPE_88E7_MIM_TAG_CNS     :
                case TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS :
                case TGF_ETHERTYPE_22F3_TRILL_TAG_CNS   :
                case TGF_ETHERTYPE_88E5_MACSEC_TAG_CNS  :
                case TGF_ETHERTYPE_8902_OAM_TAG_CNS     :
                case TGF_ETHERTYPE_8906_OAM_TAG_CNS     :
                case TGF_ETHERTYPE_8808_FC_TAG_CNS      : 
                case TGF_ETHERTYPE_NON_VALID_TAG_CNS    :
                    trafficAf->etherTypeStc.etherType = 0xeeee;
                    break;
            }
        }
        else if (trafficAf->etherTypeStc.etherType == TGF_ETHERTYPE_0800_IPV4_TAG_CNS 
                 ||trafficAf->etherTypeStc.etherType == TGF_ETHERTYPE_86DD_IPV6_TAG_CNS)
        {
            trafficAf->etherTypeStc.etherType = 0xeeee;
        }
    }
    /* correlation in UC/MC in L2 & L3 */
    if (prvTgfAfTrafficType == PRV_TGF_AF_TRAFFIC_TYPE_IP4_E)
    {
        if (trafficAf->ipv4HeaderValid != GT_TRUE)
        {
            trafficAf->ipv4Header.srcAddr[0] = 1;
            for(idx = 1 ; idx < PRV_TGF_AF_IPV4_ADDR_LEN_CNS ; idx++)
            {
                trafficAf->ipv4Header.srcAddr[idx] = (GT_U8)(cpssOsRand() % 256);
                trafficAf->ipv4Header.dstAddr[idx] = (GT_U8)(cpssOsRand() % 256);
            }
            if (trafficAf->daMac[0] == 0x01)
            {
                trafficAf->ipv4Header.dstAddr[0] = 224;
            }
            else 
            {
                trafficAf->ipv4Header.dstAddr[0] = 1;
            }
        }
        else if(trafficAf->daMacValid != GT_TRUE && trafficAf->addressTypeValid != GT_TRUE)
        {
            if (trafficAf->ipv4Header.dstAddr[0] == 224)
            {
                trafficAf->daMac[0] = 0x01;
            }
            else
            {
                trafficAf->daMac[0] = 0x00;
            }
        }
        trafficAf->etherTypeStc.etherType = TGF_ETHERTYPE_0800_IPV4_TAG_CNS;
    }
    if (prvTgfAfTrafficType == PRV_TGF_AF_TRAFFIC_TYPE_IP6_E)
    {
        if (trafficAf->ipv6HeaderValid != GT_TRUE)
        {
            trafficAf->ipv6Header.nextHeader = (TGF_NEXT_HEADER)(cpssOsRand() % PRV_TGF_AF_MAX_GTU16_CNS);
            trafficAf->ipv6Header.srcAddr[0] = 1;
            for(idx = 1 ; idx < PRV_TGF_AF_IPV6_ADDR_LEN_CNS ; idx++)
            {
                trafficAf->ipv6Header.srcAddr[idx] = (GT_U16)(cpssOsRand() % PRV_TGF_AF_MAX_GTU16_CNS);
                trafficAf->ipv6Header.dstAddr[idx] = (GT_U16)(cpssOsRand() % PRV_TGF_AF_MAX_GTU16_CNS);
            }
            if (trafficAf->daMac[0] == 0x01)
            {
                trafficAf->ipv6Header.dstAddr[0] = 0xff00;
            }
            else 
            {
                trafficAf->ipv6Header.dstAddr[0] = 1;
            }
        }
        else if(trafficAf->daMacValid != GT_TRUE && trafficAf->addressTypeValid != GT_TRUE)
        {
            if (trafficAf->ipv6Header.dstAddr[0] == 0xff00)
            {
                trafficAf->daMac[0] = 0x01;
            }
            else
            {
                trafficAf->daMac[0] = 0x00;
            }
        }
        trafficAf->etherTypeStc.etherType = TGF_ETHERTYPE_86DD_IPV6_TAG_CNS;
    }
    
    /* randomize data */
    if ( trafficAf->dataStcValid != GT_TRUE )
    {
        /*packet size*/
        if ( trafficAf->packetSizeValid != GT_TRUE )
        {
            trafficAf->packetSize = PRV_TGF_AF_PACKET_MIN_SIZE_CNS + 
                cpssOsRand() % (PRV_TGF_AF_PACKET_MAX_SIZE_CNS - PRV_TGF_AF_PACKET_MIN_SIZE_CNS);
        }
        dataSize = trafficAf->packetSize - prvTgfAfHeadersSize;
        if (trafficAf->dataStc.dataPtr != NULL)
        {
            cpssOsFree(trafficAf->dataStc.dataPtr);
        }
        trafficAf->dataStc.dataPtr = (GT_U8 *)cpssOsMalloc(dataSize);
        /*rand data*/
        for(idx = 0 ; idx < dataSize ; idx++)
        {
            trafficAf->dataStc.dataPtr[idx] = (GT_U8)(cpssOsRand() % 256);
        }
    }
    else /*preset data*/
    {
        dataSize = trafficAf->dataStc.dataLen;
        trafficAf->packetSize = dataSize + prvTgfAfHeadersSize;
    }


   /*========== PAYLOAD part ==========*/
    prvTgfAfPacketPayloadPartPtr->dataLength = dataSize;                       /* dataLength */
    prvTgfAfPacketPayloadPartPtr->dataPtr = trafficAf->dataStc.dataPtr;                 /* dataPtr */

   /*========== ETHERTYPE part ==========*/
    prvTgfAfPacketEtherTypePartPtr = &(trafficAf->etherTypeStc);

    /*========== IPv4 part ==========*/
    prvTgfAfPacketIpV4PartPtr = &(trafficAf->ipv4Header);

    /*========== IPv6 part ==========*/
    prvTgfAfPacketIpV6PartPtr = &(trafficAf->ipv6Header);

    /*========== PARTS of packet ==========*/

    idx = 0;
    prvTgfAfPacketPartArray[idx].type = TGF_PACKET_PART_L2_E;
    prvTgfAfPacketPartArray[idx].partPtr = prvTgfAfPacketL2PartPtr;
    idx++;
    if (prvTgfAfVlanTags > 0)
    {
        prvTgfAfPacketPartArray[idx].type = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfAfPacketPartArray[idx].partPtr = prvTgfAfPacketTag0PartPtr;
        idx++;
        if (prvTgfAfVlanTags == 2)
        {
            prvTgfAfPacketPartArray[idx].type = TGF_PACKET_PART_VLAN_TAG_E;
            prvTgfAfPacketPartArray[idx].partPtr = prvTgfAfPacketTag1PartPtr;
            idx++;
        }
    }
    prvTgfAfPacketPartArray[idx].type = TGF_PACKET_PART_ETHERTYPE_E;
    prvTgfAfPacketPartArray[idx].partPtr = prvTgfAfPacketEtherTypePartPtr;
    idx++;
    if (prvTgfAfV4 == 1)
    {
        prvTgfAfPacketPartArray[idx].type = TGF_PACKET_PART_IPV4_E;
        prvTgfAfPacketPartArray[idx].partPtr = prvTgfAfPacketIpV4PartPtr;
        idx++;
    }
    if (prvTgfAfV6 == 1)
    {
        prvTgfAfPacketPartArray[idx].type = TGF_PACKET_PART_IPV6_E;
        prvTgfAfPacketPartArray[idx].partPtr = prvTgfAfPacketIpV6PartPtr;
        idx++;
    }
    prvTgfAfPacketPartArray[idx].type = TGF_PACKET_PART_PAYLOAD_E;
    prvTgfAfPacketPartArray[idx].partPtr = prvTgfAfPacketPayloadPartPtr;


    /*========== PACKET to send ==========*/
     prvTgfAfPacketInfoPtr->totalLen = trafficAf->packetSize;                    /* totalLen */
     prvTgfAfPacketInfoPtr->numOfParts = prvTgfAfPacketPartsNum;                 /* numOfParts */
     prvTgfAfPacketInfoPtr->partsArray = prvTgfAfPacketPartArray;                /* partsArray */
                

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfAfPacketInfoPtr, 
                             trafficAf->burstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", 
                                 prvTgfDevNum);

}

/**
* @internal tgfAutoFlowTxStart function
* @endinternal
*
* @brief   The function starts the transmission of a single flow.
*/
GT_VOID tgfAutoFlowTxStart
(
    IN GT_VOID
)
{
    GT_STATUS       rc;
    /* AUTODOC: start transmission via ingressPortIdx */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                   prvTgfPortsArray[bridgeAf->ingressPortIdx]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, 
                                 prvTgfPortsArray[bridgeAf->ingressPortIdx]);                                 
    
}

/**
* @internal tgfAutoFlowDel function
* @endinternal
*
* @brief   The function resets all the configurations.
*/
GT_VOID tgfAutoFlowDel
(
    IN GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portIdx;

    /* AUTODOC: RESTORE SWITCH CONFIGURATION: */


    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", 
                                 prvTgfDevNum, GT_TRUE);
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfAfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfAfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfAfVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, prvTgfAfVlanId);
    
    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* restore all TPID configurations */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,0,prvTgfAfDefIngTpidEnt0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                  prvTgfDevNum);
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,1,prvTgfAfDefIngTpidEnt1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                  prvTgfDevNum);
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,0,prvTgfAfDefEgTpidEnt0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                  prvTgfDevNum);
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_EGRESS_E,1,prvTgfAfDefEgTpidEnt1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d",
                                  prvTgfDevNum);

    for(portIdx = 0 ; portIdx < PRV_TGF_PORTS_NUM_CNS ; portIdx++)
    {
     rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                          prvTgfPortsArray[portIdx],
                                          CPSS_VLAN_ETHERTYPE0_E,
                                          prvTgfAfDefIngTpidTag0Bmp);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d",
                                  prvTgfDevNum, 
                                  prvTgfPortsArray[portIdx]);
     rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,
                                          prvTgfPortsArray[portIdx],
                                          CPSS_VLAN_ETHERTYPE1_E,
                                          prvTgfAfDefIngTpidTag1Bmp);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d, %d",
                                  prvTgfDevNum, 
                                  prvTgfPortsArray[portIdx]);
     rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                          prvTgfPortsArray[portIdx],
                                          CPSS_VLAN_ETHERTYPE0_E,
                                          prvTgfAfDefEgTpidTag0Idx);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d",
                                  prvTgfDevNum, 
                                  prvTgfPortsArray[portIdx]);
     rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum,
                                          prvTgfPortsArray[portIdx],
                                          CPSS_VLAN_ETHERTYPE1_E,
                                          prvTgfAfDefEgTpidTag1Idx);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet: %d, %d",
                                  prvTgfDevNum, 
                                  prvTgfPortsArray[portIdx]);
   }




    /* AUTODOC: free allocated memory */
        cpssOsFree(prvTgfAfPacketInfoPtr);
        if (prvTgfAfPacketPartArray != NULL)
        {
            cpssOsFree(prvTgfAfPacketPartArray);
        }
        cpssOsFree(prvTgfAfPacketL2PartPtr);
        cpssOsFree(prvTgfAfPacketPayloadPartPtr);
       
    if((trafficAf->dataStcValid != GT_TRUE) && (trafficAf->dataStc.dataPtr != NULL))
    {   
        cpssOsFree(trafficAf->dataStc.dataPtr);
    }



    prvTgfAfIngTpidEnt0 = 0;
    prvTgfAfIngTpidEnt1 = 0;
    prvTgfAfEgTpidEnt0 = 0;
    prvTgfAfEgTpidEnt1 = 0;
    prvTgfAfEgTagCmd = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    prvTgfAfDefIngTpidEnt0 = 0x8100;
    prvTgfAfDefIngTpidEnt1 = 0x8100;
    prvTgfAfDefEgTpidEnt0 = 0x8100;
    prvTgfAfDefEgTpidEnt1 = 0x8100;
    prvTgfAfDefIngTpidTag0Bmp = 255;
    prvTgfAfDefIngTpidTag1Bmp = 255;
    prvTgfAfDefEgTpidTag0Idx = 0;
    prvTgfAfDefEgTpidTag1Idx = 0;
    prvTgfAfV4 = 0;
    prvTgfAfV6 = 0;
    prvTgfAfVlanTags = 0;
    prvTgfAfPacketPartsNum = 0;
    prvTgfAfTrafficType = PRV_TGF_AF_TRAFFIC_TYPE_L2_E;
    prvTgfAfSeedValid = GT_FALSE;

    /*AUTODOC: reset AutoFlow DataBase*/
    /* reset traffic & bridge structs */
    tgfAutoFlowLibInit();

    prvTgfAfPacketPayloadPartPtr = NULL;
    prvTgfAfPacketPartArray = NULL;
    prvTgfAfPacketL2PartPtr = NULL;
    prvTgfAfPacketTag0PartPtr = NULL;
    prvTgfAfPacketTag1PartPtr = NULL;
    prvTgfAfPacketEtherTypePartPtr = NULL;
    prvTgfAfPacketIpV4PartPtr = NULL;
    prvTgfAfPacketIpV6PartPtr = NULL;
    prvTgfAfPacketInfoPtr = NULL;
}
/**
* @internal tgfAutoFlowLibInit function
* @endinternal
*
* @brief   The function initializes the auto flow library,
*         and allocates the dynamic data base.
*/
GT_VOID tgfAutoFlowLibInit
(
    IN GT_VOID
)
{
    GT_U32 idx;
    /* AUTODOC: allocate and initialize AutoFlow DataBase */
    /*trafficAf*/
    if (trafficAf == NULL)
    {
        trafficAf = (PRV_TGF_AF_TRAFFIC_STC *)cpssOsMalloc(sizeof(PRV_TGF_AF_TRAFFIC_STC));
    }
    /* default PRV_TGF_AF_TRAFFIC_PARAM_TRAFFIC_TYPE_E */    
    trafficAf->trafficType = PRV_TGF_AF_TRAFFIC_TYPE_L2_E;                             
    /* default number of packets */
    trafficAf->burstCount = 1;                                
    /* default daMac and saMac */
    for (idx = 0 ; idx < PRV_TGF_AF_MAC_ADDR_LEN_CNS - 1 ; idx++)
    { 
        trafficAf->daMac[idx] = 0x00;  
        trafficAf->saMac[idx] = 0x00; 
    }
    trafficAf->daMac[idx] = 0x01;
    trafficAf->saMac[idx] = 0x02;
    /* default UC */
    trafficAf->addressType = PRV_TGF_AF_ADDRESS_TYPE_UC_E;                          
    /* default packet tagging */
    trafficAf->vlanTagType = PRV_TGF_AF_VLAN_TAG_TYPE_UNTAGGED_E;             
    /* default tag 0 PID = 0x8100 */
    trafficAf->vlanTag0.etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
    trafficAf->vlanTag0.pri = 0;
    trafficAf->vlanTag0.cfi = 0;
    trafficAf->vlanTag0.vid = 1;                              
    /* default tag 1 PID = 0x8100 */
    trafficAf->vlanTag1.etherType = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;
    trafficAf->vlanTag1.pri = 0;
    trafficAf->vlanTag1.cfi = 0;
    trafficAf->vlanTag1.vid = 1;  
    /* default EtherType */
    trafficAf->etherTypeStc.etherType = 0xEEEE;               
    /* default IPv4 header */
    trafficAf->ipv4Header.version = 4;
    trafficAf->ipv4Header.headerLen = 5;
    trafficAf->ipv4Header.typeOfService = 0;
    trafficAf->ipv4Header.totalLen = 0x2A;
    trafficAf->ipv4Header.id = 0;
    trafficAf->ipv4Header.flags = 0;
    trafficAf->ipv4Header.offset = 0;
    trafficAf->ipv4Header.timeToLive = 0x40;
    trafficAf->ipv4Header.protocol = 0x04;
    trafficAf->ipv4Header.csum = TGF_PACKET_AUTO_CALC_CHECKSUM_CNS;
    for (idx = 0 ; idx < PRV_TGF_AF_IPV4_ADDR_LEN_CNS ; idx++)
    {
        trafficAf->ipv4Header.srcAddr[idx] = 1;
        trafficAf->ipv4Header.dstAddr[idx] = 2;
    }                                                        
    /* default IPv6 header */
    trafficAf->ipv6Header.version = 6;
    trafficAf->ipv6Header.trafficClass = 0;
    trafficAf->ipv6Header.flowLabel = 0;
    trafficAf->ipv6Header.payloadLen = 0x02;
    trafficAf->ipv6Header.nextHeader = 0x03;
    trafficAf->ipv6Header.hopLimit = 0x40;
    for(idx = 0 ; idx < PRV_TGF_AF_IPV6_ADDR_LEN_CNS ; idx++)
    {
        trafficAf->ipv6Header.srcAddr[idx] = 0x11;
        trafficAf->ipv6Header.dstAddr[idx] = 0x22;
    }                                                       
    /* default packet size */
    trafficAf->packetSize = PRV_TGF_AF_PACKET_MIN_SIZE_CNS;                                                    
    /* default dataStc */
    trafficAf->dataStc.dataPtr = NULL;
    trafficAf->dataStc.dataLen = 0;                           
    /*invalidity:*/
    trafficAf->burstCountValid = GT_FALSE;
    trafficAf->daMacValid = GT_FALSE;
    trafficAf->dataStcValid = GT_FALSE;
    trafficAf->etherTypeStcValid = GT_FALSE;
    trafficAf->ipv4HeaderValid = GT_FALSE;
    trafficAf->ipv6HeaderValid = GT_FALSE;
    trafficAf->packetSizeValid = GT_FALSE;
    trafficAf->saMacValid = GT_FALSE;
    trafficAf->trafficTypeValid = GT_FALSE;
    trafficAf->addressTypeValid = GT_FALSE;
    trafficAf->vlanTag0Valid = GT_FALSE;
    trafficAf->vlanTag1Valid = GT_FALSE;
    trafficAf->vlanTagTypeValid = GT_FALSE;
    /*bridgeAf*/
    if (bridgeAf == NULL)
    {
        bridgeAf = (PRV_TGF_AF_BRIDGE_STC *)cpssOsMalloc(sizeof(PRV_TGF_AF_BRIDGE_STC));
    }
    /* default ingress Vlan */
    bridgeAf->ingressVlan = 1;  
    /* default ingress port index */
    bridgeAf->ingressPortIdx = 0;    
    /* default ingress vlan tag 0 pid */
    bridgeAf->ingTag0Pid = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;   
    /* default ingress vlan tag 1 pid */
    bridgeAf->ingTag1Pid = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;  
    /* default egress vlan tag 0 pid */
    bridgeAf->egTag0Pid = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;  
     /* default egress vlan tag 1 pid */
    bridgeAf->egTag1Pid = TGF_ETHERTYPE_8100_VLAN_TAG_CNS;   
    /* default egress tagging command */
    bridgeAf->egTagCmd = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    /*invalidity:*/
    bridgeAf->egTag0PidValid = GT_FALSE;
    bridgeAf->egTag1PidValid = GT_FALSE;
    bridgeAf->egTagCmdValid = GT_FALSE;
    bridgeAf->ingressPortIdxValid = GT_FALSE;
    bridgeAf->ingressVlanValid = GT_FALSE;
    bridgeAf->ingTag0PidValid = GT_FALSE;
    bridgeAf->ingTag1PidValid = GT_FALSE;

}

/**
* @internal tgfAutoFlowLibClose function
* @endinternal
*
* @brief   The function closes the auto flow library,
*         and frees the allocated memory for the data base.
*/
GT_VOID tgfAutoFlowLibClose
(
    IN GT_VOID
)
{
    /* AUTODOC: free allocated Data Base */
    if (trafficAf != NULL)
    {
        cpssOsFree(trafficAf);
        trafficAf = NULL;
    }
    if (bridgeAf != NULL)
    {
        cpssOsFree(bridgeAf);
        bridgeAf = NULL;
    }
}


