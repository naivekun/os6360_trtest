--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_cmd_enum.lua
--*
--* DESCRIPTION:
--*       Command enum for packet command
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes


--constants

local function CLI_check_param_packet_command_type(param,name,desc,varray,params)
    local valid, val = CLI_check_param_enum(param,name,desc)
    if not valid then
        return valid, val
    end
    if param == "route" or param == "route-and-mirror" or param == "default-route-entry" then
        ------------------------------------------------------
        -- type registration: unchecked-vlan
        -------------------------------------------------------
        CLI_type_dict["unchecked-vlan_cmd_def"] =
        {
            checker  = check_param_unchecked_vlan,
            complete = vlan_complete_param_number_max,
            min      = 1,
            max      = 4095,
            help     = "Specify IEEE 802.1Q VLAN ID"
        }
        -------------------------------------------------------
        -- type registration: portNum
        -------------------------------------------------------
        CLI_type_dict["dev_port_cmd_def"] =
        {
            checker  = check_param_dev_port,
            complete = complete_param_dev_port,
            help     = "Ethernet port"
        }
        -------------------------------------------------------
        -- type registration: trunkID
        -------------------------------------------------------
        CLI_type_dict["trunkID_cmd_def"] =
        {
            checker  = check_param_trunk,
            complete = complete_param_trunk,
            min      = 1,
            max      = MAXIMUM_TRUNK_ID,
            help     = "Valid Port-Channel interface"
        }
        -------------------------------------------------------
        -- type registration: dev_ePort
        -------------------------------------------------------
        CLI_type_dict["dev_ePort_cmd_def"] =
        {
            checker  = check_param_dev_ePort,
            help     = "dev/ePort, where ePort in range 256 to (8K-1)"
        }
        -------------------------------------------------------
        -- type registration: mac address
        -------------------------------------------------------
        CLI_type_dict["mac-address_cmd_def"] =
        {
            checker  = check_param_mac,
            complete = complete_param_mac,
            help     = "MAC address"
        }
    else
        params["interface_port_channel_cmd_def"] = 0
        params["vid"] = 0
        params["mac-address"] = 0
    end
    return valid, val
end


--------------------------------------------
-- type registration: packet_cmd
--------------------------------------------
CLI_type_dict["packet_cmd"] =
{
    checker  = CLI_check_param_packet_command_type,
    complete = CLI_complete_param_enum,
    help     = "Enter packet command",
    enum =
    {
        ["trap-to-cpu"]         = {value = "CPSS_PACKET_CMD_TRAP_TO_CPU_E",         help="trap to CPU."},
        ["hard-drop"]           = {value = "CPSS_PACKET_CMD_DROP_HARD_E",           help="drop hard."},
        ["soft-drop"]           = {value = "CPSS_PACKET_CMD_DROP_SOFT_E",           help="drop soft."},
        ["route"]               = {value = "CPSS_PACKET_CMD_ROUTE_E",               help="route."},
        ["route-and-mirror"]    = {value = "CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E",    help="route and mirror."},
        ["default-route-entry"] = {value = "CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E", help="default route entry."}
    }
}



