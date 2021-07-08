--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpss_log.lua
--*
--* DESCRIPTION:
--*       The test for testing cpss_log
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local devNum  = devEnv.dev
local devFamily, devSubFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum);
local enable_hwInit_and_port_logging=[[
end
debug
trace cpss-log lib hw-init log-type entry-level-function
trace cpss-log lib port log-type entry-level-function
end
examples
]]
local disable_hwInit_and_port_logging=[[
end
debug
no trace cpss-log lib hw-init log-type all
no trace cpss-log lib port log-type all
end
examples
]]
--the test is only for devices with devNum=0 (the expected output is for devNum=0)
if SUPPORTED_FEATURE_DECLARE(devEnv.dev, "CPSS_LOG")==false then
    setTestStateSkipped();
    return;
end

--temporarily we skip the test for lion2 and cheetah devices
if devFamily == "CPSS_PP_FAMILY_DXCH_LION2_E" or devFamily == "CPSS_PP_FAMILY_CHEETAH_E" then
    setTestStateSkipped();
    return;
end

if (devFamily == "CPSS_PP_FAMILY_DXCH_XCAT_E") or
   (devFamily == "CPSS_PP_FAMILY_DXCH_XCAT3_E") or
   (devFamily == "CPSS_PP_FAMILY_DXCH_XCAT2_E") then

    --calling cpssDxChPolicerPortGroupMeteringEntrySet to avoid random trash after init
    local meterEntry
    local ret, val

    meterEntry={}
    meterEntry["tokenBucketParams"]={
            srTcmParams = {
                cir = 0,
                cbs = 0,
                ebs = 0
            }
        }
    meterEntry["meterMode"]= "CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E"

    ret,val = myGenWrapper("cpssDxChPolicerPortGroupMeteringEntrySet", {
	    { "IN", "GT_U8", "devNum", devNum },
	    { "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", 1 },
	    { "IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", "CPSS_DXCH_POLICER_STAGE_INGRESS_0_E" },
	    { "IN", "GT_U32", "entryIndex", 5 },
	    { "IN", "CPSS_DXCH3_POLICER_METERING_ENTRY_STC", "entryPtr", meterEntry },
        { "OUT","CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT_srTcmParams","tbParamsPtr"}
    })

end

local MG_Global_Control = 0x50
local MG_Global_Interrupt_mask = 0x34
local sip_6_MG_base = 0x1d000000

if is_sip_6(devNum) then 
    -- do not let the LOG access invalid register address !
    MG_Global_Control = MG_Global_Control + sip_6_MG_base
    MG_Global_Interrupt_mask = MG_Global_Interrupt_mask + sip_6_MG_base
end

-- Disable all interrupts to avoid interference with test
local result, globalMask = myGenWrapper(
                    "cpssDrvPpHwRegisterRead", {
                        { "IN", "GT_U8",  "devNum", devNum},
                        { "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                        { "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
                        { "OUT","GT_U32", "regValue"}
                    }
                )

result = myGenWrapper(
                    "cpssDrvPpHwRegisterWrite", {
                        { "IN", "GT_U8",  "devNum", devNum},
                        { "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                        { "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
                        { "IN","GT_U32", "regValue", 0}
                    }
                )

--config
executeLocalConfig("dxCh/examples/configurations/cpss_log.txt")
executeStringCliCommands(disable_hwInit_and_port_logging)

local oldLogFile = "dxCh/examples/configurations/cpssLog.txt"

--creating the tested file with unique file name
local newLogFile = "test_cpss_log_" .. os.clock()*1000 .. ".txt"

ret = myGenWrapper("appDemoOsLogModeSet",{
    {"IN",TYPE["ENUM"],"mode",1},
    {"IN","string","name", newLogFile}
})

local function cleanBeforeExit()
    --close files
    fs.close(oldFid)
    fs.close(newFid)

    --running cpssDxChPclRuleInvalidate for disable the pcl rule we created before
    ret,val = myGenWrapper("cpssDxChPclRuleInvalidate",{
                               {"IN","GT_U8","devNum",devNum},
                               {"IN",TYPE["ENUM"],"ruleSize",1}, --ext rule
                               {"IN","GT_U32","ruleIndex",ruleIndex}   -- index of the rule
    })

    --deconfig
    printLog("Restore configuration ..")

    executeLocalConfig("dxCh/examples/configurations/cpss_log_deconfig.txt")

    -- enable interrupts back
    result = myGenWrapper(
                        "cpssDrvPpHwRegisterWrite", {
                            { "IN", "GT_U8",  "devNum", devNum},
                            { "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                            { "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
                            { "IN","GT_U32", "regValue", globalMask.regValue}
                        }
                    )

end

local function skipSomeDiffs(oldLine, newLine)
    -- ignore difference in bytes of version because it's changing:
    -- CPSS n.y.xx  OR CPSS n.y.year.x
    if (string.find(oldLine, "version%[5%]") ~= nil) and (string.find(newLine, "version%[5%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[7%]") ~= nil) and (string.find(newLine, "version%[7%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[9%]") ~= nil) and (string.find(newLine, "version%[9%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[10%]") ~= nil) and (string.find(newLine, "version%[10%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[11%]") ~= nil) and (string.find(newLine, "version%[11%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[12%]") ~= nil) and (string.find(newLine, "version%[12%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[13%]") ~= nil) and (string.find(newLine, "version%[13%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[14%]") ~= nil) and (string.find(newLine, "version%[14%]") ~= nil) then
        return 0
    end
    if (string.find(oldLine, "version%[15%]") ~= nil) and (string.find(newLine, "version%[15%]") ~= nil) then
        return 0
    end

    return 1
end


--------------------------------------------------------------------------------
---------------------------- MAIN ----------------------------------------------
--------------------------------------------------------------------------------

local cscdPortNum = devEnv.port[4]

-- values that differ in different devices. The will be substitutes before comparing.
local diffValues = {
    s_s1   = cscdPortNum,
    s_s2   = "CPSS_CSCD_PORT_NETWORK_E",
    s_rc3  = "GT_NOT_APPLICABLE_DEVICE",
    s_rc4  = "GT_NOT_APPLICABLE_DEVICE",
    s_rc5  = "GT_BAD_STATE",
    s_rc6  = "GT_NOT_APPLICABLE_DEVICE",
    s_rc7  = "GT_OK",
    s_rc8  = "GT_NOT_APPLICABLE_DEVICE",
    s_rc9  = "GT_NOT_APPLICABLE_DEVICE",
    s_rc10 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc11 = "GT_OK",
    s_s12  = "CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E",
    s_s13  = "0",
    s_rc14 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc15 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc16 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc17 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc18 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc19 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc20 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc21 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc22 = "GT_NOT_APPLICABLE_DEVICE",
    s_rc23 = "GT_FALSE",
    s_rc24 = "GT_NOT_APPLICABLE_DEVICE",
    s_s22  = "0",
    s_s23 = "CPSS_TM_ELIG_N_PRIO1_E",
    s_s24 = "0"
}

if devFamily == "CPSS_PP_FAMILY_DXCH_XCAT3_E" or
   devFamily == "CPSS_PP_FAMILY_CHEETAH_E" or
   devFamily == "CPSS_PP_FAMILY_DXCH_XCAT_E" then

    diffValues.s_rc4  = "GT_BAD_PARAM"

elseif devFamily == "CPSS_PP_FAMILY_DXCH_XCAT2_E" then

    diffValues.s_rc5  = "GT_NOT_APPLICABLE_DEVICE"

elseif devFamily == "CPSS_PP_FAMILY_DXCH_LION_E" or devFamily == "CPSS_PP_FAMILY_DXCH_LION2_E" then

    diffValues.s_rc3  = "GT_OK"
    diffValues.s_rc4  = "GT_BAD_PARAM"
    diffValues.s_rc9  = "GT_OK"
    diffValues.s_rc14 = "GT_OK"

else

    diffValues.s_rc7  = "GT_NOT_APPLICABLE_DEVICE"
    diffValues.s_rc6  = "GT_OK"
    diffValues.s_rc8  = "GT_OK"
    diffValues.s_rc10 = "GT_FAIL"
    diffValues.s_rc11 = "GT_BAD_PARAM"
    diffValues.s_s12  = "CPSS_POLICER_PACKET_SIZE_L3_ONLY_E"
    diffValues.s_s13  = "536870911"
    diffValues.s_rc14 = "GT_OK"
    diffValues.s_rc15 = "GT_OK"
    diffValues.s_rc16 = "GT_OK"
    diffValues.s_rc17 = "GT_NOT_SUPPORTED"
    diffValues.s_rc18 = "GT_OK"
    diffValues.s_rc19 = "GT_OK"
    diffValues.s_rc20 = "GT_OK"
    diffValues.s_rc21 = "GT_OK"
    diffValues.s_rc22 = "GT_NOT_INITIALIZED"

    if (devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
        (devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E")
    then
        diffValues.s_rc18 = "GT_NOT_APPLICABLE_DEVICE"
        diffValues.s_rc22 = "GT_NOT_APPLICABLE_DEVICE"
        diffValues.s_rc23 = "GT_TRUE"
        diffValues.s_rc24 = "GT_BAD_PARAM"

    elseif devFamily    == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E" and
           devSubFamily == "CPSS_PP_SUB_FAMILY_NONE_E" then
        diffValues.s_rc23 = "GT_TRUE"
        diffValues.s_rc24 = "GT_BAD_PARAM"

    elseif devFamily    == "CPSS_PP_FAMILY_DXCH_BOBCAT3_E" then
       -- data integrity supported
       diffValues.s_rc23 = "GT_TRUE"
       diffValues.s_rc24 = "GT_BAD_PARAM"
       -- TCAM Clients cannot be configured
       diffValues.s_rc17 = "GT_NOT_APPLICABLE_DEVICE"
       -- TM and Glue are not applicable
       diffValues.s_rc18 = "GT_NOT_APPLICABLE_DEVICE"
       diffValues.s_rc22 = "GT_NOT_APPLICABLE_DEVICE"
       -- TS option is wrong
       diffValues.s_rc20 = "GT_BAD_PARAM"
       -- PFC threshould
       diffValues.s_s13  = "524287"

    elseif devFamily    == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
       -- data integrity not supported yet
       --diffValues.s_rc23 = "GT_TRUE"
       --diffValues.s_rc24 = "GT_BAD_PARAM"
       -- TM and Glue are not applicable
       diffValues.s_rc18 = "GT_NOT_APPLICABLE_DEVICE"
       diffValues.s_rc22 = "GT_NOT_APPLICABLE_DEVICE"
       -- PFC threshould
       diffValues.s_s13  = "524287"

    end
end

if (isTmEnabled(devNum)) then
    diffValues.s_rc22 = "GT_OK"
    diffValues.s_s22 = "64"
    diffValues.s_s23 = "CPSS_TM_ELIG_N_FP0_E"
    diffValues.s_s24 = "8"
end

oldFid = fs.open(oldLogFile, "r")

--------------variable definitions for cpssDrvPpHwRegisterRead lib CPSS_LOG_LIB_APP_DRIVER_CALL_E-----------------
--calling cpssDrvPpHwRegisterRead, this function processing will be documented by the log
ret = myGenWrapper("cpssDrvPpHwRegisterRead",{
	{"IN","GT_U8","devNum",devNum},
	{"IN","GT_U32","portGroupId",1},
	{"IN","GT_U32","regAddr",MG_Global_Control},
	{"OUT","GT_U32","data"}
})
--------------end cpssDrvPpHwRegisterRead lib CPSS_LOG_LIB_APP_DRIVER_CALL_E--------------------------------------

--------------variable definitions for cpssDxChBrgFdbPortGroupMacEntrySet lib CPSS_LOG_LIB_BRIDGE_E-----------------
local macEntryPtr={}
macEntryPtr["key"]={}
macEntryPtr["key"]["entryType"]="CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E"
macEntryPtr["key"]["vid1"]=1
macEntryPtr["key"]["macVlan"]={}
macEntryPtr["key"]["macVlan"]["macAddr"]="11:22:33:44:55:66"
macEntryPtr["key"]["macVlan"]["vlanId"]=1
macEntryPtr["dstInterface"]={}
macEntryPtr["dstInterface"]["type"]="CPSS_INTERFACE_PORT_E"
macEntryPtr["dstInterface"]["devPort"]={}
macEntryPtr["dstInterface"]["devPort"]["hwDevNum"]=0
macEntryPtr["dstInterface"]["devPort"]["portNum"]=0
macEntryPtr["dstInterface"]["trunkId"]=1
macEntryPtr["dstInterface"]["vidx"]=2
macEntryPtr["dstInterface"]["vlanId"]=1
macEntryPtr["dstInterface"]["hwDevNum"]=0
macEntryPtr["dstInterface"]["fabricVidx"]=1
macEntryPtr["dstInterface"]["index"]=2
macEntryPtr["age"]=true
macEntryPtr["isStatic"]=true
macEntryPtr["daCommand"]="CPSS_MAC_TABLE_FRWRD_E"
macEntryPtr["saCommand"]="CPSS_MAC_TABLE_FRWRD_E"
macEntryPtr["daRoute"]=false
macEntryPtr["mirrorToRxAnalyzerPortEn"]=false
macEntryPtr["sourceID"]=3
macEntryPtr["userDefined"]=1
macEntryPtr["daQosIndex"]=4
macEntryPtr["saQosIndex"]=3
macEntryPtr["daSecurityLevel"]=6
macEntryPtr["saSecurityLevel"]=7
macEntryPtr["appSpecificCpuCode"]=true
macEntryPtr["spUnknown"]=false
macEntryPtr["saMirrorToRxAnalyzerPortEn"]=true
macEntryPtr["daMirrorToRxAnalyzerPortEn"]=false

--calling cpssDxChBrgFdbMacEntrySet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChBrgFdbMacEntrySet",{
	{"IN","GT_U8","devNum",devNum},
	{"IN","CPSS_MAC_ENTRY_EXT_STC","macEntryPtr",macEntryPtr}
})
--------------end cpssDxChBrgFdbPortGroupMacEntrySet lib CPSS_LOG_LIB_BRIDGE_E--------------------------------------

--------------variable definitions for cpssDxChCncClientByteCountModeGet lib CPSS_LOG_LIB_CNC_E-----------------
--calling cpssDxChCncClientByteCountModeGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChCncClientByteCountModeGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "CPSS_DXCH_CNC_CLIENT_ENT", "client", "CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E" },
	{ "OUT", "CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT", "countModePtr" }
})
--------------end cpssDxChCncClientByteCountModeGet lib CPSS_LOG_LIB_CNC_E--------------------------------------

--------------variable definitions for cpssDxChCfgIngressDropCntrModeGet lib CPSS_LOG_LIB_CONFIG_E-----------------
--calling cpssDxChCfgIngressDropCntrModeGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChCfgIngressDropCntrModeGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "OUT", "CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT", "modePtr" },
	{ "OUT", "GT_PORT_NUM", "portNumPtr" },
	{ "OUT", "GT_U16", "vlanPtr" }
})

--------------end cpssDxChCfgIngressDropCntrModeGet lib CPSS_LOG_LIB_HW_INIT_E--------------------------------------

--------------variable definitions for cpssDxChCosProfileEntryGet lib CPSS_LOG_LIB_COS_E-----------------
--calling cpssDxChCosProfileEntryGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChCosProfileEntryGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "profileIndex", 5 },
	{ "OUT", "CPSS_DXCH_COS_PROFILE_STC", "cosPtr" }
})
--------------end cpssDxChCosProfileEntryGet lib CPSS_LOG_LIB_COS_E--------------------------------------

--------------variable definitions for cpssDxChHwInterruptCoalescingGet lib CPSS_LOG_LIB_HW_INIT_E-----------------
--calling cpssDxChHwInterruptCoalescingGet, this function processing will be documented by the log
executeStringCliCommands(enable_hwInit_and_port_logging)
ret = myGenWrapper("cpssDxChHwInterruptCoalescingGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "OUT", "GT_BOOL", "enablePtr" },
	{ "OUT", "GT_U32", "periodPtr" },
	{ "OUT", "GT_BOOL", "linkChangeOverridePtr" }
})
executeStringCliCommands(disable_hwInit_and_port_logging)

--------------end cpssDxChHwInterruptCoalescingGet lib CPSS_LOG_LIB_HW_INIT_E--------------------------------------

--------------end cpssDxChCosProfileEntryGet lib CPSS_LOG_LIB_COS_E--------------------------------------

--------------variable definitions for cpssDxChCscdPortTypeGet lib CPSS_LOG_LIB_CSCD_E-----------------
--calling cpssDxChCscdPortTypeGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChCscdPortTypeGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PHYSICAL_PORT_NUM", "portNum", cscdPortNum},
	{ "IN", "CPSS_PORT_DIRECTION_ENT", "portDirection", 0 },
	{ "OUT", "CPSS_CSCD_PORT_TYPE_ENT", "portTypePtr" }
})

--------------end cpssDxChCscdPortTypeGet lib CPSS_LOG_LIB_CSCD_E--------------------------------------

--------------variable definitions for cpssDxChCutThroughMemoryRateLimitGet lib CPSS_LOG_LIB_CUT_THROUGH_E-----------------
--calling cpssDxChCutThroughMemoryRateLimitGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChCutThroughMemoryRateLimitGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PHYSICAL_PORT_NUM", "portNum", 0 },
	{ "OUT", "GT_BOOL", "enablePtr" },
	{ "OUT", "CPSS_PORT_SPEED_ENT", "portSpeedPtr" }
})

--------------end cpssDxChCutThroughMemoryRateLimitGet lib CPSS_LOG_LIB_CUT_THROUGH_E--------------------------------------

--------------variable definitions for cpssDxChDiagDataIntegrityEventsGet lib CPSS_LOG_LIB_DIAG_E-----------------
--calling cpssDxChDiagDataIntegrityEventsGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChDiagDataIntegrityEventsGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "evExtData", 3 },
	{ "INOUT", "GT_U32", "eventsNumPtr", 1 },
	{ "OUT", "CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC", "eventsArr" },
	{ "OUT", "GT_BOOL", "isNoMoreEventsPtr" }
})

--------------end cpssDxChDiagDataIntegrityEventsGet lib CPSS_LOG_LIB_DIAG_E--------------------------------------


--------------variable definitions for cpssDxChFabricHGLinkRxLinkLayerFcParamsGet lib CPSS_LOG_LIB_FABRIC_E-----------------
--calling cpssDxChFabricHGLinkRxLinkLayerFcParamsGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChFabricHGLinkRxLinkLayerFcParamsGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PHYSICAL_PORT_NUM", "portNum", 2 },
	{ "OUT", "CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC", "llParamsPtr" }
})

--------------end cpssDxChFabricHGLinkRxLinkLayerFcParamsGet lib CPSS_LOG_LIB_FABRIC_E--------------------------------------

--------------variable definitions for cpssDxChIpv4PrefixGet lib CPSS_LOG_LIB_IP_E-----------------
--calling cpssDxChIpv4PrefixGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChIpv4PrefixGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "routerTtiTcamRow", 5 },
	{ "IN", "GT_U32", "routerTtiTcamColumn", 5 },
	{ "OUT", "GT_BOOL", "validPtr" },
	{ "OUT", "CPSS_DXCH_IPV4_PREFIX_STC", "prefixPtr" },
	{ "OUT", "CPSS_DXCH_IPV4_PREFIX_STC", "maskPtr" }
})

--------------end cpssDxChIpv4PrefixGet lib CPSS_LOG_LIB_IP_E--------------------------------------

--------------variable definitions for cpssDxChIpfixEntryGet lib CPSS_LOG_LIB_IPFIX_E-----------------
--calling cpssDxChIpfixEntryGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChIpfixEntryGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", CPSS_DXCH_POLICER_STAGE_INGRESS_1_E },
	{ "IN", "GT_U32", "entryIndex", 3 },
	{ "IN", "GT_BOOL", "reset", true },
	{ "OUT", "CPSS_DXCH_IPFIX_ENTRY_STC", "ipfixEntryPtr" }
})

--------------end cpssDxChIpfixEntryGet lib CPSS_LOG_LIB_IPFIX_E--------------------------------------

--------------variable definitions for cpssDxChIpLpmDBCapacityGet lib CPSS_LOG_LIB_IP_LPM_E-----------------
--calling cpssDxChIpLpmDBCapacityGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChIpLpmDBCapacityGet", {
	{ "IN", "GT_U32", "lpmDBId", 5 },
	{ "OUT", "GT_BOOL", "partitionEnablePtr" },
	{ "OUT", "CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC", "indexesRangePtr" },
	{ "OUT", "CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC", "tcamLpmManagerCapcityCfgPtr" }
})

--------------end cpssDxChIpLpmDBCapacityGet lib CPSS_LOG_LIB_IP_LPM_E--------------------------------------

--------------variable definitions for cpssDxChL2MllLttEntryGet lib CPSS_LOG_LIB_L2_MLL_E-----------------
--calling cpssDxChL2MllLttEntryGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChL2MllLttEntryGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "index", 1 },
	{ "OUT", "CPSS_DXCH_L2_MLL_LTT_ENTRY_STC", "lttEntryPtr" }
})

--------------end cpssDxChL2MllLttEntryGet lib CPSS_LOG_LIB_L2_MLL_E--------------------------------------
logicalPortMappingTable = {}
logicalPortMappingTable["outputInterface"] = {}
logicalPortMappingTable["outputInterface"]["isTunnelStart"] = false
logicalPortMappingTable["outputInterface"]["tunnelStartInfo"] = {}
logicalPortMappingTable["outputInterface"]["tunnelStartInfo"]["passengerPacketType"] = 0x5AAAAAA5
logicalPortMappingTable["outputInterface"]["tunnelStartInfo"]["ptr"] = 1
--------------variable definitions for cpssDxChLogicalTargetMappingTableEntrySet lib CPSS_LOG_LIB_LOGICAL_TARGET_E-----------------
--calling cpssDxChLogicalTargetMappingTableEntrySet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChLogicalTargetMappingTableEntrySet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U8", "logicalDevNum", 24 },
	{ "IN", "GT_U8", "logicalPortNum", 0 },
	{ "IN", "CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC", "logicalPortMappingTablePtr", logicalPortMappingTable}
})

--------------end cpssDxChLogicalTargetMappingTableEntrySet lib CPSS_LOG_LIB_LOGICAL_TARGET_E--------------------------------------

--------------variable definitions for cpssDxChLpmExceptionStatusGet lib CPSS_LOG_LIB_LPM_E-----------------
--calling cpssDxChLpmExceptionStatusGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChLpmExceptionStatusGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "lpmEngine", 0 },
	{ "OUT", "CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT", "hitExceptionPtr" }
})

--------------end cpssDxChLpmExceptionStatusGet lib CPSS_LOG_LIB_LPM_E--------------------------------------

--------------variable definitions for cpssDxChMirrorAnalyzerInterfaceGet lib CPSS_LOG_LIB_MIRROR_E-----------------
--calling cpssDxChMirrorAnalyzerInterfaceGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChMirrorAnalyzerInterfaceGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "index", 3 },
	{ "OUT", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC", "interfacePtr" }
})

--------------end cpssDxChMirrorAnalyzerInterfaceGet lib CPSS_LOG_LIB_MIRROR_E--------------------------------------

--------------variable definitions for cpssDxChMultiPortGroupLookupNotFoundTableGet lib CPSS_LOG_LIB_MULTI_PORT_GROUP_E-----------------
--calling cpssDxChMultiPortGroupLookupNotFoundTableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChMultiPortGroupLookupNotFoundTableGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", 5 },
	{ "IN", "GT_U32", "sourcePortGroupId", 1 },
	{ "OUT", "CPSS_DXCH_MULTI_PORT_GROUP_LOOKUP_NOT_FOUND_STC", "infoPtr" }
})

--------------end cpssDxChMultiPortGroupLookupNotFoundTableGet lib CPSS_LOG_LIB_MULTI_PORT_GROUP_E--------------------------------------

--------------variable definitions for cpssDxChNetIfCpuCodeTableGet lib CPSS_LOG_LIB_NETWORK_IF_E-----------------
--calling cpssDxChNetIfCpuCodeTableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChNetIfCpuCodeTableGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "CPSS_NET_RX_CPU_CODE_ENT", "cpuCode", "CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E" },
	{ "OUT", "CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC", "entryInfoPtr" }
})

--------------end cpssDxChNetIfCpuCodeTableGet lib CPSS_LOG_LIB_NETWORK_IF_E--------------------------------------

--------------variable definitions for cpssDxChNstPortIngressFrwFilterGet lib CPSS_LOG_LIB_NST_E-----------------
--calling cpssDxChNstPortIngressFrwFilterGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChNstPortIngressFrwFilterGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PORT_NUM", "portNum", 5 },
	{ "IN", "CPSS_NST_INGRESS_FRW_FILTER_ENT", "filterType", "CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E" },
	{ "OUT", "GT_BOOL", "enablePtr" }
})

--------------end cpssDxChNstPortIngressFrwFilterGet lib CPSS_LOG_LIB_NST_E--------------------------------------

--------------variable definitions for cpssDxChOamSrvLoopbackStatisticalCountersGet lib CPSS_LOG_LIB_OAM_E-----------------
--calling cpssDxChOamSrvLoopbackStatisticalCountersGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChOamSrvLoopbackStatisticalCountersGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "flowId", 4 },
	{ "IN", "GT_BOOL", "clearOnRead", true },
	{ "IN", "GT_U32", "timeout", 2 },
	{ "OUT", "CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC", "lbCountersPtr" }
})

--------------end cpssDxChOamSrvLoopbackStatisticalCountersGet lib CPSS_LOG_LIB_OAM_E--------------------------------------

--------------variable definitions for cpssDxChPclRuleSet lib CPSS_LOG_LIB_PCL_E-----------------
local CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E = 4
local CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E = 1
local CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E = 1
local CPSS_INTERFACE_PORT_E = 0
local CPSS_INTERFACE_TRUNK_E = 1
local CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E = 1
local CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E = 1
local actionSTC={mirror={cpuCode="CPSS_NET_ETH_BRIDGED_LLT_E"}}
local rule={}
local ruleMsk={}
local tempMsk={}
local ruleIndex = 5
local oldLine
local newLine

--creating parameters for cpssDxChPclRuleSet function
rule["common"]={}
ruleMsk["common"]={}
rule["commonExt"]={}
ruleMsk["commonExt"]={}
rule["common"]["pclId"]=973
rule["common"]["vid"]=4
rule["common"]["qosProfile"]=5
rule["common"]["sourcePort"]=2
rule["commonExt"]["isL4Valid"]=1
rule["commonExt"]["l4Byte13"]=1
rule["commonExt"]["l4Byte1"]=1
rule["vid1"]=12
rule["vrfId"]=45
rule["up1"]=77
rule["cfi1"]=99
rule["macDa"] = "11:22:33:44:55:66"
rule["macSa"] = "66:55:44:33:22:11"
ruleMsk["common"]["pclId"]=0x3ff
ruleMsk["common"]["vid"]=8
ruleMsk["common"]["qosProfile"]=3
ruleMsk["common"]["sourcePort"]=2
ruleMsk["commonExt"]["ipProtocol"]=1
ruleMsk["commonExt"]["ipHeaderOk"]=1
ruleMsk["commonExt"]["l4Byte1"]=1
ruleMsk["vid1"]=21
ruleMsk["vrfId"]=9
ruleMsk["up1"]=11
ruleMsk["cfi1"]=43
ruleMsk["macDa"] = "66:55:44:44:55:66"
ruleMsk["macSa"] = "11:22:33:33:22:11"
actionSTC["pktCmd"]=3   --hard drop
actionSTC.mirror.cpuCode = 500;
rule["common"]["isL2Valid"]=1
ruleMsk["common"]["isL2Valid"]=0x1

local patternAndMaskType, ruleFormat
patternAndMaskType = "CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC"
ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E

--calling cpssDxChPclRuleSet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChPclRuleSet",{
    {"IN","GT_U8","devNum",devNum},
    {"IN",TYPE["ENUM"],"ruleFormat",ruleFormat},  --Ext UDB
    {"IN","GT_U32","ruleIndex",ruleIndex},   -- index of the rule
    {"IN",TYPE["ENUM"],"ruleOptionsBmp",0}, --Rule is valid
    {"IN",patternAndMaskType,"maskPtr",ruleMsk},  -- The ext UDB rules
    {"IN",patternAndMaskType,"patternPtr",rule},  --The ext UDB mask
    {"IN","CPSS_DXCH_PCL_ACTION_STC","actionPtr",actionSTC} -- The action to be taken (permit/deny)
})

--------------end cpssDxChPclRuleSet lib CPSS_LOG_LIB_PCL_E-----------------

--------------variable definitions for cpssDxChPhyPortSmiInterfaceSet lib CPSS_LOG_LIB_PHY_E-----------------
--calling cpssDxChPhyPortSmiInterfaceSet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChPhyPortSmiInterfaceSet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PHYSICAL_PORT_NUM", "portNum", 0 },
	{ "IN", "CPSS_PHY_SMI_INTERFACE_ENT", "smiInterface", 6 }
})

--------------end cpssDxChPhyPortSmiInterfaceSet lib CPSS_LOG_LIB_PHY_E--------------------------------------

--------------variable definitions for cpssDxChPolicerPortGroupMeteringEntryGet lib CPSS_LOG_LIB_POLICER_E-----------------
--calling cpssDxChPolicerPortGroupMeteringEntryGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChPolicerPortGroupMeteringEntryGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", 1 },
	{ "IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", "CPSS_DXCH_POLICER_STAGE_INGRESS_0_E" },
	{ "IN", "GT_U32", "entryIndex", 5 },
	{ "OUT", "CPSS_DXCH3_POLICER_METERING_ENTRY_STC", "entryPtr" }
})

--------------end cpssDxChPolicerPortGroupMeteringEntryGet lib CPSS_LOG_LIB_POLICER_E--------------------------------------

--------------variable definitions for cpssDxChPortPfcProfileQueueConfigGet lib CPSS_LOG_LIB_PORT_E-----------------
--calling cpssDxChPortPfcProfileQueueConfigGet, this function processing will be documented by the log
executeStringCliCommands(enable_hwInit_and_port_logging)
ret = myGenWrapper("cpssDxChPortPfcProfileQueueConfigGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "profileIndex", 2 },
	{ "IN", "GT_U8", "tcQueue", 6 },
	{ "OUT", "CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC", "pfcProfileCfgPtr" }
})
executeStringCliCommands(disable_hwInit_and_port_logging)
--------------end cpssDxChPortPfcProfileQueueConfigGet lib CPSS_LOG_LIB_PORT_E--------------------------------------

--------------variable definitions for cpssDxChProtectionLocStatusGet lib CPSS_LOG_LIB_PROTECTION_E-----------------
--calling cpssDxChProtectionLocStatusGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChProtectionLocStatusGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "index", 13 },
	{ "OUT", "CPSS_DXCH_PROTECTION_LOC_STATUS_ENT", "statusPtr" }
})

--------------end cpssDxChProtectionLocStatusGet lib CPSS_LOG_LIB_PROTECTION_E--------------------------------------

--------------variable definitions for cpssDxChPtpTsLocalActionTableGet lib CPSS_LOG_LIB_PTP_E-----------------
--calling cpssDxChPtpTsLocalActionTableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChPtpTsLocalActionTableGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PHYSICAL_PORT_NUM", "portNum", 2 },
	{ "IN", "GT_U32", "domainProfile", 2 },
	{ "IN", "GT_U32", "messageType", 7 },
	{ "OUT", "CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC", "entryPtr" }
})

--------------end cpssDxChPtpTsLocalActionTableGet lib CPSS_LOG_LIB_PTP_E--------------------------------------

--------------variable definitions for cpssSystemRecoveryStateSet lib CPSS_LOG_LIB_SYSTEM_RECOVERY_E-----------------
local systemRecovery = {}
systemRecovery["systemRecoveryState"] = 4
systemRecovery["systemRecoveryMode"] = 4
systemRecovery["systemRecoveryProcess"] = 4
--calling cpssSystemRecoveryStateSet, this function processing will be documented by the log
ret = myGenWrapper("cpssSystemRecoveryStateSet", {
	{ "IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "systemRecoveryInfoPtr", systemRecovery }
})

--------------end cpssSystemRecoveryStateSet lib CPSS_LOG_LIB_SYSTEM_RECOVERY_E--------------------------------------

--------------variable definitions for cpssDxChTcamPortGroupClientGroupSet lib CPSS_LOG_LIB_TCAM_E-----------------
--calling cpssDxChTcamPortGroupClientGroupSet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChTcamPortGroupClientGroupSet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", 1 },
	{ "IN", "CPSS_DXCH_TCAM_CLIENT_ENT", "tcamClient", 2 },
	{ "IN", "GT_U32", "tcamGroup", 20 },
	{ "IN", "GT_BOOL", "enable", true }
})

--------------end cpssDxChTcamPortGroupClientGroupSet lib CPSS_LOG_LIB_TCAM_E--------------------------------------

--------------variable definitions for cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet lib CPSS_LOG_LIB_TM_GLUE_E-----------------
--calling cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PHYSICAL_PORT_NUM", "physicalPort", 2 },
	{ "OUT", "CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC", "lengthOffsetPtr" }
})

--------------end cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet lib CPSS_LOG_LIB_TM_GLUE_E--------------------------------------

--------------variable definitions for cpssDxChTrunkPortTrunkIdGet lib CPSS_LOG_LIB_TRUNK_E-----------------
--calling cpssDxChTrunkPortTrunkIdGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChTrunkPortTrunkIdGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PHYSICAL_PORT_NUM", "portNum", 2 },
	{ "OUT", "GT_BOOL", "memberOfTrunkPtr" },
	{ "OUT", "GT_TRUNK_ID", "trunkIdPtr" }
})

--------------end cpssDxChTrunkPortTrunkIdGet lib CPSS_LOG_LIB_TRUNK_E--------------------------------------


--------------variable definitions for cpssDxChTunnelStartPortGroupGenProfileTableEntryGet lib CPSS_LOG_LIB_TUNNEL_E-----------------
--calling cpssDxChTunnelStartPortGroupGenProfileTableEntryGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChTunnelStartPortGroupGenProfileTableEntryGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", 6 },
	{ "IN", "GT_U32", "profileIndex", 3 },
	{ "OUT", "CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC", "maskPtr" }
})

--------------end cpssDxChTunnelStartPortGroupGenProfileTableEntryGet lib CPSS_LOG_LIB_TUNNEL_E--------------------------------------

--------------variable definitions for cpssDxChVntCfmReplyModeGet lib CPSS_LOG_LIB_VNT_E-----------------
--calling cpssDxChVntCfmReplyModeGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChVntCfmReplyModeGet", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "OUT", "CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT", "modePtr" }
})

--------------end cpssDxChVntCfmReplyModeGet lib CPSS_LOG_LIB_VNT_E--------------------------------------

--------------variable definitions for cpssDxChTcamManagerEntriesReservationGet lib CPSS_LOG_LIB_RESOURCE_MANAGER_E-----------------
--calling cpssDxChTcamManagerEntriesReservationGet, this function processing will be documented by the log
-- ret = myGenWrapper("cpssDxChTcamManagerEntriesReservationGet", {
	-- { "IN", "GT_VOID", "tcamManagerHandlerPtr", nil	 },
	-- { "IN", "GT_U32", "clientId", 2 },
	-- { "OUT", "CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT", "reservationTypePtr" },
	-- { "OUT", "CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC", "reservedEntriesArray" },
	-- { "INOUT", "GT_U32", "sizeOfArrayPtr" , 5},
	-- { "OUT", "CPSS_DXCH_TCAM_MANAGER_RANGE_STC", "reservationRangePtr" }
-- })

--------------end cpssDxChTcamManagerEntriesReservationGet lib CPSS_LOG_LIB_RESOURCE_MANAGER_E--------------------------------------

--------------variable definitions for cpssDxChVersionGet lib CPSS_LOG_LIB_VERSION_E-----------------
--calling cpssDxChVersionGet, this function processing will be documented by the log
ret = myGenWrapper("cpssDxChVersionGet", {
	{ "OUT", "CPSS_VERSION_INFO_STC", "versionPtr" }
})

--------------end cpssDxChVersionGet lib CPSS_LOG_LIB_VERSION_E--------------------------------------

--------------variable definitions for cpssTmAnodeConfigurationRead lib CPSS_LOG_LIB_TM_E-----------------
--calling cpssTmAnodeConfigurationRead, this function processing will be documented by the log
ret = myGenWrapper("cpssTmAnodeConfigurationRead", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_U32", "index", 8 },
	{ "OUT", "CPSS_TM_A_NODE_PARAMS_STC", "paramsPtr" }
})

--------------end cpssTmAnodeConfigurationRead lib CPSS_LOG_LIB_TM_E--------------------------------------

--------------variable definitions for cpssXsmiPortGroupRegisterWrite lib CPSS_LOG_LIB_SMI_E-----------------
--calling cpssXsmiPortGroupRegisterWrite, this function processing will be documented by the log
ret = myGenWrapper("cpssXsmiPortGroupRegisterWrite", {
	{ "IN", "GT_U8", "devNum", devNum },
	{ "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", devNum },
	{ "IN", "CPSS_PHY_XSMI_INTERFACE_ENT", "xsmiInterface", 6 },
	{ "IN", "GT_U32", "xsmiAddr", 0x25 },
	{ "IN", "GT_U32", "regAddr", 0x2c },
	{ "IN", "GT_U32", "phyDev", 0x25 },
	{ "IN", "GT_U16", "data", 3 }
})

--------------end cpssXsmiPortGroupRegisterWrite lib CPSS_LOG_LIB_SMI_E--------------------------------------

--------------variable definitions for cpssSystemDualDeviceIdModeEnableGet lib CPSS_LOG_LIB_INIT_E-----------------
--calling cpssSystemDualDeviceIdModeEnableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssSystemDualDeviceIdModeEnableGet", {
	{ "OUT", "GT_BOOL", "enablePtr" }
})

--------------end cpssSystemDualDeviceIdModeEnableGet lib CPSS_LOG_LIB_INIT_E--------------------------------------

--------------variable definitions for cpssDragoniteReadData lib CPSS_LOG_LIB_DRAGONITE_E-----------------
--calling cpssDragoniteReadData, this function processing will be documented by the log
-- ret = myGenWrapper("cpssDragoniteReadData", {
	-- { "OUT", "CPSS_GEN_DRAGONITE_DATA_STC", "dataPtr" }
-- })

--------------end cpssDragoniteReadData lib CPSS_LOG_LIB_DRAGONITE_E--------------------------------------


--stop writing the log into the file
ret = myGenWrapper("appDemoOsLogStop",{})

--open the relevant expected output file
oldFid = fs.open(oldLogFile, "r")

--open the log that was created in run time (when cpssDxChPclRuleSet ran)
newFid=fs.open(newLogFile, "r")

--check if the files were opened successfully
if oldFid == nil then
    printLog("old file is nil")
end
if newFid == nil then
    printLog("new file is nil")
end

--start reading the tested log and the expected output
oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
newLine = fs.gets(newFid)

--check if the files are not empty
if (newLine==nil and oldLine~=nil) or (newLine~=nil and oldLine==nil) then
    setFailState()
    printLog("one file is empty")
    cleanBeforeExit()
    return
end

--printing both files on the screen in this format: line number, this line in the expected output, this line in the tested output
counter=1
str = string.format("%-4s", "line") .. " " .. string.format("%-90s", "expected output").. " " .. string.format("%-90s", "tested output")
printLog(str)
while oldLine~=nil and newLine~=nil do
    str = string.format("%-4s", counter) .. " " .. string.format("%-90s", oldLine) .. " " .. string.format("%-90s", newLine)
    printLog(str)
    oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
    newLine = fs.gets(newFid)
    counter=counter+1
end

--check if there were lines left in one of the files
while oldLine~=nil do
    str = string.format("%-4s", counter) .. " " .. string.format("%-90s", oldLine) .. " " .. string.format("%-90s", "")
    printLog(str)
    oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
    counter=counter+1
end

while newLine~=nil do
    str = string.format("%-4s", counter) .. " " .. string.format("%-90s", "") .. " " .. string.format("%-90s", newLine)
    printLog(str)
    newLine = fs.gets(newFid)
        counter=counter+1
end

--close the files
fs.close(oldFid)
fs.close(newFid)

--open the expected output file again for comparing between the tested output and the expected output line by line
oldFid = fs.open(oldLogFile, "r")
newFid = fs.open(newLogFile,"r")

oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
newLine = fs.gets(newFid)

--comparing between the tested output and the expected output line by line
counter=1
while oldLine~=nil and newLine~=nil do
    if oldLine~=newLine then
        -- ignore difference in bytes of version because it's changing:
        -- CPSS n.y.xx
        if skipSomeDiffs(oldLine, newLine) == 1 then
            setFailState()
            printLog("line number:"..counter)
            printLog("expected output:"..oldLine)
            printLog("tested output:"..newLine)
            printLog("not match")
            cleanBeforeExit()
            return
        end
    end
    oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
    newLine = fs.gets(newFid)
    counter=counter+1
end

--if there are line left in one of the files, the files are not equall
if oldLine~=nil or newLine~=nil then
    setFailState()
    printLog("not in the same length")
    cleanBeforeExit()
    return
end

cleanBeforeExit()
