--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* run_unit_tests.lua
--*
--* DESCRIPTION:
--*       run unit-tests
--*
--*
--********************************************************************************




-- ************************************************************************
---
--  run_unit_test
--        @description  run all basic UT for DX_FAMILY
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function run_unit_test(params)
  local result, values 
    local status, err
	
    status = true
  
    result, values = myGenWrapper("utfTestsTypeRun", {
            { "IN", "GT_32"    , "numOfRuns", 1 },
            { "IN", "GT_BOOL"  , "fContinue", 1 },
            { "IN", "GT_32"    , "testType" , 2 },
            { "IN", "GT_32"    , "testType" , 0 }
  }
  )
  if (result ~= 0) then
    status = false
    err = returnCodes[result]
  end
  
  return status, err
end


-- ************************************************************************
---
--  run_enhanced_unit_test
--        @description  run all Enhanced UT 
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function run_enhanced_unit_test(params)
  local result, values 
    local status, err
	
    status = true
  
    result, values = myGenWrapper("utfTestsTypeRun", {
            { "IN", "GT_32"    , "numOfRuns", 1 },
            { "IN", "GT_BOOL"  , "fContinue", 1 },
            { "IN", "GT_32"    , "testType" , 6 },
            { "IN", "GT_32"    , "testType" , 0 }
  }
  )
  if (result ~= 0) then
    status = false
    err = returnCodes[result]
  end
  
  return status, err
end

-- *debug*
-- run unit-test
CLI_addCommand("debug", "run unit-test", {
    func = run_unit_test,
    help = "Run all basic UT for DX_FAMILY",
})

-- *debug*
-- run enhanced-unit-test
CLI_addCommand("debug", "run enhanced-unit-test", {
    func = run_enhanced_unit_test,
    help = "Run all Enhanced UT ",
})

