--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_pfc.lua
--*
--* DESCRIPTION:
--*       Show PFC dump
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--*******************************************************************************
--  boolToSt
--
--  @description Convert boolean to string
--
--  @param bool - boolean value
--
--  @return Converted to string boolean value: "enabled"/"disabled"
--
--*******************************************************************************
local function boolToSt(bool)
    if (bool) then
        return "enabled"
    end

    return "disabled"
end


--*******************************************************************************
--  printBlockOfData
--
--  @description Print some text (dataToPrint) and print request to exit
--
--  @param dataToPrint - data that we want to print
--  @param isLastBlock - if false will print data with request to exit
--
--  @return nil   - if we catch exit request,
--          " \n" - new start value of dataToPrint
--
--*******************************************************************************
local function printBlockOfData(dataToPrint, isLastBlock)
    local i, reply
    local lines = splitString(dataToPrint, "\n")

    for i = 1, #lines do
        print(lines[i])
    end

    if (isLastBlock == false) then
        reply = cmdLuaCLI_readLine("Type <CR> to continue, Q<CR> to stop:")

        if ((reply == "Q") or (reply == "q")) then
            return nil
        else
            return " \n"
        end
    else
        return nil
    end
end


--*******************************************************************************
--  showPfc
--
--  @description  Show PFC dump
--
--  @param  params["device"] - specific device number
--
--  @return  true on success, otherwise false and error message
--
--*******************************************************************************
local function showPfc(params)
    local devNum = params["device"]
    local portNum, maxPort
    local ret, val
    local dataToPrint = ""
    local errors = ""
    local i, tc, temp, profile


    --
    -- Global PFC configuration
    --
    dataToPrint = dataToPrint ..
                  " \n" ..
                  "Global PFC configuration\n" ..
                  "========================\n" ..
                  "\n"

    -- PFC counting mode
    ret, val = myGenWrapper("cpssPxPortPfcCountingModeGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "CPSS_PX_PORT_PFC_COUNT_MODE_ENT", "pfcCountModePtr" }
        })
    if (ret == LGT_OK) then
        if (val["pfcCountModePtr"] == "CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E") then
            temp = "buffer"
        else
            temp = "packet"
        end

        dataToPrint = dataToPrint ..
                      "PFC counting mode:           " .. temp .. "\n"
    else
        errors = errors ..
                 "cpssPxPortPfcCountingModeGet failed: " .. returnCodes[ret] .. "\n"
    end

    -- Status of PFC response
    ret, val = myGenWrapper("cpssPxPortPfcEnableGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "CPSS_PX_PORT_PFC_ENABLE_ENT", "pfcEnablePtr" }
        })
    if (ret == LGT_OK) then
        if (val["pfcEnablePtr"] == "CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E") then
            temp = "triggering only"
        else
            temp = "triggering and response"
        end

        dataToPrint = dataToPrint ..
                      "Status of PFC response:      " .. temp .. "\n"
    else
        errors = errors ..
                 "cpssPxPortPfcEnableGet failed: " .. returnCodes[ret] .. "\n"
    end

    -- Status of PFC global drop
    ret, val = myGenWrapper("cpssPxPortPfcGlobalDropEnableGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_BOOL", "enablePtr" }
        })
    if (ret == LGT_OK) then
        dataToPrint = dataToPrint ..
                      "Status of PFC global drop:   " ..
                      boolToSt(val["enablePtr"]) .. "\n"
    else
        errors = errors ..
                 "cpssPxPortPfcGlobalDropEnableGet failed: " ..
                 returnCodes[ret] .. "\n"
    end

    dataToPrint = dataToPrint .. "\n"


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return
    end


    --getting max port for device
    ret, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "Could not get number of ports for device " .. devNum .. ": " ..
                 returnCodes[ret] .. " \n"
    else

        --
        -- PFC Port configuration
        --
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "Port PFC configuration\n" ..
                      "======================\n" ..
                      "\n" ..
                      "+------+---------------------+---------------------+\n" ..
                      "| port | bound profile index | bound counter index |\n" ..
                      "+------+---------------------+---------------------+\n"

        for portNum = 0, maxPort do
            -- skip not existed ports
            if does_port_exist(devNum, portNum) then
                -- bound profile index
                ret, val = myGenWrapper("cpssPxPortPfcProfileIndexGet", {
                        { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "OUT", "GT_U32", "profileIndexPtr" }
                    })
                if (ret == LGT_OK) then
                    profile = "  " .. val["profileIndexPtr"] .. "  "
                else
                    profile = "ERROR"
                    errors = errors ..
                             "cpssPxPortPfcProfileIndexGet portNum " .. portNum ..
                             " failed: " .. returnCodes[ret] .. "\n"
                end

                -- bound counter index
                ret, val = myGenWrapper("cpssPxPortPfcSourcePortToPfcCounterGet", {
                        { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "OUT", "GT_U32", "pfcCounterNumPtr" }
                    })
                if (ret == LGT_OK) then
                    temp = string.format(" %3d ", val["pfcCounterNumPtr"])
                else
                    temp = "ERROR"
                    errors = errors ..
                             "cpssPxPortPfcSourcePortToPfcCounterGet portNum " ..
                             portNum .. " failed: " .. returnCodes[ret] .. "\n"
                end

                dataToPrint = dataToPrint ..
                              string.format("|  %2d  |        %s        |        %s        |\n",
                                            portNum, profile, temp)
            end
        end

        dataToPrint = dataToPrint ..
                      "+------+---------------------+---------------------+\n\n"


        dataToPrint = printBlockOfData(dataToPrint, false)
        if (dataToPrint == nil) then
            return
        end


        --
        -- PFC Profile configuration
        --
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "PFC Profile configuration\n" ..
                      "=========================\n" ..
                      "\n"

        -- for (profile = 0; profile <= 7; profile += 2) ...
        for profile = 0, 7, 2 do
            if ((profile % 4) == 0) then
                dataToPrint = dataToPrint ..
                          "+----+---------------------------------------+" ..
                                "---------------------------------------+\n" ..
                          "|    |        XON        |       XOFF        |" ..
                                "        XON        |       XOFF        |\n" ..
                          "| TC +-----------+-------+-----------+-------+" ..
                                "-----------+-------+-----------+-------+\n" ..
                          "|    | threshold | ratio | threshold | ratio |" ..
                                " threshold | ratio | threshold | ratio |\n" ..
                          "+----+-----------+-------+-----------+-------+" ..
                                "-----------+-------+-----------+-------+\n"
            end

            dataToPrint = dataToPrint ..
                          "|    |               profile " .. profile ..
                                "               |" ..
                                "               profile " .. (profile + 1) ..
                                "               |\n" ..
                          "+----+-----------+-------+-----------+-------+" ..
                                "-----------+-------+-----------+-------+\n"

            for tc = 0, 7 do
                dataToPrint = dataToPrint ..
                              "|  " .. tc .. " |"
                for i = 0, 1 do
                    ret, val = myGenWrapper("cpssPxPortPfcProfileQueueConfigGet", {
                            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                            { "IN",  "GT_U32", "profileIndex", (profile + i) },
                            { "IN",  "GT_U32", "tcQueue", tc },
                            { "OUT", "CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC",
                                     "pfcProfileCfgPtr" }
                        })
                    if (ret == LGT_OK) then
                        temp = val["pfcProfileCfgPtr"]

                        dataToPrint = dataToPrint ..
                                      string.format(" %9s | %5s | %9s | %5s |",
                                                    temp["xonThreshold"],
                                                    prvLuaCpssPxPortTxTailDropDbaAlphaToStr(
                                                            temp["xonAlpha"]),
                                                    temp["xoffThreshold"],
                                                    prvLuaCpssPxPortTxTailDropDbaAlphaToStr(
                                                            temp["xoffAlpha"]))
                    else
                        dataToPrint = dataToPrint ..
                                      "   ERROR   | ERROR |   ERROR   | ERROR |"

                        errors = errors ..
                                 "cpssPxPortPfcProfileQueueConfigGet" ..
                                 " profileIndex " .. (profile + i) ..
                                 " tc " .. tc ..
                                 " failed: " .. returnCodes[ret] .. "\n"
                    end
                end

                dataToPrint = dataToPrint .. "\n"
            end

            dataToPrint = dataToPrint ..
                          "+----+-----------+-------+-----------+-------+" ..
                                "-----------+-------+-----------+-------+\n"

            if (((profile + 2) % 4) == 0) then
                dataToPrint = dataToPrint .. "\n"

                dataToPrint = printBlockOfData(dataToPrint, false)
                if (dataToPrint == nil) then
                    return
                end
            end
        end
    end


    --
    -- PFC Traffic Class Queue configuration
    --
    dataToPrint = dataToPrint ..
                  "\n" ..
                  "PFC Traffic Class Queue configuration\n" ..
                  "=====================================\n" ..
                  "\n" ..
                  "+----+---------------+----------------+----------------+\n" ..
                  "| TC | XON threshold | XOFF threshold | drop threshold |\n" ..
                  "+----+---------------+----------------+----------------+\n"

    for tc = 0, 7 do
        ret, val = myGenWrapper("cpssPxPortPfcGlobalQueueConfigGet", {
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "GT_U32", "tcQueue", tc },
                { "OUT", "GT_U32", "xoffThresholdPtr" },
                { "OUT", "GT_U32", "dropThresholdPtr" },
                { "OUT", "GT_U32", "xonThresholdPtr" }
            })
        if (ret == LGT_OK) then
            dataToPrint = dataToPrint ..
                          string.format("| %2s | %13s | %14s | %14s |\n",
                                        tc, val["xonThresholdPtr"],
                                        val["xoffThresholdPtr"],
                                        val["dropThresholdPtr"])
        else
            errors = errors ..
                     "cpssPxPortPfcGlobalQueueConfigGet tc " .. tc ..
                     " failed: " .. returnCodes[ret] .. "\n"
        end
    end

    dataToPrint = dataToPrint ..
                  "+----+---------------+----------------+----------------+\n\n"


    if (errors == "") then
        printBlockOfData(dataToPrint, true)
    else
        printBlockOfData(dataToPrint, false)

        dataToPrint = " \n" ..
                      "\n" ..
                      "Errors\n" ..
                      "======\n" ..
                      "\n" ..
                      errors ..
                      "\n"

        printBlockOfData(dataToPrint, true)
    end
end


--------------------------------------------------------------------------------
-- command registration: show pfc
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show pfc", {
    func   = showPfc,
    help   = "Show dump of PFC information",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            mandatory = { "device" }
        }
    }
})
