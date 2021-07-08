--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_debug_run_unit_tests.lua
--*
--* DESCRIPTION:
--*       run unit-tests
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--************************************************************************
--  runUnitTest
--
--  @description  run all basic UT for PX_FAMILY
--
--  @param params - unused parameter
--
--  @return  true on success. Otherwise false and error message
--
--************************************************************************
local function runUnitTest(params)
    local ret, val

    ret, val = myGenWrapper("utfTestsTypeRun", {
            { "IN", "GT_32",   "numOfRuns", 1 },
            { "IN", "GT_BOOL", "fContinue", 1 },
            { "IN", "GT_32",   "testType" , 8 },
            { "IN", "GT_32",   "testType" , 0 }
        })
    if (ret ~= LGT_OK) then
        return false, "utfTestsTypeRun 1,1,8,0 failed: rc=" .. ret
    end

    return true
end


--************************************************************************
--  runEnhancedUnitTest
--
--  @description  run all enhanced UT for PX_FAMILY
--
--  @param params - unused parameter
--
--  @return  true on success. Otherwise false and error message
--
--************************************************************************
local function runEnhancedUnitTest(params)
    local ret, val

    ret, val = myGenWrapper("utfTestsTypeRun", {
            { "IN", "GT_32",   "numOfRuns", 1 },
            { "IN", "GT_BOOL", "fContinue", 1 },
            { "IN", "GT_32",   "testType" , 9 },
            { "IN", "GT_32",   "testType" , 0 }
        })
    if (ret ~= LGT_OK) then
        return false, "utfTestsTypeRun 1,1,9,0 failed: rc=" .. ret
    end

    return true
end


--******************************************************************************
-- add help: run
--******************************************************************************
CLI_addHelp("debug", "run", "Run procedure or unit tests")

--******************************************************************************
-- command registration: run unit-test
--******************************************************************************
CLI_addCommand("debug", "run unit-test", {
    func = runUnitTest,
    help = "Run all basic UT for PX_FAMILY"
})

--******************************************************************************
-- command registration: run enhanced-unit-test
--******************************************************************************
CLI_addCommand("debug", "run enhanced-unit-test", {
    func = runEnhancedUnitTest,
    help = "Run all enhanced UT for PX_FAMILY"
})
