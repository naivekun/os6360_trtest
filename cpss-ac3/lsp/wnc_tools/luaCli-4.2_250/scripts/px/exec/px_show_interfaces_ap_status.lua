--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_interfaces_ap_status.lua
--*
--* DESCRIPTION:
--*       showing of the ap status of the port
--*
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("cpssPxPortApPortStatusGet")
cmdLuaCLI_registerCfunction("wrlCpssGtU64MathAdd")

--constants



-- ************************************************************************
---
--  px_show_interfaces_ap_status_port_range
--        @description  shows statistics on traffic seen by the physical
--                      interface with many ports
--
--        @param params         - params["all_device"]: all or given
--                                devices iterating property, could be
--                                irrelevant
--                                params["mac-address"]: mac-address, could
--                                be irrelevant;
--                                params["ethernet"]: interface range,
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be
--                                irrelevant;
--                                params["vlan"]: vlan Id, could be
--                                irrelevant;
--                                params["skipped"]: skipping property,
--                                could be irrelevant
--        @param command_data   - command execution data object
--
--        @return        true on success, otherwise false and error message
--

local function px_show_interfaces_ap_status_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, header_string2, header_string3, header_string1, footer_string1, footer_string2, footer_string3, footer_string1
    local ret, localApEnablePtr
    local apEnable

    -- Command specific variables initialization.

    header_string1 =
        "\n" ..
        "Ap Port Status\n" ..
        "--------------\n" ..
        "Interface      AP Number               Mode                Speed              hcd Found      \n" ..
        "--------- -------------------- -------------------- -------------------- --------------------\n"
    footer_string1 = "\n"
    local header_string2 =
        "\n" ..
        "Interface     Fec Enabled          fcTxPauseEn          fcRxPauseEn                \n" ..
        "--------- -------------------- -------------------- --------------------           \n"
    local footer_string2 = "\n"

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            if portNum == "CPU" then portNum = 16 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                         '/' ..
                                                         tostring(portNum), 9)
            -- Traffic statistic getting.
            command_data:clearLocalStatus()
            command_data:updateStatus()

            local values
            ret, values = myGenWrapper(
                    "cpssPxPortPizzaArbiterDevStateGet", {
                        { "IN", "GT_U8"  , "devNum", devNum},                                    -- devNum
                        { "OUT","CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
                    })
            if (ret == 0) then
                OutValues = values["pizzaDeviceStatePtr"]
            end

            ret, localApParamsPtr = myGenWrapper("cpssPxPortPhysicalPortDetailedMapGet",{
                  {"IN","GT_U8","devNum",devNum},
                  {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                  {"OUT","CPSS_PX_DETAILED_PORT_MAP_STC","portMapShadowPtr"}})

            if (ret ~= GT_OK) then
                print(ret)
                print(returnCodes[ret])
                local index = string.find(returnCodes[ret],"=")
                print("Error at show interfaces ap status command : cpssPxPortPhysicalPortDetailedMapGet " .. devNum .. "," .. portNum .. " :" ..  string.sub(returnCodes[ret],index + 1))
                do return end
            end

            if(localApParamsPtr["portMapShadowPtr"]["valid"] == true) then
                -- Traffic statistic getting.
                ret, localApParamsPtr = myGenWrapper("cpssPxPortApEnableGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"OUT","GT_BOOL","apEnabled"}})

                if(localApParamsPtr["apEnabled"] == true) then
                    ret, localApParamsPtr = myGenWrapper("cpssPxPortApPortStatusGet",{
                            {"IN","GT_U8","devNum",devNum},
                            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                            {"OUT","CPSS_PX_PORT_AP_STATUS_STC","apParamsPtr"}})

                    if (ret ~= GT_OK) then
                        local index = string.find(returnCodes[ret],"=")
                        if(ret ~= 4) then
                            print("Error at show interfaces ap status command : cpssPxPortApPortStatusGet " .. devNum .. "," .. portNum .. " :" ..  string.sub(returnCodes[ret],index + 1))
                        end
                    else
                        if(localApParamsPtr~=nil and localApParamsPtr["apParamsPtr"]~=nil) then
                            outVal=localApParamsPtr["apParamsPtr"]
                            postApPortNum=outVal.postApPortNum          --postApPortNum

                            portMode=outVal.portMode
                            ifMode=string.sub(portMode.ifMode,26,-3)    --ifMode
                            speed=string.sub(portMode.speed,17,-3)      --speed

                            hcdFound=outVal.hcdFound                --hcdFound
                            fecEnabled=outVal.fecEnabled            --fecEnabled
                            fcTxPauseEn=outVal.fcTxPauseEn          --fcTxPauseEn
                            fcRxPauseEn=outVal.fcRxPauseEn          --fcRxPauseEn
                        end
                    end
                else
                    outVal=localApParamsPtr["apParamsPtr"]
                    postApPortNum = 0                --postApPortNum

                    ifMode      = string.sub("CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E",26, -3)      --ifMode
                    speed       = string.sub("CPSS_PORT_SPEED_NA_E",17, -3)                          --speed
                    hcdFound    = 0                  --hcdFound
                    fecEnabled  = 0                  --fecEnabled
                    fcTxPauseEn = false              --fcTxPauseEn
                    fcRxPauseEn = false              --fcRxPauseEn
                end
                postApPortNumStr=to_string(postApPortNum)
                hcdFoundStr=to_string(hcdFound)
                fecEnabledStr=to_string(fecEnabled)
                fcTxPauseEnStr=to_string(fcTxPauseEn)
                fcRxPauseEnStr=to_string(fcRxPauseEn)

                -- Fourth resulting string formatting and adding.
                command_data["result"] =
                string.format("%-10s%-21s%-21s%-21s%-21s",
                        devnum_portnum_string,
                        alignLeftToCenterStr(postApPortNumStr, 20),
                        alignLeftToCenterStr(ifMode, 20),
                        alignLeftToCenterStr(speed, 20),
                        alignLeftToCenterStr(hcdFoundStr, 20))
                command_data:addResultToResultArray()

                -- Fifth resulting string formatting and adding.
                command_data["result"] =
                string.format("%-10s%-21s%-21s%-21s",
                        devnum_portnum_string,
                        alignLeftToCenterStr(fecEnabledStr, 20),
                        alignLeftToCenterStr(fcTxPauseEnStr, 20),
                        alignLeftToCenterStr(fcRxPauseEnStr, 20))
                command_data:addResultToResultArray()


                command_data:updatePorts()
            end
        end
    end

    -- First resulting table string formatting.
    command_data:setResultArrayToResultStr(1, 2)
    command_data:setResultStrOnPortCount(header_string1, command_data["result"],
                                         footer_string1,
                                         "There is no port information to show.\n")

    -- Second resulting table string formatting.
    command_data:addToResultStrOnPortCount(
                header_string2, command_data:getInterlacedResultArray(2, 2),
                footer_string2)

    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end



-- ************************************************************************
---
--  px_show_interfaces_ap_status_single_port
--        @description  shows interfaces_ap_status with single port
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
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function px_show_interfaces_ap_status_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum

    local ret, localApEnablePtr
    local localApParamsPtr

    local outVal
    local postApPortNum

    local portMode
    local ifMode
    local speed
    local hcdFound
    local fecEnabled
    local fcTxPauseEn
    local fcRxPauseEn

    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
    local GT_OK = 0
    -- Command specific variables initialization.

    command_data:clearLocalStatus()

    local values
    ret, values = myGenWrapper(
            "cpssPxPortPizzaArbiterDevStateGet", {
                { "IN", "GT_U8"  , "devNum", devNum},                                    -- devNum
                { "OUT","CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
            })
    if (ret == 0) then
        OutValues = values["pizzaDeviceStatePtr"]
    end

    -- Traffic statistic getting.
    ret, localApParamsPtr = myGenWrapper("cpssPxPortApEnableGet",{
      {"IN","GT_U8","devNum",devNum},
      {"OUT","GT_BOOL","apEnabled"}})

    if(localApParamsPtr["apEnabled"] == true) then
        ret, localApParamsPtr = myGenWrapper("cpssPxPortApPortStatusGet",{
              {"IN","GT_U8","devNum",devNum},
              {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
              {"OUT","CPSS_PX_PORT_AP_STATUS_STC","apParamsPtr"}})

        if (ret ~= GT_OK) then
            local index = string.find(returnCodes[ret],"=")
            if(ret ~= 4) then
                print("Error at show interfaces ap status command : cpssPxPortApPortStatusGet " .. devNum .. "," .. portNum .. " :" ..  string.sub(returnCodes[ret],index + 1))
            end
        else
            outVal=localApParamsPtr["apParamsPtr"]
            postApPortNum=outVal.postApPortNum           --postApPortNum

            portMode=outVal.portMode
            ifMode=string.sub(portMode.ifMode,26, -3)      --ifMode
            speed=string.sub(portMode.speed,17, -3)        --speed

            hcdFound=outVal.hcdFound                  --hcdFound
            fecEnabled=outVal.fecEnabled              --fecEnabled
            fcTxPauseEn=outVal.fcTxPauseEn            --fcTxPauseEn
            fcRxPauseEn=outVal.fcRxPauseEn            --fcRxPauseEn
        end
    else
        outVal=localApParamsPtr["apParamsPtr"]
        postApPortNum = 0                --postApPortNum

        ifMode      = string.sub("CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E",26, -3)      --ifMode
        speed       = string.sub("CPSS_PORT_SPEED_NA_E",17, -3)                          --speed
        hcdFound    = 0                  --hcdFound
        fecEnabled  = 0                  --fecEnabled
        fcTxPauseEn = false              --fcTxPauseEn
        fcRxPauseEn = false              --fcRxPauseEn
    end
    print("")
    print("Ap Port Status")
    print("--------------")
    print("AP Number:\t",postApPortNum)
    print("Mode:\t\t",ifMode)
    print("Speed:\t\t",speed)
    print("hcd Found:\t",hcdFound)
    print("Fec Enabled:\t",fecEnabled)
    print("fcTxPauseEn:\t",fcTxPauseEn)
    print("fcRxPauseEn:\t",fcRxPauseEn)
    -- Resulting string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStr(header_string, command_data["result"],
                              footer_string)

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end




-- ************************************************************************
---
--  px_show_interfaces_ap_status
--        @description  show interfaces ap
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
local function px_show_interfaces_ap_status(params)
    local result, values

    -- Common variables declaration
    local command_data = Command_Data()


    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end

    -- System specific data initialization.
    command_data:enablePausedPrinting()

    if true == command_data:isOnePortInPortRange() then
        result, values = px_show_interfaces_ap_status_single_port(params, command_data)
    else
        result, values = px_show_interfaces_ap_status_port_range(params, command_data)
    end
    return result, values
end

--------------------------------------------------------------------------------
-- command registration: show interfaces ap status
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces ap status", {
    func = px_show_interfaces_ap_status,
    help = "Interface(s) ap status",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }
})

