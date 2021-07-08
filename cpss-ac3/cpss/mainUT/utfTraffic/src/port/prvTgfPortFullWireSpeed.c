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
* @file prvTgfPortFullWireSpeed.c
*
* @brief Port Tx Queue features testing
*
* @version   22
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
#include <port/prvTgfPortFWS.h>
#include <common/tgfVntGen.h>
#include <common/tgfTcamGen.h>
#include <port/prvTgfPortIfModeSpeed.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxDba.h>
#endif
/* global settings for restore configuration */
static GT_U16 prvTgfFWSFirstVlan = 0;
static GT_U16 prvTgfFWSLastVlan = 0;
static GT_U32 prvTgfFWSPortsNum = 0;
static CPSS_PORT_SPEED_ENT  prvTgfPortSpeedArr[CPSS_MAX_PORTS_NUM_CNS];
static GT_U32 prvTgfFWSUseAllPortsInVlan = 0;
/* open log */
GT_U32 prvTgfPortFWSOpenLog = 1;
#define PORT_FWS_PRV_UTF_LOG0_MAC \
    if(prvTgfPortFWSOpenLog) PRV_UTF_LOG0_MAC

#define PORT_FWS_PRV_UTF_LOG1_MAC \
    if(prvTgfPortFWSOpenLog) PRV_UTF_LOG1_MAC

#define PORT_FWS_PRV_UTF_LOG2_MAC \
    if(prvTgfPortFWSOpenLog) PRV_UTF_LOG2_MAC

/* L2 part of packet 1 */
static TGF_PACKET_L2_STC prvTgfPacketSampleL2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x11},               /* dstMac */
    {0x00, 0x88, 0x99, 0x33, 0x44, 0x11}                /* srcMac */
};

/* Ethernet type part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEhertypePart =
{
    0x3456
};

/* skip this ports from loop configuration */
CPSS_PORTS_BMP_STC  prvTgfPortFwsSkipPortsBmp;

/* boolean parameter to skip ports */
GT_U32 prvTgfPortFwsSkipPorts = 0;

/* acceptable deviation of port rates */
GT_U32 prvTgfFwsRateDeviation = 2;

/* acceptable difference of port rate and theoretical one in percentage */
GT_U32 prvTgfFwsRateDiff = 1;

/* Bobcat2 devices and above may generate FWS traffic with speed more then line rate.
   xCat3 ports 24-27 behaves same. */
GT_U32 prvTgfFwsOverRateDiff = 40;

/************************************ FWS PACKET **************************************************/

/* DATA of packet */
static GT_U8 prvTgfPayloadFWSDataArr[] =
 {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08 ,0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18 ,0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
   0x28 ,0x29, 0x2A, 0x2B, 0x2C, 0x2D,
 };

static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadFWSPart = {
    sizeof(prvTgfPayloadFWSDataArr),                       /* dataLength */
    prvTgfPayloadFWSDataArr                                /* dataPtr */
};


static TGF_PACKET_PART_STC prvTgfPacketPartsFWSArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketSampleL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEhertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadFWSPart}
};

/* LENGTH of double tagged packet */
#define PRV_TGF_PACKET_LEN_FWS_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadFWSDataArr)

/* TGF_CRC_LEN_CNS */

/* Double tagged PACKET to send */
static TGF_PACKET_STC  prvTgfPacketFWSInfoB =
{
    PRV_TGF_PACKET_LEN_FWS_CNS,                                         /* totalLen */
    (sizeof(prvTgfPacketPartsFWSArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    prvTgfPacketPartsFWSArray                                          /* partsArray */
};

/**
* @internal prvTgfPortFWSSkipPort function
* @endinternal
*
* @brief   Debug tool to remove or add ports from/to the FWS test
*/
GT_U32  prvTgfPortFWSSkipPort(GT_PHYSICAL_PORT_NUM    portNum, GT_U32 enable)
{
    GT_U32 rc = prvTgfPortFwsSkipPorts;
    if (prvTgfPortFwsSkipPorts == 0)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&prvTgfPortFwsSkipPortsBmp);

        prvTgfPortFwsSkipPorts = 1;
    }

    if (enable)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&prvTgfPortFwsSkipPortsBmp, portNum);
    }
    else
    {
        CPSS_PORTS_BMP_PORT_CLEAR_MAC(&prvTgfPortFwsSkipPortsBmp, portNum);
    }

    return rc;
}

/* list of ports to be 10G */
static GT_PHYSICAL_PORT_NUM bc3_portNumFor10G[4] = {20,21,22,23};

/**
* @internal prvTgfFWSPortsConfig function
* @endinternal
*
* @brief   Configure ports for FWS test.
*
* @param[in] devNum                   - device number
*
* @param[out] portSpeedArr[CPSS_MAX_PORTS_NUM_CNS] - array of port speed per port
*                                       None
*/
static GT_STATUS prvTgfFWSPortsConfig
(
    IN GT_U8                 devNum,
    OUT CPSS_PORT_SPEED_ENT  portSpeedArr[CPSS_MAX_PORTS_NUM_CNS]
)
{
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_STATUS               rc;
    GT_U32                  jj;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
         /* Pipe 0 and pipe 1 have too big bandwidth .
         Disable ports from FWS tests to reach 352G bandwidthon on each Pipe (704 over all). */
        /* Pipe 0 and pipe 1 ,data path 0,1,2 have 110, 120, 120:*/
         for (jj = 0 ; jj < sizeof(bc3_portNumFor10G)/ sizeof(bc3_portNumFor10G[0]) ; jj++ )
         {
             rc = prvTgfPortIfModeSpeedPortConfig(devNum, bc3_portNumFor10G[jj], CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
             if(rc != GT_OK)
             {
                 PRV_UTF_LOG3_MAC("UTF: error : prvTgfPortIfModeSpeedPortConfig:  port [%d] , mode %d, speed %d\n" ,
                                    bc3_portNumFor10G[jj],CPSS_PORT_INTERFACE_MODE_KR_E,CPSS_PORT_SPEED_10000_E);
                 return rc;
             }
         }
    }


    /*  mark all ports with invalid speed.
        this is sign that port is not exist or cannot be used. */
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        /*  mark all ports with invalid speed.
            this is sign that port is not exist or cannot be used. */
        portSpeedArr[portNum] = CPSS_PORT_SPEED_NA_E;

        if (CPSS_CPU_PORT_NUM_CNS == portNum)
        {
            /* CPU port cannot be used in the loop */
            continue;
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E &&
           portNum >= 32)
        {
            /* skip fabric ports */
            continue;
        }

        if (prvTgfPortFwsSkipPorts == 1)
        {
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&prvTgfPortFwsSkipPortsBmp, portNum))
            {
                continue;
            }
        }

        if (prvUtfIsTrafficManagerUsed() && (portNum >= 128))
        {
            /* skip interlacken channels */
            continue;
        }

        /* force link UP and configure maximal speed on port */
        rc = prvTgfPortMaxSpeedForce(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            /* port is not exist or cannot be used in the loop */
            continue;
        }

        rc = prvTgfPortSpeedGet(devNum, portNum, &portSpeedArr[portNum]);
        if (rc != GT_OK)
        {
            prvTgfPortMaxSpeedForce(devNum, portNum, GT_FALSE);
            return rc;
        }

        /* skip not configured ports */
        if (portSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            prvTgfPortMaxSpeedForce(devNum, portNum, GT_FALSE);
            continue;
        }

        portInterface.type = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum = devNum;
        portInterface.devPort.portNum = portNum;

        /* enable loopback on all ports */
        rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvTgfFWSPortSpeedGroupGet function
* @endinternal
*
* @brief   Calculate speed group per port.
*
* @param[in] portSpeedArr[CPSS_MAX_PORTS_NUM_CNS] - array of port speed per port
*
* @param[out] portSpeedGroupArr[CPSS_MAX_PORTS_NUM_CNS] - array of port speed group per port.
* @param[out] numOfGroupsPtr           - pointer to number of speed groups.
* @param[out] numOfPortsPtr            - pointer to number of ports.
*                                       None
*/
static GT_STATUS prvTgfFWSPortSpeedGroupGet
(
    IN CPSS_PORT_SPEED_ENT   portSpeedArr[CPSS_MAX_PORTS_NUM_CNS],
    OUT GT_U32               portSpeedGroupArr[CPSS_MAX_PORTS_NUM_CNS],
    OUT GT_U32              *numOfGroupsPtr,
    OUT GT_U32              *numOfPortsPtr
)
{
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  usedSpeedArr[CPSS_PORT_SPEED_NA_E + 1];
    GT_U32                  speedGroupArr[CPSS_PORT_SPEED_NA_E + 1];
    GT_U32                  i;

    cpssOsMemSet(usedSpeedArr,0, sizeof(usedSpeedArr));
    cpssOsMemSet(speedGroupArr,0xFF, sizeof(speedGroupArr));

    *numOfPortsPtr = 0;

    /* get statistic about used speeds */
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        usedSpeedArr[portSpeedArr[portNum]]++;
        if (portSpeedArr[portNum] != CPSS_PORT_SPEED_NA_E)
        {
            (*numOfPortsPtr)++;
        }
    }

    *numOfGroupsPtr = 0;
    /* calculate number of groups and group index */
    for (i = 0; i < CPSS_PORT_SPEED_NA_E; i++)
    {
        if (usedSpeedArr[i])
        {
            speedGroupArr[i] = *numOfGroupsPtr;
            (*numOfGroupsPtr)++;
        }
    }

    /* assign speed group to port */
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        portSpeedGroupArr[portNum] = speedGroupArr[portSpeedArr[portNum]];
    }

    return GT_OK;
}

/**
* @internal prvTgfFWSFDBAndVlanLoopConfig function
* @endinternal
*
* @brief   Create VLAN loops and add FDB entries.
*
* @param[in] devNum                   - device number
* @param[in] configEnable             - GT_TRUE configure all needed setting and send traffic
*                                      - GT_FALSE do not change configuation and send traffic
* @param[in] portSpeedArr[CPSS_MAX_PORTS_NUM_CNS] - array of port speed per port
* @param[in] portSpeedGroupArr[CPSS_MAX_PORTS_NUM_CNS] - array of port speed group per port.
* @param[in] firstVlanId              - first VID for configuration
* @param[in] numOfSpeedGroups         - number of speed groups
* @param[in,out] macAddrArr[]             - MAC DA for speed group
*                                      numOfPortsInUse  - number of ports
* @param[in,out] macAddrArr[]             - MAC DA for speed group
*
* @param[out] firstPortInGroupArr[CPSS_PORT_SPEED_NA_E] - array of first ports per speed group
*                                       None
*/
static GT_STATUS prvTgfFWSFDBAndVlanLoopConfig
(
    IN GT_U8                 devNum,
    IN GT_BOOL               configEnable,
    IN CPSS_PORT_SPEED_ENT   portSpeedArr[CPSS_MAX_PORTS_NUM_CNS],
    IN GT_U32                portSpeedGroupArr[CPSS_MAX_PORTS_NUM_CNS],
    IN GT_U16                firstVlanId,
    IN GT_U32                numOfSpeedGroups,
    INOUT TGF_MAC_ADDR       macAddrArr[],
    OUT GT_PHYSICAL_PORT_NUM firstPortInGroupArr[CPSS_PORT_SPEED_NA_E]
)
{
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  group;
    GT_PHYSICAL_PORT_NUM    groupMembersArr[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                  currentMemberNum;
    GT_U16                  currentVlan = firstVlanId;
    GT_U32                  ii;
    GT_STATUS               rc;
    GT_U32                  portsArray[2];
    GT_U32                  useAllPortsArray[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                  allMemberNum;

    allMemberNum = 0;

    for (group = 0; group < numOfSpeedGroups; group++)
    {
        if (group > 0)
        {
            cpssOsMemCpy(&(macAddrArr[group][0]),&(macAddrArr[group - 1][0]), sizeof(TGF_MAC_ADDR));
            macAddrArr[group][5] += 1;
        }

        currentMemberNum = 0;

        /* get group members */
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (portSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
            {
                continue;
            }

            if (group != portSpeedGroupArr[portNum])
            {
                continue;
            }

            groupMembersArr[currentMemberNum++] = portNum;
            useAllPortsArray[allMemberNum] = portNum;
            allMemberNum ++;
        }

        if (currentMemberNum == 0)
        {
            /* there is no members in a group */
            continue;
        }

        firstPortInGroupArr[group] = groupMembersArr[0];

        /* skip configuration if need */
        if (configEnable == GT_FALSE)
            continue;
        /* skip generate group of 2 port , add groups with all ports for each vlan*/
        if (1 == prvTgfFWSUseAllPortsInVlan)
            continue;

        /* groupMembersArr has list of all ports of a speed group */
        for (ii = 0; ii < currentMemberNum; ii++, currentVlan++)
        {
            /* save info for restore procedure */
            prvTgfFWSLastVlan = currentVlan;

            portsArray[0] = groupMembersArr[ii];
            rc = prvTgfBrgVlanPortVidSet(devNum, portsArray[0],currentVlan);
            if (rc != GT_OK)
            {
                PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgVlanPortVidSet: failure port %d, rc %rc\n", portNum, rc);
                return rc;
            }

            portsArray[1] = (groupMembersArr[(ii + 1) % currentMemberNum]);

            rc = prvTgfBrgDefVlanEntryWithPortsSet(currentVlan, portsArray, NULL, NULL, 2);
            if (rc != GT_OK)
            {
               PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgDefVlanEntryWithPortsSet: failure vlan %d, rc %rc\n", currentVlan, rc);
               return rc;
            }

            /* disable NA to CPU */
            rc = prvTgfBrgVlanNaToCpuEnable(currentVlan, GT_FALSE);
            if (rc != GT_OK)
            {
               PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgVlanNaToCpuEnable: failure vlan %d, rc %rc\n", currentVlan, rc);
               return rc;
            }

            rc = prvTgfBrgDefFdbMacEntryOnPortSet(&(macAddrArr[group][0]), currentVlan,
                                                  devNum,  portsArray[1],  GT_TRUE);
            if (rc != GT_OK)
            {
                PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgDefFdbMacEntryOnPortSet: failure vlan %d, rc %rc\n", currentVlan, rc);
                return rc;
            }

        }
     }

     if ((1 == prvTgfFWSUseAllPortsInVlan) && (configEnable == GT_TRUE))
     {

        PRV_UTF_LOG1_MAC("============== :  generate vlan entrys includes all ports in FWS traffic. num of ports: %d =======\n",allMemberNum);
        for (ii = 0; ii < allMemberNum ; ii++,currentVlan++)
        {
            /* save info for restore procedure */
            prvTgfFWSLastVlan = currentVlan;

            portNum = useAllPortsArray[ii];
            rc = prvTgfBrgVlanPortVidSet(devNum, portNum,currentVlan);
            if (rc != GT_OK)
            {
                PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgVlanPortVidSet: failure port %d, rc %rc\n", portNum, rc);
                return rc;
            }

            rc = prvTgfBrgDefVlanEntryWithPortsSet(currentVlan, useAllPortsArray, NULL, NULL, allMemberNum);
            if (rc != GT_OK)
            {
                PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgDefVlanEntryWrite: failure vlan %d, rc %rc\n", currentVlan, rc);
                return rc;
            }

            /* disable NA to CPU */
            rc = prvTgfBrgVlanNaToCpuEnable(currentVlan, GT_FALSE);
            if (rc != GT_OK)
            {
               PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgVlanNaToCpuEnable: failure vlan %d, rc %rc\n", currentVlan, rc);
               return rc;
            }

            if ((numOfSpeedGroups >= 2) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                /* Aldrin2 has 6 ports of 100G using FIFO 2/3. So ports
                 * 0, 8, 16 use FIFO 2 and ports 4, 12, 20 use FIFO 3. As per
                 * test all ports used are from FIFO3 so explicitly moving few
                 * ports from FIFO 3 to FIFO 2
                 */

                if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
                {
                    if ((portSpeedArr[portNum] == CPSS_PORT_SPEED_100G_E) && (portNum < 12))
                    {
                        /* use FIFO 1 for 100G ports 0, 4, 8 and use default FIFO 0 for 12, 16, 20 */
                        rc = cpssDxChPortTxMcFifoSet(devNum, portNum, 2);
                        if( GT_OK != rc )
                        {
                            return rc;
                        }
                    }
                }
                if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
                {
                    if ((portSpeedArr[portNum] == CPSS_PORT_SPEED_10000_E) && (portNum < 24))
                    {
                        /* BC3 changes port 20 (100G) to 4 ports i.e. 20,21,22,23 of 10G
                         * so assigning FIFO 0/1 to these ports.
                         */
                        rc = cpssDxChPortTxMcFifoSet(devNum, portNum, (ii % 2));
                        if( GT_OK != rc )
                        {
                            return rc;
                        }
                    }
                }
                if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E ||
                   PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
                {
                    if (portSpeedArr[portNum] == CPSS_PORT_SPEED_10000_E)
                    {
                        /* Use FIFOs 2,3 for 10G ports. */
                        rc = cpssDxChPortTxMcFifoSet(devNum, portNum, ((ii % 2) + 2));
                        if( GT_OK != rc )
                        {
                            return rc;
                        }
                    }
                }
            }
        }
    }

    return GT_OK;
}

/* variable to store configuration of MC FIFOs */
static CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC  mcFifoWeigths_store;

static GT_STATUS prvTgfFWSGlobalConfig
(
    IN GT_U8                 devNum
)
{
    GT_STATUS rc;

    /* Puma3 do not need HOL mode */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_PUMA3_E)
    {
        rc = prvTgfPortFcHolSysModeSet(devNum,PRV_TGF_PORT_PORT_HOL_E);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfPortFcHolSysModeSet: dev %d, rc %d\n", devNum, rc);
            return rc;
        }
    }

    /* disable MMPCS_SIGNAL_DETECT_CHANGE event for Bobcat2 because FWS tests
       result in storming of events on port 60. */
    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E, CPSS_EVENT_MASK_E);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("cpssEventDeviceMaskSet: dev %d, rc %d\n", devNum, rc);
            return rc;
        }
    }

    /* close not needed FDB interrupts to avoid interrupt storming because NA relearning */
    rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_MAC_NA_NOT_LEARNED_E, CPSS_EVENT_MASK_E);
    if ((rc != GT_OK) && (rc != GT_NOT_FOUND))
    {
        PORT_FWS_PRV_UTF_LOG2_MAC("cpssEventDeviceMaskSet: dev %d, rc %d\n", devNum, rc);
        return rc;
    }

    rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_EB_NA_FIFO_FULL_E, CPSS_EVENT_MASK_E);
    if ((rc != GT_OK) && (rc != GT_NOT_FOUND))
    {
        PORT_FWS_PRV_UTF_LOG2_MAC("cpssEventDeviceMaskSet: dev %d, rc %d\n", devNum, rc);
        return rc;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC  mcFifoWeigths;

        /* store MC FIFO config */
        rc = cpssDxChPortTxMcFifoArbiterWeigthsGet(devNum,  &mcFifoWeigths_store);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("cpssDxChPortTxMcFifoArbiterWeigthsGet: dev %d, rc %d\n", devNum, rc);
            return rc;
        }

        mcFifoWeigths = mcFifoWeigths_store;

        /* increase 100G ports FIFOs 0,1 weights to get FWS */
        /* BC3 and Aldrin2 use FIFOs 2,3 for 100G ports */
        mcFifoWeigths.mcFifo0 = 0x8;
        mcFifoWeigths.mcFifo1 = 0x8;
        mcFifoWeigths.mcFifo2 = 0xF;
        mcFifoWeigths.mcFifo3 = 0xF;
        rc = cpssDxChPortTxMcFifoArbiterWeigthsSet(devNum,  &mcFifoWeigths);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("cpssDxChPortTxMcFifoArbiterWeigthsSet: dev %d, rc %d\n", devNum, rc);
            return rc;
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC  mcFifoWeigths;

        /* store MC FIFO config */
        rc = cpssDxChPortTxMcFifoArbiterWeigthsGet(devNum,  &mcFifoWeigths_store);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("cpssDxChPortTxMcFifoArbiterWeigthsGet: dev %d, rc %d\n", devNum, rc);
            return rc;
        }

        mcFifoWeigths = mcFifoWeigths_store;

        /* increase 100G ports FIFOs 0,1 weights to get FWS */
        /* BC3 and Aldrin2 use FIFOs 2,3 for 100G ports */
        mcFifoWeigths.mcFifo0 = 0x8;
        mcFifoWeigths.mcFifo1 = 0x8;
        mcFifoWeigths.mcFifo2 = 0x8;
        mcFifoWeigths.mcFifo3 = 0x8;
        rc = cpssDxChPortTxMcFifoArbiterWeigthsSet(devNum,  &mcFifoWeigths);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("cpssDxChPortTxMcFifoArbiterWeigthsSet: dev %d, rc %d\n", devNum, rc);
            return rc;
        }
    }

    return GT_OK;
}

#ifndef ASIC_SIMULATION
/* number of packets are sent per port in each iteration */
static GT_U32  fwsTrafBurst = 20;

/* number of iterations to generate stable FWS traffic */
static GT_U32  fwsTrafIter = 8;
#else
/* number of packets are sent per port in each iteration */
static GT_U32  fwsTrafBurst = 2;

/* number of iterations to generate stable FWS traffic */
static GT_U32  fwsTrafIter = 1;
#endif



GT_U32 prvTgfFWSTraffCount(GT_U32 newBurst, GT_U32 newIter)
{
    GT_U32 rc = fwsTrafBurst;

    fwsTrafBurst = newBurst;
    fwsTrafIter = newIter;

    return rc;
}

/**
* @internal prvTgfFWSLoopConfigTrafficGenerate function
* @endinternal
*
* @brief   Set configuration and generate full wire speed traffic.
*
* @param[in] firstVid                 - first VID for configuration
* @param[in] packetInfoPtr            - packet to send
* @param[in] configEnable             - GT_TRUE configure all needed setting and send traffic
*                                      - GT_FALSE do not change configuation and send traffic
*                                       None
*/
GT_STATUS  prvTgfFWSLoopConfigTrafficGenerate
(
    IN GT_U16               firstVid,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_BOOL              configEnable
)
{
    GT_U32  portSpeedGroupArr[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32  numOfPortSpeedGroups;
    TGF_MAC_ADDR macDaArr[CPSS_PORT_SPEED_NA_E]; /* mac DA per speed group */
    GT_PHYSICAL_PORT_NUM firstPortInGroupArr[CPSS_PORT_SPEED_NA_E];
    TGF_PACKET_L2_STC  *prvTgfPacketL2PartPtr;
    GT_STATUS rc;
    GT_U8   devNum = prvTgfDevNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U32  ii, kk;
    GT_U32  port;
    GT_U32  traffIter;
    GT_U32  burstCount;
    GT_BOOL prevPrint;

    cpssOsMemSet(portSpeedGroupArr,0,sizeof(portSpeedGroupArr));
    /* store values */
    prvTgfFWSFirstVlan = firstVid;
    prvTgfPacketL2PartPtr = (TGF_PACKET_L2_STC*)(packetInfoPtr->partsArray[0].partPtr);
    prvTgfPacketSampleL2Part = *prvTgfPacketL2PartPtr;

    cpssOsMemCpy(macDaArr[0],prvTgfPacketSampleL2Part.daMac, sizeof(TGF_MAC_ADDR));

    if (configEnable == GT_TRUE)
    {
        /* 1. Get list of ports,
              force link UP with maximal speed,
              force full duplex mode,
              disable FC */
        rc = prvTgfFWSPortsConfig(devNum, prvTgfPortSpeedArr);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfFWSPortsConfig: failure dev %d, rc %d\n", devNum, rc);
            return rc;
        }
    }

    /* 2. Sort ports to speed groups */
    rc = prvTgfFWSPortSpeedGroupGet(prvTgfPortSpeedArr, portSpeedGroupArr,
                                    &numOfPortSpeedGroups, &prvTgfFWSPortsNum);
    if (rc != GT_OK)
    {
        PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfFWSPortSpeedGroupGet: failure dev %d, rc %d\n", devNum, rc);
        return rc;
    }

    /* 3. Create FDB and VLAN loop for each speed group  */
    rc = prvTgfFWSFDBAndVlanLoopConfig(devNum, configEnable, prvTgfPortSpeedArr, portSpeedGroupArr, firstVid, numOfPortSpeedGroups, macDaArr, firstPortInGroupArr);
    if (rc != GT_OK)
    {
        PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfFWSVlanLoopConfig: failure dev %d, rc %d\n", devNum, rc);
        return rc;
    }

    if (configEnable == GT_TRUE)
    {
        /* 5. Enable tail drop mode */
        rc = prvTgfFWSGlobalConfig(devNum);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfFWSGlobalConfig: failure dev %d, rc %d\n", devNum, rc);
            return rc;
        }
    }

    /* use several iterations to get stable FWS */
    traffIter = fwsTrafIter;
    burstCount = fwsTrafBurst;

    for (kk = 0; kk < traffIter; kk++)
    {
        /* 6. generate traffic */
        for (ii = 0; ii < numOfPortSpeedGroups; ii++)
        {
            /* set MAC DA for the group */
            cpssOsMemCpy(prvTgfPacketL2PartPtr->daMac, &(macDaArr[ii][0]), sizeof(TGF_MAC_ADDR));

            /* setup Packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, burstCount, 0, NULL);
            if (rc != GT_OK)
            {
                PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfSetTxSetupEth: failure dev %d, rc %d\n", prvTgfDevNum, rc);

                /* restore packet */
                *prvTgfPacketL2PartPtr = prvTgfPacketSampleL2Part;
                return rc;
            }

            for (port = 0; port < CPSS_MAX_PORTS_NUM_CNS; port++)
            {
                if (portSpeedGroupArr[port] != ii)
                {
                    continue;
                }

                portNum =  port;

                if (configEnable)
                {

                    /* configure WS mode */
                    rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                                              PRV_TGF_TX_MODE_CONTINUOUS_FWS_E,
                                              PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                              0);
                    if (rc != GT_OK)
                    {
                        PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfTxModeSetupEth: failure portNum %d, rc %d\n", portNum, rc);
                        /* restore packet */
                        *prvTgfPacketL2PartPtr = prvTgfPacketSampleL2Part;
                        return rc;
                    }
                }

                /* disable packet trace */
                rc = tgfTrafficTracePacketByteSet(GT_FALSE);
                if (rc != GT_OK)
                {
                    return rc;
                }

                prevPrint = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);

                /* send Packet from port portNum */
                rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
                if (rc != GT_OK)
                {
                    PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfStartTransmitingEth: failure portNum %d, rc %d\n", portNum, rc);
                    /* restore packet */
                    *prvTgfPacketL2PartPtr = prvTgfPacketSampleL2Part;
                    return rc;
                }

                /* enable packet trace */
                prvTgfTrafficPrintPacketTxEnableSet(prevPrint);

                rc = tgfTrafficTracePacketByteSet(GT_TRUE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        } /* ii */
    } /* kk */
    /* restore packet */
    *prvTgfPacketL2PartPtr = prvTgfPacketSampleL2Part;

    /* provide time for interrupts treatment to finish it before rate measurement. */
    cpssOsTimerWkAfter(500);

    return GT_OK;
}


/**
* @internal prvTgfFWSRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfFWSRestore
(
    GT_VOID
)
{
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_STATUS               rc;
    GT_U8                   devNum = prvTgfDevNum;
    GT_U16                  vid;
    GT_U32                  jj;

    /*  mark all ports with invalid speed.
        this is sign that port is not exist or cannot be used. */
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        /*  skip not configured ports */
        if (prvTgfPortSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            continue;
        }

        /* force link UP and configure maximal speed on port */
        portInterface.type = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum = devNum;
        portInterface.devPort.portNum = portNum;

        /* disable loopback */
        rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
        if (rc != GT_OK)
        {
            /* port is not exist or cannot be used in the loop */
            PORT_FWS_PRV_UTF_LOG2_MAC("tgfTrafficGeneratorPortLoopbackModeEnableSet: failure portNum %d rc %d\n", portNum, rc);
            continue;
        }

        /* disable forcing on all ports */
        rc = prvTgfPortMaxSpeedForce(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            /* port is not exist or cannot be used in the loop */
            PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfPortMaxSpeedForce: failure portNum %d rc %d\n", portNum, rc);
            continue;
        }

        /* restore default VLAN */
        rc = prvTgfBrgVlanPortVidSet(devNum, portNum, 1);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgVlanPortVidSet: failure port %d, rc %rc\n", portNum, rc);
            continue;
        }

        /* configure single burst mode */
        rc = prvTgfTxModeSetupEth(prvTgfDevNum, portNum,
                                  PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                  PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                  0);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfTxModeSetupEth: failure portNum %d, rc %rc\n", portNum, rc);
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {

         /* restore ports */
         for (jj = 0 ; jj < sizeof(bc3_portNumFor10G)/ sizeof(bc3_portNumFor10G[0]); jj++ )
         {
             /* shutdown ports to free BW */
             rc = prvTgfPortIfModeSpeedPortConfig(devNum, bc3_portNumFor10G[jj], CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_10000_E);
             if(rc != GT_OK)
             {
                 PRV_UTF_LOG3_MAC("UTF: error : prvTgfPortIfModeSpeedPortConfig:  port [%d] , mode %d, speed %d\n" ,
                                    bc3_portNumFor10G[jj],CPSS_PORT_INTERFACE_MODE_KR_E,CPSS_PORT_SPEED_10000_E);
                 return;
             }
         }

         /* restore port to be 100G */
         rc = prvTgfPortIfModeSpeedPortConfig(devNum, bc3_portNumFor10G[0], CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_100G_E);
         if(rc != GT_OK)
         {
             PRV_UTF_LOG3_MAC("UTF: error : prvTgfPortIfModeSpeedPortConfig:  port [%d] , mode %d, speed %d\n" ,
                                bc3_portNumFor10G[0],CPSS_PORT_INTERFACE_MODE_KR_E,CPSS_PORT_SPEED_10000_E);
             return;
         }
    }


    /* wait to stop all traffic */
    cpssOsTimerWkAfter(10);

    /* clean VLAN table */
    for (vid = prvTgfFWSFirstVlan; vid <= prvTgfFWSLastVlan; vid++)
    {
        rc = prvTgfBrgVlanEntryInvalidate(vid);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("prvTgfBrgVlanEntryInvalidate: failure vid %d, rc %rc\n", vid, rc);
            continue;
        }
    }
    /* Puma3 do not need HOL mode */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_PUMA3_E)
    {
        /* restore FC mode */
        rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_FC_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_PORT_PORT_FC_E);
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* reset number of ports participated in FWS test */
    prvTgfFWSPortsNum = 0;

    /* restore interrupts */
    rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_MAC_NA_NOT_LEARNED_E, CPSS_EVENT_UNMASK_E);
    if ((rc != GT_OK) && (rc != GT_NOT_FOUND))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"cpssEventDeviceMaskSet: dev %d, rc %d\n", devNum, CPSS_PP_MAC_NA_NOT_LEARNED_E);
    }

    rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_EB_NA_FIFO_FULL_E, CPSS_EVENT_UNMASK_E);
    if ((rc != GT_OK) && (rc != GT_NOT_FOUND))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssEventDeviceMaskSet: dev %d, rc %d\n", devNum, CPSS_PP_EB_NA_FIFO_FULL_E);
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        /* restore MC FIFO config */
        rc = cpssDxChPortTxMcFifoArbiterWeigthsSet(devNum,  &mcFifoWeigths_store);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChPortTxMcFifoArbiterWeigthsSet: dev %d, rc %d\n", devNum, rc);
    }

    return;
}

/**
* @internal prvTgfFWSTrafficSpeedMeasure function
* @endinternal
*
* @brief   Measure traffic speed for all ports.
*
* @param[in] granularityBitsNum       - amount of bits to shift left.
*                                      64-bit counter converted to 32-bit value by formula
*                                      val32 = (counter.l[0] >> granularityBitsNum) | (counter.l[1] << (32 - granularityBitsNum))
* @param[in] counterTypesNum          - number of MAC Counters types accomulated for measure.
* @param[in] counterTypesArr[]        - (pointer to))array of MAC Counters types accomulated for measure.
*
* @param[out] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in 1024-bits per second
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_STATUS  prvTgfFWSTrafficSpeedMeasure
(
    IN   GT_U32  granularityBitsNum,
    IN   GT_U32  counterTypesNum,
    IN   CPSS_PORT_MAC_COUNTERS_ENT counterTypesArr[],
    OUT  GT_U32  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
)
{
    GT_PHYSICAL_PORT_NUM    portNum;  /* port number */
    GT_STATUS               rc = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    GT_U64                  cntrValue; /* counter value */
    GT_U32                  startTimeStampRxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for RX */
    GT_U32                  endTimeStamp;   /* VNT time stamp start */
    GT_U32                  rxRate;         /* VNT time stamp based RX rate */
    GT_U32                  granulatedCounter; /* conter in units defined by granulation */
    GT_U32                  i;              /* loop index*/
#ifdef ASIC_SIMULATION
    GT_U32                  expectRate;     /* expected rate according to port's speed */
#endif /*ASIC_SIMULATION*/

    cpssOsMemSet(portRxRateArr, 0x00, (sizeof(GT_U32) * CPSS_MAX_PORTS_NUM_CNS));

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if (prvTgfPortSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            /* skip not used port */
            continue;
        }

        /* get counters to reset it */
        for (i = 0; (i < counterTypesNum); i++)
        {
            rc = prvTgfPortMacCounterGet(
                devNum, portNum, counterTypesArr[i], &cntrValue);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG3_MAC(
                    "prvTgfPortMacCounterGet failure RX: dev %d port %d counter type %d\n",
                    devNum, portNum, counterTypesArr[i]);
                return rc;
            }
        }

        /* store time stamp */
        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &startTimeStampRxArr[portNum]);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return rc;
        }
    }

    /* sleep */
    cpssOsTimerWkAfter(1000);

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if (prvTgfPortSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            /* skip not used port */
            continue;
        }

        /* get counters value and accomulate them */
        granulatedCounter = 0;
        for (i = 0; (i < counterTypesNum); i++)
        {
            rc = prvTgfPortMacCounterGet(
                devNum, portNum, counterTypesArr[i], &cntrValue);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG3_MAC(
                    "prvTgfPortMacCounterGet failure RX: dev %d port %d counter type %d\n",
                    devNum, portNum, counterTypesArr[i]);
                return rc;
            }
            granulatedCounter +=
                (cntrValue.l[0] >> granularityBitsNum) | (cntrValue.l[1] << (32 - granularityBitsNum));
        }

        /* get time stamp */
        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return rc;
        }

#ifdef ASIC_SIMULATION
        if(granulatedCounter)
        {
            /* if the counter shows some movement ... it is wire speed !
               because no rate knowledge at simulation */
            if (granularityBitsNum)
            {
                /* assumed byte counting */
                expectRate = prvTgfCommonDevicePortWsRateGet(devNum, portNum, 10000/*packetSize*/);
                rxRate = (expectRate * 10000/*packetSize*/);
            }
            else
            {
                /* assumed packet counting */
                expectRate = prvTgfCommonDevicePortWsRateGet(devNum, portNum, 128/*packetSize*/);
                rxRate = expectRate;
            }
        }
        else
        {
            rxRate = 0;
        }
#else
        /* calculate actual rate */
        rxRate = prvTgfCommonVntTimeStampsRateGet(
            devNum, startTimeStampRxArr[portNum], endTimeStamp, granulatedCounter);
#endif /*ASIC_SIMULATION*/

        portRxRateArr[portNum] = rxRate;
    }

    return GT_OK;
}

/**
* @internal prvTgfFWSTrafficBandwidthCheck function
* @endinternal
*
* @brief   Check that traffic is FWS.
*
* @param[in] counterTypesNum          - number of MAC Counters types accomulated for measure.
* @param[in] counterTypesArr[]        - (pointer to))array of MAC Counters types accomulated for measure.
* @param[in] printRatesToLog          - GT_TRUE - print rates to log, GT_FALSE - no print rates.
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_STATUS  prvTgfFWSTrafficBandwidthCheck
(
  IN   GT_U32                     counterTypesNum,
  IN   CPSS_PORT_MAC_COUNTERS_ENT counterTypesArr[],
  IN   GT_BOOL                    printRatesToLog
)
{
    GT_PHYSICAL_PORT_NUM    portNum;  /* port number */
    GT_STATUS               rc = GT_OK; /* return code */
    GT_STATUS               rc1 = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    GT_U32                  diff; /* difference between two values */
    GT_U32                  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]; /* port rates  in megabit/sec */
    GT_U32                  bandwidth; /* bandwidth in packets/sec */
    GT_U32                  bandwidthAccurancy; /* measrement accurancy in percents */
    GT_U32                  numOfPipes; /* amount of pipes in PP */
    GT_U32                  coreClock; /* core clock of PP in Mhz */
    GT_U32                  expectedBandwidth; /* bandwidth in packets/sec */
    GT_U32                  portGroupId; /* work variable for port groups loop */
    GT_U32                  clocksPerPacket; /* amount of clocks per packet for each pipe */

    /* provide time to finish printouts and interrupt handlers
       before start rate measurement */
    cpssOsTimerWkAfter(800);

    /* Lion2, Xcat3, Bobcat2 and above have emulation of single counter read.
       The CPSS reads all counters from HW for single counter get.
       Time stamp stores time of last read counter from all port counters
       but not required one.
       Therefore the time stamp is not accurate. */
    bandwidthAccurancy = 15;

    cpssOsMemSet(portRxRateArr, 0x00, sizeof(portRxRateArr));
    /* mesure straffic speed packets resolution */
    rc = prvTgfFWSTrafficSpeedMeasure(
        0  /*granularityBitsNum*/,
        counterTypesNum,
        counterTypesArr,
        portRxRateArr);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvTgfFWSTrafficSpeedMeasure failed\n");
        return rc;
    }

    bandwidth = 0;
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        bandwidth += portRxRateArr[portNum];
    }
    PRV_UTF_LOG1_MAC("measured bandwidth in packets/sec is %d\n", bandwidth);

    /* count port groups - relevant for SIP5 devices also */
    numOfPipes = 0;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, portGroupId)
    {
        numOfPipes ++;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, portGroupId)

    clocksPerPacket = 2;
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        clocksPerPacket = 1;
    }

    coreClock  = PRV_CPSS_PP_MAC(devNum)->coreClock;
    /* coreClock in Mhz == 1/1000000 sec */
    /* ingress pipe packet forwarding take 2 clocks */
    expectedBandwidth = (numOfPipes * coreClock * 1000000) / clocksPerPacket;
    PRV_UTF_LOG4_MAC(
        "coreClock %d clocksPerPacket %d numOfPipes %d expectedBandwidth %d\n",
        coreClock, clocksPerPacket, numOfPipes, expectedBandwidth);

    if (GT_TRUE == prvUtfIsTrafficManagerUsed())
    {
        if(IS_BOBK_DEV_CETUS_MAC(devNum))
        {
            /* TM BW 25 Gbps. All ports go through TM. */
            expectedBandwidth = 37202380;
        }
        else if(IS_BOBK_DEV_MAC(devNum))
        {
            /* Caelum TM BW 75 Gbps. Assume all ports go through TM. */
            expectedBandwidth = 111607142;
        }
        else
        {
            /* BC2 with TM use 48G + 8x10G ports */
            expectedBandwidth = 190476190;
        }

        PRV_UTF_LOG1_MAC("TM reduce expectedBandwidth to %d\n", expectedBandwidth);
    }

    diff = prvTgfCommonDiffInPercentCalc(expectedBandwidth, bandwidth);
    if (diff > bandwidthAccurancy)
    {
        PRV_UTF_LOG2_MAC(
            "prvTgfFWSTrafficBandwidthCheck bandwidth: expected %d actual %d\n",
            expectedBandwidth, bandwidth);
        rc1 = GT_FAIL;
    }

    if (printRatesToLog != GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("Speed in packets/sec per port (pairs port - speed)\n");
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (portRxRateArr[portNum] == 0) continue;
            PRV_UTF_LOG2_MAC("%3d - %12d\n", portNum, portRxRateArr[portNum]);
        }
        PRV_UTF_LOG0_MAC("=================================================\n");
    }

    return rc1;
}

/**
* @internal prvTgfFWSTrafficCheck function
* @endinternal
*
* @brief   Check that traffic is FWS.
*
* @param[in] packetSize               - packet size in bytes including CRC
* @param[in] logRateErrors            - GT_TRUE - log rate errors
*                                      GT_FALSE - do not log rate errors
*
* @param[out] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in packets per second, may be NULL
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_STATUS  prvTgfFWSTrafficCheck
(
  IN   GT_U32  packetSize,
  IN   GT_BOOL logRateErrors,
  OUT  GT_U32  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
)
{
    GT_PHYSICAL_PORT_NUM    portNum;  /* port number */
    GT_STATUS               rc = GT_OK; /* return code */
    GT_STATUS               rc1 = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    GT_U64                  cntrValue; /* counter value */
    GT_U32                  diff; /* difference between two values */
    GT_U32                  startTimeStampRxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for RX */
    GT_U32                  endTimeStamp;   /* VNT time stamp start */
    GT_U32                  rxRate;         /* VNT time stamp based RX rate */
    GT_U32                  expectRate;     /* expected rate according to port's speed */
    GT_U32                  minRateArr[CPSS_PORT_SPEED_NA_E];  /* minimal rate */
    GT_U32                  maxRateArr[CPSS_PORT_SPEED_NA_E];  /* minimal rate */
    CPSS_PORT_SPEED_ENT     speed;  /* port speed */
    GT_32                   rateMeasureIteration; /* number of allowed iterations with big deviation */
    GT_U32                  rateMaxDiff; /* rate max diff */
    CPSS_PORT_SPEED_ENT     portSpeed; /* port speed */


    /* provide time to finish printouts and interrupt handlers
       before start rate measurement */
    cpssOsTimerWkAfter(800);

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
    {
        /* Puma3 does not support FWS because of additional 24 bytes of
           fabric header on fabric ports */
        prvTgfFwsRateDiff = 9;
    }

    if(GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION2_E | UTF_PUMA3_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_XCAT3_E))
    {
        /* time stamp measurement is accurate one. */
        prvTgfFwsRateDeviation = 10;

        /* only one iteration is allowed */
        rateMeasureIteration = 1;
    }
    else
    {
        /* Lion2,Puma3, Xcat3, Bobcat2 and above have emulation of single counter read.
           The CPSS reads all counters from HW for single counter get.
           Time stamp stores time of last read counter from all port counters
           but not required one.
           Therefore the time stamp is not accurate. */
#ifdef CHX_FAMILY
        if (GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_ALDRIN2_E))
        {
            /* Aldrin2 Tail Drop DBA mode */
            GT_BOOL   enable;
            prvTgfFwsRateDeviation = 450; /*default*/
            rc = cpssDxChPortTxDbaEnableGet(prvTgfDevNum, &enable);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("cpssDxChPortTxDbaEnableGet RC %d dev %d\n", rc, prvTgfDevNum);
                return rc;
            }
            if (enable != GT_FALSE)
            {
                prvTgfFwsRateDeviation = 3000; /* may be 2000 is enough, but 1500 not */
            }
        }
#endif /*CHX_FAMILY*/
        else if (GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E))
        {
            /* not Puma3 */
            prvTgfFwsRateDeviation = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ?
                                            450: 350;
        }
        else
        {
            prvTgfFwsRateDeviation = 250;
        }

        /* 3 iterations are allowed */
        rateMeasureIteration = 3;
    }

start_rate_measure:
    cpssOsMemSet(minRateArr, 0xFF, sizeof(minRateArr));
    cpssOsMemSet(maxRateArr, 0x00, sizeof(maxRateArr));

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if (prvTgfPortSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            /* skip not used port */
            continue;
        }

        /* get counter to reset it */
        rc = prvTgfPortMacCounterGet(devNum, portNum,
                                      CPSS_GOOD_UC_PKTS_RCV_E,
                                      &cntrValue);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return rc;
        }

        /* store time stamp */
        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &startTimeStampRxArr[portNum]);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return rc;
        }
    }

    /* sleep */
    cpssOsTimerWkAfter(1000);

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if (prvTgfPortSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            /* skip not used port */
            continue;
        }

        rateMaxDiff = prvTgfFwsRateDiff;

        /* get counter value */
        rc =  prvTgfPortMacCounterGet(devNum, portNum,
                                      CPSS_GOOD_UC_PKTS_RCV_E, &cntrValue);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return rc;
        }

        /* get time stamp */
        rc = prvTgfVntPortLastReadTimeStampGet(devNum, portNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet failure RX: dev %d port %d\n", devNum, portNum);
            return rc;
        }

#ifdef ASIC_SIMULATION
        if(cntrValue.l[0])
        {
            /* if the counter shows some movement ... it is wire speed !
               because no rate knowledge at simulation */
            expectRate = prvTgfCommonDevicePortWsRateGet(devNum, portNum, packetSize);

            rxRate = expectRate;
        }
        else
        {
            rxRate = 0;
        }
#else
        /* calculate actual rate */
        rxRate = prvTgfCommonVntTimeStampsRateGet(devNum, startTimeStampRxArr[portNum], endTimeStamp, cntrValue.l[0]);
#endif /*ASIC_SIMULATION*/
        if (rxRate < minRateArr[prvTgfPortSpeedArr[portNum]])
        {
            minRateArr[prvTgfPortSpeedArr[portNum]] = rxRate;
        }

        /* update maximal rate */
        if (rxRate > maxRateArr[prvTgfPortSpeedArr[portNum]])
        {
            maxRateArr[prvTgfPortSpeedArr[portNum]] = rxRate;
        }

        /* update minimal rate */
        if (portRxRateArr)
        {
            portRxRateArr[portNum] = rxRate;
        }

        /* calculate rate according port port's speed */
        expectRate = prvTgfCommonDevicePortWsRateGet(devNum, portNum, packetSize);

        /* compare rates.
          One percent is possible difference because of oscillator inaccuracy. */
        /* for 100G ports, the difference can be bigger */
        portSpeed = prvTgfPortSpeedArr[portNum];
        if (CPSS_PORT_SPEED_100G_E == portSpeed)
        {
            /* 12% is OK for 100G */
            rateMaxDiff = 12;
        }

        diff = prvTgfCommonDiffInPercentCalc(expectRate, rxRate);
        if (diff > rateMaxDiff)
        {
            /* Bobcat2 devices and above may generate FWS traffic with speed more then line rate.
               Use dedicated limit for such cases - prvTgfFwsOverRateDiff.
               xCat3 ports 24-27 behaves same . */
            if (!((rxRate > expectRate) &&
                  (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) ||
                   ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) && (portNum >= 24))) &&
                  (diff <= prvTgfFwsOverRateDiff)))
            {
                if (logRateErrors)
                {
                    PRV_UTF_LOG3_MAC("prvTgfFWSTrafficCheck rate: port %d expected %d actual %d\n", portNum, expectRate, rxRate);
                }
                rc1 = GT_FAIL;
            }
        }
    }

    /* compare rates deviation between ports */
    for (speed = 0; speed < CPSS_PORT_SPEED_NA_E; speed++)
    {
        /* skip not used ports */
        if (maxRateArr[speed] == 0)
        {
            continue;
        }

        diff = maxRateArr[speed] - minRateArr[speed];
        if (diff > prvTgfFwsRateDeviation)
        {
            if (logRateErrors)
            {
                PRV_UTF_LOG2_MAC("prvTgfFWSTrafficCheck rate deviation: max %d min %d\n", maxRateArr[speed], minRateArr[speed]);
            }
            PRV_UTF_LOG1_MAC("prvTgfFWSTrafficCheck rateMeasureIteration %d\n", rateMeasureIteration);

            if ((--rateMeasureIteration) > 0)
            {
                /* big rate deviation may be because of not atomic read counters
                   procedure and timestamp. Try to measure again. */
                PRV_UTF_LOG1_MAC("prvTgfFWSTrafficCheck do next iteration from %d\n", rateMeasureIteration);
                goto start_rate_measure;
            }
            /* for Bobcat3 there are difference in ports rate in FWS */
            if ((0 == HARD_WIRE_TCAM_MAC(prvTgfDevNum)) && (CPSS_PORT_SPEED_100G_E != speed))
            {
                if (logRateErrors)
                {
                    PRV_UTF_LOG3_MAC(
                        "prvTgfFWSTrafficCheck difference %d exceeds deviation %d, speed %d\n",
                        diff, prvTgfFwsRateDeviation, speed);
                }
                rc1 = GT_FAIL;
            }
        }
    }

    return rc1;
}

/**
* @internal prvTgfFWSTrafficRatePrint function
* @endinternal
*
* @brief   Print traffic.
*
* @param[in] packetSize               - packet size in bytes including CRC
* @param[in] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in packets per second
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_VOID  prvTgfFWSTrafficRatePrint
(
  IN  GT_U32  packetSize,
  IN  GT_U32  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
)
{
    GT_PHYSICAL_PORT_NUM    portNum;      /* port number */
    GT_U32                  printedPorts; /* number printed ports */
    GT_U32                  summaryRatePps;  /* summary rate in PPS */
    GT_FLOAT64              summaryRateBps;  /* summary rate in BPS */
    GT_U32                  summaryRateMbps; /* summary rate in MBPS */

    PRV_UTF_LOG0_MAC("======= Port rates ======\n");
    printedPorts = 0;
    summaryRatePps = 0;
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if (!portRxRateArr[portNum])
        {
            /* skip port with 0 rate */
            continue;
        }

        PRV_UTF_LOG2_MAC("Port %3d Rate %10u\n", portNum, portRxRateArr[portNum]);
        printedPorts++;
        summaryRatePps += portRxRateArr[portNum];
    }

    summaryRateBps = (GT_FLOAT64)summaryRatePps * ((GT_FLOAT64)packetSize + 20) * 8;
    summaryRateMbps = (GT_U32)(summaryRateBps / 1000000);

    PRV_UTF_LOG1_MAC("== Number of ports %d\n", printedPorts);
    PRV_UTF_LOG2_MAC("== Summary rate in PPS %u in MBitsPS %u\n", summaryRatePps, summaryRateMbps);

    /* provide time to finish printouts */
    cpssOsTimerWkAfter(400);
}

/**
* @internal prvTgfFWSTrafficStop function
* @endinternal
*
* @brief   Stop FWS traffic but do not change configuration
*/
GT_STATUS prvTgfFWSTrafficStop
(
    GT_VOID
)
{
    GT_PHYSICAL_PORT_NUM    portNum;    /* port number */
    GT_STATUS               rc = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    CPSS_INTERFACE_INFO_STC portInterface; /* interface */

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if (prvTgfPortSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            /* skip not used port */
            continue;
        }

        portInterface.type = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum = devNum;
        portInterface.devPort.portNum = portNum;

        rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface,GT_FALSE);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("tgfTrafficGeneratorPortLoopbackModeEnableSet: failure portNum %d, rc %rc\n", portNum, rc);
            return rc;
        }
    }

    /* Sleep minimal time */
    cpssOsTimerWkAfter(10);

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if (prvTgfPortSpeedArr[portNum] == CPSS_PORT_SPEED_NA_E)
        {
            /* skip not used port */
            continue;
        }

        portInterface.type = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum = devNum;
        portInterface.devPort.portNum = portNum;

        rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface,GT_TRUE);
        if (rc != GT_OK)
        {
            PORT_FWS_PRV_UTF_LOG2_MAC("tgfTrafficGeneratorPortLoopbackModeEnableSet: failure enable portNum %d, rc %rc\n", portNum, rc);
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvTgfFWSPortsNumGet function
* @endinternal
*
* @brief   Get number of ports participating in FWS test
*/
GT_U32 prvTgfFWSPortsNumGet
(
    GT_VOID
)
{
    return prvTgfFWSPortsNum;
}

/**
* @internal prvTgfFwsGenericTestUnderFws function
* @endinternal
*
* @brief   This function call the cb function (test) under FWS and stop the FWS after that
*         1) generate FWS
*         2) checks the rate
*         3) call the CB function -- see cbFuncPtr
*         4)stop the traffic
*         when using this function the caller test have to use prvTgfFWSRestore when restore.
* @param[in] cbFuncPtr                - pointer to CB function to be called under FWS. when NULL the function
*                                      just generate traffic without stop it.
*                                       None
*/
GT_VOID prvTgfFwsGenericTestUnderFws
(
    IN PRV_TGF_UNDER_FWS_CB_FUNC cbFuncPtr
)
{

    GT_STATUS   rc;
    GT_U16    firstVid   = 5;                               /* first VLAN id for the test */
    GT_U32    packetSize = PRV_TGF_PACKET_LEN_FWS_CNS + 4;
    GT_U32    portRxRateArr[CPSS_MAX_PORTS_NUM_CNS];        /* rate of packets per port */

    /*start FWS */
    cpssOsMemSet(portRxRateArr, 0, sizeof(portRxRateArr));

    PRV_UTF_LOG0_MAC("======= : generate FWS traffic =======\n");
    /* Create FWS traffic*/
    rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, &prvTgfPacketFWSInfoB, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate\n");

    /* check FWS traffic */
    PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericTestUnderFws check FWS traffic =======\n");
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficCheck\n");

    /* call the CB of the caller */
    if (cbFuncPtr != NULL)
    {
        PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericTestUnderFws call the 'call back' =======\n");
        cpssOsTimerWkAfter(100);
        cbFuncPtr();
        PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericTestUnderFws 'call back' ended =======\n");
    }

    PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericTestUnderFws stop the FWS =======\n");
    /*stop FWS*/
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop\n");

    PRV_UTF_LOG0_MAC("======= : end  prvTgfFwsGenericTestUnderFws =======\n");

}

/**
* @internal prvTgfFwsGenericFullBandwidthTest function
* @endinternal
*
* @brief   This function call the cb function (test) under FWS and stop the FWS after that
*         1) generate Full Bandwidth traffic
*         2) checks bandwidth
*         3) call the CB function -- see cbFuncPtr
*         4)stop the traffic
*         when using this function the caller test have to use prvTgfFWSRestore when restore.
* @param[in] cbFuncPtr                - pointer to CB function to be called under FWS. when NULL the function
*                                      just generate traffic without stop it.
*                                       None
*/
GT_VOID prvTgfFwsGenericFullBandwidthTest
(
    IN PRV_TGF_UNDER_FWS_CB_FUNC cbFuncPtr
)
{

    GT_STATUS   rc;
    GT_U16    firstVid   = 5;               /* first VLAN id for the test */
    CPSS_PORT_MAC_COUNTERS_ENT counterType; /* mac counter name */
    /*start FWS */

    PRV_UTF_LOG0_MAC("======= : generate FWS traffic =======\n");
    /* Create FWS traffic*/
    rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, &prvTgfPacketFWSInfoB, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate\n");

    /* check FWS traffic */
    PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericBandwidthFwsTest check FWS traffic by ingress =======\n");
    counterType = CPSS_GOOD_UC_PKTS_RCV_E;
    rc = prvTgfFWSTrafficBandwidthCheck(
        1 /*counterTypesNum*/, &counterType/*counterTypesArr*/, GT_TRUE/*printRatesToLog*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficBandwidthCheck\n");

    /* check FWS traffic */
    PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericBandwidthFwsTest check FWS traffic by egress =======\n");
    counterType = CPSS_GOOD_UC_PKTS_SENT_E;
    rc = prvTgfFWSTrafficBandwidthCheck(
        1 /*counterTypesNum*/, &counterType/*counterTypesArr*/, GT_TRUE/*printRatesToLog*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficBandwidthCheck\n");
    /* call the CB of the caller */
    if (cbFuncPtr != NULL)
    {
        PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericBandwidthFwsTest call the 'call back' =======\n");
        cpssOsTimerWkAfter(100);
        cbFuncPtr();
        PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericBandwidthFwsTest 'call back' ended =======\n");
    }

    PRV_UTF_LOG0_MAC("======= : prvTgfFwsGenericBandwidthFwsTest stop the FWS =======\n");
    /*stop FWS*/
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop\n");

    PRV_UTF_LOG0_MAC("======= : end  prvTgfFwsGenericBandwidthFwsTest =======\n");
}

/**
* @internal prvTgfFWSUseFloodingSet function
* @endinternal
*
* @brief   Configure FWS engine to use either known unicast or flooded traffic
*
* @param[in] status                   -- 0 - known unicast traffic is used
*                                      1 - flooded traffic is used
*                                       None
*/
GT_VOID prvTgfFWSUseFloodingSet
(
  IN GT_U32 status
)
{

    prvTgfFWSUseAllPortsInVlan = status;

}

/**
* @internal prvTgfPortFwsSkipPortsSet function
* @endinternal
*
* @brief   when prvTgfPortFwsSkipPorts = 0 ,prvTgfPortFWSSkipPort clear the ports bit map
*
* @param[in] status                   -- set status
*                                       None
*/
GT_VOID prvTgfPortFwsSkipPortsSet
(
    IN GT_U32 status
)
{

    if (prvTgfPortFwsSkipPorts != status)
    {
        prvTgfPortFwsSkipPorts = status;
    }
}

/**
* @internal prvTgfPortFwsSkipPortsGet function
* @endinternal
*
* @brief   when prvTgfPortFwsSkipPorts = 0 ,prvTgfPortFWSSkipPort clear the ports bit map
*
* @param[out] statusPtr                -- get status
*                                       None
*/
GT_VOID prvTgfPortFwsSkipPortsGet
(
   OUT GT_U32 *statusPtr
)
{
    *statusPtr = prvTgfPortFwsSkipPorts;
}

/**
* @internal prvTgfPortFwsSkipPortsBmpSet function
* @endinternal
*
* @brief   set prvTgfPortFwsSkipPortsBmp
*
* @param[in] bmapStatusPtr            --set value
*                                       None
*/

GT_VOID prvTgfPortFwsSkipPortsBmpSet
(
  IN CPSS_PORTS_BMP_STC *bmapStatusPtr
)
{
    prvTgfPortFwsSkipPortsBmp = *bmapStatusPtr;
}

/**
* @internal prvTgfPortFwsSkipPortsBmpGet function
* @endinternal
*
* @brief   set prvTgfPortFwsSkipPortsBmp
*
* @param[out] origBmapStatusPtr        -- get value
*                                       None
*/
GT_VOID prvTgfPortFwsSkipPortsBmpGet
(
   OUT CPSS_PORTS_BMP_STC *origBmapStatusPtr
)
{
    *origBmapStatusPtr = prvTgfPortFwsSkipPortsBmp;
}

