--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* receiverTask.lua
--*
--* DESCRIPTION:
--*       Packet receiver task
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants


protocolArray={}  -- The array containing all the protocols
-- if not defined yet
luaGlobalGet=globalGet
luaGlobalStore=globalStore

require("dxCh/traffic/protocols")
require("dxCh/traffic/proto_all")

cmdLuaCLI_registerCfunction("luaOsSigSemSignal")
if luaOsSigSemSignal == nil then
    luaOsSigSemSignal = function()
    end
end

local status,rxdata,proccessQ

RxTxQueueID = luaGlobalGet("RxTxQueueID")
RxTxSigAckSemID = luaGlobalGet("RxTxSigAckSemID")
proccessQ=luaMsgQAttach("receiveQueue", "RX_DATA_STC",RxTxQueueID)

--
-- get protocols table, enable/disable rx to lua
--
function get_protocols()
    protocolArray = luaGlobalGet("rxProtocols")
    if type(protocolArray) ~= "table" then
        protocolArray={}
    end
    protocols_log_enabled = luaGlobalGet("rxProtocols_log_en")
    local i,proto
    local enable = false
    for i,proto in pairs(protocolArray) do
        if not proto.disabled then
            enable = true
            break
        end
    end
    myMac=luaGlobalGet("myMac")
    myIP=luaGlobalGet("myIP")
    if enable then
        wrlDxChRxConfigure(RxTxQueueID)
    else
        wrlDxChRxDisable()
    end
end

get_protocols()

while (true) do
    status, rxdata = luaMsgQRecv(proccessQ,"a") --if second paramter is not a number endless wait

    if rxdata~=nil then
        if rxdata.packet.len == 0 then
            get_protocols()
            luaOsSigSemSignal(RxTxSigAckSemID)
        else
            checkProtocol(protocolArray, rxdata)
        end
    end
end
