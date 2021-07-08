--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_interfaces_ap_stats.lua
--*
--* DESCRIPTION:
--*     Returns the AP port statistics information.
--*
--*
--*
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  ap_stats_func
--        @description  returns the ap port statistics information
--
--
--
--        @return       error message if fails
--
local function px_ap_stats_func(params)
    local command_data = Command_Data()
    local devNum, portNum, ret
    local GT_OK         = 0
    local apIntrop      = {}
    local outVals

    local header_string1 =
        "\n" ..
        "Ap Stats\n" ..
        "---------\n" ..
        "Interface   txDisCnt          abilityCnt    abilitySuccessCnt   linkFailCnt       \n" ..
        "--------- ---------------- ---------------- ----------------- ----------------    \n"
    local footer_string1 = "\n"

    local header_string2 =
        "\n" ..
        "Interface  linkSuccessCnt         hcdResoultionTime                    linkUpTime               \n" ..
        "--------- ---------------- -------------------------------- --------------------------------    \n"
    local footer_string2 = "\n"

    local tableInformation          = {}
    local currentPortInformation    = {}
    local portCount                 = 0
    local index

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

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                        command_data:getPortIterator() do
            command_data:clearPortStatus()
            if portNum == "CPU" then portNum = 16 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                         '/' ..
                                                         tostring(portNum), 9)
            -- Traffic statistic getting.
            command_data:clearLocalStatus()
            command_data:updateStatus()

            if(nil ~= devNum) then
                ret, localApParamsPtr = myGenWrapper("cpssPxPortApStatsGet",{
                    {"IN" ,"GT_U8"                          ,"devNum"       ,devNum },
                    {"IN" ,"GT_PHYSICAL_PORT_NUM"           ,"portNum"      ,portNum},
                    {"OUT","CPSS_PX_PORT_AP_STATS_STC"    ,"apStatsPtr"           },
                    {"OUT","GT_U16"                         ,"intropAbilityMaxIntervalPtr"}})
                if (ret ~= GT_OK) then
                    local index = string.find(returnCodes[ret],"=")
                    print("Error at ap stats command " .. devNum .. "/" .. portNum .. " : ".. string.sub(returnCodes[ret],index + 1))
                else
                    outVals                                             = localApParamsPtr["apStatsPtr"]
                    tableInformation[portCount] = {}
                    tableInformation[portCount].devPort                 = devNum .. "/" .. portNum
                    tableInformation[portCount].txDisCnt                = outVals.txDisCnt
                    tableInformation[portCount].abilityCnt              = outVals.abilityCnt
                    tableInformation[portCount].abilitySuccessCnt       = outVals.abilitySuccessCnt
                    tableInformation[portCount].linkFailCnt             = outVals.linkFailCnt
                    tableInformation[portCount].linkSuccessCnt          = outVals.linkSuccessCnt
                    tableInformation[portCount].hcdResoultionTime       = outVals.hcdResoultionTime
                    tableInformation[portCount].linkUpTime              = outVals.linkUpTime
                    portCount = portCount + 1
                end
            end
        end
    end

    if(portCount == 1) then
        print("\n" ..
              "Ap Stats\n" ..
              "--------\n")
        currentPortInformation = tableInformation[0]
        print("Interface:          "  .. currentPortInformation.devPort)
        print("txDisCnt:           "  .. currentPortInformation.txDisCnt)
        print("abilityCnt:         "  .. currentPortInformation.abilityCnt)
        print("abilitySuccessCnt:  "  .. currentPortInformation.abilitySuccessCnt)
        print("linkFailCnt:        "  .. currentPortInformation.linkFailCnt)
        print("linkSuccessCnt:     "  .. currentPortInformation.linkSuccessCnt)
        print("hcdResoultionTime:  "  .. currentPortInformation.hcdResoultionTime)
        print("linkUpTime:         "  .. currentPortInformation.linkUpTime)
        print("intropAbilityMaxInterval: " .. localApParamsPtr["intropAbilityMaxIntervalPtr"])
    end

    if(portCount > 1) then
        print(header_string1)
        for index = 0,portCount - 1, 1 do
            currentPortInformation = tableInformation[index]
            print(
                    string.format("%-9s" , alignLeftToCenterStr(currentPortInformation.devPort,9))                            .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.txDisCnt),16))               .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.abilityCnt),16))             .. " " ..
                    string.format("%-17s", alignLeftToCenterStr(to_string(currentPortInformation.abilitySuccessCnt),17))      .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.linkFailCnt),16))            .. " "
                    )
        end
        print(footer_string1)

        print(header_string2)
        for index = 0,portCount - 1, 1 do
            currentPortInformation = tableInformation[index]
            print(
                    string.format("%-9s" , alignLeftToCenterStr(currentPortInformation.devPort,9))                               .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.linkSuccessCnt),16))            .. " " ..
                    string.format("%-32s", alignLeftToCenterStr(to_string(currentPortInformation.hcdResoultionTime),32))         .. " " ..
                    string.format("%-32s", alignLeftToCenterStr(to_string(currentPortInformation.linkUpTime),32))                .. " "
                    )
        end
        print(footer_string2)
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: show interfaces ap stats
--------------------------------------------
CLI_addCommand("exec", "show interfaces ap stats", {
  func   = px_ap_stats_func,
  help   = "Returns the AP port statistics information.",
  params = {
        {
            type = "named",
            "#all_interfaces",
            mandatory = { "all_interfaces" }
        }
    }
  }
)

