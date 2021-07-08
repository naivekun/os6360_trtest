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
--*       show tpid info
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local function show_tpid(params)
    local result, values, tag_type, is_tag_type_supported;
    local devlist, i, devNum, line;
    local dir_idx, dir_val, dir_name;

    dir_val = {"CPSS_DIRECTION_INGRESS_E", "CPSS_DIRECTION_EGRESS_E"};

    if params.all_device ~= "all" then
        devlist = {params.all_device}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    if (#devlist == 0) then
        print("Empty device list\n");
        return false;
    end

    --[ Check that device supports cpssDxChBrgVlanTpidTagTypeGet --]

    result, tag_type = myGenWrapper(
            "cpssDxChBrgVlanTpidTagTypeGet", {
                { "IN", "GT_U8", "devNum", devlist[1]},
                { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_INGRESS_E"},
                { "IN", "GT_U32", "index", 0},
                { "OUT", "CPSS_BRG_TPID_SIZE_TYPE_ENT", "typePtr"}
            })
    is_tag_type_supported = (result == 0);

    dir_val  = {"CPSS_DIRECTION_INGRESS_E", "CPSS_DIRECTION_EGRESS_E"};
    dir_name = {"ingress", "egress "};


    line = "Device  Direction  Entry Ind   etherType"
    if is_tag_type_supported then line = line .. " Tag Type" end;
    print(line);
    line = "------  ---------  ----------  ---------";
    if is_tag_type_supported then line = line .. " --------" end;
    print(line);

    for i, devNum in pairs(devlist) do
       for dir_idx = 1, 2 do
          for i = 0, 7 do
              result, values = myGenWrapper(
                      "cpssDxChBrgVlanTpidEntryGet", {
                          { "IN", "GT_U8", "devNum", devNum},
                          { "IN", "CPSS_DIRECTION_ENT", "direction", dir_val[dir_idx]},
                          { "IN", "GT_U32", "entryIndex", i},
                          { "OUT", "GT_U32", "etherType"}
                      })
              if result ~= 0 then
                  print("Error while executing cpssDxChBrgVlanTpidEntryGet.")
                  return false
              end

              line = "  " .. to_string(devNum)
                  .. "\t" .. dir_name[dir_idx] .. " \t" .. to_string(i)
                  .. "\t" .. string.format("0x%04X",values["etherType"]);

              if is_tag_type_supported then
                 result, tag_type = myGenWrapper(
                         "cpssDxChBrgVlanTpidTagTypeGet", {
                             { "IN", "GT_U8", "devNum", devNum},
                             { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_INGRESS_E"},
                             { "IN", "GT_U32", "index", i},
                             { "OUT", "CPSS_BRG_TPID_SIZE_TYPE_ENT", "typePtr"}
                         })
                 if result ~= 0 then
                     print("Error while executing cpssDxChBrgVlanTpidTagTypeGet.")
                     return false
                 end
                 line = line .. "\t"..tag_type["typePtr"];
              end
              print(line);
          end
       end
    end
end

--show tpid device

CLI_addCommand("exec", "show tpid", {
  func   = show_tpid,
  help   = "Show tpid device",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})


local function show_tpid_ingress(params)
    local profiles
    local eths = {"CPSS_VLAN_ETHERTYPE0_E", "CPSS_VLAN_ETHERTYPE1_E"}
    local devlist
    local result, values

    if params.profile == "all" then
        profiles={0,1,2,3,4,5,6,7}
    else
        profiles={params.profile}
    end

    if params.all_device ~= "all" then
        devlist = {params.all_device}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    print("Device  Profile  Ethernet Mode   Bitmap")
    print("------  -------  -------------  ---------")

    for ekey, eth in pairs(eths) do
        for pkey, pr in pairs(profiles) do
            local i, devNum
            for i, devNum in pairs(devlist) do
                result, values = myGenWrapper(
                        "cpssDxChBrgVlanIngressTpidProfileGet", {
                            { "IN", "GT_U8", "devNum", devNum},
                            { "IN",  "GT_U32", "profile", pr},
                            { "IN",  "CPSS_ETHER_MODE_ENT", "ethMode", eth},
                            { "OUT", "GT_U32", "tpidBmp"}
                        })
                if result ~= 0 then
                    print("Error while executing cpssDxChBrgVlanIngressTpidProfileGet. "..to_string(result))
                    return false
                end
                --local tpids
                local s = ""
                for i = 0,7 do
                    if bit_and(values.tpidBmp, bit_shl(1,i)) ~= 0 then
                        if s ~= "" then
                            s = s .. ","
                        end
                        s = s .. tostring(i)
                    end
                end
                if s == "" then
                    s = "none"
                end

                local ethval
                if eth == "CPSS_VLAN_ETHERTYPE1_E" then
                    ethval = "ethertype1"
                else
                    ethval = "ethertype0"
                end

                print(" "..to_string(devNum).."\t   "..to_string(pr).."\t  "..ethval.."\t"..s)
            end
        end
    end
    return true
end


--show tpid ingress profile
CLI_addCommand("exec", "show tpid ingress", {
  func   = show_tpid_ingress,
  help   = "Show tpid device",
  params = {
      { type = "named",
          { format= "profile %profile_id", name="profile", help="TPID profile." },
          "#all_device",
        mandatory = { "profile", "all_device" }
      }
  }
})


-- show interface tpid {etherent | all}
-- this command should present ingress and egress information of the port.
-- using cpss APIs cpssDxChBrgVlanPortEgressTpidGet and cpssDxChBrgVlanPortIngressTpidProfileGet

local function show_interface_tpid(params)
    local command_data = Command_Data()
    local eths = {"CPSS_VLAN_ETHERTYPE0_E", "CPSS_VLAN_ETHERTYPE1_E"}

    command_data:initInterfaceDevPortRange()

    if command_data:getDevicePortRange() == nil then
        print("There are no initialized interfaces.")
        return false
    end

    print("Device  PortNum  Ethernet Mode  EntryIndx Profile IsDefaultProfile")
    print("------  -------  -------------  --------- ------- ----------------")

    if params.all ~= "all" then
        eths = { params.ethernet }
    end

    for ekey, eth in pairs(eths) do
        command_data:iterateOverPorts(
            function(command_data, devNum, portNum, params)
                local result, i, profile
                local values1, values2, values3
                local def_prof1 = true
                local def_prof2 = false

                result, values1 = myGenWrapper(
                    "cpssDxChBrgVlanPortIngressTpidProfileGet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN",  "GT_PORT_NUM", "portNum", portNum},
                        { "IN",  "CPSS_ETHER_MODE_ENT", "ethMode", eth},
                        { "IN",  "GT_BOOL", "isDefaultProfile", def_prof1},
                        { "OUT", "GT_U32", "profile"}
                    })
                command_data:handleCpssErrorDevPort(result, "cpssDxChBrgVlanPortIngressTpidProfileGet()")
                result, values2 = myGenWrapper(
                    "cpssDxChBrgVlanPortIngressTpidProfileGet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN",  "GT_PORT_NUM", "portNum", portNum},
                        { "IN",  "CPSS_ETHER_MODE_ENT", "ethMode", eth},
                        { "IN",  "GT_BOOL", "isDefaultProfile", def_prof2},
                        { "OUT", "GT_U32", "profile"}
                    })
                command_data:handleCpssErrorDevPort(result, "cpssDxChBrgVlanPortIngressTpidProfileGet()")

                result, values3 = myGenWrapper(
                    "cpssDxChBrgVlanPortEgressTpidGet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN",  "GT_PORT_NUM", "portNum", portNum},
                        { "IN",  "CPSS_ETHER_MODE_ENT", "ethMode", eth},
                        { "OUT", "GT_U32", "tpidEntryIndex"}
                    })
                command_data:handleCpssErrorDevPort(result, "cpssDxChBrgVlanPortEgressTpidGet()")

                local ethval
                if eth == "CPSS_VLAN_ETHERTYPE1_E" then
                    ethval = "ethertype1"
                else
                    ethval = "ethertype0"
                end

                print(string.format("   %-6s%-9s%-14s%-10s%-9s%-8s",
                    to_string(devNum),
                    to_string(portNum),
                    ethval,
                    to_string(values3.tpidEntryIndex),
                    to_string(values1.profile),
                    to_string(def_prof1)))

                print(string.format("   %-6s%-9s%-14s%-10s%-9s%-8s",
                    to_string(devNum),
                    to_string(portNum),
                    ethval,
                    to_string(values3.tpidEntryIndex),
                    to_string(values2.profile),
                    to_string(def_prof2)))

            end, params)
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

CLI_type_dict["ether_val"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Ingress or egress",
    enum = {
        ["0"] = { value="CPSS_VLAN_ETHERTYPE0_E", help="Ethernet type 0" },
        ["1"] = { value="CPSS_VLAN_ETHERTYPE1_E", help="Ethernet type 1" }
   }
}

CLI_addParamDict("ether_all", {
    {
        format = "all",
        name="all",
        help = "All interfaces"
    },
    {
        format = "ethernet %ether_val",
        name="ethernet",
        help = "Ethernet interface to configure"
    },
    alt          = { ether_all = { "all", "ethernet"}}
})

CLI_addCommand("exec", "show interfaces tpid", {
    func   = function(params)
    return do_command_with_print_nice(
            "show_interface_tpid()",
            show_interface_tpid,
            params)
    end,
    help   = "Show tpid device",
    params = {
        { type = "named",
            "#ether_all",
            mandatory = { "ether_all" }
        }
    }
})
