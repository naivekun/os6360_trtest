--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_config_dce_qcn.lua
--*
--* DESCRIPTION:
--*       DCE Quantized Congestion Notification configuration on PIPE
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--******************************************************************************
--  setDceCnm
--
--  @description Set Congestion Notification Messages configuration
--
--  @params params["device"]     - device number
--          params["flagNo"]     - if true configuration (dp, tc, up) will set
--                                 to default values.
--          params["dp"]         - Drop Precedence
--          params["up"]         - User Priority
--          params["tc"]         - Traffic Class
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function setDceCnm(params)
    local ret, val
    local cnmConfig

    -- get currect configuration
    ret, val = myGenWrapper("cpssPxPortCnMessageGenerationConfigGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "OUT", "CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC",
                     "cnmGenerationCfgPtr" }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortCnMessageGenerationConfigGet" ..
              " devNum=" .. params["device"] ..
              " failed: " .. returnCodes[ret])

        return false, "cpssPxPortCnMessageGenerationConfigGet" ..
                      " devNum=" .. params["device"] ..
                      " failed: rc=" .. ret
    end


    cnmConfig = val["cnmGenerationCfgPtr"]

    -- set configuration to default
    if (params["flagNo"]) then
        cnmConfig["priority"] = 0
        cnmConfig["dropPrecedence"] = 0
        cnmConfig["tc4pfc"] = 0

    -- set specified parameters
    else
        if (params["tc"] ~= nil) then
            cnmConfig["tc4pfc"] = params["tc"]
        end

        if (params["up"] ~= nil) then
            cnmConfig["priority"] = params["up"]
        end

        if (params["dp"] ~= nil) then
            cnmConfig["dropPrecedence"] = params["dp"]
        end
    end


    -- set new configuration
    ret, val = myGenWrapper("cpssPxPortCnMessageGenerationConfigSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "IN", "CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC",
                    "cnmGenerationCfgPtr", cnmConfig }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortCnMessageGenerationConfigSet" ..
              " devNum=" .. params["device"] ..
              " cnmGenerationCfgPtr->packetType=" .. cnmConfig["packetType"] ..
              " cnmGenerationCfgPtr->priority=" .. cnmConfig["priority"] ..
              " cnmGenerationCfgPtr->dropPrecedence=" .. cnmConfig["dropPrecedence"] ..
              " cnmGenerationCfgPtr->tc4pfc=" .. cnmConfig["tc4pfc"] ..
              " cnmGenerationCfgPtr->localGeneratedPacketTypeAssignmentEnable=" ..
                      cnmConfig["localGeneratedPacketTypeAssignmentEnable"] ..
              " cnmGenerationCfgPtr->localGeneratedPacketType=" ..
                      cnmConfig["localGeneratedPacketType"] ..
              " failed: " .. returnCodes[ret])

        return false, "cpssPxPortCnMessageGenerationConfigSet" ..
                      " devNum=" .. params["device"] ..
                      " cnmGenerationCfgPtr->packetType=" .. cnmConfig["packetType"] ..
                      " cnmGenerationCfgPtr->priority=" .. cnmConfig["priority"] ..
                      " cnmGenerationCfgPtr->dropPrecedence=" ..
                              cnmConfig["dropPrecedence"] ..
                      " cnmGenerationCfgPtr->tc4pfc=" ..
                              cnmConfig["tc4pfc"] ..
                      " cnmGenerationCfgPtr->localGeneratedPacketTypeAssignmentEnable=" ..
                              cnmConfig["localGeneratedPacketTypeAssignmentEnable"] ..
                      " cnmGenerationCfgPtr->localGeneratedPacketType=" ..
                              cnmConfig["localGeneratedPacketType"] ..
                      " failed: rc=" .. ret
    end

    return true
end


--******************************************************************************
--  setDceCnFeedback
--
--  @description Set QCN Feedback configuration
--
--  @params params["device"]     - device number
--          params["flagNo"]     - if true configuration (dp, tc, up) will set
--                                 to default values.
--          params["exp-weight"] - Delta inclusion on Feedback calculation
--          params["delta"]      - Exponent weight for CN Feedback calculation
--          params["lsb"]        - Bits that are used for qFb calculation
--          params["min"]        - Minimal value of CN Feedback
--          params["max"]        - Maximal value of CN Feedback
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function setDceCnFeedback(params)
    local ret, val
    local fbConfig

    -- get currect configuration
    ret, val = myGenWrapper("cpssPxPortCnFbCalcConfigGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "OUT", "CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC",
                     "fbCalcCfgPtr" }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortCnFbCalcConfigGet" ..
              " devNum=" .. params["device"] ..
              " failed: " .. returnCodes[ret])

        return false, "cpssPxPortCnFbCalcConfigGet" ..
                      " devNum=" .. params["device"] ..
                      " failed: rc=" .. ret
    end


    fbConfig = val["fbCalcCfgPtr"]

    -- set configuration to default
    if (params["flagNo"]) then
        fbConfig["wExp"] = 0
        fbConfig["fbLsb"] = 0
        fbConfig["deltaEnable"] = false
        fbConfig["fbMin"] = 0
        fbConfig["fbMax"] = 0

    -- set specified parameters
    else
        if (params["exp-weight"] ~= nil) then
            fbConfig["wExp"] = params["exp-weight"]
        end

        if (params["lsb"] ~= nil) then
            fbConfig["fbLsb"] = params["lsb"]
        end

        if (params["delta"] ~= nil) then
            fbConfig["deltaEnable"] = params["delta"]
        end

        if (params["min"] ~= nil) then
            fbConfig["fbMin"] = params["min"]
        end

        if (params["max"] ~= nil) then
            fbConfig["fbMax"] = params["max"]
        end

    end


    -- set new configuration
    ret, val = myGenWrapper("cpssPxPortCnFbCalcConfigSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "IN", "CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC",
                    "fbCalcCfgPtr", fbConfig }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortCnFbCalcConfigSet" ..
              " devNum=" .. params["device"] ..
              " fbCalcCfgPtr->wExp=" .. fbConfig["wExp"] ..
              " fbCalcCfgPtr->fbLsb=" .. fbConfig["fbLsb"] ..
              " fbCalcCfgPtr->deltaEnable=" .. tostring(fbConfig["deltaEnable"]) ..
              " fbCalcCfgPtr->fbMin=" .. fbConfig["fbMin"] ..
              " fbCalcCfgPtr->fbMax=" .. fbConfig["fbMax"] ..
              " failed: " .. returnCodes[ret])

        return false, "" ..
                      " devNum=" .. params["device"] ..
                      " fbCalcCfgPtr->wExp=" .. fbConfig["wExp"] ..
                      " fbCalcCfgPtr->fbLsb=" .. fbConfig["fbLsb"] ..
                      " fbCalcCfgPtr->deltaEnable=" .. tostring(fbConfig["deltaEnable"]) ..
                      " fbCalcCfgPtr->fbMin=" .. fbConfig["fbMin"] ..
                      " fbCalcCfgPtr->fbMax=" .. fbConfig["fbMax"] ..
                      " failed: rc=" .. ret
    end

    return true
end


--******************************************************************************
--  setDceCnSampleEntry
--
--  @description Configure CN Sample Interval Table entry
--
--  @params params["device"]     - device number
--          params["flagNo"]     - if true configuration (dp, tc, up) will set
--                                 to default values
--          params["entryIndex"] - index of table entry
--          params["interval"]   - CN sampling interval in resolution of
--                                 16 bytes
--          params["randBitmap"] - Bitmap that indicates which sampling
--                                 interval bits are randomize:
--                                    0 - don't randomize;
--                                    1 - randomize
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function setDceCnSampleEntry(params)
    local ret, val
    local entryConfig

    -- get current configuration
    ret, val = myGenWrapper("cpssPxPortCnSampleEntryGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "IN",  "GT_U8", "entryIndex", params["entryIndex"] },
            { "OUT", "CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC", "entryPtr" }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortCnSampleEntryGet" ..
              " devNum=" .. params["device"] ..
              " entryIndex=" .. params["entryIndex"] ..
              " failed: " .. returnCodes[ret])

        return false, "cpssPxPortCnSampleEntryGet" ..
                      " devNum=" .. params["device"] ..
                      " entryIndex=" .. params["entryIndex"] ..
                      " failed: rc=" .. ret
    end


    entryConfig = val["entryPtr"]

    -- set configuration to default
    if (params["flagNo"]) then
        entryConfig["interval"] = 0
        entryConfig["randBitmap"] = 0

    -- set specified parameters
    else
        if (params["interval"] ~= nil) then
            entryConfig["interval"] = params["interval"]
        end

        if (params["randBitmap"] ~= nil) then
            entryConfig["randBitmap"] = params["randBitmap"]
        end
    end


    -- set new configuration
    ret, val = myGenWrapper("cpssPxPortCnSampleEntrySet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "IN", "GT_U8", "entryIndex", params["entryIndex"] },
            { "IN", "CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC", "entryPtr",
                    entryConfig }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortCnSampleEntrySet" ..
              " devNum=" .. params["device"] ..
              " entryIndex=" .. params["entryIndex"] ..
              " entryPtr->interval=" .. entryConfig["interval"] ..
              " entryPtr->randBitmap=" .. entryConfig["randBitmap"] ..
              " failed: " .. returnCodes[ret])

        return false, "cpssPxPortCnSampleEntrySet" ..
                      " devNum=" .. params["device"] ..
                      " entryIndex=" .. params["entryIndex"] ..
                      " entryPtr->interval=" .. entryConfig["interval"] ..
                      " entryPtr->randBitmap=" .. entryConfig["randBitmap"] ..
                      " failed: rc=" .. ret
    end

    return true
end


--******************************************************************************
-- add help: dce qcn
-- add help: dce qcn cp
-- add help: no dce qcn
-- add help: no dce qcn cp
--******************************************************************************
CLI_addHelp("config", "dce qcn", "Configure QCN")
CLI_addHelp("config", "dce qcn cp", "Configure Congestion Point")
CLI_addHelp("config", "no dce qcn", "Configure QCN")
CLI_addHelp("config", "no dce qcn cp", "Configure Congestion Point")

--******************************************************************************
-- command registration: dce qcn cnm
--******************************************************************************
CLI_addCommand("config", "dce qcn cnm", {
    func = function(params)
               params["flagNo"] = false
               return setDceCnm(params)
           end,
    help = "Configure parameters used for all CN Messages that will be transmitted",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "dp %cos_drop_precedence",
                name = "dp",
                help = "Drop Precedence"
            },
            {
                format = "tc %cos_traffic_class",
                name = "tc",
                help = "Traffic Class"
            },
            {
                format = "up %cos_user_priority",
                name = "up",
                help = "User Priority"
            },
            mandatory = {
                "device"
            }
        }
    }
})

--******************************************************************************
-- command registration: no dce qcn cnm
--******************************************************************************
CLI_addCommand("config", "no dce qcn cnm", {
    func = function(params)
               params["flagNo"] = true
               return setDceCnm(params)
           end,
    help = "Set to default configuration used for all CN Messages that will be transmitted",
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
-- command registration: dce qcn cp feedback
--******************************************************************************
CLI_addCommand("config", "dce qcn cp feedback", {
    func = function(params)
               params["flagNo"] = false
               return setDceCnFeedback(params)
           end,
    help = "Configure QCN Feedback parameters",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "delta %enable_disable",
                name = "delta",
                help = "Delta inclusion on Feedback calculation"
            },
            {
                format = "exp-weight %dce_qcn_feedback_exp_weight",
                name = "exp-weight",
                help = "Exponent weight for CN Feedback calculation"
            },
            {
                format = "lsb %dce_qcn_feedback_lsb",
                name = "lsb",
                help = "Bits that are used for qFb calculation"
            },
            {
                format = "max %dce_qcn_feedback_min_max",
                name = "max",
                help = "Maximal value of CN Feedback"
            },
            {
                format = "min %dce_qcn_feedback_min_max",
                name = "min",
                help = "Minimal value of CN Feedback"
            },
            mandatory = {
                "device"
            }
        }
    }
})

--******************************************************************************
-- command registration: no dce qcn cp feedback
--******************************************************************************
CLI_addCommand("config", "no dce qcn cp feedback", {
    func = function(params)
               params["flagNo"] = true
               return setDceCnFeedback(params)
           end,
    help = "Set to default configuration of QCN Feedback parameters",
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
-- command registration: dce qcn cp min-sample-base
--******************************************************************************
CLI_addCommand("config", "dce qcn cp min-sample-base", {
    func = function(params)
               params["flagNo"] = false
               return setDceCnSampleEntry(params)
           end,
    help = "Configure CN Sample Intervals",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "entry %dce_qcn_cp_sample_int_table_entry_index",
                name = "entryIndex",
                help = "CN Sample Interval Table entry index"
            },
            {
                format = "interval %dce_qcn_cp_sample_int_table_interval",
                name = "interval",
                help = "CN sampling interval in resolution of 16 bytes"
            },
            {
                format = "randomize-bitmap %dce_qcn_cp_sample_int_table_rand_bitmap",
                name = "randBitmap",
                help = "Bitmap that indicates which sampling interval bits are randomize. " ..
                       "0 - don't randomize; 1 - randomize."
            },
            mandatory = {
                "device", "entryIndex"
            }
        }
    }
})

--******************************************************************************
-- command registration: no dce qcn cp min-sample-base
--******************************************************************************
CLI_addCommand("config", "no dce qcn cp min-sample-base", {
    func = function(params)
               params["flagNo"] = true
               return setDceCnSampleEntry(params)
           end,
    help = "Set to default configuration of CN Sample Intervals",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "entry %dce_qcn_cp_sample_int_table_entry_index",
                name = "entryIndex",
                help = "CN Sample Interval Table entry index"
            },
            mandatory = {
                "device", "entryIndex"
            }
        }
    }
})
