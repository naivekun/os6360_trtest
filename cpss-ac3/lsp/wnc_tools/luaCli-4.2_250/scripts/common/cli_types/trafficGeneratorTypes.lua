--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* trafficGeneratorTypes.lua
--*
--* DESCRIPTION:
--*       CLI, VPT and packet length type
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************
---
--  check_cyclicPatternArr
--        @description  ckecks cyclic pattern arrey
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--
--        @return       true and number turned to hexa on success, otherwise false and
--                      error message
--

function check_cyclicPatternArr(param, name)
    if "0x"==string.sub(param,1,2) then
        if 19>string.len(param) then 
            if nil~=tonumber(param) then
                return true, string.sub(param,3)
            else
                return false, name .. " not a number"
            end
        else
            return false, "Error: "..name.. "is bigger than 8 bytes"
        end
    else--param is DECIMAL
            num=tonumber(param)

            if nil == num then
                return false, name .. " not a number"
            end
            local hexstr = '0123456789abcdef'
            local s = ''
            while num > 0 do
                local mod = math.fmod(num, 16)
                s = string.sub(hexstr, mod+1, mod+1) .. s
                num = math.floor(num / 16)
            end
            if s == '' then s = '0' end
            return check_cyclicPatternArr("0x"..s,name)
    end
end



------------------------------------------
-- type registration: cli
--------------------------------------------
CLI_type_dict["cfi"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1,
    help="Enter CFI bit to add. (0/1)"
}

------------------------------------------
-- type registration: vpt
--------------------------------------------
CLI_type_dict["vpt"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    help="Enter VPT number to add. (0..7)"
}

------------------------------------------
-- type registration: packetLength
--------------------------------------------
CLI_type_dict["packetLength"] = {
    checker = CLI_check_param_number,
    min=20,
    max=16383,
    help="Enter packet length. (20..16383)"
}
------------------------------------------
-- type registration: packetLength
--------------------------------------------
CLI_type_dict["cyclicPatternArr"] = {
    checker = check_cyclicPatternArr,
    help="Enter cyclic pattern. (8 bytes)"
}

