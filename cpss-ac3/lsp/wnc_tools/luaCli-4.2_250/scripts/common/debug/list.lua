--********************************************************************************
--*              (c), Copyright 2013, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* list.lua
--*
--* DESCRIPTION:
--*       Print a list of all configured (available) suits
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************


-- externs
if cli_C_functions_registered("wrapl_cmdEventRun")
then


-- ************************************************************************
---
--  ExecuteGaltisCmd
--        @description  Log a list of all configured (available) suits
--
--        @param params.ut_name        - name of the specific test case
--
--        @return       true on success, otherwise false
--
local function ListTests(params)
    local status

    if params.ut_name == nil then
        status = wrapl_cmdEventRun("utfTestsList 0")
    else
        status = wrapl_cmdEventRun("utfTestsList \"" .. params.ut_name .. "\"")
    end
    return status
end

-- list [%suit_name]
CLI_addCommand("diag_manual", "list", {
    help = "Print a list of all configured (available) UT suits",
    func = ListTests,
    params = {
        { type="named",
            { format="%string", name="ut_name",
              help= "Specify the suit name to log inside test cases"}
        }
    }
})

end
