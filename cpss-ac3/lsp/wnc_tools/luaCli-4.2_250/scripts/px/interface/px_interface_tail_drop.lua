--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_interface_tail_drop.lua
--*
--* DESCRIPTION:
--*       Tail Drop of the Physical Port Tx Traffic Class Queues related
--*       commands.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--*******************************************************************************
--  setTailDrop
--
--  @description Configures Tail Drop limits on Port level
--
--  @param params -
--          params["packet-limit"] - maximal number of descriptions for a port.
--          params["buffer-limit"] - maximal number of buffers for a port.
--
--  @return true on success, otherwise false
--
--*******************************************************************************
local function setTailDrop(params)
    local iterator, devNum, portNum, ret, val
    local devices, i
    local profileIndex
    local maxBuff, maxDesc

    -- Common variables initialization.
    local command_data = Command_Data()
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()


    -- Main interface handling cycle
    if (command_data["status"] == true) then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            -- get tail drop profile index for the port
            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileIdGet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profileSetPtr" }
                })

            if ((ret ~= LGT_OK) or (val == nil)) then
                print("Could not set tail-drop for port " .. portNum)
                return false
            end

            profileIndex = val["profileSetPtr"]

            -- read profile
            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileGet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profileSet", profileIndex },
                    { "OUT", "GT_U32", "portMaxBuffLimitPtr" },
                    { "OUT", "GT_U32", "portMaxDescLimitPtr" },
                    { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT",
                      "portAlphaPtr" }
                })

            if ((ret ~= LGT_OK) or (val == nil)) then
                print("Could not set tail-drop for port " .. portNum)
                return false
            end

            -- get packet-limit if entered, else, not changing.
            if (params["packet-limit"] ~= nil) then
                maxDesc = tonumber(params["packet-limit"])
            else
                maxDesc = val["portMaxDescLimitPtr"]
            end

            -- get buffer-limit if entered, else, not changing.
            if (params["buffer-limit"] ~= nil) then
                maxBuff = tonumber(params["buffer-limit"])
            else
                maxBuff = val["portMaxBuffLimitPtr"]
            end

            -- get buffer-limit if entered, else, not changing.
            if (params["alpha"] ~= nil) then
                alpha = params["alpha"]
            else
                alpha = val["portAlphaPtr"]
            end

            -- write profile
            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileSet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profileSet", profileIndex },
                    { "IN", "GT_U32", "portMaxBuffLimit", maxBuff },
                    { "IN", "GT_U32", "portMaxDescLimit", maxDesc },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT", "portAlpha", alpha }
                })

            if (ret ~= LGT_OK) then
                print("Could not set tail-drop for port " .. portNum)
                return false
            end
        end
    end

    if (params["dba-buffer-limit"] ~= nil) then
        devices = wrLogWrapper("wrlDevList")

        for i = 1, #devices do
            ret, val = myGenWrapper("cpssPxPortTxTailDropDbaAvailableBuffSet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devices[i] },
                    { "IN", "GT_U32", "availableBuff",
                      tonumber(params["dba-buffer-limit"]) }
                })

            if (ret ~= LGT_OK) then
                print("Could not set tail drop DBA buffer limit")
                return false
            end
        end
    end

    return true
end



--*******************************************************************************
--  command registration: tail-drop
--*******************************************************************************
CLI_addCommand("interface", "tail-drop", {
    func = setTailDrop,
    help = "Configure tail drop limits at the port level",
    params = {
        {
            type = "named",
            {
                format = "dba-buffer-limit %tail_drop_dba_buffer_limit",
                name = "dba-buffer-limit",
                help = "Amount of buffers available for dynamic allocation (DBA mode only)"
            },
            {
                format = "packet-limit %tail_drop_packet_limit",
                name = "packet-limit",
                help = "Maximal number of descriptors for the port"
            },
            {
                format = "buffer-limit %tail_drop_buffer_limit",
                name = "buffer-limit",
                help = "Maximal number of buffers for the port"
            },
            {
                format = "alpha %tail_drop_alpha",
                name = "alpha",
                help = "Ratio of the free buffers used for the port thresholds"
            }
        }
    }
})



--*******************************************************************************
--  setTailDropQueue
--
--  @description Set tail drop profiles limits for particular TC.
--
--  @param params -
--          params["queue"]        - Traffic Class associated with this set of
--                                   Drop Parameters.
--          params["dp"]           - Drop Profile Parameters to associate with
--                                   the Traffic Class.
--          params["packet-limit"] - maximal number of descriptions for a port.
--          params["buffer-limit"] - maximal number of buffers for a port.
--
--
--  @return true on success, otherwise false
--
--*******************************************************************************
local function setTailDropQueue(params)
    local iterator, devNum, portNum, ret, val, tc
    local profileParams, queueID, startID, endID, dp, startDP, endDP
    local currDesc, currBuff, profileIndex
    local enableSharing, maxBuff, maxDesc

    -- Common variables initialization
    local command_data = Command_Data()
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    -- set the loop range for the queue ID
    queueID = params["queue"]
    if (queueID == "all") then
        startID = 0
        endID = 7
    else
        startID = queueID
        endID = queueID
    end

    -- set the loop range for the drop precedence
    dp = params["dp"]
    if (dp == "all") then
        startDP = 0
        endDP = 2
    else
        startDP = dp
        endDP = dp
    end


    -- Main interface handling cycle
    if (command_data["status"] == true) then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            -- get tail drop profile index for the port
            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileIdGet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profileSetPtr" }
                })

            if ((ret ~= LGT_OK) or (val == nil)) then
                print("Could not set tail-drop-queue for port " .. portNum)
                return false
            end

            profileIndex = val["profileSetPtr"]

            -- loop over the queue ID's
            for tc = startID, endID do
                -- read profile
                ret, val = myGenWrapper("cpssPxPortTxTailDropProfileTcGet",
                    {
                        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                          "profileSet", profileIndex },
                        { "IN", "GT_U32", "tc", tc },
                        { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC",
                          "tailDropProfileParamsPtr" }
                    })

                if ((ret ~= LGT_OK) or (val == nil)) then
                    print("Could not set tail-drop-queue for port " .. portNum)
                    return false
                end

                profileParams = val["tailDropProfileParamsPtr"]

                -- loop over the packet's color
                for index = startDP, endDP do
                    if (params["packet-limit"] ~= nil) then
                        -- get the name of the descriptor parameter
                        currDesc = "dp" .. index .. "MaxDescNum"

                        profileParams[currDesc] = tonumber(params["packet-limit"])
                    end

                    if (params["buffer-limit"] ~= nil) then
                        -- get the name of the buffer parameter
                        currBuff = "dp" .. index .. "MaxBuffNum"

                        profileParams[currBuff] = tonumber(params["buffer-limit"])
                    end

                    if (params["alpha"] ~= nil) then
                        -- get the name of the buffer parameter
                        currBuff = "dp" .. index .. "QueueAlpha"

                        profileParams[currBuff] = params["alpha"]
                    end
                end

                -- write profile
                ret, val = myGenWrapper("cpssPxPortTxTailDropProfileTcSet",
                    {
                        { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                        { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                          "profileSet", profileIndex },
                        { "IN", "GT_U32", "tc", tc },
                        { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC",
                          "tailDropProfileParamsPtr", profileParams }
                    })

                if (ret ~= LGT_OK) then
                    print("Could not set tail-drop-queue for port " .. portNum)
                    return false
                end
            end
        end
    end

    return true
end



--*******************************************************************************
--  command registration: tail-drop-queue
--*******************************************************************************
CLI_addCommand("interface", "tail-drop-queue", {
    func = setTailDropQueue,
    help = "Configure tail drop profile limits for particular Traffic Class",
    params = {
        {
            type = "named",
            {
                format = "queue %tail_drop_traffic_class",
                name = "queue",
                help = "Traffic class associated with this set of drop parameters"
            },
            {
                format = "dp %tail_drop_drop_precedence",
                name = "dp",
                help = "Drop precedence"
            },
            {
                format = "packet-limit %tail_drop_packet_limit",
                name = "packet-limit",
                help = "Maximal number of descriptors for the port"
            },
            {
                format = "buffer-limit %tail_drop_buffer_limit",
                name = "buffer-limit",
                help = "Maximal number of buffers for the port"
            },
            {
                format = "alpha %tail_drop_alpha",
                name = "alpha",
                help = "Ratio of the free buffers used for the port thresholds"
            },
            requirements = {
                ["dp"]           = { "queue" },
                ["packet-limit"] = { "dp" },
                ["buffer-limit"] = { "dp" },
                ["alpha"]        = { "dp" }
            },
            mandatory = { "dp" }
        }
    }
})



--*******************************************************************************
--  setTailDropPolicy
--
--  @description Set tail drop policy.
--
--  @param params -
--          params["tail-drop-policy"] - Tail Drop policy:
--                                          0 - static (DBA disabled),
--                                          1 - dynamic (DBA enabled)
--
--  @return true on success, otherwise false
--
--*******************************************************************************
local function setTailDropPolicy(params)
    local ret, val, devices, i, enable

    if (params["tail-drop-policy"] == 0) then
        enable = false
    else
        enable = true
    end


    devices = wrLogWrapper("wrlDevList")

    for i = 1, #devices do
        ret, val = myGenWrapper("cpssPxPortTxTailDropDbaModeEnableSet",
            {
                { "IN", "GT_SW_DEV_NUM", "devNum", devices[i] },
                { "IN", "GT_BOOL", "enable", enable }
            })

        if (ret ~= LGT_OK) then
            print("Could not set tail drop policy")
            return false
        end
    end

    return true
end



--*******************************************************************************
--  command registration: tail-drop-policy
--*******************************************************************************
CLI_addCommand("interface", "tail-drop-policy", {
    func = setTailDropPolicy,
    help = "Configure tail drop queueing resources allocation policy",
    params = {
        {
            type = "named",
            {
                format = "%tail_drop_policy",
                name = "tail-drop-policy",
                help = "Configure tail drop queueing resources allocation policy"
            },
            mandatory = { "tail-drop-policy" }
        }
    }
})
