--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tti_rule.lua
--*
--* DESCRIPTION:
--*       tti commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--constants


--includes


-- variables definitions
local    devNum
local    packetType
local    index
local    keysAndMasks
local    actions
local    key_values_options
local    chosenKey
local    chosenAction
local    isMask
local    listOfKeys
local    listOfActions
local    enumKeyTypeTbl

CLI_type_dict["macMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Sets the lookup Mac mode for the specified key type",
    enum = {
        ["mac-mode-da"] = { value=0, help="Use destination MAC to generate lookup TCAM key" },
        ["mac-mode-sa"] = { value=1, help="Use source MAC to generate lookup TCAM key" }
    }
}

-- ************************************************************************
--
--  clear_variables
--        @description  clears all global variables
--

local function clear_variables()
    devNum = nil
    packetType = nil
    index = nil
    chosenKey = nil
    chosenAction = nil
    listOfKeys = nil
    listOfActions = nil
    keysAndMasks = {}
    actions = {}
    key_values_options = {}
    isMask = false
    -- macMode = nil
end

-- ************************************************************************
--
--  table.contains
--        @description  return true if tbl contains element, false otherwise
--

function table.contains(tbl, element)
  local vals
  for i, vals in pairs(tbl) do
    if vals ~= nil then
        if vals == element then
          return true
        end
    end
  end
  return false
end

-- ************************************************************************
--
--  getIndexInTCAM
--        @description  return the actual index of the rule in TCAM
--

function getIndexInTCAM(relativeIndex, deviceNum)
  local offsetTTI0, vvvv

    local devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(deviceNum)", deviceNum)
    if (not is_sip_5(deviceNum)) then
        return relativeIndex
    end
    offsetTTI0,vvvv= myGenWrapper("appDemoDxChTcamTtiBaseIndexGet",{
         {"IN","GT_U8","devNum",deviceNum},
         {"IN","GT_U32","hitNum",0}
    })

    return offsetTTI0 + relativeIndex
end

-- ************************************************************************
--
--  getNumberOfTTIindices
--        @description  return the number of indices exists in the tti section of the TCAM
--
function getNumberOfTTIindices(deviceNum)
    if (not is_sip_5(deviceNum)) then
        return 64
    end
    local ttiSize = 0
    for i=0,3 do
        local ttiHitSize = myGenWrapper(
        "appDemoDxChTcamTtiNumOfIndexsGet", {
            {"IN","GT_U8", "devNum", deviceNum},
            {"IN","GT_U32","hitNum",0} })
        ttiSize = ttiSize + ttiHitSize
    end
    return ttiSize
end

-- ************************************************************************
---
--  tti_rule_enter
--        @description  enter to tti-rule mode
--
--        @param params         -   params["devID"]: device number
--                                  params["packetType"]: Packet type.
--                                  params["index"]: index of the rule (starting from 0)
--

local function tti_rule_enter(params)
    clear_variables()
    devNum = params["devID"]
    packetType = params["packetType"]
    index = params["index"]
    index = getIndexInTCAM(index,devNum)
    CLI_change_mode_push("tti-rule")
end


-- ************************************************************************
---
--  apply_tti_rule
--        @description  apply the configurations set in the tti-rule mode.
--            this function is called when exit back to the config mode
--

local function apply_tti_rule(params)
    local patternTable, patternMaskTable, actionType, actionTable
    local mask

    -- define constant enums
    local CPSS_INTERFACE_PORT_E = 0
    local CPSS_INTERFACE_VIDX_E = 2
    local CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E = 1
    local CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E = 1
    local CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E = 1
    local GT_OK = 0
    local GT_BAD_PARAM = 4

    -- define full masks
    local fullMaskForU32 = 0xFFFFFFFF
    local fullMaskForU16 = 0xFFFF
    local fullMaskFor13 = 0x1FFF
    local fullMaskForU8 = 0xFF
    local fullMaskForBool = true

    actionType = 1

    ------------------- handle the keys and masks --------------------------
    patternTable = {}
    patternTable["eth"] = {}
    patternTable["eth"]["common"] = {}

    patternMaskTable = {}
    patternMaskTable["eth"] = {}
    patternMaskTable["eth"]["common"] = {}
    -- handle input-interface if defined
    if (keysAndMasks["input-if"] ~=nil and keysAndMasks["input-if"]["keyVal"] ~= nil) then
        patternTable["eth"]["common"]["srcIsTrunk"] = false
        patternTable["eth"]["common"]["srcPortTrunk"] = tonumber(keysAndMasks["input-if"]["keyVal"])
        mask = keysAndMasks["input-if"]["mask"]
        patternMaskTable["eth"]["common"]["srcIsTrunk"] = fullMaskForBool
        if mask == nil then
            patternMaskTable["eth"]["common"]["srcPortTrunk"] = fullMaskFor13
        else
            mask = tonumber(mask)
            patternMaskTable["eth"]["common"]["srcPortTrunk"] = math.min(mask, fullMaskFor13)
        end
    end

    -- handle tag0 if defined
    if (keysAndMasks["tag0"] ~=nil and keysAndMasks["tag0"]["keyVal"] ~= nil) then
        patternTable["eth"]["common"]["isTagged"] = keysAndMasks["tag0"]["keyVal"]
        patternMaskTable["eth"]["common"]["isTagged"] = fullMaskForBool
    end

    -- handle tag1 if defined
    if (keysAndMasks["tag1"] ~=nil and keysAndMasks["tag1"]["keyVal"] ~= nil) then
        patternTable["eth"]["isVlan1Exists"] = keysAndMasks["tag1"]["keyVal"]
        patternMaskTable["eth"]["isVlan1Exists"] = fullMaskForBool
    end

    -- handle vlan0 if defined
    if (keysAndMasks["vlan0"] ~=nil and keysAndMasks["vlan0"]["keyVal"] ~= nil) then
        patternTable["eth"]["common"]["vid"] = tonumber(keysAndMasks["vlan0"]["keyVal"])
        mask = keysAndMasks["vlan0"]["mask"]
        if mask == nil then
            patternMaskTable["eth"]["common"]["vid"] = fullMaskFor13
        else
            mask = tonumber(mask)
            patternMaskTable["eth"]["common"]["vid"] = math.min(mask, fullMaskFor13)
        end
    end

    -- handle vlan1 if defined
    if (keysAndMasks["vlan1"] ~=nil and keysAndMasks["vlan1"]["keyVal"] ~= nil) then
        patternTable["eth"]["vid1"] = tonumber(keysAndMasks["vlan1"]["keyVal"])
        mask = keysAndMasks["vlan1"]["mask"]
        if mask == nil then
            patternMaskTable["eth"]["vid1"] = fullMaskForU16
        else
            mask = tonumber(mask)
            patternMaskTable["eth"]["vid1"] = math.min(mask, fullMaskForU16)
        end
    end

    -- handle mac if defined
    if (keysAndMasks["mac"] ~=nil and keysAndMasks["mac"]["keyVal"] ~= nil) then
        patternTable["eth"]["common"]["mac"] = keysAndMasks["mac"]["keyVal"]
        patternMaskTable["eth"]["common"]["mac"] = keysAndMasks["mac"]["keyVal"]
    end

    ------------------- handle the actions --------------------------
    actionTable = {}
    actionTable["type2"] = {}
    actionTable["type2"]["egressInterface"] = {}

    -- handle packet-command if defined
    if (actions["packet-command"] ~= nil) then
        actionTable["type2"]["command"] = actions["packet-command"]
    end

    -- handle redirect egress-interface if defined
    if (actions["egress-interface"] ~= nil) then
        actionTable["type2"]["redirectCommand"] = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
        local egressType
        if (actions["egress-interface"]["evidx"] ~= nil) then
            egressType = CPSS_INTERFACE_VIDX_E
            actionTable["type2"]["egressInterface"]["vidx"] = tonumber(actions["egress-interface"]["evidx"])
        elseif (actions["egress-interface"]["ethernet"] ~= nil) then
            egressType = CPSS_INTERFACE_PORT_E
            actionTable["type2"]["egressInterface"]["devPort"] = {}
            local softDevId, softPortNum
            softDevId = tonumber(actions["egress-interface"]["ethernet"]["devId"])
            local ret, hwDevId = device_to_hardware_format_convert(softDevId)
            actionTable["type2"]["egressInterface"]["devPort"]["devNum"] = hwDevId
            actionTable["type2"]["egressInterface"]["devPort"]["portNum"] = tonumber(actions["egress-interface"]["ethernet"]["portNum"])
        end
        actionTable["type2"]["egressInterface"]["type"] = egressType

    end

    -- handle vlan0-assign-mod if defined
    if (actions["vlan0_assign_modify"] ~= nil) then
        actionTable["type2"]["tag0VlanCmd"] = actions["vlan0_assign_modify"]
    end

    -- handle vlan0-assign-vid if defined
    if (actions["vlan0_assign_vlanID"] ~= nil) then
        actionTable["type2"]["tag0VlanId"] = tonumber(actions["vlan0_assign_vlanID"])
    end

    -- handle vlan0-assign hard-precedence if defined
    if (actions["vlan0_assign_hardP"] ~= nil) then
        actionTable["type2"]["tag0VlanPrecedence"] = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E
    end

    -- handle vlan1-assign-mod if defined
    if (actions["vlan1_assign_modify"] ~= nil) then
        actionTable["type2"]["tag1VlanCmd"] = actions["vlan1_assign_modify"]
    else
        actionTable["type2"]["tag1VlanCmd"]  = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E
    end

    -- handle vlan1-assign-vid if defined
    if (actions["vlan1_assign_vlanID"] ~= nil) then
        actionTable["type2"]["tag1VlanId"] = tonumber(actions["vlan1_assign_vlanID"])
    end

    -- handle vlan-nested if defined
    if (actions["vlan-nested"] ~= nil) then
        actionTable["type2"]["nestedVlanEnable"] = true
    end

    -- handle bridge-bypass if defined
    if (actions["bridge-bypass"] ~= nil) then
        actionTable["type2"]["bridgeBypass"] = true
    end

    local ret,val = myGenWrapper("cpssDxChTtiRuleSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","index",index},
                {"IN","CPSS_DXCH_TTI_RULE_TYPE_ENT","ruleType",packetType},
                {"IN","CPSS_DXCH_TTI_RULE_UNT_eth","patternPtr",patternTable},
                {"IN","CPSS_DXCH_TTI_RULE_UNT_eth","maskPtr",patternMaskTable},
                {"IN","CPSS_DXCH_TTI_ACTION_TYPE_ENT","actionType",actionType},
                {"IN","CPSS_DXCH_TTI_ACTION_UNT_type2","actionPtr",actionTable}
    })

    if (ret == GT_OK) then
        print("rule " .. index .." created")
    elseif (ret == GT_BAD_PARAM) then
        print("Error: could not create rule - Bad Params")
    else
        print("Error: could not create rule")
    end


    clear_variables()
    CLI_change_mode_pop()
end

-- ************************************************************************
---
--  abort_tti_rule
--        @description  cancel the configurations and exit back to config mode
--

local function abort_tti_rule(params)
    clear_variables()
    CLI_change_mode_pop()
end

-- ************************************************************************
---
--  add_rule_key
--        @description  adds keys and masks to the saved set of keys and mask
--            this function doesn't call an API.
--            only when exiting the tti-rule mode it will be called.
--

local function add_rule_key(params)
    local keyVal, mask, flagNo
    flagNo = params["flagNo"]

    params["flagNo"] = nil

    if next(params) == nil then
        print("Error: not enough arguments")
        return
    end

    for kParams,vParams in pairs(params) do -- runs over the keys and masks entered
        if kParams ~= "_cmdline" then
            local i = 1
            for kActions, vActions in pairs(keysAndMasks) do -- delete the existing key and mask in order to override
                if kParams == kActions then
                    keysAndMasks[kActions] = nil
                end
                i = i +1
            end
            if flagNo == nil then
                if (kParams ~= "tag0" and kParams ~= "tag1" and kParams ~= "mac") then -- rewrite the key and mask
                    if (tonumber(vParams) ~= nil) then -- no mask
                        keyVal = vParams
                    else
                        local sIndex = string.find(vParams, "/") -- mask is entered
                        keyVal = string.sub(vParams, 0, sIndex-1)
                        mask = string.sub(vParams, sIndex+1)
                    end
                else
                    keyVal = vParams
                end
                keysAndMasks[kParams] = { keyVal = keyVal, mask = mask }
            end
        end
    end
end

-- ************************************************************************
---
--  add_rule_action
--        @description  adds action to the saved set of actions
--            this function doesn't call an API.
--            only when exiting the tti-rule mode it will be called.
--


local function add_rule_action(params)
    local flagNo
    flagNo = params["flagNo"]

    params["flagNo"] = nil

    if params["vlan0-assign"] ~= nil then
        params["vlan0_assign_vlanID"] = "vlan0_assign_vlanID"
        params["vlan0_assign_modify"] = "vlan0_assign_modify"
        params["vlan0_assign_hardP"] = "vlan0_assign_hardP"
    end

    if params["vlan1-assign"] ~= nil then
        params["vlan1_assign_vlanID"] = "vlan1_assign_vlanID"
        params["vlan1_assign_modify"] = "vlan1_assign_modify"
    end


    if params["egress-interface"] ~= nil then
        local egressType = params["egress-interface"]
        local egressTbl = {}
        egressTbl[egressType] = params[egressType]

        params["egress-interface"] = egressTbl
        params[egressType] = nil
    end

    if params["redirect"] ~= nil then
        params["egress-interface"] = "egress-interface"
    end

    for kParams,vParams in pairs(params) do
        local i = 1
        for kActions, vActions in pairs(actions) do
            if kParams == kActions then
                actions[kParams] = nil
            end
            i = i +1
        end
        if (flagNo == nil) then
            actions[kParams] = vParams
        end
    end
end

--------------------------------------------
-- command registration: tti rule create
--------------------------------------------

CLI_addHelp("config", "tti", "TTI configurations")
CLI_addHelp("config", "tti rule", "Enter tti-rule command mode")
CLI_addCommand("config", "tti rule create", {
    help="Enter tti-rule command mode",
    func=tti_rule_enter,
    params={
        { type="values",
          "%devID",
          "%index",
          "%packetType"
        }
    }
})

--------------------------------------------
-- command registration: tti rule-key
--------------------------------------------
CLI_addHelp("tti-rule", "tti", "TTI configurations, define keys and actions")
CLI_addCommand("tti-rule", "tti rule-key", {
    help="define a TTI key pattern and mask",
    func=add_rule_key,
    params={
        { type="named",
          { format="index %UDBIndex", name="UDBIndex"}
        },
        { type="named",
          { format="input-interface %portNumWithMask",name = "input-if", help = "Input interface"},
          { format="tag0 %tagged", name = "tag0",help = "Specifies if  tag0 is exist"},
          { format="vlan0 %vlanIdWithMask", name = "vlan0",help = "VLAN0_ID, If mask is not defined then mask is all 1"},
          { format="tag1 %tagged", name = "tag1",help = "Specifies if  tag1 is exist"},
          { format="vlan1 %vlanIdWithMask", name = "vlan1",help = "VLAN1_ID, If mask is not defined then mask is all 1"},
          { format="mac-addr %mac-address", name="mac", help="source mac address or destination mac address"}
        }
    }
})

--------------------------------------------
-- command registration: no tti rule-key
--------------------------------------------

CLI_addCommand("tti-rule", "no tti rule-key", {
    help="undefine a TTI key pattern",
    func=function(params)
            params.flagNo = true
            add_rule_key(params)
        end,
    params={
        { type="named",
          { format="input-interface", help = "Undefine Input interface"},
          { format="tag0", help = "Undefine tag0"},
          { format="vlan0", help = "Undefine vlan0"},
          { format="tag1", help = "Undefine tag1"},
          { format="vlan1", help = "Undefine vlan1"},
          { format="mac", help = "Undefine mac address"}
         }
    }
})
--------------------------------------------
-- command registration: exit tti-rule mode
--------------------------------------------

CLI_addCommand("tti-rule", "exit", {
    func=apply_tti_rule,
    help="Apply configurations"
})

--------------------------------------------
-- command registration: abort tti-rule mode
--------------------------------------------

CLI_addCommand("tti-rule", "abort", {
    func = CLI_change_mode_pop,
    help = "Exit without applying (drop configurations)"
})


--------------------------------------------
-- command registration: tti rule-action packet-command
--------------------------------------------

CLI_addCommand("tti-rule", "tti rule-action packet-command", {
    help="Forwarding command",
    func=add_rule_action,
    params={
        { type="values",
          { format="%packet-command", help = "Forwarding command"}
        }
    }
})


--------------------------------------------
-- command registration: tti rule-action vlan1-assign
--------------------------------------------

CLI_addCommand("tti-rule", "tti rule-action vlan0-assign", {
    help="define vlan0 assignment",
    func=add_rule_action,
    params={
        { type="values",
          { format="%modify", name="vlan0_assign_modify" , help = "tag0 vlan command"},
          { format="%vlanId", name="vlan0_assign_vlanID" ,help = "tag0 VLAN-ID assignment"}
        },
        { type="named",
          { format="hard-precedence",name="vlan0_assign_hardP" , help = "whether the VID assignment can be overridden"}
        }
    }
})


--------------------------------------------
-- command registration: tti rule-action vlan1-assign
--------------------------------------------

CLI_addCommand("tti-rule", "tti rule-action vlan1-assign", {
    help="define vlan1 assignment",
    func=add_rule_action,
    params={
        { type="values",
          { format="%modify", name="vlan1_assign_modify" , help = "tag1 vlan command"},
          { format="%vlanId", name="vlan1_assign_vlanID" ,help = "tag1 VLAN-ID assignment"}
        },
        { type="named",
          { format="hard-precedence",name="vlan1_assign_hardP" , help = "whether the VID assignment can be overridden"}
        }
    }
})

--------------------------------------------
-- command registration: tti rule-action bridge-bypass
--------------------------------------------
CLI_addCommand("tti-rule", "tti rule-action", {
    help="If set the packet isn't subject to any bridge mechanism",
    func=add_rule_action,
    params={
    { type="named",
      { format="bridge-bypass",name="bridge-bypass" , help = "If set the packet isn't subject to any bridge mechanism"},
      mandatory = {"bridge-bypass"}
    }
    }
})

--------------------------------------------
-- command registration: tti rule-action vlan-nested enable
--------------------------------------------
CLI_addHelp("tti-rule", "tti rule-action", "enable nested vlan")
CLI_addCommand("tti-rule", "tti rule-action vlan-nested", {
    help="enable nested vlan",
    func=add_rule_action,
    params={
    { type="named",
      { format="enable",name="vlan-nested" , help = "enable nested vlan"},
      mandatory = {"vlan-nested"}
    }
    }
})

--------------------------------------------------------------------
-- command registration: tti rule-action redirectegress-interface
--------------------------------------------------------------------

CLI_addHelp("tti-rule", "tti rule-action redirect", "Where to redirect the packet")
CLI_addCommand("tti-rule", "tti rule-action redirect egress-interface", {
    help="Where to redirect the packet",
    func=add_rule_action,
    params={
        { type="named",
          { format="evidx %vlanId", name = "egress-interface",help = "vidx number"},
          { format="ethernet %dev_port", name = "egress-interface", help = "device number/port number"}
        }
    }
})

--------------------------------------------
-- command registration: no tti rule-action
--------------------------------------------

CLI_addCommand("tti-rule", "no tti rule-action", {
    help="undefine a TTI action",
    func=function(params)
            params.flagNo = true
            add_rule_action(params)
        end,
    params={
        { type="named",
          { format="packet-command", help = "Undefine packet-command"},
          { format="vlan0-assign",  help = "Undefine vlan0-assign"},
          { format="vlan1-assign",  help = "Undefine vlan1-assign"},
          { format="vlan-nested", help = "Undefine vlan-nested"},
          { format="bridge-bypass", help = "Undefine bridge-bypass"},
          { format="redirect", help = "Undefine redirect"}
        }
    }
})

-- ************************************************************************
---
--  tti_rule_pclId
--        @description  set pclId
--
--        @param devNum             - device number
--        @param packetType        - TTI rule type
--        @param pclID              - pclID
--
--
local function tti_rule_pclId(params)
    local devNum = params["devID"]
    local packetType = params["packetType"]
    local pclId = params["pclID"]
    local ret,val = myGenWrapper("cpssDxChTtiPclIdSet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_DXCH_TTI_KEY_TYPE_ENT","keyType",packetType},
            {"IN","GT_U32","pclId",pclId}
    })

    if ret ~= 0 then
        print("Cannot set pclId " .. to_string(val))
    end

end


-- ************************************************************************
---
--  tti_rule_macMode
--        @description  set sets the lookup Mac mode
--
--        @param devNum             - device number
--        @param packetType        - TTI rule type
--        @param macMode          - MAC mode to use: MAC DA/MAC SA
--
--
local function tti_rule_macMode(params)
    local devNum = params["devID"]
    local packetType = params["packetType"]
    local macMode = params["macMode"]

    local ret,val = myGenWrapper("cpssDxChTtiMacModeSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_DXCH_TTI_KEY_TYPE_ENT","keyType",packetType},
        {"IN","CPSS_DXCH_TTI_MAC_MODE_ENT","macMode",macMode}
    })

    if ret ~= 0 then
        print("Cannot set mac mode " .. to_string(val))
    end

end

-- ************************************************************************
---
--  tti_rule_enable
--        @description  enable/disable tti rules on specific ports
--
--        @param devNum             - device number
--        @param packetType        - TTI rule type
--        @param flagNo              - if flag exists, than disable the tti rules on ports
--
--
local function tti_rule_enable(params)
    -- Main port handling cycle
    local command_data = Command_Data()
    local keyType = params["packetType"]
    local flag = true

    if (params["flagNo"] ~= nil) then
        flag = false
    end

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    command_data:initInterfaceDeviceRange()

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            local ret,val = myGenWrapper("cpssDxChTtiPortLookupEnableSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PORT_NUM","portNum",portNum},
                {"IN","CPSS_DXCH_TTI_KEY_TYPE_ENT","keyType",keyType},
                {"IN","GT_BOOL","enable",flag}
            })
            if (ret ~= 0) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("failed to enable tti on port %d device %d" , portNum, devNum)
            end

        end


    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()
end

--------------------------------------------
-- command registration: tti pclid
--------------------------------------------
CLI_addCommand("config", "tti pclid", {
    help="define a PCLID of TTI Lookup a given packet type",
    func=tti_rule_pclId,
    params={
        { type="values",
          "%devID",
          "%packetType",
          "%pclID"
        }
    }
})

--------------------------------------------
-- command registration: tti mac mode
--------------------------------------------
CLI_addCommand("config", "tti mac-mode", {
    help="define the lookup Mac mode for the specified key type",
    func=tti_rule_macMode,
    params={
        { type="values",
            "%devID",
            "%packetType",
            "%macMode"
        }
    }
})

--------------------------------------------
-- command registration: tti enable
--------------------------------------------
CLI_addHelp("interface", "tti", "TTI configurations")
CLI_addCommand("interface", "tti enable", {
    help="enable TTI Lookup of a given packet type",
    func=tti_rule_enable,
    params={
        { type="values",
          "%packetType"
        }
    }
})

--------------------------------------------
-- command registration: tti disable
--------------------------------------------
CLI_addHelp("interface", "tti", "TTI configurations")
CLI_addCommand("interface", "tti disable", {
    func = function(params)
        params["flagNo"] = true
        tti_rule_enable(params)
    end,
    help   = "disable TTI Lookup of a given packet type",
    params={
        { type="values",
            "%packetType"
        }
    }
})
