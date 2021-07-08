--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* nat66_utils.lua
--*
--* DESCRIPTION:
--*       Any functions, variables etc that are required in several tests.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--************************************************************************
--  show_nat66_test
--        @description  tests show nat66 CLI command by comparing
--                      displayed values from CPSS and Lua database
--
local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

function show_nat66_test()
    local current_index
    local current_entry = {}
    local apiName,result,values
    local table_nat66_system_info = {}
    local localHwDevNum, outerHwDevNum
    local enableRoutingPtr

    local physicalInfoPtr = {} 
    physicalInfoPtr.devPort = {}
    physicalInfoPtr.devPort.portNum = 0

    local routeEntriesArray = {}
    routeEntriesArray.type = "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E"
    routeEntriesArray.entry = {}
    routeEntriesArray.entry.regularEntry = {}
    routeEntriesArray.entry.regularEntry.nextHopInterface = {}
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort = {}

    local entryPtr = {}
    entryPtr.nat66Entry = {}

    local vrId
    local ipAddrPtr = {}
    local prefixLenPtr = 128

    local GT_NOT_FOUND = (0x0B)

    table_nat66_system_info = getTableNat66()

    -- get the first element in the table
    current_index, current_entry =  next(table_nat66_system_info,nil)

    while current_entry do
        if current_entry.nat_name ~= nil then
            print(current_entry.nat_name .. " entry")
        
            -- get physical info for the given ePort
            apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                   "devNum",          current_entry.local_interface.devId},
                { "IN",     "GT_PORT_NUM",             "portNum",         current_entry.local_eport},
                { "OUT",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr"},
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("local interface portNum", current_entry.local_interface.portNum, values.physicalInfoPtr.devPort.portNum)
            check_expected_value("local interface port type ", "CPSS_INTERFACE_PORT_E" , values.physicalInfoPtr.type)
        
            apiName = "cpssDxChCfgHwDevNumGet"
            result, localHwDevNum = device_to_hardware_format_convert(current_entry.local_interface.devId)
            _debug(apiName .. "   result = " .. result)

            check_expected_value("local interface hwDevNum", localHwDevNum, values.physicalInfoPtr.devPort.devNum)

            -- get physical info for the given ePort
            apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                   "devNum",          current_entry.outer_interface.devId},
                { "IN",     "GT_PORT_NUM",             "portNum",         current_entry.outer_eport},
                { "OUT",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr"},
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("outer interface portNum", current_entry.outer_interface.portNum, values.physicalInfoPtr.devPort.portNum)
            check_expected_value("outer interface port type ", "CPSS_INTERFACE_PORT_E" , values.physicalInfoPtr.type)

            apiName = "cpssDxChCfgHwDevNumGet"
            result, outerHwDevNum = device_to_hardware_format_convert(current_entry.outer_interface.devId)
            _debug(apiName .. "   result = " .. result)

            check_expected_value("outer interface hwDevNum", outerHwDevNum, values.physicalInfoPtr.devPort.devNum)

            -- read an array of uc route entries to hw
            apiName = "cpssDxChIpUcRouteEntriesRead"
            result, values = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                            "devNum",               devNum },
                { "IN",     "GT_U32",                           "baseRouteEntryIndex",  current_entry.ipNextHopIndex1},
                { "INOUT",  "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntriesArray },
                { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("local vid", current_entry.local_vid, values.routeEntriesArray.entry.regularEntry.nextHopVlanId)
            check_expected_value("local eport", current_entry.local_eport, values.routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum)

            -- read an array of uc route entries to hw
            apiName = "cpssDxChIpUcRouteEntriesRead"
            result, values = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                            "devNum",               devNum },
                { "IN",     "GT_U32",                           "baseRouteEntryIndex",  current_entry.ipNextHopIndex},
                { "INOUT",  "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntriesArray },
                { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("outer vid", current_entry.outer_vid, values.routeEntriesArray.entry.regularEntry.nextHopVlanId)
            check_expected_value("outer eport", current_entry.outer_eport, values.routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum)

            -- Get NAT entry
            apiName = "cpssDxChIpNatEntryGet"
            result,values = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                                    "devNum",   devNum },
                { "IN",     "GT_U32",                                   "natIndex", current_entry.natIndex1 },
                { "OUT",     "CPSS_IP_NAT_TYPE_ENT",                     "natTypePtr"},
                { "OUT",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat66Entry",    "entryPtr"}
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("local mac address", current_entry.local_mac_addr.string, values.entryPtr.nat66Entry.macDa)
            check_expected_value("private IP address", get_full_representation_of_ipv6_addr(current_entry.private_ip.string), get_full_representation_of_ipv6_addr(values.entryPtr.nat66Entry.address))
            check_expected_value("public to private prefix length", current_entry.prefix_length, values.entryPtr.nat66Entry.prefixSize)

            -- Get NAT entry
            apiName = "cpssDxChIpNatEntryGet"
            result,values = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                                    "devNum",   devNum },
                { "IN",     "GT_U32",                                   "natIndex", current_entry.natIndex },
                { "OUT",     "CPSS_IP_NAT_TYPE_ENT",                     "natTypePtr"},
                { "OUT",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat66Entry",    "entryPtr"}
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("outer mac address", current_entry.outer_mac_addr.string, values.entryPtr.nat66Entry.macDa)
            check_expected_value("public IP address", get_full_representation_of_ipv6_addr(current_entry.public_ip.string), get_full_representation_of_ipv6_addr(values.entryPtr.nat66Entry.address))
            check_expected_value("private to public prefix length", current_entry.prefix_length, values.entryPtr.nat66Entry.prefixSize)

            -- get members from the vlan entry
            apiName = "cpssDxChBrgVlanEntryRead"
            result, values = myGenWrapper(apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "GT_U16",     "vlanId",   current_entry.local_vid },
                { "OUT",     "CPSS_PORTS_BMP_STC",  "portsMembers"},
                { "OUT",     "CPSS_PORTS_BMP_STC",  "portsTagging"},
                { "OUT",     "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"},
                { "OUT",     "GT_BOOL",  "isValid"},
                { "OUT",     "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",  "portsTaggingCmd"}
            })
            _debug(apiName .. "   result = " .. result)

            _debug("vrfId " .. values.vlanInfo.vrfId)

            vrId = values.vlanInfo.vrfId

            ipAddrPtr = "0::0" 

            while (get_full_representation_of_ipv6_addr(ipAddrPtr) ~= get_full_representation_of_ipv6_addr(current_entry.server_ip.string) or (prefixLenPtr ~= 128)) do

                apiName = "cpssDxChIpLpmIpv6UcPrefixGetNext"
                result, values = myGenWrapper(apiName, {
                    { "IN",     "GT_U32",      "lpmDBId", 0 },
                    { "IN",     "GT_U32",      "vrId",  vrId },
                    { "INOUT",  "GT_IPV6ADDR",  "ipAddrPtr", ipAddrPtr},
                    { "INOUT",  "GT_U32",  "prefixLenPtr", prefixLenPtr},
                    { "OUT",    "CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT",  "nextHopInfoPtr"},
                    { "OUT",    "GT_U32",  "tcamRowIndexPtr"},
                    { "OUT",    "GT_U32",  "tcamColumnIndexPtr"}
                })
                _debug(apiName .. "   result = " .. result)

                ipAddrPtr = values.ipAddrPtr
                prefixLenPtr = values.prefixLenPtr

                if result == GT_NOT_FOUND then 
                    break
                end
            end

            check_expected_value("server IP address", get_full_representation_of_ipv6_addr(current_entry.server_ip.string), get_full_representation_of_ipv6_addr(ipAddrPtr))

            current_index, current_entry = next(table_nat66_system_info,current_index)
        end
    end
end

