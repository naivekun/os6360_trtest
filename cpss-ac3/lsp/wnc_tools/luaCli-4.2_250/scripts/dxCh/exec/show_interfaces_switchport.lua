--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_switchport.lua
--*
--* DESCRIPTION:
--*       showing of the arp inspection status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_interfaces_switchport_func
--        @description  show's the arp inspection status
--
--        @param params         - params["ethernet"]: interface range,
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be
--                                irrelevant
--
--        @return       true
--
local function show_interfaces_switchport_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_header_string_prefix, port_header_string_suffix
    local vlan_group, vlan_group_string
    local vlanId, vlan_string, vlanInfo
    local vlan_membership_mode, vlan_membership_mode_string
    local vlan_membership_mode_string_prefix, vlan_membership_mode_string_suffix
    local port_link, port_link_string
    local port_link_string_prefix, port_link_string_suffix
    local port_vid, port_vid_string
    local port_vid_string_prefix, port_vid_string_suffix
    local dstPort, dstDev, trunk, enable, switchport_string
    local switchport_string_prefix, switchport_string_suffix
    local ingress_filtering_enable, ingress_filtering_enable_string
    local ingress_filtering_enable_string_prefix
    local ingress_filtering_enable_string_suffix
    local mac_addreess, mac_addreess_string
    local mac_addreess_mask, mac_addreess_mask_string
    local protocol_classification_table, protocol_classification_string
    local protocol_classification_header_string
    local protocol_classification_footer_string
    local mac_classification_table, mac_classification_string
    local mac_classification_header_string, mac_classification_footer_string
    local port_footer_string
    local header_string, footer_string
    local vid1_egress_filtering_enabled, vid1_egress_filtering_string
    local vid1_egress_filtering_string_prefix
    local vid1_egress_filtering_string_suffix

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllAvailableDevicesRange()
    --command_data:initInterfaceDeviceRange()
    command_data:initDevPortRange(params)

    -- Command specific variables initialization.
    port_header_string_prefix                  = "Port "
    port_header_string_suffix                  = ":"
    vlan_membership_mode_string_prefix         = "VLAN Membership mode:\t"
    vlan_membership_mode_string_suffix         = "\n"
    port_link_string_prefix                    = "Link status:\t\t"
    port_link_string_suffix                    = ""
    port_vid_string_prefix                     = "PVID:\t\t\t"
    port_vid_string_suffix                     = ""
    switchport_string_prefix                   = "Protected:\t\t"
    switchport_string_suffix                   = ""
    ingress_filtering_enable_string_prefix     = "Ingress filtering:\t"
    ingress_filtering_enable_string_suffix     = ""
    vid1_egress_filtering_string_prefix        = "VID1 Egress filtering:\t"
    vid1_egress_filtering_string_suffix        = "\n"

    protocol_classification_header_string   = "Protocol Classification:\n\n" ..
                                              "Group   VLAN \n" ..
                                              "-----  ------\n"
    protocol_classification_footer_string   = "\n\n"
    mac_classification_header_string        =
                "MAC classification:\n\n" ..
                "       Address                  Mask             Vlan \n" ..
                "----------------------  ----------------------  ------\n"
    mac_classification_footer_string        = "\n\n"
    port_footer_string                      = ""
    header_string                           = ""
    footer_string                           = ""

     -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    -- Main port handling cycle.
    if true == command_data["status"] then
        local iterator, vlan_iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            -- Port block header string formatting and adding.
            command_data:setResultStr(port_header_string_prefix,
                                      tostring(devNum) .. "/" ..
                                      tostring(portNum),
                                      port_header_string_suffix)
            command_data:addResultToResultArray()

            -- Vlan membership mode getting.
            vlan_membership_mode        = "General"
            vlan_membership_mode_string = tostring(vlan_membership_mode)

            -- Vlan membership mode string formatting and adding.
            command_data:setResultStr(vlan_membership_mode_string_prefix,
                                      vlan_membership_mode_string,
                                      vlan_membership_mode_string_suffix)
            command_data:addResultToResultArray()

            -- Port link getting.
            command_data:clearLocalStatus()

            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssDxChPortLinkStatusGet",
                                                     devNum, portNum, "link",
                                                     "GT_BOOL")
                if        0 == result then
                    port_link = values["link"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port link getting is not allowed " ..
                                            "on device %d port %d.", devNum,
                                            portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port link getting on " ..
                                          "device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    port_link_string = boolUpDownStrGet(port_link)
                else
                    port_link_string = "n/a"
                end
            end

            -- Port link string formatting and adding.
            command_data:setResultStr(port_link_string_prefix, port_link_string,
                                      port_link_string_suffix)
            command_data:addResultToResultArray()

            -- Default vlan id getting.
            command_data:clearLocalStatus()

            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssDxChBrgVlanPortVidGet",
                                                     devNum, portNum, "vid",
                                                     "GT_U16")
                if        0 == result then
                    port_vid = values["vid"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to get default " ..
                                            "vlan id on device %d port %d.",
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at default vlan id setting " ..
                                          "on device %d port %d: %s.", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    port_vid_string = tostring(port_vid)
                else
                    port_vid_string = "n/a"
                end
            end

            -- Default vlan id string formatting and adding.
            command_data:setResultStr(port_vid_string_prefix, port_vid_string,
                                      port_vid_string_suffix)
            command_data:addResultToResultArray()







            -- Switchport configuration getting.
            command_data:clearLocalStatus()
            if true == command_data["local_status"] then
                result, values=myGenWrapper("cpssDxChBrgPrvEdgeVlanPortEnableGet",{	--enable switchport
                                {"IN","GT_U8","devNum",devNum},
                                {"IN","GT_PORT_NUM","portNum",portNum},
                                {"OUT","GT_BOOL","enable"},
                                {"OUT","GT_PORT_NUM","dstPort"},
                                {"OUT","GT_HW_DEV_NUM","dstDev"},
                                {"OUT","GT_BOOL","dstTrunk"}
                                })



                if        0 == result then
                    dstPort = values["dstPort"]
                    dstDev = values["dstDev"]
                    trunk = values["dstTrunk"]
                    enable= values["enable"]

                    result, dstDev=device_port_from_hardware_format_convert(dstDev)
                    if result~=0 then
                        command_data:addError("Error at converting device number " ..
                          "from HW: %s.",
                          returnCodes[result])
                    end

                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to get switchport " ..
                                            "mode on device %d port %d.",
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at switchport mode getting " ..
                                          "on device %d port %d: %s.", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    if true==enable then
                        if true==trunk then	--switchport port-channel
                            switchport_string="Enabled, Uplink is port-channel "..dstPort
                        else	--switchport ethernet
                            switchport_string="Enabled, Uplink is "..dstDev.."/"..dstPort
                        end

                    else	--switchport is disable
                        switchport_string="Disabled"
                    end
                else
                    switchport_string = "n/a"
                end
            end

            -- Switchport mode string formatting and adding.
            command_data:setResultStr(switchport_string_prefix, switchport_string,
                                      switchport_string_suffix)
            command_data:addResultToResultArray()







            -- Ingress filtering enabling getting.
            command_data:clearLocalStatus()

            if true == command_data["local_status"] then
                result, values =
                    cpssPerPortParamGet("cpssDxChBrgVlanPortIngFltEnableGet",
                                        devNum, portNum, "enable",
                                        "GT_BOOL")
                if        0 == result then
                    ingress_filtering_enable = values["enable"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to get ingress " ..
                                            "filtering on device %d port %d.",
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at ingress setting on " ..
                                          "device %d port %d: %s.", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    ingress_filtering_enable_string =
                        tostring(ingress_filtering_enable)
                else
                    ingress_filtering_enable_string = "n/a"
                end
            end

            -- Ingress filtering enabling string formatting and adding.
            command_data:setResultStr(ingress_filtering_enable_string_prefix,
                                      ingress_filtering_enable_string,
                                      ingress_filtering_enable_string_suffix)
            command_data:addResultToResultArray()



            -- get VID1 Egress filtering settings
            if is_sip_5(devNum) then
               command_data:clearLocalStatus()
               result, values = cpssPerPortParamGet(
                  "cpssDxChBrgEgrFltPortVid1FilteringEnableGet", devNum, portNum,
                  "enable", "GT_BOOL")

               command_data:handleCpssErrorDevPort(
                  result, "getting a VID1 egress filtering status", devNum, portNum)

               if result == 0 then
                  vid1_egress_filtering_enabled = values["enable"]
                  vid1_egress_filtering_string = to_string(vid1_egress_filtering_enabled)

                  if vid1_egress_filtering_enabled then

                     result, values = cpssPerPortParamGet(
                        "cpssDxChBrgEgrFltPortVid1Get", devNum, portNum,
                        "vid1", "GT_U16")

                     command_data:handleCpssErrorDevPort(
                        result, "getting a VID1 associated with a port", devNum, portNum)

                     if result == 0 then
                        vid1_egress_filtering_string = string.format(
                           "%s, VID1=%d", vid1_egress_filtering_string, values["vid1"])
                     end
                  end
               end
               if result ~= 0 then
                  vid1_egress_filtering_string = "n/a"
               end

               -- VID1 Egress filtering string formatting and adding.
               command_data:setResultStr(vid1_egress_filtering_string_prefix,
                                         vid1_egress_filtering_string,
                                         vid1_egress_filtering_string_suffix)
               command_data:addResultToResultArray()
            end -- is_sip_5(devNum)


            -- Protocol classification table initialization.
            command_data:clearLocalStatus()
            command_data:clearVlansCount()
            protocol_classification_table = {}

            -- Port vlan handling cycle.
            if true == command_data["local_status"] then
                local vlan_iterator
                for vlan_iterator, vlanId, vlanInfo in
                    command_data:getDevicePortVlanIterator(devNum, portNum) do
                    command_data:clearVlanStatus()

                    command_data:clearLocalStatus()

                    vlan_group          = vlanInfo["stgId"]

                    vlan_group_string   = tostring(vlan_group)
                    vlan_string         = tostring(vlanId)

                    -- Protocol classification table row string formatting and
                    -- adding.
                    command_data["result"] =
                        string.format("%-7s%-8s",
                                      alignLeftToCenterStr(vlan_group_string,
                                                           5),
                                      alignLeftToCenterStr(vlan_string,
                                                           6))
                    tableAppend(protocol_classification_table,
                                command_data["result"])

                    command_data:updateStatus()

                    command_data:updateVlans()
                end

                protocol_classification_string =
                    command_data:getValueIfSuccessVlans(
                        tbltostr(protocol_classification_table, '\n'),
                        "There is no protocol classification entries to show.")
            end

			-- Protocol classification string formatting and adding.
			command_data["result"] = protocol_classification_header_string
			command_data:addResultToResultArray()
      
			for currLine in string.gmatch(protocol_classification_string, "[^\r\n]+") do
				command_data["result"] = currLine
				command_data:addResultToResultArray()
			end

			command_data["result"] = protocol_classification_footer_string
			command_data:addResultToResultArray()

            -- Mac classification table initialization.
            command_data:clearEntriesCount()
            command_data:initAllDeviceMacEntryIterator(
                { ["static_dynamic"]    = "static",
                  ["ethernet"]          = {["devId"]    = devNum,
                                           ["portNum"]   = portNum } })
            mac_classification_table = {}

            -- Port mac-entries handling cycle.
            if true == command_data["local_status"] then
                local vlan_iterator
                for vlan_iterator in command_data:getMacEntryIterator() do
                    command_data:clearEntryStatus()

                    command_data:clearLocalStatus()

                    mac_addreess                =
                                    command_data["mac_entry"]["key"]["key"]["macVlan"]["macAddr"]
                    mac_addreess_mask           = "n/a"
                    vlanId                      =
                                    command_data["mac_entry"]["key"]["key"]["macVlan"]["vlanId"]

                    mac_addreess_string         = mac_addreess
                    mac_addreess_mask_string    = mac_addreess_mask
                    vlan_string                 = tostring(vlanId)

                    command_data:updateStatus()

                    -- Mac classification table row string formatting and
                    -- adding.
                    command_data["result"] =
                        string.format("  %-22s  %-22s%-8s",
                                      mac_addreess_string,
                                      mac_addreess_mask_string,
                                      alignLeftToCenterStr(vlan_string, 6))
                    tableAppend(mac_classification_table,
                                command_data["result"])

                    command_data:updateEntries()
                end

                mac_classification_string =
                    command_data:getValueIfSuccessEntries(
                        tbltostr(mac_classification_table, '\n'),
                        "There is no Mac classification entries to show.")
            end

			-- Mac classification string formatting and adding.
			command_data["result"] = mac_classification_header_string
			command_data:addResultToResultArray()

			for currLine in string.gmatch(mac_classification_string, "[^\r\n]+") do
				command_data["result"] = currLine
				command_data:addResultToResultArray()
			end

			command_data["result"] = mac_classification_header_suffix
			command_data:addResultToResultArray()

            -- Port block footer string formatting and adding.
            command_data:setResultStr()
            command_data:addResultToResultArray(port_footer_string)

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not disable all processed ports.")
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStr(header_string, command_data["result"],
                              footer_string)

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


CLI_addCommand("exec", "show interfaces switchport", {
  func   = show_interfaces_switchport_func,
  help   = "switchport",
  params = {
      { type = "named",
          "#interface_port_channel",
        mandatory = { "interface_port_channel" }
      }
  }
})

cmdLuaCLI_registerCfunction("wrlCpssDxChNstPortIsolationTableEntryGet")
cmdLuaCLI_registerCfunction("wrlCpssIsPortSetInPortsBmp")

local function portsBmpToStr(dev, portsBmp, value)
    if value == nil then
        value = true
    end
    local port
    local result, max_port_number
    result, max_port_number = wrLogWrapper("wrlCpssDevicePortNumberGet","(devNum)",devNum)

    local pstart, pend
    local str = ""

    local function flush(str, pstart, pend)
        if str ~= "" then
            str = str .. ","
        end
        str = str .. tostring(pstart)
        if pstart == pend then
            return str
        end
        if pend == pstart + 1 then
            return str .. "," .. tostring(pend)
        end
        return str .. "-" .. tostring(pend)
    end

    for port = 0, max_port_number-1 do
        local tmp_IsPortSetInPortsBmp = wrLogWrapper("wrlCpssIsPortSetInPortsBmp","(portsBmp, port)",portsBmp, port)
        local isPortValid
        if is_sip_5(devNum) then
            -- the 'physical ports' exists regardless to 'DMA/MAC'
            isPortValid = true
        else
            -- removing Lion/Lion2 ports that not exists (12..15,28..31,44..47,59..63)
            isPortValid = does_port_exist(dev,port)
        end
        
        if  port ~= 63 and
            tmp_IsPortSetInPortsBmp == value and
            isPortValid 
        then
            if pstart == nil then
                pstart = port
                pend = port
            else
                pend = port
            end
        else
            if pstart ~= nil then
                str = flush(str, pstart, pend)
                pstart = nil
            end
        end
    end
    if pstart ~= nil then
        str = flush(str, pstart, pend)
    end
    return str
end


local function show_interfaces_switchport_isolate_func(params)
    local command_data = Command_Data()
    local interfaceInfo
    local result, i, cpuPortMember, localPortsMembers
    local iterator
    local port_table = {}
    local res0
    local nothingToShow = true

    interfaceInfo = get_default_interface_info_stc()
    interfaceInfo["type"]            = "CPSS_INTERFACE_PORT_E"

    params["all"] = true
    params["devID"] = nil

    command_data:initAllInterfacesPortIterator(params)

    print("Dev  Port   LocalPorts(isolated)    CpuPort(isolated)")
    print("--- -----   -------------------     -----------------")

    for iterator, devNum, portNum in command_data:getPortIterator() do
        res0, interfaceInfo["devPort"]["devNum"]=device_to_hardware_format_convert(devNum)
        interfaceInfo["devPort"]["portNum"]=portNum
        interfaceInfo["devNum"] = interfaceInfo["devPort"]["devNum"]

        local traffic_type_ = "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E"
        result, cpuPortMember, localPortsMembers = wrLogWrapper("wrlCpssDxChNstPortIsolationTableEntryGet",
                                                   "(devNum, traffic_type_, interfaceInfo)", 
                                                   devNum, traffic_type_, interfaceInfo)

        command_data:handleCpssErrorDevPort(result, wrLogWrapper("wrlCpssDxChNstPortIsolationTableEntryGet"))
        
        if result == 0 then --GT_OK
            local portsStr = portsBmpToStr(devNum,localPortsMembers,false)
            
            -- 'member' means that 'no filter' --> so we 'negate' 
            if(cpuPortMember == true) then 
                cpuPortMember = false
            else
                cpuPortMember = true
            end
            
            if portsStr ~= "" then
                nothingToShow = false
                print(string.format("%-5s%-5s  %-23s    %-5s",
                            to_string(devNum),
                            to_string(portNum),
                            portsStr,
                            to_string(cpuPortMember)))
            end
        end
    end
    
    if(nothingToShow == true) then
        print("     ---- nothing to show -----")
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
 end

CLI_addCommand("exec", "show interfaces switchport isolate", {
  func   = show_interfaces_switchport_isolate_func,
  help   = "Show isolated ports",
})
