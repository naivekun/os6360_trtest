--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_show_dce_cut_through.lua
--*
--* DESCRIPTION:
--*       Show dump of Cut Through configuration
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************

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
--  showDceCutThroughConfiguration
--
--  @description Show dump of Cut-Through configuration
--
--  @param params -
--          params["device"] - specific device number
--
--  @return true on success, otherwise false and error message
--
--*******************************************************************************
local function showDceCutThroughConfiguration(params)
    local dataToPrint = ""
    local errors = ""
    local devNum = params["device"]
    local portNum, maxPortNum
    local ret, val
    local up, enable, untaggedEnable, packetSize, buffersLimit

    --
    -- getting max port number for device
    --
    ret, maxPortNum = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if (ret ~= LGT_OK) then
        errors = errors ..
                 "Could not get number of ports for device " ..
                 devNum .. ": " .. returnCodes[ret] .. "\n"
        maxPort = -1
    end


    --
    -- User Priority configuration
    --
    dataToPrint = dataToPrint ..
                  " \n" ..
                  "User Priority configuration\n" ..
                  "===========================\n" ..
                  "\n" ..
                  "+----+--------------------+\n" ..
                  "| UP | cut through status |\n" ..
                  "+----+--------------------+\n"

    for up = 0, 7 do
        ret, val = myGenWrapper("cpssPxCutThroughUpEnableGet", {
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "GT_U32", "up", up },
                { "OUT", "GT_BOOL", "enablePtr"}
            })
        if (ret == LGT_OK) then
            dataToPrint = dataToPrint ..
                          string.format("|  %s |      %-8s      |\n",
                                        up, boolToSt(val["enablePtr"]))
        else
            errors = errors ..
                     "cpssPxCutThroughUpEnableGet" ..
                     " devNum=" .. devNum ..
                     " up=" .. up ..
                     " failed: " .. returnCodes[ret] .. "\n"
        end
    end

    dataToPrint = dataToPrint ..
                  "+----+--------------------+\n\n"


    dataToPrint = printBlockOfData(dataToPrint, false)
    if (dataToPrint == nil) then
        return
    end


    --
    -- Port configuration
    --
    if (maxPort ~= -1) then
        dataToPrint = dataToPrint ..
                      "\n" ..
                      "Port configuration\n" ..
                      "==================\n" ..
                      "\n" ..
                      "+------+--------------------------------+-----------------+--------------+\n" ..
                      "|      |       cut through status       |                 |              |\n" ..
                      "| port +-------------+------------------+ min packet size | buffer limit |\n" ..
                      "|      | all packets | untagged packets |                 |              |\n" ..
                      "+------+-------------+------------------+-----------------+--------------+\n"

        for portNum = 0, maxPortNum do
            if (does_port_exist(devNum, portNum)) then
                ret, val = myGenWrapper("cpssPxCutThroughPortEnableGet", {
                        { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "OUT", "GT_BOOL", "enablePtr" },
                        { "OUT", "GT_BOOL", "untaggedEnablePtr" }
                    })
                if (ret == LGT_OK) then
                    enable = boolToSt(val["enablePtr"])
                    untaggedEnable = boolToSt(val["untaggedEnablePtr"])
                else
                    enable = " ERROR "
                    untaggedEnable = " ERROR "

                    errors = errors ..
                             "cpssPxCutThroughPortEnableGet" ..
                             " devNum=" .. devNum ..
                             " portNum=" .. portNum ..
                             " failed: " ..
                             returnCodes[ret] .. "\n"
                end

                ret, val = myGenWrapper("cpssPxCutThroughMinimalPacketSizeGet", {
                        { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "OUT", "GT_U32", "sizePtr" }
                    })
                if (ret == LGT_OK) then
                    packetSize = val["sizePtr"]
                else
                    packetSize = "     ERROR     "

                    errors = errors ..
                             "cpssPxCutThroughMinimalPacketSizeGet" ..
                             " devNum=" .. devNum ..
                             " portNum=" .. portNum ..
                             " failed: " .. returnCodes[ret] .. "\n"
                end

                ret, val = myGenWrapper("cpssPxCutThroughMaxBuffersLimitGet", {
                        { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "OUT", "GT_U32", "buffersLimitPtr" }
                    })
                if (ret == LGT_OK) then
                    buffersLimit = val["buffersLimitPtr"]
                else
                    buffersLimit = "   ERROR   "

                    errors = errors ..
                             "cpssPxCutThroughMinimalPacketSizeGet" ..
                             " devNum=" .. devNum ..
                             " portNum=" .. portNum ..
                             " failed: " .. returnCodes[ret] .. "\n"
                end


                dataToPrint = dataToPrint ..
                              string.format("|  %2s  |  %8s   |     %-8s     | %15s | %12s |\n",
                                            portNum,
                                            enable,
                                            untaggedEnable,
                                            packetSize,
                                            buffersLimit)
            end
        end

        dataToPrint = dataToPrint ..
                      "+------+-------------+------------------+-----------------+--------------+\n\n"

        if (errors == "") then
            printBlockOfData(dataToPrint, true)
        else
            printBlockOfData(dataToPrint, false)

            dataToPrint = " \n\n" ..
                          "Errors\n" ..
                          "======\n" ..
                          "\n" ..
                          errors ..
                          "\n"

            printBlockOfData(dataToPrint, true)
        end
    end
end


--******************************************************************************
-- add help: show dce
--******************************************************************************
CLI_addHelp("exec", "show dce", "Display DCE configuration")

--******************************************************************************
-- command registration: show dce cut-through
--******************************************************************************
CLI_addCommand("exec", "show dce cut-through", {
    func = showDceCutThroughConfiguration,
    help = "Display Cut-Through configuration",
    params = {
        {
            type= "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            mandatory = {
                "device"
            }
        }
    }
})
