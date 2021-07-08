--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_range.lua
--*
--* DESCRIPTION:
--*       port-range type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("luaCLI_getDevInfo")
cmdLuaCLI_registerCfunction("wrlCpssDevicePortNumberGet")

--constants

--
-- Types defined:
--
-- channel-group   - The range of ports
--
do
-- ************************************************************************
---
--  add_string_to_list
--        @description  Adds the ports from start to stop in device dev to
--                      the table tab
--
--        @param tab            - The table to add the ports to, they can
--                                be accessed using by
--                                tab[devID][1]...tab[devID][n]
--        @param rev            - The table to used for reverse lookup,
--                                (first time it is called on "tab" it has
--                                to be an empty table)-- @param dev The
--                                device ID of the ports
--        @param start          - The string from which to retrieve the
--                                start of the port numbers
--        @param stop           - The string from which to retrieve the
--                                ending of the port numbers
--
--        @return       boolean check status
--
local function add_string_to_list(tab, rev, dev, start, stop)
    local i

    if (type(rev) == "nil") then
        error("Rev can't be nil")
    end
    if (stop < start) then
        start, stop = stop, start
    end
    if type(tab[dev]) == "nil" then
        tab[dev] = {}
    end


    for i = start, stop do
        if (type(rev[dev .. "/" .. i]) == "nil") then
            table.insert(tab[dev], i)
            rev[dev .. "/" .. i] = 1
        end
    end
end


-- ************************************************************************
---
--  check_param_port_range
--        @description  Check parameter that it is in the correct form
--
--        @param param          - Parameter string
--
--        @return       boolean check status
--        @return       number:paramList or string:error_string if failed
--        @return       The format of paramList
--                          paramList[devID][1]...paramList[devID][n]
--
function check_param_port_range_gen(param, remote)
    local ret, alonErr
    local i, c, state, digits, devID, start, stop, tmp, rev
    local s, maxPortNum
    --print(to_string(param) .." ".. to_string(remote))
    --Initalize parameters
    s = 1
    state = 1
    devID = 0
    maxPortNum = 0
    digits = 0
    start = 0
    stop = 0
    ret = {}
    tmp = 0
    rev = {}
    alonErr = "Wrong interface format, required format is dev/port"
    if not remote then
        alonErr = alonErr .. " or \"all\"."
    end
    alonErr = alonErr .. "\n"
    alonErr = alonErr .. "To indicate port 5 on device 0 use 0/5.\n"
    alonErr = alonErr .. "To indicate ports 4-9,12 on device 0 use 0/4-9,12"
    if param=="all" then
        if remote then
            return false, "all not applicable for remote devices"
        end
        ret=luaCLI_getDevInfo()
    else
        --Add a terminator, will save an additional check in each place
        param = param .. ","
        for i = 1,string.len(param) do
            c = string.sub(param, i, i)
            digits = digits + 1
            if (state == 1) then
                --Check the device ID
                if type(tonumber(c)) ~= "nil" then
                    --valid digit
                    devID = (devID * 10) + tonumber(c)
                elseif ((c == "/") and (digits > 1)) then
                    --This is is a valid "/"
                    maxPortNum = 8191 -- default, used for remote devices
                    if not remote then
                        if ( wrLogWrapper("wrlIsDevIdValid", "(devID)", devID) == false) then
                            return false, "Device ID " .. devID .. " doesn't exist"
                        end
                        if dxCh_family and is_sip_5(devID) then
                            maxPortNum=8191
                        else
                            tmp, maxPortNum = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devID)", devID)
                            maxPortNum=maxPortNum-1
                            if 0 ~= tmp then
                                return false, "Error at maximum port number getting " ..
                                              "of device " .. tostring(devID)
                            end
                        end
                    end
                    state = 2
                    digits = 0
                else
                    --Something is wrong with the format
                    return false, alonErr --"bad format"
                end
            elseif (state == 2) then
                --Check the first part of the port range format, or if it is a singular port
                if type(tonumber(c)) ~= "nil" then
                    --The start of a range / port is ok
                    start = (start * 10) + tonumber(c)
                elseif ((c == "-") and (digits > 1)) then
                    --It is a port range, need to check to end
                    state = 3
                    digits = 0
                elseif ((c == ",") and (digits > 1)) then
                    --It is only a single entry add it and check if there are more
                    add_string_to_list(ret, rev, devID, start, start)
                    state = 4
                    start = 0
                    stop = 0
                    digits = 0
                else
                    return false, alonErr --"bad format"
                end
            elseif (state == 3) then
                --Check that the stop number is valid
                if type(tonumber(c)) ~= "nil" then
                    --The digit is valid add it
                    stop = (stop * 10) + tonumber(c)
                elseif ((c == ",") and (digits > 1)) then
                    --Add the range to the list and return to state 2
                    if maxPortNum < stop then
                        return false, stop .. " is out of port range"
                    end
                    add_string_to_list(ret, rev, devID, start, stop)
                    state = 4
                    start = 0
                    stop = 0
                    digits = 0
                    tmp = 0
                else
                    return false, alonErr --"bad format"
                end
            elseif (state == 4) then
                --Check the number
                if type(tonumber(c)) ~= "nil" then
                    --valid digit
                    tmp = (tmp * 10) + tonumber(c)
                elseif ((c == "/") and (digits > 1)) then
                    --This is is a valid "/", this must be a device ID
                    devID = tmp
                    if not remote then
                        if (wrLogWrapper("wrlIsDevIdValid", "(devID)", devID) == false) then
                            return false, "Device ID " .. devID .. " doesn't exist"
                        end
                    end
                    state = 2
                    digits = 0
                    tmp = 0
                elseif ((c == "-") and (digits > 1)) then
                    start = tmp
                    state = 3
                    digits = 0
                elseif ((c == ",") and (digits > 1)) then
                    --It is only a single entry add it and check if there are more
                    add_string_to_list(ret, rev, devID, tmp, tmp)
                    state = 4
                    start = 0
                    stop = 0
                    digits = 0
                    tmp = 0
                else
                    --Something is wrong with the format
                    return false, alonErr --"bad format"
                end

            end
        end
    end
    --Return true and the list
    return true, ret
end

function check_param_port_range(param)
    return check_param_port_range_gen(param, false)
end

CLI_type_dict["port-range"] = {
    checker = check_param_port_range,
    help = "The range of ports. Example: 0/2-4,6 means device 0 ports 2,3,4,6 or \"all\" for all ports"
}

function check_param_remote_port_range(param)
    return check_param_port_range_gen(param, true)
end

CLI_type_dict["remote-port-range"] = {
    checker = check_param_remote_port_range,
    help = "The range of remote ports. Example: 0/2-4,6 means device 0 ports 2,3,4,6"
}

end
