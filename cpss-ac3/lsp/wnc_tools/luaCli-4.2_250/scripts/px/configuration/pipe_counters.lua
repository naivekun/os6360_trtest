
--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pipe_counters.lua
--*
--* DESCRIPTION:
--*       Centralized Counters (CNC) commands
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

function cncCounterClearByReadEnableGet(dev, blockNum)
    local ret, val

    ret, val = myGenWrapper("cpssPxCncCounterClearByReadEnableGet",{
        {"IN", "GT_U8",  "devNum", dev},
        {"OUT","GT_BOOL","enablePtr"}
    })
    if ret~=0 then
        print("cpssPxCncCounterClearByReadEnableGet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be set"
    end
    return true, val.enablePtr
end

function cncCounterClearByReadEnableSet(dev, blockNum, enable)
    local ret

    ret = myGenWrapper("cpssPxCncCounterClearByReadEnableSet",{
        {"IN", "GT_U8",  "devNum", dev},
        {"IN", "GT_BOOL","enable", enable}
    })
    if ret~=0 then
        print("cpssPxCncCounterClearByReadEnableSet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be set"
    end
    return true
end

local function clearCounters(dev, blockNum)
    local ret, val
    local clearEnabled
    local ii

    ret, clearEnabled = cncCounterClearByReadEnableGet(dev, blockNum)
    if ret == false then
        return false, clearEnabled
    end

    ret, val = cncCounterClearByReadEnableSet(dev, blockNum, true)
    if ret == false then
        return false, val
    end

    for ii=0,1023 do
        ret = myGenWrapper("cpssPxCncCounterGet",{
            {"IN",  "GT_U8",  "devNum",   dev},
            {"IN",  "GT_U32", "blockNum", blockNum},
            {"IN",  "GT_U32", "index",    ii},
            {"IN",  "CPSS_PX_CNC_COUNTER_FORMAT_ENT", "format", "CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E"},
            {"OUT", "CPSS_PX_CNC_COUNTER_STC",        "counterPtr"}
        })
        if ret~=0 then
            print("cpssPxCncCounterClearByReadEnableSet() failed: "..returnCodes[ret])
            return false,"Error, counter could not be set"
        end
    end

    ret, val = cncCounterClearByReadEnableSet(dev, blockNum, clearEnabled)
    if ret == false then
        return false, val
    end
end

local function cncClientEnable(dev, block, client, enable)
    local ret
    ret = myGenWrapper("cpssPxCncBlockClientEnableSet",{
        {"IN","GT_U8","devNum", dev},
        {"IN","GT_U32","blockNum", block},
        {"IN","CPSS_PX_CNC_CLIENT_ENT","client", client},
        {"IN","GT_BOOL","updateEnable",enable}
    })
    if ret~=0 then
        print("cpssPxCncBlockClientEnableSet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be set"
    end
    return true
end

local function cncRangeSet(dev, bmp0, bmp1)
    local ret, val

    -- local indexRangesBmp = { l={[0]=1,[1]=0}} -- table to set GT_U64
    ret, val = myGenWrapper("cpssPxCncBlockClientRangesSet",{
        {"IN","GT_U8","devNum",    dev},
        {"IN","GT_U32","blockNum", 0},
        --[[{"IN","GT_U64","indexRangesBmp", indexRangesBmp}--]]
        {"IN","GT_U32","indexRangesBmp_low",  bmp0},
        {"IN","GT_U32","indexRangesBmp_high", bmp1 }
    })
    if ret~=0 then
        print("cpssPxCncBlockClientRangesSet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be set"
    end
    return true
end

local function cncEgressProcessing(params)
    local ret, val
    local i
    local devNum = params["devID"]
    local enable
    local devices
    local activeDevices

    if (params.flagNo == nil) then
        enable = true
    else
        enable = false
    end

    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end
    activeDevices = globalGet("cncActiveDevices")
    if activeDevices == nil then
        activeDevices = {}
    end
    if (enable == true) then
        for i,val in pairs(devices) do
            if activeDevices[val] ~= nil then
                print("CNC is already turned on")
                return
            end
        end

        for j=1,#devices do
            ret, val = cncClientEnable(devices[j], 0, "CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E", true)
            if ret == false then
                return ret, val
            end

            ret, val = clearCounters(devices[j], 0)
            if ret == false then
                return ret, val
            end

            ret, val = cncRangeSet(devices[j], 1, 0)
            if ret == false then
                return ret, val
            end
        end

        for i,val in pairs(devices) do
            activeDevices[val] = "egressProcessing"
        end
        globalStore("cncActiveDevices", activeDevices)
    else
        for i,val in pairs(devices) do
            if activeDevices[val] ~= "egressProcessing" then
                print("Other CNC mode is turned on")
                return
            end
        end
        for j=1,#devices do
            ret, val = cncClientEnable(devices[j], 0, "CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E", false)
            if ret == false then
                return ret, val
            end
        end

        for i,val in pairs(devices) do
            activeDevices[val] = nil
        end
        globalStore("cncActiveDevices", activeDevices)
    end

    return true
end

local function cncIngressProcessing(params)
    local ret, val
    local i
    local devNum = params["devID"]
    local enable
    local devices
    local activeDevices

    if (params.flagNo == nil) then
        enable = true
    else
        enable = false
    end

    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end
    activeDevices = globalGet("cncActiveDevices")
    if activeDevices == nil then
        activeDevices = {}
    end
    if (enable == true) then
        for i,val in pairs(devices) do
            if activeDevices[val] ~= nil then
                print("CNC is already turned on")
                return false
            end
        end

        for j=1,#devices do
            ret, val = cncClientEnable(devices[j], 0, "CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E", true)
            if ret == false then
                return ret, val
            end

            ret, val = clearCounters(devices[j], 0)
            if ret == false then
                return ret, val
            end

            ret, val = cncRangeSet(devices[j], 1, 0)
            if ret == false then
                return ret, val
            end
        end
        for i,val in pairs(devices) do
            activeDevices[val] = "ingressProcessing"
        end
        globalStore("cncActiveDevices", activeDevices)
    else
        for i,val in pairs(devices) do
            if activeDevices[val] ~= "ingressProcessing" then
                print("Other CNC mode is turned on")
                return ret, val
            end
        end
        for j=1,#devices do
            ret, val = cncClientEnable(devices[j], 0, "CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E", false)
            if ret == false then
                return ret, val
            end
        end

        for i,val in pairs(devices) do
            activeDevices[val] = nil
        end
        globalStore("cncActiveDevices", activeDevices)
    end

    return true
end

local function egressQueue(params)
    local ret, val
    local i
    local devNum = params["devID"]
    local enable
    local devices
    local blockNums
    local activeDevices

    if (params.flagNo == nil) then
        enable = true
    else
        enable = false
    end

    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end
    activeDevices = globalGet("cncActiveDevices")
    if activeDevices == nil then
        activeDevices = {}
    end
    if (enable == true) then
        for i,val in pairs(devices) do
            if activeDevices[val] ~= nil then
                print("CNC is already turned on")
                return false
            end
        end
        for j=1,#devices do
            ret, val = cncClientEnable(devices[j], 0, "CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E", true)
            if ret == false then
                return ret, val
            end

            clearCounters(devices[j], 0)
            if ret == false then
                return ret, val
            end

            ret, val = cncRangeSet(devices[j], 0xFF, 0)
            if ret == false then
                return ret, val
            end

            ret = myGenWrapper("cpssPxCncEgressQueueClientModeSet",{
                {"IN","GT_U8","devNum", devices[j]},
                {"IN","CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT","mode",params.mode}
            })
            if ret~=0 then
                print("cpssPxCncEgressQueueClientModeSet() failed: "..returnCodes[ret])
                return false,"Error, counter could not be set"
            end

            mode = globalGet("cncEgressQueueCnMode")
            if mode == nil then
                mode = {}
            end
            mode[devices[j]] = params.mode
            globalStore("cncEgressQueueCnMode", mode)
        end
        for i,val in pairs(devices) do
            activeDevices[val] = "egressQueue"
        end
        globalStore("cncActiveDevices", activeDevices)
    else
        for i,val in pairs(devices) do
            if activeDevices[val] ~= "egressQueue" then
                print("Other CNC mode is turned on")
                return false
            end
        end
        for j=1,#devices do
            ret, val = cncClientEnable(devices[j], 0, "CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E", false)
            if ret == false then
                return ret, val
            end
        end

        globalStore("cncEgressQueueCnMode", nil)
        for i,val in pairs(devices) do
            activeDevices[val] = nil
        end
        globalStore("cncActiveDevices", activeDevices)
    end

    return true
end

CLI_addHelp("config", "counters", "CNC counters")

--------------------------------------------
-- command registration: counters egress-processing
--------------------------------------------
help_counter_egress_processing = "Enable egress processing client in the CNC."
CLI_addHelp("config", "counters egress-processing", help_counter_egress_processing)
CLI_addCommand("config", "counters egress-processing", {
  func=cncEgressProcessing,
  help=help_counter_egress_processing,
   params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
        mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- command registration: no counters egress-processing
--------------------------------------------
help_counter_no_egress_processing = "Disable egress processing client in the CNC."
CLI_addHelp("config", "no counters egress-processing", help_counter_no_egress_processing)
CLI_addCommand("config", "no counters egress-processing", {
  func=function(params)
            params.flagNo = true
            return cncEgressProcessing(params)
        end,
  help=help_counter_no_egress_processing,
   params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
        mandatory = {"devID"}
    }
   }
})

--------------------------------------------
-- command registration: counters ingress-processing
--------------------------------------------
help_counter_ingress_processing = "Enable ingress processing client in the CNC."
CLI_addHelp("config", "counters ingress-processing", help_counter_ingress_processing)
CLI_addCommand("config", "counters ingress-processing", {
  func=cncIngressProcessing,
  help=help_counter_ingress_processing,
   params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
        mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- command registration: no counters ingress-processing
--------------------------------------------
help_counter_no_ingress_processing = "Disable ingress processing client in the CNC."
CLI_addHelp("config", "no counters ingress-processing", help_counter_no_ingress_processing)
CLI_addCommand("config", "no counters ingress-processing", {
  func=function(params)
            params.flagNo = true
            return cncIngressProcessing(params)
        end,
  help=help_counter_no_ingress_processing,
   params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
        mandatory = {"devID"}
    }
   }
})

local counter_egress_queue_mode_help = "Egress queue CNC client counting mode"
local counter_egress_queue_mode_type_enum = {
    ["tail-drop"] = { value = 0, help = "Tail-drop counting mode" },
    ["cn"]        = { value = 1, help = "CN counting mode" },
    ["reduced"]   = { value = 2, help = "Queue pass/drop reduced mode" }
}
CLI_addParamDictAndType_enum("eq_mode","Egress queue CNC client mode", counter_egress_queue_mode_help, counter_egress_queue_mode_type_enum)

--------------------------------------------
-- command registration: counters egress-queue
--------------------------------------------
help_counter_egress_queue = "Enable egress queue / CN client in the CNC."
CLI_addHelp("config", "counters egress-queue", help_counter_egress_queue)
CLI_addCommand("config", "counters egress-queue", {
  func=egressQueue,
  help=help_counter_egress_queue,
   params={
    { type="named",
      { format="device %devID_all", name="devID"   ,help="The device number"},
      { format="mode %eq_mode",     name="mode"    ,help="Counting mode"},
        mandatory = {"devID", "mode"}
    }
  }
})

--------------------------------------------
-- command registration: no counters egress-queue
--------------------------------------------
help_counter_no_egress_queue = "Disable egress queue / CN client in the CNC."
CLI_addHelp("config", "no counters egress-queue", help_counter_egress_queue)
CLI_addCommand("config", "no counters egress-queue", {
  func=function(params)
            params.flagNo = true
            return egressQueue(params)
        end,
  help=help_counter_no_egress_queue,
   params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
        mandatory = {"devID"}
    }
  }
})
