/*************************************************************************
* wraplTgfPxUtils.c
*
* DESCRIPTION:
*       A lua tgf wrappers for PX devices
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
* 
*       $Revision: 1 $
* 
**************************************************************************/
#include <string.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>

#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>
#include <mainLuaWrapper/wraplTgfTypes.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>

/* Free allocated memory on error */
#define PRV_LUA_TGF_TR_EXT_VERIFY_GT_OK(rc, msg, dev, port, packetInfo, egressPortsArr)  \
    if (GT_OK != rc)                                                         \
    {                                                                        \
        lua_getglobal(L, "print");                                           \
        lua_pushfstring(L, "\n[TGF]: %s FAILED on %d/%d, rc = [%d]",         \
                    msg, dev, port, rc);                                     \
        lua_call(L, 1, 0);                                                   \
        prv_gc_TGF_PACKET_STC(&packetInfo);                                  \
        prv_gc_TGF_EXPECTED_EGRESS_INFO_STC(egressPortsArr);                 \
        lua_pushnumber(L, rc);                                               \
        return 1;                                                            \
    }

#define PRV_LUA_TGF_VERIFY_GT_OK(rc, msg)                                    \
    if (GT_OK != rc)                                                         \
    {                                                                        \
        lua_getglobal(L, "print");                                           \
        lua_pushfstring(L, "\n[TGF]: %s FAILED, rc = [%d]", msg, rc);        \
        lua_call(L, 1, 0);                                                   \
        lua_pushnumber(L, rc);                                               \
        return 1;                                                            \
    }

/*******************************************************************************
* prvLuaTgfTransmitPacketsWithCapture
*
* DESCRIPTION:
*       Transmit packets with capturing
*
* INPUTS:
*       devNum                  - ingress device number
*       portNum                 - ingress port number
*       packetInfoPtr           - (pointer to) the packet info to send
*       burstCount              - number of frames
*       numElements             - number of egress ports info in array of egressPortsArr[]
*                                   if 0 --> not checking egress ports
*                                   Accepted range: 0..3
*       egressPortsArr          - (pointer to) egress ports array to check for counters and packets
*
* OUTPUTS:
*       None
*
* RETURNS:
*       return code
*
* COMMENTS:
*
*******************************************************************************/
int prvLuaTgfPxTransmitPacketsWithCapture
(
    lua_State* L
)
{
    GT_SW_DEV_NUM       devNum;
    GT_U32              portNum;
    TGF_PACKET_STC      packetInfo;
    GT_U32              burstCount;
    GT_U32              numElements = 0;
    GT_STATUS           rc = GT_OK;
    TGF_EXPECTED_EGRESS_INFO_STC  egressPortsArr[PRV_LUA_TGF_EGRESS_PORTS_ARR_MAX];

    cmdOsMemSet(egressPortsArr, 0, PRV_LUA_TGF_EGRESS_PORTS_ARR_MAX * sizeof(TGF_EXPECTED_EGRESS_INFO_STC));

    PARAM_NUMBER(rc, devNum, 1, GT_U32);
    PARAM_NUMBER(rc, portNum, 2, GT_U32);
    PARAM_STRUCT(rc, &packetInfo, 3, TGF_PACKET_STC);
    PARAM_NUMBER(rc, burstCount, 4, GT_U32);
    PARAM_NUMBER(rc, numElements, 5, GT_U32);
    if(numElements > 0)
    {
        PARAM_ARRAY_N(rc, egressPortsArr, numElements, 6, TGF_EXPECTED_EGRESS_INFO_STC)
    }


    PRV_LUA_TGF_TR_EXT_VERIFY_GT_OK(rc, "Input parameters", devNum, portNum, packetInfo, egressPortsArr); 

    /* Send packet and check results */
    rc = tgfTrafficGeneratorPxTransmitPacketAndCheckResult(devNum, portNum, 
                                                           &packetInfo, burstCount, 
                                                           numElements, egressPortsArr);

    /* Free allocated memory after C function call */
    prv_gc_TGF_EXPECTED_EGRESS_INFO_STC(egressPortsArr);

    lua_pushnumber(L, rc);

    return 1;
}

/*******************************************************************************
* prvLuaTgfPxTransmitPackets
*
* DESCRIPTION:
*       Transmit packets with capturing
*
* INPUTS:
*       devNum                  - ingress device number
*       portNum                 - ingress port number
*       packetInfoPtr           - (pointer to) the packet info to send
*       burstCount              - number of frames
*       numElements             - number of egress ports info in array of egressPortsArr[]
*                                   if 0 --> not checking egress ports
*                                   Accepted range: 0..3
*       egressPortsArr          - (pointer to) egress ports array to check for counters and packets
*
* OUTPUTS:
*       None
*
* RETURNS:
*       return code
*
* COMMENTS:
*
*******************************************************************************/
int prvLuaTgfPxTransmitPackets
(
    lua_State* L
)
{
    GT_SW_DEV_NUM       devNum;
    GT_U32              portNum;
    TGF_PACKET_STC      packetInfo;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32              burstCount;
    GT_STATUS           rc = GT_OK;
    GT_BOOL             isWs = GT_FALSE;
    GT_BOOL             withLoopBack = GT_FALSE;
    GT_BOOL             doTxEthTransmit = GT_TRUE;
    GT_BOOL             stopTransmit = GT_FALSE;

    PARAM_NUMBER(rc, devNum, 1, GT_U32);
    PARAM_NUMBER(rc, portNum, 2, GT_U32);
    PARAM_STRUCT(rc, &packetInfo, 3, TGF_PACKET_STC);
    PARAM_NUMBER(rc, burstCount, 4, GT_U32);
    if (lua_isboolean(L, 5))
    {
        PARAM_BOOL(rc, withLoopBack, 5);
    }

    PRV_LUA_TGF_VERIFY_GT_OK(rc, "prvLuaTgfPxTransmitPackets");

    if (burstCount == 0xFFFFFFFF)
    {
        isWs = GT_TRUE;
    }
    else
    if (burstCount == 0)
    {
        stopTransmit = GT_TRUE;
    }

    if(stopTransmit == GT_TRUE)/* stop WS */
    {
        /* stop send Packet from port portNum */
        /* NOTE: this function disable the mac loopback of the port */
        rc = tgfTrafficGeneratorStopWsOnPort(devNum, portNum);

        /* the loopback already disabled in luaTgfStopWsOnPort() */
        withLoopBack = GT_FALSE;
        /* we are not sending packets now */
        doTxEthTransmit = GT_FALSE;

        prvTgfFwsUse = 0;
    }
    else
    if(isWs == GT_TRUE) /* start WS */
    {
        burstCount = prvTgfWsBurst;
        prvTgfFwsUse = 1;

        rc = tgfTrafficGeneratorWsModeOnPortSet(devNum, portNum);
        PRV_LUA_TGF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorWsModeOnPortSet");

    }

    if(doTxEthTransmit == GT_TRUE)
    {
        cmdOsMemSet(&portInterface, 0, sizeof(portInterface));

        /* setup receive outPortInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = devNum;
        portInterface.devPort.portNum = portNum;

        /* set the CPU device to send traffic */
        rc = tgfTrafficGeneratorCpuDeviceSet(devNum);
        PRV_LUA_TGF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorCpuDeviceSet");

        /* start transmitting */
        if (withLoopBack == GT_TRUE)
        {
            rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
            PRV_LUA_TGF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet enable");
        }

        /* Send packet and check results */
        rc = tgfTrafficGeneratorPxTransmitPacket(devNum, portNum, &packetInfo,
            burstCount);
    }

    if(isWs == GT_TRUE) /* start WS */
    {
        /* the loopback should not be restored when exiting the function.
           we wait until caller will call again to 'stop WS'  */
        withLoopBack = GT_FALSE;
    }

    if (withLoopBack == GT_TRUE)
    {
        /* disable loopback mode on port */
        (void) tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    }

    lua_pushnumber(L, rc);

    return 1;
}

/*******************************************************************************
* prvLuaTgfPxPortsRateGet
*
* DESCRIPTION:
*       get the rate of list of ports, each port will get : max rate ,rx,tx actual rates.
*
* INPUTS:
*       numOfPorts    - number of ports to get info
*       interval      - time between checks (milisec)

*       devNum1       - for port #1 : device number
*       portNum1      - for port #1 : port number
*       sizeOfPacket1 - for port #1 : size of packet (in bytes) include CRC !!!
*       devNum2       - for port #2 : device number
*       portNum2      - for port #2 : port number
*       sizeOfPacket2 - for port #2 : size of packet (in bytes) include CRC !!!

        ...

*       devNum n       - for port #n : device number
*       portNum n      - for port #n : port number
*       sizeOfPacket n - for port #n : size of packet (in bytes) include CRC !!!

*
* OUTPUTS:
*       None
*
* RETURNS:
*       1. return code ,

*       2. port#1 : max rate of the port
*       3. port#1 : rate of actual packets Rx to the port
*       4. port#1 : rate of actual packets Tx from the port

*       . port#2 : max rate of the port
*       . port#2 : rate of actual packets Rx to the port
*       . port#2 : rate of actual packets Tx from the port

        ...

*       . port#n : max rate of the port
*       . port#n : rate of actual packets Rx to the port
*       . port#n : rate of actual packets Tx from the port


*
* COMMENTS:
*
*******************************************************************************/
int prvLuaTgfPxPortsRateGet
(
    lua_State* L
)
{
    GT_U32               numOfPorts;
    GT_U32               interval;
    GT_STATUS            rc = GT_OK;
    GT_U8   devNumArr[GET_RATE_MAX_PORTS_CNS];
    GT_U32  portNumArr[GET_RATE_MAX_PORTS_CNS];
    GT_U32  txRateArr[GET_RATE_MAX_PORTS_CNS];
    GT_U32  rxRateArr[GET_RATE_MAX_PORTS_CNS];
    GT_U32  sizeOfPacketArr[GET_RATE_MAX_PORTS_CNS];
    GT_U32  wsRateArr[GET_RATE_MAX_PORTS_CNS];
    GT_U32  indexOnStack;
    GT_U32  ii;


    indexOnStack = 1;
    PARAM_NUMBER(rc, numOfPorts,  indexOnStack, GT_U32);
    indexOnStack++;
    PARAM_NUMBER(rc, interval,    indexOnStack, GT_U32);
    indexOnStack++;

    if(numOfPorts > GET_RATE_MAX_PORTS_CNS ||
       numOfPorts == 0)
    {
        /* Error. */
        rc = GT_BAD_SIZE;
        lua_pushnumber(L, rc);
        return 1;
    }

    for(ii = 0 ; ii  < numOfPorts ; ii++)
    {
        PARAM_NUMBER(rc, devNumArr[ii],  indexOnStack, GT_U8);
        indexOnStack++;
        PARAM_NUMBER(rc, portNumArr[ii], indexOnStack, GT_U32);
        indexOnStack++;
        PARAM_NUMBER(rc, sizeOfPacketArr[ii], indexOnStack, GT_U32);
        indexOnStack++;
    }

    /*************************************************/
    /* we are done getting parameters from the stack */
    /*************************************************/

    for(ii = 0 ; ii  < numOfPorts ; ii++)
    {
        /* get port's theoretical wire-speed rate */
        wsRateArr[ii] = prvTgfCommonDevicePortWsRateGet(devNumArr[ii],portNumArr[ii],sizeOfPacketArr[ii]);
        if (0 == wsRateArr[ii])
        {
            /* Error. */
            rc = GT_BAD_STATE;
            lua_pushnumber(L, rc);
            return 1;
        }
    }

    /* get port's actual rate */
    rc = prvTgfCommonPortsCntrRateGet_byDevPort(devNumArr,portNumArr,
        numOfPorts ,interval ,
        CPSS_GOOD_OCTETS_SENT_E , txRateArr,
        CPSS_GOOD_OCTETS_RCV_E  , rxRateArr);

    indexOnStack = 0;
    lua_pushnumber(L, rc);
    indexOnStack++;

    if (rc != GT_OK)
    {
        return indexOnStack;
    }

    for(ii = 0 ; ii  < numOfPorts ; ii++)
    {
        /* add 20 bytes IPG and preamble to get L1 byte size */
        lua_pushnumber(L, wsRateArr[ii] * (sizeOfPacketArr[ii] + 20) * 8 / 1000);
        lua_pushnumber(L, rxRateArr[ii] * 4 / 1000);
        lua_pushnumber(L, txRateArr[ii] * 4 / 1000);

        indexOnStack += 3;
    }

    return indexOnStack;
}

/*******************************************************************************
* prvLuaTgfHexStringToBin
*
* DESCRIPTION:
*       Transmit packet from CPU
*
* INPUTS:
*       inString               - string of hexadecimals
*       outArray               - binary data
*       bufSize                - size of outArray
*
* OUTPUTS:
*       bufSize                - actual size of outArray
*
* RETURNS:
*       GT_OK                  - success
*       GT_NO_RESOURCE         - to small buffer
*       GT_BAD_PTR             - bad pointer
*       GT_BAD_PARAM           - invalid data in hexstring
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS prvLuaTgfHexStringToBin
(
    const char *inString,
    GT_U8      *outArray,
    GT_U32     *bufSize
)
{
    GT_U32 strLen;
    GT_U32 ii;
    GT_U8  byte;

    CPSS_NULL_PTR_CHECK_MAC(inString);
    CPSS_NULL_PTR_CHECK_MAC(outArray);
    CPSS_NULL_PTR_CHECK_MAC(bufSize);
    strLen = cpssOsStrlen(inString);

    if(strLen / 2 > *bufSize)
    {
        return GT_NO_RESOURCE;
    }

    for(ii = 0; ii < strLen; ii += 2)
    {
        switch(inString[ii])
        {
            case '0':           byte = 0x00; break;
            case '1':           byte = 0x10; break;
            case '2':           byte = 0x20; break;
            case '3':           byte = 0x30; break;
            case '4':           byte = 0x40; break;
            case '5':           byte = 0x50; break;
            case '6':           byte = 0x60; break;
            case '7':           byte = 0x70; break;
            case '8':           byte = 0x80; break;
            case '9':           byte = 0x90; break;
            case 'a': case 'A': byte = 0xA0; break;
            case 'b': case 'B': byte = 0xB0; break;
            case 'c': case 'C': byte = 0xC0; break;
            case 'd': case 'D': byte = 0xD0; break;
            case 'e': case 'E': byte = 0xE0; break;
            case 'f': case 'F': byte = 0xF0; break;
            default: return GT_BAD_PARAM;
        }

        switch(inString[ii+1])
        {
            case '0':           byte |= 0x00; break;
            case '1':           byte |= 0x01; break;
            case '2':           byte |= 0x02; break;
            case '3':           byte |= 0x03; break;
            case '4':           byte |= 0x04; break;
            case '5':           byte |= 0x05; break;
            case '6':           byte |= 0x06; break;
            case '7':           byte |= 0x07; break;
            case '8':           byte |= 0x08; break;
            case '9':           byte |= 0x09; break;
            case 'a': case 'A': byte |= 0x0A; break;
            case 'b': case 'B': byte |= 0x0B; break;
            case 'c': case 'C': byte |= 0x0C; break;
            case 'd': case 'D': byte |= 0x0D; break;
            case 'e': case 'E': byte |= 0x0E; break;
            case 'f': case 'F': byte |= 0x0F; break;
            default: return GT_BAD_PARAM;
        }
        outArray[ii / 2] = byte;
    }

    *bufSize = strLen / 2;
    return GT_OK;
}

/*******************************************************************************
* prvLuaTgfPxNetIfSdmaSyncTxPacketSend
*
* DESCRIPTION:
*       Transmit packet from CPU
*
* INPUTS:
*       devNum                  - ingress device number
*       packetData              - data to send
*
* OUTPUTS:
*       None
*
* RETURNS:
*       return code
*
* COMMENTS:
*
*******************************************************************************/
int prvLuaTgfPxNetIfSdmaSyncTxPacketSend
(
    lua_State* L
)
{
    CPSS_PX_NET_TX_PARAMS_STC       pcktParams;
    GT_U8                           *buffList[1];
    GT_U32                          buffLenList[1];
    GT_STATUS                       st;
    GT_SW_DEV_NUM                   devNum;
    const char                      *luaPacket;

    pcktParams.txQueue = 0;
    pcktParams.recalcCrc = GT_TRUE;

    if (lua_isnumber(L, 1))
    {
        devNum = (GT_SW_DEV_NUM)lua_tonumber(L, 1);
    }
    else
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    buffList[0] = cpssOsCacheDmaMalloc(TGF_RX_BUFFER_MAX_SIZE_CNS);
    buffLenList[0] = TGF_RX_BUFFER_MAX_SIZE_CNS;

    if(NULL == buffList[0])
    {
        lua_pushinteger(L, (lua_Integer)GT_NO_RESOURCE);
        return 1;
    }

    if (lua_type(L, 2) != LUA_TSTRING)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        cpssOsCacheDmaFree(buffList[0]);
        return 1;
    }
    else
    {
        luaPacket = (char*)lua_tostring(L, 2);
    }

    st = prvLuaTgfHexStringToBin(luaPacket, buffList[0], &buffLenList[0]);
    if(GT_OK != st)
    {
        lua_pushinteger(L, (lua_Integer)st);
        cpssOsCacheDmaFree(buffList[0]);
        return 1;
    }

    st = cpssPxNetIfSdmaSyncTxPacketSend(devNum, &pcktParams, buffList,
        buffLenList, 1);

    if(GT_OK != st)
    {
        lua_pushinteger(L, (lua_Integer)st);
        cpssOsCacheDmaFree(buffList[0]);
        return 1;
    }

    cpssOsCacheDmaFree(buffList[0]);
    lua_pushinteger(L, (lua_Integer)st);
    return 1;
}
