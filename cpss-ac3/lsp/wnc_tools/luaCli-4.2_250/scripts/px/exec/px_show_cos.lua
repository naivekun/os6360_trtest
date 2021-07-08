--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_show_cos.lua
--*
--* DESCRIPTION:
--*       Show CoS attributes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--******************************************************************************
--  printRequestToExit
--
--  @description Print request to exit
--
--  @return nil, if exit needed. Otherwise return empty string
--
--******************************************************************************
local function printRequestToExit()
    local reply

    reply = cmdLuaCLI_readLine("Type <CR> to continue, Q<CR> to stop:")

    if ((reply == "Q") or (reply == "q")) then
        return nil
    else
        return ""
    end
end


--*******************************************************************************
--  cosAttrToTableRow
--
--  @description Convert CPSS_PX_COS_ATTRIBUTES_STC to table row
--
--  @param cosAttr  - CoS Attributes variable
--
--  @return String that contain 4 column with CoS attributes
--
--*******************************************************************************
local function cosAttrToTableRow(cosAttr)
    local dp = "-"

    -- Convert Drop Precedence to integer
    if (cosAttr["dropPrecedence"] == "CPSS_DP_GREEN_E") then
        dp = "green "
    elseif (cosAttr["dropPrecedence"] == "CPSS_DP_YELLOW_E") then
        dp = "yellow"
    elseif (cosAttr["dropPrecedence"] == "CPSS_DP_RED_E") then
        dp = " red  "
    end

    return string.format(" %3s | %2s | %s | %2s |",
                         cosAttr["dropEligibilityIndication"],
                         cosAttr["userPriority"],
                         dp,
                         cosAttr["trafficClass"])
end


--*******************************************************************************
--  getCosPortAttr
--
--  @description Display CoS Attributes per Port
--
--  @param params["device"] - device number
--
--  @return true on success, otherwise false and error message
--
--*******************************************************************************
local function getCosPortAttr(params)
    local devNum = params["device"]
    local portNum, maxPortNum
    local ret, val
    local errors = ""
    local isTableHeaderPrinted = false


    -- getting max port number for device
    ret, maxPortNum = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if (ret ~= LGT_OK) then
        print("\nCould not get number of ports for device " .. devNum .. ":" ..
              "\n   " .. returnCodes[ret] .. "\n")

        return false, "Could not get number of ports: " .. returnCodes[ret]
    end

    -- go over all available ports
    for portNum = 0, maxPortNum do
        if (does_port_exist(devNum, portNum)) then
            ret, val = myGenWrapper("cpssPxCosPortAttributesGet",
                {
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr" }
                })

            if (ret ~= LGT_OK) then
                errors = errors ..
                         "   port " .. portNum .. ":  " ..
                         returnCodes[ret] .. "\n"
            else
                -- print table header if not printed yet
                if (not isTableHeaderPrinted) then
                    print("\n+------+-----+----+--------+----+" ..
                          "\n| port | DEI | UP |   DP   | TC |" ..
                          "\n+------+-----+----+--------+----+")

                    isTableHeaderPrinted = true
                end

                -- print table row
                print(string.format("|  %2s  |", portNum) ..
                      cosAttrToTableRow(val["cosAttributesPtr"]))
            end
        end
    end

    -- print table footer if table is not empty
    if (isTableHeaderPrinted) then
        print("+------+-----+----+--------+----+\n")
    else
        print("")
    end

    -- print errors
    if (errors ~= "") then
        print("cpssPxCosPortAttributesGet return for:\n" .. errors)
    end

    return true
end


--******************************************************************************
-- add help: show cos
--******************************************************************************
CLI_addHelp("exec", "show cos", "Display CoS attributes")

--******************************************************************************
-- command registration: show cos port-attributes
--******************************************************************************
CLI_addCommand("exec", "show cos port-attributes", {
    func = getCosPortAttr,
    help = "Display CoS attributes per Port",
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


--******************************************************************************
--  getCosFormatEntry
--
--  @description Display CoS Attributes per Packet Type
--
--  @param params["device"] - device number
--
--  @return true on success
--
--******************************************************************************
local function getCosFormatEntry(params)
    local devNum = params["device"]
    local packetType
    local cosMode
    local ret, val
    local errors = ""
    local isTableHeaderPrinted = false


    -- go over all available ports
    for packetType = 0, 31 do
        ret, val = myGenWrapper("cpssPxCosFormatEntryGet",
            {
                { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN",  "CPSS_PX_PACKET_TYPE", "packetType", packetType },
                { "OUT", "CPSS_PX_COS_FORMAT_ENTRY_STC", "cosFormatEntryPtr" }
            })

        if (ret ~= LGT_OK) then
            errors = errors ..
                     "   packetType " .. packetType .. ":  " ..
                     returnCodes[ret] .. "\n"
        else
            -- print table header if not printed yet
            if (not isTableHeaderPrinted) then
                print("\n+-------+-------------+-------------------------------------------+------------------------+" ..
                      "\n|       |             |             packet CoS attrs              |    entry CoS attrs     |" ..
                      "\n| entry |    mode     +-------------+------------+----------------+-----+----+--------+----+" ..
                      "\n|       |             | byte offset | bit offset | number of bits | DEI | UP |   DP   | TC |" ..
                      "\n+-------+-------------+-------------+------------+----------------+-----+----+--------+----+")

                isTableHeaderPrinted = true
            end

            -- convert CoS mode to human-oriented string
            cosMode = val["cosFormatEntryPtr"]["cosMode"]

            if (cosMode == "CPSS_PX_COS_MODE_PORT_E") then
                cosMode = "port       "
            elseif (cosMode == "CPSS_PX_COS_MODE_PACKET_DSA_E") then
                cosMode = "packet DSA "
            elseif (cosMode == "CPSS_PX_COS_MODE_PACKET_L2_E") then
                cosMode = "packet L2  "
            elseif (cosMode == "CPSS_PX_COS_MODE_PACKET_L3_E") then
                cosMode = "packet L3  "
            elseif (cosMode == "CPSS_PX_COS_MODE_PACKET_MPLS_E") then
                cosMode = "packet MPLS"
            elseif (cosMode == "CPSS_PX_COS_MODE_FORMAT_ENTRY_E") then
                cosMode = "entry      "
            else
                cosMode = "   ERROR   "
            end

            -- print table row
            print(string.format("|  %2s   | %s |     %2s      |      %s     |        %s       |",
                                packetType,
                                cosMode,
                                val["cosFormatEntryPtr"]["cosByteOffset"],
                                val["cosFormatEntryPtr"]["cosBitOffset"],
                                val["cosFormatEntryPtr"]["cosNumOfBits"]) ..
                  cosAttrToTableRow(val["cosFormatEntryPtr"]["cosAttributes"]))
        end
    end

    -- print table footer if table is not empty
    if (isTableHeaderPrinted) then
        print("+-------+-------------+-------------+------------+----------------+-----+----+--------+----+\n")
    else
        print("")
    end

    -- print errors
    if (errors ~= "") then
        print("cpssPxCosFormatEntryGet return for:\n" .. errors)
    end

    return true
end


--******************************************************************************
-- command registration: show cos format-entry
--******************************************************************************
CLI_addCommand("exec", "show cos format-entry", {
    func = getCosFormatEntry,
    help = "Display CoS attributes per Packet Type",
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


--******************************************************************************
--  getCosMapTypeL2AllPorts
--
--  @description Display CoS Attributes of Port L2 Mapping entry (all ports)
--
--  @params maxEntryIndex - index of last entry
--  @params devNum        - device number
--  @params portNum       - number of first port
--  @params maxPortNum    - number of last port
--
--  @return true on success
--
--******************************************************************************
local function getCosMapTypeL2AllPorts(maxEntryIndex, devNum, portNum, maxPortNum)
    local entry
    local portNumIndex, portCount
    local tableHeader, tableRow, tableFooter
    local ret, val
    local errors = ""
    local isTableHeaderPrinted

    -- go over port quartets
    while (portNum <= maxPortNum) do
        isTableHeaderPrinted = false

        tableHeader = {
            [0] = "\n+-------+",
            [1] = "\n|       |",
            [2] = "\n| entry +",
            [3] = "\n|       |",
            [4] = "\n+-------+"
        }

        tableFooter = "+-------+"

        -- go over all entries
        for entry = 0, maxEntryIndex do
            portCount = 0
            portNumIndex = portNum
            tableRow = string.format("|  %2s   |", entry)

            -- go over 3 or less ports
            while ((portNumIndex <= maxPortNum) and (portCount < 3)) do
                if (does_port_exist(devNum, portNumIndex)) then
                    ret, val = myGenWrapper("cpssPxCosPortL2MappingGet",
                        {
                            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                            { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNumIndex },
                            { "IN",  "GT_U32", "l2Index", entry },
                            { "OUT", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr" }
                        })

                    if (ret ~= LGT_OK) then
                        errors = errors ..
                                 "   port " .. portNumIndex .. ", entry " .. entry ..
                                 ":  " .. returnCodes[ret] .. "\n"
                        tableRow = tableRow .. "   - |  - |  - |  - |"
                    else
                        tableRow = tableRow .. cosAttrToTableRow(val["cosAttributesPtr"])
                    end

                    -- concat port's part of table header and footer
                    if (not isTableHeaderPrinted) then
                        tableHeader[0] = tableHeader[0] .. "------------------------+"
                        tableHeader[1] = tableHeader[1] ..
                                         string.format("         port %-2s        |", portNumIndex)
                        tableHeader[2] = tableHeader[2] .. "-----+----+--------+----+"
                        tableHeader[3] = tableHeader[3] .. " DEI | UP |   DP   | TC |"
                        tableHeader[4] = tableHeader[4] .. "-----+----+--------+----+"

                        tableFooter = tableFooter .. "-----+----+--------+----+"
                    end

                    portCount = portCount + 1
                end

                portNumIndex = portNumIndex + 1
            end

            -- print table header if needed
            if (not isTableHeaderPrinted) then
                print(tableHeader[0] ..
                      tableHeader[1] ..
                      tableHeader[2] ..
                      tableHeader[3] ..
                      tableHeader[4])

                isTableHeaderPrinted = true
            end

            -- print table row
            print(tableRow)
        end

        -- print table footer if table is not empty
        if (isTableHeaderPrinted) then
            print(tableFooter .. "\n")

            -- exit if needed
            if ((portNumIndex <= maxPortNum) or (errors ~= "")) then
                if (printRequestToExit() == nil) then
                    return true
                end
            end
        end

        portNum = portNumIndex
    end

    -- print errors
    if (errors ~= "") then
        print("\ncpssPxCosPortL2MappingGet return for:\n" .. errors)
    end

    return true
end


--******************************************************************************
--  getCosMapSimple
--
--  @description Display CoS Attributes of DSA, MPLS, Port L2 (for one port) or
--               L3 Mapping entry
--
--  @params cosType       - CoS Mapping type
--  @params maxEntryIndex - index of last entry
--  @params devNum        - device number
--  @params portNum       - number of first port
--
--  return true on success
--
--******************************************************************************
local function getCosMapSimple(cosType, maxEntryIndex, devNum, portNum)
    local entry, countOfPrintedEntries
    local api, apiIndexName
    local ret, val
    local errors = ""
    local isTableHeaderPrinted = false


    if (cosType == "l2") then
        api = "cpssPxCosPortL2MappingGet"
        apiIndexName = "l2Index"
    elseif (cosType == "l3") then
        api = "cpssPxCosL3MappingGet"
        apiIndexName = "l3Index"
    elseif (cosType == "mpls") then
        api = "cpssPxCosMplsMappingGet"
        apiIndexName = "mplsIndex"
    elseif (cosType == "dsa") then
        api = "cpssPxCosDsaMappingGet"
        apiIndexName = "dsaIndex"
    end


    countOfPrintedEntries = 0

    -- go over all available entries
    for entry = 0, maxEntryIndex do
        if (api == "cpssPxCosPortL2MappingGet") then
            ret, val = myGenWrapper(api,
                {
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "IN",  "GT_U32", "l2Index", entry },
                    { "OUT", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr" }
                })

            if (ret ~= LGT_OK) then
                errors = errors ..
                         "   port " .. portNum .. ", entry " .. entry .. ":  " ..
                         returnCodes[ret] .. "\n"
            end
        else
            ret, val = myGenWrapper(api,
                {
                    { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN",  "GT_U32", apiIndexName, entry },
                    { "OUT", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr" }
                })

            if (ret ~= LGT_OK) then
                errors = errors ..
                         "   entry " .. entry .. ":  " ..
                         returnCodes[ret] .. "\n"
            end
        end

        if (ret == LGT_OK) then
            if ((not isTableHeaderPrinted) or
                ((isTableHeaderPrinted) and (countOfPrintedEntries % 32 == 0) and (entry ~= maxEntryIndex))) then
                -- print table footer and propose to exit
                if (isTableHeaderPrinted) then
                    print("+-------+-----+----+--------+----+")

                    if (printRequestToExit() == nil) then
                        return true
                    end
                else
                    print("")
                end

                -- print table header
                print("+-------+-----+----+--------+----+" ..
                    "\n| entry | DEI | UP |   DP   | TC |" ..
                    "\n+-------+-----+----+--------+----+")

                isTableHeaderPrinted = true
            end

            -- print table row
            print(string.format("|  %3s  |", entry) ..
                  cosAttrToTableRow(val["cosAttributesPtr"]))

            countOfPrintedEntries = countOfPrintedEntries + 1
        end
    end

    -- print table footer if table is not empty
    if (isTableHeaderPrinted) then
        print("+-------+-----+----+--------+----+\n")
    else
        print("")
    end

    -- print errors
    if (errors ~= "") then
        print(api .. " return for:\n" .. errors)
    end

    return true
end


--*******************************************************************************
--  getCosMap
--
--  @description Display CoS Attributes of DSA, MPLS, Port L2 or L3 Mapping entry
--
--  @param cosType          - CoS Mapping type
--  @param params["device"] - device number
--
--  @return true on success, otherwise false and error message
--
--*******************************************************************************
local function getCosMap(cosType, params)
    local devNum = params["device"]
    local entry, maxEntryIndex
    local portNum, portNumIndex, maxPortNum, portCount
    local tableHeader, tableRow, tableFooter
    local ret, val
    local errors = ""
    local isTableHeaderPrinted

    -- get max number of entry
    if (cosType == "l2") then
        maxEntryIndex = 15
    elseif (cosType == "l3") then
        maxEntryIndex = 63
    elseif (cosType == "mpls") then
        maxEntryIndex = 7
    elseif (cosType == "dsa") then
        maxEntryIndex = 127
    else
        maxEntryIndex = 0
    end

    -- get number of ports for cosType == "l2"
    if (cosType == "l2") then
        portNum = params["portNum"]

        -- getting max port number for device
        ret, maxPortNum = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
        if (ret ~= LGT_OK) then
            print("\nCould not get number of ports for device " .. devNum .. ":" ..
                  "\n   " .. returnCodes[ret] .. "\n")

            return false, "Could not get number of ports: " .. returnCodes[ret]
        end

        -- check input port number
        if ((portNum ~= nil) and (portNum > maxPortNum)) then
            print("\nWrong port number for device " .. devNum .. ": " ..
                  portNum .. "\n")

            return false, "Wrong port number: " .. portNum
        end

        if (portNum == nil) then
            portNum = 0
        else
            maxPortNum = portNum
        end
    end


    -- for cosType "l2" without portNum
    if ((cosType == "l2") and (portNum ~= maxPortNum)) then
        return getCosMapTypeL2AllPorts(maxEntryIndex, devNum, portNum, maxPortNum)
    else
        return getCosMapSimple(cosType, maxEntryIndex, devNum, portNum)
    end
end


--******************************************************************************
-- add help: show cos map
--******************************************************************************
CLI_addHelp("exec", "show cos map", "Display CoS attributes per Packet")

--******************************************************************************
-- command registration: show cos map
--******************************************************************************
local showCosMapParams = {
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

-- show cos map type mpls
CLI_addCommand("exec", "show cos map type mpls", {
    func = function(params)
               return getCosMap("mpls", params)
           end,
    help = "MPLS CoS Mapping entries",
    params = showCosMapParams
})

-- show cos map type dsa
CLI_addCommand("exec", "show cos map type dsa", {
    func = function(params)
               return getCosMap("dsa", params)
           end,
    help = "DSA CoS Mapping entries",
    params = showCosMapParams
})

-- show cos map type l3
CLI_addCommand("exec", "show cos map type l3", {
    func = function(params)
               return getCosMap("l3", params)
           end,
    help = "L3 CoS Mapping entries",
    params = showCosMapParams
})

-- show cos map type l2
CLI_addCommand("exec", "show cos map type l2", {
    func = function(params)
               return getCosMap("l2", params)
           end,
    help = "Port L2 CoS Mapping entries",
    params = {
        {
            type= "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "port %portNum",
                name = "portNum",
                help = "Port number"
            },
            mandatory = {
                "device"
            }
        }
    }
})
