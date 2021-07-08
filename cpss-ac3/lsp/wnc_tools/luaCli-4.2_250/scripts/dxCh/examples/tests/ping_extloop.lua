--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ping_extloop.lua
--*
--* DESCRIPTION:
--*       The test for ping through external cable loop
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

executeLocalConfig("dxCh/examples/configurations/ping.txt")
executeLocalConfig("dxCh/examples/configurations/ping_extloop.txt")

CLI_exec("traffic")

-- check if ping successful
if CLI_exec("ping 192.168.0.1 noPortRestrict") then
    printLog("ping OK")
else
    printLog("ping failed")
    setFailState()
end

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/ping_extloop_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/ping_deconfig.txt")
