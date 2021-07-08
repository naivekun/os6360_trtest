--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_cli_commands.lua
--*
--* DESCRIPTION:
--*       printing of the command tree
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

do

local function runPerfTest()
    print("Running mv_testperf(), LinuxBM only...")
    cpssGenWrapper("mv_testperf",{})
    print("done.")
    print("Running osTestPerf()...")
    cpssGenWrapper("osTestPerf",{})
    print("done.")
end

-- *debug*
-- run performance test
CLI_addHelp("debug", "run", "Run procedure")
CLI_addHelp("debug", "run performance", "Run performance related procedure")
CLI_addCommand("debug", "run performance test", {
    func=runPerfTest,
    help="Run performance test"
})

end
