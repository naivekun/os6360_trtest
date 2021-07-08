--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* push_vlan_tag.lua
--*
--* DESCRIPTION:
--*       manage push configurations on eport
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



CLI_type_dict["push_vlan_command"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Push single tag on tunnel or on passenger packet layer 2 header\n",
    enum = {
        ["single-tag-on-tunnel"]    = { value=1, help="Push single Tag on tunnel packet layer 2 header" },
        ["single-tag-on-passenger"] = { value=2, help="Push single Tag on passenger packet layer 2 header, prior to TS encapsulation" }
    }
}

CLI_type_dict["tpid_index"] = {
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    max=7,
    help = "TPID table entry index"
}

-- ************************************************************************
--
--  push_vlan_tag_eport_func
--        @description  Push a new outermost tag for traffic that egress from the eport
--
--        @param params             - params["push_vlan_command"]: vlan push command <single-tag-on-tunnel/single-tag-on-passenger>
--                                  - params["vid"]: VLAN-ID assigned in the pushed tag
--                                  - params["tpid_index"]: index into the global egress TPID table
--                                  - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function push_vlan_tag_eport_func(params)
    -- Common variables declaration
    local iterator, devNum, portNum
    local vlanCmd
    local apiName,result,values
    local command_data = Command_Data()

    -- Common variables initialization.
    --command_data:initInterfaceRangeIterator()
    --command_data:initInterfaceDeviceRange()
    command_data:initInterfaceDevPortRange()

    if params.flagNo == true then -- 'unset'
        vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E"
        params.vid = 0
        params.tpid_index = 0
    else -- 'set'
        if "single-tag-on-tunnel"==params.push_vlan_command then
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E"
        else
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E"
        end
    end

    -- configure the eport with generic iterator
    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        if is_sip_5_10(devNum) then

            --***************
            --***************
            apiName = "cpssDxChBrgVlanPortPushVlanCommandSet"
            result = myGenWrapper(
                apiName, {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PORT_NUM", "portNum", portNum},
                { "IN", "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT", "vlanCmd", vlanCmd }
            })
            command_data:handleCpssErrorDevPort(result, apiName.."()")
            _debug(apiName .. "   result = " .. result)

            --***************
            --***************
            apiName = "cpssDxChBrgVlanPortPushedTagValueSet"
            result = cpssPerPortParamSet(
                apiName,
                devNum, portNum, tonumber(params.vid), "tagValue",
                "GT_U16")
            command_data:handleCpssErrorDevPort(result, apiName.."()")
            _debug(apiName .. "   result = " .. result)

            --***************
            --***************
            apiName = "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet"
            result = cpssPerPortParamSet(
                apiName,
                devNum, portNum, params.tpid_index, "tpidEntryIndex",
                "GT_U32")
            command_data:handleCpssErrorDevPort(result, apiName.."()")
            _debug(apiName .. "   result = " .. result)

        else
            command_data:setFailStatus()
            command_data:addError("Error setting push vlan command " ..
                    "on device %d ePort %d: %s", devNum,
                    portNum, "API not applicable to device.")
            return false

        end

        command_data:updateStatus() 
        command_data:updatePorts()            
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end


-- the command looks like :
--Console(config)# interface eport 0/4001
--Console(config-if-eport)# push-vlan-tag command {SINGLE-FROM-TUNNEL|SINGLE-FROM-PASSENGER} vid {vid} tpid-index {tpid-index}
CLI_addCommand("interface_eport", "push-vlan-tag", {
    func   = push_vlan_tag_eport_func,
    help   = "Push a new outermost tag into the outgoing packet",
    params = {
        { type = "named",
            { format = "command %push_vlan_command", name = "push_vlan_command", help = "Push single tag on tunnel or passenger packet layer 2 header"},
            { format = "vid %vid", name = "vid", help = "VLAN-ID assigned in the pushed tag"},
            { format= "tpid-index %tpid_index", name = "tpid_index", help = "index of global egress TPID entry which is used for the pushed tag"},
            mandatory = { "push_vlan_command","vid","tpid_index"}
        }
    }
})

-- the command looks like :
--Console(config)# interface eport 0/4001
--Console(config-if-eport)# no push-vlan-tag
CLI_addCommand("interface_eport", "no push-vlan-tag", {
    func = function(params)
        params["flagNo"] = true
        push_vlan_tag_eport_func(params)
    end,
    help   = "Unset pushing a new outermost tag into the outgoing packet",
    params = {}
})

