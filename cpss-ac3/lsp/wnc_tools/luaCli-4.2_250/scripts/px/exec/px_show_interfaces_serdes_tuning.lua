--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_serdes_tuning.lua
--*
--* DESCRIPTION:
--*       display the serdes tuning
--*
--*
--********************************************************************************



--constants



-- ************************************************************************
---
--  show_interfaces_serdes_tuning_port_range
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

local function show_interfaces_serdes_tuning_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, header_string2, header_string3, header_string4, footer_string1, footer_string2, footer_string3, footer_string4

    local ret, localSerdesRxPtr
    local ifMode
    local localSerdesTuningPtr
    local laneNum, serdesFrequency

    -- Command specific variables initialization.
    header_string1 =
      "\n" ..
      "Serdes Tuning Config\n" ..
      "--------------------\n" ..
      "Interface LaneNum DC         LF         HF        BW          LB \n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string1 = "\n"

    header_string2 =
      "\n" ..
      "Interface LaneNum EO         txAmp      txEmphAmp    txEmph1    DFE0\n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string2 = "\n"

    header_string3 =
      "\n" ..
      "Interface LaneNum DFE1        DFE2        DFE3       DFE4        DFE5\n" ..
      "--------- ------- ----------- ----------- ---------- ----------- ----------\n"
    footer_string3 = "\n"

    header_string4 =
      "\n" ..
      "Interface LaneNum DFE6       DFE7       DFE8       DFE9       DFE10      DFE11      DFE12\n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string4 = "\n"

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

    ret, localSerdesRxPtr = myGenWrapper("cpssPxPortInterfaceModeGet",{
    {"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
    {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
    ifMode=localSerdesRxPtr["ifModePtr"] 
    if(ret~=GT_OK) then
                    command_data:addError("Error at Serdes Rx command")
    end
    
    ret, localSerdesRxPtr = myGenWrapper("prvCpssPxPortIfModeSerdesNumGet",{
    {"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
    {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
    {"OUT","GT_U32","startSerdesPtr"},
    {"OUT","GT_U32","numOfSerdesLanesPtr"}})
    local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
    local numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
    if(ret~=GT_OK) then
                    command_data:addError("Error at Serdes Rx command")
    end

    for i=0,numOfSerdesLanesPtr-1,1 do
        local laneNum = i
        ret, localSerdesTuningPtr = myGenWrapper("cpssPxPortSerdesTuningGet",{
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","GT_U32","laneNum",laneNum},
        {"IN","CPSS_PORT_SERDES_SPEED_ENT","serdesFrequency",serdesFrequency},
        {"OUT","CPSS_PORT_SERDES_TUNE_STC","tuneValuesPtr"}})

        -- Traffic statistic getting.
        command_data:clearLocalStatus()

        command_data:updateStatus()

                if(ret~=GT_OK) then
                    command_data:addError("Error at Serdes Tuning command")
                end
                local OutValues=localSerdesTuningPtr["tuneValuesPtr"]
                local DC=OutValues.DC                       --DC
                local LF=OutValues.LF                       --LF
                local HF=OutValues.HF                       --HF
                local BW=OutValues.BW                       --BW
                local LB=OutValues.LB                       --LB
                local EO=OutValues.EO                       --EO
                local txAmp=OutValues.txAmp                 --txAmp
                local txEmphAmp=OutValues.txEmphAmp         --txEmphAmp
                local txEmph1=OutValues.txEmph1             --txEmph1
                local DFE0=OutValues.DFE[0]             --DFE Vals
                local DFE1=OutValues.DFE[1]
                local DFE2=OutValues.DFE[2]
                local DFE3=OutValues.DFE[3]
                local DFE4=OutValues.DFE[4]
                local DFE5=OutValues.DFE[5]
                local DFE6=OutValues.DFE[6]
                local DFE7=OutValues.DFE[7]
                local DFE8=OutValues.DFE[8]
                local DFE9=OutValues.DFE[9]
                local DFE10=OutValues.DFE[10]
                local DFE11=OutValues.DFE[11]
                local DFE12=OutValues.DFE[12]


                laneNum=to_string(i)
                -- First resulting string formatting and adding.
                DC=to_string(DC)
                LF=to_string(LF)
                HF=to_string(HF)
                BW=to_string(BW)
                LB=to_string(LB)
                command_data["result"] =
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s",
                                  devnum_portnum_string,
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(DC, 10),
                                  alignLeftToCenterStr(LF, 10),
                                  alignLeftToCenterStr(HF, 10),
                                  alignLeftToCenterStr(BW, 10),
                                  alignLeftToCenterStr(LB, 10))
                command_data:addResultToResultArray()

                -- Second resulting string formatting and adding.
                EO=to_string(EO)
                txAmp=to_string(txAmp)
                txEmphAmp=to_string(txEmphAmp)
                txEmph1=to_string(txEmph1)
                DFE0=to_string(DFE0)
                command_data["result"] =
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-9s",
                                  devnum_portnum_string,
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(EO, 10),
                                  alignLeftToCenterStr(txAmp, 10),
                                  alignLeftToCenterStr(txEmphAmp, 10),
                                  alignLeftToCenterStr(txEmph1, 10),
                                  alignLeftToCenterStr(DFE0, 8))
                command_data:addResultToResultArray()

                -- Third resulting string formatting and adding.
                DFE1=to_string(DFE1)
                DFE2=to_string(DFE2)
                DFE3=to_string(DFE3)
                DFE4=to_string(DFE4)
                DFE5=to_string(DFE5)
                command_data["result"] = 
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s",
                                  devnum_portnum_string,
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(DFE1, 10),
                                  alignLeftToCenterStr(DFE2, 10),
                                  alignLeftToCenterStr(DFE3, 10),
                                  alignLeftToCenterStr(DFE4, 10),
                                  alignLeftToCenterStr(DFE5, 10))
                command_data:addResultToResultArray()

                -- Fourth resulting string formatting and adding.
                DFE6=to_string(DFE6)
                DFE7=to_string(DFE7)
                DFE8=to_string(DFE8)
                DFE9=to_string(DFE9)
                DFE10=to_string(DFE10)
                DFE11=to_string(DFE11)
                DFE12=to_string(DFE12)
                command_data["result"] =
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s%-11s%-11s",
                                  devnum_portnum_string, 
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(DFE6, 10),
                                  alignLeftToCenterStr(DFE7, 10),
                                  alignLeftToCenterStr(DFE8, 10),
                                  alignLeftToCenterStr(DFE9, 10),
                                  alignLeftToCenterStr(DFE10, 10),
                                  alignLeftToCenterStr(DFE11, 10),
                                  alignLeftToCenterStr(DFE12, 10))
                command_data:addResultToResultArray()

                command_data:updatePorts()

            end
        end
        -- Resulting table string formatting.

        -- First resulting table string formatting.
        command_data:setResultArrayToResultStr(1, 4)
        command_data:setResultStrOnPortCount(header_string1, command_data["result"],
                                            footer_string1,
                                            "There is no port information to show.\n")

        -- Second resulting table string formatting.
        command_data:addToResultStrOnPortCount(
            header_string2, command_data:getInterlacedResultArray(2, 4),
            footer_string2)

        -- Third resulting table string formatting.
        command_data:addToResultStrOnPortCount(
            header_string3, command_data:getInterlacedResultArray(3, 4),
            footer_string3)

        -- Fourth resulting table string formatting.
        command_data:addToResultStrOnPortCount(
            header_string4, command_data:getInterlacedResultArray(4, 4),
            footer_string4)

        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
    end
    return command_data:getCommandExecutionResults()
end




-- ************************************************************************
---
--  show_interfaces_serdes_tuning_single_port
--        @description  shows interfaces_serdes_tuning with single port
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
local function show_interfaces_serdes_tuning_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string

    local ret, localSerdesRxPtr
    local ifMode
    local startSerdesPtr
    local numOfSerdesLanesPtr
    local localSerdesTuningPtr


    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
    if portNum == "CPU" then portNum = 16 end
    -- DevNum/PortNum string forming.
    devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                 '/' ..
                                                 tostring(portNum), 9)
    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()

    local GT_OK = 0
    -- Command specific variables initialization.
    local serdesFrequency=params.serdes_speed
    command_data:clearLocalStatus()
    local laneNum=params.laneNum

    ret, localSerdesRxPtr = myGenWrapper("cpssPxPortInterfaceModeGet",{
            {"IN","GT_SW_DEV_NUM","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
    ifMode=localSerdesRxPtr["ifModePtr"] 
    if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Rx command")
    end

    ret, localSerdesRxPtr = myGenWrapper("prvCpssPxPortIfModeSerdesNumGet",{
            {"IN","GT_SW_DEV_NUM","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
            {"OUT","GT_U32","startSerdesPtr"},
            {"OUT","GT_U32","numOfSerdesLanesPtr"}})
    startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
    numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
    if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Rx command")
    end
    -- Traffic statistic strings formatting and adding.

    -- Display Serdes Tuning
    if(laneNum~=nil or numOfSerdesLanesPtr==1) then
        ret, localSerdesTuningPtr = myGenWrapper("cpssPxPortSerdesTuningGet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","CPSS_PORT_SERDES_SPEED_ENT","serdesFrequency",serdesFrequency},
                {"OUT","CPSS_PORT_SERDES_TUNE_STC","tuneValuesPtr"}})

        if(ret~=GT_OK) then
            print("Error at Serdes Tuning command")
        end

        local OutValues=localSerdesTuningPtr["tuneValuesPtr"]
        local DC=OutValues.DC
        local LF=OutValues.LF
        local HF=OutValues.HF
        local BW=OutValues.BW
        local LB=OutValues.LB
        local EO=OutValues.EO
        local txAmp=OutValues.txAmp
        local txEmphAmp=OutValues.txEmphAmp
        local txEmph1=OutValues.txEmph1
        local DFE=OutValues.DFE

        print("")
        print("Serdes Tuning")
        print("-------------")
        print("DC:\t\t\t",DC)
        print("LF:\t\t\t",LF)
        print("HF:\t\t\t",HF)
        print("BW:\t\t\t",BW)
        print("LB:\t\t\t",LB)
        print("EO:\t\t\t",EO)
        print("Amplitude:\t\t",txAmp)
        print("EmphAmp:\t\t",txEmphAmp)
        print("Emph Gen1 bit rates:\t",txEmph1)
        print("DFE Coefficient 1:\t",DFE[0])
        print("DFE Coefficient 2:\t",DFE[1])
        print("DFE Coefficient 3:\t",DFE[2])
        print("DFE Coefficient 4:\t",DFE[3])
        print("DFE Coefficient 5:\t",DFE[4])
        print("DFE Coefficient 6:\t",DFE[5])
        print("DFE Coefficient 7:\t",DFE[6])
        print("DFE Coefficient 8:\t",DFE[7])
        print("DFE Coefficient 9:\t",DFE[8])
        print("DFE Coefficient 10:\t",DFE[9])
        print("DFE Coefficient 11:\t",DFE[10])
        print("DFE Coefficient 12:\t",DFE[11])
        print("DFE Coefficient 13:\t",DFE[12])

        -- Resulting string formatting.

        command_data:setResultArrayToResultStr()

        command_data:analyzeCommandExecution()

        command_data:printCommandExecutionResults()
    else
        ret, localSerdesRxPtr = myGenWrapper("cpssPxPortInterfaceModeGet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
        ifMode=localSerdesRxPtr["ifModePtr"] 
        if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
        end

        ret, localSerdesRxPtr = myGenWrapper("prvCpssPxPortIfModeSerdesNumGet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                {"OUT","GT_U32","startSerdesPtr"},
                {"OUT","GT_U32","numOfSerdesLanesPtr"}})
        startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
        numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
        if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
        end
            header_string1 =
      "\n" ..
      "Serdes Tuning Config\n" ..
      "--------------------\n" ..
      "Interface LaneNum DC         LF         HF        BW          LB \n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string1 = "\n"

    header_string2 =
      "\n" ..
      "Interface LaneNum EO         txAmp      txEmphAmp    txEmph1    DFE0\n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string2 = "\n"

    header_string3 =
      "\n" ..
      "Interface LaneNum DFE1        DFE2        DFE3       DFE4        DFE5\n" ..
      "--------- ------- ----------- ----------- ---------- ----------- ----------\n"
    footer_string3 = "\n"

    header_string4 =
      "\n" ..
      "Interface LaneNum DFE6       DFE7       DFE8       DFE9       DFE10      DFE11      DFE12\n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string4 = "\n"

        for i=0,numOfSerdesLanesPtr-1,1 do
            local laneNum = i
            ret, localSerdesTuningPtr = myGenWrapper("cpssPxPortSerdesTuningGet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","CPSS_PORT_SERDES_SPEED_ENT","serdesFrequency",serdesFrequency},
                {"OUT","CPSS_PORT_SERDES_TUNE_STC","tuneValuesPtr"}})

                    -- Traffic statistic getting.
                    command_data:clearLocalStatus()

                    command_data:updateStatus()

            if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Tuning command")
            end
                local OutValues=localSerdesTuningPtr["tuneValuesPtr"]
                local DC=OutValues.DC                       --DC
                local LF=OutValues.LF                       --LF
                local HF=OutValues.HF                       --HF
                local BW=OutValues.BW                       --BW
                local LB=OutValues.LB                       --LB
                local EO=OutValues.EO                       --EO
                local txAmp=OutValues.txAmp                 --txAmp
                local txEmphAmp=OutValues.txEmphAmp         --txEmphAmp
                local txEmph1=OutValues.txEmph1             --txEmph1
                local DFE0=OutValues.DFE[0]             --DFE Vals
                local DFE1=OutValues.DFE[1]
                local DFE2=OutValues.DFE[2]
                local DFE3=OutValues.DFE[3]
                local DFE4=OutValues.DFE[4]
                local DFE5=OutValues.DFE[5]
                local DFE6=OutValues.DFE[6]
                local DFE7=OutValues.DFE[7]
                local DFE8=OutValues.DFE[8]
                local DFE9=OutValues.DFE[9]
                local DFE10=OutValues.DFE[10]
                local DFE11=OutValues.DFE[11]
                local DFE12=OutValues.DFE[12]


            laneNum=to_string(i)
                -- First resulting string formatting and adding.
                DC=to_string(DC)
                LF=to_string(LF)
                HF=to_string(HF)
                BW=to_string(BW)
                LB=to_string(LB)
                command_data["result"] = 
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s",
                                  devnum_portnum_string,
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(DC, 10),
                                  alignLeftToCenterStr(LF, 10),
                                  alignLeftToCenterStr(HF, 10),
                                  alignLeftToCenterStr(BW, 10),
                                  alignLeftToCenterStr(LB, 10))
                command_data:addResultToResultArray()

                -- Second resulting string formatting and adding.
                EO=to_string(EO)
                txAmp=to_string(txAmp)
                txEmphAmp=to_string(txEmphAmp)
                txEmph1=to_string(txEmph1)
                DFE0=to_string(DFE0)
                command_data["result"] =
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-9s",
                                  devnum_portnum_string, 
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(EO, 10),
                                  alignLeftToCenterStr(txAmp, 10),
                                  alignLeftToCenterStr(txEmphAmp, 10),
                                  alignLeftToCenterStr(txEmph1, 10),
                                  alignLeftToCenterStr(DFE0, 8))
                command_data:addResultToResultArray()

                -- Third resulting string formatting and adding.
                DFE1=to_string(DFE1)
                DFE2=to_string(DFE2)
                DFE3=to_string(DFE3)
                DFE4=to_string(DFE4)
                DFE5=to_string(DFE5)
                command_data["result"] = 
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s",
                                  devnum_portnum_string,
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(DFE1, 10),
                                  alignLeftToCenterStr(DFE2, 10),
                                  alignLeftToCenterStr(DFE3, 10),
                                  alignLeftToCenterStr(DFE4, 10),
                                  alignLeftToCenterStr(DFE5, 10))
                command_data:addResultToResultArray()

                -- Fourth resulting string formatting and adding.
                DFE6=to_string(DFE6)
                DFE7=to_string(DFE7)
                DFE8=to_string(DFE8)
                DFE9=to_string(DFE9)
                DFE10=to_string(DFE10)
                DFE11=to_string(DFE11)
                DFE12=to_string(DFE12)
                command_data["result"] =
                    string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s%-11s%-11s",
                                  devnum_portnum_string,
                                  alignLeftToCenterStr(laneNum, 7),
                                  alignLeftToCenterStr(DFE6, 10),
                                  alignLeftToCenterStr(DFE7, 10),
                                  alignLeftToCenterStr(DFE8, 10),
                                  alignLeftToCenterStr(DFE9, 10),
                                  alignLeftToCenterStr(DFE10, 10),
                                  alignLeftToCenterStr(DFE11, 10),
                                  alignLeftToCenterStr(DFE12, 10))
                command_data:addResultToResultArray()

                command_data:updatePorts()

        end
        -- Resulting table string formatting.

        -- First resulting table string formatting.
        command_data:setResultArrayToResultStr(1, 4)
        command_data:setResultStrOnPortCount(header_string1, command_data["result"],
                                            footer_string1,
                                            "There is no port information to show.\n")

        -- Second resulting table string formatting.
        command_data:addToResultStrOnPortCount(
            header_string2, command_data:getInterlacedResultArray(2, 4),
            footer_string2)

        -- Third resulting table string formatting.
        command_data:addToResultStrOnPortCount(
            header_string3, command_data:getInterlacedResultArray(3, 4),
            footer_string3)

        -- Fourth resulting table string formatting.
        command_data:addToResultStrOnPortCount(
            header_string4, command_data:getInterlacedResultArray(4, 4),
            footer_string4)

        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()

    end
    return command_data:getCommandExecutionResults()
end




-- ************************************************************************
---
--  show_interfaces_serdes_tuning
--        @description  show interfaces serdes tuning
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
local function show_interfaces_serdes_tuning(params)
    local  result, values
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
        result, values = show_interfaces_serdes_tuning_single_port(params, command_data)

    else
        result, values = show_interfaces_serdes_tuning_port_range(params, command_data)
    end

    return result, values
end

--------------------------------------------------------------------------------
-- command registration: show interfaces serdes tuning
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces serdes tuning", {
    func = show_interfaces_serdes_tuning,
    help = "Interface(s) serdes tuning",
    params = {
        { type= "named",   {format="serdes_speed %serdes_speed", name="serdes_speed",  help="Defines SERDES speed"  },
              {format="lane_num %lane_number_type", name="laneNum",     help="Serdes lane number"  },
              "#all_Eth",
              mandatory={"serdes_speed", "all_Eth"},
              requirements={
                ["all"] =       {"serdes_speed"},
                ["ethernet"] =     {"serdes_speed"},
                ["laneNum"] =     {"serdes_speed"}
              }
        }
    }
})


