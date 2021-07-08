
--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mpls_utils.lua
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
--  show_mpls_test
--        @description  tests show mpls CLI command by comparing
--                      displayed values from CPSS and Lua database
--
local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

function show_mpls_test()
    local current_index
    local current_entry = {}
    local result, values
    local iterator = 1
    local hwDevNum
    local valuePtr = {}
    local maskPtr = {}
    local actionPtr
    local patternPtr = {}
    patternPtr.udbArray = {}
    local interfaceInfoPtr = {}
    local macSaAddrPtr = {}
    local valuePtr
    local configPtr = {}
    local physicalInfoPtr = {}
    configPtr.mplsCfg = {}
    local ingress_label
    local i,j

    table_mpls_system_info = getTableMpls()

    -- get the first element in the table
    current_index, current_entry =  next(table_mpls_system_info,nil)

    while current_entry ~= nil do
        print("mpls entry " .. iterator)

        -- get a TTI MacToMe entry
        apiName = "cpssDxChTtiMacToMeGet"
        result, values = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                        "devNum",  current_entry.devNum },
            { "IN",     "GT_U32",                       "entryIndex",  current_entry.mac2meIndex },
            { "OUT",    "CPSS_DXCH_TTI_MAC_VLAN_STC",   "valuePtr"},
            { "OUT",    "CPSS_DXCH_TTI_MAC_VLAN_STC",   "maskPtr"},
            { "OUT",    "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC", "interfaceInfoPtr"}
        })
        _debug(apiName .. "   result = " .. result)

        check_expected_value("my mac address", current_entry.my_mac_addr.string, values.maskPtr.mac)
        check_expected_value("vlan Id", current_entry.vlan_id, values.valuePtr.vlanId)
        check_expected_value("port number", current_entry.portNum, values.interfaceInfoPtr.srcPortTrunk)

        apiName = "cpssDxChCfgHwDevNumGet"
        result, hwDevNum = device_to_hardware_format_convert(current_entry.devNum)
        _debug(apiName .. "   result = " .. result)

        check_expected_value("interface hwDevNum", hwDevNum, values.interfaceInfoPtr.srcDevice)

        if(current_entry.tsIndex~=nil)then
            -- get a tunnel start entry
            apiName = "cpssDxChTunnelStartEntryGet"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                 "devNum",                        current_entry.devNum},
                { "IN",     "GT_U32",                "routerArpTunnelStartLineIndex", current_entry.tsIndex},
                { "OUT",     "CPSS_TUNNEL_TYPE_ENT",  "tunnelType"},
                { "OUT",     "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_mplsCfg", "configPtr"},
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("next hop mac address", current_entry.nexthop_mac_addr.string, values.configPtr.mplsCfg.macDa)

            if current_entry.entropy_label0 ~= nil then
                check_expected_value("entropy label 0", current_entry.entropy_label0, values.configPtr.mplsCfg.pushEliAndElAfterLabel1)
            end

            if current_entry.entropy_label1 ~= nil then
                check_expected_value("entropy label 1", current_entry.entropy_label1, values.configPtr.mplsCfg.pushEliAndElAfterLabel2)
            end

            if current_entry.entropy_label2 ~= nil then
                check_expected_value("entropy label 2", current_entry.entropy_label2, values.configPtr.mplsCfg.pushEliAndElAfterLabel3)
            end

            if((current_entry.egress_label[1] ~= nil)and(current_entry.egress_label[2] ~= nil)and (current_entry.egress_label[3] ~= nil)) then
                check_expected_value("egress label 0", current_entry.egress_label[1], values.configPtr.mplsCfg.label3)
                check_expected_value("egress label 1", current_entry.egress_label[2], values.configPtr.mplsCfg.label2)
                check_expected_value("egress label 2", current_entry.egress_label[3], values.configPtr.mplsCfg.label1)
            else
                if((current_entry.egress_label[1] ~= nil)and(current_entry.egress_label[2] ~= nil)) then
                    check_expected_value("egress label 0", current_entry.egress_label[1], values.configPtr.mplsCfg.label2)
                    check_expected_value("egress label 1", current_entry.egress_label[2], values.configPtr.mplsCfg.label1)
                else
                    if(current_entry.egress_label[1] ~= nil) then
                        check_expected_value("egress label 0", current_entry.egress_label[1], values.configPtr.mplsCfg.label1)
                    end
                end
            end
        end

        if current_entry.cw_value ~= nil then
            -- get a Pseudo Wire control word
            apiName = "cpssDxChTunnelStartMplsPwControlWordGet"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",        "devNum",       current_entry.devNum },
                { "IN",     "GT_U32",       "index",        current_entry.cwIndex },
                { "OUT",     "GT_U32",       "valuePtr"}
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("control word", current_entry.cw_value, values.valuePtr)
        end

        -- get physical info for the given ePort
        apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet"
        result, values = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                   "devNum",          current_entry.devNum},
            { "IN",     "GT_PORT_NUM",             "portNum",         current_entry.eport},
            { "OUT",    "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr"},
        })
        _debug(apiName .. "   result = " .. result)

        check_expected_value("interface port type", "CPSS_INTERFACE_PORT_E" , values.physicalInfoPtr.type)

        -- get the TTI Rule Pattern, Mask and Action
        apiName = "cpssDxChTtiRuleGet"
        result, values = myGenWrapper(
            apiName ,{
            { "IN",     "GT_U8",                           "devNum",     current_entry.devNum },
            { "IN",     "GT_U32",                          "index",      current_entry.ttiIndex},
            { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",  "CPSS_DXCH_TTI_RULE_UDB_30_E"},
            { "OUT",    "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr"},
            { "OUT",    "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr"},
            { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
            { "OUT",    "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr"},
        })
        _debug(apiName .. "   result = " .. result)

        check_expected_value("eport", current_entry.assign_eport, values.actionPtr.type2.sourceEPort)

        j=6
        i=0
        while current_entry.ingress_label[i] ~= nil do
            ingress_label = 0
            ingress_label = bit_or(ingress_label,bit_shl(values.patternPtr.udbArray[j],16))
            ingress_label = bit_or(ingress_label,bit_shl(values.patternPtr.udbArray[j+1],8))
            ingress_label = bit_or(ingress_label,values.patternPtr.udbArray[j+2])
            ingress_label = bit_shr(ingress_label,4)

            check_expected_value("ingress label " .. i, current_entry.ingress_label[i], ingress_label)

            j=j+3
            i=i+1
        end

        iterator = iterator + 1

        current_index, current_entry = next(table_mpls_system_info,current_index)
    end
end



--************************************************************************
--  show_mpls_transit_test
--        @description  tests show mpls CLI command by comparing
--                      displayed values from CPSS and Lua database
--
function show_mpls_transit_test()
    local current_index
    local current_entry = {}
    local result, values
    local iterator = 1
    local hwDevNum
    local valuePtr = {}
    local maskPtr = {}
    local actionPtr
    local patternPtr = {}
    patternPtr.udbArray = {}
    local interfaceInfoPtr = {}
    local macSaAddrPtr = {}
    local valuePtr
    local configPtr = {}
    local physicalInfoPtr = {}
    configPtr.mplsCfg = {}
    local ingress_label
    local i,j
    local numberOfLabels = 0
    local arpMacAddrPtr = {}
    local entropyLabelsLocations = {0,0}

    table_mpls_transit_system_info = getTableMplsTransit()

    -- get the first element in the table
    current_index , current_entry = next(table_mpls_transit_system_info,nil)

    while current_entry do

        print("mpls transit entry " .. iterator)

        -- get a TTI MacToMe entry
        apiName = "cpssDxChTtiMacToMeGet"
        result, values = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                        "devNum",  current_entry.devNum },
            { "IN",     "GT_U32",                       "entryIndex",  current_entry.mac2meIndex },
            { "OUT",    "CPSS_DXCH_TTI_MAC_VLAN_STC",   "valuePtr"},
            { "OUT",    "CPSS_DXCH_TTI_MAC_VLAN_STC",   "maskPtr"},
            { "OUT",    "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC", "interfaceInfoPtr"}
        })
        _debug(apiName .. "   result = " .. result)

        check_expected_value("my mac address", current_entry.my_mac_addr.string, values.maskPtr.mac)
        check_expected_value("vlan Id", current_entry.vlan_id, values.valuePtr.vlanId)
        check_expected_value("port number", current_entry.portNum, values.interfaceInfoPtr.srcPortTrunk)

        apiName = "cpssDxChCfgHwDevNumGet"
        result, hwDevNum = device_to_hardware_format_convert(current_entry.devNum)
        _debug(apiName .. "   result = " .. result)

        check_expected_value("interface hwDevNum", hwDevNum, values.interfaceInfoPtr.srcDevice)

        if current_entry.cw_value ~= nil then
            -- get a Pseudo Wire control word
            apiName = "cpssDxChTunnelStartMplsPwControlWordGet"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",        "devNum",       current_entry.devNum },
                { "IN",     "GT_U32",       "index",        current_entry.cwIndex },
                { "OUT",    "GT_U32",       "valuePtr"}
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("control word", current_entry.cw_value, values.valuePtr)
        end

        if((current_entry.mpls_command=="CPSS_DXCH_TTI_MPLS_SWAP_CMD_E")or
           (current_entry.mpls_command=="CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E")or
           (current_entry.mpls_command=="CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E"))
            then  numberOfLabels = 1
        else
            numberOfLabels = 0 --POP1, POP2, POP3, NOP
        end

        if(numberOfLabels==0)then
            -- set a tunnel start entry
            apiName = "cpssDxChIpRouterArpAddrRead"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",         "devNum",         current_entry.devNum},
                { "IN",     "GT_U32",        "routerArpIndex", current_entry.arpIndex},
                { "OUT",     "GT_ETHERADDR",  "arpMacAddrPtr"},
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("next hop mac address", current_entry.nexthop_mac_da_addr.string, values.arpMacAddrPtr)
        else
            -- get a tunnel start entry
            apiName = "cpssDxChTunnelStartEntryGet"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                 "devNum",                        current_entry.devNum},
                { "IN",     "GT_U32",                "routerArpTunnelStartLineIndex", current_entry.tsIndex},
                { "OUT",     "CPSS_TUNNEL_TYPE_ENT",  "tunnelType"},
                { "OUT",     "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_mplsCfg", "configPtr"},
            })
            _debug(apiName .. "   result = " .. result)

            check_expected_value("next hop mac address", current_entry.nexthop_mac_da_addr.string, values.configPtr.mplsCfg.macDa)
            check_expected_value("mpls new label", current_entry.mpls_new_label, values.configPtr.mplsCfg.label1)
        end

       -- get physical info for the given ePort
        apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet"
        result, values = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                   "devNum",          current_entry.devNum},
            { "IN",     "GT_PORT_NUM",             "portNum",         current_entry.assign_eport},
            { "OUT",    "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr"},
        })
        _debug(apiName .. "   result = " .. result)

        check_expected_value("interface port type", "CPSS_INTERFACE_PORT_E" , values.physicalInfoPtr.type)

        -- get the TTI Rule Pattern, Mask and Action
        apiName = "cpssDxChTtiRuleGet"
        result, values = myGenWrapper(
            apiName ,{
            { "IN",     "GT_U8",                           "devNum",     current_entry.devNum },
            { "IN",     "GT_U32",                          "index",      current_entry.ttiIndex},
            { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",  "CPSS_DXCH_TTI_RULE_UDB_30_E"},
            { "OUT",    "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr"},
            { "OUT",    "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr"},
            { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
            { "OUT",    "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr"},
        })
        _debug(apiName .. "   result = " .. result)

        check_expected_value("eport", current_entry.assign_eport, values.actionPtr.type2.sourceEPort)
        check_expected_value("mpls command", current_entry.mpls_command, values.actionPtr.type2.mplsCommand)

        j=6
        i=0
        while current_entry.ingress_label[i] ~= nil do
            ingress_label = 0
            ingress_label = bit_or(ingress_label,bit_shl(values.patternPtr.udbArray[j],16))
            ingress_label = bit_or(ingress_label,bit_shl(values.patternPtr.udbArray[j+1],8))
            ingress_label = bit_or(ingress_label,values.patternPtr.udbArray[j+2])
            ingress_label = bit_shr(ingress_label,4)


            check_expected_value("ingress label" .. i, current_entry.ingress_label[i], ingress_label)

            if((j<18) and -- can not add entropy label after udb[18] since it is not supported more then 6 labels.
                ((entropyLabelsLocations[0]~=nill)and(entropyLabelsLocations[0]==(i)))or
                ((entropyLabelsLocations[1]~=nill)and(entropyLabelsLocations[1]==(i))))then
                j=j+9
            else
                j=j+3
            end
            i=i+1
        end

        if(values.actionPtr.type2.egressInterface.type == "CPSS_INTERFACE_TRUNK_E") then
            check_expected_value("egress port channel trunkId", current_entry.egress_interface_port_channel.trunkId, values.actionPtr.type2.egressInterface.trunkId)

            local i,v
            local res, vals = get_trunk_device_port_list(current_entry.egress_interface_port_channel.trunkId)
            if res == 0 and next(vals) ~= nil then
                for i,v in pairs(vals[0]) do

                    -- Gets full 48-bit Router MAC SA in Global MAC SA table
                    result, values = lua_cpssDxChIpRouterGlobalMacSaGet(devNum,v)
                    _debug(apiName .. "   result = " .. result)

                    check_expected_value("egress mac sa address", current_entry.egress_mac_sa_addr.string, values.macSaAddrPtr)


                    -- Get router mac sa index refered to global MAC SA table
                    apiName = "cpssDxChIpRouterPortGlobalMacSaIndexGet"
                    result, values = myGenWrapper(
                        apiName, {
                        { "IN",     "GT_U8",           "devNum",          devNum},
                        { "IN",     "GT_PORT_NUM",     "portNum",         v},
                        { "OUT",     "GT_U32",          "routerMacSaIndex"},
                    })
                    _debug(apiName .. "   result = " .. result)

                    check_expected_value("IpRouterPortGlobalMacSaIndex",v%256, values.routerMacSaIndex)
                end
            end

        else
            apiName = "cpssDxChCfgHwDevNumGet"
            result, hwDevNum = device_to_hardware_format_convert(current_entry.egress_interface_port_channel.devPort.devNum)
            _debug(apiName .. "   result = " .. result)

            check_expected_value("egress ethernet/eport devNum", hwDevNum, values.actionPtr.type2.egressInterface.devPort.devNum)
            check_expected_value("egress ethernet/eport portNum", current_entry.egress_interface_port_channel.devPort.portNum, values.actionPtr.type2.egressInterface.devPort.portNum)

            -- Gets full 48-bit Router MAC SA in Global MAC SA table
            result, values = lua_cpssDxChIpRouterGlobalMacSaGet(devNum,current_entry.egress_interface_port_channel.devPort.portNum)
            _debug(apiName .. "   result = " .. result)

            check_expected_value("egress mac sa address", current_entry.egress_mac_sa_addr.string, values.macSaAddrPtr)

            -- Get router mac sa index refered to global MAC SA table
           apiName = "cpssDxChIpRouterPortGlobalMacSaIndexGet"
           result, values = myGenWrapper(
               apiName, {
               { "IN",     "GT_U8",           "devNum",           devNum     },
               { "IN",     "GT_PORT_NUM",     "portNum",          current_entry.egress_interface_port_channel.devPort.portNum },
               { "OUT",     "GT_U32",          "routerMacSaIndex"},
           })
           _debug(apiName .. "   result = " .. result)

           check_expected_value("IpRouterPortGlobalMacSaIndex",current_entry.egress_interface_port_channel.devPort.portNum%256, values.routerMacSaIndex)

        end

        iterator = iterator + 1

        current_index , current_entry = next(table_mpls_transit_system_info,current_index)
    end
end

