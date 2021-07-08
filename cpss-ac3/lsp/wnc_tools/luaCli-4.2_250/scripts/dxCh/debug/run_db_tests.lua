--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* db_tests.lua
--*
--* DESCRIPTION:
--*       Run UT tests in lua debug mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


local function run_db_tests(params)
    local tests = {".cpssDxChBrgCntDropCntrModeSet"
             -- "test2",
             --  "test3"
    }

    for t=1, #tests do
        local rc = myGenWrapper("utfTestsRun", {
            { "IN", "string", "testPath", tests[t] },
            { "IN", "GT_U32", "numOfRuns", 1 },
            { "IN", "GT_BOOL", "continueFlag", false }
        })
    end

end

CLI_addCommand("diag_auto", "run_tests", {
    func = run_db_tests,
    help = "Run required tests",
    params={
 	    { type="named",
            { format="device %devID",name="devID", help="The device number" }
        }
    }
})
