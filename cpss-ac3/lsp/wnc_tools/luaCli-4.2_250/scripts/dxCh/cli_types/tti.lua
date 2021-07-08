--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tti.lua
--*
--* DESCRIPTION:
--*       tti type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--********************************************************************************


--------------------------------------------
-- type registration: packet-command
--------------------------------------------
CLI_type_dict["packet-command"] = {
    checker     = CLI_check_param_enum,
    complete    = CLI_complete_param_enum,
    help        = "packet-command",
    enum = {
        ["forward"] = { value=0, help="forward packet" },
        ["mirror"] = { value=1, help="mirror packet to CPU" }, 
        ["trap"] = { value=2, help="trap packet to CPU" }, 
        ["hard-drop"] = { value=3, help="hard drop packet" }, 
        ["soft-drop"] = { value=4, help="soft drop packet" }
        }
}

--------------------------------------------
-- type registration: modify
--------------------------------------------
CLI_type_dict["modify"] = {
    checker     = CLI_check_param_enum,
    complete    = CLI_complete_param_enum,
    help        = "",
    enum = {
        ["untagged"] = { value=1, help="modify vlan only for untagged packets" }, 
        ["tagged"] = { value=2, help="modify vlan only for tagged packets" }, 
        ["all"] = { value=3, help="modify vlan to all packets" }
        }
}

--------------------------------------------
-- type registration: tagged
--------------------------------------------
CLI_type_dict["tagged"] = {
    checker     = CLI_check_param_enum,
    complete    = CLI_complete_param_enum,
    help        = "",
    enum = {
        ["tagged"] = { value=true, help="Specifies if the tag is exist" },
        ["untagged"] = { value=false, help="Specifies if the tag isn't exist" }
        }
}

--------------------------------------------
-- type registration: packetType
--------------------------------------------
CLI_type_dict["packetType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "packet types",
    enum = {
        ["legacy-ipv4-tunnel"] = { value=0, help="IPv4 TTI key type" },
        ["legacy-mpls"] = { value=1, help="MPLS TTI key type" },
        ["legacy-ethernet"] = { value=2, help="Ethernet TTI key type" },
		["legacy-mim"] = { value=3, help="Mac in Mac TTI key type" }
    }
}

function check_index(param, name, desc)
    local devNum  = devEnv.dev
    param = tonumber(param)
    
    if param == nil then
        return false, name .. " not a number"
    end

    stat, index = CLI_check_param_number(param, name, desc)
    if stat == false then
        return false, index
    end

	if index > getNumberOfTTIindices(devNum) then
		return false, index .. " is too high"
	end
	
    return true, param

end

local function complete_index(param, name, desc)
    local compl, help = CLI_complete_param_number(param, name, desc)

    return compl, help
end

--------------------------------------------
-- type registration: index
--------------------------------------------
CLI_type_dict["index"] = {
    checker     = check_index,
    complete    = complete_index,
    min         = 0,  
    help        = "index number"
}

--------------------------------------------
-- type registration: index
--------------------------------------------
CLI_type_dict["UDBIndex"] = {
    checker     = check_index,
    complete    = complete_index,
    min         = 0,   
	max			= 29,
    help        = "UDB index number"
}

