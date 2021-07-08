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


-- *debug*
-- show cli commands
CLI_addHelp("debug", "show", "Print debug information")
CLI_addHelp("debug", "show cli", "Print debug information related to CLI")
CLI_addCommand("debug", "show cli commands", {
    func=CLI_show_desc,
    constFuncArgs={ "current" },
    help="Print the command tree"
})
CLI_addCommand("debug", "show cli commands all", {
    func=CLI_show_desc,
    constFuncArgs={ "all" },
    help="Print the command tree for all folders"
})
