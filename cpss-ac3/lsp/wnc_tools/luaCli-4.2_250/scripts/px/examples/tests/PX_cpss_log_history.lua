--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_cpss_log_history.lua
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


local diffValues = {
    s_s1   = "skip_line",
    s_s2   = "skip_line"
}

local  skip_line_string="skip_line"

diffValues.s_s1  = skip_line_string
diffValues.s_s2  = skip_line_string

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

--config
executeLocalConfig("px/examples/configurations/PX_cpss_log_history.txt")

--creating the tested file with unique file name
local clockval=os.clock()*1000
local filename="test_cpss_log_"..clockval..".txt"
ret = myGenWrapper("appDemoOsLogModeSet",{
    {"IN",TYPE["ENUM"],"mode",1},
    {"IN","string","name", filename}
})

--calling cpssPxPtpTaiClockCycleGet, this function processing will be documented in log history
ret = myGenWrapper("cpssPxPtpTaiClockCycleGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"OUT","GT_U32","secondsPtr"},
	{"OUT","GT_U32","nanoSecondsPtr"}
})
--calling cpssPxNetIfSdmaRxCountersGet, this function processing will be documented in log history
ret = myGenWrapper("cpssPxNetIfSdmaRxCountersGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"IN","GT_U32","rxQueue",0},
	{"OUT","CPSS_PX_NET_SDMA_RX_COUNTERS_STC","rxCountersPtr"}
})

--calling cpssPxEgressBypassModeGet, this function processing will be documented in log history
ret = myGenWrapper("cpssPxEgressBypassModeGet",{
	{"IN","GT_SW_DEV_NUM","devNum",devNum},
    {"OUT","GT_BOOL","phaBypassEnablePtr"},
	{"OUT","GT_BOOL","ppaClockEnablePtr"}
})

--calling cpssPxCncCounterWraparoundEnableGet, giving non valid devNum in order to cause an error that will invoke log history printing
ret = myGenWrapper("cpssPxCncCounterWraparoundEnableGet",{
	{"IN","GT_SW_DEV_NUM","devNum",11},
	{"OUT","GT_BOOL","enablePtr"}
})
--empty the history by resizing the stack to size of 2
ret = myGenWrapper("cpssLogStackSizeSet",{
    {"IN","GT_U32","size",2}
})

--calling cpssPxCncCounterWraparoundEnableGet again, giving non valid devNum in order to cause an error that will invoke log history printing which now should include only the current function
ret = myGenWrapper("cpssPxCncCounterWraparoundEnableGet",{
	{"IN","GT_SW_DEV_NUM","devNum",11},
	{"OUT","GT_BOOL","enablePtr"}
})
--stop writing the log into the file
ret = myGenWrapper("appDemoOsLogStop",{})

--Debug output of current log
-- newFid = fs.open(filename, "r")
-- newLine = fs.gets(newFid)
-- print(newLine)
-- while newLine~=nil do
--     print(newLine)
--     newLine = fs.gets(newFid)
-- end
-- fs.close(newFid)

--open the expected file
oldFid=fs.open("px/examples/configurations/PX_cpssLogHistory.txt","r")

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
    executeLocalConfig("px/examples/configurations/PX_cpss_log_history_deconfig.txt")
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
        oldLine = fs.gets(oldFid)
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
        oldLine = fs.gets(oldFid)
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
oldFid=fs.open("px/examples/configurations/PX_cpssLogHistory.txt","r")
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
            executeLocalConfig("px/examples/configurations/PX_cpss_log_history_deconfig.txt")
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
        oldLine = fs.gets(oldFid)
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
    executeLocalConfig("px/examples/configurations/PX_cpss_log_history_deconfig.txt")
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
executeLocalConfig("px/examples/configurations/PX_cpss_log_history_deconfig.txt")

-- enable interrupts back
result = myGenWrapper(
                    "cpssDrvPpHwRegisterWrite", {
                        { "IN", "GT_U8",  "devNum", devNum},
                        { "IN", "GT_U32", "portGroupId", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                        { "IN", "GT_U32", "regAddr", MG_Global_Interrupt_mask },
                        { "IN","GT_U32", "regValue", globalMask.regValue}
                    }
                )
