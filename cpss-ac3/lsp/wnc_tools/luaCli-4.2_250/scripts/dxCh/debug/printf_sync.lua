--********************************************************************************
--*              (c), Copyright 2013, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* set_uart_sync.lua
--*
--* DESCRIPTION:
--*       Set/unset synchronous mode for UART (linux BM)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************


-- externs


-- list [%suit_name]
CLI_addCommand("debug", "printf-sync", {
    help = "Force UART output to be synchronous",
    func = function()
        myGenWrapper("osPrintSyncEnable",{{"IN","GT_BOOL","enable",true}})
        return true
    end
})

CLI_addCommand("debug", "no printf-sync", {
    help = "Force UART output to be asynchronous",
    func = function()
        myGenWrapper("osPrintSyncEnable",{{"IN","GT_BOOL","enable",false}})
        return true
    end
})
