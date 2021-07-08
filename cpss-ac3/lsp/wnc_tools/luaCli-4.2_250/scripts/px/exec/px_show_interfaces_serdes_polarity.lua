--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_serdes_polarity.lua
--*
--* DESCRIPTION:
--*       display the Serdes Polarity configuration
--*
--*
--********************************************************************************

--constants

-- ************************************************************************

local function Ret_Table_Update(Ret_Table, devNum, portNum,laneNum,invertTx,invertRx)

    if (Ret_Table ~= nil ) then

        Ret_Table[devNum]                   = tableInsert(Ret_Table[devNum],nil)
        Ret_Table[devNum][portNum]          = tableInsert(Ret_Table[devNum][portNum], nil)
        Ret_Table[devNum][portNum][laneNum] = tableInsert(Ret_Table[devNum][portNum][laneNum], nil)

        Ret_Table[devNum][portNum][laneNum].invertTx = invertTx
        Ret_Table[devNum][portNum][laneNum].invertRx = invertRx

    end
    return Ret_Table

end


-- ************************************************************************
local function PrintLane(command_data,devNum,portNum,laneNum,Ret_Table)

    local devnum_portnum_string
    local GT_OK = 0
    local ret, val
    local laneNumS
    local invertTx
    local invertRx


    ret, val = myGenWrapper("cpssPxPortSerdesPolarityGet",{
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","GT_U32","laneNum",laneNum},
        {"OUT","GT_BOOL","invertTx"},  
        {"OUT","GT_BOOL","invertRx"}}) 

    if(ret~=GT_OK) then
        command_data:addError("Error cpssPxPortSerdesPolarityGet: device %d " ..
                              "port %d: lane %d: %s", devNum, portNum, laneNum,
                              returnCodes[ret])

    else

        devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                 '/' ..
                                                 tostring(portNum), 9)
        command_data:clearLocalStatus()
        command_data:updateStatus()

        laneNumS= to_string(laneNum)
        invertTx= to_string(val.invertTx)
        invertRx= to_string(val.invertRx)

        command_data["result"] =
            string.format("%-10s%-8s%-8s%-8s",
                          devnum_portnum_string,
                          alignLeftToCenterStr(laneNumS, 7),
                          alignLeftToCenterStr(invertTx, 7),
                          alignLeftToCenterStr(invertRx, 7))

        Ret_Table = Ret_Table_Update(Ret_Table, devNum, portNum,laneNum,val.invertTx,val.invertRx)

        command_data:addResultToResultArray()
        command_data:updatePorts()

    end

    return ret,Ret_Table

end

-- ************************************************************************
---
--  show_interfaces_serdes_polarity
--        @description  show interfaces Serdes Polarity
--
--        @param params         - params["all"]: all devices port or all
--                                ports of given device cheking switch,
--                                could be irrelevant;
--                                params["devID"]: checked device number,
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil;
--                                params["port-channel"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--
--        @return       true on success, otherwise false and error message
--
-- ************************************************************************
function show_interfaces_serdes_polarity(params)
-- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, footer_string1
    local ret, val
    local numOfSerdesLanesPtr
    local laneNum
    local Ret_Table = {}
    local command_data = Command_Data()


    --print("params  = "..to_string(params))

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end

    -- Command specific variables initialization.
    header_string1 =
                "\n" ..
                "Serdes Polarity Config\n" ..
                "----------------\n" ..
                "Interface LaneNum invertTx invertRx\n" ..
                "--------- ------- -------  --------\n"
    footer_string1 = "\n"  

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            if portNum == "CPU" then portNum = 16 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                         '/' ..
                                                         tostring(portNum), 9)
            command_data:clearLocalStatus()
            laneNum=params["laneNum"]

            if laneNum == "all" then
                ret,numOfSerdesLanesPtr= PxGetNuberSerdesOfPort(devNum,portNum)
                if( ret == 0 and numOfSerdesLanesPtr > 0) then
                    for i=0,numOfSerdesLanesPtr-1,1 do

                        laneNum=i
                        ret, Ret_Table =  PrintLane(command_data,devNum,portNum,laneNum,Ret_Table)
                        if(ret~=GT_OK) then
                            break
                        end
                    end
                end -- if( ret == 0 and numOfSerdesLanesPtr > 0)
            else
                ret, Ret_Table =  PrintLane(command_data,devNum,portNum,laneNum,Ret_Table)
            end
        end

        -- Resulting table string formatting.
        command_data:setResultArrayToResultStr(1, 1)
        command_data:setResultStrOnPortCount(header_string1, command_data["result"], footer_string1,
            "There is no port information to show.\n")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()

    end

    --print("Ret_Table  = "..to_string(Ret_Table))
    return command_data:getCommandExecutionResults(),Ret_Table

end

--------------------------------------------------------------------------------
-- command registration: show interfaces Serdes Polarity
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces serdes polarity", {
    func = show_interfaces_serdes_polarity,
    help = "Interface(s) serdes polarity",
    params = {
        {   type= "named",  {format="lane_num %lane_number_type", name="laneNum", help="Serdes lane number  or all"},
                                "#all_interfaces",
                        mandatory={"all_interfaces"}
        }
    }
})

