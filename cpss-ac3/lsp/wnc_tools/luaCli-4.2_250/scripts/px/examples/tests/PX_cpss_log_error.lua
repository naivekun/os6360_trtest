--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_cpss_log_error.lua
--*
--* DESCRIPTION:
--*       The test for testing cpss_log_error
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local skip = true

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


local function cleanBeforeExit()
--close files
fs.close(oldFid)
fs.close(newFid)

--deconfig
printLog("Restore configuration ..")
executeLocalConfig("px/examples/configurations/PX_cpss_log_error_deconfig.txt")

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
executeLocalConfig("px/examples/configurations/PX_cpss_log_error.txt")

--creating the tested file with unique file name
local clockval=os.clock()*1000
local filename="test_cpss_log_"..clockval..".txt"
ret = myGenWrapper("appDemoOsLogModeSet",{
    {"IN",TYPE["ENUM"],"mode",1},
    {"IN","string","name", filename}
})

--calling cpssPxDiagPrbsSerdesStatusGet, this function processing will be documented by the log
ret = myGenWrapper("cpssPxDiagPrbsSerdesStatusGet",{
	{"IN","GT_SW_DEV_NUM","devNum",0},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",0},
    {"IN","GT_U32","laneNum",1666},
	{"OUT","GT_BOOL","lockedPtr"},
    {"OUT","GT_U32","errorCntrPtr"},
    {"OUT","GT_U64","patternCntrPtr"},
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
oldFid=fs.open("px/examples/configurations/PX_cpssLogError.txt","r")

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
oldFid=fs.open("px/examples/configurations/PX_cpssLogError.txt","r")
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

