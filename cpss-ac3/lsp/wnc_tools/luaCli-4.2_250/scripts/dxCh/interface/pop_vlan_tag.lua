--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pop_vlan_tag.lua
--*
--* DESCRIPTION:
--*       manage pop configurations on eport
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--require
--constants

local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

CLI_type_dict["4B_8B_tag"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "4-bytes VLAN tag or 8-bytes VLAN tag",
    enum = {
        ["4B-vlan-tag"] = { value=1, help="4-Bytes VLAN tag" },
        ["8B-E-Tag"]    = { value=2, help="8-Bytes E-Tag" }
    }
}

-- ************************************************************************
--
--  pop_vlan_tag_eport_func
--        @description  Set the number of tag words to pop for traffic that ingress to the eport
--
--        @param params             - params["tag_length"]: number of tag words to pop <4-BYTES/8-BYTES>
--                                  - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function pop_vlan_tag_eport_func(params)
    -- Common variables declaration
    local iterator, devNum, portNum
    local numberOfTagWords
    local apiName,result
    local command_data = Command_Data()

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    if params.flagNo == true then -- 'unset'
        numberOfTagWords = 0
    else -- 'set'
        if "4B-vlan-tag"==params.tag_length then
            numberOfTagWords = 1
        else
            numberOfTagWords = 2
        end
    end

        -- configure the eport with generic iterator
    for iterator, devNum, portNum in command_data:getPortIterator() do

        --***************
        --***************
        apiName = "cpssDxChBrgVlanPortNumOfTagWordsToPopSet"
        result = cpssPerPortParamSet(
            apiName,
            devNum, portNum, numberOfTagWords, "numberOfTagWords",
            "GT_U32")
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- the command looks like :
--Console(config)# interface eport 0/4001
--Console(config-if-eport)# pop-vlan-tag {4-BYTES|8-BYTES}
CLI_addCommand("interface_eport", "pop-vlan-tag", {
    func   = pop_vlan_tag_eport_func,
    help   = "Set the number of tag words to pop for traffic that ingress to the eport",
    params = {
        { type = "named",
            { format = "4B-vlan-tag",
                help = "Popping 4-bytes VLAN Tag"  },
            { format = "8B-E-Tag",
                help = "Popping 8-bytes E-Tag"     },
            alt       = { tag_length    =    { "4B-vlan-tag", "8B-E-Tag" }  },
            mandatory = { "tag_length" }
        }
    }
})

-- the command looks like :
--Console(config)# interface eport 0/4001
--Console(config-if-eport)# no pop-vlan-tag
CLI_addCommand("interface_eport", "no pop-vlan-tag", {
    func = function(params)
        params["flagNo"] = true
        pop_vlan_tag_eport_func(params)
    end,
    help   = "Unset the number of tag words to pop for traffic that ingress to the eport",
    params = {}
})


