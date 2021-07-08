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
local skip = true

-- --this test is temporary skipped
-- if skip == true then
    -- setTestStateSkipped();
    -- return;
-- end

--the test is only for devices with devNum=0 (the expected output is for devNum=0)
if SUPPORTED_FEATURE_DECLARE(devEnv.dev, "CPSS_LOG") == false then
    setTestStateSkipped();
    return;
end

local MG_Global_Interrupt_mask = 0x34
local sip_6_MG_base = 0x1d000000

if is_sip_6(devNum) then
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


local function cleanBeforeExit()
--close files
fs.close(oldFid)
fs.close(newFid)

--deconfig
printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/cpss_log_error_deconfig.txt")

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


--config
executeLocalConfig("dxCh/examples/configurations/cpss_log_error.txt")

--creating the tested file with unique file name
local clockval=os.clock()*1000
local filename="test_cpss_log_"..clockval..".txt"
ret = myGenWrapper("appDemoOsLogModeSet",{
	{"IN",TYPE["ENUM"],"mode",1},
	{"IN","string","name", filename}
})

--variable definitions 
local macEntryPtr={}

--creating parameters for cpssDxChBrgFdbPortGroupMacEntrySet function
macEntryPtr["key"]={}
macEntryPtr["key"]["entryType"]="CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E"
macEntryPtr["key"]["vid1"]=1
macEntryPtr["key"]["key"]={}
macEntryPtr["key"]["key"]["macVlan"]={}
macEntryPtr["key"]["key"]["macVlan"]["macAddr"]="11:22:33:44:55:66"
macEntryPtr["key"]["key"]["macVlan"]["vlanId"]=1
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

-- --calling cpssLogHistoryDump, to output the stack content into log
-- ret = myGenWrapper("cpssLogHistoryDump",{})

--stop writing the log into the file
ret = myGenWrapper("appDemoOsLogStop",{})

--open the expected file
oldFid=fs.open("dxCh/examples/configurations/cpssLogError.txt","r")

--open the log that was created in run time (when cpssDxChPclRuleSet ran)
newFid=fs.open(filename,"r")

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

--open the expected output files again for comparing between the tested output and the expected output line by line
oldFid=fs.open("dxCh/examples/configurations/cpssLogError.txt","r")
newFid=fs.open(filename,"r")
oldLine = fs.gets(oldFid)
newLine = fs.gets(newFid)

--comparing between the tested output and the expected output line by line
counter=1
while oldLine~=nil and newLine~=nil do
	if string.lower(oldLine)~=string.lower(newLine) then
		setFailState()
		printLog("line number:"..counter)
		printLog("expected output:"..oldLine)
		printLog("tested output:"..newLine)
		printLog("not match")
		cleanBeforeExit()
		return
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

