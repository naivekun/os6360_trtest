--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* eventTable_types.lua
--*
--* DESCRIPTION:
--*       event talbe type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:eventType
--


CLI_type_dict["eventType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "event types",
    enum = {
        ["CPSS_PP_EVENTS"] = { value=0, help="PP events" },
        ["CPSS_XBAR_EVENTS"] = { value=1, help="XBAR events" },
        ["CPSS_FA_EVENTS"] = { value=2, help="FA events" }
    }
}












