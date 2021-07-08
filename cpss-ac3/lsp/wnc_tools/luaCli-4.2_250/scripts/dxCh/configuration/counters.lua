--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* counters.lua
--*
--* DESCRIPTION:
--*       Centralized Counters (CNC) commands
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes


--constants


local function prvGetNumberOfBlocks(devID)

  local ret,val
  local totalSize, sizePerBlock
  local numberOfBlocks
  ret, val = myGenWrapper("cpssDxChCfgTableNumEntriesGet",{
        {"IN","GT_U8","devNum",devID},
        {"IN","CPSS_DXCH_CFG_TABLES_ENT","table","CPSS_DXCH_CFG_TABLE_CNC_E"},
        {"OUT","GT_U32","numEntriesPtr"}
  })

  if ret~=0 then
    return false, false,"Error, counter could not be set"
  end

  totalSize = val["numEntriesPtr"]

  ret, val = myGenWrapper("cpssDxChCfgTableNumEntriesGet",{
        {"IN","GT_U8","devNum",devID},
        {"IN","CPSS_DXCH_CFG_TABLES_ENT","table","CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E"},
        {"OUT","GT_U32","numEntriesPtr"}
  })

  if ret~=0 then
    return false,false, "Error, counter could not be set"
  end

  sizePerBlock = val["numEntriesPtr"]

  numberOfBlocks = totalSize / sizePerBlock

  return numberOfBlocks, sizePerBlock
end

-- ************************************************************************
--  ingressVlanCounters
--
--  @description command enables Ingress VLAN pass/drop client in the CNC.
--         The command binds both pass and drop VLAN counters if count-drop defined.
--          The command binds only pass VLAN counters if count-drop not defined.
--
--  @param params - params["count-drop"]    - set if drop VLAN counters will be defined.
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************



local function ingressVlanCounters(params)
  local ret, val
  local blockNumbers, i
  local devNum = params["devID"]
  local CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E = 4
  local DropBitMapBase = 0x10000
  local devices

  local enable

  if (params.flagNo == nil) then
    enable = true
  else
    enable = false
  end
  -- get blocks
  if(enable==false)then
      blockNumbers = getCNCBlock("IngressVlanPassAndDrop")
  else
      if (params["countDrop"] ~= nil) then -- get block for PASS and DROP
        blockNumbers = getCNCBlock("IngressVlanPassAndDrop")
      else -- get block for PASS only
        blockNumbers = getCNCBlock("IngressVlanPass")
      end
  end
  if blockNumbers == -1 then
    return false, "Wrong client name for selected device"
  end
  if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end

  for j=1,#devices do
    local numberOfBlocks, sizeOfBlock, details = prvGetNumberOfBlocks(devices[j])
    if (numberOfBlocks == false) then
      return false, details
    end
    local numberOfNeededBlocks = math.ceil(0x1000 / sizeOfBlock)
    for i=1, #blockNumbers do -- bind all PASS and DROP (if defined) blocks with client
      local blockID = blockNumbers[i]
      ret = myGenWrapper("cpssDxChCncBlockClientEnableSet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"IN","GT_U32","blockNum",blockID},
        {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E},
        {"IN","GT_BOOL","updateEnable",enable}
      })
      if ret~=0 then
        print("cpssDxChCncBlockClientEnableSet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be set"
      end
    end
    if (params.flagNo == nil) then
      local isDrop = false
      local passAndDropCounter = 1
      local bmp
      for i=1, #blockNumbers do -- set ranges
        if (passAndDropCounter > numberOfNeededBlocks and isDrop == false) then
          isDrop = true
        end

        bmp = math.pow(2,i-1)
        if (isDrop == true) then
          bmp = DropBitMapBase * math.pow(2, passAndDropCounter-numberOfNeededBlocks-1)
        end

        -- local indexRangesBmp = { l={[0]=bmp,[1]=0}} -- table to set GT_U64
        ret, val = myGenWrapper("cpssDxChCncBlockClientRangesSet",{
          {"IN","GT_U8","devNum",devices[j]},
          {"IN","GT_U32","blockNum",blockNumbers[i]},
          {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E},
                    --[[{"IN","GT_U64","indexRangesBmp", indexRangesBmp}--]]
                    {"IN","GT_U32","indexRangesBmp_low", bmp},
                    {"IN","GT_U32","indexRangesBmp_high", 0}
        })

        if ret~=0 then
          print("cpssDxChCncBlockClientRangesSet() failed: "..returnCodes[ret])
          return false,"Error, counter could not be set"
        end

        passAndDropCounter = passAndDropCounter + 1
      end
    end
  end

  if (enable == true) then
    if (params["countDrop"] ~= nil) then
      globalStore("IngressVlan", "PassAndDrop")
    else
      if (globalGet("IngressVlan") ~= "PassAndDrop") then
        globalStore("IngressVlan", "Pass")
      end
    end
  else
    globalStore("IngressVlan", nil)
  end

    return true
end

--------------------------------------------
-- command registration: counters ingress-vlan
--------------------------------------------

CLI_addCommand("config", "counters ingress-vlan", {
  func=ingressVlanCounters,
  help="enables Ingress VLAN pass/drop client in the CNC.",
   params={
    { type="named",
    { format="device %devID_all", name="devID" ,help="The device number"},
      { format="count-drop", name="countDrop", help="set if drop VLAN counters will be defined." },

    requirements = {
      ["countDrop"] = {"devID"}
    },
    mandatory = {"devID"}
    }
  }
})


--------------------------------------------
-- command registration: no counters ingress-vlan
--------------------------------------------

CLI_addCommand("config", "no counters ingress-vlan", {
  func=function(params)
      params.flagNo = true
      return ingressVlanCounters(params)
    end,
  help="disable Ingress VLAN pass/drop client in the CNC.",
   params={
    { type="named",
    { format="device %devID_all", name="devID" ,help="The device number"},
    mandatory = {"devID"}
    }
   }
})

-- ************************************************************************
--  egressQueueCounters
--
--  @description enables Egress queue pass/tail drop client in the CNC.
--         The command binds both pass and drop VLAN counters if count-drop defined.
--          The command binds only pass VLAN counters if count-drop not defined.
--
--  @param params - params["count-drop"]    - set if drop VLAN counters will be defined.
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************



local function egressQueueCounters(params)
  local ret, val
  local blockNumbers, i
  local devNum = params["devID"]
  local CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E = 6
  local DropBitMapBase = 0x100
  local devices
  local usedCnc

  local enable

  if (params.flagNo == nil) then
    enable = true
  else
    enable = false
  end
  if(enable==false)then
      usedCnc = "EgressQueuePassAndDrop"
      blockNumbers = getCNCBlock(usedCnc)
  else
      -- get blocks
      if (params["countDrop"] ~= nil) then -- get block for PASS and DROP
        usedCnc = "EgressQueuePassAndDrop"
        blockNumbers = getCNCBlock(usedCnc)
      else -- get block for PASS only
        usedCnc = "EgressQueuePass"
        blockNumbers = getCNCBlock(usedCnc)
      end
  end
  if blockNumbers == -1 then
    return false, "Wrong client name for selected device"
  end
  if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end

  for j=1,#devices do
    local numberOfBlocks, sizeOfBlock, details = prvGetNumberOfBlocks(devices[j])
    if (numberOfBlocks == false) then
      return false, details
    end
    local numberOfNeededBlocks = math.ceil(0x1000 / sizeOfBlock)
      if(is_sip_5_20(nil) and usedCnc == "EgressQueuePassAndDrop") then
        -- the formula changed and supports more bits
        numberOfNeededBlocks = math.ceil(0x2000 / sizeOfBlock)
      else
        numberOfNeededBlocks = math.ceil(0x1000 / sizeOfBlock)
      end
    
    for i=1, #blockNumbers do -- bind all PASS and DROP (if defined) blocks with client
      local blockID = blockNumbers[i]

      ret = myGenWrapper("cpssDxChCncBlockClientEnableSet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"IN","GT_U32","blockNum",blockID},
        {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E},
        {"IN","GT_BOOL","updateEnable",enable}
      })
      if ret~=0 then
        print("cpssDxChCncBlockClientEnableSet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be set"
      end

    end
    if (params.flagNo == nil) then
      local isDrop = false
      local passAndDropCounter = 1
      local bmp
      for i=1, #blockNumbers do -- set ranges
        if (passAndDropCounter > numberOfNeededBlocks and isDrop == false) then
          isDrop = true
        end

        bmp = math.pow(2,i-1)
        if (isDrop == true) then
          bmp = DropBitMapBase * math.pow(2, passAndDropCounter-numberOfNeededBlocks-1)
        end

        ret, val = myGenWrapper("cpssDxChCncBlockClientRangesSet",{
          {"IN","GT_U8","devNum",devices[j]},
          {"IN","GT_U32","blockNum",blockNumbers[i]},
          {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E},
                    --[[{"IN","GT_U64","indexRangesBmp", indexRangesBmp}--]]
                    {"IN","GT_U32","indexRangesBmp_low", bmp},
                    {"IN","GT_U32","indexRangesBmp_high", 0}
        })

        if ret~=0 then
          print("cpssDxChCncBlockClientRangesSet() failed: "..returnCodes[ret])
          return false,"Error, counter could not be set"
        end

        passAndDropCounter = passAndDropCounter + 1
      end
    end
  end

  if (enable == true) then
    if (params["countDrop"] ~= nil) then
      globalStore("EgressQueue", "PassAndDrop")
    else
      if (globalGet("EgressQueue") ~= "PassAndDrop") then
        globalStore("EgressQueue", "Pass")
      end
    end
  else
    globalStore("EgressQueue", nil)
  end

    return true
end

--------------------------------------------
-- command registration: counters egress-queue
--------------------------------------------

CLI_addCommand("config", "counters egress-queue", {
  func=egressQueueCounters,
  help="enable Egress queue pass/tail drop client in the CNC.",
   params={
    { type="named",
    { format="device %devID_all", name="devID" ,help="The device number"},
      { format="count-drop", name="countDrop", help="set if drop VLAN counters will be defined." },

    requirements = {
      ["countDrop"] = {"devID"}
    },
    mandatory = {"devID"}
    }
  }
})


--------------------------------------------
-- command registration: no counters egress-queue
--------------------------------------------

CLI_addCommand("config", "no counters egress-queue", {
  func=function(params)
      params.flagNo = true
      return egressQueueCounters(params)
    end,
  help="disable Egress queue pass/tail drop client in the CNC.",
   params={
    { type="named",
    { format="device %devID_all", name="devID" ,help="The device number"},
    mandatory = {"devID"}
    }
   }
})

-- ************************************************************************
--  cncOffsetForNatClientSet
--
--  @description - Set the CNC offset for NAT client.
--
--  @param params - params["devID"]    - the device number
--                  params["offset"]  - CNC offset for NAT client
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************

local function cncOffsetForNatClientSet(params)
    local ret, val
    local i
    local devNum = params["devID"]
    local offset
    local restore
    local devices

    if (params.flagNo == nil) then
        restore = false
    else
        restore = true
    end

    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end

    for i=1,#devices do
        if(restore==true)then
            offset = 0
        else
            offset = tonumber(params["offset"])
        end

        ret = myGenWrapper("cpssDxChCncOffsetForNatClientSet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN","GT_U32","offset",offset}
        })
        if ret~=0 then
            print("cncOffsetForNatClientSet() failed: "..returnCodes[ret])
            return false,"Error, Cnc offset for NAT Client could not be set"
        end
    end

    return true
end


--------------------------------------------
-- command registration: counters NAT
--------------------------------------------

CLI_addCommand("config", "counters nat-offset", {
    func=cncOffsetForNatClientSet,
    help="Set the CNC offset for NAT client.",
    params={
        { type="named",
            { format="device %devID_all", name="devID" ,help="The device number"},
            { format="offset %offset", name="offset", help="CNC offset for NAT client"},

            mandatory = {"devID","offset"}
        }
    }
})


--------------------------------------------
-- command registration: no counters NAT
--------------------------------------------

CLI_addCommand("config", "no counters nat-offset", {
    func=function(params)
        params.flagNo = true
        return cncOffsetForNatClientSet(params)
    end,
    help="Set default CNC offset for NAT client.",
    params={
        { type="named",
            { format="device %devID_all", name="devID" ,help="The device number"},
            mandatory = {"devID"}
        }
    }
})

-- ************************************************************************
---
--  cncOffsetForNatClientGet
--        @description  show CNC offset for NAT client
--
--        @param params none
--
--        @return       true on success, otherwise false and error message
--
local function cncOffsetForNatClientGet(params)
    local ret, val
    local devNum = tonumber(params["devID"])

    ret,val = myGenWrapper("cpssDxChCncOffsetForNatClientGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","offsetPtr"}
    })
    if ret~=0 then
        print("cncOffsetForNatClientGet() failed: "..returnCodes[ret])
        return false,"Error, Cnc offset for NAT Client could not be read"
    end

    print("Cnc offset for NAT Client: "..val["offsetPtr"])

    return true
end

--------------------------------------------------------------------------------
-- command registration: show counters nat-offset
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show counters nat-offset", {
        func=function(params)
            return cncOffsetForNatClientGet(params)
        end,
        help="Show CNC offset for NAT client.",
        help   = "Show CNC offset for NAT client",
        params={
                 { type="named",
                 { format="device %device", name="devID" ,help="The device number"},
                 mandatory = {"devID"}
                }
            }
})



-- ************************************************************************
--  arpNatCounters
--
--  @description command enables NAT client in the CNC.
--
--  @param params - params["devID"]        - device number.
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function arpNatCounters(params)
    local ret, val
    local blockNumber, i
    local devNum = params["devID"]
    local enable
    local devices
    
    if (params.flagNo == nil) then
        enable = true
    else
        enable = false
    end

    -- enums
    local CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E = 8

    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end
    for j=1,#devices do
        -- enable counting for ARP/NAT/TTI client
        ret = myGenWrapper("cpssDxChCncCountingEnableSet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT","cncUnit","CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E"},
                {"IN","GT_BOOL","enable",true}
            })
        if ret~=0 then
                print("cpssDxChCncCountingEnableSet() failed: "..returnCodes[ret])
                return false,"Error, counting enable could not be set"
        end

        local blockNumber = getCNCBlock("ArpNatTableAccess")
        if blockNumbers == -1 then
          return false, "Wrong client name for selected device"
        end
        for i=1, #blockNumber do
            ret = myGenWrapper("cpssDxChCncBlockClientEnableSet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","blockNum",blockNumber[i]},
                {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E},
                {"IN","GT_BOOL","updateEnable",true}
            })
            if ret~=0 then
                print("cpssDxChCncBlockClientEnableSet() failed: "..returnCodes[ret])
                return false,"Error, counter could not be set"
            end
            -- local indexRangesBmp = { l={[0]=1,[1]=0}} -- table to set GT_U64
            ret, val = myGenWrapper("cpssDxChCncBlockClientRangesSet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","blockNum",blockNumber[i]},
                {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E},
                --[[{"IN","GT_U64","indexRangesBmp", indexRangesBmp}--]]
                {"IN","GT_U32","indexRangesBmp_low", 1},
                {"IN","GT_U32","indexRangesBmp_high", 0}
            })
            if ret~=0 then
                print("cpssDxChCncBlockClientRangesSet() failed: "..returnCodes[ret])
                return false,"Error, counter could not be set"
            end

            ret, val = myGenWrapper("cpssDxChCncClientByteCountModeSet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E},
                {"IN","CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT","countMode", "CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E"}
            })
            if ret~=0 then
                print("cpssDxChCncClientByteCountModeSet() failed: "..returnCodes[ret])
                return false,"Error, counter could not be set"
            end
        end
    end

    if (enable == true) then
        globalStore("ArpNat", "pass")
    else
        if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end
        for j=1,#devices do
            -- disable counting for ARP/NAT/TTI client
            ret = myGenWrapper("cpssDxChCncCountingEnableSet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT","cncUnit","CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E"},
                    {"IN","GT_BOOL","enable",false}
                })
            if ret~=0 then
                    print("cpssDxChCncCountingEnableSet() failed: "..returnCodes[ret])
                    return false,"Error, counting enable could not be set"
            end

            local blockNumber = getCNCBlock("ArpNatTableAccess")
            if blockNumbers == -1 then
              return false, "Wrong client name for selected device"
            end
            for i=1, #blockNumber do
                ret = myGenWrapper("cpssDxChCncBlockClientEnableSet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","blockNum",blockNumber[i]},
                    {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E},
                    {"IN","GT_BOOL","updateEnable",false}
                })
                if ret~=0 then
                    print("cpssDxChCncBlockClientEnableSet() failed: "..returnCodes[ret])
                    return false,"Error, counter could not be set"
                end
            end
        end


        globalStore("ArpNat",nil)
    end

    return true
end

--------------------------------------------
-- command registration: counters ARP_NAT
--------------------------------------------

CLI_addCommand("config", "counters ARP_NAT", {
  func=arpNatCounters,
  help="enables ARP/NAT client in the CNC.",
   params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
        mandatory = {"devID"}
    }
  }
})


--------------------------------------------
-- command registration: no counters ARP_NAT
--------------------------------------------

CLI_addCommand("config", "no counters ARP_NAT", {
  func=function(params)
            params.flagNo = true
            return arpNatCounters(params)
        end,
  help="disable ARP/NAT client in the CNC.",
   params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
        mandatory = {"devID"}
    }
   }
})


