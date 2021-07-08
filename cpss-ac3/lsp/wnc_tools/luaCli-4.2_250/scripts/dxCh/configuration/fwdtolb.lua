--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fwdtolb.lua
--*
--* DESCRIPTION:
--*     'Forwarding to loopback/service port' related commands.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- ************************************************************************
---
--  fwdToLbEnablePerDev
--        @description  enable forwarding to Loopback Port generally
--                      for the specifified device. The function is indended to
--                      be called by generic_all_device_func.
--
--        @param command_data CLI command data object
--
--        @param devNum       a device number
--
--        @param params       CLI command parameters
--
--        @return       true on success, otherwise false and error message
--
local function fwdToLbEnablePerDev(command_data, devNum, params)
    if not is_supported_feature(devNum, "FWD_TO_LB") then
        command_data:addErrorAndPrint(
            "device %d doesn't support 'Forwarding to Loopback Port'",
            devNum)
        return
    end

    local enable = not params.flagNo
    local rxProf = params.rxProfile
    local txProf = params.txProfile

    -- fill range of traffic classses
    local tcRange = params.tcRange
    if not is_sip_5_10(devNum) then
        -- bobcat2 a0 doesn't support the feature for traffic classes.
        -- Fill tcRange with single value to call the API once.
        tcRange = {0}
    elseif tcRange == nil then
        -- sip5.10 and above.
        -- if params.tcRange is not specified treat it as 'all'
        tcRange = {}
        for tc=0,trafficClassMaxGet(devNum) do
            table.insert(tcRange, tc)
        end
    end

    for _, tc in pairs(tcRange) do
        local isErr, result, values = genericCpssApiWithErrorHandler(
            command_data, "cpssDxChPortLoopbackEnableSet",
            {
                {"IN", "GT_U8",   "devNum",     devNum},
                {"IN", "GT_U32",  "srcProfile", rxProf},
                {"IN", "GT_U32",  "trgProfile", txProf},
                {"IN", "GT_U32",  "tc",         tc},
                {"IN", "GT_BOOL", "enable",     enable}
        })
        if isErr then
            return
        end
    end
end


-- ************************************************************************
---
--  pktTypeFwdFromCpuEnablePerDev
--        @description  enable forwarding to Loopback Port for
--                      FORWARD,  FROM_CPU packet types
--
--        @param params CLI command parameters
--
--        @return       true on success, otherwise false and error message
--
local function pktTypeFwdFromCpuEnablePerDev(command_data, devNum, params)
    if not is_supported_feature(devNum, "FWD_TO_LB") then
        command_data:addErrorAndPrint(
            "device %d doesn't support 'Forwarding to Loopback Port'",
            devNum)
        return
    end

    local singleTargetEn = not (params.flagNo or params.multiTarget)
    local multiTargetEn  = not (params.flagNo or params.unicast)
    local fromCpuEn      = not (params.flagNo or params.forwardOnly)

    local isErr, result, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet",
        {
            { "IN", "GT_U8",     "devNum",         devNum},
            { "IN", "GT_BOOL",   "fromCpuEn",      fromCpuEn},
            { "IN", "GT_BOOL",   "singleTargetEn", singleTargetEn},
            { "IN", "GT_BOOL",   "multiTargetEn",  multiTargetEn}
    })
    if isErr then
        return
    end
end

-- ************************************************************************
---
--  pktTypeToCpuEnablePerDev
--        @description  enable forwarding to Loopback Port for
--                      TO_CPU packet type
--
--        @param params CLI command parameters
--
--        @return       true on success, otherwise false and error message
--
local function pktTypeToCpuEnablePerDev(command_data, devNum, params)
    if not is_supported_feature(devNum, "FWD_TO_LB") then
        command_data:addErrorAndPrint(
            "device %d doesn't support 'Forwarding to Loopback Port'",
            devNum)
        return
    end

    for _, cpuCode in pairs(params.cpuCodeRange) do
        cpuCode = dsa_cpu_code_to_api_convert(cpuCode)
        local isErr, result, values = genericCpssApiWithErrorHandler(
            command_data, "cpssDxChPortLoopbackPktTypeToCpuSet",
            {
                {"IN",  "GT_U8",                    "devNum",          devNum},
                {"IN",  "CPSS_NET_RX_CPU_CODE_ENT", "cpuCode",         cpuCode},
                {"IN",  "GT_PHYSICAL_PORT_NUM",     "loopbackPortNum", params.lbPort},
                {"IN",  "GT_BOOL",                  "enable",          not params.flagNo}
        })
        if isErr then
            return
        end
    end
end

-- ************************************************************************
---
--  pktTypeToAnalyzerEnablePerDev
--        @description  enable forwarding to Loopback Port for
--                      TO_ANALYZER packet types
--
--        @param params CLI command parameters
--
--        @return       true on success, otherwise false and error message
--
local function pktTypeToAnalyzerEnablePerDev(command_data, devNum, params)
    if not is_supported_feature(devNum, "FWD_TO_LB") then
        command_data:addErrorAndPrint(
            "device %d doesn't support 'Forwarding to Loopback Port'",
            devNum)
        return
    end

    local isErr, result, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPortLoopbackPktTypeToAnalyzerSet",
        {
            {"IN",  "GT_U8",               "devNum",          devNum},
            {"IN",  "GT_U32",              "analyzerIndex",   params.analyzerIndex},
            {"IN", "GT_PHYSICAL_PORT_NUM", "loopbackPortNum", params.lbPort},
            {"IN", "GT_BOOL",              "enable",          not params.flagNo}
    })
    if isErr then
        return
    end
end

local function evidxMappingEnablePerDev(command_data, devNum, params)
    if not is_supported_feature(devNum, "FWD_TO_LB") then
        command_data:addErrorAndPrint(
            "device %d doesn't support 'Forwarding to Loopback Port'",
            devNum)
        return
    end

    local isErr, result, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPortLoopbackEvidxMappingSet",
        {
            {"IN",  "GT_U8",   "devNum",     devNum},
            {"IN",  "GT_U32",  "vidxOffset", params.vidxOffset},
            {"IN",  "GT_BOOL", "enable",     not params.flagNo}
    })
    if isErr then
        return
    end
end


local function egressMirroringEnablePerDev(command_data, devNum, params)
    if not is_supported_feature(devNum, "FWD_TO_LB") then
        command_data:addErrorAndPrint(
            "device %d doesn't support 'Forwarding to Loopback Port'",
            devNum)
        return
    end

    local isErr, result, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPortLoopbackEnableEgressMirroringSet",
        {
            {"IN",  "GT_U8",  "devNum", devNum},
            {"IN", "GT_BOOL", "enable", not params.flagNo}
    })
    if isErr then
        return
    end
end




--------------------------------------------------------------------------------
-- command registration: forward-to-loopback profile
--------------------------------------------------------------------------------
CLI_addHelp("config", "forward-to-loopback",
            "'Forwarding to Loopback/Service Port' commands")

CLI_addCommand({"config"}, "forward-to-loopback enable",
    {
        func = function(params)
            return generic_all_device_func(fwdToLbEnablePerDev, params)
        end,
        help = "enable forwarding to Loopback Port generally",
        params = {
            { type = "named",
              "#all_device",
              { format = "rx-profile %loopback_profile", name = "rxProfile", help = "ingress loopback profile"},
              { format = "tx-profile %loopback_profile", name = "txProfile", help = "egress loopback profile"},
              { format = "traffic-classes %traffic_class_range ", name = "tcRange", help = "traffic classes range"},
              mandatory = {"all_device", "rxProfile", "txProfile"}
            },
        }
})

--------------------------------------------------------------------------------
-- command registration: forward-to-loopback profile
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "no forward-to-loopback enable",
    {
        func=function(params)
            params.flagNo=true
            return generic_all_device_func(fwdToLbEnablePerDev, params)
        end,
        help = "disable forwarding to loopback/service port generally",
        params = {
            { type = "named",
              "#all_device",
              { format = "rx-profile %loopback_profile", name = "rxProfile", help = "ingress loopback profile"},
              { format = "tx-profile %loopback_profile", name = "txProfile", help = "egress loopback profile"},
              { format = "traffic-classes %traffic_class_range ", name = "tcRange", help = "traffic classes range"},
              mandatory = {"all_device", "rxProfile", "txProfile"}
            },

        }
})

--------------------------------------------------------------------------------
-- command registration: forward-to-loopback packet-type-enable forward-from-cpu
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "forward-to-loopback packet-type-enable forward-from-cpu",
    {
        func = function(params)
            return generic_all_device_func(pktTypeFwdFromCpuEnablePerDev, params)
        end,
        help = "enable forwarding to Loopback Port for FORWARD, FROM_CPU packet types",
        params = {
            { type = "named",

              "#all_device",
              {format="unicast", name="unicast", help = "unicast traffic"},
              {format="multi-target", name="multiTarget", help = "multi-target traffic"},
              {format="any", name="any", help = "any kind (unicast, multi-target) of traffic"},
              {format="forward-only", name="forwardOnly", help = "enable for FORWARD packet type only"},
              alt = {trafficKind = {"unicast", "multiTarget", "any"}},
              requirements =
                  {["trafficKind"] = {"all_device"}},
              mandatory    = {"all_device"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: no forward-to-loopback packet-type-enable forward-from-cpu
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "no forward-to-loopback packet-type-enable forward-from-cpu",
    {
        func = function(params)
            params.flagNo = true
            return generic_all_device_func(pktTypeFwdFromCpuEnablePerDev, params)
        end,
        help = "disable forwarding to loopback/service port for FORWARD, FROM_CPU packet types",
        params = {
            { type = "named",
              "#all_device",
              mandatory = {"all_device"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: forward-to-loopback packet-type-enable to-cpu
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "forward-to-loopback packet-type-enable to-cpu",
    {
        func = function(params)
            return generic_all_device_func(pktTypeToCpuEnablePerDev, params)
        end,
        help = "enable forwarding to Loopback Port for TO_CPU packet type",
        params = {
            { type = "named",
              "#all_device",
              {format = "cpu-codes %cpu_code_range", name = "cpuCodeRange", help = "CPU codes range"},
              {format = "loopback-port %port", name = "lbPort", help = "Loopback Port number"},
              requirements = {
                  ["cpuCodeRange"] = {"all_device"},
                  ["lbPort"]       = {"all_device"}},
              mandatory = {"all_device", "cpuCodeRange", "lbPort"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: no forward-to-loopback packet-type-enable to-cpu
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "no forward-to-loopback packet-type-enable to-cpu",
    {
        func = function(params)
            params.flagNo = true
            return generic_all_device_func(pktTypeToCpuEnablePerDev, params)
        end,
        help = "disable forwarding to Loopback Port for TO_CPU packet type",
        params = {
            { type = "named",
              "#all_device",
              {format = "cpu-codes %cpu_code_range", name = "cpuCodeRange", help = "CPU codes range"},
              requirements = {["cpuCodeRange"] = {"all_device"}},
              mandatory = {"all_device", "cpuCodeRange"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: forward-to-loopback packet-type-enable to-analyzer
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "forward-to-loopback packet-type-enable to-analyzer",
    {
        func = function(params)
            return generic_all_device_func(pktTypeToAnalyzerEnablePerDev, params)
        end,
        help = "enable forwarding to Loopback Port for TO_ANALYZER packet type",
        params = {
            { type = "named",
              "#all_device",
              {format = "analyzer-index %analyzer_index", name="analyzerIndex", help = "analyzer index"},
              {format = "loopback-port %port", name = "lbPort", help = "Loopback Port number"},
              requirements = {
                  ["analyzerIndex"] = {"all_device"},
                  ["lbPort"]        = {"all_device"},
              },
              mandatory = {"all_device", "analyzerIndex", "lbPort"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: no forward-to-loopback packet-type-enable to-analyzer
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "no forward-to-loopback packet-type-enable to-analyzer",
    {
        func = function(params)
            params.flagNo = true
            return generic_all_device_func(pktTypeToAnalyzerEnablePerDev, params)
        end,
        help = "enable forwarding to Loopback Port for TO_ANALYZER packet type",
        params = {
            { type = "named",
              "#all_device",
              {format="analyzer-index %analyzer_index", name="analyzerIndex", help = "analyzer index"},
              requirements = {
                  ["analyzerIndex"] = {"all_device"}},

              mandatory = {"all_device", "analyzerIndex"}
            }
        }
    }
)


--------------------------------------------------------------------------------
-- command registration: forward-to-loopback evidx-mapping
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, " forward-to-loopback evidx-mapping",
    {
        func = function(params)
            return generic_all_device_func(evidxMappingEnablePerDev, params)
        end,
        help = "enable eVIDX mapping for multi-target traffic forwarded to Loopback Port and assing an eVIDX offset value used for the new eVIDX value calculation.",
        params = {
            { type = "named",
              "#all_device",
              {format="vidx-offset %evidx", name="vidxOffset", help = "eVIDX offset"},
              mandatory = {"all_device", "vidxOffset"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: no forward-to-loopback evidx-mapping
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "no forward-to-loopback evidx-mapping",
    {
        func = function(params)
            params.flagNo = true
            return generic_all_device_func(evidxMappingEnablePerDev, params)
        end,
        help = "disable eVIDX mapping for multi-target traffic forwarded to Loopback Port",
        params = {
            { type = "named",
              "#all_device",
              mandatory = {"all_device"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: forward-to-loopback egress-mirroring
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, " forward-to-loopback egress-mirroring",
    {
        func = function(params)
            return generic_all_device_func(egressMirroringEnablePerDev, params)
        end,
        help = "Enable ePort/eVlan Egress Mirroring for packet forwarded to loopback/service port.",
        params = {
            { type = "named",
              "#all_device",
              mandatory = {"all_device"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: no forward-to-loopback egress-mirroring
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "no forward-to-loopback egress-mirroring",
    {
        func = function(params)
            params.flagNo = true
            return generic_all_device_func(egressMirroringEnablePerDev, params)
        end,
        help = "Disable ePort/eVlan Egress Mirroring for packet forwarded to loopback/service port.",
        params = {
            { type = "named",
              "#all_device",
              mandatory = {"all_device"}
            }
        }
    }
)
