--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* proto_control.lua
--*
--* DESCRIPTION:
--*       Functions to control receiver task and protocols
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

require("px/traffic/protocols")

cmdLuaCLI_registerCfunction("luaTaskCreate")
cmdLuaCLI_registerCfunction("luaOsSigSemCreate")
cmdLuaCLI_registerCfunction("luaOsSigSemWait")

-- ************************************************************************
---
--  createReceiverTask
--        @description  This function creates a receiver task which will
--                      receive all packets then route it to protocol
--                      handlers or client applications (for example ping)
--
--        @return               - true/false
--
function createReceiverTask()
    local rcvTaskID, rcvTaskQueueID
    rcvTaskQueueID= luaGlobalGet("RxTxQueueID")  -- "RxTxQueueID" - global parameter for the rcv queue id
    if rcvTaskQueueID == nil then
        receiveQueue=luaMsgQCreate("receiveQueue", "PX_RX_DATA_STC", 10)
        if type(receiveQueue) ~= "table" then
            print("createReceiverTask(): luaMsgQCreate() returned "..to_string(receiveQueue))
            return false
        end
        rcvTaskQueueID = receiveQueue.msgqId
        luaGlobalStore("RxTxQueueID",rcvTaskQueueID)  -- "RxTxQueueID" - global parameter for the rcv queue id
    end
    if luaGlobalGet("RxTxSigAckSemID") == nil then
        local semId = luaOsSigSemCreate("RxTxSigAckSem")
        if type(semId) == "number" then
            luaGlobalStore("RxTxSigAckSemID",semId)
        else
            print("createReceiverTask(): luaOsSigSemCreate() returned "..to_string(semId))
            return false
        end
    end
    rcvTaskID= luaGlobalGet("RxTxTaskID")  -- "RxTxTaskID" - global parameter for the rcv task id
    if rcvTaskID == nil then
        rcvTaskID = luaTaskCreate("RxTxTask","px/traffic/receiverTask.lua")  -- the "RxTxTask" name is a reserved word
        if type(rcvTaskID) ~= "number" then
            return false
        end
        luaGlobalStore("RxTxTaskID",rcvTaskID)  -- "RxTxTaskID" - global parameter for the rcv task id
    end
    return true
end

-- ************************************************************************
---
--  signalReceiverTask
--        @description  Signal to receiver task to read protocol descriptions
--
--
function signalReceiverTask()
    if not createReceiverTask() then
        return false
    end
    local proccessQ=luaMsgQAttach("receiveQueue", "PX_RX_DATA_STC", luaGlobalGet("RxTxQueueID"))
    local semId = luaGlobalGet("RxTxSigAckSemID")
    luaMsgQSend(proccessQ,nil)
    luaOsSigSemWait(semId,500) -- wait ack
end

-- ************************************************************************
---
--  protocolAdd
--        @description  Add protocol to array
--                      This function used for both protocol handlers
--                      executed in receiver task and client applications
--                      (ping, capture)
--
--        @param name   - protocol name
--        @param proto  - protocol definition (filter rules and handler)
--
function protocolAdd(name, proto)
    local pa = luaGlobalGet("rxProtocols")
    if type(pa) ~= "table" then
        pa={}
    end
    pa[name] = proto
    luaGlobalStore("rxProtocols", pa)
    if not proto.disabled then
        signalReceiverTask()
    end
end

-- ************************************************************************
---
--  protocolEnable
--        @description  Enable/disable protocol
--
--        @param name   - protocol name
--        @param enable - enable/disable flag (boolean)
--
function protocolEnable(name,enable)
    local pa = luaGlobalGet("rxProtocols")
    if type(pa) ~= "table" then
        pa={}
    end
    if pa[name] == nil then
        return false
    end
    local signal = false
    if enable then
        if pa[name].disabled then signal = true end
        pa[name].disabled = nil
    else
        if not pa[name].disabled then signal = true end
        pa[name].disabled = true
    end
    luaGlobalStore("rxProtocols", pa)
    if signal then
        signalReceiverTask()
    end
end

-- ************************************************************************
---
--  registerRx
--        @description  Register client side RX
--
--        @param name     - client name
--        @param ident    - filter rules (see protocols), default all packets
--        @param queueLen - queue length, default 10
--        @return         - queue to receive packets from
--
function registerRx(name,ident,queueLen)
    if ident == nil then ident = {} end
    if queueLen == nil then queueLen = 10 end
    local rxQ = luaMsgQCreate(name.."Queue", "PX_RX_DATA_STC", queueLen)
    if type(rxQ) ~= "table" then
        print("registerRx(): luaMsgQCreate() returned "..to_string(rxQ))
        return nil
    end

    protocolAdd(name,{
        name = name,
        func = rxQ.msgqId,
        ident = ident
    })
    return rxQ
end

-- ************************************************************************
---
--  unregisterRx
--        @description  Unregister client side RX
--
--        @param name     - client name
--        @param rxQ      - queue returned by registerRx()
--
function unregisterRx(name,rxQ)
    local pa = luaGlobalGet("rxProtocols")
    if type(pa) ~= "table" then
        pa={}
    end
    if pa[name] ~= nil then
        pa[name] = nil
        luaGlobalStore("rxProtocols", pa)
        signalReceiverTask()
    end

    luaMsgQDelete(rxQ)
end
