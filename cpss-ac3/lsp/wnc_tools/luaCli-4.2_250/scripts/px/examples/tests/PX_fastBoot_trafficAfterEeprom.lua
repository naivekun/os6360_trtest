--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_fastBoot_trafficAfterEeprom.lua
--*
--* DESCRIPTION:
--*       fastBoot: check traffic after Eeprom configuration
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

local skip = false
--this test is temporary skipped
if skip == true then
    setTestStateSkipped();
    return;
end 

local error = false

local devNum   = devEnv.dev
local port1    = 0
local port2    = 4
local port3    = 5
local port4    = 12
local commonCpuPort = 16

local devNum  = devEnv.dev
local devFamily, subFamily

-- special definition for customer

local payload
local rc
local result,value
local cntrValue
local configFileName   = "PX_fastBoot_trafficAfterEeprom"

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

local function clearCounters()
    _debug("\n---------- CLEAR counters ------------")

    --read the counters
    result, value = myGenWrapper(
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_RCV_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the counters
    result, value = myGenWrapper(
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the counters
    result, value = myGenWrapper(
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    -- read egress counters
    result, value = myGenWrapper(
        "appDemoPxPortEgressCntrsGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U8",                        "cntrSetNum",   0 },
        { "OUT",    "CPSS_PORT_EGRESS_CNTR_STC",    "egrCntrPtr"}
    })

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    -- read Ingress Drop Counter
    result, value = myGenWrapper(
        "appDemoPxCfgIngressDropCntrGet_HardCodedRegAddr", {
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
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
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
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
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
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
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
        "appDemoPxPortEgressCntrsGet_HardCodedRegAddr", {
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
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
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
        "appDemoPxMacCounterGet_HardCodedRegAddr", {
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
        "appDemoPxCfgIngressDropCntrGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",    "devNum",    devNum  },
        { "OUT",    "GT_U32",   "counterPtr"}
    })
    _debug("\nIngress drop counter "..to_string(value.counterPtr))
    if tonumber(value.counterPtr) ~= 0 then
        print("ERROR in value of ingress drop counter "..to_string(value.counterPtr))
        error = true
    end
end

-- run pre-test config
local function preTestConfig()

    --generate test packets
    result, payload = pcall(dofile, "px/examples/packets/PX_fastBoot_trafficAfterEeprom.lua")

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

    callWithErrorHandling(
        "appDemoPxPortModeSpeedSet", {
            { "IN", "GT_U8",        "devNum", devNum  }
        })
    --Enable forwarding to the target port
    callWithErrorHandling (
        "appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                   "devNum",    devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",   port1  },
        { "IN", "GT_BOOL",                 "enable",    false  }
    })

    --Enable forwarding to the target port
    callWithErrorHandling (
        "appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                   "devNum",    devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",   port3  },
        { "IN", "GT_BOOL",                 "enable",    false  }
    })

    --set continuous mode
    callWithErrorHandling(
        "appDemoPxIngressPortMapEntrySet_HardCodedRegAddr", {
        { "IN", "GT_SW_DEV_NUM",      "devNum",           devNum  },
        { "IN", "CPSS_PX_TABLE_ENT",  "tableType",        "CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E"   },
        { "IN", "GT_U32",             "entryIndex",       16 * port1 },
        { "IN", "CPSS_PX_PORTS_BMP",  "portsBmp",         0x0FFFE }
    })

    --set profile per src port to direct traffic to needed port
    callWithErrorHandling(
        "appDemoPxIngressPortMapEntrySet_HardCodedRegAddr", {
        { "IN", "GT_SW_DEV_NUM",      "devNum",           devNum  },
        { "IN", "CPSS_PX_TABLE_ENT",  "tableType",        "CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E"   },
        { "IN", "GT_U32",             "entryIndex",       16 * port1 },
        { "IN", "CPSS_PX_PORTS_BMP",  "portsBmp",         0x1FFFF }
    })

    --unset set force link up for port3
    callWithErrorHandling(
        -- opposite of "appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr"
        "cpssPxPortForceLinkPassEnableSet", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port3 },
        { "IN", "GT_BOOL",              "state",    false }
    })

    --unset set force link up for port1
    callWithErrorHandling(
        -- opposite of "appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr"
        "cpssPxPortForceLinkPassEnableSet", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port1 },
        { "IN", "GT_BOOL",              "state",    false }
    })

    callWithErrorHandling (
        -- opposite of "appDemoPxPortLoopbackModeEnableSet_HardCodedRegAddr"
        "prvCpssPxPortInternalLoopbackEnableSet", {
        { "IN", "GT_U8",                    "devNum",               devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",              port1  },
        { "IN", "GT_BOOL",                  "enable",               false }
    })

    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName,true))
end


local function mainFunction()

    _debug("\nmainFunction() ")

    printLog("\n Start ... \n")
    ------------------------------ SYSTEM CONFIGURATION PART ---------------------
    --[[ System configuration
      1.    ports port1,port3 belong to vlan X.
      2.    port1 is in 'loopback' mode.
      3.    sending packet to cpu port 63.
      4.    the packet egress from port1.
      5.    the loopback ingress the packet again to port1.
      6.    the packet egress from port3 and the replication to port1 is filtered.
    ]]--

    --bind the DQ port to the DMA number
    callWithErrorHandling(
        "appDemoPxPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "txqNum",           0x10 },
        { "IN", "GT_U32",       "txDmaNum",         0x10 } -- SDMA CPU port number is 0x10
    })


    callWithErrorHandling(
        "appDemoPxPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "txqNum",           port3 },
        { "IN", "GT_U32",       "txDmaNum",         port3 }
    })

    callWithErrorHandling(
        "appDemoPxPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "txqNum",           port1 },
        { "IN", "GT_U32",       "txDmaNum",         port1 }
    })

    --configure BMA local physical port 2 RxDMA port  mapping
    callWithErrorHandling(
        "appDemoPxPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "physPort",         0x10 },
        { "IN", "GT_U32",       "rxDMAPort",        0x10 } -- SDMA CPU port number is 0x48
    })

    callWithErrorHandling(
    "appDemoPxPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "physPort",         port1 },
        { "IN", "GT_U32",       "rxDMAPort",        port1 }
    })

    callWithErrorHandling(
    "appDemoPxPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "physPort",         port3 },
        { "IN", "GT_U32",       "rxDMAPort",        port3 }
    })

    --configure RxDMA 2 phys port mapping
    callWithErrorHandling(
        "appDemoPxPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "rxDmaNum",         0x10 }, -- SDMA CPU port number is 0x10
        { "IN", "GT_U32",       "physPort",         0x10 },
    })

    callWithErrorHandling(
        "appDemoPxPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "rxDmaNum",         port1 },
        { "IN", "GT_U32",       "physPort",         port1 },
    })

    callWithErrorHandling(
        "appDemoPxPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "rxDmaNum",         port3 },
        { "IN", "GT_U32",       "physPort",         port3 },
    })

    --Bobcat2 Port mapping initial configuration
    callWithErrorHandling(
        "appDemoPxPipePortMappingConfigSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "physPort",         port1 }

    })

    callWithErrorHandling(
        "appDemoPxPipePortMappingConfigSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "physPort",         port3 }
    })

    callWithErrorHandling(
        "appDemoPxPipePortMappingConfigSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_U32",       "physPort",         16 }
    })

    --init cpu port resources
    callWithErrorHandling(
        "appDemoPxCpuPortPizzaResources_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  }
    })

    --set force link up for port1
    callWithErrorHandling(
        "appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port1 },
        { "IN", "GT_BOOL",              "state",    true }
    })

    --set force link up for port3
    callWithErrorHandling(
        "appDemoPxPortForceLinkPassEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port3 },
        { "IN", "GT_BOOL",              "state",    true }
    })

    local interface  ={type    = "CPSS_INTERFACE_PORT_E",
                       devPort = { devNum  = devNum,
                                   portNum = port1}}

    -- set port1 as MAC loopback
    callWithErrorHandling (
        "appDemoPxPortLoopbackModeEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                    "devNum",               devNum  },
        { "IN", "CPSS_INTERFACE_INFO_STC",  "portInterfacePtr",     interface  },
        { "IN", "GT_BOOL",                  "enable",               true }
    })

    --Enable forwarding to the target port
    callWithErrorHandling (
        "appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                   "devNum",    devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",   port1  },
        { "IN", "GT_BOOL",                 "enable",    true  }
    })

    --Enable forwarding to the target port
    callWithErrorHandling (
        "appDemoPxIngressPortTargetEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                   "devNum",    devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",   port3  },
        { "IN", "GT_BOOL",                 "enable",    true  }
    })

    --set continuous mode
    callWithErrorHandling(
        "appDemoPxIngressPortMapEntrySet_HardCodedRegAddr", {
        { "IN", "GT_SW_DEV_NUM",      "devNum",           devNum  },
        { "IN", "CPSS_PX_TABLE_ENT",  "tableType",        "CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E"   },
        { "IN", "GT_U32",             "entryIndex",       16 * port1 },
        { "IN", "CPSS_PX_PORTS_BMP",  "portsBmp",         0x0FFFF }
    })

    --set profile per src port to direct traffic to needed port
    callWithErrorHandling(
        "appDemoPxIngressPortMapEntrySet_HardCodedRegAddr", {
        { "IN", "GT_SW_DEV_NUM",      "devNum",           devNum  },
        { "IN", "CPSS_PX_TABLE_ENT",  "tableType",        "CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E"   },
        { "IN", "GT_U32",             "entryIndex",       16 * port1 },
        { "IN", "CPSS_PX_PORTS_BMP",  "portsBmp",         0x1FFFF }
    })

    ------------------------------ MAIN PART ---------------------------------------

    clearCounters()

    --send continuous traffic
    printLog("\nSend continuous traffic ")
    callWithErrorHandling(
        "appDemoPxSdmaTxPacketSend_HardCodedRegAddr", {
        { "IN",     "GT_U8",       "devNum",                devNum  },
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
               "appDemoPxEventRestore", {})
    delay(1000);

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
