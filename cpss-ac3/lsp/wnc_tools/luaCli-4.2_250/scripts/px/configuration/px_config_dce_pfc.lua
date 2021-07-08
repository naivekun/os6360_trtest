--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_config_dce_pfc.lua
--*
--* DESCRIPTION:
--*       Configure PFC
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--*******************************************************************************
--  dcePfcProfileQueueConfigSet
--
--  @description  Sets PFC profile configurations for given queue.
--
--  @param  params["device"]        - device number
--          params["profileIndex"]  - PFC profile index
--          params["tc"]            - Traffic Class
--          params["xonThreshold"]  - XON Threshold
--          params["xoffThreshold"] - XOFF Threshold
--          params["xonAlpha"]      - XON Alpha ratio
--          params["xoffAlpha"]     - XOFF Alpha ratio
--
--  @return  true on success, otherwise false
--
--*******************************************************************************
local function dcePfcProfileQueueConfigSet(params)
    local ret, val
    local tc, tcMin, tcMax
    local profileConfig
    local rc = true

    if (params["tc"] == "all") then
        tcMin = 0
        tcMax = 7
    else
        tcMin = params["tc"]
        tcMax = params["tc"]
    end

    for tc = tcMin, tcMax do
        ret, val = myGenWrapper("cpssPxPortPfcProfileQueueConfigGet", {
                { "IN",  "GT_SW_DEV_NUM", "devNum", params["device"] },
                { "IN",  "GT_U32", "profileIndex", params["profileIndex"] },
                { "IN",  "GT_U32", "tcQueue", tc },
                { "OUT", "CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC", "pfcProfileCfgPtr" }
            })
        if (ret ~= LGT_OK) then
            if (rc) then
                print("Errors:")
            end

            print("cpssPxPortPfcProfileQueueConfigGet" ..
                  " devNum=" .. params["device"] ..
                  " profileIndex=" .. params["profileIndex"] ..
                  " tcQueue=" .. tc ..
                  " failed: " .. returnCodes[ret])

            rc = false
        else
            profileConfig = val["pfcProfileCfgPtr"]

            if (params["xonThreshold"] ~= nil) then
                profileConfig["xonThreshold"] = params["xonThreshold"]
            end
            if (params["xoffThreshold"] ~= nil) then
                profileConfig["xoffThreshold"] = params["xoffThreshold"]
            end
            if (params["xonAlpha"] ~= nil) then
                profileConfig["xonAlpha"] = params["xonAlpha"]
            end
            if (params["xoffAlpha"] ~= nil) then
                profileConfig["xoffAlpha"] = params["xoffAlpha"]
            end


            ret, val = myGenWrapper("cpssPxPortPfcProfileQueueConfigSet", {
                    { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
                    { "IN", "GT_U32", "profileIndex", params["profileIndex"] },
                    { "IN", "GT_U32", "tcQueue", tc },
                    { "IN", "CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC", "pfcProfileCfgPtr",
                            profileConfig }
                })
            if (ret ~= LGT_OK) then
                if (rc) then
                    print("Errors:")
                end

                print("cpssPxPortPfcProfileQueueConfigSet" ..
                      " devNum=" .. params["device"] ..
                      " profileIndex=" .. params["profileIndex"] ..
                      " tcQueue=" .. tc ..
                      " pfcProfileCfgPtr->xonThreshold=" .. profileConfig["xonThreshold"] ..
                      " pfcProfileCfgPtr->xoffThreshold=" .. profileConfig["xoffThreshold"] ..
                      " pfcProfileCfgPtr->xonAlpha=" .. profileConfig["xonAlpha"] ..
                      " pfcProfileCfgPtr->xoffAlpha=" .. profileConfig["xoffAlpha"] ..
                      " failed: " .. returnCodes[ret])

                rc = false
            end
        end
    end

    return rc
end


--*******************************************************************************
--  dcePfcGlobalDropEnableSet
--
--  @description  Enable/Disable PFC global drop
--
--  @param  params["device"] - device number
--          params["enable"] - if true PFC global drop will be enabled.
--                             if false PFC global drop will be disabled.
--
--  @return  true on success, otherwise false and error message
--
--*******************************************************************************
local function dcePfcGlobalDropEnableSet(params)
    local ret, val

    ret, val = myGenWrapper("cpssPxPortPfcGlobalDropEnableSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "IN", "GT_BOOL", "enable", params["enable"] }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortPfcGlobalDropEnableSet" ..
              " devNum=" .. params["device"] ..
              " portNum=" .. params["enable"] ..
              " failed: " .. returnCodes[ret])
        return false, "cpssPxPortPfcGlobalDropEnableSet" ..
                      " devNum=" .. params["device"] ..
                      " enable=" .. params["enable"] ..
                      " failed: rc[" .. ret .. "]"
    end

    return true
end


--*******************************************************************************
--  dcePfcGlobalQueueConfigSet
--
--  @description  Set PFC configuration for given tc queue
--
--  @param  params["device"] - device number
--          params["tc"]            - Traffic Class
--          params["xonThreshold"]  - XON Threshold
--          params["xoffThreshold"] - XOFF Threshold
--          params["dropThreshold"] - Drop Threshold
--
--  @return  true on success, otherwise false
--
--*******************************************************************************
local function dcePfcGlobalQueueConfigSet(params)
    local ret, val
    local tc, tcMin, tcMax
    local xonThreshold, xoffThreshold, dropThreshold
    local rc = true

    if (params["tc"] == "all") then
        tcMin = 0
        tcMax = 7
    else
        tcMin = params["tc"]
        tcMax = params["tc"]
    end

    for tc = tcMin, tcMax do
        ret, val = myGenWrapper("cpssPxPortPfcGlobalQueueConfigGet", {
                { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
                { "IN", "GT_U32", "tcQueue", tc },
                { "OUT", "GT_U32", "xoffThresholdPtr" },
                { "OUT", "GT_U32", "dropThresholdPtr" },
                { "OUT", "GT_U32", "xonThresholdPtr" }
            })
        if (ret ~= LGT_OK) then
            if (rc) then
                print("Errors:")
            end

            print("cpssPxPortPfcGlobalDropEnableGet" ..
                  " devNum=" .. params["device"] ..
                  " tcQueue=" .. tc ..
                  " failed: " .. returnCodes[ret])

            rc = false
        else
            if (params["xonThreshold"] ~= nil) then
                xonThreshold = params["xonThreshold"]
            else
                xonThreshold = val["xonThresholdPtr"]
            end

            if (params["xoffThreshold"] ~= nil) then
                xoffThreshold = params["xoffThreshold"]
            else
                xoffThreshold = val["xoffThresholdPtr"]
            end

            if (params["dropThreshold"] ~= nil) then
                dropThreshold = params["dropThreshold"]
            else
                dropThreshold = val["dropThresholdPtr"]
            end

            ret, val = myGenWrapper("cpssPxPortPfcGlobalQueueConfigSet", {
                    { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
                    { "IN", "GT_U32", "tcQueue", tc },
                    { "IN", "GT_U32", "xoffThreshold", xoffThreshold },
                    { "IN", "GT_U32", "dropThreshold", dropThreshold },
                    { "IN", "GT_U32", "xonThreshold", xonThreshold }
                })
            if (ret ~= LGT_OK) then
                if (rc) then
                    print("Errors:")
                end

                print("cpssPxPortPfcGlobalDropEnableSet" ..
                      " devNum=" .. params["device"] ..
                      " tcQueue=" .. tc ..
                      " xonThreshold=" .. xonThreshold ..
                      " xoffThreshold=" .. xoffThreshold ..
                      " dropThreshold=" .. dropThreshold ..
                      " failed: " .. returnCodes[ret])

                rc = false
            end
        end
    end

    return rc
end


--******************************************************************************
-- add help: dce
-- add help: dce priority-flow-control
-- add help: dce priority-flow-control global
-- add help: no dce
-- add help: no dce priority-flow-control
-- add help: no dce priority-flow-control global
--******************************************************************************
CLI_addHelp("config", "dce", "Configure DCE")
CLI_addHelp("config", "dce priority-flow-control", "Configure PFC")
CLI_addHelp("config", "dce priority-flow-control global",
            "Performing global PFC configuration")
CLI_addHelp("config", "no dce", "Configure DCE")
CLI_addHelp("config", "no dce priority-flow-control", "Configure PFC")
CLI_addHelp("config", "no dce priority-flow-control global",
            "Performing global PFC configuration")

--******************************************************************************
-- command registration: dce priority-flow-control profile
--******************************************************************************
CLI_addCommand("config", "dce priority-flow-control profile", {
    func = dcePfcProfileQueueConfigSet,
    help ="Configure PFC profile",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "profile-index %dce_pfc_profile",
                name = "profileIndex",
                help = "PFC profile index"
            },
            {
                format = "tc %tail_drop_traffic_class",
                name = "tc",
                help = "Traffic Class"
            },
            {
                format = "xoff-alpha %tail_drop_alpha",
                name = "xoffAlpha",
                help = "XOFF Alpha ratio"
            },
            {
                format = "xoff-threshold %dce_pfc_threshold",
                name = "xoffThreshold",
                help = "XOFF Threshold"
            },
            {
                format = "xon-alpha %tail_drop_alpha",
                name = "xonAlpha",
                help = "XON Alpha ratio"
            },
            {
                format = "xon-threshold %dce_pfc_threshold",
                name = "xonThreshold",
                help = "XON Threshold"
            },
            mandatory = {
                "device", "profileIndex", "tc"
            }
        }
    }
})

--******************************************************************************
-- command registration: dce priority-flow-control global drop
--******************************************************************************
CLI_addCommand("config", "dce priority-flow-control global drop", {
    func = function(params)
               params["enable"] = true
               return dcePfcGlobalDropEnableSet(params)
           end,
    help = "Enable PFC global drop",
    params = {
        {
            type = "named",
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
-- command registration: no dce priority-flow-control global drop
--******************************************************************************
CLI_addCommand("config", "no dce priority-flow-control global drop", {
    func = function(params)
               params["enable"] = false
               return dcePfcGlobalDropEnableSet(params)
           end,
    help = "Disable PFC global drop",
    params = {
        {
            type = "named",
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
-- command registration: dce priority-flow-control global queue
--******************************************************************************
CLI_addCommand("config", "dce priority-flow-control global queue", {
    func = dcePfcGlobalQueueConfigSet,
    help = "Performing global PFC configuration for given queue",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "tc %tail_drop_traffic_class",
                name = "tc",
                help = "Traffic Class"
            },
            {
                format = "drop-threshold %dce_pfc_threshold",
                name = "dropThreshold",
                help = "Drop Threshold"
            },
            {
                format = "xoff-threshold %dce_pfc_threshold",
                name = "xoffThreshold",
                help = "XOFF Threshold"
            },
            {
                format = "xon-threshold %dce_pfc_threshold",
                name = "xonThreshold",
                help = "XON Threshold"
            },
            mandatory = {
                "device", "tc"
            }
        }
    }
})
