--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tail_drop.lua
--*
--* DESCRIPTION:
--*       Tail-drop global commands
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local function enableTailDrop(params)
  local devNum = params["devID"]
  local devices
  local j
  local ret
  local dbaEnable

  local command_data = Command_Data()
  if params["policy"] == "dynamic" then
    dbaEnable = true
  elseif params["policy"] == "static" then
    dbaEnable = false
  end

  if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end

  for j=1,#devices do
    local devFamily, subFamily = wrlCpssDeviceFamilyGet(devices[j]);
    if (devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT3_E") then
        ret = myGenWrapper("cpssDxChPortTxDbaDebugControl",{
                {"IN", "GT_U8",   "devNum",              devices[j]},
                {"IN", "GT_BOOL", "startStopDba",        dbaEnable},
                {"IN", "GT_BOOL", "startStopStatistics", dbaEnable}
              })
        if 0~=ret then
            command_data:setFailStatus()
            command_data:addError("Error in cpssDxChPortTxDbaDebugControl call: "..returnCodes[ret].."\n")
        end
    else
        ret = myGenWrapper("cpssDxChPortTxDbaEnableSet",{
                {"IN", "GT_U8",   "devNum", devices[j]},
                {"IN", "GT_BOOL", "enable", dbaEnable}
              })
        if 0~=ret then
            command_data:setFailStatus()
            command_data:addError("Error in cpssDxChPortTxDbaEnableSet call: "..returnCodes[ret].."\n")
        end
    end
    if params["dba-buffer-limit"] ~= nil then
        if(params["dba-buffer-limit"] > 65535) and (devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT3_E") then
          command_data:addError("Buffer limit is out of range\n")
          command_data:setFailStatus()
          return
        end
        ret = myGenWrapper("cpssDxChPortTxDbaAvailBuffSet",{
                {"IN", "GT_U8",   "devNum", devices[j]},
                {"IN", "GT_U32", "maxBuff", params["dba-buffer-limit"]}
              })
        if 0~=ret then
            command_data:setFailStatus()
            command_data:addError("Error in cpssDxChPortTxDbaAvailBuffSet call: "..returnCodes[ret].."\n")
            return
        end
    end
  end
end

--------------------------------------------
-- command registration: tail-drop-policy
--------------------------------------------
CLI_addHelp("config", "tail-drop-policy", "Configure the tail-drop resource allocation policy")

CLI_type_dict["dba_buf_limit"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1048575,
    help="Total number of buffers for DBA"
}

CLI_addHelp("config", "tail-drop-policy dynamic", "Configure the tail-drop dynamic resource allocation policy")
CLI_addCommand("config", "tail-drop-policy dynamic", {
  func=function(params)
    params["policy"] = "dynamic"
    enableTailDrop(params)
  end,
  help="Configure the tail-drop dynamic resource allocation policy",
   params={
    { type="named",
      { format = "device %devID_all", name = "devID", help = "Device number"},
      { format="dba-buffer-limit %dba_buf_limit", name="dba-buffer-limit",
        help="Number of buffers available for dynamic allocation. Changes will be affected only for Dynamic Buffers Allocation (DBA) mode." },
      mandatory = {"devID"}
    }
  }
})

CLI_addHelp("config", "tail-drop-policy static", "Configure the tail-drop static resource allocation policy")
CLI_addCommand("config", "tail-drop-policy static", {
  func=function(params)
    params["policy"] = "static"
    enableTailDrop(params)
  end,
  help="Configure the tail-drop static resource allocation policy",
   params={
    { type="named",
      { format = "device %devID_all", name = "devID", help = "Device number"},
      mandatory = {"devID"}
    }
  }
})

CLI_addHelp("config", "no tail-drop-policy", "Configure the tail-drop static resource allocation policy")
CLI_addCommand("config", "no tail-drop-policy", {
  func=function(params)
    params["policy"] = "static"
    enableTailDrop(params)
  end,
  help="Configure the tail-drop static resource allocation policy",
   params={
    { type="named",
      { format = "device %devID_all", name = "devID", help = "Device number"},
      mandatory = {"devID"}
    }
  }
})

local function enableTailDropStatistic(params)
  local devNum = params["devID"]
  local devices
  local j
  local ret
  local values
  local enableState

  local command_data = Command_Data()
  if params["flagNo"] ~= nil then
    enableState = false
  else
    enableState = true
  end

  if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end
  for j=1,#devices do
    ret, values = myGenWrapper("cpssDxChPortTxBufferStatisticsEnableGet",{
      {"IN", "GT_U8",   "devNum", devices[j]},
      {"OUT", "GT_BOOL", "enPortStatPtr"},
      {"OUT", "GT_BOOL", "enQueueStatPtr"}
    })
    if 0~=ret then
      command_data:setFailStatus()
      command_data:addError("Error in cpssDxChPortTxBufferStatisticsEnableGet call: "..returnCodes[ret].."\n")
    end

    if(params["stat"] == "port") then
      values["enPortStatPtr"] = enableState
    end

    if(params["stat"] == "queue") then
      values["enQueueStatPtr"] = enableState
    end

    if(params["stat"] == "both") then
      values["enPortStatPtr"] = enableState
      values["enQueueStatPtr"] = enableState
    end

    ret, values = myGenWrapper("cpssDxChPortTxBufferStatisticsEnableSet",{
      {"IN", "GT_U8",   "devNum", devices[j]},
      {"IN", "GT_BOOL", "enPortStat", values["enPortStatPtr"]},
      {"IN", "GT_BOOL", "enQueueStat", values["enQueueStatPtr"]}
    })
    if 0~=ret then
      command_data:setFailStatus()
      command_data:addError("Error in cpssDxChPortTxBufferStatisticsEnableSet call: "..returnCodes[ret].."\n")
    end
  end
end

--------------------------------------------
-- command registration: taildrop-statistic-type
--------------------------------------------
CLI_type_dict["taildrop-statistic-type"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Tail-drop statistics type\n",
  enum = {
      ["port"] = { value="port", help="Tail-drop statistics per port" },
      ["queue"] = { value="queue", help="Tail-drop statistics per port per queue" },
      ["both"] = { value="both", help="Tail-drop statistics per port and per port per queue" },
 }
}

CLI_addHelp("config", "tail-drop-statistic", "Enable tail-drop statistic")
CLI_addCommand("config", "tail-drop-statistic", {
  func=enableTailDropStatistic,
  help="Enable tail-drop statistic",
   params={
    { type="values",
      { format="%taildrop-statistic-type", name="stat", help="Statistics type" }
    },
    { type="named",
      { format = "device %devID_all", name = "devID", help = "Device number"},
      mandatory = {"devID"}
    }
  }
})

CLI_addHelp("config", "no tail-drop-statistic", "Disable tail-drop statistic")
CLI_addCommand("config", "no tail-drop-statistic", {
  func=function(params)
    params["flagNo"] = true
    enableTailDropStatistic(params)
  end,
  help="Disable tail-drop statistic",
   params={
    { type="values",
      { format="%taildrop-statistic-type", name="stat", help="Statistics type" }
    },
    { type="named",
      { format = "device %devID_all", name = "devID", help = "Device number"},
      mandatory = {"devID"}
    }
  }
})