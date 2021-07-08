--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cmdDefs.lua
--*
--* DESCRIPTION:
--*       lua sturt-up scripts
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

CLI_prompts["bpe"] = "(bpe)#"
CLI_addCommand("exec", "bpe-mode", {
    func=CLI_change_mode_push,
    constFuncArgs={"bpe"},
    help="Enter BPE mode"
}) 


require_safe("commands/trunk")
require_safe("commands/echannel")
require_safe("commands/global")
require_safe("commands/interface")
require_safe("commands/debug")
