--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpss_log_history.lua
--*
--* DESCRIPTION:
--*       The test for testing cpss_log, log history feature
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


local diffValues = {
	s_s1   = "skip_line",
	s_s2   = "skip_line"
}

local  skip_line_string="skip_line"

diffValues.s_s1  = skip_line_string
diffValues.s_s2  = skip_line_string

--config
executeLocalConfig("dxCh/examples/configurations/cpss_log_history.txt")

--creating the tested file with unique file name
local clockval=os.clock()*1000
local filename="test_cpss_log_"..clockval..".txt"
ret = myGenWrapper("appDemoOsLogModeSet",{
	{"IN",TYPE["ENUM"],"mode",1},
	{"IN","string","name", filename}
})

local function skipSomeDiffs(oldLine, newLine)
    -- ignore difference in bytes of line number because it's changing:
    -- line[%]
	local i,j,x,y
    if (string.find(oldLine, "line%[") ~= nil) and (string.find(newLine, "line%[") ~= nil) then
        i,j = string.find(oldLine, "line%[")
		x,y = string.find(oldLine, "%].")
		oldLine = string.sub(oldLine,0,i-1)..string.sub(oldLine,y+1)
		i,j = string.find(newLine, "line%[")
		x,y = string.find(newLine, "%].")
		newLine = string.sub(newLine,0,i-1)..string.sub(newLine,y+1)
		if oldLine == newLine then
			return 0
		else 
			return 1	
		end
    end
    return 1
end

--calling cpssDxChBrgGenIgmpSnoopEnableGet, this function processing will be documented in log history
ret = myGenWrapper("cpssDxChBrgGenIgmpSnoopEnableGet",{
	{"IN","GT_U8","devNum",0},
	{"IN","GT_PORT_NUM","portNum",0},
	{"OUT","GT_BOOL","statusPtr"}
})

--calling cpssDxChIpExceptionCommandGet, this function processing will be documented in log history
ret = myGenWrapper("cpssDxChIpExceptionCommandGet",{
	{"IN","GT_U8","devNum",0},
	{"IN","CPSS_DXCH_IP_EXCEPTION_TYPE_ENT","exceptionType","CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E"},
	{"IN","CPSS_IP_PROTOCOL_STACK_ENT","protocolStack","CPSS_IP_PROTOCOL_IPV4_E"},
	{"OUT","CPSS_PACKET_CMD_ENT","exceptionCmdPtr"}
})

--calling cpssDxChPortDefaultUPGet, this function processing will be documented in log history
ret = myGenWrapper("cpssDxChPortDefaultUPGet",{
	{"IN","GT_U8","devNum",0},
	{"IN","GT_PORT_NUM","portNum",0},
	{"OUT","GT_U8","defaultUserPrioPtr"}
})

--calling cpssDxChNstPortIsolationEnableGet, giving non valid devNum in order to cause an error that will invoke log history printing
ret = myGenWrapper("cpssDxChNstPortIsolationEnableGet",{
	{"IN","GT_U8","devNum",11},
	{"OUT","GT_BOOL","enablePtr"}
})

--empty the history by resizing the stack to size of 2
ret = myGenWrapper("cpssLogStackSizeSet",{
	{"IN","GT_U32","size",2}
})

--calling cpssDxChNstPortIsolationEnableGet again, giving non valid devNum in order to cause an error that will invoke log history printing which now should include only the current function
ret = myGenWrapper("cpssDxChNstPortIsolationEnableGet",{
	{"IN","GT_U8","devNum",11},
	{"OUT","GT_BOOL","enablePtr"}
})

--stop writing the log into the file
ret = myGenWrapper("appDemoOsLogStop",{})

--open the expected file
oldFid=fs.open("dxCh/examples/configurations/cpssLogHistory.txt","r")

--open the log that was created in run time
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
	fs.close(oldFid)
	fs.close(newFid)
	printLog("Restore configuration ..")
	executeLocalConfig("dxCh/examples/configurations/cpss_log_history_deconfig.txt")
	-- enable interrupts back
	result = myGenWrapper(
						"cpssDrvPpHwRegisterWrite", {
							{ "IN", "GT_U8",  "devNum", devNum},
							{ "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
							{ "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
							{ "IN","GT_U32", "regValue", globalMask.regValue}
						}
					)

	return
end

--printing both files on the screen in this format: line number, this line in the expected output, this line in the tested output 
counter=1
str = string.format("%-4s", "line") .. " " .. string.format("%-90s", "expected output").. " " .. string.format("%-90s", "tested output")
printLog(str)
while oldLine~=nil and newLine~=nil do
	str = string.format("%-4s", counter) .. " " .. string.format("%-90s", oldLine) .. " " .. string.format("%-90s", newLine)
	printLog(str)

	oldLine=skip_line_string
	while (oldLine ~= nil) and (string.find(oldLine, skip_line_string) ~= nil) do
		oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
	end

	newLine = fs.gets(newFid)
	counter=counter+1
end

--check if there were lines left in one of the files 
while oldLine~=nil do
	str = string.format("%-4s", counter) .. " " .. string.format("%-90s", oldLine) .. " " .. string.format("%-90s", "")
	printLog(str)
	oldLine=skip_line_string
	while (oldLine ~= nil) and (string.find(oldLine, skip_line_string) ~= nil) do
		oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
	end
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
oldFid=fs.open("dxCh/examples/configurations/cpssLogHistory.txt","r")
newFid=fs.open(filename,"r")
oldLine = fs.gets(oldFid)
newLine = fs.gets(newFid)

--comparing between the tested output and the expected output line by line
counter=1
while oldLine~=nil and newLine~=nil do
	if string.lower(oldLine)~=string.lower(newLine) then
		if skipSomeDiffs(string.lower(oldLine), string.lower(newLine)) == 1 then
			setFailState()
			printLog("line number:"..counter)
			printLog("expected output:"..oldLine)
			printLog("tested output:"..newLine)
			printLog("not match")
			fs.close(oldFid)
			fs.close(newFid)
			printLog("Restore configuration ..")
			executeLocalConfig("dxCh/examples/configurations/cpss_log_history_deconfig.txt")
			-- enable interrupts back
			result = myGenWrapper(
								"cpssDrvPpHwRegisterWrite", {
									{ "IN", "GT_U8",  "devNum", devNum},
									{ "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
									{ "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
									{ "IN","GT_U32", "regValue", globalMask.regValue}
								}
							)

			return
		end
	end
	oldLine=skip_line_string
	while (oldLine ~= nil) and (string.find(oldLine, skip_line_string) ~= nil) do
		oldLine = globalLogGetLineFormatted(fs.gets(oldFid), diffValues)
	end
	newLine = fs.gets(newFid)
	counter=counter+1
end

--if there are line left in one of the files, the files are not equall
if oldLine~=nil or newLine~=nil then
	setFailState()
	printLog("not in the same length")
	fs.close(oldFid)
	fs.close(newFid)
	printLog("Restore configuration ..")
	executeLocalConfig("dxCh/examples/configurations/cpss_log_history_deconfig.txt")
	-- enable interrupts back
	result = myGenWrapper(
						"cpssDrvPpHwRegisterWrite", {
							{ "IN", "GT_U8",  "devNum", devNum},
							{ "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
							{ "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
							{ "IN","GT_U32", "regValue", globalMask.regValue}
						}
					)

	return
end

--close files
fs.close(oldFid)
fs.close(newFid)

--deconfig
printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/cpss_log_history_deconfig.txt")

-- enable interrupts back
result = myGenWrapper(
					"cpssDrvPpHwRegisterWrite", {
						{ "IN", "GT_U8",  "devNum", devNum},
						{ "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
						{ "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
						{ "IN","GT_U32", "regValue", globalMask.regValue}
					}
				)


