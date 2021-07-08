--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* type_cpss_enable.lua
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--*******************************************************************************


CLI_type_dict["cpssApiLogMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Select filtering mode",
    enum = {
        ["set-only"] = {
            value = "set-only",
            help  = "Logging set API's only"
        },
        ["set-and-get"] = {
            value = "set-and-get",
            help  = "Logging both -> set and get API's (default)"
        },
        ["verbose"] = {
            value = "verbose",
            help  = "Logging in file and on-screen"
        },
        ["laconic"] = {
            value = "laconic",
            help  = "Logging in file only"
        }
    }
}
