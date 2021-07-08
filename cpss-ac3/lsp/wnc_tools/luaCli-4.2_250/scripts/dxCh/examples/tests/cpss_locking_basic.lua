--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpss_locking_basic.lua
--*
--* DESCRIPTION:
--*       Basic test for CPSS protection (mutex locking). 
--*       Test 2 APIs accessing to the same register
--* 
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

NOT_SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_CHEETAH_E")

local tasks = 2
local maxDelay = 45
local devNum  = devEnv.dev

task_param1 = {
    {
        api1 = "cpssDxChPclLookup1ForNotRoutedPacketsEnableSet",
        api2 = "cpssDxChPclLookup1ForNotRoutedPacketsEnableGet",
        param1 = {{"IN","GT_U8","devNum", devNum},{"IN","GT_BOOL","enable", false}},
        param2 = {{"IN","GT_U8","devNum", devNum},{"OUT","GT_BOOL","enablePtr"}},
        param3 = {{"IN","GT_U8","devNum", devNum},{"IN","GT_BOOL","enable", true}},
        checkValue = false,
        count = 1000,
        delay = 0
    },
}
task_param2 = {
    {
        api1 = "cpssDxChPclPortIngressLookup0Sublookup1TypeSet",
        api2 = "cpssDxChPclPortIngressLookup0Sublookup1TypeGet",
        param1 = {{"IN","GT_U8","devNum", devNum},{"IN","CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT","lookupType", "CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_FULL_E"}},
        param2 = {{"IN","GT_U8","devNum", devNum},{"OUT","CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT","lookupTypePtr"}},
        param3 = {{"IN","GT_U8","devNum", devNum},{"IN","CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_ENT","lookupType", "CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_PARTIAL_E"}},
        checkValue = "CPSS_DXCH_PCL_IPCL01_LOOKUP_TYPE_FULL_E",
        count = 1000,
        delay = 0
    },
}

-- MAIN
printLog("Create task")
local msgQ = luaMsgQCreate("tasks", "GT_U32", 5)
local taskIds = {}

for i=1,tasks do
    f = loadstring("return task_param"..i)

    taskIds[i] = {}
    taskIds[i].tid = luaTaskCreate("test_task"..i, "dxCh/examples/common/cpss_locking_basic_task_utils.lua", { msgQ, f() })
    taskIds[i].tname = "test_task"..i
    taskIds[i].params = f()
end

printLog("Wait for tasks")
for i=1,tasks do
    new_instance_shell_cmd_wait(taskIds[i].tid, maxDelay)
end

local msg = 0
for i=1,tasks do
    status, msg = luaMsgQRecv(msgQ, 1000)
    if status == 0 then
        if msg == 1 then
            setFailState()
        end
    end
end


