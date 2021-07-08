--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_protected.lua
--*
--* DESCRIPTION:
--*       send all traffic to an uplink port 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants



-- ************************************************************************
---
--  switchport
--        @description  isolate traffic at Layer 2 from other protected ports on the switch
--
--        @param params             - params["flagNo"]: no command property
--                   --
--        @return       true on success, otherwise false and error message
--
local function switchport(params)
    local result, values, ret
    local dstTable
    local dstDev,dstPort, dstTrunk
    local all_ports, dev_ports, devnum      
    local enable=1

    all_ports = getGlobal("ifRange")--get table of ports    
    
    if true == params.flagNo  then
        enable=0
    end

    dstTable=params[params["etOrPrtChnl"]]
    if "ethernet"==params["etOrPrtChnl"] then
        dstDev=dstTable.devId
        dstPort=dstTable.portNum
        dstTrunk=0    --dstTrunk=false
    end
    
    if "port-channel"==params["etOrPrtChnl"] then
        dstPort= dstTable
        dstTrunk=1    --dstTrunk=true
    end
    

    
    
    for devNum, dev_ports in pairs(all_ports) do
    
        ret = myGenWrapper("cpssDxChBrgPrvEdgeVlanEnable",{  --enable device
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_BOOL","enable,",1}
            })

        if 0~=ret then
            print("Failed to enable switchport on device: ".. devNum)
        else
            for k, port in pairs(dev_ports) do

                result, values =
                    device_to_hardware_format_convert(dstDev)
                if        0 == result                   then
                    dstDev = values
                else
                    print("Failed to convert device number to HW: ".. devNum.." port: "..port.." to device: "..dstDev.." port: "..dstPort)
                end

                ret = myGenWrapper("cpssDxChBrgPrvEdgeVlanPortEnable",{  --enable switchport
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PORT_NUM","portNum",port},
                    {"IN","GT_BOOL","enable",enable},
                    {"IN","GT_PORT_NUM","dstPort",dstPort},
                    {"IN","GT_U8","GT_HW_DEV_NUM",dstDev},
                    {"IN","GT_BOOL","dstTrunk,",dstTrunk}
                    })

                if 0~=ret then
                    if 0==dstTrunk then
                        print("Failed to enable switchport on device: ".. devNum.." port: "..port.." to device: "..dstDev.." port: "..dstPort)
                    else
                        print("Failed to enable switchport on device: ".. devNum.." port: "..port.." to trunk: "..dstPort)

                    end
                end
            end
        end
    end

end

--------------------------------------------------------------------------------
-- command registration: switchport protected
--------------------------------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "switchport protected", {
  func   = switchport,
   help   = "Override the Filtering Database decision and send all traffic to an uplink port",
   params = {
        { type = "named",
            "#etOrPrtChnl",
          mandatory = { "etOrPrtChnl" }
        }
    }
  
})

--------------------------------------------------------------------------------
-- command registration: no switchport protected
--------------------------------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "no switchport protected", {
    func=function(params)
        params.flagNo=true
        return switchport(params)
    end,
  help   = "Disable overriding Filtering Database decision",

  }
)


