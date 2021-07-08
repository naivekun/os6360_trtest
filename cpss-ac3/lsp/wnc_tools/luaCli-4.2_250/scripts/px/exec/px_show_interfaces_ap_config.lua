--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_interfaces_ap_config.lua
--*
--* DESCRIPTION:
--*       showing the ap config of the port
--*
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssGtU64MathAdd")

--constants



-- ************************************************************************
---
--  px_show_interfaces_ap_config_port_range
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

local function px_show_interfaces_ap_config_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, header_string2, header_string3, header_string4, footer_string1, footer_string2, footer_string3, footer_string4
    local ret, localApEnablePtr
    local apEnable

    configGet=false
    -- Command specific variables initialization.

    header_string1 =
        "\n" ..
    "Ap Port Config\n" ..
    "--------------\n" ..
        "Interface  fcPause     fcAsmDir   fecSupport fecRequire noneceDisable   laneNum \n" ..
        "--------- ---------- ------------ ---------- ---------- ------------- ----------\n"
    footer_string1 = "\n"
    header_string2 =
        "\n" ..
        "Interface      Mode1      Speed1       fecAb1           fecReq1         Mode2      Speed2       fecAb2           fecReq2       Mode3       Speed3      fecAb3           fecReq3      \n" ..
        "--------- --------------- ------   -------------    -------------- --------------- ------   -------------    --------------  ------------  ------   -------------   --------------   \n"
    footer_string2 = "\n"

    header_string3 =
        "\n" ..
        "Interface      Mode4      Speed4      Mode5      Speed5      Mode6      Speed6      Mode7      Speed7      Mode8      Speed8      Mode9      Speed9      Mode10     Speed10\n" ..
        "--------- --------------- ------ --------------- ------ --------------- ------ --------------- ------ --------------- ------ --------------- ------ --------------- ------- \n"
    footer_string3 = "\n"

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
                        { "IN", "GT_U8"  , "devNum", devNum},                                   -- devNum
                        { "OUT","CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
                    })
            if (ret == 0) then
                OutValues = values["pizzaDeviceStatePtr"]
            end

            ret, localApEnablePtr = myGenWrapper("cpssPxPortApPortConfigGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_BOOL","apEnablePtr"},
                {"OUT","CPSS_PX_PORT_AP_PARAMS_STC","apParamsPtr"}})
            if (ret ~= GT_OK) then
                local index = string.find(returnCodes[ret],"=")
                if(ret ~= 4) then
                    print("Error at show interfaces ap config command : cpssPxPortApPortConfigGet " .. devNum .. "," .. portNum .. " :" ..  string.sub(returnCodes[ret],index + 1))
                end
            else
                apEnable=localApEnablePtr["apEnablePtr"]   --apEnable
                postApPortNum=0

                if(localApEnablePtr~=nil and localApEnablePtr["apParamsPtr"]~=nil) then
                    configGet=true
                    OutValues=localApEnablePtr["apParamsPtr"]
                    modesAdvertiseArr=OutValues.modesAdvertiseArr
                    mode4=string.sub(modesAdvertiseArr[3].ifMode,26,-3)
                    mode5=string.sub(modesAdvertiseArr[4].ifMode,26,-3)
                    mode6=string.sub(modesAdvertiseArr[5].ifMode,26,-3)
                    mode7=string.sub(modesAdvertiseArr[6].ifMode,26,-3)
                    mode8=string.sub(modesAdvertiseArr[7].ifMode,26,-3)
                    mode9=string.sub(modesAdvertiseArr[8].ifMode,26,-3)
                    mode10=string.sub(modesAdvertiseArr[9].ifMode,26,-3)
                    speed1=string.sub(modesAdvertiseArr[0].speed,17,-3)
                    speed2=string.sub(modesAdvertiseArr[1].speed,17,-3)
                    speed3=string.sub(modesAdvertiseArr[2].speed,17,-3)
                    speed4=string.sub(modesAdvertiseArr[3].speed,17,-3)
                    speed5=string.sub(modesAdvertiseArr[4].speed,17,-3)
                    speed6=string.sub(modesAdvertiseArr[5].speed,17,-3)
                    speed7=string.sub(modesAdvertiseArr[6].speed,17,-3)
                    speed8=string.sub(modesAdvertiseArr[7].speed,17,-3)
                    speed9=string.sub(modesAdvertiseArr[8].speed,17,-3)
                    speed10=string.sub(modesAdvertiseArr[9].speed,17,-3)
                    if (modesAdvertiseArr[0].speed=="CPSS_PORT_SPEED_NA_E") then
                        fecAb1 = "NA"
                        fecReq1 = "NA"
                        mode1 = "NA"
                    else
                        mode1=string.sub(modesAdvertiseArr[0].ifMode,26,-3)
                        fecAb1=string.sub(OutValues.fecAbilityArr[0],16,-7)
                        fecReq1=string.sub(OutValues.fecRequestedArr[0],16,-7)
                    end

                    if (modesAdvertiseArr[1].speed=="CPSS_PORT_SPEED_NA_E") then
                        fecAb2 = "NA"
                        fecReq2 = "NA"
                        mode2 = "NA"
                    else
                        fecAb2=string.sub(OutValues.fecAbilityArr[1],16,-7)
                        fecReq2=string.sub(OutValues.fecRequestedArr[1],16,-7)
                        mode2=string.sub(modesAdvertiseArr[1].ifMode,26,-3)
                    end
                    if (modesAdvertiseArr[2].speed=="CPSS_PORT_SPEED_NA_E") then
                        fecAb3 = "NA"
                        fecReq3 = "NA"
                        mode3 = "NA"
                    else
                        fecAb3=string.sub(OutValues.fecAbilityArr[2],16,-7)
                        fecReq3=string.sub(OutValues.fecRequestedArr[2],16,-7)
                        mode3=string.sub(modesAdvertiseArr[2].ifMode,26,-3)
                    end

                    fcPause=OutValues.fcPause
                    fcAsmDir=string.sub(OutValues.fcAsmDir,32,-3)
                    fecSupported=OutValues.fecSupported
                    fecRequired=OutValues.fecRequired
                    noneceDisable=OutValues.noneceDisable
                    laneNum=OutValues.laneNum

                    if(laneNum>1000) then
                        laneNum=0
                    end
                    -- First resulting string formatting and adding.
                    fcPause=to_string(fcPause)
                    fecSupported=to_string(fecSupported)
                    fecRequired=to_string(fecRequired)
                    noneceDisable=to_string(noneceDisable)
                    laneNum=to_string(laneNum)
                    command_data["result"] =
                    string.format("%-10s%-11s%-13s%-11s%-11s%-14s%-11s",
                          devnum_portnum_string,
                          alignLeftToCenterStr(fcPause, 10),
                          alignLeftToCenterStr(fcAsmDir, 12),
                          alignLeftToCenterStr(fecSupported, 10),
                          alignLeftToCenterStr(fecRequired, 10),
                          alignLeftToCenterStr(noneceDisable, 13),
                          alignLeftToCenterStr(laneNum, 10))
                    command_data:addResultToResultArray()

                    -- Second resulting string formatting and adding
                    command_data["result"] =
                    string.format("%-10s%-16s%-7s%-17s%-17s%-16s%-7s%-17s%-17s%-16s%-7s%-17s%-17s",
                          devnum_portnum_string,
                          alignLeftToCenterStr(mode1, 15),
                          alignLeftToCenterStr(speed1, 6),
                          alignLeftToCenterStr(fecAb1, 17),
                          alignLeftToCenterStr(fecReq1,17),
                          alignLeftToCenterStr(mode2, 15),
                          alignLeftToCenterStr(speed2, 6),
                          alignLeftToCenterStr(fecAb2, 17),
                          alignLeftToCenterStr(fecReq2,17),
                          alignLeftToCenterStr(mode3, 15),
                          alignLeftToCenterStr(speed3, 6),
                          alignLeftToCenterStr(fecAb3, 17),
                          alignLeftToCenterStr(fecReq3,17))
                    command_data:addResultToResultArray()

                    -- Third resulting string formatting and adding.
                    command_data["result"] =
                    string.format("%-10s%-16s%-7s%-16s%-7s%-16s%-7s%-16s%-7s%-16s%-7s%-16s%-7s%-16s%-7s",
                          devnum_portnum_string,
                          alignLeftToCenterStr(mode4, 15),
                          alignLeftToCenterStr(speed4, 6),
                          alignLeftToCenterStr(mode5, 15),
                          alignLeftToCenterStr(speed5, 6),
                          alignLeftToCenterStr(mode6, 15),
                          alignLeftToCenterStr(speed6, 6),
                          alignLeftToCenterStr(mode7, 15),
                          alignLeftToCenterStr(speed7, 6),
                          alignLeftToCenterStr(mode8, 15),
                          alignLeftToCenterStr(speed8, 6),
                          alignLeftToCenterStr(mode9, 15),
                          alignLeftToCenterStr(speed9, 6),
                          alignLeftToCenterStr(mode10, 15),
                          alignLeftToCenterStr(speed10, 6))
                    command_data:addResultToResultArray()

                    command_data:updatePorts()
                end
            end
        end
    end

    -- First resulting table string formatting.
    command_data:setResultArrayToResultStr(1, 3)
    command_data:setResultStrOnPortCount(header_string1, command_data["result"],
      footer_string1,
      "There is no port information to show.\n")

    -- Second resulting table string formatting.
    command_data:addToResultStrOnPortCount(
      header_string2, command_data:getInterlacedResultArray(2, 3),
      footer_string2)

    -- Third resulting table string formatting.
--    command_data:addToResultStrOnPortCount(
--      header_string3, command_data:getInterlacedResultArray(3, 3),
--      footer_string3)
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  px_show_interfaces_ap_config_single_port
--        @description  shows interfaces_ap_config with single port
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
local function px_show_interfaces_ap_config_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum

    local ret, localApEnablePtr
    local localApParamsPtr

    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
    local GT_OK = 0
    local stop = false
    -- Command specific variables initialization.

    command_data:clearLocalStatus()

    -- Traffic statistic getting.

    local values
    ret, values = myGenWrapper(
            "cpssPxPortPizzaArbiterDevStateGet", {
                { "IN", "GT_U8"  , "devNum", devNum},                                    -- devNum
                { "OUT","CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
            })
    if (ret == 0) then
        OutValues = values["pizzaDeviceStatePtr"]
    end

    -- Traffic statistic strings formatting and adding.
    ret, localApEnablePtr = myGenWrapper("cpssPxPortApPortConfigGet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"OUT","GT_BOOL","apEnablePtr"},
          {"OUT","CPSS_PX_PORT_AP_PARAMS_STC","apParamsPtr"}})
    if (ret ~= GT_OK) then
        local index = string.find(returnCodes[ret],"=")
        if(ret ~= 4) then
            print("Error at show interfaces ap config command : cpssPxPortApPortConfigGet " .. devNum .. "," .. portNum .. " :" ..  string.sub(returnCodes[ret],index + 1))
        end
    else
        local apEnable=localApEnablePtr["apEnablePtr"]   --apEnable
        print("")
        print("Ap Port Config")
        print("--------------")
        print("AP Enable:\t",apEnable)

        if(localApEnablePtr~=nil and localApEnablePtr["apParamsPtr"]~=nil) then
            OutValues=localApEnablePtr["apParamsPtr"]
            fcPause=OutValues.fcPause            --fcPause
            fcAsmDir=OutValues.fcAsmDir            --fcAsmDir
            fecSupported=OutValues.fecSupported        --fecSupported
            fecRequired=OutValues.fecRequired        --fecRequired
            noneceDisable=OutValues.noneceDisable      --noneceDisable
            laneNum=OutValues.laneNum            --laneNum
            modesAdvertiseArr=OutValues.modesAdvertiseArr --modesAdvertiseArr
            print("fcPause:\t",fcPause)
            print("fcAsmDir:\t",string.sub(fcAsmDir,32,-3))
            print("fecSupported:\t",fecSupported)
            print("fecRequired:\t",fecRequired)
            print("noneceDisable:\t",noneceDisable)
            print("laneNum:\t",laneNum)
            if(stop==false and modesAdvertiseArr[0]~=nil and modesAdvertiseArr[0].ifMode~=nil and modesAdvertiseArr[0].speed~=nil) then
                if (modesAdvertiseArr[0].speed=="CPSS_PORT_SPEED_NA_E") then
                    fecAb = "NA"
                    fecReq = "NA"
                    ifMode = "NA"
                    stop = true
                else
                    fecAb=string.sub(OutValues.fecAbilityArr[0],16,-7)
                    fecReq=string.sub(OutValues.fecRequestedArr[0],16,-7)
                    ifMode=string.sub(modesAdvertiseArr[0].ifMode,26,-3)
                end
                print("mode1:\t\t",ifMode,"\nspeed1:\t\t", string.sub(modesAdvertiseArr[0].speed,17,-3),"\nfecAbil1:\t",fecAb,"\nfecReq1:\t",fecReq)
            end
            if(stop==false and modesAdvertiseArr[1]~=nil and modesAdvertiseArr[1].ifMode~=nil and modesAdvertiseArr[1].speed~=nil) then
                if (modesAdvertiseArr[1].speed=="CPSS_PORT_SPEED_NA_E") then
                    fecAb = "NA"
                    fecReq = "NA"
                    ifMode = "NA"
                    stop = true
                else
                    fecAb=string.sub(OutValues.fecAbilityArr[1],16,-7)
                    fecReq=string.sub(OutValues.fecRequestedArr[1],16,-7)
                    ifMode=string.sub(modesAdvertiseArr[1].ifMode,26,-3)
                end
                print("mode2:\t\t",ifMode,"\nspeed2:\t\t", string.sub(modesAdvertiseArr[1].speed,17,-3),"\nfecAbil2:\t",fecAb,"\nfecReq2:\t",fecReq)
            end
            if(stop==false and modesAdvertiseArr[2]~=nil and modesAdvertiseArr[2].ifMode~=nil and modesAdvertiseArr[2].speed~=nil) then
                if (modesAdvertiseArr[2].speed=="CPSS_PORT_SPEED_NA_E") then
                    fecAb = "NA"
                    fecReq = "NA"
                    ifMode = "NA"
                    stop = true
                else
                    fecAb=string.sub(OutValues.fecAbilityArr[2],16,-7)
                    fecReq=string.sub(OutValues.fecRequestedArr[2],16,-7)
                    ifMode=string.sub(modesAdvertiseArr[2].ifMode,26,-3)
                end
                print("mode3:\t\t",ifMode,"\nspeed3:\t\t", string.sub(modesAdvertiseArr[2].speed,17,-3),"\nfecAbil3:\t",fecAb,"\nfecReq3:\t",fecReq)
            end
            if(stop==false and modesAdvertiseArr[3]~=nil and modesAdvertiseArr[3].ifMode~=nil and modesAdvertiseArr[3].speed~=nil) then
                if (modesAdvertiseArr[3].speed=="CPSS_PORT_SPEED_NA_E") then
                    fecAb = "NA"
                    fecReq = "NA"
                    ifMode = "NA"
                    stop = true
                else
                    fecAb=string.sub(OutValues.fecAbilityArr[3],16,-7)
                    fecReq=string.sub(OutValues.fecRequestedArr[3],16,-7)
                    ifMode=string.sub(modesAdvertiseArr[3].ifMode,26,-3)
                end
                print("mode4:\t\t",ifMode,"\nspeed4:\t\t", string.sub(modesAdvertiseArr[3].speed,17,-3),"\nfecAbil4:\t",fecAb,"\nfecReq4:\t",fecReq)
            end


--            if(modesAdvertiseArr[4]~=nil and modesAdvertiseArr[4].ifMode~=nil and modesAdvertiseArr[4].speed~=nil) then
--                print("mode5:\t\t",string.sub(modesAdvertiseArr[4].ifMode,26,-3),"\nspeed5:\t\t", string.sub(modesAdvertiseArr[4].speed,17,-3))
--            end
--            if(modesAdvertiseArr[5]~=nil and modesAdvertiseArr[5].ifMode~=nil and modesAdvertiseArr[5].speed~=nil) then
--                print("mode6:\t\t",string.sub(modesAdvertiseArr[5].ifMode,26,-3),"\nspeed6:\t\t", string.sub(modesAdvertiseArr[5].speed,17,-3))
--            end
--            if(modesAdvertiseArr[6]~=nil and modesAdvertiseArr[6].ifMode~=nil and modesAdvertiseArr[6].speed~=nil) then
--                print("mode7:\t\t",string.sub(modesAdvertiseArr[6].ifMode,26,-3),"\nspeed7:\t\t", string.sub(modesAdvertiseArr[6].speed,17,-3))
--            end
--            if(modesAdvertiseArr[7]~=nil and modesAdvertiseArr[7].ifMode~=nil and modesAdvertiseArr[7].speed~=nil) then
--                print("mode8:\t\t",string.sub(modesAdvertiseArr[7].ifMode,26,-3),"\nspeed8:\t\t", string.sub(modesAdvertiseArr[7].speed,17,-3))
--            end
--            if(modesAdvertiseArr[8]~=nil and modesAdvertiseArr[8].ifMode~=nil and modesAdvertiseArr[8].speed~=nil) then
--                print("mode9:\t\t",string.sub(modesAdvertiseArr[8].ifMode,26,-3),"\nspeed9:\t\t", string.sub(modesAdvertiseArr[8].speed,17,-3))
--            end
--            if(modesAdvertiseArr[9]~=nil and modesAdvertiseArr[9].ifMode~=nil and modesAdvertiseArr[9].speed~=nil) then
--                print("mode10:\t\t",string.sub(modesAdvertiseArr[9].ifMode,26,-3),"\nspeed10:\t", string.sub(modesAdvertiseArr[9].speed,17,-3))
--            end

        end

        -- Resulting string formatting.
        command_data:setResultArrayToResultStr()
        command_data:setResultStr(header_string, command_data["result"],
                                  footer_string)

        command_data:analyzeCommandExecution()

        command_data:printCommandExecutionResults()

    end
    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  px_show_interfaces_ap_config
--        @description  show interfaces ap config
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
local function px_show_interfaces_ap_config(params)
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
        result, values = px_show_interfaces_ap_config_single_port(params, command_data)
    else
        result, values = px_show_interfaces_ap_config_port_range(params, command_data)
    end
    return result, values
end

--------------------------------------------------------------------------------
-- command registration: show interfaces ap config
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces ap config", {
    func = px_show_interfaces_ap_config,
    help = "Interface(s) ap config",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }
})

