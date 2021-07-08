--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_fwdtolb.lua
--*
--* DESCRIPTION:
--*     Types introdused by 'Forwarding to loopback/service port' related
--*      CLI commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChMaximumMirrorAnalyzerIndex")

CLI_type_dict["evidx"] = {
    checker = CLI_check_param_number,
    min  = 0,
    max  = 4095,
    help = "eVidx number."
}

CLI_type_dict["loopback_profile"] = {
    checker = CLI_check_param_number,
    min = 0,
    max = 3,
    help = "Loopback profile number."
}

local result, maxAnalyzerIndex = wrlCpssDxChMaximumMirrorAnalyzerIndex()
if result ~= 0 then
    -- an error occured
    maxAnalyzerIndex = 6
end
CLI_type_dict["analyzer_index"] = {
    checker = CLI_check_param_number,
    min=0,
    max = maxAnalyzerIndex,
    help = "Analyzer index number"
}

CLI_type_dict["traffic_class_range"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=7,
    complete = CLI_complete_param_number_range,
    help = "Traffic Classes (0..7) range. Examples: \"7\", \"0,2-4,7\", \"all\""
}
