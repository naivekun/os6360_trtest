--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* loopback.lua
--*
--* DESCRIPTION:
--*       configure loopback mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  loopbackHandler
--        @description  enable/disable loopback
--
--        @param params             - params["flagNo"]: no command property
--                    params["loopback_mode"]: internal/serdes
--                    params[params["loopback_mode"]]: internal/0/1/2.
--                    0: serdes rx2tx
--                    1: serdes analogTx2rx
--                    2: serdes digitalTx2rx
--
--        @return       true on success, otherwise false and error message
--
local function loopbackHandler(params)
    local isOk = true



    local serdesModesStr={
        CPSS_PX_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E  = "analogTx2rx",
        CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E = "digitalTx2rx",
        CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E = "rx2tx" }

    local enable         = not params.flagNo
    local configInternalOnly = params["loopback_mode"] == "internal"
    local enableInternal = enable and params["loopback_mode"] == "internal"
    local enableSerdes   = enable and not enableInternal

    local serdesMode     = enableSerdes and params[params["loopback_mode"]]
        or "CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E"



    local all_ports, dev_ports, devNum
    all_ports = getGlobal("ifRange")--get table of ports

    for devNum, dev_ports in pairs(all_ports) do--disable internal


        for k, port in pairs(dev_ports) do
                    ret = myGenWrapper(
                        "cpssPxPortInternalLoopbackEnableSet",{  --enable internal
                            {"IN","GT_SW_DEV_NUM","devNum",devNum},
                            {"IN","GT_U32","portNum",port},
                            {"IN","GT_BOOL","enable", enableInternal}
                    })
                if ret ~= 0 then
                    print(string.format("%d/%d, loopback internal : %s",
                                        devNum, port, returnCodes[ret]))
                    isOk = false
                end

                if not configInternalOnly then
                    local ret = myGenWrapper(
                        "cpssPxPortSerdesLoopbackModeSet",{--enable serdes
                            {"IN","GT_SW_DEV_NUM","devNum",devNum},
                            {"IN","GT_U32","portNum",port},
                            {"IN","CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT","mode", serdesMode}
                    })
                    if ret ~= 0 and enable then -- ignore result for NO command. Some devices does not support it.
                        print(string.format("%d/%d, loopback serdes %s : %s",
                                        devNum, port,
                                        serdesModesStr[serdesMode],
                                        returnCodes[ret]))
                        isOk = false
                    end
                end
        end
    end
    return isOk
end



--------------------------------------------------------------------------------
-- command registration: loopback mode
--------------------------------------------------------------------------------
CLI_addHelp("interface", "loopback", "Configure loopback")
CLI_addCommand("interface", "loopback mode", {
  func   = loopbackHandler,
  help   = "Define loopback mode",
  params = {{ type = "named", "#loopback", mandatory = {"loopback_mode"}}}
})

--------------------------------------------------------------------------------
-- command registration: no loopback
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no loopback", {
    func=function(params)
        params.flagNo=true
        return loopbackHandler(params)
    end,
    help   = "Disable loopback",
})


