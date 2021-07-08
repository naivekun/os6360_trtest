--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* lane_number_type.lua.lua
--*
--* DESCRIPTION:
--*       lane_number_type.lua type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

cmdLuaCLI_registerCfunction("wrlCpssDxChPortNumberOfSerdesLanesGet")

-- ************************************************************************
---
--  check_lane_number
--        @description  ckecks prbs lane number
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--
--        @return       true and portNum on success, otherwise false and
--                      error message
--


local function check_lane_number(param, name, desc)
    local result, values, index
    local all_ports, dev_ports, devNum
    local mode
    local temp
    param = tonumber(param)
    if nil == param then
        return false, name .. " not a number"
    end
    mode=CLI_current_mode_get()
    if (mode=="interface") then
        
        all_ports = getGlobal("ifRange")--get table of ports
        for devNum, dev_ports in pairs(all_ports) do
            for k, port in pairs(dev_ports) do          
                result,values=wrLogWrapper("wrlCpssDxChPortNumberOfSerdesLanesGet","(devNum,port)", devNum, port)
                if 0~=result then
                    return false, returnCodes[result]
                else
                    if param>=values  then
                        return false, "Wrong lane number for device "..devNum.." port "..port..". Lane number for this port should be less than "..values
                    end
                end
            end
        end
    end
    return true, param
end


-- ************************************************************************
---
--  check_lane_number_or_all
--        @description  check's Vlan id parameter or "all" that it is
--                      in the correct form
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        device numer
--
local function check_lane_number_or_all(param,name, desc)
    if param == "all" then
        return true, param
    end
    return check_lane_number(param,name)
end


-- ************************************************************************
---
--  complete_lane_number_all
--        @description  autocompletes lane number parameter
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        device number autocomplete
--
local function complete_lane_number_all(param,name, desc)
    local compl, help = CLI_complete_param_number(param,name, desc)
    if prefix_match(param, "all") then
        table.insert(compl,"all")
        help[#compl] = "Apply to all lane number"
    end
    return compl, help
end



-------------------------------------------------------
-- type registration: lane_number_type
-------------------------------------------------------
CLI_type_dict["lane_number_type"] =
{
    checker  = check_lane_number,
    help     = "Enter lane number"
}

-------------------------------------------------------
-- type registration: LaneNumberOrAll
-------------------------------------------------------
CLI_type_dict["LaneNumberOrAll"] = {
    checker = check_lane_number_or_all,
    complete = complete_lane_number_all,
    help = "Enter lane number or all"
}

-- CLI_type_dict["LaneNumberOrAll"] = {
--     checker = check_lane_number_or_all,
--     help = "Enter lane number or all"
-- }
