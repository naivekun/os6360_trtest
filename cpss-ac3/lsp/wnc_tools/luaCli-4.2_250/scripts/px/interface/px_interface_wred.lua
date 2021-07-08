--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_interface_wred.lua
--*
--* DESCRIPTION:
--*       Enable/disable WRTD for {Queue,DP} descriptor/buffer limits.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--*******************************************************************************
--  printWrtdError
--
--* @description Print error message for setWrtd
--
--*******************************************************************************
local function printWrtdError(portNum)
    print("Could not set WRED for port " .. portNum)
end


--*******************************************************************************
--  setWrtd
--
--* @description Enable/disable WRTD for {Queue,DP} descriptor/buffer limits.
--
--  @param params -
--          params["queue"]  - Traffic Class associated with this set of Drop
--                             Parameters.
--          params["dp"]     - Drop Profile PArameters to associate with the
--                             Traffic Class.
--          params["flagNo"] - true if no flag set. Otherwise false
--
--  @return true on success, otherwise false
--
--*******************************************************************************
local function setWrtd(params)
    local iterator, devNum, portNum, ret, val
    local enablers, queueId, dpId, startTc, endTc, startDp, endDp
    local tc, dp, profile, tcDpLimit

    -- Common variables initialization.
    local command_data = Command_Data()
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()


    -- set the loop range for the queue ID
    queueId = params["queue"]
    if (queueId == "all") then
        startTc = 0
        endTc = 7
    else
        startTc = queueId
        endTc = queueId
    end

    -- set the loop range for the drop precedence
    local dpId = params["dp"]
    if (dpId == "all") then
        startDp = 0
        endDp = 2
    else
        startDp = dpId
        endDp = dpId
    end

    tcDpLimit = not params["flagNo"]

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
                printWrtdError(portNum)
                return false
            end

            profile = val["profileSetPtr"]

            for tc = startTc, endTc do
                for dp = startDp, endDp do
                    -- get Random Tail drop Threshold status
                    ret, val = myGenWrapper("cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet",
                        {
                            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                            { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                              "profile", profile },
                            { "IN", "GT_U32", "dp", dp },
                            { "IN", "GT_U32", "tc", tc },
                            { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC",
                              "enablersPtr" }
                        })

                    if (ret ~= LGT_OK) then
                        printWrtdError(portNum)
                        return false
                    end

                    enablers = val["enablersPtr"]
                    enablers["tcDpLimit"] = tcDpLimit

                    -- enable/disable WRTD for {Queue,DP} descriptor/buffer limits
                    ret, val = myGenWrapper("cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet",
                        {
                            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                            { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                              "profile", profile },
                            { "IN", "GT_U32", "dp", dp },
                            { "IN", "GT_U32", "tc", tc },
                            { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC",
                              "enablersPtr", enablers }
                        })

                    if (ret ~= LGT_OK) then
                        printWrtdError(portNum)
                        return false
                    end
                end
            end
        end
    end

    return true
end


--*******************************************************************************
-- command registration: wred
--*******************************************************************************
CLI_addCommand("interface", "wred", {
    func =  function(params)
                params["flagNo"] = false
                return setWrtd(params)
            end,
    help = "Enable WRED for a specific TC/DP",
    params = {
        {
            type = "named",
            {
                format = "queue %tail_drop_traffic_class",
                name = "queue",
                help = "Traffic class associated with the set of drop parameters"
            },
            {
                format = "dp %tail_drop_drop_precedence",
                name = "dp",
                help = "Drop precedence"
            },
            requirements = {
                ["dp"] = { "queue" }
            },
            mandatory = {"dp"}
        }
    }
})


--*******************************************************************************
-- command registration: no wred
--*******************************************************************************
CLI_addCommand("interface", "no wred", {
    func =  function(params)
                params["flagNo"] = true
                return setWrtd(params)
            end,
    help = "Disable WRED for a specific TC/DP",
    params = {
        {
            type = "named",
            {
                format = "queue %tail_drop_traffic_class",
                name = "queue",
                help = "Traffic class associated with the set of drop parameters"
            },
            {
                format = "dp %tail_drop_drop_precedence",
                name = "dp",
                help = "Drop precedence"
            },
            requirements = {
                ["dp"] = { "queue" }
            },
            mandatory = {"dp"}
        }
    }
})
