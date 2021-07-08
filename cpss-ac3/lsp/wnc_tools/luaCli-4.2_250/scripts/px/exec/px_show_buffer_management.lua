--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_buffer_management.lua
--*
--* DESCRIPTION:
--*       Show dump of Buffer Management information
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

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
--  showBufferManagement
--
--  @description  Show dump of Buffer Management information
--
--  @param params["device"] - specific device number
--
--*******************************************************************************
local function showBufferManagement(params)
    local devNum = params["device"]
    local dataToPrint = " "
    local errors = ""
    local portNum, maxPort
    local ret, val
    local xOnLimit, xOffLimit, rxBufLimit, numberOfUniqPackets, fcProfileNum
    local txDmaBurstLimitStatus, almostFullThreshold, fullThreshold

    --getting max port for device
    ret, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "Couldn't get number of ports for device " .. devNum .. ": " ..
                 returnCodes[ret] .. "\n"
        maxPort = -1
    end


    --
    -- Global FC configuration
    --
    ret, val = myGenWrapper("cpssPxPortBufMgGlobalXoffLimitGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "xoffLimitPtr" }
        })
    if (ret ~= LGT_OK) then
        errors = errors ..
                "cpssPxPortBufMgGlobalXoffLimitGet failed: " ..
                returnCodes[ret] .. "\n"
    end

    xOffLimit = val["xoffLimitPtr"]

    ret, val = myGenWrapper("cpssPxPortBufMgGlobalXonLimitGet",{
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "xonLimitPtr" }
        })
    if (ret ~= LGT_OK) then
       errors = errors ..
                "cpssPxPortBufMgGlobalXonLimitGet failed: " ..
                returnCodes[ret] .. "\n"
    end

    xOnLimit = val["xonLimitPtr"]

    if ((xOnLimit ~= nil) or (xOffLimit ~= nil)) then
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "Global FC configuration\n" ..
                      "=======================\n" ..
                      "\n"

        if (xOnLimit ~= nil) then
            dataToPrint = dataToPrint ..
                          string.format("%-22s [ %5d ]\n",
                                        "Global XON limit:",
                                        xOnLimit)
        end
        if (xOffLimit ~= nil) then
            dataToPrint = dataToPrint ..
                          string.format("%-22s [ %5d ]\n",
                                        "Global XOFF limit:",
                                        xOffLimit)
        end

        dataToPrint = dataToPrint .. "\n"
    end


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return
    end


    --
    -- Port to FC Profile binding
    --
    dataToPrint = dataToPrint ..
                  "\n" ..
                  "Port to FC Profile binding\n" ..
                  "==========================\n" ..
                  "\n" ..
                  "+------+-----------------------------+\n" ..
                  "| port |           profile           |\n" ..
                  "+------+-----------------------------+\n"

    for portNum = 0, maxPort do
        -- skip not existed ports
        if does_port_exist(devNum, portNum) then
            ret, val = myGenWrapper("cpssPxPortBufMgRxProfileGet",{
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "CPSS_PORT_RX_FC_PROFILE_SET_ENT", "profileSetPtr" }
                })
            if (ret ~= LGT_OK) then
                errors = errors ..
                         "cpssPxPortBufMgRxProfileGet port " .. portNum .. " failed: " ..
                         returnCodes[ret] .. "\n"
            else
                dataToPrint = dataToPrint ..
                              string.format("|  %2d  ", portNum) ..
                              "| " .. val["profileSetPtr"] .. " |\n"
            end
        end
    end

    dataToPrint = dataToPrint ..
                  "+------+-----------------------------+\n\n"


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return
    end


    --
    -- FC Profile configuration
    --
    dataToPrint = dataToPrint ..
                  "\n" ..
                  "FC Profile configuration\n" ..
                  "========================\n" ..
                  "\n" ..
                  "+-----------------------------+--------------+-----------+------------+\n" ..
                  "|           profile           | buffer limit | XON limit | XOFF limit |\n" ..
                  "+-----------------------------+--------------+-----------+------------+\n"

    for fcProfileNum = 0, 7 do
        ret, val = myGenWrapper("cpssPxPortBufMgProfileRxBufLimitGet", {
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "CPSS_PORT_RX_FC_PROFILE_SET_ENT", "profileSet", fcProfileNum },
                { "OUT", "GT_U32", "rxBufLimitPtr" }
            })
        if (ret == LGT_OK) then
            rxBufLimit = string.format("%12d", val["rxBufLimitPtr"])
        else
            if (ret == LGT_NOT_APPLICABLE_DEVICE) then
                rxBufLimit = "    N/A     "
            else
                rxBufLimit = "   ERROR    "
            end

            errors = errors ..
                     "cpssPxPortBufMgProfileRxBufLimitGet profile " ..
                     fcProfileNum .. " failed: " .. returnCodes[ret] .. "\n"
        end

        ret, val = myGenWrapper("cpssPxPortBufMgProfileXoffLimitGet",{
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "CPSS_PORT_RX_FC_PROFILE_SET_ENT", "profileSet", fcProfileNum },
                { "OUT", "GT_U32", "xoffLimitPtr" }
            })
        if (ret == LGT_OK) then
            xOffLimit = string.format("%10d", val["xoffLimitPtr"])
        else
            if (ret == LGT_NOT_APPLICABLE_DEVICE) then
                xOffLimit = "    N/A    "
            else
                xOffLimit = "   ERROR   "
            end

            errors = errors ..
                     "cpssPxPortBufMgProfileXoffLimitGet profile " ..
                     fcProfileNum .. " failed: " .. returnCodes[ret] .. "\n"
        end

        ret, val = myGenWrapper("cpssPxPortBufMgProfileXonLimitGet",{
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "CPSS_PORT_RX_FC_PROFILE_SET_ENT", "profileSet", fcProfileNum },
                { "OUT", "GT_U32", "xonLimitPtr" }
            })
        if (ret == LGT_OK) then
            xOnLimit = string.format("%9d", val["xonLimitPtr"])
        else
            if (ret == LGT_NOT_APPLICABLE_DEVICE) then
                xOnLimit = "   N/A    "
            else
                xOnLimit = "  ERROR   "
            end

            errors = errors ..
                     "cpssPxPortBufMgProfileXonLimitGet profile " .. fcProfileNum ..
                     " failed: " .. returnCodes[ret] .. "\n"
        end

        dataToPrint = dataToPrint ..
                      "| CPSS_PORT_RX_FC_PROFILE_" .. (fcProfileNum + 1) .. "_E " ..
                      "| " .. rxBufLimit .. " " ..
                      "| " .. xOnLimit .. " " ..
                      "| " .. xOffLimit .. " |\n"
    end

    dataToPrint = dataToPrint ..
                  "+-----------------------------+--------------+-----------+------------+\n\n"


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return
    end


    --
    -- Resources allocation
    --
    dataToPrint = dataToPrint ..
                  "\n" ..
                  "Resources allocation\n" ..
                  "====================\n" ..
                  "\n"

    ret, val = myGenWrapper("cpssPxPortBufMgGlobalPacketNumberGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "numOfPacketsPtr" }
        })
    if (ret == LGT_OK) then
        dataToPrint = dataToPrint ..
                      string.format("%-36s [ %10d ]\n",
                                    "Total amount of unique packets:",
                                    val["numOfPacketsPtr"])
    else
        errors = errors ..
                 "cpssPxPortBufMgGlobalPacketNumberGet failed: " ..
                 returnCodes[ret] .. "\n"
    end

    ret,val = myGenWrapper("cpssPxPortBufMgGlobalRxBufNumberGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "GT_U32", "numOfBuffersPtr" }
        })
    if (ret == LGT_OK) then
        dataToPrint = dataToPrint ..
                      string.format("%-36s [ %10d ]\n",
                                    "Total amount of allocated buffers:",
                                    val["numOfBuffersPtr"])
    else
        errors = errors ..
                 "cpssPxPortBufMgGlobalRxBufNumberGet failed: " ..
                 returnCodes[ret] .. "\n"
    end

    dataToPrint = dataToPrint ..
                  "\n" ..
                  "+------+-------------------+\n" ..
                  "| port | allocated buffers |\n" ..
                  "+------+-------------------+\n"

    for portNum = 0,maxPort do
        -- skip not existed ports
        if does_port_exist(devNum, portNum) then
            ret, val = myGenWrapper("cpssPxPortBufMgRxBufNumberGet", {
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "GT_U32", "numOfBuffersPtr" }
                })
            if (ret == LGT_OK) then
                dataToPrint = dataToPrint ..
                              string.format("|  %2d  | %17d |\n",
                                            portNum,
                                            val["numOfBuffersPtr"])
            else
                errors = errors ..
                         "cpssPxPortBufMgRxBufNumberGet port " .. portNum ..
                         "failed: " .. returnCodes[ret] .. "\n"
            end
        end
    end

    dataToPrint = dataToPrint ..
                  "+------+-------------------+\n\n"


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return
    end


    --
    -- TXDMA burst limit thresholds
    --
    dataToPrint = dataToPrint ..
                  "\n" ..
                  "TX DMA burst limit thresholds\n" ..
                  "=============================\n" ..
                  "\n" ..
                  "+------+----------+--------------------------+\n" ..
                  "|      |          |     threshold, bytes     |\n" ..
                  "| port |  status  +-------------+------------+\n" ..
                  "|      |          | almost full |    full    |\n" ..
                  "+------+----------+-------------+------------+\n"

    for portNum = 0, maxPort do
        -- skip not existed ports
        if does_port_exist(devNum, portNum) then
            ret, val = myGenWrapper("cpssPxPortBufMgTxDmaBurstLimitEnableGet",{
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "GT_BOOL", "enablePtr" }
                })
            if (ret == LGT_OK) then
                if (val["enablePtr"]) then
                    txDmaBurstLimitStatus = "enabled "
                else
                    txDmaBurstLimitStatus = "disabled"
                end
            else
                txDmaBurstLimitStatus = " ERROR  "
                errors = errors ..
                         "cpssPxPortBufMgTxDmaBurstLimitEnableGet portNum " .. portNum ..
                         " failed: " .. returnCodes[ret] .. "\n"
            end

            ret, val = myGenWrapper("cpssPxPortBufMgTxDmaBurstLimitThresholdsGet", {
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "GT_U32", "almostFullThresholdPtr" },
                    { "OUT", "GT_U32", "fullThresholdPtr" }
                })
            if (ret == LGT_OK) then
                almostFullThreshold = string.format("%11d", val["almostFullThresholdPtr"])
                fullThreshold = string.format("%10d", val["fullThresholdPtr"])
            else
                almostFullThreshold = "   ERROR   "
                fullThreshold = "  ERROR   "
            end

            dataToPrint = dataToPrint ..
                          string.format("|  %2d  | %s | %s | %s |\n",
                                        portNum, txDmaBurstLimitStatus,
                                        almostFullThreshold, fullThreshold)
        end
    end

    dataToPrint = dataToPrint ..
                  "+------+----------+-------------+------------+\n\n"


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
-- command registration: show buffer-management
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show buffer-management", {
    func   = showBufferManagement,
    help   = "Show dump of Buffer Management information",
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
