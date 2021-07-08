--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_dce_qcn.lua
--*
--* DESCRIPTION:
--*       Show current QCN configuration
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


-- Minimal amount of lines that will be printed by printBlockOfData()
local PRV_PRINT_BLOCK_MIN_LINE_COUNT = 32

-- Default Ports list for showDceQcn()
local PRV_DEFAULT_PORT_LIST = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
}

-- Default TC list for showDceQcn()
local PRV_DEFAULT_TC_LIST = {
    0, 1, 2, 3, 4, 5, 6, 7
}

-- Default CN Profile list for showDceQcn()
local PRV_DEFAULT_PROFILE_LIST = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
}


--*******************************************************************************
--  boolToStr
--
--  @description Convert boolean to string
--
--  @param bool - boolean value
--
--  @return Converted to string boolean value: "enabled"/"disabled"
--
--*******************************************************************************
local function boolToStr(bool)
    if (bool == true) then
        return "enabled"
    elseif (bool == false) then
        return "disabled"
    else
        return "ERROR"
    end
end


--*******************************************************************************
--  packetLengthEnumToStr
--
--  @description Convert CPSS_PX_PORT_CN_PACKET_LENGTH_ENT enum value to string
--
--  @param enumValue - enum value
--
--  @return Converted to string enum value
--
--*******************************************************************************
local function packetLengthEnumToStr(enumValue)
    if (enumValue == "CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E") then
        return "original packet length"
    elseif (enumValue == "CPSS_PX_PORT_CN_LENGTH_1_5_KB_E") then
        return "1.5 KB"
    elseif (enumValue == "CPSS_PX_PORT_CN_LENGTH_2_KB_E") then
        return "2 KB"
    elseif (enumValue == "CPSS_PX_PORT_CN_LENGTH_10_KB_E") then
        return "10 KB"
    else
        return "ERROR: Wrong enum value: " .. enumValue
    end
end


--*******************************************************************************
--  decToBinStr
--
--  @description Convert integer to string with binary value
--
--  @param  intValue          - value that we want to convert
--          countOfByte       - count of byte on input value
--          isSeparatorNeeded - if true, every 4 bits will be dot-separated
--
--  @return Converted to string value
--
--*******************************************************************************
local function decToBinStr(intValue, countOfByte, isSeparatorNeeded)
    local str = ""
    local i, maxBitNumber, currBit

    intValue = math.floor(intValue)
    maxBitNumber = 8 * countOfByte - 1

    for i = 0, maxBitNumber do
        currBit = intValue % 2
        intValue = math.floor(intValue / 2)

        str = ""  .. currBit .. str

        if ((isSeparatorNeeded) and (((i % 4) == 3) and (i ~= maxBitNumber))) then
            str = "." .. str
        end
    end

    return str
end


--*******************************************************************************
--  dpToStr
--
--  @description Convert CPSS_DP_LEVEL_ENT enum value to string
--
--  @param dp - Drop Precedence enum value
--
--  @return Converted to string DP enum value
--
--*******************************************************************************
local function dpToStr(dp)
    if (dp == "CPSS_DP_GREEN_E") then
        return "green"
    elseif (dp == "CPSS_DP_YELLOW_E") then
        return "yellow"
    elseif (dp == "CPSS_DP_RED_E") then
        return "red"
    else
        return "ERROR: Wrong DP: " .. dp
    end
end


--*******************************************************************************
--  printBlockOfData
--
--  @description Print some text (dataToPrint) and print request to exit
--
--  @param dataToPrint - data that we want to print
--  @param isLastBlock - if false will print data with request to exit
--
--  @return nil - if we catch exit request,
--          ""  - new start value of dataToPrint
--
--*******************************************************************************
local function printBlockOfData(dataToPrint, isLastBlock)
    local i, reply
    local lines = splitString(" \n" .. dataToPrint, "\n")

    -- exit if amount of lines less then PRV_PRINT_BLOCK_MIN_LINE_COUNT
    if ((isLastBlock == false) and (#lines < PRV_PRINT_BLOCK_MIN_LINE_COUNT)) then
        return dataToPrint
    end

    -- print lines
    for i = 1, #lines do
        print(lines[i])
    end

    -- if this block is not last - print request to exit
    if (isLastBlock == false) then
        reply = cmdLuaCLI_readLine("Type <CR> to continue, Q<CR> to stop:")

        if ((reply == "Q") or (reply == "q")) then
            return nil
        else
            return ""
        end
    else
        return nil
    end
end


--*******************************************************************************
--  showDceQcnPortConfiguration
--
--  @description Print Congestion Notification per Port configuration
--
--  @param dataToPrint - data that we want to print
--  @param errors      - errors list
--  @param devNum      - device number
--  @param portList    - list of ports that will be shown
--
--  @return dataToPrint - updated value of dataToPrint
--          errors      - updated errors list
--
--*******************************************************************************
local function showDceQcnPortConfiguration(dataToPrint, errors, devNum, portList)
    local ret, val
    local portIndex, portNum
    local terminationStatus, flowControlStatus


    dataToPrint = dataToPrint ..
                  "+------+-------------+--------------+\n" ..
                  "| port | termination | flow control |\n" ..
                  "+------+-------------+--------------+\n"

    for portIndex = 1, #portList do
        portNum = portList[portIndex]

        -- termination
        ret, val = myGenWrapper("cpssPxPortCnTerminationEnableGet", {
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                { "OUT", "GT_BOOL", "enablePtr" }
            })
        if (ret ~= LGT_OK) then
            errors = errors ..
                     "cpssPxPortCnTerminationEnableGet" ..
                     " devNum=" .. devNum ..
                     " portNum=" .. portNum ..
                     " failed: " .. returnCodes[ret] .. "\n"

            terminationStatus = " ERROR "
        else
            terminationStatus = boolToStr(val["enablePtr"])
        end

        -- flow control
        ret, val = myGenWrapper("cpssPxPortCnFcEnableGet", {
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                { "OUT", "GT_BOOL", "enablePtr" }
            })
        if (ret ~= LGT_OK) then
            errors = errors ..
                     "cpssPxPortCnFcEnableGet" ..
                     " devNum=" .. devNum ..
                     " portNum=" .. portNum ..
                     " failed: " .. returnCodes[ret] .. "\n"

            flowControlStatus = " ERROR "
        else
            flowControlStatus = boolToStr(val["enablePtr"])
        end

        dataToPrint = dataToPrint ..
                      string.format("|  %2s  |   %-8s  |   %8s   |\n",
                                    portNum,
                                    terminationStatus,
                                    flowControlStatus)
    end

    dataToPrint = dataToPrint ..
                  "+------+-------------+--------------+\n"

    return dataToPrint, errors
end


--*******************************************************************************
--  showDceQcnCnProfileConfiguration
--
--  @description Print Congestion Notification Profile configuration per TC
--
--  @param dataToPrint      - data that we want to print
--  @param errors           - errors list
--  @param devNum           - device number
--  @param profileList      - list of profiles that will be shown
--  @param profileIndexFrom - index of first profile in list that we want to print
--                            Max. count of printed profiles is 2
--  @param tcList           - Traffic Classes list.
--  @param isHeaderNeeded   - is table header needed.
--                            Applicable values: true,false
--
--  @return dataToPrint - updated value of dataToPrint
--          errors      - updated errors list
--
--*******************************************************************************
local function showDceQcnCnProfileConfiguration(dataToPrint, errors, devNum,
                                                profileList, profileIndexFrom,
                                                tcList, isHeaderNeeded)
    local ret, val
    local profileIndex2, profileIndex, profileIndexTo, tcIndex

    -- calculate profileIndexTo
    if ((#profileList - profileIndexFrom) >= 1) then
        profileIndexTo = profileIndexFrom + 1           -- print just 2 profiles
    elseif (#profileList == profileIndexFrom) then
        profileIndexTo = profileIndexFrom
    else
        return dataToPrint, errors
    end

    -- add Table Header if needed
    if (isHeaderNeeded) then
        if (profileIndexTo > profileIndexFrom) then
            dataToPrint = dataToPrint ..
                          "+----+---------+-----------+-------+---------+-----------+-------+\n" ..
                          "|    | CN mode | threshold | alpha | CN mode | threshold | alpha |\n" ..
                          "+ TC +---------+-----------+-------+---------+-----------+-------+\n"
        else
            dataToPrint = dataToPrint ..
                          "+----+---------+-----------+-------+\n" ..
                          "|    | CN mode | threshold | alpha |\n" ..
                          "+ TC +---------+-----------+-------+\n"
        end
    end

    -- go over all profiles in list on specified range
    for profileIndex = profileIndexFrom, profileIndexTo, 2 do
        if ((profileIndex + 1) <= profileIndexTo) then
            dataToPrint = dataToPrint ..
                          string.format("|    |        CN profile %-2d        |        CN profile %-2d        |\n" ..
                                        "+----+---------+-----------+-------+---------+-----------+-------+\n",
                                        profileList[profileIndex], profileList[profileIndex + 1])
        else
            dataToPrint = dataToPrint ..
                          string.format("|    |        CN profile %-2d        |\n" ..
                                        "+----+---------+-----------+-------+\n",
                                        profileList[profileIndex])
        end

        -- go over all Traffic Classes
        for tcIndex = 1, #tcList do
            dataToPrint = dataToPrint ..
                          string.format("| %2d |", tcList[tcIndex])

            -- go over 2 (or 1) profiles in table line
            for profileIndex2 = 0, 1 do
                if ((profileIndex + profileIndex2) <= profileIndexTo) then
                    ret, val = myGenWrapper("cpssPxPortCnProfileQueueConfigGet", {
                            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                            { "IN",  "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT", "profileSet",
                                     "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_" ..
                                     profileList[profileIndex + profileIndex2] .. "_E" },
                            { "IN",  "GT_U32", "tcQueue", tcList[tcIndex] },
                            { "OUT", "CPSS_PX_PORT_CN_PROFILE_CONFIG_STC", "cnProfileCfgPtr" }
                        })
                    if (ret == LGT_OK) then
                        if (val["cnProfileCfgPtr"]["cnAware"] == true) then
                            dataToPrint = dataToPrint .. "  aware  |"
                        elseif (val["cnProfileCfgPtr"]["cnAware"] == false) then
                            dataToPrint = dataToPrint .. "  blind  |"
                        else
                            dataToPrint = dataToPrint .. "  ERROR  |"
                        end

                        dataToPrint = dataToPrint..
                                      string.format(" %9d | %5d |",
                                                    val["cnProfileCfgPtr"]["threshold"],
                                                    prvLuaCpssPxPortTxTailDropDbaAlphaToStr(
                                                            val["cnProfileCfgPtr"]["alpha"]))
                    else
                        dataToPrint = dataToPrint ..
                                      "     ERROR     | ERROR |"

                        errors = errors ..
                                 "cpssPxPortCnProfileQueueConfigGet" ..
                                 " devNum=" .. devNum ..
                                 " profileSet=" .. (profileList[profileIndex + profileIndex2]) ..
                                 " tcQueue=" .. (tcList[tcIndex]) ..
                                 " failed: " .. returnCodes[ret] .. "\n"
                    end
                end
            end

            dataToPrint = dataToPrint .. "\n"
        end

        -- print table footer
        if ((profileIndex + 1) <= profileIndexTo) then
            dataToPrint = dataToPrint ..
                          "+----+---------+-----------+-------+---------+-----------+-------+\n"
        else
            dataToPrint = dataToPrint ..
                          "+----+---------+-----------+-------+\n"
        end
    end

    return dataToPrint, errors
end


--*******************************************************************************
--  showDceQcnCnmTriggeringState
--
--  @description Print Congestion Notification Triggering State per Port,TC
--
--  @param dataToPrint    - data that we want to print
--  @param errors         - errors list
--  @param devNum         - device number
--  @param portList       - list of ports that will be shown
--  @param portIndexFrom  - index of first port in list that we want to print
--                          Max. count of printed ports is 2
--  @param tcList         - Traffic Classes list.
--  @param isHeaderNeeded - is table header needed.
--                          Applicable values: true,false
--
--  @return dataToPrint - updated value of dataToPrint
--          errors      - updated errors list
--
--*******************************************************************************
local function showDceQcnCnmTriggeringState(dataToPrint, errors, devNum, portList,
                                            portIndexFrom, tcList, isHeaderNeeded)
    local ret, val
    local portIndex2, portIndex, portIndexTo, tcIndex

    -- calculate portIndexTo
    if ((#portList - portIndexFrom) >= 1) then
        portIndexTo = portIndexFrom + 1
    elseif (#portList == portIndexFrom) then
        portIndexTo = portIndexFrom
    else
        return dataToPrint, errors
    end

    -- add Table Header if needed
    if (isHeaderNeeded) then
        if (portIndexTo > portIndexFrom) then
            dataToPrint = dataToPrint ..
                          "+----+--------------+----------+--------------+----------+\n" ..
                          "|    | snapshot of  |  sample  | snapshot of  |  sample  |\n" ..
                          "|    | queue length | interval | queue length | interval |\n" ..
                          "+ TC +--------------+----------+--------------+----------+\n"
        else
            dataToPrint = dataToPrint ..
                          "+----+--------------+----------+\n" ..
                          "|    | snapshot of  |  sample  |\n" ..
                          "|    | queue length | interval |\n" ..
                          "+ TC +--------------+----------+\n"
        end
    end

    -- go over all ports in list on specified range
    for portIndex = portIndexFrom, portIndexTo, 2 do
        if ((portIndex + 1) <= portIndexTo) then
            dataToPrint = dataToPrint ..
                          string.format("|    |         port %2d         |         port %2d         |\n" ..
                                        "+----+--------------+----------+--------------+----------+\n",
                                        portList[portIndex], portList[portIndex + 1])
        else
            dataToPrint = dataToPrint ..
                          string.format("|    |         port %2d         |\n" ..
                                        "+----+--------------+----------+\n",
                                        portList[portIndex])
        end

        -- go over all Traffic Classes
        for tcIndex = 1, #tcList do
            dataToPrint = dataToPrint ..
                          string.format("| %2d |", tcList[tcIndex])

            -- go over 2 (or 1) ports in table line
            for portIndex2 = 0, 1 do
                if ((portIndex + portIndex2) <= portIndexTo) then
                    ret, val = myGenWrapper("cpssPxPortCnMessageTriggeringStateGet", {
                            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                            { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",
                                     portList[portIndex + portIndex2] },
                            { "IN",  "GT_U8", "tcQueue", tcList[tcIndex] },
                            { "OUT", "GT_U32", "qcnSampleIntervalPtr" },
                            { "OUT", "GT_U32", "qlenOldPtr" }
                        })
                    if (ret == LGT_OK) then
                        dataToPrint = dataToPrint..
                                      string.format(" %12d | %8d |",
                                                    val["qlenOldPtr"],
                                                    val["qcnSampleIntervalPtr"])
                    else
                        dataToPrint = dataToPrint ..
                                      "     ERROR    |   ERROR  |"

                        errors = errors ..
                                 "cpssPxPortCnMessageTriggeringStateGet" ..
                                 " devNum=" .. devNum ..
                                 " portNum=" .. (portList[portIndex + portIndex2]) ..
                                 " tcQueue=" .. (tcList[tcIndex]) ..
                                 " failed: " .. returnCodes[ret] .. "\n"
                    end
                end
            end

            dataToPrint = dataToPrint .. "\n"
        end

        -- print table footer
        if ((portIndex + 1) <= portIndexTo) then
            dataToPrint = dataToPrint ..
                          "+----+--------------+----------+--------------+----------+\n"
        else
            dataToPrint = dataToPrint ..
                          "+----+--------------+----------+\n"
        end
    end

    return dataToPrint, errors
end


--*******************************************************************************
--  showDceQcn
--
--  @description Print Quantized Congestion Notification configuration
--
--  @param param["device"]      - device number
--  @param param["portList"]    - list of ports that will be shown
--  @param param["profileList"] - list of CN Profiles that will be shown
--  @param param["tcList"]      - list of Traffic Classes that will be shown
--
--*******************************************************************************
local function showDceQcn(params)
    local dataToPrint = ""
    local errors = ""
    local ret, val
    local devNum = params["device"]
    local portIndex, portNum, maxPortNum, portList
    local tcIndex, tc, tcList
    local profileIndex, profile, profileList
    local i, isHeaderNeeded


    if (params["portList"] == nil) then
        portList = PRV_DEFAULT_PORT_LIST
    else
        portList = params["portList"]
    end

    if (params["tcList"] == nil) then
        tcList = PRV_DEFAULT_TC_LIST
    else
        tcList = params["tcList"]
    end

    if (params["profileList"] == nil) then
        profileList = PRV_DEFAULT_PROFILE_LIST
    else
        profileList = params["profileList"]
    end


    --
    -- Global configuration
    --
    dataToPrint = dataToPrint ..
                  "Global Configuration\n" ..
                  "====================\n" ..
                  "\n"

    -- Status of CN mechanism
    ret, val = myGenWrapper("cpssPxPortCnModeEnableGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_BOOL", "enablePtr" }
        })
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "cpssPxPortCnModeEnableGet" ..
                 " devNum=" .. devNum ..
                 " failed: " .. returnCodes[ret] .. "\n"
    else
        dataToPrint = dataToPrint ..
                      "Congestion Notification status:                " ..
                      boolToStr(val["enablePtr"]) .. "\n"
    end

    -- Destination of CN messages
    ret, val = myGenWrapper("cpssPxPortCnQueueStatusModeEnableGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_BOOL", "enablePtr" },
            { "OUT", "GT_PORT_NUM", "targetPortPtr" }
        })
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "cpssPxPortCnQueueStatusModeEnableGet" ..
                 " devNum=" .. devNum ..
                 " failed: " .. returnCodes[ret] .. "\n"
    else
        dataToPrint = dataToPrint ..
                      "Destination of CN messages:                    "
        if (val["enablePtr"]) then
            dataToPrint = dataToPrint ..
                          "port " .. val["targetPortPtr"] .. "\n"
        else
            dataToPrint = dataToPrint ..
                          "sampled packet\'s source\n"
        end
    end

    -- CN frames packet length
    ret, val = myGenWrapper("cpssPxPortCnPacketLengthGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "CPSS_PX_PORT_CN_PACKET_LENGTH_ENT", "packetLengthPtr" }
        })
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "cpssPxPortCnPacketLengthGet" ..
                 " devNum=" .. devNum ..
                 " failed: " .. returnCodes[ret] .. "\n"
    else
        dataToPrint = dataToPrint ..
                      "CN triggering logic frame packet length:       " ..
                      packetLengthEnumToStr(val["packetLengthPtr"]) .. "\n"
    end

    -- Amount of buffers available for DBA for CN
    ret, val = myGenWrapper("cpssPxPortCnAvailableBuffGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "availableBuffPtr" }
        })
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "cpssPxPortCnAvailableBuffGet" ..
                 " devNum=" .. devNum ..
                 " failed: " .. returnCodes[ret] .. "\n"
    else
        dataToPrint = dataToPrint ..
                      "Amount of buffers available for DBA for CN:    " ..
                      val["availableBuffPtr"] .. "\n"
    end

    dataToPrint = dataToPrint .. "\n"


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return nil
    end


    --
    -- Feedback calculation configuration
    --
    ret, val = myGenWrapper("cpssPxPortCnFbCalcConfigGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC", "fbCalcCfgPtr" }
        })
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "cpssPxPortCnFbCalcConfigGet" ..
                 " devNum=" .. devNum ..
                 " failed: " .. returnCodes[ret] .. "\n"
    else
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "Feedback calculation configuration\n" ..
                      "==================================\n" ..
                      "\n" ..

                      "Exponent weight for Feedback calculation:      " ..
                      val["fbCalcCfgPtr"]["wExp"] .. "\n" ..

                      "Bits used for Quantized Feedback calculation:  0b" ..
                      decToBinStr(val["fbCalcCfgPtr"]["fbLsb"], 1, false) .. "\n" ..

                      "Delta inclusion in Feedback calculation:       " ..
                      boolToStr(val["fbCalcCfgPtr"]["deltaEnable"]) .. "\n" ..

                      "Minimum value of Feedback:                     " ..
                      val["fbCalcCfgPtr"]["fbMin"] .. "\n" ..

                      "Maximum value of Feedback:                     " ..
                      val["fbCalcCfgPtr"]["fbMax"] .. "\n" ..

                      "\n"
    end


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return nil
    end


    --
    -- Port Configuration
    --
    if (#portList > 0) then
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "Port Configuration\n" ..
                      "==================\n" ..
                      "\n"

        dataToPrint, errors = showDceQcnPortConfiguration(dataToPrint, errors,
                                                          devNum, portList)

        dataToPrint = dataToPrint .. "\n"

        dataToPrint = printBlockOfData(dataToPrint, false)
        if (dataToPrint == nil) then
            return nil
        end
    end


    --
    -- CN Sample Table
    --
    dataToPrint = dataToPrint ..
                  "\n" ..
                  "CN Sample Table\n" ..
                  "===============\n" ..
                  "\n" ..
                  "+-------+----------+------------------------+\n" ..
                  "| entry | interval | randomize range bitmap |\n" ..
                  "+-------+----------+------------------------+\n"

    for i = 0, 7 do
        ret, val = myGenWrapper("cpssPxPortCnSampleEntryGet", {
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "GT_U8", "entryIndex", i },
                { "OUT", "CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC", "entryPtr" }
            })
        if (ret ~= LGT_OK) then
            errors = errors ..
                     "cpssPxPortCnSampleEntryGet" ..
                     " devNum=" .. devNum ..
                     " entryIndex=" .. i ..
                     " failed: " .. returnCodes[ret] .. "\n"
        else
            dataToPrint = dataToPrint ..
                          string.format("|   %d   | %8s |   %s  |\n",
                                        i,
                                        val["entryPtr"]["interval"],
                                        decToBinStr(val["entryPtr"]["randBitmap"],
                                                    2, true))
        end
    end

    dataToPrint = dataToPrint ..
                  "+-------+----------+------------------------+\n\n"


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return nil
    end


    --
    -- CN Profile configuration
    --
    if ((#profileList > 0) and (#tcList > 0)) then
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "CN Profile configuration\n" ..
                      "========================\n" ..
                      "\n"

        isHeaderNeeded = true

        for profileIndex = 1, #profileList, 2 do
            dataToPrint, errors = showDceQcnCnProfileConfiguration(dataToPrint, errors,
                                                                   devNum, profileList,
                                                                   profileIndex, tcList,
                                                                   isHeaderNeeded)

            dataToPrint = printBlockOfData(dataToPrint, false)
            if (dataToPrint == nil) then
                return nil
            elseif (dataToPrint == "") then     -- if data has been printed, print
                isHeaderNeeded = true           -- Table Header again
            else                            -- if data has NOT been printed, do
                isHeaderNeeded = false      -- NOT print Table Header
            end
        end

        dataToPrint = dataToPrint .. "\n"
    end


    --
    -- CN Messages configuration
    --
    dataToPrint = dataToPrint ..
                  "\n" ..
                  "CN Messages configuration\n" ..
                  "=========================\n" ..
                  "\n"

    -- general configuration
    ret, val = myGenWrapper("cpssPxPortCnMessageGenerationConfigGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC", "cnmGenerationCfgPtr" }
        })
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "cpssPxPortCnMessageGenerationConfigGet" ..
                 " devNum=" .. devNum ..
                 " failed: " .. returnCodes[ret] .. "\n"
    else
        local cnmCfg = val["cnmGenerationCfgPtr"]

        dataToPrint = dataToPrint ..

                      "Packet type associated with the QCN frames on RX port:    " ..
                      cnmCfg["packetType"] .. "\n" ..

                      "New User Priority for QCN frames:                         " ..
                      cnmCfg["priority"] .. "\n" ..

                      "New Drop Precedence for QCN frames:                       " ..
                      dpToStr(cnmCfg["dropPrecedence"]) .. "\n" ..

                      "New Traffic Class for QCN frames:                         " ..
                      "TC" .. cnmCfg["tc4pfc"] .. "\n" ..

                      "Packet Type assignment for QCN frames from local device:  " ..
                      boolToStr(cnmCfg["localGeneratedPacketTypeAssignmentEnable"]) .. "\n" ..

                      "Packet Type assigned to QCN frames from local device:     " ..
                      cnmCfg["localGeneratedPacketType"] .. "\n\n"
    end

    if ((#portList > 0) and (#tcList > 0)) then
        -- CN Messages Triggering State table
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "CN Messages Triggering State table:\n"

        isHeaderNeeded = true

        for portIndex = 1, #portList, 2 do
            dataToPrint, errors = showDceQcnCnmTriggeringState(dataToPrint, errors,
                                                               devNum, portList,
                                                               portIndex, tcList,
                                                               isHeaderNeeded)

            dataToPrint = printBlockOfData(dataToPrint, false)
            if (dataToPrint == nil) then
                return nil
            elseif (dataToPrint == "") then -- if data has been printed, print Table Header again
                isHeaderNeeded = true
            else                            -- if data has NOT been printed, do NOT print Table Header
                isHeaderNeeded = false
            end
        end
    end


    -- CN Messages Target Ports Map
    if (#portList > 0) then
        dataToPrint = dataToPrint ..
                      "\n\n" ..
                      "CN Messages Target Ports Map:\n" ..
                      "+------+--------------------------------------------------+\n" ..
                      "|      |           target ports selected by \"+\"           |\n" ..
                      "| port +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+\n" ..
                      "|      | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14|15|16|\n" ..
                      "+------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+\n"

        for portIndex = 1, #portList do
            portNum = portList[portIndex]

            ret, val = myGenWrapper("cpssPxPortCnMessagePortMapEntryGet", {
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "CPSS_PX_PORTS_BMP", "portsBmpPtr" }
                })
            if (ret == LGT_OK) then
                dataToPrint = dataToPrint ..
                              string.format("|  %2s  |", portNum)

                for i = 0, 16 do
                    if (bit_and(val["portsBmpPtr"], bit_shl(1, i)) ~= 0) then
                        dataToPrint = dataToPrint .. " +|"
                    else
                        dataToPrint = dataToPrint .. "  |"
                    end
                end

                dataToPrint = dataToPrint .. "\n"
            else
                errors = errors ..
                         "cpssPxPortCnMessagePortMapEntryGet" ..
                         " devNum=" .. devNum ..
                         " portNum=" .. portNum ..
                         " failed: " .. returnCodes[ret] .. "\n"
            end
        end

        dataToPrint = dataToPrint ..
                      "+------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+\n\n"

        dataToPrint = printBlockOfData(dataToPrint, false)
        if (dataToPrint == nil) then
            return nil
        end
    end


    --
    -- 802.3x/PFC Pause time
    --
    if (#portList > 0) then
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "802.3x/PFC Pause time\n" ..
                      "=====================\n" ..
                      "\n" ..
                      "+------+---------------------------------------------------------------+\n" ..
                      "|      |                          timer index                          |\n" ..
                      "| port +-------+-------+-------+-------+-------+-------+-------+-------+\n" ..
                      "|      |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |\n" ..
                      "+------+-------+-------+-------+-------+-------+-------+-------+-------+\n"

        for portIndex = 1, #portList do
            portNum = portList[portIndex]

            dataToPrint = dataToPrint ..
                          string.format("|  %2s  |", portNum)

            for i = 0, 7 do
                ret, val = myGenWrapper("cpssPxPortCnFcTimerGet", {
                        { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "IN",  "GT_U32", "index", i },
                        { "OUT", "GT_U32", "timerPtr" }
                    })
                if (ret == LGT_OK) then
                    dataToPrint = dataToPrint ..
                                  string.format(" %5s |", val["timerPtr"])
                else
                    dataToPrint = dataToPrint ..
                                  " ERROR |"

                    errors = errors ..
                             "cpssPxPortCnFcTimerGet" ..
                             " devNum=" .. devNum ..
                             " portNum=" .. portNum ..
                             " index=" .. i ..
                             " failed: " .. returnCodes[ret] .. "\n"
                end
            end

            dataToPrint = dataToPrint .. "\n"
        end

        dataToPrint = dataToPrint ..
                      "+------+-------+-------+-------+-------+-------+-------+-------+-------+\n\n"

        dataToPrint = printBlockOfData(dataToPrint, false)
        if (dataToPrint == nil) then
            return nil
        end
    end

    --
    -- Print errors
    --
    if (errors == "") then
        printBlockOfData(dataToPrint, true)
    else
        dataToPrint = printBlockOfData(dataToPrint, false)
        if (dataToPrint == nil) then
            return nil
        end

        dataToPrint = dataToPrint ..
                      "\n" ..
                      "Errors\n" ..
                      "======\n" ..
                      "\n" ..
                      errors ..
                      "\n"

        printBlockOfData(dataToPrint, true)
    end
end


--******************************************************************************
-- add help: show dce
--******************************************************************************
CLI_addHelp("exec", "show dce", "Display DCE configuration")

--******************************************************************************
-- command registration: show dce qcn
--******************************************************************************
CLI_addCommand("exec", "show dce qcn", {
    func = showDceQcn,
    help = "Display Quantized Congestion Notification (QCN) configuration",
    params = {
        {
            type= "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "ports %dce_qcn_port_list",
                name = "portList",
                help = "The range of ports"
            },
            {
                format = "profiles %dce_qcn_profile_list",
                name = "profileList",
                help = "The range of CN Profiles"
            },
            {
                format = "tc %dce_qcn_tc_list",
                name = "tcList",
                help = "The range of Traffic Classes"
            },
            mandatory = {
                "device"
            }
        }
    }
})
