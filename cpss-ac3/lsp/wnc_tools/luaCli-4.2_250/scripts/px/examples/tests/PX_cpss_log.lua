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
local MG_Global_Control = 0x50
local MG_Global_Interrupt_mask = 0x34

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
executeLocalConfig("px/examples/configurations/PX_cpss_log.txt")

local oldLogFile = "px/examples/configurations/PX_cpssLog.txt"

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
    printLog("Restore configuration ..")

    executeLocalConfig("px/examples/configurations/PX_cpss_log_deconfig.txt")

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

--------------------------------------------------------------------------------
---------------------------- MAIN ----------------------------------------------
--------------------------------------------------------------------------------

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
--------------variable definitions for cpssPxCncEgressQueueClientModeGet lib CPSS_LOG_LIB_CNC_E-----------------
--calling cpssPxCncEgressQueueClientModeGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxCncEgressQueueClientModeGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
	{"OUT","CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT","modePtr"}
})
--------------end cpssPxCncEgressQueueClientModeGet lib CPSS_LOG_LIB_CONFIG_E--------------------------------------
--------------variable definitions for cpssPxCfgDevEnableGet lib CPSS_LOG_LIB_CONFIG_E-----------------
--calling cpssPxCfgDevEnableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxCfgDevEnableGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
	{"OUT","GT_BOOL","enablePtr"}
})
--------------end cpssPxCfgDevEnableGet lib CPSS_LOG_LIB_CONFIG_E--------------------------------------
--------------variable definitions for cpssPxCosPortAttributesGet lib CPSS_LOG_LIB_COS_E-----------------
--calling cpssPxCosPortAttributesGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxCosPortAttributesGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",0},
	{"OUT","CPSS_PX_COS_ATTRIBUTES_STC","cosAttributesPtr"}
})
--------------end cpssPxCosPortAttributesGet lib CPSS_LOG_LIB_COS_E--------------------------------------
--------------variable definitions for cpssPxHwPpSoftResetSkipParamGet lib CPSS_LOG_LIB_HW_INIT_E-----------------
--calling cpssPxHwPpSoftResetSkipParamGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxHwPpSoftResetSkipParamGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","CPSS_HW_PP_RESET_SKIP_TYPE_ENT","skipType","CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E"},
	{"OUT","GT_BOOL","skipEnablePtr"}
})
--------------end cpssPxHwPpSoftResetSkipParamGet lib CPSS_LOG_LIB_HW_INIT_E--------------------------------------
--------------variable definitions for cpssPxCutThroughUpEnableGet lib CPSS_LOG_LIB_CUT_THROUGH_E-----------------
--calling cpssPxCutThroughUpEnableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxCutThroughUpEnableGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","GT_U32","up",1},
	{"OUT","GT_BOOL","enablePtr"}
})
--------------end cpssPxCutThroughUpEnableGet lib CPSS_LOG_LIB_CUT_THROUGH_E--------------------------------------
--------------variable definitions for cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet lib CPSS_LOG_LIB_DIAG_E-----------------
--calling cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",0},
    {"IN","GT_U32","laneNum",1},
	{"OUT","GT_BOOL","enablePtr"}
})
--------------end cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet lib CPSS_LOG_LIB_DIAG_E--------------------------------------
--------------variable definitions for cpssPxIngressEtherTypeGet lib CPSS_LOG_LIB_INGRESS_E-----------------
--calling cpssPxIngressEtherTypeGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxIngressEtherTypeGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","CPSS_PX_INGRESS_ETHERTYPE_ENT","configType","CPSS_PX_INGRESS_ETHERTYPE_MPLS1_E"},
	{"OUT","GT_U16","etherTypePtr"},
    {"OUT","GT_BOOL","validPtr"}
})
--------------end cpssPxIngressEtherTypeGet lib CPSS_LOG_LIB_INGRESS_E--------------------------------------
--------------variable definitions for cpssPxEgressBypassModeGet lib CPSS_LOG_LIB_EGRESS_E-----------------
--calling cpssPxEgressBypassModeGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxEgressBypassModeGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"OUT","GT_BOOL","phaBypassEnablePtr"},
	{"OUT","GT_BOOL","ppaClockEnablePtr"}
})
--------------end cpssPxEgressBypassModeGet lib CPSS_LOG_LIB_EGRESS_E--------------------------------------
--------------variable definitions for cpssPxNetIfSdmaRxCountersGet lib CPSS_LOG_LIB_NETWORK_IF_E-----------------
--calling cpssPxNetIfSdmaRxCountersGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxNetIfSdmaRxCountersGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","GT_U32","rxQueue",7},
	{"OUT","CPSS_PX_NET_SDMA_RX_COUNTERS_STC","rxCountersPtr"}
})
--------------end cpssPxNetIfSdmaRxCountersGet lib CPSS_LOG_LIB_NETWORK_IF_E--------------------------------------
--------------variable definitions for cpssPxPhySmiMdcDivisionFactorGet lib CPSS_LOG_LIB_PHY_E-----------------
--calling cpssPxPhySmiMdcDivisionFactorGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxPhySmiMdcDivisionFactorGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
	{"OUT","CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT","divisionFactorPtr"}
})
--------------end cpssPxPhySmiMdcDivisionFactorGet lib CPSS_LOG_LIB_PHY_E--------------------------------------
--------------variable definitions for cpssPxPortPfcEnableGet lib CPSS_LOG_LIB_PORT_E-----------------
--calling cpssPxPortPfcEnableGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxPortPfcEnableGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
	{"OUT","CPSS_PX_PORT_PFC_ENABLE_ENT","pfcEnablePtr"}
})
--------------end cpssPxPortPfcEnableGet lib CPSS_LOG_LIB_PORT_E--------------------------------------
--------------variable definitions for cpssPxPtpTaiClockCycleGet lib CPSS_LOG_LIB_PTP_E-----------------
--calling cpssPxPtpTaiClockCycleGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxPtpTaiClockCycleGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"OUT","GT_U32","secondsPtr"},
	{"OUT","GT_U32","nanoSecondsPtr"}
})
--------------end cpssPxPtpTaiClockCycleGet lib CPSS_LOG_LIB_PTP_E--------------------------------------
--------------variable definitions for cpssPxVersionGet lib CPSS_LOG_LIB_VERSION_E-----------------
--calling cpssPxVersionGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxVersionGet",{
	{"OUT","GT_VERSION","versionPtr"}
})
--------------end cpssPxVersionGet lib CPSS_LOG_LIB_VERSION_E--------------------------------------
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

--stop writing the log into the file
ret = myGenWrapper("appDemoOsLogStop",{})

--Debug output of current log
newFid = fs.open(newLogFile, "r")
newLine = fs.gets(newFid)
print(newLine)
while newLine~=nil do
    print(newLine)
    newLine = fs.gets(newFid)
end
fs.close(newFid)

--open the relevant expected output file
oldFid = fs.open(oldLogFile, "r")

--open the log that was created in run time 
newFid=fs.open(newLogFile, "r")

--check if the files were opened successfully
if oldFid == nil then
    printLog("old file is nil")
end
if newFid == nil then
    printLog("new file is nil")
end

--start reading the tested log and the expected output
oldLine = fs.gets(oldFid)
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
    oldLine = fs.gets(oldFid)
    newLine = fs.gets(newFid)
    counter=counter+1
end

--check if there were lines left in one of the files
while oldLine~=nil do
    str = string.format("%-4s", counter) .. " " .. string.format("%-90s", oldLine) .. " " .. string.format("%-90s", "")
    printLog(str)
    oldLine = fs.gets(oldFid)
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

oldLine = fs.gets(oldFid)
newLine = fs.gets(newFid)

--comparing between the tested output and the expected output line by line
counter=1
while oldLine~=nil and newLine~=nil do
    if oldLine~=newLine then
        setFailState()
        printLog("line number:"..counter)
        printLog("expected output:"..oldLine)
        printLog("tested output:"..newLine)
        printLog("not match")
        cleanBeforeExit()
    end
    oldLine = fs.gets(oldFid)
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
