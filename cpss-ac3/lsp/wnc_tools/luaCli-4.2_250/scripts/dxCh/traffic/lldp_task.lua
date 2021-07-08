--********************************************************************************
--*              (c), Copyright 2013, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* lldp_task.lua
--*
--* DESCRIPTION:
--*       sending lldb paket
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

-- externs
--require("dxCh/traffic/RxTx")
cmdLuaCLI_registerCfunction("luaMsgQRecv")
cmdLuaCLI_registerCfunction("luaGlobalGet")
cmdLuaCLI_registerCfunction("luaCLI_getDevInfo")
cmdLuaCLI_registerCfunction("luaMsgQAttach")
cmdLuaCLI_registerCfunction("luaMsgQCreate")
cmdLuaCLI_registerCfunction("luaMsgQDelete")
cmdLuaCLI_registerCfunction("luaMsgQSend")

do

lldp_cntrl = task_Param
current_timeout = 0
lldp_data = nil

cmdLuaCLI_registerCfunction("wrlCpssCommonCpssVersionGet")

local vers="41"

if vers == 41 then
cmdLuaCLI_registerCfunction("prvLuaTgfTransmitPackets")
-- use prvLuaTgfTransmitPackets() on 4.1
-- ************************************************************************
---
--  sendPacketLldb
--        @description  Sending packets
--
--        @param dev          - device number
--        @param portTable    - port table
--        @param packetData   - string with packet data
--
--        @return       0 on success, otherwise 1 and error message
--
function sendPacketLldb(dev, portTable,packetData)
    local len = string.len(packetData)/2
    if len < 64 then
        len = 64
    end
    packetInfo = { partsArray= {
        { type="TGF_PACKET_PART_PAYLOAD_E", partPtr = { dataLength=len,dataPtr=packetData }}
    }}

    local i, ret
    for i=1,#portTable do
        -- transmit packet
        ret = prvLuaTgfTransmitPackets(dev, portTable[i], packetInfo, 1)
        if ret ~= 0 then
            status="Error sending packet through port device "..dev .. " port ".. portTable[i] .." : " .. to_string(ret)
            ret=1
            break
        end
    end

    return ret
end

else
transmitTaskQ = nil
replyQ = nil


-- ************************************************************************
---
--  sendPacketLldb
--        @description  Sending packets
--
--        @param dev          - device number
--        @param portTable    - port table
--        @param packetData   - string with packet data
--
--        @return       0 on success, otherwise 1 and error message
--
function sendPacketLldb(dev, portTable,packetData)
    local ret, status,i

    if transmitTaskQ == nil then
        --activates the TX queue task if its not activated and receives the tasks id
        local txTaskQueue
        ret, txTaskQueue = cpssGenWrapper("transmitTaskQueueIdGet", { { "OUT", "GT_U32", "msgqId" } })
        if ret ~= 0 then
            return ret, "Error creating tx queue"
        end
        transmitTaskQ = luaMsgQAttach("txTaskQ", "TX_PARAMS_STC", txTaskQueue.msgqId)
        -- create reply queue
        replyQ = luaMsgQCreate("replyQueue", "GT_U32", 10);
    end

    for i=1,#portTable do
        -- transmit packet
        local minLen = 64

        status = luaMsgQSend(transmitTaskQ,
            {
                replyQ = replyQ.msgqId,
                srcInterface = {
                    type = "port",
                    devPort = 
                    {
                        devNum = dev,
                        portNum = portTable[i]
                    }
                },
                packet =
                {
                    len=math.max(minLen,string.len(packetData)/2 + 4),
                    data= packetData
                }
            })
            
        if (type(status)~="string") then
            -- recv status
            status = luaMsgQRecv(replyQ)
        end

        if (type(status)=="string") then
            status="Error sending packet through port device "..dev .. " port ".. portTable[i] .." : " .. status
            ret=1
            break
        end
    end

    return ret,status
end

end -- ver ~= "41"

while (true) do
    recv_timeout = current_timeout * 1000
    if current_timeout == 0 then
        recv_timeout = nil -- forever
    end
    status, v = luaMsgQRecv(lldp_cntrl, recv_timeout)

    if status == 0 then
        current_timeout = v
        if current_timeout ~= 0 then
            lldp_data = luaGlobalGet("n")
        end
    end
    if current_timeout ~= "0" then
        for dev, data in pairs(lldp_data) do
            sendPacketLldb(dev, data.portTable, data.data)
            --cpssGenWrapper("osPrintf", {{"IN", "string", "format", "Send Pack: ".. data.data .. "\n"}})
            --cpssGenWrapper("osPrintf", {{"IN", "string", "format", "Send Pack: %s\n"},
            --                   {"IN","string","s", data.data}})
        end
    end
end

end
