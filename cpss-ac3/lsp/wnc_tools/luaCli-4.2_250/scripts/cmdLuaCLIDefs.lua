--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cmdLuaCLIDefs.lua
--*
--* DESCRIPTION:
--*       lua sturt-up scripts
--*
--* FILE REVISION NUMBER:
--*       $Revision: 40 $
--*
--********************************************************************************


-- LuaCLI globals

-- list of callbacks to update device depended types
typeCallBacksList = {}

--[[
-- Configure garbage collector
collectgarbage("setpause",10)
collectgarbage("setstepmul",100)
collectgarbage("restart")
]]--


-- Globals for family detection
dxCh_family = false
px_family = false

-- DX by default
-- switched by setModeDX|setModePX
cmdLuaCLI_registerCfunction("wrlCpssLuaModeNumGet")
lua_mode = wrlCpssLuaModeNumGet()


if fs.exists("dxCh/initEnvironment.lua")  then
    dxCh_family = true
end

if fs.exists("px/initEnvironment.lua")  then
    px_family = true
end

if ((dxCh_family == true) and (px_family == true)) then
  if lua_mode == 0 then
    dxCh_family   = true
    px_family     = false
    print("\nDX Mode\n")
  else
    dxCh_family   = false
    px_family     = true
    print("\nPX Mode\n")
  end
end

-- print("\nMode = " .. tostring(lua_mode) .. "\ndxCh_family = " .. tostring(dxCh_family) .. "\npx_family = " .. tostring(px_family))

--includes
require("common/generic/common")
require("common/generic/string")
require("common/misc/trace")
require("CLI")

function require_safe(name)
    local status, str = pcall(require, name)
    if not status then
        print("Failed to load module "..name..": "..str)
    end
    return status
end

function require_safe_dx(name)
    if dxCh_family == false then
        return 0
    end
    local status, str = pcall(require, "dxCh/"..name)
    if not status then
        print("Failed to load module dxCh/"..name..": "..str)
    end
    return status
end

function require_safe_px(name)
    if px_family == false then
        return 0
    end
    local status, str = pcall(require, "px/"..name)
    if not status then
        print("Failed to load module px/"..name..": "..str)
    end
    return status
end

cmdLuaCLI_registerCfunction("cpssGlobalMtxLock")
cmdLuaCLI_registerCfunction("cpssGlobalMtxUnlock")
if cpssGlobalMtxLock ~= nil and cpssGlobalMtxUnlock ~= nil then
    CLI_execution_lock = cpssGlobalMtxLock
    CLI_execution_unlock = cpssGlobalMtxUnlock
end

-- DBG:
--[[
function splitline(s)
    local res = {}
    local idx = 1
    local w
    for w in string.gmatch(s, "[^%s%c]+") do
        res[idx] = w
        idx = idx + 1
    end
    return res
end
function cmdLuaCLI_registerCfunction()
end
function cmdLuaCLI_callCfunction()
end
function cpssGenWrapper()
    return 1,{}
end
]]--


-- ************************************************************************
---
--  cli_C_functions_registered
--        @description  Check that all listed functions are registered
--              if function is not registered yet then try to register it
--
--
--        @return       true if all functions are registered successfully
--
function cli_C_functions_registered(...)
    local print_saved = print
    print = function()
    end
    local ret = true
    local arr = { ... }
    local i
    for i=1,#arr do
        if type(arr[i]) == "string" then
            if type(_G[arr[i]]) == "nil" then
                cmdLuaCLI_registerCfunction(arr[i])
            end
            if type(_G[arr[i]]) ~= "function" then
                ret = false
            end
        end
    end

    print = print_saved
    return ret
end


require_safe("common/generic/command_patterns")
require_safe("common/generic/return_codes")
require_safe_dx("generic/enums")
require_safe_dx("generic/const")
require_safe("common/generic/const")
require("common/misc/iterators")
require_safe("common/misc/genwrapper_API")
require_safe("common/misc/myGenWrapper")
require("common/misc/wraplCpssCommon")
require_safe("common/misc/printing_with_pause")
require_safe("common/generic/system_capabilities")
require_safe("common/generic/gen_utils")
require_safe_dx("generic/pbr_utils")
require_safe_dx("generic/shared_resources")
require_safe_dx("generic/l2mll_util")
require_safe_dx("generic/policer_util")
require_safe_dx("generic/serdes_utils")
require_safe_px("generic/px_serdes_utils")
require_safe_dx("generic/tcam_utils")
require_safe_dx("generic/arp_utils")

require_safe("common/cli_types/all")
require_safe_dx("cli_types/types_serdes")

--require tftp file first to ensure existing of tftp (also when this file is damaged)



-- Different states
CLI_prompts["debug"] = "(debug)#"
CLI_prompts["config"] = "(config)#"
CLI_prompts["tti-rule"] = "(config-tti-rule)#"
CLI_prompts["running-config"] = "(running-config)#"
CLI_prompts["traffic"] = "(traffic)#"

-- commands to sitch between states
CLI_addCommand(nil, "exit", {
    func=CLI_change_mode_pop,
    help="Switch to the parent mode"
})
CLI_addCommand(nil, "end", {
    func=CLI_change_mode_pop_all,
    help="Switch to the root mode"
})
CLI_addCommand("exec", "debug-mode", {
    func=CLI_change_mode_push,
    constFuncArgs={"debug"},
    help="Exit from the EXEC to debug mode"
})
CLI_addCommand("exec", "configure", {
    func=CLI_change_mode_push,
    constFuncArgs={"config"},
    help="Enter configuration mode"
})
CLI_addCommand("exec", "traffic", {
    func=CLI_change_mode_push,
    constFuncArgs={"traffic"},
    help="Switch to traffic mode"
})
CLI_addCommand("config", "running-config", {
    func=CLI_change_mode_push,
    constFuncArgs={"running-config"},
    help="Switch to running configuration editing mode"
})
CLI_addCommand("exec", "micro-init", {
    func=CLI_change_mode_push,
    constFuncArgs={"micro-init"},
    help="Enter micro-init mode"
})
CLI_addCommand(nil, "echo", {
    extraParametersAllowed=true,
    help="echo words from command line",
    func=function(params, ...)
        print(table.concat({...}," "))
    end
})
CLI_addCommand(nil, "sleep", {
    extraParametersAllowed=true,
    help="sleep msec",
    func=function(params)
        local msec = tonumber(params["msec"])
        if(msec == nill) then
            print("inputed msec value is not number")
        else
            cpssGenWrapper("osTimerWkAfter",{{"IN","GT_U32","mils",msec}})
        end
    end,
    params =
    {
        { type="values",
           { format = "%string", name = "msec", help = "sleep msec" },
        }
    }
})

require_safe("common/debug/tftp")

--access-list mode
CLI_prompts["config-acl"] = "(config-acl)#"
-- ************************************************************************
---
--  access_list_config
--        @description  Change the context to access list context
--
--        @param params - params["pclID"]   - The pcl id
--                        params["devID"]   - The device id
--                        params["direction"] - Ingress/Egress (optional)
--
--        @return       true on success
--
function access_list_config(params)
    setGlobal("g_pclID", params["pclID"])
    setGlobal("g_pclIDDev", params["devID"])
    local direction = params["direction"]
    if (direction == nil) then
        direction = "ingress"
    end
    setGlobal("g_pclDirection", direction)
    CLI_change_mode_push("config-acl")
    return true
end

CLI_addHelp("exec",                 "no",   "Negate command")

CLI_addHelp("config",               "no",   "Negate command")
CLI_addHelp("debug",                "no",   "Negate command")
CLI_addHelp("interface",            "no",   "Negate command")
CLI_addHelp("vlan_configuration",   "no",   "Negate command")

CLI_addHelp("config", "access-list", "Switch to access list editing mode")

CLI_addCommand("config", "access-list", {
    func=access_list_config,
    help="The command defines as IP standard access list",
    params={
    { type="named",
      { format="device %devID_all", name="devID" ,help="The device number"},
      { format="ingress", name="direction", help="direction: Ingress (optional)" },
      { format="egress", name="direction", help="direction: Egress (optional)" },
      { format="pcl-ID %pclID", name="pclID", help="The PCL ID number (0-1023)" },
        requirements = {["pclID"] = { "devID" }, ["direction"] = { "devID" }},
        mandatory={"pclID"}}
    }
})

--packet-type-key mode
CLI_prompts["packet-type-key"] = "(packet-type-key)#"
-- ************************************************************************
---
--  packet_type_key_config
--        @description  Change the context to packet-type-key context
--
--        @param params -  params["devID"]   - The device id
--                         params["direction"] - Ingress/Egress (optional)
--                         params["packetType"] - Index of the packet type key in the table
--                         (APPLICABLE RANGES: 0..31)
--
--        @return       true on success
--
function packet_type_key_config(params)
    setGlobal("g_pipeIDDev", params["devID"])
    local direction = params["direction"]
    if (direction == nil) then
        direction = "ingress"
    end
    setGlobal("g_pipeDirection", direction)
    setGlobal("g_pipePacketType", params["packet_type"])
    CLI_change_mode_push("packet-type-key")
    return true
end

CLI_addHelp("config", "packet-type-key", "Switch to packet type key editing mode")

CLI_addCommand("config", "packet-type-key", {
    func=packet_type_key_config,
    help="The command defines configurations per packet type",
    params={
        { type="named",
          { format="device %devID_all", name="devID" ,help="The device number"},
          { format="ingress", name="direction", help="direction: Ingress" },
          { format="egress", name="direction", help="direction: Egress" },
          { format="packet-type %show_packetType", name="packet_type", help="Packet Type Index" },
            requirements = {["direction"] = { "devID" }, ["packet_type"] = { "direction" }},
            mandatory = {"packet_type"}
        }
    }
})

-- config-if mode
CLI_prompts["interface"] = "(config-if)#"

-- config-if-eport mode
CLI_prompts["interface_eport"] = "(config-if-eport)#"


-- ************************************************************************
---
--  interface_range_ethernet
--        @description  Change the context to interface context
--
--        @param params         - The parameters
--
--        @return       true on success, otherwise false and error message
--
function interface_range_ethernet(params)
    setGlobal("ifType", "ethernet")
    setGlobal("ifRange", params["port-range"])
    CLI_change_mode_push("interface")
    return true
end
CLI_addHelp("config", "interface", "Select an interface to configure")
CLI_addHelp("config", "no interface", "Select an interface to configure")
CLI_addHelp("interface", "interface", "Interface configuring")
CLI_addHelp("interface", "no interface", "Interface configuring")




--------------------------------------------------------------------------------
-- mode registration: port-channel
--------------------------------------------------------------------------------
CLI_prompts["port-channel"] = "(port-channel)#"

-- vlan_config mode
CLI_prompts["vlan_configuration"] = "(config-vlan)#"


--------------------------------------------------------------------------------
-- parameter dictionary registration: dev
--------------------------------------------------------------------------------
CLI_addParamDict("dev", {
    { format = "dev %devID",    name = "dev",
                            help = "Specific device"                          }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: all_device
--------------------------------------------------------------------------------
CLI_addParamDict("all_device", {
    { format = "device %devID_all",
                                name = "all_device",
                            help = "Apply to the given deviceID or to all " ..
                                   "the devices in the system"                }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: single_interface
--------------------------------------------------------------------------------
CLI_addParamDict("single_interface", {
    { format = "interface %port-range",
                                name = "ethernet",
                            help = "Ethernet interface to configure"          }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: interface_mode
--------------------------------------------------------------------------------
CLI_addParamDict("interface_mode", {
    { format="mode %port_interface_mode",
                                name="port_interface_mode",
                            help = "interface mode"                            }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: autodetect
--------------------------------------------------------------------------------
CLI_addParamDict("autodetect", {
    { format = "%autodetect",           name = "autodetect",
                            help = "Auto-detect port's actual mode and speed" }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: port_channel
--------------------------------------------------------------------------------
CLI_addParamDict("port_channel", {
    { format = "port-channel %trunkID", name = "port_channel",
                            help = "Port-Channel interface"                   }
})


--------------------------------------------------------------------------------
-- parameter dictionary registration: port_channel_or_nil
--------------------------------------------------------------------------------
CLI_addParamDict("port_channel_or_nil", {
    { format = "%trunkID", name = "port_channel",
                            help = "Port-Channel interface"                   }
})


--------------------------------------------------------------------------------
-- parameter dictionary registration: vlan
--------------------------------------------------------------------------------
CLI_addParamDict("vlan", {
    { format = "vlan %vlanId",  help = "Specific IEEE 802.1Q VLAN ID"         }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: vid
--------------------------------------------------------------------------------
CLI_addParamDict("vid", {
    { format = "vid %vlanId",   help = "Specific IEEE 802.1Q VLAN ID"         }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: vid_unchecked
--------------------------------------------------------------------------------
CLI_addParamDict("vid_unchecked", {
    { format = "vid %unchecked-vlan",   help = "Specific IEEE 802.1Q VLAN ID" }
})
-- used in 'ip next-hop' command, shows up in autocomplete only for certain packet commands
CLI_addParamDict("vid_unchecked_cmd_def", {
    { format = "vid %unchecked-vlan_cmd_def",   help = "Specific IEEE 802.1Q VLAN ID" }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: tag_vlan
--------------------------------------------------------------------------------
CLI_addParamDict("tag_vlan", {
    { format = "tag %vlanId",           name="vlan",
                                help = "Specific IEEE 802.1Q VLAN ID"         }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: configurable_vlan
--------------------------------------------------------------------------------
CLI_addParamDict("configurable_vlan", {
    { format = "vid %configurable-vlan",
                                        name="configurable_vlan",
                            help = "Specific IEEE 802.1Q VLAN ID"             }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: deleted_vlan
--------------------------------------------------------------------------------
CLI_addParamDict("deleted_vlan", {
    { format = "vid %vlanId",           name="deleted_vlan",
                            help = "Specific IEEE 802.1Q VLAN ID"             }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: vlan_range
--------------------------------------------------------------------------------
CLI_addParamDict("vlan_range", {
    { format = "vlan %vlan-range",      name="vlan_range",
                            help = "IEEE 802.1Q Vlans"                        }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: next_hop_vid
--------------------------------------------------------------------------------
CLI_addParamDict("next_hop_vid", {
    { format = "next-hop-vid %vlanId",      name="vlan",
                            help = "Next hop vlan id"                         }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: mac
--------------------------------------------------------------------------------
CLI_addParamDict("mac", {
    { format = "mac %mac-address",          name = "mac-address",
                            help = "Specific MAC address"                     }
})
-- used in 'ip next-hop' command, shows up in autocomplete only for certain packet commands
CLI_addParamDict("mac_cmd_def", {
    { format = "mac %mac-address_cmd_def",          name = "mac-address",
                            help = "Specific MAC address"                     }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: mac-address
--------------------------------------------------------------------------------
CLI_addParamDict("mac-address", {
    { format = "address %mac-address",  name="mac-address",
                            help = "Specific MAC address"                     }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: next_hop_interface
--------------------------------------------------------------------------------
CLI_addParamDict("next_hop_interface", {
    { format = "next-hop-interface %dev_port",
                                            name="next_hop_interface",
                            help = "Next hop interface device/port"          }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: router_arp
--------------------------------------------------------------------------------
CLI_addParamDict("router_arp", {
    { format = "arp %arp_entry_index",      name="router_arp",
                            help = "Apply to specific Arp Address index"     }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: router-arp-index
--------------------------------------------------------------------------------
CLI_addParamDict("router_arp_index", {
    { format = "arp-index %arp_entry_index",
                                            name="router_arp_index",
                            help = "Apply to specific Arp Address index"     }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: ipv4_dip
--------------------------------------------------------------------------------
CLI_addParamDict("ipv4_dip", {
    { format = "group %ipv4_mc",              name="ipv4_dip",
                            help = "Ipv4 multicast group address"            }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: ipv6_dip
--------------------------------------------------------------------------------
CLI_addParamDict("ipv6_dip", {
    { format = "group %ipv6_mc",              name="ipv6_dip",
                            help = "Ipv6 multicast group address"             }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: multicast_mac_address
--------------------------------------------------------------------------------
CLI_addParamDict("multicast_mac_address", {
    { format = "address %mac-address-mc",     name="multicast_mac_address",
                            help = "Specific multicast mac address"           }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: multicast_ip_address
--------------------------------------------------------------------------------
CLI_addParamDict("multicast_ip_address", {
    { format = "address %multicast_ip",       name="multicast_ip_address",
                            help = "Specific multicast ip address"            }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: multicast_address
--------------------------------------------------------------------------------
CLI_addParamDict("multicast_address", {
    { format = "address %multicast",          name="multicast_address",
                            help = "Specific multicast address"                   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: source_ip
--------------------------------------------------------------------------------
CLI_addParamDict("source_ip", {
    { format = "source %ipv4_ipv6",           name="source_ip",
                            help = "Source IP address"                        }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: default_gateway
--------------------------------------------------------------------------------
CLI_addParamDict("default_gateway", {
    { format = "default-gateway %ipv4_gw",
                                              name="default_gateway",
                            help = "Default gateway IP address"               }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: ipv4_ipv6_version
--------------------------------------------------------------------------------
CLI_addParamDict("ipv4_ipv6_version", {
    { format = "ip-version %ipv4_ipv6",     name="ipv4_ipv6_version",
                            help = "Apply to given ip-address"               }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: next_hop
--------------------------------------------------------------------------------
CLI_addParamDict("next_hop", {
    { format = "nexthop %next-hop-id",      name="next_hop",
                            help = "Apply to given next-hop id"               }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: packet_cmd
--------------------------------------------------------------------------------
CLI_addParamDict("cmd", {
    { format = "cmd %packet_cmd",      name="packet_cmd",
                            help = "Packet command"               }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: priority
--------------------------------------------------------------------------------
CLI_addParamDict("priority", {
    { format = "priority %priority",        name="priority",
                            help = "802.1Q Priority"                          }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: priority_enable
--------------------------------------------------------------------------------
CLI_addParamDict("priority_enable", {
    { format = "priority %priority enable", name="priority",
                            help = "802.1Q Priority",
                            help_enable = "Priority enabling"                 }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: feedback_weight
--------------------------------------------------------------------------------
CLI_addParamDict("feedback_weight", {
    { format = "feedback-weight %feedback_weight",
                                            name="feedback_weight",
                            help = "Feedback weight specification"            }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: pfc
--------------------------------------------------------------------------------
CLI_addParamDict("pfc", {
    { format = "pfc %set_point_bytes",      name="pfc",
                            help = "Specify the set point for PFC queues"     }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: non-pfc
--------------------------------------------------------------------------------
CLI_addParamDict("non-pfc", {
    { format = "non-pfc %set_point_bytes",  name="non-pfc",
                            help = "Specify the set point for non-PFC queues" }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: pfc_profile
--------------------------------------------------------------------------------
CLI_addParamDict("pfc_profile", {
    { format = "profile %pfc_profile",      name = "pfc_profile",
                            help = "The PFC profile number which is being " ..
                                   "configured"                               }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: packet_length
--------------------------------------------------------------------------------
CLI_addParamDict("packet_length", {
    { format = "packet-length %packet_length_bytes",
                                            name="packet_length",
                            help = "Specifies the default packet length in " ..
                                   "bytes"                                    }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: min_sample_base
--------------------------------------------------------------------------------
CLI_addParamDict("min_sample_base", {
    { format = "min-sample-base %min_sample_base",
                                            name="min_sample_base",
                            help = "Specify the minimal sample base in " ..
                                   "bytes; the value should be a multiple " ..
                                   "of 16"                                    }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: xoff_bit11
--------------------------------------------------------------------------------
CLI_addParamDict("xoff_bit11", {
    { format = "xoff %xoff_bit11",          name = "xoff",
                            help = "Xoff threshold, in decimal notation"      }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: xon_bit11
--------------------------------------------------------------------------------
CLI_addParamDict("xon_bit11", {
    { format = "xon %xon_bit11",            name = "xon",
                            help = "Xon threshold, in decimal notation"      }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: drop_threshold_bit11
--------------------------------------------------------------------------------
CLI_addParamDict("drop_threshold_bit11", {
    { format = "drop %drop_threshold_bit11",
                                            name = "drop_threshold",
                            help = "Drop threshold, in decimal notation"      }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: skipped
--------------------------------------------------------------------------------
CLI_addParamDict("skipped", {
    { format = "skipped",
                               help = "Display only aged-out addresses"       }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: interface
--------------------------------------------------------------------------------
CLI_addParamDict("interface", {
    { format = "ethernet %port-range",
                            help = "Ethernet interface to configure"          },
    { format = "port-channel %trunkID",
                            help = "Valid Port-Channel interface"                 },
    alt      = { interface              = { "ethernet", "port-channel" }      }
})

-------------------------------------------------------------------------------
-- parameter dictionary registration: interface_single_port_port_channel
-------------------------------------------------------------------------------
CLI_addParamDict("interface_single_port_port_channel", {
    { format = "ethernet %portNum",
                            help = "Specific ethernet port"                   },
    { format = "port-channel %trunkID",
                            help = "Valid Port-Channel interface"                 },
    alt      = { interface_single_port_port_channel =
                                          { "ethernet", "port-channel" }      }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: interface_port_channel
--------------------------------------------------------------------------------
CLI_addParamDict("interface_port_channel", {
  { format = "ethernet %dev_port",    help = "Specific ethernet dev/port" },
  { format = "port-channel %trunkID", help = "Valid Port-Channel interface"},
  { format = "eport %dev_ePort",      help = "Specific dev/ePort"},
  alt = { interface_port_channel = { "ethernet", "port-channel", "eport" }}
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: interface_port_ethernet
--------------------------------------------------------------------------------
CLI_addParamDict("interface_port_ethernet", {
  { format = "ethernet %dev_port",    help = "Specific ethernet dev/port" },
  alt = { interface_port_ethernet = { "ethernet" }}
})

-- used in 'ip next-hop' command, shows up in autocomplete only for certain packet commands
CLI_addParamDict("interface_port_channel_cmd_def", {
  { format = "ethernet %dev_port_cmd_def",    help = "Specific ethernet dev/port" },
  { format = "port-channel %trunkID_cmd_def", help = "Valid Port-Channel interface"},
  { format = "eport %dev_ePort_cmd_def",      help = "Specific dev/ePort"},
  alt = { interface_port_channel_cmd_def = { "ethernet", "port-channel", "eport" }}
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: interface_port_channel_remote
--------------------------------------------------------------------------------
CLI_addParamDict("interface_port_channel_remote", {
  { format = "ethernet %remote_dev_port",    help = "Specific ethernet dev/port" },
  { format = "port-channel %trunkID", help = "Valid Port-Channel interface"},
  { format = "eport %dev_ePort",      help = "Specific dev/ePort"},
  alt = { interface_port_channel_remote = { "ethernet", "port-channel", "eport" }}
})
--------------------------------------------------------------------------------
-- parameter dictionary registration: add_remove
--------------------------------------------------------------------------------
CLI_addParamDict("add_remove", {
    { format = "add",       help = "Adding of item(s)"                        },
    { format = "remove",    help = "Removing of item(s)"                      }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: dynamic_all
--------------------------------------------------------------------------------
CLI_addParamDict("dynamic_all", {
    { format = "dynamic",       name = "dynamic_all",
                            help = "dynamic addressed"                        },
    { format = "all",           name = "dynamic_all",
                            help = "static and dynamic addressed"             }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: static_dynamic
--------------------------------------------------------------------------------
CLI_addParamDict("static_dynamic", {
    { format = "static",        name = "static_dynamic",
                            help = "static addresses"                         },
    { format = "dynamic",       name = "static_dynamic",
                            help = "dynamic addresses"                        }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: regular_extended
--------------------------------------------------------------------------------
CLI_addParamDict("regular_extended", {
    { format = "regular",
                            help = "Regular DSA tag (1 word DSA tag)"         },
    { format = "extended",
                            help = "Extended DSA tag (2 word DSA tag)"        },
    { format = "3_words",
                            help = "3 words DSA tag"                          },
    { format = "4_words",
                            help = "4 words DSA tag"                          },
    { format = "network",
                            help = "Regular network port (not using DSA tag)" },
    alt      = { regular_extended   = { "regular", "extended", "3_words",
                                        "4_words",  "network"}}
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: forward_drop
--------------------------------------------------------------------------------
CLI_addParamDict("forward_drop", {
    { format = "forward",
                            help = "Egress port to forward unregistered " ..
                                   "multicast frames"                         },
    { format = "drop",
                            help = "Egress port to filter unregistered " ..
                                   "multicast frames. See usage guidelines " ..
                                   "for the case that the port is a router " ..
                                   "port"                                     },
    alt      = { forward_drop   = { "forward", "drop" }                      }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: ip_mac
--------------------------------------------------------------------------------
CLI_addParamDict("ip_mac", {
    { format = "format ip",     name = "ip",
                            help = "Apply to ip address"                     },
    { format = "format mac",    name = "mac",
                            help = "Apply to mac address"                    },
    alt      = { ip_mac   = { "ip", "mac" }                                  }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: unicast_multicast_type
--------------------------------------------------------------------------------
CLI_addParamDict("unicast_multicast_type", {
    { format = "type unicast",  name = "unicast",
                            help = "Apply to unicast entries"                },
    { format = "type multicast",
                                name = "multicast",
                            help = "Apply to multicast entries"              },
    alt      = { unicast_multicast   = { "unicast", "multicast" }            }
})
--------------------------------------------------------------------------------
-- parameter dictionary registration: increment_decrement_ttl
--------------------------------------------------------------------------------
CLI_addParamDict("increment_decrement_ttl", {
    { format = "decrement-ttl false",
                                name = "increment_ttl",
                            help = "Time decrementing to live field of " ..
                                   "the packet"                              },
    { format = "decrement-ttl true",
                                name = "decrement_ttl",
                            help = "Time incrementing to live field of " ..
                                   "the packet"                              },
    alt      = { increment_decrement_ttl = { "increment_ttl",
                                                           "decrement_ttl" } }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: vlan_range_all
--------------------------------------------------------------------------------
CLI_addParamDict("vlan_range_all", {
    { format = "vid %vlan-range",
                                name="vlan_range",
                            help = "IEEE 802.1Q Vlans"                        },
    { format = "all",           name="all_vlans",
                            help = "Configure all vlans"                      },
    alt      = { vlan_range_all = { "vlan_range", "all_vlans" }               }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: configurable_vlan_range_all
--------------------------------------------------------------------------------
CLI_addParamDict("configurable_vlan_range_all", {
    { format = "vid %configurable-vlan-range",
                                name="vlan_range",
                            help = "IEEE 802.1Q Vlans"                        },
    { format = "all",           name="all_vlans",
                            help = "Configure all vlans"                      },
    alt      = { configurable_vlan_range_all = { "vlan_range", "all_vlans" }  }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: static_dynamic_all
--------------------------------------------------------------------------------
CLI_addParamDict("static_dynamic_all", {
    { format = "static",        name = "static_dynamic_all",
                            help = "static addresses"                         },
    { format = "dynamic",       name = "static_dynamic_all",
                            help = "dynamic addresses"                        },
    { format = "all",           name = "static_dynamic_all",
                            help = "static and dynamic addresses"             }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: ethernet_devId
--------------------------------------------------------------------------------
CLI_addParamDict("ethernet_devId", {
    { format = "ethernet %port-range",
                            help = "Ethernet interface to configure"          },
    { format = "%devID",                    name="devId",
                            help = "Specific device"                          },
    requirements = { devId          = { "ethernet"                        }   },
    alt          = { ethernet_devId = { "ethernet"                        }   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: all_interfaces
--------------------------------------------------------------------------------
CLI_addParamDict("all_interfaces", {
    { format = "all",   help = "Apply to all ports of specific device "..
                               "or all available devices"                     },
    { format = "ethernet %port-range",
                        help = "Ethernet interface to configure"              },
    { format = "port-channel %trunkID",
                        help = "Valid Port-Channel interface"                 },
    { format = "%devID",                    name="devID",
                            help = "Apply to all ports of all available " ..
                                   "devices"                                  },
    requirements = { devID          = { "all"                             }   },
    alt          = { all_interfaces = { "all", "ethernet", "port-channel" }   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: all_interfaces_ver1
--------------------------------------------------------------------------------
-- The <all_interfaces_ver1> is fixing next bug in <all_interfaces>
-- The <all_interfaces> not allowed any parameter to come after it.
-- ===========
-- next is difference in behaviour of the 2 types :
-- using <all_interfaces> for all ports of a specific device (0) was done like:
-- Console# show pip drop-port all 0
-- using <all_interfaces_ver1> would look like:
-- Console# show pip drop-port device 0
--
CLI_addParamDict("all_interfaces_ver1", {
    { format = "all",   help = "Apply to all ports all available devices"     },
    { format = "ethernet %port-range",
                        help = "Ethernet interface to configure"              },
    { format = "port-channel %trunkID",
                        help = "Valid Port-Channel interface"                 },
    { format = "device %devID_all", name="devID",
                         help = "Apply to all ports in the device "           },
    alt          = { all_interfaces = { "all", "ethernet", "port-channel" ,"devID"}   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: all_interfaces
--------------------------------------------------------------------------------
CLI_addParamDict("all_Eth", {
    { format = "all",   help = "Apply to all ports of specific device "..
                               "or all available devices"                     },
    { format = "ethernet %port-range",
                        help = "Ethernet interface to configure"              },
    { format = "%devID",                    name="devID",
                            help = "Apply to all ports of all available " ..
                                   "devices"                                  },
    requirements = { devID          = { "all"                             }   },
    alt          = { all_Eth = { "all", "ethernet" }   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: tagged_untagged
--------------------------------------------------------------------------------
CLI_addParamDict("tagged_untagged", {
    { format = "tagged",
                            help = "tagged"             },
    { format = "untagged",
                            help = "untagged"           },
    alt          = { tagged_untagged = { "tagged", "untagged" }               }
})

-------------------------------------------------------------------------------
-- parameter dictionary registration: rx_tx_both
--------------------------------------------------------------------------------
CLI_addParamDict("rx_tx_both", {
    { format = "rx",   help = "Specific rx"},
    { format = "tx",   help = "Specific tx"},
    { format = "both", help = "Specific both Rx and Tx"},
    alt = { rx_tx_both = { "rx", "tx", "both" }}
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: loopback
--------------------------------------------------------------------------------

CLI_addParamDict("loopback", {
    { format = "internal",
                            help = "Internal loopback"               },
    { format = "serdes %LoopbackSerdeType",
                            help = "serial deserial loopback"                 },
    alt      = { loopback_mode = { "internal", "serdes" }      }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: etOrPrtChnl (ethernet or port-channel
--------------------------------------------------------------------------------
CLI_addParamDict("etOrPrtChnl", {

    { format = "ethernet %dev_port",
                        help = "Ethernet interface to configure"              },
    { format = "port-channel %trunkID",
                        help = "Valid Port-Channel interface"                 },
    alt          = { etOrPrtChnl = {  "ethernet", "port-channel" }   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: payloadType (payload-random or payload-cyclic {pattern- 8 bytes})
--------------------------------------------------------------------------------
CLI_addParamDict("payloadType", {

    { format = "payload-random",
                        help = "Configure the payload to be random"              },
    { format = "payload-cyclic %cyclicPatternArr",
                        help = "Configure the payload to be cyclic"                 },
    alt          = { payload = {  "payload-random", "payload-cyclic" }   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: trafficGenLengthType (length-random | length %length)
--------------------------------------------------------------------------------
CLI_addParamDict("trafficGenLengthType", {

    { format = "length-random",
                        help = "Configure the packet length type to be random"              },
    { format = "length %packetLength",
                        help = "Configure the packet length type to be constant"                 },
    alt          = { packet_length = {  "length-random", "length" }   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: destOrSrcPort
-------------------------------------------------------------------------------

CLI_addParamDict("destOrSrcPort", {

    { format = "destination-port",
                        help = "Configure port as destination port"              },
    { format = "source-port",
                        help = "Configure port as source port"                 },
    alt          = { mode = {  "destination-port", "source-port" }   }
})

--------------------------------------------------------------------------------
-- parameter dictionary registration: dataOrFilename
-------------------------------------------------------------------------------

CLI_addParamDict("dataOrFilename", {

    { format = "file %string",
                        help = "Enter a file name that contains the packet data"              },
    { format = "data %string",
                        help = "Enter packet data as string"                 },
    alt          = { inputString = {  "file", "data" }   }
})


cli_C_functions_registered("wraplLoadFileFromLocalFS")
function loadable_file_check(params)
    if params.ip ~= nil then  --load from tftp file server
        local res,e

        local port = params.port
        if port == nil then
            port=69   --the default port
        end
        res,e = lua_tftp("get",params.ip["string"],port, params.filename, params.filename)
        if res==0 then
            print("File received successfuly\n")
        else
            print("Receiving file failed : " .. e )
            return false
        end
    elseif params.localFs ~= nil then
        if wraplLoadFileFromLocalFS == nil then
            print("Can't to load file from local FS: not supported")
            return false
        end
        if not wraplLoadFileFromLocalFS(params.filename) then
            print("Failed to load file from local FS")
            return false
        end
    end
    local fid, errormsg = fs.open(params.filename, "r")
    if fid == nil then
        print("Can't open "..params.filename..": "..errormsg)
        return false
    end
    fs.close(fid)
    return true
end

CLI_addParamDict("loadableFile", {
    { format="from %ipv4", name="ip" ,help = "The ip to download the file from (optional)"},
    { format="port %l4port",name="port", help = "The file server port" },
    { format="copyFromLocalFs",name="localFs", help="Copy file from local FS (optional)"},
    alt={source={"localFs","ip"}},
    requirements={port={"ip"}}
})


--init the global variables struct for thread communication
--cmdLuaCLI_registerCfunction("luaCLI_setGlobalVal")
--cmdLuaCLI_registerCfunction("luaCLI_getGlobalVal")
cmdLuaCLI_registerCfunction("luaGlobalStore")
cmdLuaCLI_registerCfunction("luaGlobalGet")


-- load history from global data storage
cmdLuaCLI_history=luaGlobalGet("cmdLuaCLI_history")
if cmdLuaCLI_history == nil then cmdLuaCLI_history={} end

CLI_running_Cfg_Tbl=luaGlobalGet("CLI_running_Cfg_Tbl")
if CLI_running_Cfg_Tbl == nil then CLI_running_Cfg_Tbl={} end
--filtered history, only working commands

-- CLI running config saving data.
CLI_first_line_in_running_Cfg_Tbl           =
    luaGlobalGet("CLI_first_line_in_running_Cfg_Tbl")
CLI_number_of_first_line_in_running_Cfg_Tbl    =
    luaGlobalGet("CLI_number_of_first_line_in_running_Cfg_Tbl")
if nil == CLI_number_of_first_line_in_running_Cfg_Tbl  then
    CLI_number_of_first_line_in_running_Cfg_Tbl    = 1
end


-- include implementation
require_safe_dx("exec/show_port_monitor")
require_safe_dx("exec/show_ip_arp")
require_safe_dx("exec/show_ip_arp_inspection")
require_safe_dx("exec/show_ip_next_hop")
require_safe_dx("exec/show_ip_route")
require_safe_dx("exec/show_ipv6_route")
require_safe_dx("exec/show_interfaces_switchport")
require_safe_dx("exec/show_mac_address_table")
require_safe_dx("exec/show_mac_address_table_aging")
require_safe_dx("exec/show_mac_address_table_count")
require_safe_dx("exec/clear_bridge_interface")
require_safe_dx("exec/clear_mac_address_table_dynamic")
require_safe_dx("exec/clear_mac_counters")
require_safe_px("exec/px_clear_mac_counters")
require_safe_dx("exec/show_interfaces_status")
require_safe_px("exec/show_interfaces_status")
require_safe_dx("exec/show_vlan")
require_safe_dx("exec/show_bridge_multicast_mode")
require_safe_dx("exec/show_bridge_multicast_address_table")
require_safe_dx("exec/show_bridge_multicast_unregistered")
require_safe("common/exec/reload")
require_safe_dx("exec/show_capabilities_device")
require_safe_px("exec/px_show_capabilities_device")
require_safe_dx("exec/show_dce_cut_through")
require_safe_px("exec/px_show_dce_cut_through")
require_safe_px("exec/px_show_dce_qcn")
require_safe_dx("exec/show_dce_qcn")
require_safe("common/exec/show_history")
require_safe_dx("exec/show_interfaces_configuration")
require_safe_px("exec/show_interfaces_configuration")
require_safe_dx("exec/show_interfaces_port_channel")
require_safe("common/exec/show_version")
require_safe("common/exec/show_running_config")
require_safe_dx("exec/show_system_policy_tcam_utilization")
require_safe_dx("exec/show_system_router_tti_tcam_utilization")
require_safe_dx("exec/show_bridge_drop_counter")
require_safe_dx("exec/show_nat_drop_counter")
require_safe_dx("exec/show_packet_trace_counters")
require_safe_dx("exec/show_mll_fifo_drop")
require_safe_dx("exec/show_pfc")
require_safe_px("exec/px_show_pfc")
require_safe_dx("exec/show_tail_drop")
require_safe_px("exec/px_show_tail_drop")
require_safe_px("exec/px_show_cos")
require_safe_dx("exec/show_flow_control")
require_safe_px("exec/show_flow_control")
require_safe_dx("exec/show_buffer_management")
require_safe_px("exec/px_show_buffer_management")
require_safe_dx("exec/show_ccfc")

require_safe("common/exec/cpssAPI")
require_safe("common/exec/exec_genwrapper_JSON")

require_safe_dx("exec/show_tpid")
require_safe_dx("exec/show_mac_pdu")
require_safe_dx("exec/show_counters")
require_safe_px("exec/show_pipe_counters")
require_safe_dx("exec/show_tti_rule")
require_safe_dx("exec/show_bpe_802_1_br")
require_safe_dx("exec/show_data_integrity_events")
require_safe_dx("exec/show_data_integrity_errors")
require_safe_dx("exec/show_interfaces_ap_config")
require_safe_dx("exec/show_interfaces_ap_status")
require_safe_dx("exec/show_interfaces_ap_stats")
require_safe_dx("exec/show_interfaces_ap_introp")
require_safe_dx("exec/show_interfaces_serdes_polarity")
require_safe_px("exec/px_show_interfaces_serdes_polarity")
require_safe_dx("exec/show_interfaces_serdes_tx")
require_safe_px("exec/px_show_interfaces_serdes_tx")
require_safe_dx("exec/show_interfaces_serdes_rx")
require_safe_px("exec/px_show_interfaces_serdes_rx")
require_safe_px("exec/px_show_interfaces_serdes_tuning")
require_safe_dx("exec/show_interfaces_serdes_temperature")
require_safe_dx("exec/show_interfaces_serdes_voltage")
require_safe_dx("exec/show_pizza_arbiter")
require_safe_px("exec/PX_show_pizza_arbiter")
require_safe_dx("exec/show_pizza_port")
require_safe_dx("exec/show_eee")
require_safe_dx("exec/show_pip")
require_safe_dx("exec/show_led")
require_safe_px("exec/show_led")
require_safe_dx("exec/show_traffic")
require_safe_px("exec/px_show_traffic")
require_safe_dx("exec/show_interfaces_speed")
require_safe_px("exec/show_interfaces_speed")
require_safe_dx("exec/show_port_resources")
require_safe_px("exec/px_show_port_resources")
require_safe_dx("exec/show_port_smi_mapping")
require_safe_dx("exec/show_running_parallel_tasks")
require_safe_dx("exec/show_bridge_sanity_check")
require_safe_dx("exec/show_storm_control")

require_safe_px("exec/px_show_interfaces_mac_counters")

require_safe_px("exec/px_show_interfaces_ap_status")
require_safe_px("exec/px_show_interfaces_ap_stats")
require_safe_px("exec/px_show_interfaces_ap_introp")
require_safe_px("exec/px_show_interfaces_ap_config")

if (dxCh_family == true) then

  table.insert(typeCallBacksList, function()
    if is_sip_5() then
      require_safe_dx("exec/show_interfaces_mac_counters_bc2")
      require_safe_dx("exec/show_interfaces_serdes_tuning_bobk")
    else
      require_safe_dx("exec/show_interfaces_mac_counters")
      require_safe_dx("exec/show_interfaces_serdes_tuning")
    end
  end
  )

end


--todo add bobk - CPSS_CAELUM_E
if DeviceFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E" then
    -- require_safe_dx("exec/show_tm")
end
require_safe("common/running_config/delete")
require_safe("common/running_config/running_config")
require_safe_dx("configuration/dce_qcn_cp")
require_safe_dx("configuration/dce_qcn_cp_set_point")
require_safe_dx("configuration/back_pressure")
require_safe_px("configuration/back_pressure")
require_safe_dx("configuration/bridge_aging")
require_safe_dx("configuration/bridge_fdb_hash_mode")
require_safe_dx("configuration/channel_group")
require_safe_dx("configuration/EEE")
require_safe_dx("configuration/flow_control")
require_safe_dx("configuration/flow_control_periodic")
require_safe_px("configuration/flow_control_periodic")
require_safe("common/configuration/hostname")
require_safe("common/configuration/interface")
require_safe_dx("configuration/interface_range_ethernet")
require_safe_dx("configuration/interface_range_vlan")
require_safe_dx("configuration/interface_vlan")
require_safe_dx("configuration/jumbo_frame")
require_safe_dx("configuration/mac_address")
require_safe_dx("configuration/ip_routing")
require_safe_dx("configuration/mac_address_table_aging_time")
require_safe_dx("configuration/mac_address_table_aging_mode")
require_safe_dx("configuration/mac_address_table_static")
require_safe_dx("configuration/negotiation")
require_safe_px("configuration/negotiation")
require_safe_dx("configuration/port_mac_learning")
require_safe("common/configuration/set_mac_and_ip")
require_safe_dx("configuration/ip_arp")
require_safe_dx("configuration/ip_arp_inspection")
require_safe_dx("configuration/ip_next_hop")
require_safe_dx("configuration/ip_mc_next_hop")
require_safe_dx("configuration/ip_route")
require_safe_dx("configuration/ipv6_route")
require_safe_dx("configuration/ip_mc_route")
require_safe_dx("configuration/dce_cut_through")
require_safe_px("configuration/px_config_dce_cut_through")
require_safe_px("configuration/px_config_dce_pfc")
require_safe_px("configuration/px_config_dce_qcn")
require_safe_dx("configuration/dce_cut_through_enable")
require_safe_dx("configuration/dce_qcn_cnm")
require_safe_dx("configuration/PCLAddRule")
require_safe_dx("configuration/PCLPortBind")
require_safe_dx("configuration/policy")
require_safe_dx("configuration/port_channel_load_balance")
require_safe_dx("configuration/QOS")
require_safe_px("configuration/px_config_qos")
require_safe_px("configuration/px_config_cos")
require_safe_dx("configuration/diag_temp_sensors_select")
require_safe_dx("configuration/diag_threshold_temp")
require_safe_dx("configuration/bridge_drop_counter")
require_safe_dx("configuration/port_map")
require_safe_dx("configuration/hw_dev")
require_safe_dx("configuration/tpid")
require_safe_dx("configuration/tti_rule")
require_safe_dx("configuration/vlan_tag1_if_zero")
require_safe_dx("configuration/PCLAddRule")
require_safe_dx("configuration/PCLPortBind")
require_safe_dx("configuration/wred_mask")
require_safe_px("configuration/px_config_wred_mask")
require_safe_dx("configuration/mac_pdu")
require_safe_dx("configuration/counters")
require_safe_px("configuration/pipe_counters")
require_safe_dx("configuration/ip_tcp_set_port")
require_safe_dx("configuration/bpe_802_1_br_my_info")
require_safe("common/configuration/vss_internal_db_manager")
require_safe_dx("configuration/vss_config")
require_safe_dx("configuration/vlan_mapping")
require_safe_dx("configuration/vlan_service_id")
require_safe_dx("configuration/nat44")
require_safe_dx("configuration/nat66")
require_safe_dx("configuration/spbm_config")
require_safe_dx("configuration/ip_vrf")
require_safe_dx("configuration/pcl_global_cfg")
require_safe_dx("configuration/ap-engine")
require_safe_dx("configuration/global_pip")
require_safe_dx("configuration/global_led")
require_safe_px("configuration/global_led")
require_safe_dx("configuration/bridge_sanity_check")
require_safe_dx("configuration/micro_init")
require_safe_px("configuration/micro_init")
require_safe_dx("configuration/isolate_lookup_bits")
require_safe_dx("configuration/tail_drop")

require_safe_px("configuration/pipe_ingress")
require_safe_px("configuration/pipe_ingress_hash")

--todo add bobk - CPSS_CAELUM_E
if DeviceFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E" then
    require_safe_dx("configuration/tm_cfg")
end


require_safe_dx("interface/an_phy_mode")
require_safe_dx("interface/bridge_multicast_ipv4_address")
require_safe_dx("interface/bridge_multicast_ipv4_mode")
require_safe_dx("interface/bridge_multicast_ipv4_source_group_address")
require_safe_dx("interface/bridge_multicast_ipv6_address")
require_safe_dx("interface/bridge_multicast_ipv6_mode")
require_safe_dx("interface/bridge_multicast_ipv6_source_group_address")
require_safe_dx("interface/bridge_multicast_mac_address")
require_safe_dx("interface/bridge_multicast_unregistered")
require_safe_dx("interface/cascade_dsa")
require_safe_dx("interface/crc_port_disable")
require_safe_px("interface/crc_port_disable")
require_safe_dx("interface/dce_cut_though_enable")
require_safe_dx("interface/dce_cut_through_untagged_enable")
require_safe_px("interface/px_interface_dce_cut_through")
require_safe_px("interface/px_interface_dce_pfc")
require_safe_px("interface/px_interface_dce_qcn")
require_safe_dx("interface/dce_qcn_cp_enable_queue")
require_safe_dx("interface/dce_priority_flow_control_enable")
require_safe_dx("interface/ip_routing")
require_safe_dx("interface/ip_address")
require_safe_dx("interface/port_monitor")
require_safe_dx("interface/qos_default_up")
require_safe_dx("interface/switchport_allowed_vlan")
require_safe_dx("interface/shutdown")
require_safe_dx("interface/switchport_customer_vlan")
require_safe_dx("interface/switchport_ingress_filtering")
require_safe_dx("interface/switchport_pvid")
require_safe_dx("interface/vlan_mac_learning")
require_safe_dx("interface/vlan_mtu")
require_safe_dx("interface/loopback")
require_safe_px("interface/px_loopback")
require_safe_dx("interface/switchport_protected")
require_safe_dx("interface/prbs")
require_safe_dx("interface/traffic_generator")
require_safe_px("interface/px_traffic_generator")
require_safe_dx("interface/auto_tune")
require_safe_px("interface/px_auto_tune")
require_safe_dx("interface/packet-trace")
require_safe_dx("interface/policyIf")
require_safe_dx("interface/tail_drop")
require_safe_px("interface/px_interface_tail_drop")
require_safe_px("interface/px_interface_cos")
require_safe_dx("interface/wred")
require_safe_px("interface/px_interface_wred")
require_safe_dx("interface/rmon")
require_safe_px("interface/px_rmon")
require_safe_dx("interface/switchport_isolate")
require_safe_dx("interface/capwap")
require_safe_dx("interface/bpe_802_1_br_neighbor_info")
require_safe_dx("interface/vsi")
require_safe_dx("interface/switchport_egress_filtering")
require_safe_dx("interface/switchport_vlan_mapping")
require_safe_dx("interface/index_management_util")
require_safe_dx("vlan/bpe_802_1_br_flood_etag_info")
require_safe_dx("interface/mpls")
require_safe_dx("interface/mpls_transit")
require_safe_dx("interface/push_vlan_tag")
require_safe_dx("interface/pop_vlan_tag")
require_safe_dx("interface/vss_aggregate_eports")
require_safe_dx("interface/spbm_port")
require_safe_dx("interface/tuntap")
require_safe_dx("interface/speed")
require_safe_px("interface/speed")
require_safe_dx("interface/vlan_vrf_id")
require_safe_dx("interface/vlan_fdb_lookup_mode")
require_safe_dx("interface/ap-port")
require_safe_dx("interface/port-manager")
require_safe_px("interface/px_port-manager")
require_safe_dx("interface/interface_eee")
require_safe_dx("interface/interface_pip")
require_safe_dx("interface/interface_led")
require_safe_px("interface/PX_interface_led")
require_safe_dx("interface/serdes_polarity")
require_safe_px("interface/px_serdes_polarity")
require_safe_dx("interface/auto_detect")
require_safe_dx("interface/ap_introp_set")
require_safe_dx("interface/lanes_swap")
require_safe_px("interface/PX_interface_egress_ha")
require_safe_px("interface/px_serdes_rx")
require_safe_px("interface/px_serdes_tx")
require_safe_px("interface/px_ap-port")
require_safe_px("interface/px_ap_introp_set")
require_safe_dx("interface/serdes_ap_rx")
require_safe_px("interface/px_serdes_ap_rx")

if (dxCh_family == true) then

  table.insert(typeCallBacksList, function()
    if is_sip_5_15()
    then
      require_safe_dx("interface/serdes_rx_bobk")
      require_safe_dx("interface/serdes_tx_bobk")
    else
      require_safe_dx("interface/serdes_rx")
      require_safe_dx("interface/serdes_tx")
    end
  end
  )

end

require_safe_dx("interface/storm_control")
require_safe_dx("interface/serdes_reset")
require_safe_px("interface/px_serdes_reset")



require_safe("common/debug/cpss_enable")
require_safe_dx("debug/show_TCAM")
require_safe_dx("debug/dce_priority_flow_control")
require_safe_dx("debug/dce_priority_flow_control_bind")
require_safe_dx("debug/dce_priority_flow_control_global_drop")
require_safe_dx("debug/dce_priority_flow_control_global_queue")
require_safe_dx("debug/debugUtilitiesWrapper_API_02")
require_safe_dx("debug/cider")
require_safe_dx("debug/eom")
require_safe_px("debug/eom")
require_safe_dx("debug/show_descriptor")
require_safe("common/debug/show_cli_commands")
require_safe("common/debug/show_globals")
require_safe("common/debug/show_RAMFS")
require_safe("common/debug/os_test_perf")
require_safe("common/debug/list")

require_safe("common/debug/serdesConf")
require_safe_dx("debug/serdesConf")
require_safe_px("debug/serdesConf")

require_safe("common/debug/register_write")
require_safe("common/debug/register_read")
require_safe("common/debug/set_print_pause")
require_safe("common/debug/phySmi")
require_safe_dx("debug/run_ut")
require_safe_dx("debug/run_db_tests")
require_safe("common/debug/handleMultiLine")
require_safe_dx("debug/configure_pizza_arbiter")
require_safe("common/debug/execute_galtis_cmd")
require_safe("common/debug/upgradeFirmware")
require_safe_dx("debug/dsa_tag_calculator")
require_safe("common/debug/eventTable")
require_safe("common/debug/dfx_read")
require_safe("common/debug/dfx_write")
require_safe("common/debug/internal_pci_reg_read")
require_safe("common/debug/internal_pci_reg_write")
require_safe("common/debug/log")
require_safe_dx("debug/run_unit_tests")
require_safe_px("debug/px_debug_run_unit_tests")
require_safe_dx("debug/serdes_sequence")
require_safe("common/debug/memory_dump")
require_safe_dx("debug/bobk_policer_dump")
require_safe_dx("debug/printf_sync")
require_safe_dx("debug/i2c_byte_read")
require_safe_dx("debug/i2c_byte_write")
require_safe_dx("debug/set_serdes_port")

require_safe("common/traffic/RxTx")
require_safe_dx("traffic/RxTx")
require_safe_px("traffic/RxTx")
require_safe_dx("traffic/lldp_send")
require_safe_dx("traffic/capture")
require_safe_px("traffic/capture")
require_safe_dx("traffic/rx_cpu_rate_limit")
require_safe_dx("traffic/cpu_code")
if (dxCh_family == true) then
  require_safe_dx("traffic/traffic_generator_emulate")
else
  require_safe_px("traffic/traffic_generator_emulate")
end

if dxCh_family then
    table.insert(
        typeCallBacksList,
        function()
            for _,dev in pairs(wrlDevList()) do
                if is_sip_5(dev) then
                    require_safe_dx("interface/fwdtolb_inf")
                    require_safe_dx("exec/show_fwdtolb")
                    require_safe_dx("configuration/fwdtolb")

                    break
                end
            end
        end
    )
end


-- init device-dependent globals
require_safe("initEnvironment")
require_safe("common/packets_utils")


if dxCh_family and (is_sip_5(nil) or DeviceFamily == "CPSS_PP_FAMILY_DXCH_XCAT3_E") or px_family then
    require_safe("common/debug/iDebug")
end


-- dxCh_family = false
-- px_family = false

local isDxExamplesExists = false
local isPxExamplesExists = false

if (dxCh_family == true) then
  isDxExamplesExists = fs.exists("dxCh/examples/run.lua")
end

if (px_family == true) then
  isPxExamplesExists = fs.exists("px/examples/run.lua")
end

-- load examples only if enabled
if ((not isDxExamplesExists) and (not isPxExamplesExists)) then
  print("LUA_CLI without examples")
else
  print("LUA_CLI with examples")

  if isDxExamplesExists then
    require_safe_dx("examples/run")
    require_safe_dx("examples/misc/log_trace")
    require_safe_dx("examples/common/common")
    require_safe_dx("examples/common/serdes_utils")
  else
    require_safe_px("examples/run")
    require_safe_px("examples/common/packets_utils")
  end
end

if fs.exists("cmdDefs.lua")  then
  require_safe("cmdDefs")
end

-- simulation special unitities (under 'debug simulation')
require_safe("common/debug/asicSimulation")
require_safe("common/exec/show_simulation_slan")

require_safe("common/lua2c/lua2c")


-- register init module, configuration commands and callbacks
require("initCLI")


-- after all files 'required'
-- init the general resource manager
general_resource_manager_init_pre_defined_tables()
