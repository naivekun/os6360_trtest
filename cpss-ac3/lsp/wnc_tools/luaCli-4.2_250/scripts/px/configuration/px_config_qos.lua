--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_config_qos.lua
--*
--* DESCRIPTION:
--*       QOS related commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--*******************************************************************************
--  qosWrrQueueThreshold
--
--  @description Sets qos queue threshold
--
--  @param params -
--          params["queueId"]   - Queue number
--          params["threshold"] - Threshold of dropping dp2 packets
--          params["flagNo"]    - Flags the no command
--
--  @return Execution results from command_data
--
--*******************************************************************************
local function qosWrrQueueThreshold(params)
    local ret, val
    local i, devices, profileIndex, thresholdPercent
    local tailDropProfile, profile, tc, portMaxBuffLimit, portMaxDescLimit

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


    if (params["flagNo"] == true) then
        thresholdPercent = 80
    else
        thresholdPercent = params["threshold-percentage"]
    end

    tc = params["queue-id"] - 1


    devices = wrLogWrapper("wrlDevList")
    for i = 1, #devices do
        for profileIndex = 1, 16 do
            profile = profileEnum[profileIndex]

            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileGet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devices[i] },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profileSet", profile },
                    { "OUT", "GT_U32", "portMaxBuffLimitPtr" },
                    { "OUT", "GT_U32", "portMaxDescLimitPtr" },
                    { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT",
                      "portAlphaPtr" }
                })

            portMaxBuffLimit = val["portMaxBuffLimitPtr"]
            portMaxDescLimit = val["portMaxDescLimitPtr"]

            if (ret ~= LGT_OK) then
                print("Could not get maximal port limits of buffers and " ..
                      "descriptors for device " .. devices[i] ..
                      ", profile " .. profile)
                return false
            end

            ret, val = myGenWrapper("cpssPxPortTxTailDropProfileTcGet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devices[i] },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profile", profile },
                    { "IN", "GT_U32", "tc", tc },
                    { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC",
                      "tailDropProfileParamsPtr" }
                })

            if (ret ~= LGT_OK) then
                print("Could not get tail drop profiles limits for particular " ..
                      "Traffic Class for device " .. devices[i] ..
                      ", profile " .. profile)
                return false
            end

            tailDropProfile = val["tailDropProfileParamsPtr"]

            tailDropProfile["dp0MaxBuffNum"] =
                    math.floor(portMaxBuffLimit * thresholdPercent / 100)
            tailDropProfile["dp0MaxDescNum"] =
                    math.floor(portMaxDescLimit * thresholdPercent / 100)
            tailDropProfile["dp1MaxBuffNum"] =
                    math.floor(portMaxBuffLimit * thresholdPercent / 100)
            tailDropProfile["dp1MaxDescNum"] =
                    math.floor(portMaxDescLimit * thresholdPercent / 100)
            tailDropProfile["dp2MaxBuffNum"] =
                    math.floor(portMaxBuffLimit * thresholdPercent / 100)
            tailDropProfile["dp2MaxDescNum"] =
                    math.floor(portMaxDescLimit * thresholdPercent / 100)
            tailDropProfile["tcMaxBuffNum"] =
                    math.floor(portMaxBuffLimit * thresholdPercent / 100)
            tailDropProfile["tcMaxDescNum"] =
                    math.floor(portMaxDescLimit * thresholdPercent / 100)

            ret = myGenWrapper("cpssPxPortTxTailDropProfileTcSet",
                {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devices[i] },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT",
                      "profile", profile },
                    { "IN", "GT_U32", "tc", tc },
                    { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC",
                      "tailDropProfileParamsPtr", tailDropProfile }
                })

            if (ret ~= LGT_OK) then
                print("Could not set tail drop profiles limits for particular " ..
                      "Traffic Class for device " .. devices[i] ..
                      ", profile " .. profile)
                return false
            end
        end
    end
end


--*******************************************************************************
-- command registration: qos wrr-queue threshold
--*******************************************************************************
CLI_addCommand("config", "qos wrr-queue threshold", {
    func =  function(params)
                params["flagNo"] = false
                return qosWrrQueueThreshold(params)
            end,
    help = "Assign queue thresholds globally",
    params = {
        {
            type = "named",
            {
                format = "queue-id %qos_queue_id",
                name = "queue-id",
                help = "Specified queue number to which the tail-drop threshold is assigned"
            },
            {
                format = "threshold-percentage %qos_threshold_percentage",
                name = "threshold-percentage",
                help = "Specified queue threshold percentage value"
            },
            mandatory = {
                "queue-id",
                "threshold-percentage"
            }
        }
    }
})


--*******************************************************************************
-- command registration: no qos wrr-queue threshold
--*******************************************************************************
CLI_addCommand("config", "no qos wrr-queue threshold", {
    func =  function(params)
                params["flagNo"] = true
                return qosWrrQueueThreshold(params)
            end,
    help = "Assign queue thresholds to default value",
    params = {
        {
            type = "named",
            {
                format = "queue-id %qos_queue_id",
                name = "queue-id",
                help = "Specified queue number to which the tail-drop threshold is assigned"
            },
            mandatory = { "queue-id" }
        }
    }
})
