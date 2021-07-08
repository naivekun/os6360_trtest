--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_isolate.lua
--*
--* DESCRIPTION:
--*       Port isolation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants
local vlanId = 1

cmdLuaCLI_registerCfunction("wrlCpssDxChNstPortIsolationTableEntrySet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNstPortIsolationPortDelete")
cmdLuaCLI_registerCfunction("wrlCpssDxChNstPortIsolationTableEntryGet")

local function init_interface_info_stc()
    return {['fabricVidx']  = 0,
            ['type']        = "CPSS_INTERFACE_PORT_E",
            ['index']       = 0,
            ['vidx']        = 0,
            ['vlanId']      = 0,
            ['devNum']      = 0,
            ['trunkId']     = 0,
            ['devPort']     ={['portNum']   = 0,
                              ['devNum']    = 0}}
end

-- return false on error
-- return interfaceInfo on success
local function build_interface_info_stc()
    local result, values
    local interfaceInfo
    interfaceInfo = init_interface_info_stc()
    interfaceInfo.type = nil
    
    local ifType = getGlobal("ifType")
    local ifPortChannel = getGlobal("ifPortChannel")

    local dev_list = wrLogWrapper("wrlDevList")

    
    for dummy, devNum in pairs(dev_list) do
        --local devNum = device_to_hardware_format_convert(dev)
        if is_sip_5(devNum) then
            result, values =
            myGenWrapper("cpssDxChNstPortIsolationLookupBitsGet", {
                { "IN",  "GT_U8",  "devNum",   devNum },
                { "OUT", "GT_U32", "numberOfPortBitsPtr"  },
                { "OUT", "GT_U32", "numberOfDeviceBitsPtr"},
                { "OUT", "GT_U32", "numberOfTrunkBitsPtr" }});

            if result ~= 0 then
                print("Error while getting Port Isolation Lookup Bits information. "..returnCodes[result])
                return false
            end
        end

        break
    end

    if ifPortChannel ~= nil then
        interfaceInfo.type = "CPSS_INTERFACE_TRUNK_E"
        interfaceInfo.trunkId = ifPortChannel
        if is_sip_5(devNum) then
            interfaceInfo.trunkId = interfaceInfo.trunkId % values["numberOfTrunkBitsPtr"]
        end
    elseif ifType == "ethernet" then
        local ifRange = getGlobal("ifRange")
        local numports = 0
        local dev, ports
        for dev,ports in pairs(ifRange) do
            res0, interfaceInfo.devPort.devNum = device_to_hardware_format_convert(dev)
            interfaceInfo.type = "CPSS_INTERFACE_PORT_E"
            interfaceInfo.devNum = interfaceInfo.devPort.devNum
            interfaceInfo.devPort.portNum = ports[1]
            numports = numports + #ports

            --print("interfaceInfo.devPort.portNum: "..interfaceInfo.devPort.portNum)
            --print("interfaceInfo.devPort.devNum: "..interfaceInfo.devPort.devNum)
            if is_sip_5(dev) then
                -- the port is 6 bits value , see calling to cpssDxChNstPortIsolationLookupBitsSet(...)
                interfaceInfo.devPort.portNum = interfaceInfo.devPort.portNum % bit_shl(1, values["numberOfPortBitsPtr"])
                interfaceInfo.devPort.devNum  = interfaceInfo.devPort.devNum  % bit_shl(1, values["numberOfDeviceBitsPtr"])
                interfaceInfo.devNum          = interfaceInfo.devNum          % bit_shl(1, values["numberOfDeviceBitsPtr"])
            end
            --print("interfaceInfo.devPort.portNum: "..interfaceInfo.devPort.portNum)
            --print("interfaceInfo.devPort.devNum: "..interfaceInfo.devPort.devNum)

        end
        if numports ~= 1 then
            print("Only one port should be specified in the interface.")
            return false
        end
    else
        print("Interface of not supported type.")
        return false
    end

    return interfaceInfo
end


local function switchport_isolate_func(params)
    local interfaceInfo
    local result, i, portTable
    local res0
    local devNum
    local dev_list = wrLogWrapper("wrlDevList")
    local values
    local vlanInfo
    
    local cmd  = "CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E"
    local mode = "CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E"

    if(params.cmd) then
        if("L2" == params.cmd) then
            cmd  = "CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E"
            mode = "CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E"
        elseif("L3" == params.cmd) then
            cmd  = "CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E"
            mode = "CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E"
        elseif("L2L3" == params.cmd) then
            cmd  = "CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E"
            mode = "CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E"
        else
            print("Wrong cmd value given "..cmd". Use 'L2', 'L3' or 'L2L3'")
            return false
        end
    else
        params.cmd = "L2"
    end

    if(nil == explicit_set_PortIsolationLookupBits) then
        explicit_set_PortIsolationLookupBits = 0
    end
    -- must be called before build_interface_info_stc() 
    -- that relay on settings of cpssDxChNstPortIsolationLookupBitsSet(...)
    for dummy, devNum in pairs(dev_list) do
        if is_sip_5(devNum) and (0 == explicit_set_PortIsolationLookupBits) then
            -- patch for BC2 devices with hwDevId > 0x10
            -- the table has 0x880 entries
            -- default index expression is dev[4:0]port[7:0]
            -- update it to dev[4:0]port[6:0]
            -- also support Falcon ports > 63 (6 bits)
            result, values =
            myGenWrapper("cpssDxChNstPortIsolationLookupBitsSet", {
                { "IN",     "GT_U8",           "devNum",           devNum },
                { "IN",    "GT_U32",           "numberOfPortBits",   6   },    -- default was 7
                { "IN",    "GT_U32",           "numberOfDeviceBits", 5     },  -- as default
                { "IN",    "GT_U32",           "numberOfTrunkBits",  7    }}); -- as default
            if result ~= 0 then
                print("Error while setting Port Isolation Lookup Bits information. "..returnCodes[result])
                return false
            end
        end
    end

    interfaceInfo = build_interface_info_stc()
    
    if(interfaceInfo == false) then
        return false
    end

    no_switchport_isolate_func(params) -- disable previously isolated ports

    for dummy, devNum in pairs(dev_list) do
        result, values =
        myGenWrapper("cpssDxChBrgVlanEntryRead", {
            { "IN",     "GT_U8",                        "devNum",   devNum },
            { "IN",     "GT_U16",                       "vlanId",   vlanId },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsMembers"     },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsTagging"     },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"         },
            { "OUT",    "GT_BOOL",                      "isValid"          },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",
                                                        "portsTaggingCmd"  }})

        if result ~= 0 then
            print("Error while getting vlan information. "..returnCodes[result])
            return false
        end

--        debug_table_print(values)
--        print("values.vlanInfo.portIsolationMode: "..values.vlanInfo.portIsolationMode)

        values.vlanInfo.portIsolationMode = cmd

        result =
            myGenWrapper("cpssDxChBrgVlanEntryWrite", {
                { "IN", "GT_U8",                        "devNum",       devNum       },
                { "IN", "GT_U16",                       "vlanId",       vlanId       },
                { "IN", "CPSS_PORTS_BMP_STC",           "portsMembers", values.portsMembers },
                { "IN", "CPSS_PORTS_BMP_STC",           "portsTagging", values.portsTagging },
                { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"    , values.vlanInfo     },
                { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",
                                                 "portsTaggingCmd", values.portsTaggingCmd  }})
        if result ~= 0 then
            print("Error while setting vlan information. "..returnCodes[result])
            return false
        end

        result, values=myGenWrapper("cpssDxChNstPortIsolationEnableSet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_BOOL","enable", true}
        })
        if result ~= 0 then
            print("Error while calling cpssDxChNstPortIsolationEnableSet. "..returnCodes[result])
            return false
        end
    end

    for devNum, portTable in pairs(params["port-range"]) do
        for i=1, #portTable do
            local portNum = portTable[i]

            if is_sip_5(devNum) then
                result, values=myGenWrapper("cpssDxChNstPortIsolationModeSet",{
                                {"IN","GT_U8","devNum",devNum},
                                {"IN","GT_PORT_NUM","portNum",portNum},
                                {"IN","CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT","mode", mode}
                })
                if result ~= 0 then
                    print("Error while calling cpssDxChNstPortIsolationModeSet. "..returnCodes[result])
                    return false
                end
            end

            if("L2" == params.cmd) then
                local traffic_type_ = "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E"
                result = wrLogWrapper("wrlCpssDxChNstPortIsolationPortDelete", 
                          "(devNum, traffic_type_, interfaceInfo, portNum)", 
                          devNum, traffic_type_, interfaceInfo, portNum)
                if result ~= 0 then
                    print("Error while calling wrlCpssDxChNstPortIsolationPortDelete. "..returnCodes[result])
                    return false
                end
            elseif("L3" == params.cmd) then
                local traffic_type_ = "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E"
                result = wrLogWrapper("wrlCpssDxChNstPortIsolationPortDelete", 
                          "(devNum, traffic_type_, interfaceInfo, portNum)", 
                          devNum, traffic_type_, interfaceInfo, portNum)
                if result ~= 0 then
                    print("Error while calling wrlCpssDxChNstPortIsolationPortDelete. "..returnCodes[result])
                    return false
                end
            elseif("L2L3" == params.cmd) then
                local traffic_type_ = "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E"
                result = wrLogWrapper("wrlCpssDxChNstPortIsolationPortDelete", 
                          "(devNum, traffic_type_, interfaceInfo, portNum)", 
                          devNum, traffic_type_, interfaceInfo, portNum)
                if result ~= 0 then
                    print("Error while calling wrlCpssDxChNstPortIsolationPortDelete. "..returnCodes[result])
                    return false
                end
                local traffic_type_ = "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E"
                result = wrLogWrapper("wrlCpssDxChNstPortIsolationPortDelete", 
                          "(devNum, traffic_type_, interfaceInfo, portNum)", 
                          devNum, traffic_type_, interfaceInfo, portNum)
                if result ~= 0 then
                    print("Error while calling wrlCpssDxChNstPortIsolationPortDelete. "..returnCodes[result])
                    return false
                end
            end

        end
    end
    return true
end

function no_switchport_isolate_func(params)
    local command_data = Command_Data()
    local localPortsMembers  = wrLogWrapper("wrlCpssClearBmpPort")
    local i
    local interfaceInfo
    local res0
    local result, values

    command_data:initInterfaceDevPortRange() 
    
    interfaceInfo = build_interface_info_stc()
    
    if(interfaceInfo == false) then
        return false
    end
    
--[[    
    params["all"] = true
    params["devID"] = nil

    command_data:initAllInterfacesPortIterator(params)
   ]]--

    for i=0,0xFF do
        localPortsMembers = wrLogWrapper("wrlCpssSetBmpPort", "(localPortsMembers, i)", localPortsMembers, i)
    end

    for iterator, devNum, portNum in command_data:getPortIterator() do
    
        local traffic_type_ = "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E"
        result = wrLogWrapper("wrlCpssDxChNstPortIsolationTableEntrySet", 
                        "(devNum, traffic_type_, interfaceInfo, true, localPortsMembers)",
                        devNum, traffic_type_, interfaceInfo, true, localPortsMembers) 

        command_data:handleCpssErrorDevPort(result, "wrlCpssDxChNstPortIsolationTableEntrySet")

        local traffic_type_ = "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E"
        result = wrLogWrapper("wrlCpssDxChNstPortIsolationTableEntrySet", 
                        "(devNum, traffic_type_, interfaceInfo, true, localPortsMembers)",
                        devNum, traffic_type_, interfaceInfo, true, localPortsMembers) 

        command_data:handleCpssErrorDevPort(result, "wrlCpssDxChNstPortIsolationTableEntrySet")

--[[ -- do not shut-down base on single port !!!
        result, values=myGenWrapper("cpssDxChNstPortIsolationEnableSet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_BOOL","enable", false}
        })
        if result ~= 0 then
            print("Error while calling cpssDxChNstPortIsolationEnableSet. "..returnCodes[result])
            return false
        end
--]]
    end


    -- Restore vlan 1 configuration
    local dev_list = wrLogWrapper("wrlDevList")
    local devNum

    for dummy, devNum in pairs(dev_list) do

        result, values =
        myGenWrapper("cpssDxChBrgVlanEntryRead", {
            { "IN",     "GT_U8",                        "devNum",   devNum },
            { "IN",     "GT_U16",                       "vlanId",   vlanId },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsMembers"     },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsTagging"     },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"         },
            { "OUT",    "GT_BOOL",                      "isValid"          },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",
                                                        "portsTaggingCmd"  }})

        if result ~= 0 then
            print("Error while getting vlan information. "..returnCodes[result])
            return false
        end

        values.vlanInfo.portIsolationMode = "CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E"

        result =
            myGenWrapper("cpssDxChBrgVlanEntryWrite", {
                { "IN", "GT_U8",                        "devNum",       devNum  },
                { "IN", "GT_U16",                       "vlanId",       vlanId  },
                { "IN", "CPSS_PORTS_BMP_STC",           "portsMembers", values.portsMembers },
                { "IN", "CPSS_PORTS_BMP_STC",           "portsTagging", values.portsTagging },
                { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"    , values.vlanInfo     },
                { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",
                                                     "portsTaggingCmd", values.portsTaggingCmd  }})
        if result ~= 0 then
            print("Error while setting vlan information. "..returnCodes[result])
            return false
        end

--        debug_table_print(values)
     end


    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------
-- command registration: switchport isolate
--------------------------------------------------------
CLI_addCommand("interface", "switchport isolate", {
  func   = switchport_isolate_func,
  help   = "Configure ports to isolate",
  params = {
               { type = "values","%port-range"},
               { type = "named",
                   { format="cmd %s", name="cmd", help="The command: L2, L3 or L2L3, default is L2" },
               }
           }
})


--------------------------------------------------------
-- command registration: no switchport isolate
--------------------------------------------------------
CLI_addCommand("interface", "no switchport isolate", {
  func   = no_switchport_isolate_func,
  help   = "Disable ports isolating",
  -- params = {
        -- { type = "values","%port-range"}
  -- }
})

