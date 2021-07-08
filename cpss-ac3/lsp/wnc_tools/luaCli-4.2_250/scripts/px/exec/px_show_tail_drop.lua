--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_show_tail_drop.lua
--*
--* DESCRIPTION:
--*       Show tail drop dump
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--  Message print into tables when device is not applicable
local MSG_NOT_APPLICABLE_DEVICE = "N/A"

--  Message print into tables when API return NOT GT_OK
local MSG_ERROR = "ERROR"



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
--  prvLuaCpssPxPortTxTailDropDbaAlphaToStr
--
--  @description Convert CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT to string
--
--  @param alpha - alpha value
--
--  @return Converted to string value ("0" / "0.25" / "0.5" / "1" / "2" / "4")
--          on success, "fail" on fail
--
--*******************************************************************************
function prvLuaCpssPxPortTxTailDropDbaAlphaToStr(alpha)
    if (alpha == "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E") then
        return "0"
    elseif (alpha == "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E") then
        return "0.25"
    elseif(alpha == "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E") then
        return "0.5"
    elseif(alpha == "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E") then
        return "1"
    elseif(alpha == "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E") then
        return "2"
    elseif(alpha == "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E") then
        return "4"
    end

    return "fail"
end


--*******************************************************************************
--  checkReturnCode
--
--  @description Check API return code and set error message to val if needed
--
--  @param ret     - API return code
--         errors  - list of failed APIs with error descriptions
--         val     - API output values
--         api     - API name
--
--  @return Updated errors and val
--
--*******************************************************************************
local function checkReturnCode(ret, errors, val, api)
    if (ret == LGT_NOT_APPLICABLE_DEVICE) then
        val = { numberPtr = MSG_NOT_APPLICABLE_DEVICE }
    elseif (ret ~= LGT_OK) then
        errors = errors .. api .. " failed: " .. returnCodes[ret] .. "\n"
        val = { numberPtr = MSG_ERROR }
    end

    return errors, val
end


--*******************************************************************************
--  printBlockOfData
--
--  @description Print some text (dataToPrint) and print request to exit
--
--  @param dataToPrint - data that we want to print
--
--  @return nil   - if we catch exit request,
--          " \n" - new start value of dataToPrint
--
--*******************************************************************************
local function printBlockOfData(dataToPrint)
    local i, reply
    local lines = splitString(dataToPrint, "\n")

    for i = 1, #lines do
        print(lines[i])
    end

    reply = cmdLuaCLI_readLine("Type <CR> to continue, Q<CR> to stop:")

    if ((reply == "Q") or (reply == "q")) then
        return nil
    else
        return " \n"
    end
end


--*******************************************************************************
--  getTailDropConfiguration
--
--  @description Show tail drop
--
--  @param params -
--          params["device"] - specific device number
--
--  @return true on success, otherwise false and error message
--
--*******************************************************************************
local function getTailDropConfiguration(params)
    local dataToPrint = ""
    local errors = ""
    local devNum = params["device"]
    local maxPortNum
    local i, ret, val, temp
    local isFirstIteration, isNotApplicableDevice
    local profileIndex, dp

    local profileEnum = {
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_1_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_2_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_3_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_4_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_5_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_6_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_8_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_9_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_10_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_11_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_12_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_13_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_14_E",
        "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E"
    }


    --
    -- getting max port number for device
    --
    ret, maxPortNum = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if (ret ~= 0) then
        errors = errors .. "Could not get number of ports for device " ..
                 devNum .. ": " .. returnCodes[ret] .. "\n"
        maxPortNum = -1
    end


    --
    -- Number of buffers/descriptors
    --
    dataToPrint = dataToPrint .. " \n" ..
                  "Number of buffers/descriptors\n" ..
                  "=============================\n\n" ..
                  "+---------------------+-----------+-------------+\n" ..
                  "|                     |  Buffers  | Descriptors |\n" ..
                  "+---------------------+-----------+-------------+\n"

    -- Number of buffers/descriptors:  Total allocated
    ret, val = myGenWrapper("cpssPxPortTxTailDropGlobalBuffNumberGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "numberPtr" }
        })

    errors, temp = checkReturnCode(ret, errors, val,
                                   "cpssPxPortTxTailDropGlobalBuffNumberGet")

    ret, val = myGenWrapper("cpssPxPortTxTailDropGlobalDescNumberGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "numberPtr" }
        })

    errors, val = checkReturnCode(ret, errors, val,
                                  "cpssPxPortTxTailDropGlobalDescNumberGet")

    dataToPrint = dataToPrint .. string.format("| %-19s | %9s | %11s |\n",
                                               "Total allocated",
                                               temp["numberPtr"],
                                               val["numberPtr"])

    -- Number of buffers/descriptors:  MC allocated
    ret, val = myGenWrapper("cpssPxPortTxTailDropMcastBuffNumberGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "numberPtr" }
        })

    errors, temp = checkReturnCode(ret, errors, val,
                                   "cpssPxPortTxTailDropMcastBuffNumberGet")

    ret, val = myGenWrapper("cpssPxPortTxTailDropMcastDescNumberGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "numberPtr" }
        })

    errors, val = checkReturnCode(ret, errors, val,
                                  "cpssPxPortTxTailDropMcastDescNumberGet")

    dataToPrint = dataToPrint .. string.format("| %-19s | %9s | %11s |\n",
                                               "MC allocated",
                                               temp["numberPtr"],
                                               val["numberPtr"])

    -- Number of buffers/descriptors:  MC limit
    ret, val = myGenWrapper("cpssPxPortTxTailDropMcastBuffersLimitGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "mcastMaxBuffNumPtr" }
        })

    errors, temp = checkReturnCode(ret, errors, val,
                                   "cpssPxPortTxTailDropMcastBuffersLimitGet")

    ret, val = myGenWrapper("cpssPxPortTxTailDropMcastPcktDescLimitGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "mcastMaxDescNumPtr" }
        })

    errors, val = checkReturnCode(ret, errors, val,
                                  "cpssPxPortTxTailDropMcastPcktDescLimitGet")

    dataToPrint = dataToPrint .. string.format("| %-19s | %9s | %11s |\n",
                                               "MC limit",
                                               temp["mcastMaxBuffNumPtr"],
                                               val["mcastMaxDescNumPtr"])

    dataToPrint = dataToPrint .. "+---------------------+-----------+-------------+\n\n\n"


    --
    -- UC Tail Drop status
    --
    ret, val = myGenWrapper("cpssPxPortTxTailDropUcEnableGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_BOOL", "enablePtr" }
        })

    if (ret == LGT_NOT_APPLICABLE_DEVICE) then
        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "UC Tail Drop status:",
                                                   MSG_NOT_APPLICABLE_DEVICE)
    elseif (ret == LGT_OK) then
        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "UC Tail Drop status:",
                                                   boolToSt(val["enablePtr"]))
    else
        errors = errors .. "cpssPxPortTxTailDropUcEnableGet failed: " ..
                 returnCodes[ret] .. "\n"
    end


    --
    -- Tail Drop buffers allocation policy
    --
    ret, val = myGenWrapper("cpssPxPortTxTailDropDbaModeEnableGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_BOOL", "enablePtr" }
        })
    if (ret == LGT_NOT_APPLICABLE_DEVICE) then
        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "Tail Drop buffers allocation policy:",
                                                   MSG_NOT_APPLICABLE_DEVICE)
    elseif (ret == LGT_OK) then
        if (val["enablePtr"]) then
            temp = "dynamic"
        else
            temp = "static "
        end

        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "Tail Drop buffers allocation policy:",
                                                   temp)
    else
        errors = errors .. "cpssPxPortTxTailDropDbaModeEnableGet failed: " ..
                 returnCodes[ret] .. "\n"
    end


    --
    -- Tail Drop DBA buffer limit
    --
    ret, val = myGenWrapper("cpssPxPortTxTailDropDbaAvailableBuffGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "availableBuffPtr" }
        })
    if (ret == LGT_NOT_APPLICABLE_DEVICE) then
        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "Tail Drop DBA buffer limit:",
                                                   MSG_NOT_APPLICABLE_DEVICE)
    elseif (ret == LGT_OK) then
        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "Tail Drop DBA buffer limit:",
                                                   val["availableBuffPtr"])
    else
        errors = errors .. "cpssPxPortTxTailDropDbaAvailableBuffGet failed: " ..
                 returnCodes[ret] .. "\n"
    end


    --
    -- Tail Drop Packet Mode length
    --
    ret, val = myGenWrapper("cpssPxPortTxTailDropPacketModeLengthGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "lengthPtr" }
        })
    if (ret == LGT_NOT_APPLICABLE_DEVICE) then
        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "Tail Drop Packet Mode length:",
                                                   MSG_NOT_APPLICABLE_DEVICE)
    elseif (ret == LGT_OK) then
        dataToPrint = dataToPrint .. string.format("%-36s [ %8s ]\n",
                                                   "Tail Drop Packet Mode length:",
                                                   val["lengthPtr"])
    else
        errors = errors .. "cpssPxPortTxTailDropPacketModeLengthGet failed: " ..
                 returnCodes[ret] .. "\n"
    end

    dataToPrint = dataToPrint .. "\n"


    dataToPrint = printBlockOfData(dataToPrint)
    if (dataToPrint == nil) then
        return
    end


    --
    -- Tail Drop profiles
    --
    dataToPrint = dataToPrint .. "\n\n" ..
                  "Tail Drop profiles\n" ..
                  "==================\n\n"

    for profileIndex = 1, 16 do
        profile = profileEnum[profileIndex]
        ret, val = myGenWrapper("cpssPxPortTxTailDropProfileGet",
            {
                { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                  "profileSet", profile },
                { "OUT", "GT_U32", "portMaxBuffLimitPtr" },
                { "OUT", "GT_U32", "portMaxDescLimitPtr" },
                { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT",
                  "portAlphaPtr" }
            })

        if (ret ~= LGT_OK) then
            errors = errors .. "cpssPxPortTxTailDropProfileGet failed: " ..
                     returnCodes[ret] .. "\n"
            dataToPrint = dataToPrint .. string.format("Profile: %60s\n",
                                                       "[ " .. profile .. " ]")
        else
            dataToPrint = dataToPrint ..
                          string.format("Profile: %60s\n",
                                        "[ " .. profile .. " ]") ..
                          string.format("%-54s [ %10s ]\n",
                                        "Max buffers limit:",
                                        val["portMaxBuffLimitPtr"]) ..
                          string.format("%-54s [ %10s ]\n",
                                        "Max descriptors limit:",
                                        val["portMaxDescLimitPtr"]) ..
                          string.format("%-54s [ %10s ]\n",
                                        "Ratio of the free buffers:",
                                        prvLuaCpssPxPortTxTailDropDbaAlphaToStr(
                                                val["portAlphaPtr"]))
        end

        dataToPrint = dataToPrint ..
                      "+----+------------+------------+-----------------------------+" ..
                      "-----------------------------+-----------------------------+\n" ..
                      "|    |            |            |              DP0            |" ..
                      "              DP1            |              DP2            |\n" ..
                      "| TC |  Max Buff  |  Max Desc  +----------+----------+-------+" ..
                      "----------+----------+-------+----------+----------+-------+\n" ..
                      "|    |            |            | Max Buff | Max Desc | Alpha |" ..
                      " Max Buff | Max Desc | Alpha | Max Buff | Max Desc | Alpha |\n" ..
                      "+----+------------+------------+----------+----------+-------+" ..
                      "----------+----------+-------+----------+----------+-------+\n"

        for tcQueue = 0, 7 do
            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileTcGet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profileSet", profile },
                    { "IN", "GT_U32", "tc", tcQueue },
                    { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC",
                      "profileParamsPtr" }
                })

            if (ret ~= LGT_OK) then
                errors = errors .. "cpssPxPortTxTailDropProfileTcGet failed: " ..
                         returnCodes[ret] .. "\n"
                dataToPrint = dataToPrint ..
                              string.format("| %2s | %10s | %10s | %8s | %8s | %5s |" ..
                                            " %8s | %8s | %5s | %8s | %8s | %5s |\n",
                                            tcQueue, MSG_ERROR, MSG_ERROR, MSG_ERROR,
                                            MSG_ERROR, MSG_ERROR, MSG_ERROR, MSG_ERROR,
                                            MSG_ERROR, MSG_ERROR, MSG_ERROR, MSG_ERROR)
            else
                dataToPrint = dataToPrint ..
                              string.format("| %2s | %10s | %10s | %8s | %8s | %5s |" ..
                                            " %8s | %8s | %5s | %8s | %8s | %5s |\n",
                                            tcQueue,
                                            val["profileParamsPtr"]["tcMaxBuffNum"],
                                            val["profileParamsPtr"]["tcMaxDescNum"],
                                            val["profileParamsPtr"]["dp0MaxBuffNum"],
                                            val["profileParamsPtr"]["dp0MaxDescNum"],
                                            prvLuaCpssPxPortTxTailDropDbaAlphaToStr(
                                                    val["profileParamsPtr"]["dp0QueueAlpha"]),
                                            val["profileParamsPtr"]["dp1MaxBuffNum"],
                                            val["profileParamsPtr"]["dp1MaxDescNum"],
                                            prvLuaCpssPxPortTxTailDropDbaAlphaToStr(
                                                    val["profileParamsPtr"]["dp1QueueAlpha"]),
                                            val["profileParamsPtr"]["dp2MaxBuffNum"],
                                            val["profileParamsPtr"]["dp2MaxDescNum"],
                                            prvLuaCpssPxPortTxTailDropDbaAlphaToStr(
                                                    val["profileParamsPtr"]["dp2QueueAlpha"]))
            end
        end

        dataToPrint = dataToPrint ..
                      "+----+------------+------------+----------+----------+-------+" ..
                      "----------+----------+-------+----------+----------+-------+\n\n"

        if ((profileIndex % 2) == 0) then
            dataToPrint = printBlockOfData(dataToPrint)
            if (dataToPrint == nil) then
                return
            end
        end
    end


    --
    -- Random Tail Drop
    --
    dataToPrint = dataToPrint .. "\n\n" ..
                  "Random Tail Drop\n" ..
                  "================\n\n"

    isNotApplicableDevice = false

    for profileIndex = 1, 16 do
        profile = profileEnum[profileIndex]
        isFirstIteration = true

        for tcQueue = 0, 7 do
            for dp = 0, 2 do
                ret, val = myGenWrapper("cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet",
                    {
                        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                          "profile", profile },
                        { "IN", "GT_U32", "dp", dp },
                        { "IN", "GT_U32", "tc", tcQueue },
                        { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC",
                          "enablersPtr" }
                    })

                if (isFirstIteration) then
                    if (ret ~= LGT_NOT_APPLICABLE_DEVICE) then
                        dataToPrint = dataToPrint ..
                                      string.format("Tail Drop WRTD Profile:  %42s\n",
                                                    "[ " .. profile .. " ]") ..
                                      "+----+----+-----------+-----------+-----------+-------------------+\n" ..
                                      "| TC | DP | tcDpLimit | portLimit |  tcLimit  | shared Pool Limit |\n" ..
                                      "+----+----+-----------+-----------+-----------+-------------------+\n"

                        isFirstIteration = false
                    else
                        dataToPrint = dataToPrint .. "\nDevice is not applicable\n"
                        isNotApplicableDevice = true
                        break
                    end
                end

                if (ret ~= LGT_OK) then
                    errors = errors ..
                             "cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet failed: " ..
                             returnCodes[ret] .. "\n"
                else
                    dataToPrint = dataToPrint ..
                                  string.format("| %2s | %2s | %8s  | %8s  |" ..
                                                " %8s  |     %8s      |\n",
                                                tcQueue, dp,
                                                boolToSt(val["enablersPtr"]["tcDpLimit"]),
                                                boolToSt(val["enablersPtr"]["portLimit"]),
                                                boolToSt(val["enablersPtr"]["tcLimit"]),
                                                boolToSt(val["enablersPtr"]["sharedPoolLimit"]))
                end
            end

            if (isNotApplicableDevice) then
                break
            end
        end

        if (isNotApplicableDevice) then
            break
        end

        dataToPrint = dataToPrint ..
                      "+----+----+-----------+-----------+-----------+-------------------+\n\n"

        dataToPrint = printBlockOfData(dataToPrint)
        if (dataToPrint == nil) then
            return
        end
    end


    --
    -- Port to Tail Drop Profile binding
    --
    dataToPrint = dataToPrint .. "\n\n" ..
                  "Port to Tail Drop Profile binding\n" ..
                  "=================================\n\n" ..
                  "+------+----------------------------------------+\n" ..
                  "| Port |                Profile                 |\n" ..
                  "+------+----------------------------------------+\n"

    for portNum = 0, maxPortNum do
        if (does_port_exist(devNum, portNum)) then
            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileIdGet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profileSetPtr" }
                })

            if (ret ~= LGT_OK) then
                errors = errors .. "cpssPxPortTxTailDropProfileIdGet failed: " ..
                         returnCodes[ret] .. "\n"
            else
                dataToPrint = dataToPrint ..
                              string.format("| %3s  | %-38s |\n", portNum,
                                            val["profileSetPtr"])
            end
        end
    end

    dataToPrint = dataToPrint .. "+------+----------------------------------------+\n\n"


    -- append errors to output data
    if (errors ~= "") then
        dataToPrint = dataToPrint .. "\n\nErrors:\n=======\n\n" .. errors .. "\n\n"
    end


    local lines = splitString(dataToPrint, "\n")
    for i = 1, #lines do
        print(lines[i])
    end
end



--******************************************************************************
-- command registration: show tail-drop
--******************************************************************************
CLI_addCommand("exec", "show tail-drop", {
    func = getTailDropConfiguration,
    help = "The dump of all tail drop related information",
    params = {
        {
            type= "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            mandatory = { "device" }
        }
    }
})
