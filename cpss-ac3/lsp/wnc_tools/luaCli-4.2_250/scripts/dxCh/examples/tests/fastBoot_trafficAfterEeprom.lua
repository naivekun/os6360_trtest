--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fastBoot_trafficAfterEeprom.lua
--*
--* DESCRIPTION:
--*       fastBoot: check traffic after Eeprom configuration
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
SUPPORTED_DEV_DECLARE(devEnv.dev, "CPSS_PP_FAMILY_DXCH_LION_E", "CPSS_PP_FAMILY_DXCH_XCAT_E", "CPSS_PP_FAMILY_DXCH_XCAT2_E", "CPSS_PP_FAMILY_DXCH_XCAT3_E", "CPSS_PP_FAMILY_DXCH_BOBCAT2_E","CPSS_PP_FAMILY_DXCH_LION2_E","CPSS_PP_FAMILY_DXCH_ALDRIN_E","CPSS_PP_FAMILY_DXCH_AC3X_E")

--includes
cmdLuaCLI_registerCfunction("wrlDxChPortsModeSpeedSet")

local error = false
local port1
local port2
local port3
local port4
local commonCpuPort = 63

-- special definition for customer
local devNum  = devEnv.dev
local devFamily, subFamily

devFamily, subFamily = wrlCpssDeviceFamilyGet(devNum)

local CETUS_DEV_ID = 0xBE00
local CAELUM_DEV_ID = 0xBC00

devType = wrlCpssDeviceTypeGet(devNum)

if math.floor(devType / 0x10000) == CAELUM_DEV_ID then
    setTestStateSkipped()
    return
end

if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_NONE_E" == subFamily) then
    -- force link up + reset mac counters is done only for ports 4,58
    port1   = 4
    port2   = 67 -- dummy the port has link down
    port3   = 58
    port4   = 69 -- dummy the port has link down
elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily) then
    -- force link up + reset mac counters is done only for ports 56,59
    port1   = 56
    port2   = 64 -- dummy the port has link down
    port3   = 59
    port4   = 71 -- dummy the port has link down
elseif ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily) then
    -- force link up + reset mac counters is done only for ports 8,24
    port1   = 8
    port2   = 2 -- dummy the port has link down
    port3   = 24
elseif ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily)
then
    port1   = 8
    port2   = 2 -- dummy the port has link down
    port3   = 25
else
    -- force link up + reset mac counters is done only for ports 0,8
    port1   = 0
    port2   = 2 -- dummy the port has link down
    port3   = 8
end

-- special definition for customer

local pktInfo
local payload
local rc
local result,value
local cntrValue
local configFileName
local deconfigFileName = "fastBoot_trafficAfterEeprom"

local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
        local ret = cpssGenWrapper("simLogTaskAddLog",{{"IN","string","str","LUA: "..debugString.."\n"}}, true)
    end
end


local function startHwTrace()
    local ret
    print("\n\n\nStart hw access trace\n\n\n")
    ret = myGenWrapper("appDemoTraceHwAccessOutputModeSet",{{"IN",TYPE["ENUM"],"mode",2}})  --output to db

    print("ret is:"..ret)

     if (ret==0) then
        ret = myGenWrapper("appDemoTraceHwAccessEnable",{
            {"IN","GT_U8","devNum",0},
            {"IN",TYPE["ENUM"],"accessType",1},  -- 0 -read, 1--write 2-both 3-delay&write 4-all
            {"IN","GT_BOOL","enable",true}
        })
     end

        if (ret~=0) then
            print("Could not perform trace action, status is:"..ret)
            return false,"Could not perform trace action, status is:"..ret
        end

    return true
end

local function stopHwTrace()
    local ret

    print("\n\n\nStop hw access trace\n\n\n")

    ret = myGenWrapper("appDemoTraceHwAccessEnable",{
        {"IN","GT_U8","devNum",0},
        {"IN",TYPE["ENUM"],"accessType",1},  -- 0 -read, 1--write 2-both 3-delay&write 4-all
        {"IN","GT_BOOL","enable",0}
    })

        if (ret~=0) then
            print("Could not perform trace action, status is:"..ret)
            return false,"Could not perform trace action, status is:"..ret
        end

    return true
end

local function callWithErrorHandling(cpssApi, params)
   _debug("Try to call "..cpssApi)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       _debug("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   else
       _debug("OK of calling "..cpssApi)

   end
   return rc, value
end

local delayTimeInSec = 4.095

if ("CPSS_PP_FAMILY_DXCH_XCAT_E" == devFamily) then
    configFileName = "fastBoot_xCat_trafficAfterEeprom"
elseif ("CPSS_PP_FAMILY_DXCH_XCAT2_E" == devFamily) then
    configFileName = "fastBoot_xCat2_trafficAfterEeprom"
elseif ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
    configFileName = "fastBoot_xCat3_trafficAfterEeprom"
elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_NONE_E" == subFamily) then
    configFileName = "fastBoot_bobcat2_trafficAfterEeprom"
elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily) then
    configFileName = "fastBoot_Cetus_trafficAfterEeprom"
elseif ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily) then
    configFileName = "fastBoot_Lion2_trafficAfterEeprom"
elseif ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) then
    configFileName = "fastBoot_Aldrin_trafficAfterEeprom"
elseif ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) then
    configFileName = "fastBoot_Aldrin_trafficAfterEeprom"
else
    configFileName = "fastBoot_Lion_trafficAfterEeprom"
end

local function clearCounters()
    _debug("\n---------- CLEAR counters ------------")

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_RCV_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    -- read egress counters
    result, value = myGenWrapper(
        "appDemoDxChPortEgressCntrsGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U8",                        "cntrSetNum",   0 },
        { "OUT",    "CPSS_PORT_EGRESS_CNTR_STC",    "egrCntrPtr"}
    })

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    -- read Ingress Drop Counter
    result, value = myGenWrapper(
        "appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",    "devNum",    devNum  },
        { "OUT",    "GT_U32",   "counterPtr"}
    })
end

local function readCounters()

    _debug("\n---------- read counters ------------")

    --Get counter after sleep
    _debug("delay for 1 second")
    delay(1000 * delayTimeInSec) -- one second

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_RCV_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value.cntrValuePtr))
    _debug("port "..port1.." CPSS_GOOD_OCTETS_RCV_E  "..cntrValue)
    if tonumber(cntrValue) == 0 then
        print("ERROR in value of counter CPSS_GOOD_OCTETS_RCV_E "..cntrValue.." on port "..port1)
        error = true
    end

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value.cntrValuePtr))
    _debug("port "..port1.." CPSS_GOOD_OCTETS_SENT_E  "..cntrValue)
    if tonumber(cntrValue) == 0 then
        print("ERROR in value of counter CPSS_GOOD_OCTETS_SENT_E "..cntrValue.." on port "..port1)
        error = true
    end

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value.cntrValuePtr))
    _debug("port "..port3.." CPSS_GOOD_OCTETS_SENT_E  "..cntrValue)
    if tonumber(cntrValue) == 0 then
        print("ERROR in value of counter CPSS_GOOD_OCTETS_SENT_E "..cntrValue.." on port "..port3)
        error = true
    end

    --------- read drop counters ------------

    _debug("\n---------- read DROP counters ------------")

    -- read egress counters
    result, value = myGenWrapper(
        "appDemoDxChPortEgressCntrsGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U8",                        "cntrSetNum",   0 },
        { "OUT",    "CPSS_PORT_EGRESS_CNTR_STC",    "egrCntrPtr"}
    })
    _debug("Egress drop counters: ")
    _debug("brgEgrFilterDisc "..to_string(value.egrCntrPtr.brgEgrFilterDisc))
    _debug("txqFilterDisc "..to_string(value.egrCntrPtr.txqFilterDisc))
    _debug("egrFrwDropFrames "..to_string(value.egrCntrPtr.egrFrwDropFrames))
    _debug("mcFifo3_0DropPkts "..to_string(value.egrCntrPtr.mcFifo3_0DropPkts))
    _debug("mcFifo7_4DropPkts "..to_string(value.egrCntrPtr.mcFifo7_4DropPkts))
    -- ***** Waiting for bug fix in simulation
    --if tonumber(value.egrCntrPtr.brgEgrFilterDisc) ~= 0 then
    --    print("ERROR in value of egress drop counter brgEgrFilterDisc "..to_string(value.egrCntrPtr.brgEgrFilterDisc))
    --    error = true
    --end
    if tonumber(value.egrCntrPtr.txqFilterDisc) ~= 0 then
        print("ERROR in value of egress drop counter txqFilterDisc "..to_string(value.egrCntrPtr.txqFilterDisc))
        error = true
    end
    if tonumber(value.egrCntrPtr.egrFrwDropFrames) ~= 0 then
        print("ERROR in value of egress drop counter egrFrwDropFrames "..to_string(value.egrCntrPtr.egrFrwDropFrames))
        error = true
    end
    if tonumber(value.egrCntrPtr.mcFifo3_0DropPkts) ~= 0 then
        print("ERROR in value of egress drop counter mcFifo3_0DropPkts "..to_string(value.egrCntrPtr.mcFifo3_0DropPkts))
        error = true
    end
    if tonumber(value.egrCntrPtr.mcFifo7_4DropPkts) ~= 0 then
        print("ERROR in value of egress drop counter mcFifo7_4DropPkts "..to_string(value.egrCntrPtr.mcFifo7_4DropPkts))
        error = true
    end

    --read the drop event counter
    _debug("\nDrop event counters:")
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value["cntrValuePtr"]))
    _debug("port "..port1.." CPSS_DROP_EVENTS_E  "..cntrValue)
    if tonumber(cntrValue) ~= 0 then
        print("ERROR in value of counter CPSS_DROP_EVENTS_E "..cntrValue.." on port "..port1)
        error = true
    end

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value["cntrValuePtr"]))
    _debug("port "..port3.." CPSS_DROP_EVENTS_E  "..cntrValue)
    if tonumber(cntrValue) ~= 0 then
        print("ERROR in value of counter CPSS_DROP_EVENTS_E "..cntrValue.." on port "..port3)
        error = true
    end

    -- read Ingress Drop Counter
    result, value = myGenWrapper(
        "appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",    "devNum",    devNum  },
        { "OUT",    "GT_U32",   "counterPtr"}
    })
    _debug("\nIngress drop counter "..to_string(value.counterPtr))
    if tonumber(value.counterPtr) ~= 0 then
        print("ERROR in value of ingress drop counter "..to_string(value.counterPtr))
        error = true
    end

end

-- restore Eeprom configuration for Bobcat2
local function restoreEepromConfigBobcat2()

    print("\nRestore Eeprom configuration")

    local tcQueue={0,1,2,3,4,5,6,7}
    for i=1,#tcQueue do
        --Enable enqueuing to a Traffic Class queue
        callWithErrorHandling (
            "prvCpssDxChPortTxQueueingEnableSet", {
            { "IN", "GT_U8",                "devNum",      devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",     58  },
            { "IN", "GT_U8",                "tcQueue",     tcQueue[i]  },
            { "IN", "GT_BOOL",              "enable",      true  }
        })

    end





    local ports={58}
    for i=1,#ports do
        --Enable a specified port
        callWithErrorHandling (
            "prvCpssDxChPortEnableSet", {
            { "IN", "GT_U8",                    "devNum",   devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",  ports[i] },
            { "IN", "GT_BOOL",                   "enable",  true   }

        })

    end
    callWithErrorHandling(
        "appDemoDxChPortModeSpeedSet", {
        { "IN", "GT_U8",        "devNum", devNum  }
    })
end

-- restore Eeprom configuration for Lion
local function restoreEepromConfigLion()

    print("\nRestore Eeprom configuration")

    local vlans={0x201,0x401,0x801}
    for i=1,#vlans do
        --invalidate VLAN
        callWithErrorHandling (
            "cpssDxChBrgVlanEntryInvalidate", {
            { "IN", "GT_U8",     "devNum",      devNum  },
            { "IN", "GT_U16",    "vlanId",      vlans[i]  }
        })

    end

    local ports={0,2,4,6,8,9,34,36,38,39,40,41,42}
    for i=1,#ports do
        --disable STP state of port
        callWithErrorHandling (
            "cpssDxChBrgStpStateSet", {
            { "IN", "GT_U8",                    "dev",      devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",  ports[i] },
            { "IN", "GT_U16",                   "stpId",    0   },
            { "IN", "CPSS_STP_STATE_ENT",       "state",    "CPSS_STP_DISABLED_E" }

        })

    end

    ports={0,2,4,6,8,9}
    for i=1,#ports do
        --set port's default VID0
        callWithErrorHandling (
                "cpssDxChBrgVlanPortVidSet", {
                { "IN", "GT_U8",                    "devNum",      devNum  },
                { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                { "IN", "CPSS_DIRECTION_ENT",       "direction",   "CPSS_DIRECTION_INGRESS_E" },
                { "IN", "GT_U16",                   "vlanId",      1}

        })

        --disables ingress policy per port
        callWithErrorHandling (
                "cpssDxChPclPortIngressPolicyEnable", {
                { "IN", "GT_U8",                    "devNum",      devNum  },
                { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                { "IN", "GT_BOOL",                  "enable",      false }

        })

        --enable forwarding a new mac address message to CPU per port
        callWithErrorHandling (
                "cpssDxChBrgFdbNaToCpuPerPortSet", {
                { "IN", "GT_U8",                    "devNum",      devNum  },
                { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                { "IN", "GT_BOOL",                  "enable",      true }

        })

        --enable learning of new source MAC addresses for packets received on port
        callWithErrorHandling (
                "cpssDxChBrgFdbPortLearnStatusSet", {
                { "IN", "GT_U8",                    "devNum",      devNum  },
                { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                { "IN", "GT_BOOL",                  "status",      true },
                { "IN", "CPSS_PORT_LOCK_CMD_ENT",   "cmd",         "CPSS_LOCK_FRWRD_E" }

        })

        --disable Ingress Filtering for specific port
        callWithErrorHandling (
                "cpssDxChBrgVlanPortIngFltEnable", {
                { "IN", "GT_U8",                    "devNum",      devNum  },
                { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                { "IN", "GT_BOOL",                  "enable",      false }

        })

        --set port access frame type
        callWithErrorHandling (
                "cpssDxChBrgVlanPortAccFrameTypeSet", {
                { "IN", "GT_U8",                            "devNum",      devNum  },
                { "IN", "GT_PORT_NUM",                      "portNum",     ports[i] },
                { "IN", "CPSS_PORT_ACCEPT_FRAME_TYPE_ENT",  "frameType",   "CPSS_PORT_ACCEPT_FRAME_ALL_E" }

        })

    end

    --Set MRU value for a VLAN MRU profile
    callWithErrorHandling (
                "cpssDxChBrgVlanMruProfileValueSet", {
                { "IN", "GT_U8",                    "devNum",      devNum  },
                { "IN", "GT_U32",                   "mruIndex",    0 },
                { "IN", "GT_U32",                   "mruValue",    0x05F2 }

    })

    local rules={100,101,102}
    for i=1,#rules do
        --invalidates the Policy rule
        callWithErrorHandling (
                    "cpssDxChPclPortGroupRuleValidStatusSet", {
                    { "IN", "GT_U8",                    "devNum",        devNum  },
                    { "IN", "GT_PORT_GROUPS_BMP",       "portGroupsBmp", 1  }, -- portGroup 0 in bitmap
                    { "IN", "CPSS_PCL_RULE_SIZE_ENT",   "ruleSize",      "CPSS_PCL_RULE_SIZE_STD_E"  },
                    { "IN", "GT_U32",                   "ruleIndex",     rules[i]  },
                    { "IN", "GT_BOOL",                  "valid",         false }

        })
    end

    -- Fast ports configuration - not including port 0,2,4,6,8,9 configured in the eeprom
    callWithErrorHandling (
                "appDemoDxChPortFastInit", {
                { "IN", "GT_U8", "devNum", devNum}
    })

end

-- restore Eeprom configuration for xCat
local function restoreEepromConfigXCat()

    print("\nRestore Eeprom configuration")

    -- enable the sending to CPU of NA
    for port=0,27,1 do
        callWithErrorHandling(
        "cpssDxChBrgFdbNaToCpuPerPortSet", {
            { "IN", "GT_U8",                    "devNum",      devNum   },
            { "IN", "GT_PORT_NUM",              "portNum",     port     },
            { "IN", "GT_BOOL",                  "enable",      true     }
        })
    end

    -- enable the sending to CPU of NA
    callWithErrorHandling(
    "cpssDxChBrgFdbNaToCpuPerPortSet", {
        { "IN", "GT_U8",                    "devNum",      devNum   },
        { "IN", "GT_PORT_NUM",              "portNum",     63       },
        { "IN", "GT_BOOL",                  "enable",      true     }
    })

    -- enable the sending to CPU of AA/TA
    callWithErrorHandling(
    "cpssDxChBrgFdbAAandTAToCpuSet", {
        { "IN", "GT_U8",                    "devNum",      devNum   },
        { "IN", "GT_BOOL",                  "enable",      true     }
    })

end

-- restore Eeprom configuration for Lion2
local function restoreEepromConfigLion2()

    print("\nRestore Eeprom configuration")

    callWithErrorHandling (
        "appDemoDxChPortModeSpeedSet", {
        { "IN", "GT_U8", "devNum", devNum}
    })

end

-- restore Eeprom configuration for xCat3
local function restoreEepromConfigXCat3()

    print("\nRestore Eeprom configuration")
    local result, values
    local portsMembers = {}

    -- configure ports in KR mode
    portsMembers = {27--[[,28,29]]}
    result, values =  wrLogWrapper("wrlDxChPortsModeSpeedSet",
                        "(devNum, portsMembers, true, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E)",
                        devNum, portsMembers, true, "CPSS_PORT_INTERFACE_MODE_KR_E", "CPSS_PORT_SPEED_10000_E")
   if result ~= 0 then
       setFailState()
       _debug("ERROR of calling wrlDxChPortsModeSpeedSet: "..returnCodes[result])
   else
       _debug("OK of calling wrlDxChPortsModeSpeedSet")
   end
end

-- run pre-test config
local function preTestConfig()

    --generate test packets
    do
        if ("CPSS_PP_FAMILY_DXCH_XCAT_E" == devFamily or "CPSS_PP_FAMILY_DXCH_XCAT2_E" == devFamily or "CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_xCat_trafficAfterEeprom.lua")

        elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_NONE_E" == subFamily) then
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_bobcat2_trafficAfterEeprom.lua")

        elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily) then
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_Cetus_trafficAfterEeprom.lua")

        elseif ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily) then
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_xCat_trafficAfterEeprom.lua")

        elseif ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) then
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_Cetus_trafficAfterEeprom.lua")

        elseif ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) then
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_Cetus_trafficAfterEeprom.lua")

        else
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_Lion_trafficAfterEeprom.lua")
        end
    end

    if not result then
        printLog ('Error in packet generator')
        setFailState()
        return
    end

    _debug('Load config')
    --set config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName))

    _debug('Config loaded')
end

-- run post-test config
local function postTestConfig()

    _debug("\npostTestConfig() ")
	
	
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) then
        _debug("\rRestore port mapping ")
        --restore port mapping  
        callWithErrorHandling (
        "appDemoDxChPortMappingRestore", {
            { "IN", "GT_U8", "devNum", devNum}
        })
    end
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily) then
        callWithErrorHandling(
        "appDemoDxChPortModeSpeedSet", {
            { "IN", "GT_U8",        "devNum", devNum  }
        })
    end

    if ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily)
    then
        callWithErrorHandling (
        "appDemoDxChPortMappingRestore", {
            { "IN", "GT_U8", "devNum", devNum}
        })
        callWithErrorHandling(
        "appDemoDxChPortModeSpeedSet", {
            { "IN", "GT_U8",        "devNum", devNum  }
        })
    end

    --unset set force link up for port3
    callWithErrorHandling(
        -- opposite of "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr"
        "cpssDxChPortForceLinkPassEnableSet", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port3 },
        { "IN", "GT_BOOL",              "state",    false }
    })

    --unset set force link up for port1
    callWithErrorHandling(
        -- opposite of "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr"
        "cpssDxChPortForceLinkPassEnableSet", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port1 },
        { "IN", "GT_BOOL",              "state",    false }
    })

    --[[
    local interface =  {interface={type    = "CPSS_INTERFACE_PORT_E",
                       devPort = { devNum  = devNum,
                                   portNum = port1}}}
    --]]

    callWithErrorHandling (
        -- opposite of "appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr"
        "cpssDxChPortInternalLoopbackEnableSet", {
        { "IN", "GT_U8",                    "devNum",               devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",              port1  },
        { "IN", "GT_BOOL",                  "enable",               false }
    })


    --unset port1 as RX mirror
    callWithErrorHandling(
        -- opposite of "appDemoDxChMirrorRxPortSet_HardCodedRegAddr"
        "cpssDxChMirrorRxPortSet", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_PORT_NUM",  "mirrPort",         port1   },
        { "IN", "GT_BOOL",      "enable",           false },
        { "IN", "GT_U32",       "index",            0 }
    })

    interface =  {interface={type    = "CPSS_INTERFACE_PORT_E",
                  devPort = { devNum  = 0x10,
                              portNum = port1}}}
    --unset port1 as analyzer port
    callWithErrorHandling(
        -- opposite of "appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr"
        "cpssDxChMirrorAnalyzerInterfaceSet", {
        { "IN", "GT_U8",                                    "devNum",       devNum },
        { "IN", "GT_U32",                                   "index",        0 },
        { "IN", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC",  "interfacePtr", interface }
    })
    callWithErrorHandling(
        -- opposite of "appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr"
        "cpssDxChMirrorAnalyzerInterfaceSet", {
        { "IN", "GT_U8",                                    "devNum",       devNum },
        { "IN", "GT_U32",                                   "index",        1 },
        { "IN", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC",  "interfacePtr", interface }
    })

    -- change mode from fastBoot test
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","fastBootSkipOwnDeviceInit"},
        {"IN","GT_U32","value",0}
    })

    -- change mode from fastBoot test
    callWithErrorHandling(
        "cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet", {
        { "IN", "GT_BOOL",  "enable",       false }
    })

    if (("CPSS_PP_FAMILY_DXCH_XCAT2_E" == devFamily) or ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily)) then
        ports={0,2,4,6,8,9}
        for i=1,#ports do
            callWithErrorHandling (
                    "cpssDxChBrgFdbNaStormPreventSet", {
                    { "IN", "GT_U8",                    "devNum",      devNum  },
                    { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                    { "IN", "GT_BOOL",                  "enable",      false }

            })
        end
    end

    -- restore configurtion from Eeprom
    if ("CPSS_PP_FAMILY_DXCH_LION_E" == devFamily) then
        restoreEepromConfigLion()
    end
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_NONE_E" == subFamily) then
        restoreEepromConfigBobcat2()
        fillDeviceEnvironment()
    end
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily) then
        fillDeviceEnvironment()
    end
    if ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) then
        fillDeviceEnvironment()
    end
    if ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) then
        fillDeviceEnvironment()
    end
    if ("CPSS_PP_FAMILY_DXCH_XCAT_E" == devFamily) then
        restoreEepromConfigXCat()
    end
    if ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily) then
        restoreEepromConfigLion2()
    end
    if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
        restoreEepromConfigXCat3()
    end

	--restore default vlan and pvid   
        callWithErrorHandling (
        "appDemoDxChVlanPvidRestore", {
            { "IN", "GT_U8", "devNum", devNum}
        })
    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName(deconfigFileName,true))
end


local function mainFunction()

    _debug("\nmainFunction() ")

    printLog("\n Start ... \n")
    ------------------------------ SYSTEM CONFIGURATION PART ---------------------
    --[[ System configuration
      1.    ports port1,port3 belong to vlan X.
      2.    port1 is in 'loopback' mode.
      3.    sending packet with 'dsa' tag 'from_cpu' to cpu port 63.
      4.    the packet egress from port1 without 'dsa' tag on vlan X.
      5.    the loopback ingress the packet again to port1.
      6.    the packet egress from port3 and the replication to port1 is filtered.
      for   xCat, xCat2, xCat3: X=1, for Lion: X=0x801, for bobcat2: X=4094
    ]]--

    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) then
        --bind the DQ port to the DMA number
        callWithErrorHandling(
            "appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "txqNum",           0x3F },
            { "IN", "GT_U32",       "txDmaNum",         0x48 } --for BOBCAT2 SDMA CPU port number is 0x48
        })


        callWithErrorHandling(
            "appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "txqNum",           port3 },
            { "IN", "GT_U32",       "txDmaNum",         port3 }
        })

        callWithErrorHandling(
            "appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "txqNum",           port1 },
            { "IN", "GT_U32",       "txDmaNum",         port1 }
        })

        --configure BMA local physical port 2 RxDMA port  mapping
        callWithErrorHandling(
            "appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         0x3F },
            { "IN", "GT_U32",       "rxDMAPort",        0x48 } --for BOBCAT2 SDMA CPU port number is 0x48
        })

        if ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" ~= devFamily) and
            ("CPSS_PP_FAMILY_DXCH_AC3X_E" ~= devFamily)
        then
            callWithErrorHandling(
            "appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
                { "IN", "GT_U8",        "devNum",           devNum  },
                { "IN", "GT_U32",       "physPort",         port1 },
                { "IN", "GT_U32",       "rxDMAPort",        port1 }
            })

            callWithErrorHandling(
            "appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
                { "IN", "GT_U8",        "devNum",           devNum  },
                { "IN", "GT_U32",       "physPort",         port3 },
                { "IN", "GT_U32",       "rxDMAPort",        port3 }
            })
        end

        --configure RxDMA 2 phys port mapping
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         0x48 }, --for BOBCAT2 SDMA CPU port number is 0x48
            { "IN", "GT_U32",       "physPort",         0x3F },

        })

        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port1 },
            { "IN", "GT_U32",       "physPort",         port1 },

        })

        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port3 },
            { "IN", "GT_U32",       "physPort",         port3 },

        })

        --Bobcat2 Port mapping initial configuration
        callWithErrorHandling(
            "appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         port1 }

        })

        callWithErrorHandling(
            "appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         port3 }

        })

        callWithErrorHandling(
            "appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         63 }

        })

        local interface  ={type    = "CPSS_INTERFACE_PORT_E",
                           devPort = { devNum  = 0x10,
                                       portNum = port1}}

        --Set physical info for ePort 4 in the E2PHY mapping table
        callWithErrorHandling(
            "appDemoDxChBrgEportToPhysicalPortTargetMappingTableSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                    "devNum",           devNum  },
            { "IN", "GT_PORT_NUM",              "portNum",          port1 },
            { "IN", "CPSS_INTERFACE_INFO_STC",  "physicalInfoPtr",  interface }

        })

    end

    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) then
        --set cpu port as ingress cascade
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      commonCpuPort   },
            { "IN", "GT_U32",                  "portRxDmaNum", 0x48   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E" }
        })
    else
        --set cpu port as ingress cascade
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      commonCpuPort   },
            { "IN", "GT_U32",                  "portRxDmaNum", commonCpuPort   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E" }
        })
    end

    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) then
        --init cpu port resources
        callWithErrorHandling(
            "appDemoDxChCpuPortPizzaResources_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  }
        })
    end
    local interface  ={type    = "CPSS_INTERFACE_PORT_E",
                       devPort = { devNum  = devNum,
                                   portNum = port1}}
    --startHwTrace()
    -- set port1 as MAC loopback
    callWithErrorHandling (
        "appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                    "devNum",               devNum  },
        { "IN", "CPSS_INTERFACE_INFO_STC",  "portInterfacePtr",     interface  },
        { "IN", "GT_BOOL",                  "enable",               true }
    })

    --stopHwTrace()

    --set port1 as RX mirror
    callWithErrorHandling(
        "appDemoDxChMirrorRxPortSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_PORT_NUM",  "mirrPort",         port1   },
        { "IN", "GT_BOOL",      "enable",           true },
        { "IN", "GT_U32",       "index",            0 }
    })

    --set own device
    callWithErrorHandling(
        "setOwnDevice_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  }
    })

    --set force link up for port1
    callWithErrorHandling(
        "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port1 },
        { "IN", "GT_BOOL",              "state",    true }
    })


    local interface =  {interface={type    = "CPSS_INTERFACE_PORT_E",
                       devPort = { devNum  = 0x10,
                                   portNum = port1}}}
    --set port1 as analyzer port
    callWithErrorHandling(
        "appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                                    "devNum",       devNum },
        { "IN", "GT_U32",                                   "index",        0 },
        { "IN", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC",  "interfacePtr", interface }
    })

    --set analyzer interface index
    callWithErrorHandling(
        "appDemoDxChMirrorRxGlobalAnalyzerInterfaceIndexSet_HardCodedRegAddr", {
        { "IN", "GT_U8",    "devNum",       devNum },
        { "IN", "GT_BOOL",  "enable",       true },
        { "IN", "GT_U32",   "index",        0 }
    })

    --set set force link up for port3
    callWithErrorHandling(
        "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port3 },
        { "IN", "GT_BOOL",              "state",    true }
    })

    if (("CPSS_PP_FAMILY_DXCH_XCAT2_E" == devFamily) or ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily)) then
        ports={0,2,4,6,8,9}
        for i=1,#ports do
            -- prevent NA messages storm
            callWithErrorHandling (
                    "cpssDxChBrgFdbNaStormPreventSet", {
                    { "IN", "GT_U8",                    "devNum",      devNum  },
                    { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                    { "IN", "GT_BOOL",                  "enable",      true }

            })
        end
    end

    ------------------------------ MAIN PART ---------------------------------------

    clearCounters()

    --send continuous traffic
    printLog("\nSend continuous traffic ")
    callWithErrorHandling(
        "appDemoDxChSdmaTxPacketSend_HardCodedRegAddr", {
        { "IN",     "GT_U8",       "devNum",                devNum  },
        { "IN",     "GT_BOOL",     "isPacketWithVlanTag",   false},
        { "IN",     "GT_U8",       "vid",                   1       },
        { "IN",     "TGF_PACKET_PAYLOAD_STC", "packet",     payload["broadcast_1_to_all_vlan_X"] },
        { "IN",     "GT_U8",       "dstPortNum",            port1   }
    })

    readCounters()

    -- change mode from PEX only
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","initRegDefaults"},
        {"IN","GT_U32","value",0}
    })

    -- change mode to fastBoot test
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","fastBootSkipOwnDeviceInit"},
        {"IN","GT_U32","value",1}
    })

    if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily)
    then
        -- special setting in appDemo that analyzer port will not be '0' but port1
        callWithErrorHandling(
            "appDemoDbEntryAdd",{
            {"IN","string","namePtr","fastBootSetMirrorAnalyzerPort"},
            {"IN","GT_U32","value", port1}
        })
    end

    --no DB reset, only remove device
    callWithErrorHandling(
        "cpssResetSystemNoSoftReset", {})

    local systemRecoveryInfo = {}
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
    }
    --Set system recovery state init
    printLog("\nSet system recovery state INIT ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    -- run full init system
    printLog("\nRun full init system ")
    callWithErrorHandling("cpssReInitSystem",{})

    readCounters()


    local systemRecoveryInfo = {}
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
    }

    readCounters()

    --Set system recovery state catchup
    printLog("\nSet system recovery state CATCH_UP ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    readCounters()

    local systemRecoveryInfo = {}
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
    }
    --Set system recovery state completion
    printLog("\nSet system recovery state COMPLETION ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })


    --Open interrupts
    callWithErrorHandling (
               "appDemoDxChEventRestore", {})
    delay(1000);

    --[[
        NOTE: next operation with appDemoAllowProcessingOfAuqMessages() needed because:
        1. the full wire speed packet generated 'AUQ FULL' with (the same) mac addresses.
        2. the appDemo that wanted to handle those messages was ignored by CPSS in function
        prvDxChBrgFdbAuFuMsgBlockGet() in code of:
            if ((systemRecoveryInfo.systemRecoveryState != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E) &&
                 (systemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE))
            {
                *numOfAuFuPtr = 0;/* no messages to retrieve */
                return /* it's not error for log */ GT_NO_MORE;
            }

        this means that while 'systemRecoveryState != 'COMPLETION_STATE' the appDemo could not handle the messages.

        but once the 'AUQ FULL' reached ... no more NA messages will come , and not more interrupts on that register (interrupt cause,mask register)

        so now that the systemRecoveryState == 'COMPLETION_STATE' ... we need to handle all those waiting messages!
    ]]--
    printLog("\n Allow AppDemo to handle the AUQ messages that were ignored during 'systemRecoveryState != 'COMPLETION_STATE' ")
    -- we disable and then enable to allow the appDemo to force polling into the AUQ
    callWithErrorHandling("appDemoAllowProcessingOfAuqMessages",{{"IN",  "GT_BOOL", "enable",  false}})
    callWithErrorHandling("appDemoAllowProcessingOfAuqMessages",{{"IN",  "GT_BOOL", "enable",   true}})

    readCounters()

    -- called from initSysem function that handles cpssInitSystem command ("exec" context)
    -- but should also be automatically called at reloading LUA CLI
    fillDeviceEnvironment()

end


-- run pre-test config
preTestConfig()
-- run the test
mainFunction()
-- run post-test config
postTestConfig()

if error == true then setFailState() end



