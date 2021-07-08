--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_capabilities_device.lua
--*
--* DESCRIPTION:
--*       showing of system capabilities
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDevicePortNumberGet")
cmdLuaCLI_registerCfunction("wrlCpssDeviceTypeGet")
cmdLuaCLI_registerCfunction("wrlCpssDeviceRevisionGet")

--constants

-- ************************************************************************
--  sipBmpToString
--        @description  transporm a bitmap with supported SIP versions
--                      to a string in accordance with CPSS_GEN_SIP_ENT names.
--                      Example:
--                          0x6  - > "SIP_5, SIP5_10",
--                          because 0x6 has 2nd, 3rd set bits and this
--                          appropriates to  CPSS_GEN_SIP_5_E (2) and
--                          CPSS_GEN_SIP_5_10_E (3) constants.
--
--        @param sipBmpArr - array of bitmaps
--
--        @return       a string.
--
local function sipBmpToString(sipBmpArr)
    local tab = {}              -- table of strings
    for i=0,#sipBmpArr do
        local n = 0
        local bmp = sipBmpArr[i]
        while bmp > 0 do
            local bit = bit_and(bmp, 0x1)
            if bit == 1 then
                local name = cmdLuaCLI_callCfunction("mgmType_to_lua_CPSS_GEN_SIP_ENT", n)
                if name == nil then
                    name = string.format("UNKNOWN(%d)", n)
                else
                    name = string.gsub(name, "^CPSS_GEN_SIP_(.*)_E$", "%1")
                end
                table.insert(tab, name)
            end
            bmp = bit_shr(bmp,1)
            n = n + 1
        end
    end
    -- transform table to string
    return table.concat(tab, ', ')
end

-- ************************************************************************
---
--  show_capabilities_device_func_given_device
--        @description  shows system capabilities for given device
--
--        @param command_data   - command execution data object
--        @param devNum         - device number
--        @param header_string  - printed header string
--
--        @return       true on success, otherwise false and error message
--
local function show_capabilities_device_func_given_device(command_data, devNum, header_str)
    -- Common variables declaration
    local result, values
    -- Command  specific variables declaration
    local vlan_entries_number_string_prefix
    local vlan_entries_number_string_suffix
    local vlan_entries_number, vlan_entries_number_string
    local fdb_entries_number_string_prefix
    local fdb_entries_number_string_suffix
    local fdb_entries_number, fdb_entries_number_string
    local pcl_action_entries_number_string_prefix
    local pcl_action_entries_number_string_suffix
    local pcl_action_entries_number
    local pcl_action_entries_number_string
    local pcl_tcam_entries_number_string_prefix
    local pcl_tcam_entries_number_string_suffix
    local pcl_tcam_entries_number, pcl_tcam_entries_number_string
    local router_next_hop_entries_number_string_prefix
    local router_next_hop_entries_number_string_suffix
    local router_next_hop_entries_number
    local router_next_hop_entries_number_string
    local lookup_translation_entries_number_string_prefix
    local lookup_translation_entries_number_string_suffix
    local lookup_translation_entries_number
    local lookup_translation_entries_number_string
    local router_tcam_entries_number_string_prefix
    local router_tcam_entries_number_string_suffix
    local router_tcam_entries_number
    local router_tcam_entries_number_string
    local mll_pair_entries_number_string_prefix
    local mll_pair_entries_number_string_suffix
    local mll_pair_entries_number, mll_pair_entries_number_string
    local policer_meter_entries_number_string_prefix
    local policer_meter_entries_number_string_suffix
    local policer_meter_entries_number
    local policer_meter_entries_number_string
    local policer_billing_counters_entries_number_string_prefix
    local policer_billing_counters_entries_number_string_suffix
    local policer_billing_counters_entries_number
    local policer_billing_counters_entries_number_string
    local vidx_entries_number_string_prefix
    local vidx_entries_number_string_suffix
    local vidx_entries_number, vidx_entries_number_string
    local arp_tunnel_start_entries_number_string_prefix
    local arp_tunnel_start_entries_number_string_suffix
    local arp_tunnel_start_entries_number
    local arp_tunnel_start_entries_number_string
    local spanning_tree_entries_number_string_prefix
    local spanning_tree_entries_number_string_suffix
    local spanning_tree_entries_number
    local spanning_tree_entries_number_string
    local qos_profiles_number_string_prefix
    local qos_profiles_number_string_suffix
    local qos_profiles_number, qos_profiles_number_string
    local mac_to_me_entries_number_string_prefix
    local mac_to_me_entries_number_string_suffix
    local mac_to_me_entries_number
    local mac_to_me_entries_number_string
    local centralized_counters_entries_number_string_prefix
    local centralized_counters_entries_number_string_suffix
    local centralized_counters_entries_number
    local centralized_counters_entries_number_string
    local cnc_blocks_number_string_prefix
    local cnc_blocks_number_string_suffix
    local cnc_blocks_number, cnc_blocks_number_string
    local trunk_entries_number_string_prefix
    local trunk_entries_number_string_suffix
    local trunk_entries_number, trunk_entries_number_string
    local max_port_number_string_prefix
    local max_port_number_string_suffix
    local max_port_number, max_port_number_string
    local device_type_string_prefix, device_type_string_suffix
    local device_type, device_type_string
    local device_revision_string_prefix, device_revision_string_suffix
    local device_revision, device_revision_string
    local device_exisitg_ports_bitmap0_string_prefix
    local device_exisitg_ports_bitmap0_string_suffix
    local device_exisitg_ports_bitmap0, device_exisitg_ports_bitmap0_string
    local device_exisitg_ports_bitmap1_string_prefix
    local device_exisitg_ports_bitmap1_string_suffix
    local device_exisitg_ports_bitmap1, device_exisitg_ports_bitmap1_string
    local device_exisitg_ports_bitmap2_string_prefix
    local device_exisitg_ports_bitmap2_string_suffix
    local device_exisitg_ports_bitmap2, device_exisitg_ports_bitmap2_string
    local device_exisitg_ports_bitmap3_string_prefix
    local device_exisitg_ports_bitmap3_string_suffix
    local device_exisitg_ports_bitmap3, device_exisitg_ports_bitmap3_string
    local supportedSip_prefix
    local supportedSip
    local header_string, footer_string

    -- Command specific variables initialization.
    if nil == header_str then
        header_string                        = ""
    else
        header_string                        = header_str
    end
    vlan_entries_number_string_prefix        =
                                    "Number of entries in VLAN table\t\t\t\t\t"
    vlan_entries_number_string_suffix        = ""
    fdb_entries_number_string_prefix         =
                                    "Number of entries in FDB table\t\t\t\t\t"
    fdb_entries_number_string_suffix         = ""
    pcl_action_entries_number_string_prefix  =
                                "Number of entries in PCL ACTION table\t\t\t\t"
    pcl_action_entries_number_string_suffix  = ""
    pcl_tcam_entries_number_string_prefix    =
                                    "Number of entries in PCL TCAM\t\t\t\t\t"
    pcl_tcam_entries_number_string_suffix    = ""
    router_next_hop_entries_number_string_prefix =
                            "Number of entries in ROUTER NEXT HOP table\t\t\t"
    router_next_hop_entries_number_string_suffix = ""
    lookup_translation_entries_number_string_prefix =
                "Number of entries in ROUTER LTT (lookup translation) table\t"
    lookup_translation_entries_number_string_suffix = ""
    router_tcam_entries_number_string_prefix =
                                    "Number of entries in ROUTER TCAM\t\t\t\t"
    router_tcam_entries_number_string_suffix = ""
    mll_pair_entries_number_string_prefix    =
                                    "Number of entries in MLL PAIR table\t\t\t\t"
    mll_pair_entries_number_string_suffix    = ""
    policer_meter_entries_number_string_prefix =
                                "Number of entries in POLICER METER table\t\t\t"
    policer_meter_entries_number_string_suffix = ""
    policer_billing_counters_entries_number_string_prefix =
                    "Number of entries in POLICER BILLING COUNTERS table\t\t"
    policer_billing_counters_entries_number_string_suffix = ""
    vidx_entries_number_string_prefix        =
                                    "Number of entries in VIDX table\t\t\t\t\t"
    vidx_entries_number_string_suffix        = ""
    arp_tunnel_start_entries_number_string_prefix =
                            "Number of entries in ARP TUNNEL START table\t\t\t"
    arp_tunnel_start_entries_number_string_suffix = ""
    spanning_tree_entries_number_string_prefix =
                        "Number of entries in STG (spanning tree) table\t\t\t"
    spanning_tree_entries_number_string_suffix = ""
    qos_profiles_number_string_prefix        =
                                    "Number of QOS PROFILES in system\t\t\t\t"
    qos_profiles_number_string_suffix        = ""
    mac_to_me_entries_number_string_prefix   =
                                "Number of entries in MAC-TO-ME table\t\t\t\t"
    mac_to_me_entries_number_string_suffix   = ""
    centralized_counters_entries_number_string_prefix =
                                "Number of entries in entire CNC table\t\t\t\t"
    centralized_counters_entries_number_string_suffix = ""
    cnc_blocks_number_string_prefix          =
                                 "Number of entires of single BLOCK of CNC\t\t\t"
    cnc_blocks_number_string_suffix          = ""
    trunk_entries_number_string_prefix       =
                                    "Number of entries in TRUNK table\t\t\t\t"
    trunk_entries_number_string_suffix       = ""
    max_port_number_string_prefix            = "Max port number\t\t\t\t\t\t\t"
    max_port_number_string_suffix            = ""
    device_type_string_prefix                = "Device Type\t\t\t\t\t\t\t"
    device_type_string_suffix                = ""
    device_revision_string_prefix            = "Device Revision\t\t\t\t\t\t\t"
    device_revision_string_suffix            = ""
    device_exisitg_ports_bitmap0_string_prefix  =
                                        "DeviceExistingPortsBitmap.0\t\t\t\t\t"
    device_exisitg_ports_bitmap0_string_suffix  = ""
    device_exisitg_ports_bitmap1_string_prefix  =
                                        "DeviceExistingPortsBitmap.1\t\t\t\t\t"
    device_exisitg_ports_bitmap1_string_suffix  = ""
    device_exisitg_ports_bitmap2_string_prefix  =
                                        "DeviceExistingPortsBitmap.2\t\t\t\t\t"
    device_exisitg_ports_bitmap2_string_suffix  = ""
    device_exisitg_ports_bitmap3_string_prefix  =
                                        "DeviceExistingPortsBitmap.3\t\t\t\t\t"
    device_exisitg_ports_bitmap3_string_suffix  = ""
    supportedSip_prefix =               "Supported SIP versions\t\t\t\t\t\t"
    footer_string                            = "\n"

    -- Getting of VLAN table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        vlan_entries_number = system_capability_get_table_size(devNum, "VLAN")
        vlan_entries_number_string = strIfNotNil(vlan_entries_number)
    end

    command_data:updateStatus()

    -- VLAN table entries number string formatting and adding.
    command_data:setResultStr(vlan_entries_number_string_prefix,
                              vlan_entries_number_string,
                              vlan_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of FDB table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        fdb_entries_number = system_capability_get_table_size(devNum, "FDB")
        fdb_entries_number_string = strIfNotNil(fdb_entries_number)
    end

    command_data:updateStatus()

    -- FDB table entries number string formatting and adding.
    command_data:setResultStr(fdb_entries_number_string_prefix,
                              fdb_entries_number_string,
                              fdb_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of PCL ACTION table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        pcl_action_entries_number = system_capability_get_table_size(
                                        devNum, "PCL_ACTION")
        pcl_action_entries_number_string = strIfNotNil(pcl_action_entries_number)
    end

    command_data:updateStatus()

    -- PCL ACTION table entries number string formatting and adding.
    command_data:setResultStr(pcl_action_entries_number_string_prefix,
                              pcl_action_entries_number_string,
                              pcl_action_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of PCL TCAM entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        pcl_tcam_entries_number = system_capability_get_table_size(
                                        devNum, "PCL_TCAM")
        pcl_tcam_entries_number_string = strIfNotNil(pcl_tcam_entries_number)
    end

    command_data:updateStatus()

    -- PCL TCAM table entries number string formatting and adding.
    command_data:setResultStr(pcl_tcam_entries_number_string_prefix,
                              pcl_tcam_entries_number_string,
                              pcl_tcam_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of ROUTER NEXT HOP table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        router_next_hop_entries_number = system_capability_get_table_size(
                                             devNum, "ROUTER_NEXT_HOP")
        router_next_hop_entries_number_string =
            strIfNotNil(router_next_hop_entries_number)
    end

    command_data:updateStatus()

    -- ROUTER NEXT HOP table entries number string formatting and adding.
    command_data:setResultStr(router_next_hop_entries_number_string_prefix,
                              router_next_hop_entries_number_string,
                              router_next_hop_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of ROUTER LTT (lookup translation) table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        lookup_translation_entries_number = system_capability_get_table_size(
                                                devNum, "ROUTER_LTT")
        lookup_translation_entries_number_string =
            strIfNotNil(lookup_translation_entries_number)
    end

    command_data:updateStatus()

    -- ROUTER LTT (lookup translation) table entries number string
    -- formatting and adding.
    command_data:setResultStr(lookup_translation_entries_number_string_prefix,
                              lookup_translation_entries_number_string,
                              lookup_translation_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of ROUTER TCAM entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        router_tcam_entries_number = system_capability_get_table_size(
                                         devNum, "ROUTER_TCAM")
        router_tcam_entries_number_string =
            strIfNotNil(router_tcam_entries_number)
    end

    command_data:updateStatus()

    -- ROUTER TCAM table entries number string formatting and adding.
    command_data:setResultStr(router_tcam_entries_number_string_prefix,
                              router_tcam_entries_number_string,
                              router_tcam_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of MLL PAIR table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        mll_pair_entries_number = system_capability_get_table_size(
                                         devNum, "MLL_PAIR")
        mll_pair_entries_number_string =
            strIfNotNil(mll_pair_entries_number)
    end

    command_data:updateStatus()

    -- MLL PAIR table entries number string formatting and adding.
    command_data:setResultStr(mll_pair_entries_number_string_prefix,
                              mll_pair_entries_number_string,
                              mll_pair_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of POLICER METER table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        policer_meter_entries_number = system_capability_get_table_size(
                                          devNum, "POLICER_METERS")
        policer_meter_entries_number_string =
            strIfNotNil(policer_meter_entries_number)
    end

    command_data:updateStatus()

    -- POLICER METER table entries number string formatting and adding.
    command_data:setResultStr(policer_meter_entries_number_string_prefix,
                              policer_meter_entries_number_string,
                              policer_meter_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of POLICER BILLING COUNTERS table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        policer_billing_counters_entries_number =
            system_capability_get_table_size(devNum, "POLICER_BILLING_COUNTERS")
        policer_billing_counters_entries_number_string =
            strIfNotNil(policer_billing_counters_entries_number)
    end

    command_data:updateStatus()

    -- POLICER BILLING COUNTERS table entries number string
    -- formatting and adding.
    command_data:setResultStr(policer_billing_counters_entries_number_string_prefix,
                              policer_billing_counters_entries_number_string,
                              policer_billing_counters_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of VIDX table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        vidx_entries_number = system_capability_get_table_size(devNum, "VIDX")
        vidx_entries_number_string = strIfNotNil(vidx_entries_number)
    end

    command_data:updateStatus()

    -- VIDX table entries number string formatting and adding.
    command_data:setResultStr(vidx_entries_number_string_prefix,
                              vidx_entries_number_string,
                              vidx_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of ARP TUNNEL START table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        arp_tunnel_start_entries_number = system_capability_get_table_size(
                                              devNum, "ARP")
        arp_tunnel_start_entries_number_string =
            strIfNotNil(arp_tunnel_start_entries_number)
    end

    command_data:updateStatus()

    -- ARP TUNNEL START table entries number string formatting and adding.
    command_data:setResultStr(arp_tunnel_start_entries_number_string_prefix,
                              arp_tunnel_start_entries_number_string,
                              arp_tunnel_start_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of STG (spanning tree) table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        spanning_tree_entries_number = system_capability_get_table_size(
                                              devNum, "STG")
        spanning_tree_entries_number_string =
            strIfNotNil(spanning_tree_entries_number)
    end

    command_data:updateStatus()

    -- STG (spanning tree) table entries number string formatting and adding.
    command_data:setResultStr(spanning_tree_entries_number_string_prefix,
                              spanning_tree_entries_number_string,
                              spanning_tree_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of QOS PROFILES number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        qos_profiles_number = system_capability_get_table_size(devNum,
                                                               "QOS_PROFILE")
        qos_profiles_number_string =
            strIfNotNil(qos_profiles_number)
    end

    command_data:updateStatus()

    -- QOS PROFILES number string formatting and adding.
    command_data:setResultStr(qos_profiles_number_string_prefix,
                              qos_profiles_number_string,
                              qos_profiles_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of MAC-TO-ME table entries number in system.
    command_data:clearStatus()

    if true  == command_data["status"] then
        mac_to_me_entries_number = system_capability_get_table_size(devNum,
                                                                    "MAC_TO_ME")
        mac_to_me_entries_number_string =
            strIfNotNil(mac_to_me_entries_number)
    end

    command_data:updateStatus()

    -- MAC-TO-ME table entries number string formatting and adding.
    command_data:setResultStr(mac_to_me_entries_number_string_prefix,
                              mac_to_me_entries_number_string,
                              mac_to_me_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of CNC (centralized counters) entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        centralized_counters_entries_number = system_capability_get_table_size(
                                                  devNum, "CNC")
        centralized_counters_entries_number_string =
            strIfNotNil(centralized_counters_entries_number)
    end

    command_data:updateStatus()

    -- CNC BLOCK (centralized counters) entries number string
    -- formatting and adding.
    command_data:setResultStr(centralized_counters_entries_number_string_prefix,
                              centralized_counters_entries_number_string,
                              centralized_counters_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of CNC BLOCKS number in system
    command_data:clearStatus()

    if true  == command_data["status"] then
        cnc_blocks_number = system_capability_get_table_size(devNum,
                                                             "CNC_BLOCKS")
        cnc_blocks_number_string = strIfNotNil(cnc_blocks_number)
    end

    command_data:updateStatus()

    -- CNC BLOCKS number string formatting and adding.
    command_data:setResultStr(cnc_blocks_number_string_prefix,
                              cnc_blocks_number_string,
                              cnc_blocks_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of TRUNK table entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        trunk_entries_number = system_capability_get_table_size(devNum,
                                                             "TRUNK")
        trunk_entries_number_string = strIfNotNil(trunk_entries_number)
    end

    command_data:updateStatus()

    -- TRUNK table entries number string formatting and adding.
    command_data:setResultStr(trunk_entries_number_string_prefix,
                              trunk_entries_number_string,
                              trunk_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device max port number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result, values = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
        if     0 == result then
            max_port_number = values
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at max port number getting of device %d: %s ",
                                  devNum, values)
        end

        if 0 == result then
            max_port_number_string = strIfNotNil(max_port_number - 1)
        else
            max_port_number_string = "n/a"
        end
    end

    command_data:updateStatus()

    -- Device max port number string formatting and adding.
    command_data:setResultStr(max_port_number_string_prefix,
                              max_port_number_string,
                              max_port_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device type.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result = wrLogWrapper("wrlCpssDeviceTypeGet", "(devNum)", devNum)
        if nil ~= result then
            device_type_string = string.format("%x", result)
        else
            device_type_string = "n/a"
            command_data:setFailStatus()
            command_data:addError("Error at type getting of device %d", devNum)
        end
    end

    command_data:updateStatus()

    -- Device max port number string formatting and adding.
    command_data:setResultStr(device_type_string_prefix,
                              device_type_string,
                              device_type_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device revision.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result = wrLogWrapper("wrlCpssDeviceRevisionGet", "(devNum)", devNum)

        if nil ~= result then
            device_revision_string = strIfNotNil(result)
        else
            device_revision_string = "n/a"
            command_data:setFailStatus()
            command_data:addError("Error at type getting of device %d", devNum)
        end
    end

    command_data:updateStatus()

    -- Device max port number string formatting and adding.
    command_data:setResultStr(device_revision_string_prefix,
                              device_revision_string,
                              device_revision_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device exisitg ports bitmap.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result, values = cpssPerDeviceParamGet("cpssDxChCfgDevInfoGet",
                                               devNum, "devInfo",
                                               "CPSS_DXCH_CFG_DEV_INFO_STC")
        if        0 == result then
            device_exisitg_ports_bitmap0 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][0]
            device_exisitg_ports_bitmap1 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][1]
            device_exisitg_ports_bitmap2 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][2]
            device_exisitg_ports_bitmap3 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][3]

            supportedSip = values["devInfo"]["genDevInfo"]["supportedSipBmp"]

        elseif 0x10 == result then
            command_data:setFailStatus()
            command_data:addWarning("Exisitg ports bitmap getting is not " ..
                                    "allowed on device %d.", devNum)
        elseif    0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at exisitg ports bitmap getting " ..
                                  "on device %d: %s.", devNum,
                                  returnCodes[result])
        end

        if 0 == result then
        device_exisitg_ports_bitmap0_string =
            string.format("%x", device_exisitg_ports_bitmap0)
            device_exisitg_ports_bitmap1_string =
                string.format("%x", device_exisitg_ports_bitmap1)
            device_exisitg_ports_bitmap2_string =
                string.format("%x", device_exisitg_ports_bitmap2)
            device_exisitg_ports_bitmap3_string =
                string.format("%x", device_exisitg_ports_bitmap3)
        else
            device_exisitg_ports_bitmap0_string = "n/a"
            device_exisitg_ports_bitmap1_string = "n/a"
            device_exisitg_ports_bitmap2_string = "n/a"
            device_exisitg_ports_bitmap3_string = "n/a"
        end
    end

    command_data:updateStatus()

    -- Device exisitg ports bitmap string formatting and adding.
    command_data:setResultStr(device_exisitg_ports_bitmap0_string_prefix,
                              device_exisitg_ports_bitmap0_string,
                              device_exisitg_ports_bitmap0_string_suffix)
    command_data:addResultToResultArray()
    command_data:setResultStr(device_exisitg_ports_bitmap1_string_prefix,
                              device_exisitg_ports_bitmap1_string,
                              device_exisitg_ports_bitmap1_string_suffix)
    command_data:addResultToResultArray()
    command_data:setResultStr(device_exisitg_ports_bitmap2_string_prefix,
                              device_exisitg_ports_bitmap2_string,
                              device_exisitg_ports_bitmap2_string_suffix)
    command_data:addResultToResultArray()
    command_data:setResultStr(device_exisitg_ports_bitmap3_string_prefix,
                              device_exisitg_ports_bitmap3_string,
                              device_exisitg_ports_bitmap3_string_suffix)
    command_data:addResultToResultArray()

    command_data:setResultStr(supportedSip_prefix, sipBmpToString(supportedSip), "")
    command_data:addResultToResultArray()

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStr(header_string, command_data["result"],
                              footer_string)

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  show_capabilities_device_func_all_devices
--        @description  shows system capabilities for all avaible devices
--
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_capabilities_device_func_all_devices(command_data)
    -- Common variables declaration
    local res = true
    local val = nil
    local result, values
    local devNum

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getAllAvailableDevicesIterator() do
            result, values = show_capabilities_device_func_given_device( command_data, devNum,
                                                                         "Device " .. tostring(devNum) .. ":")
            if result ~= true then
                res = result
                val = values
            end
        end
    end

    return res, val
end


-- ************************************************************************
---
--  show_capabilities_device_func
--        @description  shows of version information for the system
--
--        @param params         - params["all"]: property of all avaible
--                                device capabilities showing (alternative
--                                to params["devID"]);
--                                params["devID"]: checked device id
--                                (alternative to params["all"])
--
--        @return       true on success, otherwise false and error message
--
local function show_capabilities_device_func(params)
    -- Common variables declaration
    local result, values
    local command_data = Command_Data()

    -- Common variables initialization.
    command_data:clearResultArray()

    if params["all_device"] ~= "all" then
        result, values =
            show_capabilities_device_func_given_device(command_data, params["all_device"])
    else
        result, values =
            show_capabilities_device_func_all_devices(command_data)
    end

    return result, values
end


--------------------------------------------------------------------------------
-- command registration: show capabilities
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show capabilities", {
  func   = show_capabilities_device_func,
  help   = "Displaing of system capabilities",
  params = {
      { type = "named",
          "#all_device",
          mandatory = { "all_device" }
      }
  }
})
