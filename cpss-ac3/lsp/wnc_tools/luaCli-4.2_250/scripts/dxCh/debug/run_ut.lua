--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* run_ut.lua
--*
--* DESCRIPTION:
--*       Run UT in lua debug mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  run_ut
--        @description  Reads from a register
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function run_ut(params)
    local result, values 
    local status, err

    status = true

    result, values = myGenWrapper(
    "utfLogOutputSelect", {
        { "IN", "GT_32"  , "eLogOutput", 1 }
    })
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end


    status = true

    if params.test_case ~= nil then
        --"testPathPtr"
        result, values = myGenWrapper(
        "utfTestsRun", {
                        {"IN", "string", "testPathPtr", params.test_case},
                        { "IN", "GT_32"  , "numOfRuns", 1 },
                        { "IN", "GT_BOOL"  , "continueFlag", 0 }
        }
        )
    else
        result, values = myGenWrapper(
        "utfTestsTypeRun", {
            { "IN", "GT_32"  , "numOfRuns", 1 },
                        { "IN", "GT_BOOL"  , "fContinue", 0 },
                        { "IN", "GT_32"  , "testType", 1 },
                        { "IN", "GT_32"  , "testType", 2 },
                        { "IN", "GT_32"  , "testType", 4 },
                        { "IN", "GT_32"  , "testType", 5 },
                        { "IN", "GT_32"  , "testType", 0 }
        }
        )
    end
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end

    return status, err
end


CLI_prompts["diagnostics"] = "(diagnostics)#"
CLI_prompts["diag_manual"] = "(diag-manual)#"
CLI_prompts["diag_auto"] = "(diag-auto)#"

CLI_addCommand("exec", "diagnostics",  {
    func=CLI_change_mode_push,
    constFuncArgs={"diagnostics"},
    help="Switch to the diagnostics mode"
})
CLI_addCommand("diagnostics", "manual", {
    func=CLI_change_mode_push,
    constFuncArgs={"diag_manual"},
    help="Switch to the manual diagnostics mode"
})
CLI_addCommand("diagnostics", "automatic", {
    func=CLI_change_mode_push,
    constFuncArgs={"diag_auto"},
    help="Switch to the automatic diagnostics mode"
})  

-- run_ut [name %string] [device %deID]
CLI_addCommand("diag_manual", "run_ut", {
    func = run_ut,
    help = "Run Unit Tests",
    params={
        {
            type="named",
            {
                format="name %string",
                name = "test_case",
                help = "Name of the specific test case to run"
            },
            {
                format="device %devID",
                name="devID",
                help="The device number"
            }
        }
    }
})
