--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_tpid.lua
--*
--* DESCRIPTION:
--*       configure tpid
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local function tpid_func(params)
    local devlist, direction, entryIndex, etherType, tag_type;

    if params.direct_type == "ingress" then
        direction = "CPSS_DIRECTION_INGRESS_E"
    elseif params.direct_type == "egress" then
        direction = "CPSS_DIRECTION_EGRESS_E"
    else
        print("Wrong direction type")
        return false
        --  direction = nil
    end

    if params["type"] == "4-bytes-tag" then
        tag_type = "CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E"
    elseif params["type"] == "8-bytes-tag" then
        tag_type = "CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E"
    elseif params["type"] == "6-bytes-tag" then
        tag_type = "CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E"
    else
        tag_type = nil
    end

    entryIndex = tonumber(params.entry_index)
    etherType = tonumber(params.ether_type)

    if params.devID ~= "all" then
        devlist = {params.devID}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    local i, devNum
    for i, devNum in pairs(devlist) do
        local result, values
        result, values = myGenWrapper(
            "cpssDxChBrgVlanTpidEntrySet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "CPSS_DIRECTION_ENT", "direction", direction},
                { "IN", "GT_U32", "entryIndex", entryIndex},
                { "IN", "GT_U16", "etherType", etherType}
            })
        if result ~= 0 then
            print("Error while executing cpssDxChBrgVlanTpidEntrySet")
            return false
        end
        if (tag_type ~= nil) then
            result, result = myGenWrapper(
                    "cpssDxChBrgVlanTpidTagTypeSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "CPSS_DIRECTION_ENT", "direction", direction},
                        { "IN", "GT_U32", "index", entryIndex},
                        { "IN", "CPSS_BRG_TPID_SIZE_TYPE_ENT", "type", tag_type}
                    })
                if result ~= 0 then
                    print("Error while executing cpssDxChBrgVlanTpidTagTypeSet")
                    return false
                end
        end
    end

    return true
end

CLI_type_dict["ingress_egress"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Ingress or egress",
    enum = {
        ["ingress"] = { value="ingress", help="Ingress" },
        ["egress"] =  { value="egress", help="Egress" }
   }
}

CLI_type_dict["4_or_8_bytes_tag"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "4/8/6 bytes-tag",
    enum = {
        ["4-bytes-tag"] = { value="4-bytes-tag", help="4-bytes-tag" },
        ["8-bytes-tag"] = { value="8-bytes-tag", help="8-bytes-tag" },
        ["6-bytes-tag"] = { value="6-bytes-tag", help="6-bytes-tag" }
   }
};

CLI_addCommand("config", "tpid", {
  func   = tpid_func,
  help   = "Configure tpid",
  params = {
      { type = "named",
          { format= "device %devID_all", name="devID", help="The device number" },
          { format= "direction %ingress_egress", name = "direct_type", help = "Direction"},
          { format= "index %d",     name = "entry_index",   help = "Entry index for TPID table (APPLICABLE RANGES: 0..7)"},
          { format= "etherType %ethertype", name = "ether_type",    help = "Ethernet type"},
          { format= "tag-type %4_or_8_bytes_tag", name = "type",    help = "Tag type"},
        mandatory = { "devID", "direct_type", "entry_index", "ether_type"}
      }
  }
})


local function tpid_func_ingress(params)
    local profiles, tpidBmp
    local devlist
    
    if params.devID ~= "all" then
        devlist = {params.devID}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    if params.profile == "all" then
        profiles={0,1,2,3,4,5,6,7}
    else
        profiles={params.profile}
    end

    tpidBmp = 0

    local i, devNum
    for i = 1, #params.entry_index_range do
        tpidBmp = bit_or(tpidBmp, bit_shl(1,params.entry_index_range[i]))
    end

    for key,pr in pairs(profiles) do
        for i, devNum in pairs(devlist) do
            local result, values = myGenWrapper(
                    "cpssDxChBrgVlanIngressTpidProfileSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_U32", "profile", pr},
                        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", params.ether_type_index},
                        { "IN", "GT_U32", "tpidBmp", tpidBmp}
                    })

            if result ~= 0 then
                print("Error while executing cpssDxChBrgVlanIngressTpidProfileSet")
                return false
            end
        end
    end
end

local function check_number_all(param,name,desc)
    if param == "all" then
        return true, param
    end
    return CLI_check_param_number(param,name,desc)
end
local function complete_number_all(param,name,desc)
    local compl, help = CLI_complete_param_number(param, name, desc)
    if prefix_match(param, "all") then
        table.insert(compl,"all")
        help[#compl] = "Apply to all profiles"
    end
    return compl, help
end

CLI_type_dict["profile_id"] = {
    checker  = check_number_all,
    complete = complete_number_all,
    min=0,
    max=7,
    help = "Profile ID"
}

CLI_type_dict["ether_type_index"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Ingress or egress",
    enum = {
        ["0"] = { value="CPSS_VLAN_ETHERTYPE0_E", help="Ethertype 0" },
        ["1"] =  { value="CPSS_VLAN_ETHERTYPE1_E", help="Ethertype 1" }
   }
}

CLI_addCommand("config", "tpid ingress", {
  func   = tpid_func_ingress,
  help   = "Configure tpid ingress profile",
  params = {
      { type = "named",
          { format = "profile %profile_id", name="profile", help="TPID profile." },
          { format = "device %devID_all",   name="devID",   help="The device number" },
          { format = "ether_type_index %ether_type_index", name = "ether_type_index", help = "TAG0/TAG1 selector {0 | 1}"},
          { format = "tpid-range %tpid_range",     name = "entry_index_range",   help = "range of tpids of command 3, for example 0-3,6"},
        mandatory = { "profile", "devID", "ether_type_index", "entry_index_range"}
      }
  }
})

local function tpid_func_ingress_bind(params)
    local command_data = Command_Data()

    if params.profile == "all" then
        params.profileList = {0,1,2,3,4,5,6,7}
    else
        params.profileList = {params.profile}
    end
    command_data:initInterfaceDevPortRange()
     -- now iterate
    command_data:iterateOverPorts(
        function(command_data, devNum, portNum, params)
            local result, i, profile
            for i,profile in pairs(params.profileList) do
                result = myGenWrapper(
                    "cpssDxChBrgVlanPortIngressTpidProfileSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_PORT_NUM", "portNum", portNum},
                        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", params.ether_type_index},
                        { "IN", "GT_BOOL", "isDefaultProfile", params.is_default},
                        { "IN", "GT_U32", "profile", profile}
                    })
                command_data:handleCpssErrorDevPort(result, "cpssDxChBrgVlanPortIngressTpidProfileSet(), profile: "..to_string(profile))
            end

        end, params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

CLI_addCommand({"interface", "interface_eport"}, "tpid ingress", {
  func   = tpid_func_ingress_bind,
  help   = "Binds ingress port to tpid profile",
  params = {
      { type = "named",
          { format = "profile %profile_id", name="profile", help="TPID profile." },
          { format = "ether_type_index %ether_type_index", name = "ether_type_index", help = "TAG0/TAG1 selector {0 | 1}"},
          { format = "is_default %bool", name="is_default", help="TPID profile." },
        mandatory = { "profile", "ether_type_index", "is_default"}
      }
  }
})

local function tpid_func_egress_bind(params)
    local command_data = Command_Data()

    command_data:initInterfaceDevPortRange()

    command_data:iterateOverPorts(
        function(command_data, devNum, portNum, params)
            local result
            result = myGenWrapper(
                "cpssDxChBrgVlanPortEgressTpidSet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PORT_NUM", "portNum", portNum},
                    { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", params.ether_type_index},
                    { "IN", "GT_U32", "tpidEntryIndex", params.bind},
                })
            command_data:handleCpssErrorDevPort(result, "cpssDxChBrgVlanPortEgressTpidSet(), index: ")
        end, params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

CLI_addCommand({"interface", "interface_eport"}, "tpid egress", {
  func   = tpid_func_egress_bind,
  help   = "Binds egress port to tpid index",
  params = {
      { type = "named",
          { format = "bind %tpid_index", name="bind", help="TPID table entry index" },
          { format = "ether_type_index %ether_type_index", name = "ether_type_index", help = "TAG0/TAG1 selector {0 | 1}"},
          mandatory = { "bind", "ether_type_index"}
      }
  }
})

local function tpid_func_tunnel_start_bind(params)
    local command_data = Command_Data()

    command_data:initInterfaceDevPortRange()

    command_data:iterateOverPorts(
        function(command_data, devNum, portNum, params)
            local result
            result = myGenWrapper(
                "cpssDxChTunnelStartHeaderTpidSelectSet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PORT_NUM", "portNum", portNum},
                    { "IN", "GT_U32", "tpidEntryIndex", params.bind},
                })
            command_data:handleCpssErrorDevPort(result, "cpssDxChTunnelStartHeaderTpidSelectSet(), index: ")
        end, params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

CLI_addCommand({"interface", "interface_eport"}, "tpid tunnel-start", {
  func   = tpid_func_tunnel_start_bind,
  help   = "Binds tunnel-start on port to tpid index",
  params = {
      { type = "named",
          { format = "bind %tpid_index", name="bind", help="TPID table entry index" },
          mandatory = { "bind"}
      }
  }
})

CLI_addHelp("interface", "tpid", "Bind port to tpid")

