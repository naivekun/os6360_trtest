--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_interface_egress_ha.lua
--*
--* DESCRIPTION:
--*       PIPE device : Port's Egress :
--*         1. header alteration table entry.
--*         2. source port info
--*         3. target port info
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--        printFromShow(555)
--[[ this file commands:
********* egress ha commands
CLI_addCommand("interface", "egress ha 802.1br-E2U", {
CLI_addCommand("interface", "egress ha 802.1br-U2E", {
CLI_addCommand("interface", "egress ha 802.1br-U2C", {
CLI_addCommand("interface", "egress ha 802.1br-C2U", {
CLI_addCommand("interface", "egress ha 802.1br-U2CPU", {
CLI_addCommand("interface", "egress ha 802.1br-CPU2U", {
CLI_addCommand("interface", "egress ha 802.1br-U2IPL", {
CLI_addCommand("interface", "egress ha 802.1br-IPL2IPL", {
CLI_addCommand("interface", "egress ha 802.1br-U2E_M4", {
CLI_addCommand("interface", "egress ha 802.1br-U2E_M8", {
CLI_addCommand("interface", "egress ha DSA-ET2U", {
CLI_addCommand("interface", "egress ha DSA-EU2U", {
CLI_addCommand("interface", "egress ha DSA-U2E", {
CLI_addCommand("interface", "egress ha DSA-Mrr2E", {
CLI_addCommand("interface", "egress ha DSA-CPU2U", {
CLI_addCommand("interface", "egress ha DSA-QCN", {
CLI_addCommand("interface", "egress ha do-not-modify", {
CLI_addCommand("interface", "no egress ha", {
********* egress source-port commands
CLI_addCommand("interface", "egress source-port 802.1br", {
CLI_addCommand("interface", "egress source-port DSA", {
CLI_addCommand("interface", "no egress source-port", {
********* egress target-port commands
CLI_addCommand("interface", "egress target-port common", {
CLI_addCommand("interface", "no egress target-port", {
********* show commands : egress ha
CLI_addCommand("exec", "show egress ha port-actionTypes", {
CLI_addCommand("exec", "show egress ha port-actionType-detailed", {
********* show commands : egress source port
CLI_addCommand("exec", "show egress source-port", {
********* show commands : egress target port
CLI_addCommand("exec", "show egress target-port types", {
CLI_addCommand("exec", "show egress target-port-detailed", {
********* show commands : egress internl-DB
CLI_addCommand("exec", "show egress internal-DB", {
]]--
--includes
cmdLuaCLI_registerCfunction("wrlCpssPxEgressSourcePortEntrySet")

--constants
local action_no_extra_info     =  "no-extra-info"

local action_802_1br_E2U       =  "802.1br-E2U"
local action_802_1br_E2U_Untagged =  "802.1br-E2U-Untagged"
local action_802_1br_U2E       =  "802.1br-U2E"
local action_802_1br_U2C       =  "802.1br-U2C"
local action_802_1br_C2U       =  "802.1br-C2U"
local action_802_1br_U2CPU     =  "802.1br-U2CPU"
local action_802_1br_CPU2U     =  "802.1br-CPU2U"
local action_802_1br_U2IPL     =  "802.1br-U2IPL"
local action_802_1br_IPL2IPL   =  "802.1br-IPL2IPL"
local action_802_1br_U2E_M4    =  "802.1br-U2E_M4"
local action_802_1br_U2E_M8    =  "802.1br-U2E_M8"
local action_802_1br_Drop      =  "802.1br-Drop"


local action_DSA_ET2U          =  "DSA-ET2U"
local action_DSA_EU2U          =  "DSA-EU2U"
local action_DSA_U2E           =  "DSA-U2E"
local action_DSA_Mrr2E         =  "DSA-Mrr2E"
local action_DSA_CPU2U         =  "DSA-CPU2U"
local action_DSA_QCN           =  "DSA-QCN"

local action_do_not_modify     =  "do-not-modify"


local source_port_type_DSA     = "DSA"
local source_port_type_802_1br = "802.1br"
local source_port_type_802_1br_untagged = "802.1br-Untagged"
local source_port_type_NA      = "NA"

local info_target_port_common  = "common"
local info_target_port_dsa_qcn  = "DSA-QCN"
local info_target_port_do_not_modify  = "do-not-modify"

-- return 'CLI' action type from cpss action type
local function get_cli_ha_action_type(actionType)
    for index,entry in pairs(CLI_type_dict["show_actionType"].enum) do
        if entry.value == actionType then
            return index
        end
    end
    -- not found
    return "ERROR : NOT FOUND"
end

-- return {actionType = ??,actionInfo = {??}}
-- call CPSS to get values
local function cpss_get_packetType_info(command_data,devNum,portNum,packetType)
    local did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
        "cpssPxEgressHeaderAlterationEntryGet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", packetType },
        { "OUT", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType"},
        { "OUT", "CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT", "operationInfo"}
        })

    if did_error then
        return nil --error indication
    end

    local actionType = get_cli_ha_action_type(OUT_values.operationType)

    return {actionType = actionType,
            actionInfo = OUT_values.operationInfo}
end
-- return {portType = ?? , portInfo = {??}}
-- call CPSS to get values
local function cpss_get_sourceInfo(command_data,devNum,portNum)
    command_data.dontStateErrorOnCpssFail = true
    local did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
        "cpssPxEgressSourcePortEntryGet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "OUT", "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT", "infoType"},
        { "OUT", "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT", "portInfo"}
        })
    --print("result" , to_string(result))
    if did_error then
        return nil --error indication
    end

    -- return 'CLI' prot type from cpss port type
    local function get_cli_source_port_type(portType)
        local source_port_db = {
            [source_port_type_802_1br] = "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E",
            [source_port_type_802_1br_untagged] = "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E",
            [source_port_type_DSA]     = "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E",
            [source_port_type_NA]      = "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E"
        }

        for index,entry in pairs(source_port_db) do
            if entry == portType then
                return index
            end
        end
        -- not found
        return "ERROR : NOT FOUND"
    end

    local portType = get_cli_source_port_type(OUT_values.infoType)


    return {portType = portType,
            portInfo = OUT_values.portInfo}
end

-- return 'CLI' prot type from cpss port type
local function get_cli_target_port_type(portType)
    for index,entry in pairs(CLI_type_dict["show_targetPortType"].enum) do
        if entry.value == portType then
            return index
        end
    end
    -- not found
    return "ERROR : NOT FOUND"
end

-- return {portType = ?? , portInfo = {??}}
-- call CPSS to get values
local function cpss_get_targetInfo(command_data,devNum,portNum)
    command_data.dontStateErrorOnCpssFail = true
    local did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
        "cpssPxEgressTargetPortEntryGet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "OUT", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT", "infoType"},
        { "OUT", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT", "portInfo"}
        })
    if did_error then
        return nil --error indication
    end

    local portType = get_cli_target_port_type(OUT_values.infoType)
    local retValue = {portType = portType,
            portInfo = OUT_values.portInfo}

    return retValue
end

--##################################
--##################################

-- implement per port : 'interface' --> interface mode command
local function per_port__egress_ha_802_1br_E2U(command_data, devNum, portNum, params)
    --print(to_string(params))

    local operationInfo = {
        eTag = {
            E_PCP = params.E_PCP,
            E_DEI = params.E_DEI,
            Ingress_E_CID_base = params.Ingress_E_CID_base,
            Direction = params.Direction,
            Upstream_Specific = params.Upstream_Specific,
            GRP = params.GRP,
            E_CID_base = params.E_CID_base,
            Ingress_E_CID_ext = params.Ingress_E_CID_ext,
            E_CID_ext = params.E_CID_ext
        }
    }

    if params.vlan_tag_TPID ~= nil then
        operationInfo.vlanTagTpid = params.vlan_tag_TPID
        operationType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E"
    else
        operationInfo.TPID  = params.TPID
        operationType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E"
    end

    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressHeaderAlterationEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", params.packetType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType", operationType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC", "operationInfoPtr", operationInfo }
    })

end
-- implement : 'interface' --> 802_1br-E2U command
local function interface_egress_ha_802_1br_E2U(params)
    -- set defaults
    if(params.TPID == nil) then params.TPID = "0x893f" end

    return generic_port_range_func(per_port__egress_ha_802_1br_E2U,params)
end

local help_string_egress_ha = "set egress ha info "
local help_string_egress_ha_802_1br_E2U = help_string_egress_ha .. "for 802_1br_E2U action"
local help_string_egress_ha_802_1br_E2U_Untagged = help_string_egress_ha .. "for 802_1br_E2U untagged action"
local help_packetType = "the packet type (that was assigned by ingress processing).(APPLICABLE RANGE: 0..31)"
local help_ha_info_not_needed = "the action not requires additional info."
local help_string_egress_ha_802_1br_drop = "the action drops the packet with specified packet type on specified target port"

CLI_addHelp("interface", "egress", "Set egress processing commands")
CLI_addHelp("interface", "egress ha", help_string_egress_ha)
CLI_addHelp("interface", "egress ha 802.1br-E2U", help_string_egress_ha_802_1br_E2U)
--[[
egress ha 802.1br-E2U packetType <number> etag-TPID<number> etag-E_PCP<number> etag-E_DEI<number>
                                          etag-Ingress_E_CID_base<number> etag-Direction <number> etag-Upstream_Specific<number>
                                          etag-GRP<number> etag-E_CID_base<number> etag-Ingress_E_CID_ext<number>
                                          etag-E_CID_ext<number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-E2U", {
    func = interface_egress_ha_802_1br_E2U,
    help = help_string_egress_ha_802_1br_E2U,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            {format="etag-1.TPID %GT_U32",    name="TPID", help = "ETAG: TPID (etherType) 16 bits field. (usually 0x893f)" },
            {format="etag-2.E_PCP %GT_U32",   name="E_PCP", help = "ETAG: Priority code point (PCP): the Iinterface 802.1p class of service. 3 bits field." },
            {format="etag-3.E_DEI %GT_U32",   name="E_DEI", help = "ETAG: Drop eligible indicator . single bit field." },
            {format="etag-4.Ingress_E_CID_base %GT_U32",name="Ingress_E_CID_base", help = "ETAG: this field used for Source filtering for multi-destination traffic is performed only on the PE Extender Ports. 12 bits field." },
            {format="etag-5.Direction %GT_U32",name="Direction", help = "ETAG: Direction. 1 bit field.(upstream =0, downstream = 1) " },
            {format="etag-6.Upstream_Specific %GT_U32",name="Upstream Specific", help = "ETAG: Upstream Specific. 1 bit field." },
            {format="etag-7.GRP %GT_U32",     name="GRP", help = "ETAG: indication of groups ranges. 2 bits field. value 1..3 means MC ranges. value 0 means UC range." },
            {format="etag-8.E_CID_base %GT_U32",name="E_CID_base", help = "ETAG: the source ECID on upstream , and the target ECID on UC downstream. on MC together with GRP hold 14 bits 'MC group' 12 bits field." },
            {format="etag-9.Ingress_E_CID_ext %GT_U32",name="Ingress_E_CID_ext", help = "ETAG: extension to Ingress_E_CID_base. 8 bits field." },
            {format="etag-10.E_CID_ext %GT_U32",name="E_CID_ext", help = "ETAG: extension to E_CID_base. 8 bits field." }
            ,
            mandatory = { "packetType"}
            }
   }
})

CLI_addHelp("interface", "egress ha 802.1br-E2U_Untagged", help_string_egress_ha_802_1br_E2U_Untagged)
--[[
egress ha 802.1br-E2U packetType <number> etag-TPID<number> etag-E_PCP<number> etag-E_DEI<number>
                                          etag-Ingress_E_CID_base<number> etag-Direction <number> etag-Upstream_Specific<number>
                                          etag-GRP<number> etag-E_CID_base<number> etag-Ingress_E_CID_ext<number>
                                          etag-E_CID_ext<number>  vlan_tag_TPID<number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-E2U-Untagged", {
    func = interface_egress_ha_802_1br_E2U,
    help = help_string_egress_ha_802_1br_E2U_Untagged,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            {format="etag-1.E_PCP %GT_U32",   name="E_PCP", help = "ETAG: Priority code point (PCP): the Iinterface 802.1p class of service. 3 bits field." },
            {format="etag-2.E_DEI %GT_U32",   name="E_DEI", help = "ETAG: Drop eligible indicator . single bit field." },
            {format="etag-3.Ingress_E_CID_base %GT_U32",name="Ingress_E_CID_base", help = "ETAG: this field used for Source filtering for multi-destination traffic is performed only on the PE Extender Ports. 12 bits field." },
            {format="etag-4.Direction %GT_U32",name="Direction", help = "ETAG: Direction. 1 bit field.(upstream =0, downstream = 1) " },
            {format="etag-5.Upstream_Specific %GT_U32",name="Upstream Specific", help = "ETAG: Upstream Specific. 1 bit field." },
            {format="etag-6.GRP %GT_U32",     name="GRP", help = "ETAG: indication of groups ranges. 2 bits field. value 1..3 means MC ranges. value 0 means UC range." },
            {format="etag-7.E_CID_base %GT_U32",name="E_CID_base", help = "ETAG: the source ECID on upstream , and the target ECID on UC downstream. on MC together with GRP hold 14 bits 'MC group' 12 bits field." },
            {format="etag-8.Ingress_E_CID_ext %GT_U32",name="Ingress_E_CID_ext", help = "ETAG: extension to Ingress_E_CID_base. 8 bits field." },
            {format="etag-9.E_CID_ext %GT_U32",name="E_CID_ext", help = "ETAG: extension to E_CID_base. 8 bits field." },
            {format="vlan_tag_TPID  %ethertype",   name="vlan_tag_TPID", help = "Vlan Tag TPID for untagged packet. 16-bits field in HEX (i.e. 0x8100, etc)" }
            ,
            mandatory = { "packetType", "vlan_tag_TPID"}
            }
   }
})


--##################################
--##################################

-- implement per port : 'interface' --> interface mode command
local function per_port__egress_ha_802_1br_Drop(command_data, devNum, portNum, params)
    --print(to_string(params))

    local operationInfo = {}
    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressHeaderAlterationEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", params.packetType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E" },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC", "operationInfoPtr", operationInfo }
    })

end

-- implement : 'interface' --> 802_1br-Drop command
local function interface_egress_ha_802_1br_Drop(params)
    return generic_port_range_func(per_port__egress_ha_802_1br_Drop, params)
end

--[[
egress ha 802.1br-Drop packetType <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-Drop", {
    func = interface_egress_ha_802_1br_Drop,
    help = help_string_egress_ha_802_1br_drop,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
             mandatory = { "packetType"}
        }
   }
})

--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__egress_ha_extra_param_not_Needed(command_data, devNum, portNum, params)
    --print(to_string(params))

    local operationInfo = {
        TPID  = params.not_needed
    }

    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressHeaderAlterationEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", params.packetType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType", params.actionType },
        { "IN", "CPSS_802_1BR_ETAG_STC", "notNeeded", operationInfo }
    })

end
-- implement : 'interface' --> extra param no needed command
local function interface_egress_ha_extra_param_not_Needed(params)

    return generic_port_range_func(per_port__egress_ha_extra_param_not_Needed,params)
end

local help_string_egress_ha_802_1br_U2E = help_string_egress_ha .. "for 802_1br_U2E action"
CLI_addHelp("interface", "egress ha 802.1br-U2E", help_string_egress_ha_802_1br_U2E)
--[[
egress ha 802.1br-U2E packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-U2E", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_802_1br_U2E,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
local help_string_egress_ha_802_1br_U2C = help_string_egress_ha .. "for 802_1br_U2C action"
CLI_addHelp("interface", "egress ha 802.1br-U2C", help_string_egress_ha_802_1br_U2C)
--[[
egress ha 802.1br-U2C packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-U2C", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_802_1br_U2C,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
local help_string_egress_ha_802_1br_C2U = help_string_egress_ha .. "for 802_1br_C2U action"
CLI_addHelp("interface", "egress ha 802.1br-C2U", help_string_egress_ha_802_1br_C2U)
--[[
egress ha 802.1br-C2U packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-C2U", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_802_1br_C2U,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
local help_string_egress_ha_802_1br_U2CPU = help_string_egress_ha .. "for 802_1br_U2CPU action"
CLI_addHelp("interface", "egress ha 802.1br-U2CPU", help_string_egress_ha_802_1br_U2CPU)
--[[
egress ha 802.1br-U2CPU packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-U2CPU", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_802_1br_U2CPU,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
local help_string_egress_ha_802_1br_CPU2U = help_string_egress_ha .. "for 802_1br_CPU2U action"
CLI_addHelp("interface", "egress ha 802.1br-CPU2U", help_string_egress_ha_802_1br_CPU2U)
--[[
egress ha 802.1br-CPU2U packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-CPU2U", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_802_1br_CPU2U,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
local help_string_egress_ha_802_1br_U2IPL = help_string_egress_ha .. "for 802_1br_U2IPL action"
CLI_addHelp("interface", "egress ha 802.1br-U2IPL", help_string_egress_ha_802_1br_U2IPL)
--[[
egress ha 802.1br-U2IPL packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-U2IPL", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_802_1br_U2IPL,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
local help_string_egress_ha_802_1br_IPL2IPL = help_string_egress_ha .. "for 802_1br_IPL2IPL action"
CLI_addHelp("interface", "egress ha 802.1br-IPL2IPL", help_string_egress_ha_802_1br_IPL2IPL)
--[[
egress ha 802.1br-IPL2IPL packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-IPL2IPL", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_802_1br_IPL2IPL,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--##################################
--##################################

-- implement per port : 'interface' --> interface mode command
local function per_port__egress_ha_802_1br_U2E_M4(command_data, devNum, portNum, params)
    --print(to_string(params))
    local operationInfo = {
        pcid = {
            [0] = params.PCID1,
            [1] = params.PCID2,
            [2] = params.PCID3
        }
    }
    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressHeaderAlterationEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", params.packetType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E" },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC", "operationInfoPtr", operationInfo }
    })

end
-- implement : 'interface' --> 802_1br-U2E_M4 command
local function interface_egress_ha_802_1br_U2E_M4(params)

    return generic_port_range_func(per_port__egress_ha_802_1br_U2E_M4,params)
end

--######################################################
--######################################################
local help_string_egress_ha_802_1br_U2E_M4 = help_string_egress_ha .. "for 802_1br_U2E_M4 action"
CLI_addHelp("interface", "egress ha 802.1br-U2E_M4", help_string_egress_ha_802_1br_U2E_M4)
--[[
egress ha 802.1br-U2E_M4 packetType <number> PCID1 <number PCID2<number PCID3 <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-U2E_M4", {
    func = interface_egress_ha_802_1br_U2E_M4,
    help = help_string_egress_ha_802_1br_U2E_M4,
    params={
     { type="named",
         {format="packetType %GT_U32",   name="packetType", help = help_packetType },
         {format="PCID1 %GT_U32",   name="PCID1", help = "PCID1 12 bits field" },
         {format="PCID2 %GT_U32",   name="PCID2", help = "PCID2 12 bits field" },
         {format="PCID3 %GT_U32",   name="PCID3", help = "PCID3 12 bits field" }
         ,
         mandatory = { "packetType","PCID1","PCID2","PCID3"}
         },
    }

})
--##################################
--##################################

-- implement per port : 'interface' --> interface mode command
local function per_port__egress_ha_802_1br_U2E_M8(command_data, devNum, portNum, params)

    --print(to_string(params))

    local operationInfo = {

        pcid = {
            [0] = params.PCID1,
            [1] = params.PCID2,
            [2] = params.PCID3,
            [3] = params.PCID4,
            [4] = params.PCID5,
            [5] = params.PCID6,
            [6] = params.PCID7
        }
    }
    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressHeaderAlterationEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", params.packetType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E" },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC", "operationInfoPtr", operationInfo }
    })

end
-- implement : 'interface' --> 802_1br-U2E_M8 command
local function interface_egress_ha_802_1br_U2E_M8(params)

    return generic_port_range_func(per_port__egress_ha_802_1br_U2E_M8,params)
end

--######################################################
--######################################################
local help_string_egress_ha_802_1br_U2E_M8 = help_string_egress_ha .. "for 802_1br_U2E_M8 action"
CLI_addHelp("interface", "egress ha 802.1br-U2E_M8", help_string_egress_ha_802_1br_U2E_M8)
--[[
egress ha 802.1br-U2E_M8 packetType <number> PCID1 <number PCID2<number PCID3 <number> PCID4 <number PCID5<number PCID6 <number> PCID7 <number>
]]--
CLI_addCommand("interface", "egress ha 802.1br-U2E_M8", {
    func = interface_egress_ha_802_1br_U2E_M8,
    help = help_string_egress_ha_802_1br_U2E_M8,
    params={
     { type="named",
         {format="packetType %GT_U32",   name="packetType", help = help_packetType },
         {format="PCID1 %GT_U32",   name="PCID1", help = "PCID1 12 bits field" },
         {format="PCID2 %GT_U32",   name="PCID2", help = "PCID2 12 bits field" },
         {format="PCID3 %GT_U32",   name="PCID3", help = "PCID3 12 bits field" },
         {format="PCID4 %GT_U32",   name="PCID4", help = "PCID4 12 bits field" },
         {format="PCID5 %GT_U32",   name="PCID5", help = "PCID5 12 bits field" },
         {format="PCID6 %GT_U32",   name="PCID6", help = "PCID6 12 bits field" },
         {format="PCID7 %GT_U32",   name="PCID7", help = "PCID7 12 bits field" }
         ,
         mandatory = { "packetType","PCID1","PCID2","PCID3","PCID4","PCID5","PCID6","PCID7"}
         },
    }

})


--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__egress_ha_DSA_E2U(command_data, devNum, portNum, params)
    --print(to_string(params))

    local operationInfo = {
        dsaForward = {
        srcTagged = params.srcTagged,
        hwSrcDev = params.hwSrcDev,
        srcPortOrTrunk = params.srcPortOrTrunk,
        srcIsTrunk = params.srcIsTrunk,
        cfi = params.cfi,
        up = params.up,
        vid = params.vid
        }
    }

    -- trick : call the 'CPSS_PX_REGULAR_DSA_FORWARD_STC' member that common for :
    -- DSA-ET2U and DSA-EU2U

    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressHeaderAlterationEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", params.packetType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType", params.actionType },
        { "IN", "CPSS_PX_REGULAR_DSA_FORWARD_STC", "operationInfoPtr", operationInfo.dsaForward }
    })

end

-- implement : 'interface' --> DSA-ET2U and DSA-EU2U command
local function interface_egress_ha_DSA_E2U(params)

    -- set defaults

    return generic_port_range_func(per_port__egress_ha_DSA_E2U,params)
end


local help_string_egress_ha_DSA_ET2U = help_string_egress_ha .. "for DSA-ET2U action"
CLI_addHelp("interface", "egress ha DSA-ET2U", help_string_egress_ha_DSA_ET2U)
--[[
egress ha DSA-ET2U packetType <number> dsa-1.srcTagged <bool>  dsa-2.hwSrcDev <number> dsa-3.srcPortOrTrunk <number>
    dsa-4.srcIsTrunk <bool> dsa-5.cfi <number> dsa-6.up <number> dsa-7.vid <number>

]]--
CLI_addCommand("interface", "egress ha DSA-ET2U", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E"
                return interface_egress_ha_DSA_E2U(params)
           end
           ,
    help = help_string_egress_ha_DSA_ET2U,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            {format="dsa-1.srcTagged %bool" ,   name="srcTagged",  help = "source packet received tagged." },
            {format="dsa-2.hwSrcDev %GT_U32" ,   name="hwSrcDev",  help = "HW source device.(APPLICABLE RANGE: 0..31)" },
            {format="dsa-3.srcPortOrTrunk %GT_U32" ,   name="srcPortOrTrunk",  help = "source port/trunkId.(APPLICABLE RANGE: 0..31)" },
            {format="dsa-4.srcIsTrunk %bool" ,   name="srcIsTrunk",  help = "indication the packet was received from trunk" },
            {format="dsa-5.cfi %GT_U32" ,   name="cfi",  help = "Drop eligible indicator (was Canonical Format Indicator).(APPLICABLE RANGE: 0..1)" },
            {format="dsa-6.up %GT_U32" ,   name="up",  help = "user priority (up): the IEEE 802.1p class of service.(APPLICABLE RANGE: 0..7)" },
            {format="dsa-7.vid %GT_U32" ,   name="vid",  help = "VLAN identifier ,specifying the VLAN to which the frame belongs.(APPLICABLE RANGE: 0..4095)" }
            ,
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
local help_string_egress_ha_DSA_EU2U = help_string_egress_ha .. "for DSA-EU2U action"
CLI_addHelp("interface", "egress ha DSA-EU2U", help_string_egress_ha_DSA_EU2U)
--[[
egress ha DSA-EU2U packetType <number> dsa-1.srcTagged <bool>  dsa-2.hwSrcDev <number> dsa-3.srcPortOrTrunk <number>
    dsa-4.srcIsTrunk <bool> dsa-5.cfi <number> dsa-6.up <number> dsa-7.vid <number>

]]--
CLI_addCommand("interface", "egress ha DSA-EU2U", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E"
                return interface_egress_ha_DSA_E2U(params)
           end
           ,
    help = help_string_egress_ha_DSA_EU2U,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            {format="dsa-1.srcTagged %bool" ,   name="srcTagged",  help = "source packet received tagged." },
            {format="dsa-2.hwSrcDev %GT_U32" ,   name="hwSrcDev",  help = "HW source device.(APPLICABLE RANGE: 0..31)" },
            {format="dsa-3.srcPortOrTrunk %GT_U32" ,   name="srcPortOrTrunk",  help = "source port/trunkId.(APPLICABLE RANGE: 0..31)" },
            {format="dsa-4.srcIsTrunk %bool" ,   name="srcIsTrunk",  help = "indication the packet was received from trunk" },
            {format="dsa-5.cfi %GT_U32" ,   name="cfi",  help = "Drop eligible indicator (was Canonical Format Indicator).(APPLICABLE RANGE: 0..1)" },
            {format="dsa-6.up %GT_U32" ,   name="up",  help = "user priority (up): the IEEE 802.1p class of service.(APPLICABLE RANGE: 0..7)" },
            {format="dsa-7.vid %GT_U32" ,   name="vid",  help = "VLAN identifier ,specifying the VLAN to which the frame belongs.(APPLICABLE RANGE: 0..4095)" }
            ,
            mandatory = { "packetType"}
            }
   }
})
--######################################################
--######################################################
local help_string_egress_ha_DSA_U2E = help_string_egress_ha .. "for DSA-U2E action"
CLI_addHelp("interface", "egress ha DSA-U2E", help_string_egress_ha_DSA_U2E)
--[[
egress ha DSA-U2E packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha DSA-U2E", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_DSA_U2E,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})
--######################################################
--######################################################
local help_string_egress_ha_DSA_Mrr2E = help_string_egress_ha .. "for DSA-Mrr2E action"
CLI_addHelp("interface", "egress ha DSA-Mrr2E", help_string_egress_ha_DSA_Mrr2E)
--[[
egress ha DSA-Mrr2E packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha DSA-Mrr2E", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_DSA_Mrr2E,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})
--######################################################
--######################################################
local help_string_egress_ha_DSA_CPU2U = help_string_egress_ha .. "for DSA-CPU2U action"
CLI_addHelp("interface", "egress ha DSA-CPU2U", help_string_egress_ha_DSA_CPU2U)
--[[
egress ha DSA-CPU2U packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "egress ha DSA-CPU2U", {
    func = function (params)
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_egress_ha_DSA_CPU2U,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})
--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__egress_ha_DSA_QCN(command_data, devNum, portNum, params)
    --print(to_string(params))

    local operationInfo = {
        macSa = params.macSa,

        dsaExtForward = {
            srcTagged = params.srcTagged,
            hwSrcDev = params.hwSrcDev,
            srcPortOrTrunk = params.srcPortOrTrunk,
            srcIsTrunk = params.srcIsTrunk,
            cfi = params.cfi,
            up = params.up,
            vid = params.vid,

            egrFilterRegistered = params.egrFilterRegistered,
            dropOnSource = params.dropOnSource,
            packetIsLooped = params.packetIsLooped,
            wasRouted = params.wasRouted,
            srcId = params.srcId,
            qosProfileIndex = params.qosProfileIndex,

            useVidx = params.useVidx,
            trgVidx = params.trgVidx,
            trgPort = params.trgPort,
            hwTrgDev = hwTrgDev
        },

        cnmTpid = params.cnmTpid
    }

    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressHeaderAlterationEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "GT_U32", "packetType", params.packetType },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT", "operationType", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E" },
        { "IN", "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC", "operationInfoPtr", operationInfo }
    })

end

-- implement : 'interface' --> DSA-ET2U and DSA-EU2U command
local function interface_egress_ha_DSA_QCN(params)

    -- set defaults

    return generic_port_range_func(per_port__egress_ha_DSA_QCN,params)
end

local help_string_egress_ha_DSA_QCN = help_string_egress_ha .. "for DSA-QCN action"
CLI_addHelp("interface", "egress ha DSA-QCN", help_string_egress_ha_DSA_QCN)
--[[
egress ha DSA-QCN packetType <number> mac-Sa <mac-address> dsa-1.srcTagged <bool>  dsa-2.hwSrcDev <number> dsa-3.srcPortOrTrunk <number>
    dsa-4.srcIsTrunk <bool> dsa-5.cfi <number> dsa-6.up <number> dsa-7.vid <number>

]]--
CLI_addCommand("interface", "egress ha DSA-QCN", {
    func = interface_egress_ha_DSA_QCN,
    help = help_string_egress_ha_DSA_QCN,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },

            {format="mac-Sa %mac-address",   name="macSa", help = "the SA mac address." },

            {format="dsaExt-01.srcTagged %bool" ,   name="srcTagged",  help = "source packet received tagged." },
            {format="dsaExt-02.hwSrcDev %GT_U32" ,   name="hwSrcDev",  help = "HW source device.(APPLICABLE RANGE: 0..31)" },
            {format="dsaExt-03.srcPortOrTrunk %GT_U32" ,   name="srcPortOrTrunk",  help = "source port/trunkId.(APPLICABLE RANGE: 0..127)" },
            {format="dsaExt-04.srcIsTrunk %bool" ,   name="srcIsTrunk",  help = "indication the packet was received from trunk" },
            {format="dsaExt-05.cfi %GT_U32" ,   name="cfi",  help = "Drop eligible indicator (was Canonical Format Indicator).(APPLICABLE RANGE: 0..1)" },
            {format="dsaExt-06.up %GT_U32" ,   name="up",  help = "user priority (up): the IEEE 802.1p class of service.(APPLICABLE RANGE: 0..7)" },
            {format="dsaExt-07.vid %GT_U32" ,   name="vid",  help = "VLAN identifier ,specifying the VLAN to which the frame belongs.(APPLICABLE RANGE: 0..4095)" },

            {format="dsaExt-08.egrFilterRegistered %bool" ,   name="egrFilterRegistered",  help = "Indicates that the packet is Egress filtered as a registered packet" },
            {format="dsaExt-09.dropOnSource %bool" ,   name="dropOnSource",  help = "Drop packet on its source device indicator, for Fast Stack Fail over Recovery Support." },
            {format="dsaExt-10.packetIsLooped %bool" ,   name="packetIsLooped",  help = "Packet is looped indicator, for Fast Stack Fail over Recovery Support." },
            {format="dsaExt-11.wasRouted %bool" ,   name="wasRouted",  help = "Indicates whether the packet is routed." },
            {format="dsaExt-12.srcId %GT_U32" ,   name="srcId",  help = "Packet's Source ID (APPLICABLE RANGE: 0..31)" },
            {format="dsaExt-13.qosProfileIndex %GT_U32" ,   name="qosProfileIndex",  help = "Packet's QoS Profile (APPLICABLE RANGE: 0..127)" },
            {format="dsaExt-14.useVidx %bool" ,   name="useVidx",  help = "indication to use vidx or {hwTrgDev,trgPort}" },
            {format="dsaExt-15.trgVidx %GT_U32" ,   name="trgVidx",  help = "the target vidx represent mutli-destination domain. NOTE: relevant when useVidx == GT_TRUE (APPLICABLE RANGE: 0..4095)" },
            {format="dsaExt-16.trgPort %GT_U32" ,   name="trgPort",  help = "the target port. NOTE: relevant when useVidx == GT_FALSE (APPLICABLE RANGE: 0..36)" },
            {format="dsaExt-17.hwTrgDev %GT_U32" ,   name="hwTrgDev",  help = "the target HW device. NOTE: relevant when useVidx == GT_FALSE (APPLICABLE RANGE: 0..36)" },

            {format="cnmTpid %GT_U16",   name="cnmTpid", help = "the CNM TPID (etherType). (APPLICABLE RANGE: 0..0xFFFF)" }
            ,

            mandatory = { "packetType" ,"macSa" ,"cnmTpid"}
            }
   }
})
--######################################################
--######################################################
local help_string_egress_ha_do_not_modify = help_string_egress_ha .. "for do-not-modify action"
    CLI_addHelp("interface", "egress ha do-not-modify", help_string_egress_ha_do_not_modify)
    --[[
    egress ha do-not-modify packetType <number> not-needed <number>
    ]]--
    CLI_addCommand("interface", "egress ha do-not-modify", {
        func = function (params)
                    params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E"
                    return interface_egress_ha_extra_param_not_Needed(params)
               end
               ,
        help = help_string_egress_ha_do_not_modify,
        params={
            { type="named",
                {format="packetType %GT_U32",   name="packetType", help = help_packetType },
                {format="not-needed %GT_U32" ,   name="not_needed",  help = help_ha_info_not_needed }
                ,
                mandatory = { "packetType"}
                }
       }
    })

--######################################################
--######################################################
local help_string_no_egress_ha = "Restore default egress header alteration for given packet number"
CLI_addHelp("interface", "no egress ha", help_string_no_egress_ha)
--[[
no egress ha packetType <number> not-needed <number>
]]--
CLI_addCommand("interface", "no egress ha", {
    func = function (params)
                params.not_needed = 0
                params.actionType = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E"
                return interface_egress_ha_extra_param_not_Needed(params)
           end
           ,
    help = help_string_no_egress_ha,
    params={
        { type="named",
            {format="packetType %GT_U32",   name="packetType", help = help_packetType },
            mandatory = { "packetType"}
            }
   }
})

--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__source_port_802_1br(command_data, devNum, portNum, params)
    --print(to_string(params))

    if params.untagged == nil then
        local portInfo = {
            pcid = params.pcid,
            srcPortInfo = {srcFilteringVector = params.srcFilteringVector},
            upstreamPort = params.upstreamPort
        }

        local status = wrlCpssPxEgressSourcePortEntrySet(devNum, portNum, "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E", portInfo)
    else
        local portInfo = {
            pcid = params.pcid,
            srcPortInfo = {vlanTag = {vid = params.vid, pcp = 0, dei = 0}},
            upstreamPort = 0
        }
        if params.pcp then
            portInfo.srcPortInfo.vlanTag.pcp = params.pcp
        end
        if params.dei then
            portInfo.srcPortInfo.vlanTag.dei = params.dei
        end

        local status = wrlCpssPxEgressSourcePortEntrySet(devNum, portNum, "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E", portInfo)
    end

end

-- implement : 'interface' --> source_port_802_1br command
local function interface_egress_source_port_802_1br(params)

    -- set defaults

    return generic_port_range_func(per_port__source_port_802_1br,params)
end

local help_string_egress_source_port = "set egress source-port info "
local help_string_egress_source_port_802_1br = help_string_egress_source_port .. "for 802.1br case"
CLI_addHelp("interface", "egress source-port", help_string_egress_source_port)
CLI_addHelp("interface", "egress source-port 802.1br", help_string_egress_source_port_802_1br)
--[[
egress source-port 802.1br pcid <number> srcFilteringVector <number>
]]--
CLI_addCommand("interface", "egress source-port 802.1br", {
    func = interface_egress_source_port_802_1br,
    help = help_string_egress_source_port_802_1br,
    params={
        { type="named",
            {format="pcid %GT_U32",   name="pcid", help = "the PCID (802.1br Port Channel ID) of the port (APPLICABLE RANGE: 0..4095)" },
            {format="srcFilteringVector %GT_U32" ,   name="srcFilteringVector",  help = "port bitmap for port filtering mandatory for IPL2IPL only otherwise = 0" },
            {format="upstreamPort %bool" ,   name="upstreamPort",  help = "hold the indication that the source port is an Upstream port for 802.1br-E2U" }
            ,
            mandatory = { "pcid","srcFilteringVector", "upstreamPort"}
            }
   }
})


local help_string_egress_source_port_802_1br_untagged = help_string_egress_source_port .. "for 802.1br Untagged packets"
CLI_addHelp("interface", "egress source-port 802.1br-Untagged", help_string_egress_source_port_802_1br_untagged)
--[[
egress source-port 802.1br-Untagged
]]--
CLI_addCommand("interface", "egress source-port 802.1br-Untagged", {
    func = function (params)
                params.untagged = true
                return interface_egress_source_port_802_1br(params)
           end,
    help = help_string_egress_source_port_802_1br_untagged,
    params={
        { type="named",
            {format="pcid %GT_U32",   name="pcid", help = "the PCID (802.1br Port Channel ID) of the port (APPLICABLE RANGE: 0..4095)" },
            {format="vid %GT_U32" ,   name="vid",  help = "VLAN ID of the VLAN tag added to a packet (1-4095)" },
            {format="pcp %GT_U32" ,   name="pcp",  help = "VLAN Priority Code Point (PCP) of the VLAN tag added to a packet. (0-7)" },
            {format="dei %GT_U32" ,   name="dei",  help = "Drop eligible indicator of the VLAN tag added to a packet. (0-1)" }
            ,
            mandatory = { "pcid","vid"}
        }
   }
})

--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__source_port_DSA(command_data, devNum, portNum, params)
    --print(to_string(params))

    local portInfo = {
        srcPortNum = params.srcPortNum
    }

    local status = wrlCpssPxEgressSourcePortEntrySet(devNum, portNum,
                                                     "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E",
                                                     portInfo)
end

-- implement : 'interface' --> source_port_DSA command
local function interface_egress_source_port_DSA(params)

    -- set defaults

    return generic_port_range_func(per_port__source_port_DSA,params)
end

local help_string_egress_source_port_DSA = help_string_egress_source_port .. "for DSA case"
CLI_addHelp("interface", "egress source-port DSA", help_string_egress_source_port_DSA)
--[[
egress source-port DSA srcPortNum <number>
]]--
CLI_addCommand("interface", "egress source-port DSA", {
    func = interface_egress_source_port_DSA,
    help = help_string_egress_source_port_DSA,
    params={
        { type="named",
            {format="srcPortNum %GT_U32" ,   name="srcPortNum",  help = "the source port number (that represent the entry) (APPLICABLE RANGE: 0..15) " }
            ,
            mandatory = { "srcPortNum"}
            }
   }
})

--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__source_port_not_needed(command_data, devNum, portNum, params)
    --print(to_string(params))

    local portInfo = {
        pcid = 0,
    }
    local status = wrlCpssPxEgressSourcePortEntrySet(devNum, portNum,
                                                     "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E",
                                                     portInfo)
end

-- implement : 'interface' --> source_port_not_needed command
local function interface_egress_source_port_not_needed(params)

    -- set defaults

    return generic_port_range_func(per_port__source_port_not_needed,params)
end

local help_port_info_not_needed = "dummy field for types that no extra info needed"
local help_string_no_egress_source_port = "Restore default egress source-port info"
CLI_addHelp("interface", "no egress source-port", help_string_no_egress_source_port)
--[[
no egress source-port <number>
]]--
CLI_addCommand("interface", "no egress source-port", {
    func = interface_egress_source_port_not_needed,
    help = help_string_no_egress_source_port,
    params={}
})

--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__target_port_common(command_data, devNum, portNum, params)
    --print(to_string(params))

    local portInfo = {
        tpid = params.tpid,
        pcid = params.pcid,
        egressDelay = params.egressDelay
    }

    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressTargetPortEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT", "infoType", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E" },
        { "IN", "CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC", "portInfoPtr", portInfo }
    })

end

-- implement : 'interface' --> target_port_common command
local function interface_egress_target_port_common(params)

    -- set defaults

    return generic_port_range_func(per_port__target_port_common,params)
end

local help_string_egress_target_port = "set egress target-port info "
local help_string_egress_target_port_common = help_string_egress_target_port .. "for common case"
CLI_addHelp("interface", "egress target-port", help_string_egress_target_port)
CLI_addHelp("interface", "egress target-port common", help_string_egress_target_port_common)
--[[
egress target-port common tpid <number> pcid <number> egressDelay <number>
]]--
CLI_addCommand("interface", "egress target-port common", {
    func = interface_egress_target_port_common,
    help = help_string_egress_target_port_common,
    params={
        { type="named",
            {format="1.tpid %GT_U16",   name="tpid", help = "TPID (etherType) (APPLICABLE RANGE: 0..0xFFFF)" },
            {format="2.pcid %GT_U32",   name="pcid", help = "the PCID (802.1br Port Channel ID) of the port. (APPLICABLE RANGE: 0..4095)" },
            {format="3.egressDelay %GT_U32" ,   name="egressDelay",  help = "The egress delay in nanoseconds, from the timestamping point until the actual interface. (APPLICABLE RANGE: 0..0xFFFFF)" }
            ,
            mandatory = { } -- no mandatory
            }
   }
})

--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__target_port_not_needed(command_data, devNum, portNum, params)
    local portInfo = {
        tpid = 0
    }

    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressTargetPortEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT", "infoType", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E" },
        { "IN", "CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC", "portInfoPtr", portInfo }
    })

end

-- implement : 'interface' --> target_port_not_needed command
local function interface_egress_target_port_not_needed(params)

    -- set defaults

    return generic_port_range_func(per_port__target_port_not_needed,params)
end

local help_string_no_egress_target_port = "Restore default egress target-port info"
CLI_addHelp("interface", "no egress target-port", help_string_no_egress_target_port)
--[[
no egress target-port <number>
]]--
CLI_addCommand("interface", "no egress target-port", {
    func = interface_egress_target_port_not_needed,
    help = help_string_no_egress_target_port,
    params={}
})
--######################################################
--######################################################
-- implement per port : 'interface' --> interface mode command
local function per_port__interface_egress_target_port_dsa_qcn(command_data, devNum, portNum, params)
    --print(to_string(params))

    local portInfo = {
        tpid = params.not_needed
    }

    genericCpssApiWithErrorHandler(command_data,"cpssPxEgressTargetPortEntrySet", {
        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
        { "IN", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT", "infoType", "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E" },
        { "IN", "CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC", "portInfoPtr", portInfo }
    })

end

-- implement : 'interface' --> target_port_dsa_qcn command
local function interface_egress_target_port_dsa_qcn(params)

    -- set defaults

    return generic_port_range_func(per_port__interface_egress_target_port_dsa_qcn,params)
end

local help_string_egress_target_port_dsa_qcn = help_string_egress_target_port .. "for DSA-QCN case"
CLI_addHelp("interface", "egress target-port DSA-QCN", help_string_egress_target_port_dsa_qcn)
--[[
egress target-port not-needed not-needed <number>
]]--
CLI_addCommand("interface", "egress target-port DSA-QCN", {
    func = interface_egress_target_port_dsa_qcn,
    help = help_string_egress_target_port_dsa_qcn,
    params={
      { type="values",
            {format="%GT_U32",   name="not_needed", help = help_port_info_not_needed }
      }
   }
})



--##################################
--##################################
-- the packetType is 0..31
local max_packetType = 31
-- the show support 'packetType' as 'all' (meaning --> 0..31)
local function CLI_check_param_packetType____all(param,name,desc,varray,params)
    if param == "all" then
        --filled by command
        return true,{"all"}
    end
    return false,""
end
local function CLI_complete_param_packetType____all(param,name,desc,varray,params)
    if params.packetType == "all" then
        return {"all" , "all values (0.." .. tostring(max_packetType) .. ")"}
    end
    return {},{}
end
-- the show support 'packetType' as range of numbers
local function CLI_check_param_packetType_number(param,name,desc,varray,params)
    local my_dictionary = {min = 0 , max = max_packetType}
    return CLI_check_param_number_range(param,name,my_dictionary,varray,params)
end
local function CLI_complete_param_packetType_number(param,name,desc,varray,params)
    local my_dictionary = {min = 0 , max = max_packetType}
    local ret1,ret2 =  CLI_complete_param_number_range(param,name,my_dictionary,varray,params)
    return ret1,ret2
end

--##################################
--##################################

local function table_length(T)
  local count = 0
  for dummy in pairs(T) do count = count + 1 end
  return count
end

local function per_port__show_egress_ha_port_packetType_actionType(command_data, devNum, portNum, params)
    local packetType_list = params.packetType -- list of range/single or {"all"}

    if(packetType_list == nil) then packetType_list = "all" end

    if packetType_list == "all" then
        packetType_list = {}
        for index = 0, max_packetType do
            table.insert(packetType_list,index)
        end
    end

    --print("22222222222222222222")



    --print(to_string(params))
    --print("packetType_list" , to_string(packetType_list))

    -- Resulting string formatting and adding.
    local devPortStr = tostring(devNum).."/"..tostring(portNum)

    for index,packetType in pairs(packetType_list) do
        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end

        command_data["result"] =
            string.format("%-11s %-28d %-18s", devPortStr, packetType , to_string(cpssInfo.actionType))

        command_data:addResultToResultArray()
    end
end

local function show_egress_ha_port_packetType_actionType(params)
    local header_string, footer_string

    --print("1111111111111111111111",to_string(params))

    header_string =
        "\n" ..
        "Dev/Port   packetType     action type   \n" ..
        "---------  ------------  -------------  \n"
    footer_string = "\n"

    params.header_string = header_string
    params.footer_string = footer_string

    return generic_all_ports_show_func(per_port__show_egress_ha_port_packetType_actionType,params)
end

CLI_addHelp("exec", "show egress", "Show the egress info")
CLI_addHelp("exec", "show egress ha", "Show the egress ha info")
local help_string_show_egress_ha_action_types = "Show the egress ha actions (per port(s) , per packetType(s))."
CLI_addHelp("exec", "show egress ha port-actionTypes", help_string_show_egress_ha_action_types)
local help_show_packetType = "Use single/range/'all' for packetType(s) (range 0..31)."
-- special dictionary for show commands for the 'packetType' parameter
CLI_type_dict["show_packetType"] = {
    checker  = {CLI_check_param_packetType_number     , CLI_check_param_packetType____all},
    complete = {CLI_complete_param_packetType_number  , CLI_complete_param_packetType____all },
    help= help_show_packetType --[[actually set in runtime by proper 'complete' function]]
}

--[[
show egress ha port-actionTypes <all/device/ethernet> [packetType <all/range(s) in 0..31>]
]]--
CLI_addCommand("exec", "show egress ha port-actionTypes", {
  func   = show_egress_ha_port_packetType_actionType,
  help   = help_string_show_egress_ha_action_types,
  params = {
    { type = "named",

        "#all_interfaces_ver1",
        {format="packetType %show_packetType",   name="packetType", help = "(optional)" .. help_show_packetType .. "default is 'all'" },

        requirements = { packetType = { "all_interfaces" } },
        mandatory = { "all_interfaces" }
    }}
})
--##################################
--##################################

local help_show_actionType = "Show info about specific actionType."

local help_actionType_no_extra_info = "actions without extra info"
local help_802_1br_E2U     = "info about 802.1br-E2U"
local help_802_1br_E2U_Untagged     = "info about 802.1br-E2U-Untagged"
local help_802_1br_U2E     = "info about 802.1br-U2E"
local help_802_1br_U2C     = "info about 802.1br-U2C"
local help_802_1br_C2U     = "info about 802.1br-C2U"
local help_802_1br_U2CPU   = "info about 802.1br-U2CPU"
local help_802_1br_CPU2U   = "info about 802.1br-CPU2U"
local help_802_1br_U2IPL   = "info about 802.1br-U2IPL"
local help_802_1br_IPL2IPL = "info about 802.1br-IPL2IPL"
local help_802_1br_U2E_M4  = "info about 802.1br-U2E_M4"
local help_802_1br_U2E_M8  = "info about 802.1br-U2E_M8"
local help_802_1br_Drop    = "info about 802.1br-Drop"

local help_DSA_ET2U        = "info about DSA-ET2U"
local help_DSA_EU2U        = "info about DSA-EU2U"
local help_DSA_U2E         = "info about DSA-U2E"
local help_DSA_Mrr2E       = "info about DSA-Mrr2E"
local help_DSA_CPU2U       = "info about DSA-CPU2U"
local help_DSA_QCN         = "info about DSA-QCN"

local help_do_not_modify   = "info about do-not-modify"

local help_string_show_egress_ha_detailed_action = "Show detailed info about ha specific action (per port(s))"
CLI_addHelp("exec", "show egress ha port-actionType-detailed", help_string_show_egress_ha_detailed_action)

local info_type_no_extra_info = 1
local info_type_802_1br_eTag  = 2
local info_type_dsa_Forward   = 3
local info_type_dsa_qcn       = 4
local info_type_802_1br_U2E_M4  = 5
local info_type_802_1br_U2E_M8  = 6
local info_type_802_1br_E2U_Untagged  = 7

-- special dictionary for show commands for the 'actionType' parameter
CLI_type_dict["show_actionType"] =
{
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = help_show_actionType,
    enum =
    {
        [action_no_extra_info ] = {infoType = info_type_no_extra_info, value = action_no_extra_info , help = help_actionType_no_extra_info} ,
        [action_802_1br_E2U    ] = {infoType = info_type_802_1br_eTag , value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E"   , help =   help_802_1br_E2U   },
        [action_802_1br_E2U_Untagged] = {infoType = info_type_802_1br_E2U_Untagged, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E"   , help =   help_802_1br_E2U_Untagged   },
        [action_802_1br_U2E    ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E"   , help =   help_802_1br_U2E   },
        [action_802_1br_U2C    ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E"   , help =   help_802_1br_U2C   },
        [action_802_1br_C2U    ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E"   , help =   help_802_1br_C2U   },
        [action_802_1br_U2CPU  ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E" , help =   help_802_1br_U2CPU },
        [action_802_1br_CPU2U  ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E" , help =   help_802_1br_CPU2U },
        [action_802_1br_U2IPL  ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E" , help =   help_802_1br_U2IPL },
        [action_802_1br_IPL2IPL] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E" , help =   help_802_1br_IPL2IPL },
        [action_802_1br_U2E_M4 ] = {infoType = info_type_802_1br_U2E_M4, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E"   , help =   help_802_1br_U2E_M4   },
        [action_802_1br_U2E_M8 ] = {infoType = info_type_802_1br_U2E_M8, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E"   , help =   help_802_1br_U2E_M8   },
        [action_802_1br_Drop   ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E" , help =   help_802_1br_Drop },

        [action_DSA_ET2U      ] = {infoType = info_type_dsa_Forward  , value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E"      , help =   help_DSA_ET2U      },
        [action_DSA_EU2U      ] = {infoType = info_type_dsa_Forward  , value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E"      , help =   help_DSA_EU2U      },
        [action_DSA_U2E       ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E"       , help =   help_DSA_U2E       },
        [action_DSA_Mrr2E     ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E"     , help =   help_DSA_Mrr2E     },
        [action_DSA_CPU2U     ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E"     , help =   help_DSA_CPU2U     },
        [action_DSA_QCN       ] = {infoType = info_type_dsa_qcn      , value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E"       , help =   help_DSA_QCN       },

        [action_do_not_modify ] = {infoType = info_type_no_extra_info, value = "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E" , help =   help_do_not_modify }
    }
}

local header_info_type_no_extra_info =
    "\n" ..
    "NOTE: the action holds NO extra info !!!\n" ..
    "\n" ..
    "p_t     - is 'packetType' (in decimal format)\n" ..
    "\n" ..
    "Dev/Port   action type             p_t  \n" ..
    "---------  --------------------    ---  \n"

local header_info_type_E2U_Untagged =
    "\n" ..
    "NOTE: the action holds info for E2U untagged packet\n" ..
    "\n" ..
    "p_t     - is 'packetType' (in decimal format)\n" ..
    "\n" ..
    "TPID    - TPID of the VLAN Tag added to E2U untagged packet (in HEX format - without '0x')\n" ..
    "\n" ..
    "Dev/Port   action type             p_t     TPID \n" ..
    "---------  --------------------    ---     ---- \n"

-- return true/false : is actionType hold 'no extra info'
local function is_action_hold_no_extra_info(actionType)
    local array_of_no_extra_info = {
     action_802_1br_U2E       ,
     action_802_1br_E2U_Untagged,
     action_802_1br_U2C       ,
     action_802_1br_C2U       ,
     action_802_1br_U2CPU     ,
     action_802_1br_CPU2U     ,
     action_802_1br_U2IPL     ,
     action_802_1br_IPL2IPL   ,
     action_802_1br_Drop      ,

     action_DSA_U2E           ,
     action_DSA_Mrr2E         ,
     action_DSA_CPU2U         ,

     action_do_not_modify
    }
    for index,value in pairs(array_of_no_extra_info) do
        if value == actionType then
            return true
        end
    end
    -- not found
    return false
end

-- function to print info according to header_info_type_no_extra_info
local function show_info_type_no_extra_info(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local actionType = params.actionType

    for packetType = 0, max_packetType do
        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end

        if(params.actionType == nil)then
            if(is_action_hold_no_extra_info(cpssInfo.actionType))then
                actionType = cpssInfo.actionType
            end
        end
        if(cpssInfo.actionType == actionType) then
            params.counted_num_entries = params.counted_num_entries + 1
            if(params.only_count_num_entrys) then
                -- we not really need to count ... just need to make sure 'not empty'
                break
            end
            --print info only for those types that 'hold_no_extra_info'
            command_data["result"] =
                string.format("%-9s %-25s %-3d", devPortStr, actionType , packetType)
            command_data:addResultToResultArray()
        end
    end
end

-- function to print info according to header_info_type_E2U_untagged
local function show_info_type_E2U_Untagged(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local actionType = params.actionType

    for packetType = 0, max_packetType do
        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end

        if(params.actionType == nil)then
            if(is_action_hold_no_extra_info(cpssInfo.actionType))then
                actionType = cpssInfo.actionType
            end
        end
        if(cpssInfo.actionType == actionType) then
            params.counted_num_entries = params.counted_num_entries + 1
            if(params.only_count_num_entrys) then
                -- we not really need to count ... just need to make sure 'not empty'
                break
            end
            --print info only for those types that 'hold_no_extra_info'
            command_data["result"] =
                string.format("%-9s %-25s %-6d %-4x", devPortStr, actionType , packetType, cpssInfo.actionInfo.info_802_1br_E2U.vlanTagTpid)
            command_data:addResultToResultArray()
        end
    end
end

local header_info_type_802_1br_eTag =
    "\n" ..
    "NOTE: the action holds 802.1BR info \n" ..
    "\n" ..
    "p_t     - is 'packetType' (in decimal format)\n" ..
    "PCP     - is 'E_PCP' \n" ..
    "DEI     - is 'E_DEI' \n" ..
    "I_ECID  - is 'Ingress_E_CID_base' (in HEX format - without '0x')\n" ..
    "Dir     - is 'Direction' \n" ..
    "US      - is 'Upstream Specific' \n" ..
    "ECID    - is 'E_CID_base' (in HEX format - without '0x')\n" ..
    "I_ECIDx - is 'Ingress_E_CID_ext' (in HEX format - without '0x')\n" ..
    "ECIDx   - is 'E_CID_ext' (in HEX format - without '0x')\n" ..
    "\n" ..
    "Dev/Port   action type  p_t TPID PCP DEI I_ECID Dir US GRP ECID I_ECIDx ECIDx \n" ..
    "---------  ------------ --- ---- --- --- ------ --- -- --- ---- ------- ----- \n"

-- return true/false : is actionType hold ETAG
local function is_action_hold_802_1br_eTag(actionType)
    local array_of_802_1br_eTag_info = {
     action_802_1br_E2U
    }

    for index,value in pairs(array_of_802_1br_eTag_info) do
        if value == actionType then
            return true
        end
    end
    -- not found
    return false
end

-- function to print info according to header_info_type_802_1br_eTag
local function show_info_type_802_1br_eTag(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local actionType = params.actionType
    local myLine

    for packetType = 0, max_packetType do
        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end

        if(params.actionType == nil)then
            if(is_action_hold_802_1br_eTag(cpssInfo.actionType))then
                actionType = cpssInfo.actionType
            end
        end

        if(cpssInfo.actionType == actionType) then
            params.counted_num_entries = params.counted_num_entries + 1
            if(params.only_count_num_entrys) then
                -- we not really need to count ... just need to make sure 'not empty'
                break
            end

            local data = cpssInfo.actionInfo.info_802_1br_E2U.eTag

            --print("data",to_string(data))

            myLine =
                string.format("%-9s %-18s %-3d", devPortStr, actionType , packetType)
            myLine = myLine..
                string.format(" %-4x %-3d %-3d", data.TPID, data.E_PCP , data.E_DEI)
            myLine = myLine..
                string.format(" %-6x %-3d %-2d %-3d", data.Ingress_E_CID_base, data.Direction , data.Upstream_Specific, data.GRP)
            myLine = myLine..
                string.format(" %-4x %-7x %-2x", data.E_CID_base, data.Ingress_E_CID_ext , data.E_CID_ext)

            command_data["result"] = myLine
            command_data:addResultToResultArray()
        end
    end
end

-- return true/false : is actionType hold ETAG
local function is_action_hold_802_1br_M4(actionType)
    local array_of_802_1br_M4_info = {
     action_802_1br_U2E_M4
    }

    for index,value in pairs(array_of_802_1br_M4_info) do
        if value == actionType then
            return true
        end
    end
    -- not found
    return false
end


local header_info_type_802_1br_U2E_M4 =
    "\n" ..
    "NOTE: the action holds 802.1BR info \n" ..
    "\n" ..
    "p_t     - is 'packetType' (in decimal format)\n" ..
    "\n" ..
    "Dev/Port   action type  p_t PCID1 PCID2 PCID3 \n" ..
    "---------  ------------ --- ----- ----- ----- \n"

-- function to print info according to header_info_type_802_1br_U2E_M4
local function show_info_type_802_1br_U2E_M4(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local actionType = params.actionType
    local myLine

    for packetType = 0, max_packetType do
        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end
        if(params.actionType == nil)then
            if(is_action_hold_802_1br_M4(cpssInfo.actionType))then
                actionType = cpssInfo.actionType
            end
        end

        if(cpssInfo.actionType == actionType) then
            params.counted_num_entries = params.counted_num_entries + 1
            if(params.only_count_num_entrys) then
                -- we not really need to count ... just need to make sure 'not empty'
                break
            end
            local data = cpssInfo.actionInfo.info_802_1br_U2E_MC

            --print("data",to_string(data))
             myLine =
                string.format("%-9s %-13s %-3d", devPortStr, actionType , packetType)
            myLine = myLine..
                string.format(" %-5x %-5d %-5d", data.pcid[0], data.pcid[1] , data.pcid[2])

            command_data["result"] = myLine
            command_data:addResultToResultArray()
        end
    end
end
-- return true/false : is actionType hold ETAG
local function is_action_hold_802_1br_M8(actionType)
    local array_of_802_1br_M8_info = {
     action_802_1br_U2E_M8
    }
    for index,value in pairs(array_of_802_1br_M8_info) do
        if value == actionType then
            return true
        end
    end
    -- not found
    return false
end



local header_info_type_802_1br_U2E_M8 =
    "\n" ..
    "NOTE: the action holds 802.1BR info \n" ..
    "\n" ..
    "p_t     - is 'packetType' (in decimal format)\n" ..
    "\n" ..
    "Dev/Port   action type  p_t PCID1 PCID2 PCID3 PCID4 PCID5 PCID6 PCID7 \n" ..
    "---------  ------------ --- ----- ----- ----- ----- ----- ----- ----- \n"


-- function to print info according to header_info_type_802_1br_U2E_M8
local function show_info_type_802_1br_U2E_M8(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local actionType = params.actionType
    local myLine

    for packetType = 0, max_packetType do
        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end

        if(params.actionType == nil)then
            if(is_action_hold_802_1br_M8(cpssInfo.actionType))then
                actionType = cpssInfo.actionType
            end
        end

        if(cpssInfo.actionType == actionType) then
            params.counted_num_entries = params.counted_num_entries + 1
            if(params.only_count_num_entrys) then
                -- we not really need to count ... just need to make sure 'not empty'
                break
            end

            local data = cpssInfo.actionInfo.info_802_1br_U2E_MC
            --print("data",to_string(data))

            myLine =
                string.format("%-9s %-13s %-3d", devPortStr, actionType , packetType)
            myLine = myLine..
                string.format(" %-5x %-5d %-5d", data.pcid[0], data.pcid[1] , data.pcid[2])
            myLine = myLine..
                string.format(" %-5x %-5d %-5d %-5d", data.pcid[3], data.pcid[4], data.pcid[5], data.pcid[6])

            command_data["result"] = myLine
            command_data:addResultToResultArray()
        end
    end
end



local header_info_type_dsa_Forward =
    "\n" ..
    "NOTE: the action holds DSA-forward info \n" ..
    "\n" ..
    "p_t - is 'packetType' (in decimal format)\n" ..
    "s_t - is 'srcTagged' ('t' is true , 'f' is false)\n" ..
    "s_d - is 'hwSrcDev'  (in HEX format - without '0x')\n" ..
    "s_p - is 'srcPortOrTrunk'  (in HEX format - without '0x')\n" ..
    "tr  - is 'srcIsTrunk' ('t' is true , 'f' is false)\n" ..
    "vid - is 'vid' (in HEX format - without '0x')\n" ..
    "\n" ..
    "Dev/Port   action type  p_t s_t s_d s_p tr cfi up  vid \n" ..
    "---------  ------------ --- --- --- --- -- --- --- --- \n"

local function bool_to_string(bool)
    if bool == true then return "t" end
    return "f"
end

-- return true/false : is actionType hold dsa_Forward
local function is_action_hold_dsa_Forward(actionType)
    local array_of_dsa_Forward_info = {
     action_DSA_ET2U,
     action_DSA_EU2U
    }
    for index,value in pairs(array_of_dsa_Forward_info) do
        if value == actionType then
            return true
        end
    end
    -- not found
    return false
end

-- function to print info according to header_info_type_dsa_Forward
local function show_info_type_dsa_Forward(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local actionType = params.actionType
    local myLine

    for packetType = 0, max_packetType do
        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end

        if(params.actionType == nil)then
            if(is_action_hold_dsa_Forward(cpssInfo.actionType))then
                actionType = cpssInfo.actionType
            end
        end

        if(cpssInfo.actionType == actionType) then
            params.counted_num_entries = params.counted_num_entries + 1
            if(params.only_count_num_entrys) then
                -- we not really need to count ... just need to make sure 'not empty'
                break
            end

            local data = cpssInfo.actionInfo.info_dsa_ET2U.dsaForward

            myLine =
                string.format("%-9s %-18s %-3d", devPortStr, actionType , packetType)
            myLine = myLine..
                string.format(" %-3s %-3x %-3x", bool_to_string(data.srcTagged), data.hwSrcDev , data.srcPortOrTrunk)
            myLine = myLine..
                string.format(" %-2s %-3d %-2d", bool_to_string(data.srcIsTrunk), data.cfi , data.up)
            myLine = myLine..
                string.format(" %-3x", data.vid)

            command_data["result"] = myLine
            command_data:addResultToResultArray()
        end
    end
end


local header_info_type_dsa_qcn =
    "\n" ..
    "NOTE: the action holds DSA-QCN info \n" ..
    "\n" ..
    "p_t - is 'packetType' (in decimal format)\n" ..
    "s_t - is 'DSA.srcTagged' ('t' is true , 'f' is false)\n" ..
    "s_d - is 'DSA.hwSrcDev'  (in HEX format - without '0x')\n" ..
    "s_p - is 'DSA.srcPortOrTrunk'  (in HEX format - without '0x')\n" ..
    "t   - is 'DSA.srcIsTrunk' ('t' is true , 'f' is false)\n" ..
    "c   - is 'DSA.cfi'\n" ..
    "vid - is 'DSA.vid' (in HEX format - without '0x')\n" ..
    "f   - is 'DSA.egrFilterRegistered' ('t' is true , 'f' is false)\n" ..
    "d   - is 'DSA.dropOnSource' ('t' is true , 'f' is false)\n" ..
    "l   - is 'DSA.packetIsLooped' ('t' is true , 'f' is false)\n" ..
    "r   - is 'DSA.wasRouted' ('t' is true , 'f' is false)\n" ..
    "sId - is 'DSA.srcId' (in HEX format - without '0x')\n" ..
    "qos - is 'DSA.qosProfileIndex' (in HEX format - without '0x')\n" ..
    "x   - is 'DSA.useVidx' ('t' is true , 'f' is false)\n" ..
    "target - is {DSA.hwTrgDev,DSA.trgPort} when useVidx = false \n" ..
    "cnmTpid - is 'cnmTpid' (in HEX format - without '0x')\n"..
    "         is trgVidx  when useVidx = true (in HEX format - without '0x')\n"..
    "\n" ..
    "Dev/Port  p_t source-mac-addr   s_t s_d s_p t c up vid f d l r sId qos x target  cnmTpid\n" ..
    "--------- --- ----------------- --- --- --- - - -- --- - - - - --- --- - ------- -------\n"

-- return true/false : is actionType hold dsa_QCN
local function is_action_hold_dsa_QCN(actionType)
    local array_of_dsa_QCN_info = {
     action_DSA_QCN
    }
    for index,value in pairs(array_of_dsa_QCN_info) do
        if value == actionType then
            return true
        end
    end
    -- not found
    return false
end

-- function to print info according to header_info_type_dsa_QCN
local function show_info_type_dsa_qcn(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local actionType = params.actionType
    local myLine

    for packetType = 0, max_packetType do

        local cpssInfo = cpss_get_packetType_info(command_data,devNum,portNum,packetType)
        if(cpssInfo == nil) then
            return
        end

        if(params.actionType == nil)then
            if(is_action_hold_dsa_QCN(cpssInfo.actionType))then
                actionType = cpssInfo.actionType
            end
        end

        if(cpssInfo.actionType == actionType) then
            params.counted_num_entries = params.counted_num_entries + 1
            if(params.only_count_num_entrys) then
                -- we not really need to count ... just need to make sure 'not empty'
                break
            end

            local data = cpssInfo.actionInfo.info_dsa_QCN

            myLine =
                string.format("%-9s %-3d", devPortStr, packetType)
            myLine = myLine..
                string.format(" %-17s", data.macSa);
            myLine = myLine..
                string.format(" %-3s %-3x %-3x", bool_to_string(data.dsaExtForward.srcTagged), data.dsaExtForward.hwSrcDev , data.dsaExtForward.srcPortOrTrunk)
            myLine = myLine..
                string.format(" %-1s %-1d %-2d", bool_to_string(data.dsaExtForward.srcIsTrunk), data.dsaExtForward.cfi , data.dsaExtForward.up)
            myLine = myLine..
                string.format(" %-3x", data.dsaExtForward.vid)

            myLine = myLine..
                string.format(" %-1s %-1s %-1s %-1s",
                    bool_to_string(data.dsaExtForward.egrFilterRegistered) ,
                    bool_to_string(data.dsaExtForward.dropOnSource) ,
                    bool_to_string(data.dsaExtForward.packetIsLooped) ,
                    bool_to_string(data.dsaExtForward.wasRouted))

            myLine = myLine..
                string.format(" %-3x %-3x", data.dsaExtForward.srcId , data.dsaExtForward.qosProfileIndex)

            myLine = myLine..
                string.format(" %-1s", bool_to_string(data.dsaExtForward.useVidx))

            if(false == data.dsaExtForward.useVidx) then
                myLine = myLine..
                    string.format(" {%-2x,%-2x}", data.dsaExtForward.hwTrgDev , data.dsaExtForward.trgPort)
            else
                myLine = myLine..
                    string.format(" vidx%-3x", data.dsaExtForward.trgVidx)
            end

            myLine = myLine..
                string.format(" %-4x", data.cnmTpid)

            command_data["result"] = myLine
            command_data:addResultToResultArray()
        end
    end
end

local show_header_info = {
    [info_type_no_extra_info] = {header = header_info_type_no_extra_info , show_func = show_info_type_no_extra_info},
    [info_type_802_1br_eTag ] = {header = header_info_type_802_1br_eTag  , show_func = show_info_type_802_1br_eTag },
    [info_type_dsa_Forward  ] = {header = header_info_type_dsa_Forward   , show_func = show_info_type_dsa_Forward  },
    [info_type_dsa_qcn      ] = {header = header_info_type_dsa_qcn       , show_func = show_info_type_dsa_qcn      },
    [info_type_802_1br_U2E_M4 ] = {header = header_info_type_802_1br_U2E_M4  , show_func = show_info_type_802_1br_U2E_M4 },
    [info_type_802_1br_U2E_M8 ] = {header = header_info_type_802_1br_U2E_M8  , show_func = show_info_type_802_1br_U2E_M8 },
    [info_type_802_1br_E2U_Untagged] = {header = header_info_type_E2U_Untagged , show_func = show_info_type_E2U_Untagged}
}

local function show_egress_ha_port_detailed_action(params)
    local header_string, footer_string
    local IN_infoType

    params.counted_num_entries = nil
    --print("params",to_string(params))

    if (params.actionType == nil and params.IN_infoType == nil) then
        local last_val1,last_val2
        isCounted = false
        for index , entry in pairs(show_header_info) do
            params.disablePrinting = true
            params.IN_infoType = index
            -- indicate that first we need to count entries
            params.only_count_num_entrys = true

            last_val1,last_val2 = show_egress_ha_port_detailed_action(params)
            if isCounted then
                params.disablePrinting = false
            end
            -- counting done into (params.counted_num_entries)
            if params.counted_num_entries ~= nil then
                -- we have entries to print
                params.only_count_num_entrys = nil
                params.disablePrinting = false
                last_val1,last_val2 = show_egress_ha_port_detailed_action(params)
                isCounted = true
            end
        end
        return last_val1,last_val2
    end

    if(params.actionType ~= nil) then
        params.actionType = get_cli_ha_action_type(params.actionType)
    end
    if(params.IN_infoType == nil)then
        IN_infoType = CLI_type_dict["show_actionType"].enum[params.actionType].infoType
    else
        IN_infoType = params.IN_infoType
    end

    if (params.counted_num_entries == nil) then
        params.counted_num_entries = 0
    end
    -- dedicated header in proper format
    header_string = show_header_info[IN_infoType].header
    -- dedicated function to print the info in proper format
    local show_func = show_header_info[IN_infoType].show_func

    footer_string = "\n"

    if(not params.only_count_num_entrys) then
        params.header_string = header_string
        params.footer_string = footer_string
    else
        params.header_string = nil
        params.footer_string = nil
    end

    return generic_all_ports_recursive_show_func(show_func,params)
end

--[[
show egress ha port-actionType-detailed <all/device/ethernet> [action<from 'list'>]
]]--
CLI_addCommand("exec", "show egress ha port-actionType-detailed", {
  func   = function(params)
        params.command_data = Command_Data()
        params.command_data:enablePausedPrinting()
    local ret =show_egress_ha_port_detailed_action(params)
        params.command_data:disablePausedPrinting()
        return ret
  end,
  help   = help_string_show_egress_ha_detailed_action,
  params = {
    { type = "named",

        "#all_interfaces_ver1",
        {format="actionType %show_actionType",   name="actionType", help = "(optional)"..help_show_actionType},

        requirements = { actionType = { "all_interfaces" }},
        mandatory = { "all_interfaces" }
    }}
})

--##################################
--##################################
local help_show_egress_source_port = "Show egress info about source port(s)"
CLI_addHelp("exec", "show egress source-port", help_show_egress_source_port)

local function per_port__show_egress_source_port(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)

    local cpssInfo = cpss_get_sourceInfo(command_data,devNum,portNum)
    if(cpssInfo == nil) then
        return
    end
    --print("cpssInfo",to_string(cpssInfo))
    if cpssInfo.portType == "802.1br" then
        local data = cpssInfo.portInfo.info_802_1br
        command_data["result"] =
            string.format("%-10s %-16s %-13x %-19d %-13s %-8s %-8s %-8s", devPortStr, cpssInfo.portType , data.pcid , data.srcPortInfo.srcFilteringVector, bool_to_string(data.upstreamPort),
                          "NA", "NA", "NA")
    else
        if cpssInfo.portType == "802.1br-Untagged" then
            local data = cpssInfo.portInfo.info_802_1br
            command_data["result"] =
                string.format("%-10s %-16s %-13x %-19s %-13s %-8s %-8s %-8s", devPortStr, cpssInfo.portType, data.pcid, "NA", bool_to_string(data.upstreamPort),
                              data.srcPortInfo.vlanTag.pcp, data.srcPortInfo.vlanTag.dei, data.srcPortInfo.vlanTag.vid)
        else
            command_data["result"] =
                string.format("%-10s %-16s %-13s %-19s %-13s %-8s %-8s %-8s", devPortStr, cpssInfo.portType ,"NA", "NA", "NA","NA","NA","NA")
        end
    end

    command_data:addResultToResultArray()
end

local function show_egress_source_port(params)
    local header_string, footer_string

    --print("1111111111111111111111",to_string(params))
    header_string =
        "\n" ..
        "port type           - hold the source port type \n" ..
        "pcid                - hold pcid for 802.1br-E2U , 802.1br-E2U-Untagged , 802.1br-U2E , 802.1br-U2C (in HEX format - without '0x')\n" ..
        "                      not relevant to other types \n" ..
        "srcFilteringVector  - hold the port bitmap (16 bits) for 802.1br-IPL2IPL port filtering( \n" ..
        "                      not relevant to other types \n" ..
        "upstreamPort        - hold the indication that the source port is an Upstream port for 802.1br-E2U , 802.1br-E2U-Untagged\n" ..
        "                      not relevant to other types \n" ..
        "srcPortNum          - hold the source port number (that represent the entry in ETAG/DSA) \n" ..
        "                      relevant only to those mentioned for <pcid> \n" ..
        "pcp                 - priority code point  (PCP): the IEEE 802.1p class of service \n" ..
        "                      relevant to untagged packet \n" ..
        "dei                 - drop eligible indicator \n" ..
        "                      relevant to untagged packet \n" ..
        "vid                 - VLAN identifier, specifying the VLAN to which the frame belongs \n" ..
        "                      relevant to untagged packet \n" ..

        "\n" ..
        "Dev/Port   port type       pcid          srcPortNum/         upstreamPort   pcp     dei     vid     \n" ..
        "                                         srcFilteringVector                                         \n" ..
        "---------  ---------       ------------  ------------------  ------------   ---     ---     ------  \n"
    footer_string = "\n"

    params.header_string = header_string
    params.footer_string = footer_string
    params.bypass_final_return = true

    return generic_all_ports_show_func(per_port__show_egress_source_port,params)
end

--[[
show egress source-port <all/device/ethernet>
]]--
CLI_addCommand("exec", "show egress source-port", {
  func   = show_egress_source_port,
  help   = help_show_egress_source_port,
  params = {
    { type = "named",

        "#all_interfaces_ver1",

        mandatory = { "all_interfaces"}
    }}
})

--##################################
--##################################
local help_show_egress_target_port = "Show egress info about target port(s)"
local help_show_egress_target_port_types = help_show_egress_target_port .. "- the type of port."
CLI_addHelp("exec", "show egress target-port",help_show_egress_target_port)
CLI_addHelp("exec", "show egress target-port types", help_show_egress_target_port_types)

local function per_port__show_egress_target_port_types(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)

    local cpssInfo = cpss_get_targetInfo(command_data,devNum,portNum)
    if(cpssInfo == nil) then
        return
    end

    command_data["result"] =
        string.format("%-10s %-10s", devPortStr, cpssInfo.portType)

    command_data:addResultToResultArray()
end

local function show_egress_target_port_types(params)
    local header_string, footer_string

    header_string =
        "\n" ..
        "Dev/Port   port type \n" ..
        "---------  --------- \n"
    footer_string = "\n"

    params.header_string = header_string
    params.footer_string = footer_string

    params.bypass_final_return = true
    return generic_all_ports_show_func(per_port__show_egress_target_port_types,params)
end
--[[
show egress target-port types <all/device/ethernet>
]]--
CLI_addCommand("exec", "show egress target-port types", {
  func   = show_egress_target_port_types,
  help   = help_show_egress_target_port_types,
  params = {
    { type = "named",

        "#all_interfaces_ver1",

        mandatory = { "all_interfaces"}
    }}
})
--##################################
--##################################
--[[
show egress target-port-detailed <all/device/ethernet> type <from 'list'>
]]--
local help_show_targetPortType = "optional parameter to show only ports with this type"
local help_show_egress_target_port_detailed = help_show_egress_target_port .. " detailed info."
CLI_addHelp("exec", "show egress target-port-detailed",help_show_egress_target_port_detailed)

-- special dictionary for show commands for the 'actionType' parameter
CLI_type_dict["show_targetPortType"] =
{
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = help_show_targetPortType,
    enum =
    {
        [info_target_port_common ]        = {infoType = info_target_port_common       , value = "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E"         , help =   info_target_port_common },
        [info_target_port_dsa_qcn]        = {infoType = info_target_port_dsa_qcn      , value = "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E"        , help =   info_target_port_dsa_qcn },
        [info_target_port_do_not_modify ] = {infoType = info_target_port_do_not_modify, value = "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E"  , help =   info_target_port_do_not_modify }
    }
}
local header_info_target_port_common =
        "\n" ..
        "NOTE: the action holds common info\n" ..
        "\n" ..
        "port type    - hold the target port type \n" ..
        "tpid         - hold tpid (in HEX format - without '0x') \n" ..
        "pcid         - hold pcid (in HEX format - without '0x') \n" ..
        "delay        - hold egressDelay (in HEX format - without '0x') \n" ..
        "\n" ..
        "Dev/Port   port type tpid pcid delay \n" ..
        "---------  --------- ---- ---- ----- \n"

local function show_info_target_port_common(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    local cpssInfo = cpss_get_targetInfo(command_data,devNum,portNum)
    if(cpssInfo == nil) then
        return
    end

    if(cpssInfo.portType ~= info_target_port_common) then
        return
    end

    params.counted_num_entries = params.counted_num_entries + 1
    if(params.only_count_num_entrys) then
        -- we not really need to count ... just need to make sure 'not empty'
        return
    end

    local myLine =
        string.format("%-10s %-10s", devPortStr, cpssInfo.portType)
    myLine = myLine ..
        string.format(" %-4x", cpssInfo.portInfo.info_common.tpid)
    myLine = myLine ..
        string.format(" %-4x %-5x", cpssInfo.portInfo.info_common.pcid, cpssInfo.portInfo.info_common.egressDelay)

    command_data["result"] = myLine
    command_data:addResultToResultArray()

    --print("show_info_target_port_common",devNum,portNum)
end

local header_info_target_port_dsa_qcn =
        "\n" ..
        "NOTE: the action holds no extra info for 'DSA-QCN' type\n" ..
        "\n" ..
        "port type    - hold the target port type \n" ..
        "\n" ..
        "Dev/Port   port type \n" ..
        "---------  --------- \n"
local function show_info_target_port_dsa_qcn(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)

    local cpssInfo = cpss_get_targetInfo(command_data,devNum,portNum)
    if(cpssInfo == nil) then
        return
    end

    if(cpssInfo.portType ~= info_target_port_dsa_qcn) then
        return
    end

    params.counted_num_entries = params.counted_num_entries + 1
    if(params.only_count_num_entrys) then
        -- we not really need to count ... just need to make sure 'not empty'
        return
    end

    command_data["result"] =
        string.format("%-10s %-10s", devPortStr, cpssInfo.portType)

    command_data:addResultToResultArray()

    --print("show_info_target_port_do_not_modify",devNum,portNum)
end

local header_info_target_port_do_not_modify =
        "\n" ..
        "NOTE: the action holds no extra info for 'do-not-modify' type\n" ..
        "\n" ..
        "port type    - hold the target port type \n" ..
        "\n" ..
        "Dev/Port   port type \n" ..
        "---------  --------- \n"
local function show_info_target_port_do_not_modify(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)

    local cpssInfo = cpss_get_targetInfo(command_data,devNum,portNum)
    if(cpssInfo == nil) then
        return
    end

    if(cpssInfo.portType ~= info_target_port_do_not_modify) then
        return
    end

    params.counted_num_entries = params.counted_num_entries + 1
    if(params.only_count_num_entrys) then
        -- we not really need to count ... just need to make sure 'not empty'
        return
    end

    command_data["result"] =
        string.format("%-10s %-10s", devPortStr, cpssInfo.portType)

    command_data:addResultToResultArray()

    --print("show_info_target_port_do_not_modify",devNum,portNum)
end

local show_target_port_info = {
    [info_target_port_common       ] = {header = header_info_target_port_common        , show_func = show_info_target_port_common          },
    [info_target_port_dsa_qcn      ] = {header = header_info_target_port_dsa_qcn       , show_func = show_info_target_port_dsa_qcn         },
    [info_target_port_do_not_modify] = {header = header_info_target_port_do_not_modify , show_func = show_info_target_port_do_not_modify   }
}

local function show_egress_target_port_detailed(params)
    local header_string, footer_string
    local IN_infoType
    local IN_need_all = false

    --print("params",to_string(params))
    params.counted_num_entries = 0

    if (params.targetPortType == nil and params.IN_infoType == nil) then
        local last_val1,last_val2
        for index , entry in pairs(show_target_port_info) do
            params.IN_infoType = index
            -- indicate that first we need to count entries
            params.only_count_num_entrys = true
            last_val1,last_val2 = show_egress_target_port_detailed(params)

            -- counting done into (params.counted_num_entries)
            if params.counted_num_entries ~= 0 then
                -- we have entries to print
                params.only_count_num_entrys = nil
                last_val1,last_val2 = show_egress_target_port_detailed(params)
            end
        end

        return last_val1,last_val2
    end

    if(params.targetPortType ~= nil) then
        params.targetPortType = get_cli_target_port_type(params.targetPortType)
    end

    if(params.IN_infoType == nil)then
        IN_infoType = CLI_type_dict["show_targetPortType"].enum[params.targetPortType].infoType
    else
        IN_infoType = params.IN_infoType
    end

    -- dedicated header in proper format
    header_string = show_target_port_info[IN_infoType].header
    -- dedicated function to print the info in proper format
    local show_func = show_target_port_info[IN_infoType].show_func

    footer_string = "\n"

    if(not params.only_count_num_entrys) then
        params.header_string = header_string
        params.footer_string = footer_string
    else
        params.header_string = nil
        params.footer_string = nil
    end
    params.bypass_final_return = true
    return generic_all_ports_show_func(show_func,params)
end

CLI_addCommand("exec", "show egress target-port-detailed", {
  func   = show_egress_target_port_detailed,
  help   = help_show_egress_target_port_detailed,
  params = {
    { type = "named",

        "#all_interfaces_ver1",
        --NOTE: when this parameter omitted , the first port define the 'targetPortType'.
        --          so ports with different type are skipped !
        --      when this parameter exists , it define the 'targetPortType'.
        --          so ports with different type are skipped !
        {format="type %show_targetPortType",   name="targetPortType", help = help_show_targetPortType},

        mandatory = { "all_interfaces"}
    }}
})

CLI_addCommand("exec", "show egress internal-DB", {
  func   = function ()
    print("egress_DB",to_string(egress_DB))
    end
})

