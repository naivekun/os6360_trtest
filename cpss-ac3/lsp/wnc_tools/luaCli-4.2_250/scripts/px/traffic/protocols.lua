--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* protocols.lua
--*
--* DESCRIPTION:
--*       Packet analizing and protocol implementations tools
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
require("common/traffic/protocols")

do
    local print_saved = print
    print = function()
    end
    cmdLuaCLI_registerCfunction("prvLuaTgfTransmitPackets")
    cmdLuaCLI_registerCfunction("wrlPxRxConfigure")
    cmdLuaCLI_registerCfunction("wrlPxRxDisable")
    print = print_saved
end
if CLI_execution_unlock == nil or CLI_execution_lock == nil then
    CLI_execution_lock = function()
    end
    CLI_execution_unlock = function()
    end
end
if wrlPxRxConfigure == nil then
    function wrlPxRxConfigure(msgqId)
        cpssGenWrapper("luaPxRxConfigure", {{ "IN", "GT_U32", "rxQId", msgqId }})
    end
    function wrlPxRxDisable()
        cpssGenWrapper("luaPxRxDisable", {})
    end
end
