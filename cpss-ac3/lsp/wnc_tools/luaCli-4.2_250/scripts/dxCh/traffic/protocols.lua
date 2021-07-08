--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* protocols.lua
--*
--* DESCRIPTION:
--*       Packet analizing and protocol implementations tools
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

require("common/traffic/protocols")

do
    local print_saved = print
    print = function()
    end
    cmdLuaCLI_registerCfunction("prvLuaTgfTransmitPackets")
    cmdLuaCLI_registerCfunction("wrlDxChRxConfigure")
    cmdLuaCLI_registerCfunction("wrlDxChRxDisable")
    print = print_saved
end
if CLI_execution_unlock == nil or CLI_execution_lock == nil then
    CLI_execution_lock = function()
    end
    CLI_execution_unlock = function()
    end
end
if wrlDxChRxConfigure == nil then
    function wrlDxChRxConfigure(msgqId)
        cpssGenWrapper("luaDxChRxConfigure", {{ "IN", "GT_U32", "rxQId", msgqId }})
    end
    function wrlDxChRxDisable()
        cpssGenWrapper("luaDxChRxDisable", {})
    end
end

-- ************************************************************************
---
--  sendPacket_devport
--        @description  This function receives a device number, port
--                      number and data to send,
--                      if successful return 0,
--                      if not returns ret~=0 and a status
--
--        @param devNum         - device numver
--        @param portNum        - port number
--        @param packetData     - data
--        @param minLen         - minimum packet length
--
function sendPacket_devport_40(devNum,portNum,packetData,minLen)
    local ret, txTaskQueue,replyQ,status,message,i

    if (minLen==nil) then minLen=64 end
    if (minLen < 64) then minLen=64 end
    --activates the TX queue task if its not activated and receives the tasks id
    ret, txTaskQueue = cpssGenWrapper("transmitTaskQueueIdGet", { { "OUT", "GT_U32", "msgqId" } })
    if ret ~= 0 then
        return ret, "Error creating tx queue"
    else
        transmitTaskQ = luaMsgQAttach("txTaskQ", "TX_PARAMS_STC", txTaskQueue.msgqId)
    end
    -- create reply queue
    replyQ = luaMsgQCreate("replyQueue", "GT_U32", 10);

    status = luaMsgQSend(transmitTaskQ,
        {replyQ = replyQ.msgqId,srcInterface = {type = "port",devPort = { devNum = devNum, portNum = portNum } },
        packet = { len=math.max(minLen,string.len(packetData)/2), data= packetData }  })

    if (type(status)~="string") then
        -- recv status
        status = luaMsgQRecv(replyQ)
    end

    luaMsgQDelete(replyQ)
    return ret,status
end
sendPacket_devport = sendPacket_devport_40


function sendPacket_devport_41(devNum,portNum,packetData,minLen)
    local ret, txTaskQueue,replyQ,status,message,i

    if (minLen==nil) then minLen=60 end
    if (minLen < 60) then minLen=60 end
    -- add padding to minLen
    if string.len(packetData) < minLen*2 then
        packetData = packetData .. string.rep("0",minLen*2-string.len(packetData))
    end
    protocols_log(string.format("sending packet to %d/%d data=%s",
            devNum, portNum, packetData))
    packetInfo = { partsArray= {
        { type="TGF_PACKET_PART_PAYLOAD_E", partPtr = packetData }
    }}

    -- transmit packet
    CLI_execution_unlock()
    status = prvLuaTgfTransmitPackets(devNum, portNum, packetInfo, 1)
    CLI_execution_lock()
    return status
end

if prvLuaTgfTransmitPackets ~= nil then
    sendPacket_devport = sendPacket_devport_41
end
